#pragma once
// ==================== IR (إرسال + تعلّم) ====================
// - المرسل: LED IR على طرف TX (PWM كاريير عن طريق LEDC) - يفضل عبر ترانزستور
// - المستقبل: VS1838B / TSOP على طرف RX (خرجه بيبقى HIGH وقت الراحة)
// - الإشارات بتتخزن RAW (أطوال النبضات بالميكروثانية) في LittleFS: /ir.json + /ir/<id>.bin
// - الأطراف قابلة للتغيير من الويب وبتتحفظ في /ir_cfg.json
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#define IR_MAX_EDGES        600        // أقصى عدد حواف في الإشارة الواحدة
#define IR_GAP_US           40000UL    // صمت أطول من كده = نهاية الإشارة
#define IR_LEARN_TIMEOUT_MS 15000

static int irTx = 4, irRx = 6;
static int irTxActive = -1;

static volatile uint32_t irTs[IR_MAX_EDGES];
static volatile uint16_t irEdges = 0;
static volatile uint32_t irLastEdge = 0;

enum IrState { IR_IDLE, IR_WAITING, IR_CAPTURED, IR_TIMEOUT };
static IrState irState = IR_IDLE;
static uint32_t irLearnStart = 0;
static uint16_t irPending[IR_MAX_EDGES];
static uint16_t irPendingLen = 0;

// أطراف مسموحة (بدون 0/3/45/46 strapping ولا USB ولا UART0 ولا PSRAM ولا أطراف المايك/الأمب (5,7,8)/الـ LED)
static bool irPinOk(int p) {
  static const int ok[] = {1,2,4,6,9,10,11,12,13,14,18,21,38,39,40,41,42,47};
  for (size_t i = 0; i < sizeof(ok) / sizeof(ok[0]); i++) if (ok[i] == p) return true;
  return false;
}

static void IRAM_ATTR irIsr() {
  uint32_t now = (uint32_t)esp_timer_get_time();
  if (irEdges == 0 && gpio_get_level((gpio_num_t)irRx) == 1) return;   // لازم أول حافة تبقى نازلة (بداية نبضة)
  if (irEdges < IR_MAX_EDGES) irTs[irEdges++] = now;
  irLastEdge = now;
}

static void irStopRx() { detachInterrupt(digitalPinToInterrupt(irRx)); }

static void irApplyTx() {
  if (irTxActive >= 0) { ledcWrite(irTxActive, 0); ledcDetach(irTxActive); pinMode(irTxActive, INPUT); }
  ledcAttach(irTx, 38000, 8);
  ledcWrite(irTx, 0);
  irTxActive = irTx;
}

static void irLoadCfg() {
  File f = LittleFS.open("/ir_cfg.json", "r");
  if (!f) return;
  StaticJsonDocument<128> d;
  if (!deserializeJson(d, f)) {
    int t = d["tx"] | irTx, r = d["rx"] | irRx;
    if (irPinOk(t) && irPinOk(r) && t != r) { irTx = t; irRx = r; }
  }
  f.close();
}

static void irSaveCfg() {
  File f = LittleFS.open("/ir_cfg.json", "w");
  if (!f) return;
  f.printf("{\"tx\":%d,\"rx\":%d}", irTx, irRx);
  f.close();
}

static void irInit() {
  LittleFS.mkdir("/ir");
  irLoadCfg();
  irApplyTx();
  Serial.printf("IR ready | TX=GPIO%d RX=GPIO%d\n", irTx, irRx);
}

