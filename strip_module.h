#pragma once
// ==================== مشترك MTTL-W01 (LG U+ / TONLY) ====================
// الـ ESP32 هنا هو "الكنترولر": المشترك بيتصل بيه (TCP outbound) على بورت 10086.
// - لازم تعمل إعداد للمشترك مرة واحدة (من تطبيق MTTL Control > Device setup) وتكتب فيه IP الـ ESP32 بدل IP الكمبيوتر.
// - البروتوكول نص عادي (UTF-8) وكل سطر بينتهي بـ \r\n (والفيرموير بيبعت NUL padding بعد السطر - بنتجاهله).
// - runAction نوع "strip" و val = "<مخرج>:<حالة>"  مثال: 2:on | 2:off | 3:toggle | 0:off (0 = كل المخارج)
#include <Arduino.h>
#include <WiFi.h>

#define STRIP_PORT      10086
#define STRIP_POLL_MS   10000UL     // up:getinfo:all كل 10 ثواني
#define STRIP_DEAD_MS   45000UL     // مفيش أي بيانات بالمدة دي = الجلسة ميتة
#define STRIP_PEND_MS   10000UL     // اتصال جديد لازم يبعت bootinfo خلال المدة دي
#define STRIP_LINE_MAX  2048

struct StripOutlet {
  bool on = false;
  bool ovOk = true;     // F3: on = طبيعي
  bool htOk = true;     // F4: on = طبيعي
  float w = 0;          // F5 / 1000
  uint32_t wh = 0;      // F6 (hex) = Wh تراكمي
  int temp = 0;         // F11
  uint8_t ev = 0;       // F10 (hex) 00 = طبيعي
};

static WiFiServer stripServer(STRIP_PORT);
static WiFiClient stripCli;          // الجلسة المعتمدة (بعد bootinfo)
static WiFiClient stripPend;         // اتصال جديد مستني bootinfo
static String stripBuf, stripPendBuf;
static bool stripIdent = false;
static String stripMac, stripFw, stripModel;
static StripOutlet stripOut[4];
static bool stripHaveTele = false;
static uint32_t stripLastRx = 0, stripLastTele = 0, stripLastPoll = 0, stripPollAt = 0, stripPendSince = 0;

static int stripSplit(const String& s, char sep, String* out, int maxN) {
  int n = 0, pos = 0;
  while (n < maxN) {
    int i = s.indexOf(sep, pos);
    if (i < 0) { out[n++] = s.substring(pos); break; }
    out[n++] = s.substring(pos, i);
    pos = i + 1;
  }
  return n;
}

static bool stripIsHex(const String& s, size_t len) {
  if (s.length() != len) return false;
  for (size_t i = 0; i < len; i++) if (!isxdigit((unsigned char)s[i])) return false;
  return true;
}

static void stripSendLine(WiFiClient& c, const String& s) {
  if (c.connected()) { c.print(s); c.print("\r\n"); }
}

static void stripDrop(const char* why) {
  if (stripCli) stripCli.stop();
  if (stripIdent) Serial.printf("Strip disconnected (%s)\n", why);
  stripIdent = false;
  stripBuf = "";
  stripHaveTele = false;
}

// up:bootinfo:<MODEL>;<MAC1>;<MAC2>;<FW>;connect
static bool stripParseBoot(const String& line) {
  String p[6];
  int n = stripSplit(line.substring(12), ';', p, 6);
  if (n != 5) return false;
  if (p[0].length() < 1 || p[0].length() > 32) return false;
  if (!stripIsHex(p[1], 12) || !stripIsHex(p[2], 12) || !p[1].equalsIgnoreCase(p[2])) return false;
  if (p[3].length() < 1 || p[3].length() > 64 || p[4] != "connect") return false;
  stripModel = p[0];
  stripMac = p[1]; stripMac.toUpperCase();
  stripFw = p[3];
  return true;
}

