#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <WakeOnLan.h>
#include <driver/i2s.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"
#include "audio_clips.h"   // مقاطع "مرحبا" و "تمام" (ملف مرفق في نفس الفولدر)
#include "pwa_assets.h"   // PWA: manifest + service worker + أيقونات (ملف مرفق في نفس الفولدر)
#include "index_html.h"    // صفحة الويب (ملف مرفق في نفس الفولدر)

// لازم يتعرّف هنا فوق (قبل أي دالة) عشان الـ prototypes اللي Arduino بيولّدها تلقائياً تشوفه
struct Clip { const int16_t* d; uint32_t n; };
// --- الإعدادات الشبكية والتوكن ---
const char* ssid = "We";
const char* password = "20121975."; // أدخل كلمة سر الواي فاي
const String WIT_AI_TOKEN = "XHEAZVV4PYENGBS6YUNWKTQEZOFNPTPM";

// --- إعدادات المايك INMP441 (I2S) ---
#define I2S_WS   15
#define I2S_SD   17
#define I2S_SCK  16
#define I2S_PORT I2S_NUM_0
#define RECORD_TIME_SECONDS 6 // أقصى مدة تسجيل (بيقف قبلها لما تسكت)
// --- كشف نهاية الكلام (VAD) - لو بيقطع كلامك أو مش بيحس بيك عدّل الأرقام دي ---
#define VAD_CHUNK         320     // 20ms @16kHz
// VAD_MIN_LEVEL بقى قابل للضبط من صفحة الويب (vadMinLevel) - الافتراضي 8
#define VAD_NOISE_MULT    3.0f    // الكلام لازم يعدي ضوضاء الغرفة بكام مرة
#define VAD_MAX_NOISE     30      // سقف تقدير الضوضاء
#define VAD_SILENCE_MS    700     // سكوت بعد الكلام بالمللي ثانية = نهاية الأمر
#define VAD_NO_SPEECH_MS  3000    // لو مفيش كلام خالص بعد المدة دي: وقف
#define VAD_MIN_SPEECH_MS 120     // أقل مدة كلام تتحسب (بيتجاهل الطرقعات)
#define VAD_PAD_MS        250     // نسيب صمت قبل وبعد الكلام

// --- مكبر الصوت MAX98357A (I2S) ---
// التوصيل: VIN->5V | GND->GND | BCLK->GPIO5 | LRC->GPIO7 | DIN->GPIO8 | SD و GAIN سيبهم فاضيين (GAIN فاضي = 9dB)
#define AMP_BCLK    5
#define AMP_LRC     7
#define AMP_DIN     8
#define AMP_PORT    I2S_NUM_1   // المايك على I2S_NUM_0
#define SPK_VOLUME  80          // مستوى الصوت 0-100 (قلله لو في تشويه)

#define RGB_LED_PIN RGB_BUILTIN
#define STANDBY_LED_MS   60000UL   // الـ LED الأزرق ينور المدة دي بعد آخر نشاط وبعدين يطفي
#define STANDBY_LED_BLUE 15        // سطوع الأزرق في الاستعداد (0-255)

// --- لو ماسمعتكش / مافهمتكش: بتسمّعك الرد وتفتح المايك تاني من غير wake word ---
#define RETRY_MAX 1           // عدد المحاولات الإضافية بعد أول فشل (0 = ألغي الميزة)

// --- الريلاي (2 قناة) ---
#define RELAY1_PIN 1
#define RELAY2_PIN 2
#define RELAY_ACTIVE_LOW 1    // 1 = موديول الريلاي بيشتغل لما الطرف LOW (أغلب الموديولات) | 0 = بيشتغل HIGH
// التوصيل (عزل تام): ESP32 3.3V->VCC | شاحن 5V->JD-VCC (شيل الـ jumper) | GND مشترك | GPIO1->IN1 | GPIO2->IN2
// ملحوظة: متستخدمش نفس الأطراف دي لـ IR من صفحة الويب

// --- تحديث OTA (عن طريق الواي فاي) ---
#define OTA_HOSTNAME "voice-assistant"
#define OTA_PASSWORD "12345678"   // غيّره! بيتطلب في Arduino IDE وفي صفحة /update (اسم المستخدم: admin)
#define WAKE_WORD "alexa"  // jarvis | alexa | hiesp
#define MIC_SHIFT 12          // gain ثابت لكشف الـ wake word: 14=الأصلي | 13=2x | 12=4x | 11=8x
#define REC_TARGET_PEAK 24000 // أقصى ارتفاع للصوت المسجل بعد التطبيع (من 32767)
#define REC_MAX_GAIN 400.0f   // أقصى تكبير مسموح للتسجيل

WebServer server(80);
WiFiUDP udp;
WakeOnLan WOL(udp);

// --- مكتبات ESP-SR الخاصة بـ WakeNet ---
#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "model_path.h"

const esp_wn_iface_t *wakenet = NULL;
model_iface_data_t *model_data = NULL;
srmodel_list_t *models = NULL;
int chunk_size = 0;
int16_t *pcm16_chunk = NULL;
int32_t *raw_chunk = NULL;

// ==================== الوقت (NTP) ====================
// التوقيت المحلي لمصر (EET / EEST - توقيت صيفي تلقائي). غيّره لو في بلد تانية.
#define TZ_INFO "EET-2EEST,M4.5.5/0,M10.5.4/24"
static volatile bool ntpFlag = false;           // بيتفعّل لما NTP يضبط الساعة (الطباعة بتتم من loop)
static bool timeSynced() { return time(nullptr) > 1700000000L; }

// بادئة كل سطر لوج: [hh:mm:ss.mmm AM/PM] لو الساعة مضبوطة، أو [+HH:MM:SS.mmm] (وقت من تشغيل الجهاز) لو لسه
static size_t tsPrefix(char* out, size_t cap) {
  struct timeval tv; gettimeofday(&tv, nullptr);
  if (tv.tv_sec > 1700000000L) {
    struct tm t; localtime_r(&tv.tv_sec, &t);
    int h12 = t.tm_hour % 12 == 0 ? 12 : t.tm_hour % 12;                 // نظام 12 ساعة
    return snprintf(out, cap, "[%02d:%02d:%02d.%03d %s] ", h12, t.tm_min, t.tm_sec, (int)(tv.tv_usec / 1000), t.tm_hour < 12 ? "AM" : "PM");
  }
  uint32_t ms = millis();
  return snprintf(out, cap, "[+%02u:%02u:%02u.%03u] ", (unsigned)(ms / 3600000UL), (unsigned)(ms / 60000UL % 60), (unsigned)(ms / 1000UL % 60), (unsigned)(ms % 1000UL));
}
static String nowStr() {                        // "YYYY-MM-DD hh:mm:ss AM/PM" أو "" لو الساعة مش مضبوطة
  if (!timeSynced()) return "";
  time_t n = time(nullptr); struct tm t; localtime_r(&n, &t);
  char b[24]; strftime(b, sizeof(b), "%Y-%m-%d %I:%M:%S %p", &t);
  return String(b);
}

// ==================== Serial Monitor (نسخة من كل اللوج في ذاكرة للويب) ====================
#define LOG_SIZE 8192
static char logBuf[LOG_SIZE];
static uint32_t logTotal = 0, logFloor = 0;   // logFloor: أي لوج قبله اتمسح