// بيتنادى من loop() ومن حالة الويب: بيحدد نهاية الالتقاط
static void irPoll() {
  if (irState != IR_WAITING) return;
  uint16_t n = irEdges;
  bool gap = n >= 4 && (uint32_t)((uint32_t)esp_timer_get_time() - irLastEdge) > IR_GAP_US;
  if (gap || n >= IR_MAX_EDGES) {
    irStopRx();
    n = irEdges;
    uint16_t k = 0;
    for (uint16_t i = 1; i < n && k < IR_MAX_EDGES; i++) {
      uint32_t d = irTs[i] - irTs[i - 1];
      irPending[k++] = d > 65535 ? 65535 : (uint16_t)d;
    }
    if (k % 2 == 0 && k > 0) k--;               // لازم تنتهي بـ mark
    if (k < 5) {                                // ضوضاء: كمّل انتظار
      irEdges = 0;
      attachInterrupt(digitalPinToInterrupt(irRx), irIsr, CHANGE);
      return;
    }
    irPendingLen = k;
    irState = IR_CAPTURED;
    Serial.printf("IR captured: %u pulses\n", k);
  } else if (millis() - irLearnStart > IR_LEARN_TIMEOUT_MS) {
    irStopRx();
    irState = IR_TIMEOUT;
    Serial.println("IR learn timeout");
  }
}

static void irStartLearn() {
  if (irState == IR_WAITING) irStopRx();
  irEdges = 0;
  pinMode(irRx, INPUT);
  attachInterrupt(digitalPinToInterrupt(irRx), irIsr, CHANGE);
  irLearnStart = millis();
  irState = IR_WAITING;
  Serial.println("IR learning... press a remote button");
}

static void irSendRaw(const uint16_t* d, uint16_t n, uint16_t khz, uint8_t rep) {
  if (khz < 30 || khz > 60) khz = 38;
  if (rep < 1) rep = 1;
  if (rep > 5) rep = 5;
  ledcChangeFrequency(irTx, khz * 1000UL, 8);
  const uint32_t duty = 85;                       // ~33% duty
  for (uint8_t r = 0; r < rep; r++) {
    uint32_t t = (uint32_t)esp_timer_get_time();
    for (uint16_t i = 0; i < n; i++) {
      ledcWrite(irTx, (i % 2 == 0) ? duty : 0);   // الزوجي = mark
      t += d[i];
      while ((int32_t)((uint32_t)esp_timer_get_time() - t) < 0) { }
    }
    ledcWrite(irTx, 0);
    if (r + 1 < rep) delay(40);
  }
}

static void irLoadMeta(DynamicJsonDocument& doc) {
  File f = LittleFS.open("/ir.json", "r");
  if (f) { deserializeJson(doc, f); f.close(); }
  if (!doc.is<JsonArray>()) { doc.clear(); doc.to<JsonArray>(); }
}

static bool irSaveMeta(DynamicJsonDocument& doc) {
  File f = LittleFS.open("/ir.json", "w");
  if (!f) return false;
  serializeJson(doc, f);
  f.close();
  return true;
}

// بتتنادى من runAction (خطوة نوعها "ir" و val = رقم الإشارة)
bool irSendById(int id) {
  DynamicJsonDocument m(8192);
  irLoadMeta(m);
  JsonVariant found;
  for (JsonVariant c : m.as<JsonArray>()) if ((int)c["id"] == id) { found = c; break; }
  if (found.isNull()) { Serial.println("IR code not found: " + String(id)); return false; }
  File f = LittleFS.open("/ir/" + String(id) + ".bin", "r");
  if (!f) return false;
  size_t n = f.size() / 2;
  if (n == 0 || n > IR_MAX_EDGES) { f.close(); return false; }
  uint16_t* buf = (uint16_t*)malloc(n * 2);
  if (!buf) { f.close(); return false; }
  f.read((uint8_t*)buf, n * 2);
  f.close();
  irSendRaw(buf, (uint16_t)n, found["khz"] | 38, found["rep"] | 1);
  Serial.printf("Sent IR: %s (%u pulses)\n", found["name"].as<String>().c_str(), (unsigned)n);
  free(buf);
  return true;
}