// up:getinfo:1:<rec>:2:<rec>:3:<rec>:4:<rec>   (كل rec فيه 12 حقل بينهم ;)
static bool stripParseTele(const String& body) {
  String t[9];
  if (stripSplit(body, ':', t, 9) != 8) return false;
  StripOutlet tmp[4];
  uint8_t seen = 0;
  for (int k = 0; k < 4; k++) {
    int ch = t[k * 2].toInt();
    if (ch < 1 || ch > 4 || (seen & (1 << (ch - 1)))) return false;
    String f[13];
    if (stripSplit(t[k * 2 + 1], ';', f, 13) != 12) return false;
    if (!stripIsHex(f[6], 8) || !stripIsHex(f[10], 2)) return false;
    StripOutlet& o = tmp[ch - 1];
    o.on = f[1] == "on";
    o.ovOk = f[3] == "on";
    o.htOk = f[4] == "on";
    o.w = f[5].toInt() / 1000.0f;
    o.wh = (uint32_t)strtoul(f[6].c_str(), nullptr, 16);
    o.ev = (uint8_t)strtoul(f[10].c_str(), nullptr, 16);
    o.temp = f[11].toInt();
    seen |= 1 << (ch - 1);
  }
  if (seen != 0x0F) return false;
  for (int i = 0; i < 4; i++) stripOut[i] = tmp[i];
  stripHaveTele = true;
  stripLastTele = millis();
  return true;
}

static void stripRequestInfo() {
  stripSendLine(stripCli, "up:getinfo:all");
  stripLastPoll = millis();
}

// سطر جاي من الجلسة المعتمدة
static void stripHandleLine(String line) {
  line.trim();
  if (line.length() == 0) return;
  stripLastRx = millis();
  if (line.startsWith("up:getinfo:")) {
    if (!stripParseTele(line.substring(11))) Serial.println("Strip: bad telemetry frame");
    return;
  }
  if (line.startsWith("up:event:")) line = "up:" + line.substring(9);
  if (line.startsWith("up:onoff:")) {                       // up:onoff:<1-4>:<on|off>
    int n = line.substring(9, 10).toInt();
    String s = line.substring(11);
    s.toLowerCase();
    if (line.charAt(10) == ':' && n >= 1 && n <= 4 && (s == "on" || s == "off")) {
      stripOut[n - 1].on = (s == "on");
      Serial.printf("Strip outlet %d -> %s\n", n, s.c_str());
    }
    return;
  }
  Serial.println("Strip: " + line.substring(0, 80));
}

// قراءة من سوكت (حد أقصى بايتات في الدورة عشان الـ loop ما يتعطلش)
static void stripReadFrom(WiFiClient& c, String& buf, bool pending) {
  int guard = 0;
  while (c.available() && guard++ < 600) {
    int ch = c.read();
    if (ch < 0) break;
    if (ch == 0 || ch == '\r') continue;
    if (ch != '\n') {
      if (buf.length() < STRIP_LINE_MAX) buf += (char)ch; else buf = "";
      continue;
    }
    String line = buf; buf = "";
    line.trim();
    if (line.length() == 0) continue;
    if (!pending) { stripHandleLine(line); continue; }
    // اتصال جديد: ما بنعتمده غير لما يبعت bootinfo صحيح
    if (line.startsWith("up:bootinfo:") && stripParseBoot(line)) {
      if (stripCli) stripCli.stop();                       // الاتصال الجديد يكسب (زي ما الدليل بيقول)
      stripCli = stripPend;
      stripPend = WiFiClient();
      stripPendBuf = "";
      stripBuf = "";
      stripIdent = true;
      stripHaveTele = false;
      stripLastRx = millis();
      Serial.printf("Strip connected: %s | MAC %s | FW %s\n", stripModel.c_str(), stripMac.c_str(), stripFw.c_str());
      stripRequestInfo();
      return;                                               // الباقي بيتقرا من stripCli في الدورة الجاية
    }
  }
}

static void stripInit() {
  stripServer.begin();
  stripServer.setNoDelay(true);
  Serial.printf("Strip controller listening | %s:%d\n", WiFi.localIP().toString().c_str(), STRIP_PORT);
}

// بيتنادى من loop()
static void stripPoll() {
  uint32_t now = millis();

  if (stripServer.hasClient()) {
    WiFiClient n = stripServer.accept();
    if (n) {
      if (stripPend) stripPend.stop();
      stripPend = n;
      stripPend.setNoDelay(true);
      stripPendBuf = "";
      stripPendSince = now;
      Serial.println("Strip: incoming connection from " + n.remoteIP().toString());
    }
  }

  if (stripPend) {
    if (!stripPend.connected() && !stripPend.available()) stripPend.stop();
    else if ((int32_t)(now - stripPendSince) > (int32_t)STRIP_PEND_MS) { stripPend.stop(); Serial.println("Strip: pending connection timed out"); }
    else stripReadFrom(stripPend, stripPendBuf, true);
  }

  if (!stripIdent) return;
  if (!stripCli.connected() && !stripCli.available()) { stripDrop("socket closed"); return; }
  stripReadFrom(stripCli, stripBuf, false);
  now = millis();                                            // مهم: ممكن stripLastRx يكون اتحدّث فوق، فنقارن بوقت جديد
  if ((int32_t)(now - stripLastRx) > (int32_t)STRIP_DEAD_MS) { stripDrop("timeout"); return; }

  if (stripPollAt && (int32_t)(now - stripPollAt) >= 0) { stripPollAt = 0; stripRequestInfo(); }
  else if ((int32_t)(now - stripLastPoll) >= (int32_t)STRIP_POLL_MS) stripRequestInfo();
}