// كل سطر جديد بيتكتب له وقته من الجهاز نفسه (في السيريال الحقيقي وفي صفحة الويب)
class TeeSerial : public Print {
  bool bol = true;                              // بداية سطر؟
  char ob[160]; size_t ol = 0;
  void emit(char c) {
    logBuf[logTotal % LOG_SIZE] = c; logTotal++;
    ob[ol++] = c;
    if (ol >= sizeof(ob)) flushOut();
  }
  void flushOut() { if (ol) { Serial.write((const uint8_t*)ob, ol); ol = 0; } }
 public:
  using Print::write;
  void begin(unsigned long baud) { Serial.begin(baud); }
  size_t write(uint8_t c) override { return write(&c, 1); }
  size_t write(const uint8_t* b, size_t n) override {
    for (size_t i = 0; i < n; i++) {
      char c = (char)b[i];
      if (bol && c != '\n' && c != '\r') {
        char p[40]; size_t pl = tsPrefix(p, sizeof(p));
        for (size_t k = 0; k < pl && k < sizeof(p) - 1; k++) emit(p[k]);
        bol = false;
      }
      emit(c);
      if (c == '\n') bol = true;
    }
    flushOut();
    return n;
  }
};
static TeeSerial LogSerial;
#define Serial LogSerial   // من هنا أي Serial.print بيروح للسيريال الحقيقي + صفحة الويب


// دالة التحكم بإضاءة الـ RGB LED
static uint32_t ledIdleSince = 0;   // آخر مرة حصل فيها نشاط (أي لون غير الاستعداد)
static int8_t   ledLast = -1;       // -1 = غير معروف | 0 = مطفي | 1 = أزرق استعداد

static void rgbRaw(uint8_t r, uint8_t g, uint8_t b) {
#ifdef RGB_BUILTIN
  neopixelWrite(RGB_LED_PIN, r, g, b);
#endif
}

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
  rgbRaw(r, g, b);
  ledIdleSince = millis();   // أي نشاط بيبدأ عدّاد الدقيقة من جديد
  ledLast = -1;
}

// بيتنادى من loop(): أزرق دقيقة بعد آخر نشاط، وبعدها مطفي
static void standbyLed() {
  int8_t want = (millis() - ledIdleSince < STANDBY_LED_MS) ? 1 : 0;
  if (want != ledLast) {
    rgbRaw(0, 0, want ? STANDBY_LED_BLUE : 0);
    ledLast = want;
  }
}

void flashRGB(uint8_t r, uint8_t g, uint8_t b, int count = 2) {
  for (int i = 0; i < count; i++) {
    setRGB(r, g, b);
    delay(150);
    setRGB(0, 0, 0);
    delay(150);
  }
}


// ==================== الصوت (MAX98357A عبر I2S) ====================
void audioInit() {
  i2s_config_t cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = AUDIO_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,   // نفس العينة يمين وشمال = صوت كامل
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 6,
    .dma_buf_len = 256,
    .use_apll = false,
    .tx_desc_auto_clear = true,                     // سكوت تلقائي وقت الراحة = مفيش هسهسة
    .fixed_mclk = 0
  };
  i2s_pin_config_t pins = {
    .mck_io_num = I2S_PIN_NO_CHANGE,
    .bck_io_num = AMP_BCLK,
    .ws_io_num = AMP_LRC,
    .data_out_num = AMP_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  esp_err_t e = i2s_driver_install(AMP_PORT, &cfg, 0, NULL);
  if (e == ESP_OK) e = i2s_set_pin(AMP_PORT, &pins);
  Serial.printf("MAX98357A I2S (BCLK=%d LRC=%d DIN=%d): %s\n", AMP_BCLK, AMP_LRC, AMP_DIN, e == ESP_OK ? "OK" : "FAILED");
}

// مجموعات ردود (بيختار واحد عشوائي ومش بيكرر نفس الرد مرتين ورا بعض)
static const Clip WAKE_CLIPS[] = { {wake1_pcm, wake1_len}, {wake2_pcm, wake2_len}, {wake3_pcm, wake3_len}, {wake4_pcm, wake4_len} };
static const Clip OK_CLIPS[]   = { {ok1_pcm, ok1_len}, {ok2_pcm, ok2_len}, {ok3_pcm, ok3_len}, {ok4_pcm, ok4_len} };
void playClip(const int16_t* data, uint32_t len);
static void playRandom(const Clip* c, size_t n, int& last) {
  int i = esp_random() % n;
  if (n > 1 && i == last) i = (i + 1) % n;
  last = i;
  playClip(c[i].d, c[i].n);
}
static int lastWake = -1, lastOk = -1;
static bool witFailed = false;   // true لو المشكلة في الاتصال بـ Wit.ai (مش في الكلام)

// تشغيل مقطع PCM مخزن (16-bit signed, 16kHz, مونو)
void playClip(const int16_t* data, uint32_t len) {
  int16_t buf[256 * 2];            // 256 فريم ستيريو
  size_t bw;
  for (uint32_t i = 0; i < len;) {
    uint32_t n = len - i < 256 ? len - i : 256;
    for (uint32_t k = 0; k < n; k++) {
      int16_t s = (int16_t)(((int32_t)data[i + k] * SPK_VOLUME) / 100);
      buf[2 * k] = s;
      buf[2 * k + 1] = s;
    }
    i2s_write(AMP_PORT, buf, n * 4, &bw, portMAX_DELAY);
    i += n;
  }
  // سكوت بحجم الـ DMA كله: لما يخلص يبقى آخر الصوت اتشغل بالكامل
  memset(buf, 0, sizeof(buf));
  for (int k = 0; k < 6; k++) i2s_write(AMP_PORT, buf, sizeof(buf), &bw, portMAX_DELAY);
}

// نغمة بسيطة (جيب) بدون ملف صوت: freq بالهرتز، ms بالمللي ثانية (مع fade عشان ما يطلعش طقطقة)
static void playTone(int freq, int ms) {
  int16_t buf[256 * 2];
  size_t bw;
  const uint32_t total = 16000UL * ms / 1000, fade = 160;
  float ph = 0, inc = 2.0f * PI * freq / 16000.0f;
  for (uint32_t i = 0; i < total;) {
    uint32_t n = total - i < 256 ? total - i : 256;
    for (uint32_t k = 0; k < n; k++) {
      uint32_t pos = i + k, left = total - pos;
      float env = pos < fade ? pos / (float)fade : (left < fade ? left / (float)fade : 1.0f);
      int16_t v = (int16_t)(sinf(ph) * 12000.0f * env * SPK_VOLUME / 100.0f);
      ph += inc;
      buf[2 * k] = v;
      buf[2 * k + 1] = v;
    }
    i2s_write(AMP_PORT, buf, n * 4, &bw, portMAX_DELAY);
    i += n;
  }
}
// نغمة الخطأ: نغمتين نازلين
static void playErrorTone() {
  int16_t buf[256 * 2];
  size_t bw;
  playTone(380, 130);
  memset(buf, 0, sizeof(buf));
  i2s_write(AMP_PORT, buf, 128 * 4, &bw, portMAX_DELAY);   // فاصل قصير
  playTone(240, 230);
  for (int k = 0; k < 6; k++) i2s_write(AMP_PORT, buf, sizeof(buf), &bw, portMAX_DELAY);   // سكوت: آخر الصوت يخلص
}

// 0 = المحاولة الأولى | 1.. = إعادة المحاولة (من غير نداء). في الإعادة بنكتفي بنغمة خطأ بدل ما نكرر نفس الجملة
static int retryNo = 0;
static void failReply(const int16_t* data, uint32_t len) {
  if (retryNo > 0) playErrorTone(); else playClip(data, len);
}

// تفريغ ما التقطه المايك أثناء تشغيل الصوت (عشان ميسمعش نفسه)
void flushMic() {
  int32_t tmp[256];
  size_t br = 1;
  while (br > 0) i2s_read(I2S_PORT, tmp, sizeof(tmp), &br, 0);
}

// --- واجهة الويب HTML ---
// الواجهة موجودة في index_html.h