static void irRegisterRoutes() {
  server.on("/api/ir/cfg", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", "{\"tx\":" + String(irTx) + ",\"rx\":" + String(irRx) + "}");
  });

  server.on("/api/ir/cfg", HTTP_POST, []() {
    int t = server.arg("tx").toInt(), r = server.arg("rx").toInt();
    if (!irPinOk(t) || !irPinOk(r) || t == r) { server.send(400, "text/plain", "BAD PIN"); return; }
    if (irState == IR_WAITING) { irStopRx(); irState = IR_IDLE; }
    irRx = r;
    if (t != irTx) { irTx = t; irApplyTx(); }
    irSaveCfg();
    Serial.printf("IR pins saved | TX=GPIO%d RX=GPIO%d\n", irTx, irRx);
    server.send(200, "text/plain", "OK");
  });

  server.on("/api/ir/learn", HTTP_POST, []() {
    irStartLearn();
    server.send(200, "text/plain", "OK");
  });

  server.on("/api/ir/status", HTTP_GET, []() {
    irPoll();
    const char* s = irState == IR_WAITING ? "waiting" : irState == IR_CAPTURED ? "captured" : irState == IR_TIMEOUT ? "timeout" : "idle";
    String j = String("{\"state\":\"") + s + "\",\"len\":" + String(irState == IR_CAPTURED ? irPendingLen : 0) + "}";
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", j);
  });

  server.on("/api/ir/list", HTTP_GET, []() {
    DynamicJsonDocument m(8192);
    irLoadMeta(m);
    String out;
    serializeJson(m, out);
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", out);
  });

  // id=0 → حفظ الإشارة الملتقطة كجديدة | id>0 → تعديل الاسم/التردد/التكرار فقط
  server.on("/api/ir/save", HTTP_POST, []() {
    int id = server.arg("id").toInt();
    String name = server.arg("name");
    name.trim();
    int khz = server.arg("khz").toInt();
    int rep = server.arg("rep").toInt();
    if (khz < 30 || khz > 60) khz = 38;
    if (rep < 1 || rep > 5) rep = 1;
    if (name.length() == 0) { server.send(400, "text/plain", "NO NAME"); return; }

    DynamicJsonDocument m(8192);
    irLoadMeta(m);
    JsonArray arr = m.as<JsonArray>();
    if (id == 0) {
      if (irState != IR_CAPTURED) { server.send(409, "text/plain", "NOTHING CAPTURED"); return; }
      int mx = 0;
      for (JsonObject o : arr) mx = max(mx, (int)o["id"]);
      id = mx + 1;
      File f = LittleFS.open("/ir/" + String(id) + ".bin", "w");
      if (!f) { server.send(500, "text/plain", "ERROR"); return; }
      f.write((const uint8_t*)irPending, irPendingLen * 2);
      f.close();
      JsonObject o = arr.createNestedObject();
      o["id"] = id; o["name"] = name; o["khz"] = khz; o["rep"] = rep; o["len"] = irPendingLen;
      irState = IR_IDLE;
    } else {
      bool found = false;
      for (JsonObject o : arr) if ((int)o["id"] == id) { o["name"] = name; o["khz"] = khz; o["rep"] = rep; found = true; break; }
      if (!found) { server.send(404, "text/plain", "NOT FOUND"); return; }
    }
    if (m.overflowed() || !irSaveMeta(m)) { server.send(500, "text/plain", "ERROR"); return; }
    Serial.println("IR saved: " + name);
    server.send(200, "text/plain", String(id));
  });

  server.on("/api/ir/send", HTTP_POST, []() {
    if (server.arg("pending") == "1") {                 // تجربة الإشارة الملتقطة قبل الحفظ
      if (irState != IR_CAPTURED) { server.send(409, "text/plain", "NOTHING CAPTURED"); return; }
      irSendRaw(irPending, irPendingLen, server.arg("khz").toInt(), server.arg("rep").toInt());
      server.send(200, "text/plain", "OK");
      return;
    }
    bool ok = irSendById(server.arg("id").toInt());
    server.send(ok ? 200 : 404, "text/plain", ok ? "OK" : "FAILED");
  });

  server.on("/api/ir/delete", HTTP_POST, []() {
    int id = server.arg("id").toInt();
    DynamicJsonDocument m(8192);
    irLoadMeta(m);
    JsonArray arr = m.as<JsonArray>();
    bool found = false;
    for (size_t i = 0; i < arr.size(); i++) if ((int)arr[i]["id"] == id) { arr.remove(i); found = true; break; }
    if (!found) { server.send(404, "text/plain", "NOT FOUND"); return; }
    LittleFS.remove("/ir/" + String(id) + ".bin");
    irSaveMeta(m);
    server.send(200, "text/plain", "OK");
  });

  // ---- نسخة احتياطية: قراءة النبضات الخام لإشارة (للتصدير) ----
  server.on("/api/ir/raw", HTTP_GET, []() {
    int id = server.arg("id").toInt();
    File f = LittleFS.open("/ir/" + String(id) + ".bin", "r");
    if (!f) { server.send(404, "text/plain", "NOT FOUND"); return; }
    size_t n = f.size() / 2;
    if (n == 0 || n > IR_MAX_EDGES) { f.close(); server.send(500, "text/plain", "BAD SIZE"); return; }
    uint16_t* buf = (uint16_t*)malloc(n * 2);
    if (!buf) { f.close(); server.send(500, "text/plain", "NO MEM"); return; }
    f.read((uint8_t*)buf, n * 2);
    f.close();
    String out;
    out.reserve(n * 6 + 2);
    for (size_t i = 0; i < n; i++) { if (i) out += ','; out += String(buf[i]); }
    free(buf);
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/plain", out);
  });

  // ---- نسخة احتياطية: استيراد إشارة جديدة ----
  // POST /api/ir/import?name=..&khz=38&rep=1   والـ body = النبضات مفصولة بفواصل
  // بترجّع الرقم الجديد للإشارة
  server.on("/api/ir/import", HTTP_POST, []() {
    String name = server.arg("name");
    name.trim();
    int khz = server.arg("khz").toInt();
    int rep = server.arg("rep").toInt();
    if (khz < 30 || khz > 60) khz = 38;
    if (rep < 1 || rep > 5) rep = 1;
    if (name.length() == 0) { server.send(400, "text/plain", "NO NAME"); return; }

    String body = server.arg("plain");
    uint16_t* buf = (uint16_t*)malloc(IR_MAX_EDGES * 2);
    if (!buf) { server.send(500, "text/plain", "NO MEM"); return; }
    size_t n = 0;
    int pos = 0, L = body.length();
    while (pos < L && n < IR_MAX_EDGES) {
      int c = body.indexOf(',', pos);
      if (c < 0) c = L;
      long v = body.substring(pos, c).toInt();
      if (v < 1 || v > 65535) { free(buf); server.send(400, "text/plain", "BAD DATA"); return; }
      buf[n++] = (uint16_t)v;
      pos = c + 1;
    }
    if (n < 5 || n % 2 == 0) { free(buf); server.send(400, "text/plain", "BAD LEN"); return; }   // لازم تنتهي بـ mark

    DynamicJsonDocument m(8192);
    irLoadMeta(m);
    JsonArray arr = m.as<JsonArray>();
    int mx = 0;
    for (JsonObject o : arr) mx = max(mx, (int)o["id"]);
    int id = mx + 1;
    File f = LittleFS.open("/ir/" + String(id) + ".bin", "w");
    if (!f) { free(buf); server.send(500, "text/plain", "ERROR"); return; }
    f.write((const uint8_t*)buf, n * 2);
    f.close();
    free(buf);
    JsonObject o = arr.createNestedObject();
    o["id"] = id; o["name"] = name; o["khz"] = khz; o["rep"] = rep; o["len"] = (int)n;
    if (m.overflowed() || !irSaveMeta(m)) { LittleFS.remove("/ir/" + String(id) + ".bin"); server.send(500, "text/plain", "ERROR"); return; }
    Serial.println("IR imported: " + name);
    server.send(200, "text/plain", String(id));
  });
}