static bool stripReady() { return stripIdent && stripCli.connected(); }

// n: 0 = كل المخارج | 1..4   s: on / off / toggle
static bool stripSet(int n, String s) {
  s.trim(); s.toLowerCase();
  if (n < 0 || n > 4) return false;
  if (s != "on" && s != "off" && s != "toggle" && s != "1" && s != "0") return false;
  if (!stripReady()) { Serial.println("Strip: not connected"); return false; }
  bool on;
  if (s == "on" || s == "1") on = true;
  else if (s == "off" || s == "0") on = false;
  else if (n == 0) { bool any = false; for (int i = 0; i < 4; i++) any |= stripOut[i].on; on = !any; }
  else on = !stripOut[n - 1].on;
  stripSendLine(stripCli, String("up:onoff:") + n + ":" + (on ? "on" : "off"));
  if (n == 0) { for (int i = 0; i < 4; i++) stripOut[i].on = on; } else stripOut[n - 1].on = on;   // تقدير مؤقت لحد ما التليمتري يأكد
  stripPollAt = millis() + 700;                              // اطلب التليمتري بعد الأمر للتأكيد
  Serial.printf("Strip cmd: outlet %d -> %s\n", n, on ? "on" : "off");
  return true;
}

// "<n>:<state>"
static bool stripValOk(const String& v) {
  int c = v.indexOf(':');
  if (c < 1) return false;
  String n = v.substring(0, c), s = v.substring(c + 1);
  s.trim(); s.toLowerCase();
  if (n.length() != 1 || n[0] < '0' || n[0] > '4') return false;
  return s == "on" || s == "off" || s == "toggle";
}

// بتتنادى من runAction (خطوة نوعها "strip")
bool stripAction(const String& val) {
  int c = val.indexOf(':');
  if (c < 1) return false;
  return stripSet(val.substring(0, c).toInt(), val.substring(c + 1));
}

static void stripRegisterRoutes() {
  server.on("/api/strip/status", HTTP_GET, []() {
    String j = "{\"ip\":\"" + WiFi.localIP().toString() + "\",\"port\":" + String(STRIP_PORT) +
               ",\"on\":" + (stripReady() ? "true" : "false") +
               ",\"mac\":\"" + (stripIdent ? stripMac : String("")) + "\",\"fw\":\"" + (stripIdent ? stripFw : String("")) +
               "\",\"age\":" + String(stripHaveTele ? (long)((millis() - stripLastTele) / 1000) : -1) + ",\"o\":[";
    for (int i = 0; i < 4; i++) {
      const StripOutlet& o = stripOut[i];
      if (i) j += ",";
      j += "{\"on\":" + String(o.on ? 1 : 0) + ",\"w\":" + String(o.w, 1) + ",\"wh\":" + String(o.wh) +
           ",\"t\":" + String(o.temp) + ",\"ok\":" + String((o.ovOk && o.htOk && o.ev == 0) ? 1 : 0) + "}";
    }
    j += "]}";
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", j);
  });

  server.on("/api/strip/set", HTTP_POST, []() {
    int n = server.arg("n").toInt();
    String s = server.arg("s");
    if (n < 0 || n > 4 || !(s == "on" || s == "off" || s == "toggle")) { server.send(400, "text/plain", "BAD"); return; }
    if (!stripReady()) { server.send(409, "text/plain", "NOT CONNECTED"); return; }
    server.send(stripSet(n, s) ? 200 : 500, "text/plain", "OK");
  });

  server.on("/api/strip/refresh", HTTP_POST, []() {
    if (!stripReady()) { server.send(409, "text/plain", "NOT CONNECTED"); return; }
    stripRequestInfo();
    server.send(200, "text/plain", "OK");
  });
}