// ==================== معالجة الميكروفون: فلتر + حساسية قابلة للضبط من الويب ====================
// INMP441: حساسيته -26 dBFS عند 94 dB SPL، يعني كلام عادي على متر تقريباً RMS ~ 30 فقط (مقياس 16-bit).
// الحد القديم (40) كان أعلى من كلام عادي، وده سبب إنك محتاج تزعق.
struct Biquad {
  float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
  float run(float x) { float y = b0 * x + z1; z1 = b1 * x - a1 * y + z2; z2 = b2 * x - a2 * y; return y; }
  void reset() { z1 = 0; z2 = 0; }
  void hpf(float fc, float fs) {                      // High-pass Butterworth: بيشيل الـ DC والرمبلة
    float w = 2.0f * PI * fc / fs, c = cosf(w), al = sinf(w) / (2.0f * 0.7071f), a0 = 1.0f + al;
    b0 = (1 + c) / 2 / a0; b1 = -(1 + c) / a0; b2 = b0; a1 = -2 * c / a0; a2 = (1 - al) / a0;
  }
};
static Biquad hpfWake, hpfRec;
static float micGain = 16.0f;       // تكبير الـ wake word (16 = زي MIC_SHIFT 12 القديم)
static float vadMinLevel = 8.0f;    // أقل مستوى يعتبر كلام أثناء التسجيل (مقياس 16-bit)
static float noiseIdle = 0;         // ضوضاء الغرفة وقت الراحة (بتتحدّث باستمرار)
static int lastPeak16 = 0, lastSpeechMs = 0;
static float lastRecGain = 0;
static bool heardOn = true;         // تسجيل الجمل المسموعة (وطباعتها في السيريال)

static void micLoadCfg() {
  File f = LittleFS.open("/mic_cfg.json", "r");
  if (!f) return;
  StaticJsonDocument<128> d;
  if (!deserializeJson(d, f)) {
    float g = d["g"] | micGain, v = d["v"] | vadMinLevel;
    if (g >= 2 && g <= 128) micGain = g;
    if (v >= 2 && v <= 60) vadMinLevel = v;
    heardOn = (d["h"] | 1) != 0;
  }
  f.close();
}
static void micSaveCfg() {
  File f = LittleFS.open("/mic_cfg.json", "w");
  if (!f) return;
  f.printf("{\"g\":%.1f,\"v\":%.1f,\"h\":%d}", micGain, vadMinLevel, heardOn ? 1 : 0);
  f.close();
}

// --- دالة تهيئة I2S ---
void i2s_init() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 512,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };
  
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  hpfWake.hpf(120.0f, 16000.0f);
  hpfRec.hpf(120.0f, 16000.0f);
}

// --- دالة تهيئة موديل WakeNet (ESP-SR) ---
void initWakeNet() {
  models = esp_srmodel_init("model");
  if (!models) { Serial.println("Model partition not found!"); return; }

  char *model_name = esp_srmodel_filter(models, ESP_WN_PREFIX, WAKE_WORD);
  if (!model_name) {
    Serial.println("Wake word model not found! Available models:");
    for (int i = 0; i < models->num; i++) Serial.println(models->model_name[i]);
    model_name = esp_srmodel_filter(models, ESP_WN_PREFIX, NULL);
  }
  if (!model_name) { Serial.println("No WakeNet model found!"); return; }
  Serial.printf("Using model: %s\n", model_name);

  wakenet = esp_wn_handle_from_name(model_name);
  model_data = wakenet->create(model_name, DET_MODE_95);  // مايك واحد - 95 أكثر حساسية
  if (!model_data) { Serial.println("WakeNet create failed!"); return; }

  chunk_size = wakenet->get_samp_chunksize(model_data);
  raw_chunk = (int32_t*)malloc(chunk_size * sizeof(int32_t));
  pcm16_chunk = (int16_t*)malloc(chunk_size * sizeof(int16_t));
  Serial.printf("WakeNet loaded. Chunk size: %d\n", chunk_size);
}

// --- دالة فحص كلمة التنبيه ---
bool checkWakeWord() {
  if (!wakenet || !model_data || !raw_chunk || !pcm16_chunk) return false;

  size_t bytes_read = 0;
  i2s_read(I2S_PORT, raw_chunk, chunk_size * sizeof(int32_t), &bytes_read, 100 / portTICK_PERIOD_MS);
  if (bytes_read < chunk_size * sizeof(int32_t)) return false;

  const float g = micGain / 256.0f;
  float acc = 0;
  for (int i = 0; i < chunk_size; i++) {
    float v = hpfWake.run((float)(raw_chunk[i] >> 8));      // 24-bit بعد إزالة DC والرمبلة
    acc += fabsf(v);
    float s = v * g;
    if (s > 32767) s = 32767;
    if (s < -32768) s = -32768;
    pcm16_chunk[i] = (int16_t)s;
  }
  float lvl = acc / chunk_size / 256.0f;                    // مستوى الصوت بمقياس 16-bit (من غير تكبير)
  if (noiseIdle <= 0) noiseIdle = lvl;
  else if (lvl < noiseIdle * 2.0f) noiseIdle += (lvl - noiseIdle) * 0.02f;     // تتبع سريع للهدوء
  else noiseIdle += (lvl - noiseIdle) * 0.0005f;                               // الكلام العالي ما يرفعش الضوضاء بسرعة

  if (wakenet->detect(model_data, pcm16_chunk) > 0) {
    Serial.println("\n[Wake Word Detected]");
    return true;
  }
  return false;
}
// --- دالة تسجيل الصوت وتحويله إلى نص عبر Wit.ai ---
String recordAndTranscribe() {
  setRGB(255, 0, 0);   // لون أحمر: ابدأ يتكلم الآن (جاري التسجيل)

  i2s_zero_dma_buffer(I2S_PORT);

  Serial.println("Start Recording Speech...");

  const int sample_rate = 16000;
  const int total_samples = sample_rate * RECORD_TIME_SECONDS; 

  int32_t* raw_buffer = NULL;
  int16_t* pcm16_buffer = NULL;

  if (psramFound()) {
    raw_buffer = (int32_t*)ps_malloc(total_samples * sizeof(int32_t));
    pcm16_buffer = (int16_t*)ps_malloc(total_samples * sizeof(int16_t));
  } else {
    raw_buffer = (int32_t*)malloc(total_samples * sizeof(int32_t));
    pcm16_buffer = (int16_t*)malloc(total_samples * sizeof(int16_t));
  }

  if(!raw_buffer || !pcm16_buffer) { 
    Serial.println("Memory Allocation Error"); 
    if(raw_buffer) free(raw_buffer);
    if(pcm16_buffer) free(pcm16_buffer);
    setRGB(0, 0, 0);
    return ""; 
  }

  // --- تسجيل على دفعات 20ms + كشف نهاية الكلام ---
  const int total_chunks   = total_samples / VAD_CHUNK;
  const int chunkMs        = VAD_CHUNK * 1000 / sample_rate;
  const int silenceChunks  = VAD_SILENCE_MS / chunkMs;
  const int noSpeechChunks = VAD_NO_SPEECH_MS / chunkMs;
  const int minSpeech      = VAD_MIN_SPEECH_MS / chunkMs;
  float noise = noiseIdle > 0 ? noiseIdle : 1e9f;            // ضوضاء الغرفة من وقت الراحة (مش من الكلام نفسه)
  hpfRec.reset();
  int collected = 0, speechChunks = 0, firstSpeech = -1, lastSpeech = -1, silenceRun = 0, c = 0;
  float thrShown = 0;

  for (c = 0; c < total_chunks; c++) {
    int32_t* p = raw_buffer + (size_t)c * VAD_CHUNK;
    size_t br = 0;
    i2s_read(I2S_PORT, p, VAD_CHUNK * sizeof(int32_t), &br, 1000 / portTICK_PERIOD_MS);
    int got = br / sizeof(int32_t);
    collected += got;
    if (got < VAD_CHUNK) break;
    for (int i = 0; i < got; i++) {                            // فلتر high-pass على التسجيل نفسه
      float y = hpfRec.run((float)(p[i] >> 8));
      if (y > 8388607.0f) y = 8388607.0f;
      if (y < -8388608.0f) y = -8388608.0f;
      p[i] = ((int32_t)y) << 8;
    }

    int64_t sm = 0;
    for (int i = 0; i < got; i++) sm += (p[i] >> 8);
    int32_t dcc = (int32_t)(sm / got);
    int64_t ab = 0;
    for (int i = 0; i < got; i++) { int32_t v = (p[i] >> 8) - dcc; ab += v < 0 ? -v : v; }
    float e = (float)ab / got / 256.0f;                       // مستوى الطاقة بمقياس 16-bit

    if (e < noise) noise = e;                                  // أهدى لحظة = ضوضاء الغرفة
    float nz = noise > VAD_MAX_NOISE ? VAD_MAX_NOISE : noise;
    float thr = nz * VAD_NOISE_MULT;
    if (thr < vadMinLevel) thr = vadMinLevel;
    thrShown = thr;

    if (e > thr) {
      if (firstSpeech < 0) firstSpeech = c;
      speechChunks++;
      lastSpeech = c;
      silenceRun = 0;
    } else {
      silenceRun++;
      if (firstSpeech >= 0 && silenceRun >= silenceChunks) {
        if (speechChunks >= minSpeech) break;                  // خلص كلامه
        firstSpeech = -1; speechChunks = 0; lastSpeech = -1;   // كان طرقعة بس، كمّل استنى
      }
    }
    if (firstSpeech < 0 && c >= noSpeechChunks) break;         // محدش اتكلم
  }
  Serial.printf("VAD: noise=%.1f thr=%.1f speechChunks=%d recorded=%dms\n", noise, thrShown, speechChunks, collected * 1000 / sample_rate);

  if (speechChunks < minSpeech) {                              // مفيش كلام: متبعتش لـ Wit.ai
    free(raw_buffer);
    free(pcm16_buffer);
    setRGB(0, 0, 0);
    return "";
  }

  // نقص الصمت الزيادة من الأول والآخر (نسيب شوية padding)
  const int padSamples = VAD_PAD_MS * sample_rate / 1000;
  int startIdx = firstSpeech * VAD_CHUNK - padSamples;
  if (startIdx < 0) startIdx = 0;
  int endIdx = (lastSpeech + 1) * VAD_CHUNK + padSamples;
  if (endIdx > collected) endIdx = collected;
  int actual_samples = endIdx - startIdx;
  int32_t* rb = raw_buffer + startIdx;

  // --- تطبيع تلقائي: إزالة DC + ضبط الـ gain حسب أعلى قمة في التسجيل ---
  // INMP441 بيدي 24-bit في أعلى 32-bit؛ >>8 بيدي قيمة 24-bit، و/256 = مقياس 16-bit
  int64_t sum = 0;
  for (int i = 0; i < actual_samples; i++) sum += (rb[i] >> 8);
  int32_t dc = actual_samples ? (int32_t)(sum / actual_samples) : 0;

  int32_t peak24 = 1;
  for (int i = 0; i < actual_samples; i++) {
    int32_t v = (rb[i] >> 8) - dc;
    if (v < 0) v = -v;
    if (v > peak24) peak24 = v;
  }
  float scale = (float)REC_TARGET_PEAK / (float)peak24;   // من 24-bit مباشرةً لـ 16-bit
  float maxScale = REC_MAX_GAIN / 256.0f;
  if (scale > maxScale) scale = maxScale;
  Serial.printf("Mic peak (16-bit scale): %d | gain: %.1fx\n", (int)(peak24 >> 8), scale * 256.0f);
  lastPeak16 = (int)(peak24 >> 8); lastRecGain = scale * 256.0f; lastSpeechMs = speechChunks * chunkMs;

  for (int i = 0; i < actual_samples; i++) {
    int32_t sample = (int32_t)(((rb[i] >> 8) - dc) * scale);
    if (sample > 32767) sample = 32767;
    if (sample < -32768) sample = -32768;
    pcm16_buffer[i] = (int16_t)sample;
  }

  free(raw_buffer);

  setRGB(128, 0, 255); // لون بنفسجي: جاري المعالجة الإرسال لـ Wit.ai
  Serial.println("Recording Done. Processing with Wit.ai...");

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(15);

  HTTPClient http;
  http.setTimeout(15000);

  String resultText = "";
  witFailed = false;

  if (http.begin(client, "https://api.wit.ai/speech")) {
    http.addHeader("Authorization", "Bearer " + WIT_AI_TOKEN);
    http.addHeader("Content-Type", "audio/raw;encoding=signed-integer;bits=16;rate=16000;endian=little");

    int httpResponseCode = http.POST((uint8_t*)pcm16_buffer, actual_samples * sizeof(int16_t));
    String response = http.getString();
    http.end();

    if (httpResponseCode == 200) {
      int lastTextPos = response.lastIndexOf("\"text\"");
      if (lastTextPos != -1) {
        int startQuote = response.indexOf("\"", lastTextPos + 7);
        int endQuote = response.indexOf("\"", startQuote + 1);
        if (startQuote != -1 && endQuote != -1) {
          resultText = response.substring(startQuote + 1, endQuote);
          resultText.toLowerCase();
          resultText.trim();
        }
      }
      if (heardOn) { Serial.print("Transcribed Text: "); Serial.println(resultText); }
    } else {
      Serial.printf("Wit.ai HTTP Error Code: %d\n", httpResponseCode);
      witFailed = true;
    }
  } else {
    Serial.println("Unable to connect to Wit.ai server");
    witFailed = true;
  }

  free(pcm16_buffer);
  return resultText;
}

// --- تطبيع النص العربي: يشيل التشكيل ويوحّد الحروف عشان المطابقة تبقى مرنة ---
static String normalizeAr(const String& in) {
  String out;
  out.reserve(in.length());
  const uint8_t* s = (const uint8_t*)in.c_str();
  size_t n = in.length();
  for (size_t i = 0; i < n;) {
    uint8_t b = s[i];
    int len = b < 0x80 ? 1 : (b >> 5) == 6 ? 2 : (b >> 4) == 14 ? 3 : (b >> 3) == 30 ? 4 : 1;
    if (i + len > n) len = 1;
    uint32_t cp = len == 1 ? b : (len == 2 ? (b & 0x1F) : (len == 3 ? (b & 0x0F) : (b & 0x07)));
    for (int k = 1; k < len; k++) cp = (cp << 6) | (s[i + k] & 0x3F);
    i += len;

    if ((cp >= 0x064B && cp <= 0x065F) || cp == 0x0670 || cp == 0x0640) continue;   // تشكيل + تطويل
    if (cp == 0x0623 || cp == 0x0625 || cp == 0x0622 || cp == 0x0671) cp = 0x0627;  // أ إ آ ٱ -> ا
    else if (cp == 0x0629) cp = 0x0647;                                             // ة -> ه
    else if (cp == 0x0649) cp = 0x064A;                                             // ى -> ي
    else if (cp == 0x0624) cp = 0x0648;                                             // ؤ -> و
    else if (cp == 0x0626) cp = 0x064A;                                             // ئ -> ي
    else if (cp >= 0x0660 && cp <= 0x0669) cp = '0' + (cp - 0x0660);                // أرقام هندية
    else if (cp >= 0x06F0 && cp <= 0x06F9) cp = '0' + (cp - 0x06F0);
    else if (cp >= 'A' && cp <= 'Z') cp = cp + 32;
    else if (cp == 0x060C || cp == 0x061B || cp == 0x061F || (cp < 0x80 && !isalnum((int)cp))) cp = ' ';

    if (cp == ' ' && (out.length() == 0 || out[out.length() - 1] == ' ')) continue;  // مسافة واحدة بس
    if (cp < 0x80) out += (char)cp;
    else if (cp < 0x800) { out += (char)(0xC0 | (cp >> 6)); out += (char)(0x80 | (cp & 0x3F)); }
    else { out += (char)(0xE0 | (cp >> 12)); out += (char)(0x80 | ((cp >> 6) & 0x3F)); out += (char)(0x80 | (cp & 0x3F)); }
  }
  out.trim();

  // شيل "ال" التعريف من أول كل كلمة (4 حروف فأكتر) - النور = نور
  String res;
  int pos = 0;
  while (pos <= (int)out.length()) {
    int sp = out.indexOf(' ', pos);
    if (sp < 0) sp = out.length();
    String w = out.substring(pos, sp);
    if (w.length() >= 8 && w.startsWith("\xD8\xA7\xD9\x84")) w = w.substring(4);
    if (res.length()) res += ' ';
    res += w;
    pos = sp + 1;
  }
  return res;
}

// --- أدوات ملف الأوامر + تنفيذ الأوامر ---
// شكل commands.json الجديد:
// [ {"id":1,"name":"نور السقف","cat":"إضاءة",
//    "phrases":["شغل نور السقف","شغلي نور السقف"],
//    "steps":[{"type":"http","val":"http://..."},{"type":"wol","val":"AA:BB:.."}]} ]
#define CMD_DOC_SIZE 16384

#include "ir_module.h"   // IR: تعلّم + إرسال (ملف مرفق في نفس الفولدر)
#include "strip_module.h" // المشترك MTTL-W01: الـ ESP32 هو الكنترولر (ملف مرفق في نفس الفولدر)

// ==================== الريلاي ====================
static const int relayPin[2] = { RELAY1_PIN, RELAY2_PIN };
static bool relayOn[2] = { false, false };

static void relayWrite(int i, bool on) {
  relayOn[i] = on;
  digitalWrite(relayPin[i], (on != (bool)RELAY_ACTIVE_LOW) ? HIGH : LOW);
}

static void relayInit() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(relayPin[i], RELAY_ACTIVE_LOW ? HIGH : LOW);   // مطفي قبل ما الطرف يبقى خرج (من غير نبضة)
    pinMode(relayPin[i], OUTPUT_OPEN_DRAIN);   // push-pull عادي: الـ VCC على 3.3V فمفيش داعي لـ open-drain
    relayWrite(i, false);
  }
}

// n = 1 أو 2 | s = "on" / "off" / "toggle"
static bool relaySet(int n, String s) {
  if (n < 1 || n > 2) return false;
  s.toLowerCase(); s.trim();
  int i = n - 1;
  if (s == "on" || s == "1") relayWrite(i, true);
  else if (s == "off" || s == "0") relayWrite(i, false);
  else if (s == "toggle") relayWrite(i, !relayOn[i]);
  else return false;
  Serial.printf("Relay %d -> %s\n", n, relayOn[i] ? "ON" : "OFF");
  return true;
}

// صيغة الخطوة: "1:on" / "2:off" / "1:toggle"
static bool relayAction(const String& val) {
  int c = val.indexOf(':');
  if (c < 0) return false;
  return relaySet(val.substring(0, c).toInt(), val.substring(c + 1));
}

String cmdValue(JsonVariant v) {
  String s = v["url"] | "";
  if (s.length() == 0) s = v["val"] | "";
  return s;
}

// تحويل الصيغة القديمة {trigger:{type,url}} للصيغة الجديدة (مرة واحدة)
void migrateOld(DynamicJsonDocument& doc) {
  DynamicJsonDocument tmp(CMD_DOC_SIZE);
  JsonArray arr = tmp.to<JsonArray>();
  int id = 1;
  for (JsonPair kv : doc.as<JsonObject>()) {
    String key = kv.key().c_str();
    JsonObject c = arr.createNestedObject();
    c["id"] = id++;
    c["name"] = key;
    c["cat"] = "عام";
    c.createNestedArray("phrases").add(key);
    JsonObject s = c.createNestedArray("steps").createNestedObject();
    s["type"] = kv.value()["type"].as<String>();
    s["val"] = cmdValue(kv.value());
  }
  doc.clear();
  doc.set(tmp);
}

bool loadCmds(DynamicJsonDocument& doc) {
  File f = LittleFS.open("/commands.json", "r");
  if (!f) return false;
  DeserializationError e = deserializeJson(doc, f);
  f.close();
  if (e) return false;
  if (doc.is<JsonObject>()) migrateOld(doc);
  return doc.is<JsonArray>();
}

bool saveCmds(DynamicJsonDocument& doc) {
  File f = LittleFS.open("/commands.json", "w");
  if (!f) return false;
  serializeJson(doc, f);
  f.close();
  return true;
}

bool runAction(const String& type, const String& val) {
  if (type == "http") {
    HTTPClient http_req;
    http_req.setTimeout(3000);
    http_req.begin(val);
    int res = http_req.GET();
    Serial.printf("Executed HTTP: %s | Result: %d\n", val.c_str(), res);
    http_req.end();
    return res > 0 && res < 400;
  }
  if (type == "ir") return irSendById(val.toInt());
  if (type == "relay") return relayAction(val);
  if (type == "strip") return stripAction(val);
  if (type == "delay") return true;                  // بيتعالج في execSteps (من غير ما يعطل الجهاز)
  if (type == "wol") {
    WOL.sendMagicPacket(val);
    Serial.println("Sent WOL to: " + val);
    return true;
  }
  return false;
}

// ---- تنفيذ خطوات الأمر بالترتيب (مع دعم خطوة "delay" من غير ما نوقف الجهاز) ----
// خطوة delay: val = عدد الثواني (مثال 5 أو 2.5). الباقي من الخطوات بيتنفذ بعد المدة من جوه loop() عن طريق jobPoll()
#define JOB_MAX 4
struct CmdJob { bool used = false; uint32_t at = 0; uint8_t idx = 0; String steps; };
static CmdJob jobs[JOB_MAX];

static uint32_t delayMs(const String& v) {
  float sec = v.toFloat();
  if (sec < 0) sec = 0;
  if (sec > 3600) sec = 3600;
  return (uint32_t)(sec * 1000.0f);
}

static bool execSteps(const String& json, uint8_t from) {
  DynamicJsonDocument d(4096);
  if (deserializeJson(d, json)) return false;
  JsonArray steps = d.as<JsonArray>();
  bool all = true;
  for (uint8_t i = from; i < steps.size(); i++) {
    String t = steps[i]["type"].as<String>();
    String v = steps[i]["val"].as<String>();
    if (t == "delay") {
      if (i + 1 >= steps.size()) break;              // delay في الآخر ملوش لازمة
      int slot = -1;
      for (int j = 0; j < JOB_MAX; j++) if (!jobs[j].used) { slot = j; break; }
      if (slot < 0) { Serial.println("Too many delayed commands"); return false; }
      jobs[slot].used = true;
      jobs[slot].at = millis() + delayMs(v);
      jobs[slot].idx = i + 1;
      jobs[slot].steps = json;
      Serial.printf("Delay %.1fs then continue from step %u\n", delayMs(v) / 1000.0f, (unsigned)(i + 2));
      return all;
    }
    Serial.printf("Step %u/%u [%s] %s\n", (unsigned)(i + 1), (unsigned)steps.size(), t.c_str(), v.substring(0, 60).c_str());
    if (!runAction(t, v)) all = false;
    delay(30);
  }
  return all;
}

// بيتنادى من loop(): بيكمّل الأوامر اللي استنت الـ delay
static void jobPoll() {
  for (int j = 0; j < JOB_MAX; j++) {
    if (!jobs[j].used || (int32_t)(millis() - jobs[j].at) < 0) continue;
    String st = jobs[j].steps;
    uint8_t idx = jobs[j].idx;
    jobs[j].used = false;
    jobs[j].steps = "";
    Serial.printf("Delay finished -> resuming at step %u\n", (unsigned)(idx + 1));
    execSteps(st, idx);
  }
}

// تنفيذ كل خطوات الأمر بالترتيب
bool runCommand(JsonVariant c) {
  JsonArray steps = c["steps"].as<JsonArray>();
  if (steps.isNull() || steps.size() == 0) return false;
  String json;
  serializeJson(steps, json);
  return execSteps(json, 0);
}

// ---- سجل آخر ما سمعه الجهاز (بيظهر في الويب عشان تضيفه كجملة لأمر) ----
#define HEARD_N 10
static String heardTxt[HEARD_N];
static bool heardOk[HEARD_N];
static uint8_t heardCnt = 0, heardHead = 0;
static void heardClear() {
  for (int i = 0; i < HEARD_N; i++) heardTxt[i] = "";
  heardCnt = 0; heardHead = 0;
}
static void heardAdd(const String& t, bool ok) {
  if (!heardOn || t.length() == 0) return;
  heardTxt[heardHead] = t; heardOk[heardHead] = ok;
  heardHead = (heardHead + 1) % HEARD_N;
  if (heardCnt < HEARD_N) heardCnt++;
}

// ---- مطابقة تقريبية (بتسامح مع أخطاء التفريغ) على مستوى الحروف مش البايتات ----
static int toCps(const String& s, uint16_t* out, int maxN) {
  const uint8_t* b = (const uint8_t*)s.c_str();
  int n = 0;
  for (size_t i = 0; b[i] && n < maxN;) {
    uint8_t c = b[i];
    uint32_t cp = '?'; int len = 1;
    if (c < 0x80) cp = c;
    else if ((c >> 5) == 6 && b[i + 1]) { cp = ((c & 0x1F) << 6) | (b[i + 1] & 0x3F); len = 2; }
    else if ((c >> 4) == 14 && b[i + 1] && b[i + 2]) { cp = ((c & 0x0F) << 12) | ((b[i + 1] & 0x3F) << 6) | (b[i + 2] & 0x3F); len = 3; }
    else if ((c >> 3) == 30) len = 4;
    out[n++] = (uint16_t)cp;
    i += len;
  }
  return n;
}
static int editDist(const uint16_t* a, int na, const uint16_t* b, int nb) {
  if (na > 32 || nb > 32) return 99;
  int prev[33], cur[33];
  for (int j = 0; j <= nb; j++) prev[j] = j;
  for (int i = 1; i <= na; i++) {
    cur[0] = i;
    for (int j = 1; j <= nb; j++) {
      int cost = a[i - 1] == b[j - 1] ? 0 : 1;
      int v = prev[j] + 1;
      if (cur[j - 1] + 1 < v) v = cur[j - 1] + 1;
      if (prev[j - 1] + cost < v) v = prev[j - 1] + cost;
      cur[j] = v;
    }
    memcpy(prev, cur, sizeof(int) * (nb + 1));
  }
  return prev[nb];
}
// كل كلمة في الجملة المحفوظة لازم يبقى ليها كلمة قريبة في الكلام (كلمات قصيرة ≤3 حروف: لازم تطابق بالظبط)
static bool wordsFuzzyMatch(const String& spoken, const String& phrase) {
  uint16_t sw[12][33]; int sl[12]; int ns = 0;
  int pos = 0;
  while (ns < 12 && pos <= (int)spoken.length()) {
    int sp = spoken.indexOf(' ', pos); if (sp < 0) sp = spoken.length();
    String w = spoken.substring(pos, sp);
    if (w.length()) { sl[ns] = toCps(w, sw[ns], 32); ns++; }
    pos = sp + 1;
  }
  if (ns == 0) return false;
  bool any = false;
  pos = 0;
  while (pos <= (int)phrase.length()) {
    int sp = phrase.indexOf(' ', pos); if (sp < 0) sp = phrase.length();
    String w = phrase.substring(pos, sp);
    pos = sp + 1;
    if (!w.length()) continue;
    uint16_t pw[33]; int pl = toCps(w, pw, 32);
    int tol = pl <= 3 ? 0 : (pl <= 6 ? 1 : 2);
    bool hit = false;
    for (int i = 0; i < ns && !hit; i++) if (editDist(sw[i], sl[i], pw, pl) <= tol) hit = true;
    if (!hit) return false;
    any = true;
  }
  return any;
}

// --- دالة البحث عن الأمر وتنفيذه ---
// 1) أطول جملة محفوظة موجودة جوه الكلام  2) لو مفيش: أي جملة كل كلماتها موجودة بأي ترتيب
bool executeCommand(String spokenText) {   // true = خلصنا | false = ماسمعتش/مافهمتش (نعيد المحاولة)
  if (witFailed) {                                     // مشكلة اتصال مش مشكلة سمع
    playClip(nonet_pcm, nonet_len);
    flashRGB(255, 0, 0, 3);
    return true;                                       // مفيش فايدة من إعادة المحاولة
  }
  if (spokenText.length() == 0) {
    failReply(mesmaatak_pcm, mesmaatak_len);           // "لم أسمعك جيداً" (في الإعادة: نغمة خطأ)
    flashRGB(255, 100, 0, 2);
    return false;
  }

  String spoken = normalizeAr(spokenText);
  if (heardOn) Serial.println("Normalized: " + spoken);

  DynamicJsonDocument doc(CMD_DOC_SIZE);
  if (!loadCmds(doc)) return true;

  JsonVariant best;
  size_t bestLen = 0;
  for (JsonVariant c : doc.as<JsonArray>()) {
    for (JsonVariant p : c["phrases"].as<JsonArray>()) {
      String ph = normalizeAr(p.as<String>());
      if (ph.length() > bestLen && spoken.indexOf(ph) != -1) {
        best = c;
        bestLen = ph.length();
      }
    }
  }

  if (bestLen == 0) {                                  // المرور التاني: الكلمات بأي ترتيب
    for (JsonVariant c : doc.as<JsonArray>()) {
      for (JsonVariant p : c["phrases"].as<JsonArray>()) {
        String ph = normalizeAr(p.as<String>());
        if (ph.length() <= bestLen) continue;
        bool all = ph.length() > 0;
        int pos = 0;
        while (all && pos <= (int)ph.length()) {
          int sp = ph.indexOf(' ', pos);
          if (sp < 0) sp = ph.length();
          String w = ph.substring(pos, sp);
          if (w.length() && spoken.indexOf(w) == -1) all = false;
          pos = sp + 1;
        }
        if (all) { best = c; bestLen = ph.length(); }
      }
    }
  }

  if (bestLen == 0) {                                  // المرور التالت: مطابقة تقريبية (أخطاء حروف بسيطة في التفريغ)
    for (JsonVariant c : doc.as<JsonArray>()) {
      for (JsonVariant p : c["phrases"].as<JsonArray>()) {
        String ph = normalizeAr(p.as<String>());
        if (ph.length() <= bestLen) continue;
        if (wordsFuzzyMatch(spoken, ph)) { best = c; bestLen = ph.length(); Serial.println("Fuzzy match: " + ph); }
      }
    }
  }
  heardAdd(spoken, bestLen > 0);

  if (bestLen > 0) {
    Serial.println("Matched command: " + best["name"].as<String>());
    bool ok = runCommand(best);
    if (ok) {
      playRandom(OK_CLIPS, sizeof(OK_CLIPS) / sizeof(OK_CLIPS[0]), lastOk);   // تمام / حاضر ...
      flashRGB(0, 255, 0, 2);
    } else {
      playClip(fail_pcm, fail_len);                    // حدث خطأ
      flashRGB(255, 0, 0, 3);
    }
  } else {
    Serial.println("Command not recognized in saved JSON list.");
    failReply(nomatch_pcm, nomatch_len);               // "لم أفهم الأمر" (في الإعادة: نغمة خطأ)
    flashRGB(255, 200, 0, 2);
    return false;
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  
  if (!LittleFS.begin(true)) {
    Serial.println("Formatting LittleFS...");
    LittleFS.format();
    LittleFS.begin(true);
  }

  if (!LittleFS.exists("/commands.json")) {
    File f = LittleFS.open("/commands.json", "w");
    f.print("[]");
    f.close();
  }
  { // تحويل الملف القديم تلقائياً لو لسه بالصيغة القديمة
    DynamicJsonDocument d(CMD_DOC_SIZE);
    if (loadCmds(d)) saveCmds(d);
  }

  relayInit();
  irInit();
  micLoadCfg();

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);          // يمنع وضع توفير الطاقة: ده سبب شائع لـ "unreachable" لحد ما تعمل ريست
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  Serial.println("\nWiFi Connected! IP Address: " + WiFi.localIP().toString());

  sntp_set_time_sync_notification_cb([](struct timeval*) { ntpFlag = true; });
  configTzTime(TZ_INFO, "pool.ntp.org", "time.google.com", "time.cloudflare.com");   // مش بيعطل: بيضبط في الخلفية

  WOL.setBroadcastAddress("192.168.1.255");

  // --- OTA من Arduino IDE (Tools > Port > Network ports) ---
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.onStart([]() { setRGB(255, 160, 0); Serial.println("OTA start"); });
  ArduinoOTA.onEnd([]() { setRGB(0, 255, 0); Serial.println("\nOTA done"); });
  ArduinoOTA.onProgress([](unsigned int p, unsigned int t) { Serial.printf("OTA %u%%\r", p * 100 / t); });
  ArduinoOTA.onError([](ota_error_t e) { setRGB(255, 0, 0); Serial.printf("OTA error %u\n", e); });
  ArduinoOTA.begin();
  audioInit();
  i2s_init();
  initWakeNet();

  // --- مسارات السيرفر ---
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });

  server.on("/api/commands", HTTP_GET, []() {
    File f = LittleFS.open("/commands.json", "r");
    server.streamFile(f, "application/json");
    f.close();
  });

  // إضافة/تعديل أمر: JSON {id(0=جديد), name, cat, phrases[], steps[{type,val}]}
  server.on("/api/save", HTTP_POST, []() {
    DynamicJsonDocument in(4096);
    if (deserializeJson(in, server.arg("plain"))) { server.send(400, "text/plain", "BAD JSON"); return; }
    String name = in["name"] | "";
    name.trim();
    String cat = in["cat"] | "عام";
    cat.trim();
    if (cat.length() == 0) cat = "عام";
    JsonArray ph = in["phrases"].as<JsonArray>();
    JsonArray st = in["steps"].as<JsonArray>();
    if (name.length() == 0 || ph.isNull() || st.isNull()) { server.send(400, "text/plain", "BAD REQUEST"); return; }

    DynamicJsonDocument doc(CMD_DOC_SIZE);
    if (!loadCmds(doc)) { doc.clear(); doc.to<JsonArray>(); }
    JsonArray arr = doc.as<JsonArray>();

    int id = in["id"] | 0;
    JsonObject c;
    if (id > 0) { for (JsonObject o : arr) if ((int)o["id"] == id) { c = o; break; } }
    if (c.isNull()) {
      int mx = 0;
      for (JsonObject o : arr) mx = max(mx, (int)o["id"]);
      id = mx + 1;
      c = arr.createNestedObject();
    } else {
      c.clear();
    }
    c["id"] = id;
    c["name"] = name;
    c["cat"] = cat;
    JsonArray np = c.createNestedArray("phrases");
    for (JsonVariant p : ph) {
      String s = p.as<String>();
      s.toLowerCase(); s.trim();
      if (s.length()) np.add(s);
    }
    JsonArray ns = c.createNestedArray("steps");
    for (JsonVariant s : st) {
      String t = s["type"] | "http";
      String v = s["val"] | "";
      v.trim();
      if (v.length() == 0 || (t != "http" && t != "wol" && t != "ir" && t != "relay" && t != "strip" && t != "delay")) continue;
      if (t == "strip" && !stripValOk(v)) continue;
      if (t == "delay") { float sec = v.toFloat(); if (sec <= 0) continue; if (sec > 3600) sec = 3600; v = String(sec, 1); }
      JsonObject o = ns.createNestedObject();
      o["type"] = t;
      o["val"] = v;
    }
    if (np.size() == 0 || ns.size() == 0) { server.send(400, "text/plain", "EMPTY"); return; }
    if (doc.overflowed()) { server.send(507, "text/plain", "FULL"); return; }
    if (!saveCmds(doc)) { server.send(500, "text/plain", "ERROR"); return; }
    Serial.println("Command saved: " + name);
    server.send(200, "text/plain", String(id));
  });

  server.on("/api/delete", HTTP_POST, []() {
    int id = server.arg("id").toInt();
    DynamicJsonDocument doc(CMD_DOC_SIZE);
    if (!loadCmds(doc)) { server.send(404, "text/plain", "NOT FOUND"); return; }
    JsonArray arr = doc.as<JsonArray>();
    bool found = false;
    for (size_t i = 0; i < arr.size(); i++) {
      if ((int)arr[i]["id"] == id) { arr.remove(i); found = true; break; }
    }
    if (!found) { server.send(404, "text/plain", "NOT FOUND"); return; }
    if (!saveCmds(doc)) { server.send(500, "text/plain", "ERROR"); return; }
    Serial.println("Command deleted: " + String(id));
    server.send(200, "text/plain", "OK");
  });

  // يشتغل GET و POST عشان HTTP Shortcuts: /api/run?id=N
  server.on("/api/run", HTTP_ANY, []() {
    int id = server.arg("id").toInt();
    DynamicJsonDocument doc(CMD_DOC_SIZE);
    if (!loadCmds(doc)) { server.send(404, "text/plain", "NOT FOUND"); return; }
    for (JsonVariant c : doc.as<JsonArray>()) {
      if ((int)c["id"] == id) {
        Serial.println("Manual run: " + c["name"].as<String>());
        bool ok = runCommand(c);
        server.send(ok ? 200 : 502, "text/plain", ok ? "OK" : "FAILED");
        return;
      }
    }
    server.send(404, "text/plain", "NOT FOUND");
  });

  server.on("/api/clear", HTTP_POST, []() {
    File file = LittleFS.open("/commands.json", "w");
    if (file) {
      file.print("[]");
      file.close();
      Serial.println("All commands cleared");
      server.send(200, "text/plain", "CLEARED");
    } else {
      server.send(500, "text/plain", "ERROR");
    }
  });

  // محاكاة جملة منطوقة (بدون مايك)
  server.on("/api/say", HTTP_POST, []() {
    String text = server.arg("text");
    text.toLowerCase();
    text.trim();
    server.send(200, "text/plain", "OK");
    if (heardOn) Serial.println("Simulated text: " + text);
    executeCommand(text);
  });

  // Serial Monitor: بيرجّع اللوج من بعد الموضع since، والموضع الجديد في هيدر X-Next
  server.on("/api/log", HTTP_GET, []() {
    uint32_t since = (uint32_t)server.arg("since").toInt();
    uint32_t oldest = logTotal > LOG_SIZE ? logTotal - LOG_SIZE : 0;
    if (oldest < logFloor) oldest = logFloor;
    if (since < oldest || since > logTotal) since = oldest;
    String out;
    out.reserve(logTotal - since + 1);
    for (uint32_t i = since; i < logTotal; i++) out += logBuf[i % LOG_SIZE];
    server.sendHeader("X-Next", String(logTotal));
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/plain; charset=utf-8", out);
  });

  server.on("/api/log/clear", HTTP_POST, []() {
    memset(logBuf, 0, LOG_SIZE);
    logFloor = logTotal;
    server.send(200, "text/plain", "OK");
  });

  server.on("/api/heard/clear", HTTP_POST, []() {
    heardClear();
    server.send(200, "text/plain", "OK");
  });
  server.on("/api/heard/cfg", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", String("{\"on\":") + (heardOn ? 1 : 0) + "}");
  });
  server.on("/api/heard/cfg", HTTP_POST, []() {
    heardOn = server.arg("on") != "0";
    if (!heardOn) heardClear();
    micSaveCfg();
    server.send(200, "text/plain", "OK");
  });

  server.on("/api/info", HTTP_GET, []() {
    String j = "{\"ip\":\"" + WiFi.localIP().toString() + "\",\"rssi\":" + String(WiFi.RSSI()) +
               ",\"heap\":" + String(ESP.getFreeHeap()) + ",\"psram\":" + String(ESP.getFreePsram()) +
               ",\"up\":" + String((uint32_t)(millis() / 1000)) + ",\"wake\":\"" WAKE_WORD "\",\"sync\":" + String(timeSynced() ? 1 : 0) + ",\"now\":\"" + nowStr() + "\"}";
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", j);
  });

  // --- الوقت: GET بيرجّع وقت الجهاز | POST ts=<epoch ثواني UTC> بيضبطه (لو NTP مش شغال) ---
  server.on("/api/time", HTTP_GET, []() {
    struct timeval tv; gettimeofday(&tv, nullptr);
    String j = String("{\"sync\":") + (timeSynced() ? 1 : 0) + ",\"ts\":" + String((uint32_t)tv.tv_sec) + ",\"now\":\"" + nowStr() + "\",\"tz\":\"" TZ_INFO "\"}";
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", j);
  });
  server.on("/api/time", HTTP_POST, []() {
    long long ts = atoll(server.arg("ts").c_str());
    if (ts < 1700000000LL) { server.send(400, "text/plain", "BAD"); return; }
    if (timeSynced() && server.arg("force") != "1") { server.send(200, "text/plain", "SKIP"); return; }
    struct timeval tv = { (time_t)ts, 0 };
    settimeofday(&tv, nullptr);
    Serial.println("Time set from browser: " + nowStr());
    server.send(200, "text/plain", "OK");
  });

  server.on("/api/reboot", HTTP_POST, []() {
    server.send(200, "text/plain", "REBOOTING");
    delay(300);
    ESP.restart();
  });

  
  // --- تحكم مباشر في الريلاي: /api/relay?n=1&s=on|off|toggle  (بدون s بيرجّع الحالة بس) ---
  server.on("/api/relay", HTTP_ANY, []() {
    int n = server.arg("n").toInt();
    if (server.hasArg("s") && !relaySet(n, server.arg("s"))) { server.send(400, "text/plain", "BAD"); return; }
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", String("{\"r1\":") + (relayOn[0] ? 1 : 0) + ",\"r2\":" + (relayOn[1] ? 1 : 0) + "}");
  });

  // --- OTA من المتصفح: افتح http://<IP>/update وارفع ملف .bin ---
  server.on("/update", HTTP_GET, []() {
    if (!server.authenticate("admin", OTA_PASSWORD)) return server.requestAuthentication();
    server.send(200, "text/html; charset=utf-8",
      "<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'>"
      "<body style='font-family:sans-serif;max-width:420px;margin:40px auto;padding:0 16px;direction:rtl'>"
      "<h3>تحديث الفيرموير (OTA)</h3>"
      "<input type=file id=f accept='.bin'><br><br>"
      "<button onclick='up()'>رفع</button> <progress id=p value=0 max=100 style='width:100%'></progress>"
      "<pre id=m></pre>"
      "<script>function up(){var f=document.getElementById('f').files[0];if(!f)return;"
      "var x=new XMLHttpRequest(),d=new FormData();d.append('fw',f,f.name);"
      "x.upload.onprogress=function(e){document.getElementById('p').value=e.loaded*100/e.total};"
      "x.onload=function(){document.getElementById('m').textContent=x.status==200?'تم! الجهاز بيعيد التشغيل...':'فشل: '+x.responseText};"
      "x.onerror=function(){document.getElementById('m').textContent='انقطع الاتصال (ممكن يكون اتحدّث وبيعيد التشغيل)'};"
      "x.open('POST','/update');x.send(d)}</script></body>");
  });
  server.on("/update", HTTP_POST, []() {
    if (!server.authenticate("admin", OTA_PASSWORD)) return server.requestAuthentication();
    bool ok = !Update.hasError();
    server.sendHeader("Connection", "close");
    server.send(ok ? 200 : 500, "text/plain", ok ? "OK" : Update.errorString());
    if (ok) { delay(500); ESP.restart(); }
  }, []() {
    static bool authed = false;
    HTTPUpload& up = server.upload();
    if (up.status == UPLOAD_FILE_START) {
      authed = server.authenticate("admin", OTA_PASSWORD);
      if (!authed) return;
      setRGB(255, 160, 0);
      Serial.printf("Web OTA: %s\n", up.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
    } else if (!authed) {
      return;
    } else if (up.status == UPLOAD_FILE_WRITE) {
      if (Update.write(up.buf, up.currentSize) != up.currentSize) Update.printError(Serial);
    } else if (up.status == UPLOAD_FILE_END) {
      if (Update.end(true)) Serial.printf("Web OTA done: %u bytes\n", up.totalSize);
      else Update.printError(Serial);
    } else if (up.status == UPLOAD_FILE_ABORTED) {
      Update.abort();
    }
  });

  server.on("/api/heard", HTTP_GET, []() {
    DynamicJsonDocument d(3072);
    JsonArray a = d.to<JsonArray>();
    for (int k = 0; k < heardCnt; k++) {
      int idx = (heardHead - 1 - k + HEARD_N * 2) % HEARD_N;
      JsonObject o = a.createNestedObject();
      o["t"] = heardTxt[idx];
      o["ok"] = heardOk[idx] ? 1 : 0;
    }
    String out; serializeJson(d, out);
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", out);
  });
  server.on("/api/mic", HTTP_GET, []() {
    String j = String("{\"g\":") + String(micGain, 1) + ",\"v\":" + String(vadMinLevel, 1) + ",\"n\":" + String(noiseIdle, 1) +
               ",\"pk\":" + lastPeak16 + ",\"gain\":" + String(lastRecGain, 0) + ",\"ms\":" + lastSpeechMs + "}";
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", j);
  });
  server.on("/api/mic", HTTP_POST, []() {
    float g = server.arg("g").toFloat(), v = server.arg("v").toFloat();
    if (g < 2 || g > 128 || v < 2 || v > 60) { server.send(400, "text/plain", "BAD"); return; }
    micGain = g; vadMinLevel = v;
    micSaveCfg();
    server.send(200, "text/plain", "OK");
  });

  irRegisterRoutes();
  stripRegisterRoutes();
  pwaRegisterRoutes();
  stripInit();
  server.begin();
  Serial.println("Web Server Started. Say '" WAKE_WORD "' to wake up!");
  ledIdleSince = millis();          // يبدأ عدّاد دقيقة الـ LED الأزرق
  ledLast = -1;
}

void loop() {
  // مراقب الواي فاي: لو الاتصال وقع بيعيد الاتصال تلقائي (كل 10 ثواني)
  static uint32_t lastWifiCheck = 0;
  if (millis() - lastWifiCheck > 10000) {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) { Serial.println("WiFi lost, reconnecting..."); WiFi.reconnect(); }
  }
  ArduinoOTA.handle();
  server.handleClient();
  if (ntpFlag) { ntpFlag = false; Serial.println("Time synced (NTP): " + nowStr()); }
  irPoll();
  stripPoll();
  jobPoll();
  
  // أزرق خفيف دقيقة بعد آخر نشاط (استعداد) وبعدها يطفي
  standbyLed();

  // البحث عن كلمة التنبيه
  if (checkWakeWord()) {
    setRGB(0, 255, 255);                 // سيان: الكلمة اتمسكت (استنى الرد)
    playRandom(WAKE_CLIPS, sizeof(WAKE_CLIPS) / sizeof(WAKE_CLIPS[0]), lastWake);  // رد قصير - الأحمر بعده = اتكلم
    flushMic();                          // نتخلص من صوت السماعة اللي دخل المايك
    retryNo = 0;
    String text = recordAndTranscribe();
    bool done = executeCommand(text);
    // ماسمعتش/مافهمتش: الرد اتقال، دلوقتي المايك بيتفتح تاني (أحمر) من غير ما تنادي عليها
    for (int r = 1; !done && r <= RETRY_MAX; r++) {
      Serial.printf("Retry %d/%d - listening again (no wake word needed)\n", r, RETRY_MAX);
      flushMic();
      retryNo = r;
      text = recordAndTranscribe();
      done = executeCommand(text);
    }
    retryNo = 0;
    flushMic();
  }
}