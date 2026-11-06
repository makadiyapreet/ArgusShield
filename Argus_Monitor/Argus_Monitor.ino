/*
  ====================================================================
  ARGUS v4 - Autonomous Threat Detection and Response for IoT Networks
  MONITOR NODE (ESP32 DevKit V1)

  Hardware used (nothing else is needed):
    ESP32 DevKit, DHT11 (or DHT22), 1-channel relay, RGB LED
    (COMMON ANODE - see LED_COMMON_ANODE below) + 3x 220 ohm, active
    buzzer, 1 loose jumper wire (tamper sensor), the on-board BOOT
    button, and a protected load (5V fan or LED + resistor) wired
    through the relay.

  Libraries (Library Manager):  DHT sensor library, Adafruit Unified
  Sensor, PubSubClient.  Everything else is built into the ESP32 core.
  Board: "ESP32 Dev Module".  Serial Monitor: 115200 baud.

  DETECTION MODULES (13)
   0 Rogue AP    evil twin: same SSID, different BSSID (async scan)
   1 ARP / MITM  gateway MAC in the ESP32's own ARP cache changes
   2 MQTT policy unknown topics/devices, bad zero-trust key, oversized
                 payload (overflow probe), message floods
   3 Firmware    real SHA-256 of the running firmware image vs trusted
   4 Sensor      DHT range / jump / disconnect + fake data on MQTT topic
   5 Wi-Fi link  deauth-style disconnect bursts + sudden signal drop
   6 Tamper      loose wire on GPIO 13 (capacitive touch) = "case opened"
   7 Probe/Scan  repeated unknown web paths or PIN brute-force = recon
   8 Replay      a validly-keyed MQTT message reusing an old/old sequence
                 number = a captured packet being replayed
   9 Memory/DoS  free heap collapsing = payload-driven exhaustion attack
  10 Reboot      the last reset was a panic / watchdog / brownout, not a
                 normal power-on or operator reset = possible fault attack
  11 Session     dashboard commands from a second IP within 2 minutes of
                 the first = possible stolen PIN / session hijack
  12 Wi-Fi flood a sudden jump in total networks visible (any SSID) = flooding
                 the air with fake beacons, or jamming
  CORRELATION   1 module = WARNING; 2+ modules in 20 s = CRITICAL
  RESPONSE      relay isolation, LED, buzzer, Telegram, hash-chained log
  EXTRAS        modes (Armed / Monitor only / Lockdown), self-heal,
                Telegram commands, PIN-protected dashboard controls,
                demo scenarios, hardware self-test, mDNS (argus.local),
                zero-trust MQTT device keys + live device registry,
                passive Wi-Fi "scout" scan of every nearby network

  DETAIL LOGGING (v4.1) - a module that detects a THRESHOLD being crossed
  (N wrong PINs, N unknown pages, N msg/s, N drops) now also logs every
  individual contributing event on its own line (logDetail(), low
  severity), in addition to the one high-severity verdict once the
  threshold is actually crossed. So a PIN brute-force shows as 5 separate
  "Wrong PIN attempt" lines plus 1 "Dashboard brute-force" verdict, a scan
  shows every path hit plus 1 "possible scan" verdict, and so on for MQTT
  floods and Wi-Fi disconnect bursts. These detail lines cannot by
  themselves trigger isolation (correlation needs 2+ DIFFERENT modules),
  they only make the evidence trail visible in the alert feed/timeline/log.

  Serial commands: help status verify sim <0-12> isolate release
                   trust rebase wipe
  ====================================================================
*/

// ============================ USER SETTINGS ============================
#define ARGUS_VERSION   "4.1"
const char* WIFI_SSID   = "YOUR_WIFI_SSID";      // the REAL, trusted network (2.4 GHz)
const char* WIFI_PASS   = "YOUR_WIFI_PASSWORD";
const char* TRUSTED_EXTRA_BSSIDS = "";           // other APs of the SAME network, e.g. "AA:BB:CC:DD:EE:FF,11:22:33:44:55:66"
const char* ADMIN_PIN   = "CHANGE_THIS_PIN";      // dashboard control PIN
const char* MQTT_HOST   = "";                      // laptop running Mosquitto ("" = MQTT off)
const uint16_t MQTT_PORT = 1883;
const char* BOT_TOKEN   = "";                      // Telegram bot token ("" = disabled)
const char* CHAT_ID     = "";                      // Telegram chat / group id
const char* DEVICE_KEYS = "sensor1:CHANGE_ME,sensor2:CHANGE_ME";  // zero-trust MQTT: "id:key,id2:key2" - a device must send its key
                                                 // with every message or it is treated as unauthenticated

#define DHT_TYPE          DHTesp::DHT22                  // DHT11 or DHT22
#define RELAY_ACTIVE_LOW  true                   // blue relay boards are usually active-LOW. If the load is
                                                 // OFF at boot, change this value and re-upload.
#define LED_COMMON_ANODE  true                   // true = common leg to 3V3 (most RGB LEDs sold loose are this).
                                                 // false = common leg to GND (common cathode). If the LED lights
                                                 // the wrong colours or stays lit backwards, flip this and re-upload.
#define USE_ARP_CHECK     1                      // 0 = remove ARP module if your core version will not compile it
#define USE_TAMPER        1                      // 0 = no tamper wire fitted
// Relay wiring: protected load on COM + NC.  Coil OFF = load powered, coil ON = isolated.

// ============================ INCLUDES =================================
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <time.h>
#include <math.h>
#include "mbedtls/sha256.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#if USE_ARP_CHECK
#include "lwip/etharp.h"
#include "lwip/netif.h"
#endif

// ============================ PIN MAP ==================================
#define PIN_DHT       4
#define PIN_RELAY     5
#define PIN_LED_R    25
#define PIN_LED_G    26
#define PIN_LED_B    27
#define PIN_BUZZER   14
#define PIN_TAMPER   13      // touch pad T4: connect ONE loose jumper wire here
#define PIN_BOOT      0      // on-board BOOT button
#define PIN_ONBOARD   2      // on-board blue LED

// ============================ TUNING ===================================
const unsigned long WINDOW_MS     = 20000;   // events this close together are "correlated"
const unsigned long QUIET_MS      = 30000;   // calm time needed before self-heal
const unsigned long DEBOUNCE_MS   = 3000;    // per-module alert rate limit
const unsigned long AP_SCAN_EVERY = 15000;
const int           AP_FLOOD_MIN  = 12;      // need at least this many networks visible before a jump counts
const int           AP_FLOOD_JUMP = 6;       // and this much more than the learned normal count
const int           FLOOD_PER_SEC = 15;      // MQTT msgs/sec treated as a flood
const size_t        LOG_MAX_BYTES = 40000;   // rotate log file above this size
const char* TOPIC_TEMP = "argus/dev/sensor1/temp";
const char* TOPIC_HUM  = "argus/dev/sensor1/hum";
const char* LOG_PATH   = "/argus_log.txt";
const char* LOG_OLD    = "/argus_log_old.txt";

// ============================ TYPES / GLOBALS ==========================
enum Module : uint8_t { M_AP, M_ARP, M_MQTT, M_FW, M_SENS, M_LINK, M_TAMPER, M_SCAN, M_REPLAY, M_DOS, M_REBOOT, M_SESSION, M_JAM, M_COUNT };
const char* MOD_NAME[M_COUNT] = {"Rogue AP", "ARP/MITM", "MQTT policy", "Firmware", "Sensor", "Wi-Fi link", "Tamper", "Probe/Scan", "Replay", "Memory/DoS", "Reboot", "Session", "Wi-Fi flood"};
enum Sev : uint8_t { SEV_LOW = 1, SEV_MED = 2, SEV_HIGH = 3, SEV_CRIT = 4 };
enum Level : uint8_t { L_NOMINAL = 0, L_WARNING = 1, L_CRITICAL = 2 };
enum Mode : uint8_t { MODE_ARMED = 0, MODE_MONITOR = 1, MODE_LOCKDOWN = 2 };

struct Alert { uint32_t ms; uint8_t mod; uint8_t sev; char msg[60]; };
struct Incident { uint32_t start; uint32_t end; char why[56]; };

const int MAX_ALERTS = 60;
const int MAX_INC = 8;
const int HIST_N = 120;

Alert alerts[MAX_ALERTS];
int alertHead = 0, alertCount = 0;
Incident incs[MAX_INC];
int incHead = 0, incCount = 0;
bool moduleSeen[M_COUNT];
bool moduleOff[M_COUNT];
unsigned long moduleLast[M_COUNT];
uint32_t moduleCnt[M_COUNT];
unsigned long lastEventMs = 0;
String lastMsg = "";

Level level = L_NOMINAL;
Mode mode = MODE_ARMED;
bool isolated = false, muted = false;
unsigned long alarmUntil = 0, beepUntil = 0, lastHealTry = 0, lastWarnTg = 0;

DHTesp dht;
WiFiClient netClient;
PubSubClient mqtt(netClient);
WebServer server(80);
WiFiUDP udp;
Preferences prefs;

String chainHead = "GENESIS";
uint32_t chainSeq = 0, bootCount = 0;
bool fsOK = false;

// sensor state
float lastT = NAN, lastH = NAN;
int dhtFail = 0;
String lastSelfTemp = "", lastSelfHum = "";
unsigned long lastSelfPubMs = 0, mqttArmedAt = 0, lastMqttTry = 0;
volatile int mqttCount = 0;
int16_t hT[HIST_N], hH[HIST_N], hR[HIST_N];
int histN = 0, histHead = 0;

// network state
String trustedBssid = "";
bool scanning = false;
unsigned long lastScan = 0, wifiLostAt = 0;
uint8_t gwMac[6] = {0};
bool gwKnown = false;
unsigned long lastKick = 0;

// Wi-Fi "scout" - every nearby network seen on the last scan, not just the trusted SSID
struct ApSeen { char ssid[24]; char bssid[18]; int8_t rssi; uint8_t ch; bool open; bool self; };
const int MAX_SCOUT = 10;
ApSeen scout[MAX_SCOUT];
int scoutN = 0;
float apBaseline = -1;                             // slow-moving average of "how many networks is normal here"
unsigned long scoutAt = 0;

// zero-trust MQTT device registry - every device id seen, and whether its last message had a valid key
struct DevSeen { char id[16]; unsigned long lastMs; bool trusted; bool everBad; uint32_t msgs; uint32_t lastSeq; };
const int MAX_DEV = 6;
DevSeen devs[MAX_DEV];
int devCount = 0;

// probe / port-scan style detection against the web server
unsigned long probeT[8] = {0};
int probeI = 0;

// MQTT replay protection - this node's own outgoing sequence counter
uint32_t selfSeq = 0;

// memory / DoS watch
uint32_t heapHigh = 0;
unsigned long lastHeapAlert = 0;

// dashboard session / control-hijack watch
String lastCmdIp = "";
unsigned long lastCmdMs = 0;

// link state
volatile unsigned long discT[6] = {0, 0, 0, 0, 0, 0};
volatile uint8_t discI = 0;
volatile int lastReason = 0;
float rssiEma = 0;
int rssiN = 0, rssiBad = 0;

// tamper state
float touchBase = 0;
int touchHits = 0;

// firmware state
String fwNow = "";
bool fwOK = true;

// telegram
SemaphoreHandle_t tgMux;
String tgOut[6];
int tgOutN = 0;
volatile int pendingCmd = 0;     // 1 status 2 isolate 3 release 4 mute 5 unmute 6 armed 7 monitor 8 help
volatile int tgLastCode = 0;

// web auth
int pinFails = 0;
unsigned long pinLockUntil = 0;

// scenarios
int scenarioStep = 0;
unsigned long scenarioAt = 0;

// ============================ PROTOTYPES ===============================
void raiseEvent(Module m, Sev s, const String& msgIn, bool sim = false);
void logDetail(Module m, Sev s, const String& msgIn);
void evaluate();
void logRaw(const String& payload);
void tg(const String& s);
bool readGwMac(uint8_t* out);

// ============================ SMALL HELPERS ============================
String sha256Hex(const String& in) {
  uint8_t out[32];
  mbedtls_sha256((const unsigned char*)in.c_str(), in.length(), out, 0);
  String hex = "";
  hex.reserve(64);
  char b[3];
  for (int i = 0; i < 32; i++) { snprintf(b, sizeof(b), "%02x", out[i]); hex += b; }
  return hex;
}

// make any text (MQTT topics, SSIDs...) harmless for logs, JSON and HTML
String clean(const String& s, size_t maxLen = 55) {
  String o;
  for (size_t i = 0; i < s.length() && i < maxLen; i++) {
    char c = s[i];
    bool ok = (c >= 32 && c < 127 && c != '"' && c != '\\' && c != '<' && c != '>' && c != '|' && c != ';');
    o += ok ? c : '_';
  }
  return o;
}

String nowStr() {
  struct tm t;
  if (getLocalTime(&t, 0)) { char b[24]; strftime(b, sizeof(b), "%d-%m %H:%M:%S", &t); return String(b); }
  return "t+" + String(millis() / 1000) + "s";
}

String macStr(const uint8_t* m) {
  char b[18];
  snprintf(b, sizeof(b), "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
  return String(b);
}

// ============================ ZERO-TRUST DEVICE REGISTRY ================
String deviceKeyFor(const String& id) {           // looks "id" up in DEVICE_KEYS = "id:key,id2:key2"
  String s(DEVICE_KEYS);
  int i = 0;
  while (i < (int)s.length()) {
    int c = s.indexOf(',', i);
    if (c < 0) c = s.length();
    String pair = s.substring(i, c);
    int cp = pair.indexOf(':');
    if (cp > 0 && pair.substring(0, cp) == id) return pair.substring(cp + 1);
    i = c + 1;
  }
  return "";
}

int touchDevice(const String& id, bool trusted) {
  for (int i = 0; i < devCount; i++) {
    if (id == devs[i].id) { devs[i].lastMs = millis(); devs[i].trusted = trusted; devs[i].msgs++; if (!trusted) devs[i].everBad = true; return i; }
  }
  if (devCount < MAX_DEV) {
    DevSeen& d = devs[devCount];
    strncpy(d.id, id.c_str(), sizeof(d.id) - 1); d.id[sizeof(d.id) - 1] = 0;
    d.lastMs = millis(); d.trusted = trusted; d.everBad = !trusted; d.msgs = 1; d.lastSeq = 0;
    return devCount++;
  }
  return -1;
}

// ============================ MODULE 8: REPLAY PROTECTION ===============
// A device may optionally sign its payload "key|seq|value" with an ever-
// increasing seq. If seq does not grow, the packet is either a captured
// copy being replayed, or a clock/counter fault - either way it is not
// trusted as a fresh reading.
bool replayCheck(int devIdx, uint32_t seq, const String& id) {
  if (devIdx < 0 || seq == 0) return true;            // no sequence sent = legacy device, nothing to check
  if (seq <= devs[devIdx].lastSeq) {
    raiseEvent(M_REPLAY, SEV_HIGH, "Replay suspected: " + id + " reused/old sequence " + String(seq) + " (last " + String(devs[devIdx].lastSeq) + ")");
    return false;
  }
  devs[devIdx].lastSeq = seq;
  return true;
}

// ============================ MODULE 7: PROBE / SCAN DETECTOR ===========
void probeHit(const String& path) {                // called on every unknown web path (404)
  unsigned long now = millis();
  probeT[probeI % 8] = now;
  probeI++;
  int c = 0;
  for (int i = 0; i < 8; i++) if (probeT[i] && now - probeT[i] < 8000) c++;
  logDetail(M_SCAN, SEV_LOW, "Unknown path hit (" + String(c) + "/6): " + path);
  if (c >= 6) {
    raiseEvent(M_SCAN, SEV_HIGH, String(c) + " unknown web paths in 8s - possible scan");
    for (int i = 0; i < 8; i++) probeT[i] = 0;
  }
}

String urlEnc(const String& s) {
  String o;
  char b[4];
  for (size_t i = 0; i < s.length(); i++) {
    uint8_t c = s[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.') o += (char)c;
    else { snprintf(b, sizeof(b), "%%%02X", c); o += b; }
  }
  return o;
}

bool bssidTrusted(const String& b) {
  if (b == trustedBssid) return true;
  String extra = String(TRUSTED_EXTRA_BSSIDS);
  extra.toUpperCase();
  return extra.length() > 0 && extra.indexOf(b) >= 0;
}

// ============================ OUTPUTS ==================================
void relayCoil(bool on) {
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, (on != RELAY_ACTIVE_LOW) ? HIGH : LOW);
}
void isolateLoad() { relayCoil(true); }
void restoreLoad() { relayCoil(false); }

void setLed(bool r, bool g, bool b) {
  if (LED_COMMON_ANODE) { r = !r; g = !g; b = !b; }   // common anode: GPIO LOW lights that colour
  digitalWrite(PIN_LED_R, r); digitalWrite(PIN_LED_G, g); digitalWrite(PIN_LED_B, b);
}
void beep(unsigned ms) { beepUntil = millis() + ms; }

void outputsTick() {
  unsigned long now = millis();
  bool blink = ((now / 300) % 2) == 0;
  if (WiFi.status() != WL_CONNECTED && now > 25000) setLed(false, false, blink);       // blue blink = offline
  else if (level == L_CRITICAL) setLed(isolated ? true : blink, false, false);          // red (solid if isolated)
  else if (level == L_WARNING) setLed(true, true, false);                               // yellow
  else setLed(false, true, false);                                                      // green
  bool buz = false;
  if (!muted) {
    if (now < alarmUntil) buz = ((now / 200) % 2) == 0;
    else if (now < beepUntil) buz = true;
  }
  digitalWrite(PIN_BUZZER, buz ? HIGH : LOW);
  digitalWrite(PIN_ONBOARD, level == L_CRITICAL ? (((now / 120) % 2) == 0) : (((now / 1000) % 2) == 0 && (now % 1000) < 100));
}

// ============================ STORAGE + HASH-CHAINED LOG ===============
// line format:  seq;time;text|prevHash|hash      hash = SHA256(prevHash + "seq;time;text")
void logRaw(const String& payload) {
  String p = clean(payload, 120);
  String line = String(++chainSeq) + ";" + nowStr() + ";" + p;
  String h = sha256Hex(chainHead + line);
  if (fsOK) {
    File f = LittleFS.open(LOG_PATH, FILE_APPEND);
    if (f && f.size() > LOG_MAX_BYTES) {        // rotate: the chain simply continues in the new file
      f.close();
      LittleFS.remove(LOG_OLD);
      LittleFS.rename(LOG_PATH, LOG_OLD);
      f = LittleFS.open(LOG_PATH, FILE_APPEND);
    }
    if (f) { f.println(line + "|" + chainHead + "|" + h); f.close(); }
  }
  chainHead = h;
  prefs.putString("head", chainHead);
  prefs.putUInt("seq", chainSeq);
  Serial.println("[Log] " + line);
}

String verifyChain() {
  if (!fsOK) return "no storage";
  File f = LittleFS.open(LOG_PATH, FILE_READ);
  if (!f) return "no log file yet";
  String prev = "";
  long lastSeq = -1;
  int n = 0;
  while (f.available()) {
    String ln = f.readStringUntil('\n');
    ln.trim();
    if (!ln.length()) continue;
    n++;
    int i2 = ln.lastIndexOf('|');
    int i1 = (i2 > 0) ? ln.lastIndexOf('|', i2 - 1) : -1;
    int sc = ln.indexOf(';');
    if (i1 < 0 || i2 < 0 || sc < 1) { f.close(); return "BROKEN at line " + String(n) + " (malformed)"; }
    String line = ln.substring(0, i1), p = ln.substring(i1 + 1, i2), h = ln.substring(i2 + 1);
    long seq = ln.substring(0, sc).toInt();
    if (n == 1) {
      prev = p;                                                  // first line sets the anchor
      if (seq == 1 && p != "GENESIS") { f.close(); return "BROKEN at line 1 (bad genesis)"; }
    } else if (seq != lastSeq + 1) { f.close(); return "BROKEN at line " + String(n) + " (sequence gap)"; }
    if (p != prev || sha256Hex(prev + line) != h) { f.close(); return "BROKEN at line " + String(n); }
    prev = h;
    lastSeq = seq;
  }
  f.close();
  if (n == 0) return "log empty";
  if (prev != chainHead || (uint32_t)lastSeq != chainSeq) return "BROKEN: log truncated or replaced (head mismatch)";
  return "OK - " + String(n) + " entries verified";
}

// ============================ TELEGRAM =================================
void tg(const String& s) {
  if (strlen(BOT_TOKEN) == 0) return;
  xSemaphoreTake(tgMux, portMAX_DELAY);
  if (tgOutN < 6) tgOut[tgOutN++] = s;
  else { for (int i = 1; i < 6; i++) tgOut[i - 1] = tgOut[i]; tgOut[5] = s; }
  xSemaphoreGive(tgMux);
}

void tgSend(const String& text) {
  WiFiClientSecure c;
  c.setInsecure();                                   // coursework demo: no certificate pinning
  HTTPClient h;
  String url = String("https://api.telegram.org/bot") + BOT_TOKEN + "/sendMessage?chat_id=" + CHAT_ID + "&text=" + urlEnc(text);
  if (h.begin(c, url)) {
    h.setTimeout(6000);
    tgLastCode = h.GET();
    h.end();
  }
}

void tgPoll(long& offset, bool& first) {
  WiFiClientSecure c;
  c.setInsecure();
  HTTPClient h;
  String url = String("https://api.telegram.org/bot") + BOT_TOKEN + "/getUpdates?timeout=0&limit=3&offset=" + String(offset);
  if (!h.begin(c, url)) return;
  h.setTimeout(6000);
  int code = h.GET();
  tgLastCode = code;
  if (code == 200) {
    String body = h.getString();
    int pos = 0;
    while (true) {
      int u = body.indexOf("\"update_id\":", pos);
      if (u < 0) break;
      long id = body.substring(u + 12).toInt();
      if (id >= offset) offset = id + 1;
      int next = body.indexOf("\"update_id\":", u + 10);
      String seg = (next < 0) ? body.substring(u) : body.substring(u, next);
      pos = u + 10;
      if (first) continue;                           // ignore messages that arrived while we were off
      int ci = seg.indexOf("\"chat\":{\"id\":");
      int ti = seg.indexOf("\"text\":\"");
      if (ci < 0 || ti < 0) continue;
      String chat = seg.substring(ci + 13);
      int comma = chat.indexOf(',');
      if (comma > 0) chat = chat.substring(0, comma);
      if (chat != String(CHAT_ID)) continue;         // only our chat may command the device
      String txt = seg.substring(ti + 8);
      int q = txt.indexOf('"');
      if (q > 0) txt = txt.substring(0, q);
      txt.toLowerCase();
      int at = txt.indexOf('@');
      if (at > 0) txt = txt.substring(0, at);
      if (txt == "/status") pendingCmd = 1;
      else if (txt == "/isolate") pendingCmd = 2;
      else if (txt == "/release") pendingCmd = 3;
      else if (txt == "/mute") pendingCmd = 4;
      else if (txt == "/unmute") pendingCmd = 5;
      else if (txt == "/armed") pendingCmd = 6;
      else if (txt == "/monitor") pendingCmd = 7;
      else if (txt == "/help" || txt == "/start") pendingCmd = 8;
    }
    first = false;
  }
  h.end();
}

void tgTask(void*) {
  long offset = 0;
  bool first = true;
  unsigned long lastPoll = 0;
  for (;;) {
    if (WiFi.status() == WL_CONNECTED && strlen(BOT_TOKEN) > 0) {
      String msg = "";
      xSemaphoreTake(tgMux, portMAX_DELAY);
      if (tgOutN > 0) {
        msg = tgOut[0];
        for (int i = 1; i < tgOutN; i++) tgOut[i - 1] = tgOut[i];
        tgOutN--;
      }
      xSemaphoreGive(tgMux);
      if (msg.length()) tgSend(msg);
      else if (millis() - lastPoll > 4000) { lastPoll = millis(); tgPoll(offset, first); }
    }
    vTaskDelay(pdMS_TO_TICKS(400));
  }
}

// ============================ INCIDENTS / ISOLATION ====================
void startIncident(const String& why) {
  Incident& i = incs[incHead];
  i.start = millis();
  i.end = 0;
  String w = clean(why, 55);
  strncpy(i.why, w.c_str(), sizeof(i.why) - 1);
  i.why[sizeof(i.why) - 1] = 0;
  incHead = (incHead + 1) % MAX_INC;
  if (incCount < MAX_INC) incCount++;
}
void endIncident() {
  if (incCount == 0) return;
  Incident& i = incs[(incHead - 1 + MAX_INC) % MAX_INC];
  if (i.end == 0) i.end = millis();
}
void doIsolate(const String& why) {
  if (isolated) return;
  isolated = true;
  isolateLoad();
  alarmUntil = millis() + 8000;
  startIncident(why);
  logRaw("ISOLATION " + why);
}
void doRelease(const String& why) {
  if (!isolated) return;
  isolated = false;
  restoreLoad();
  endIncident();
  logRaw("RELEASE " + why);
}

// ============================ CORRELATION ENGINE =======================
void evaluate() {
  unsigned long now = millis();
  uint16_t mask = 0;
  bool critEvent = false;
  int cnt = 0;
  for (int i = 0; i < alertCount; i++) {
    int idx = (alertHead - 1 - i + MAX_ALERTS) % MAX_ALERTS;
    if (now - alerts[idx].ms <= WINDOW_MS) {
      mask |= (1 << alerts[idx].mod);
      cnt++;
      if (alerts[idx].sev >= SEV_CRIT) critEvent = true;
    }
  }
  int nm = __builtin_popcount(mask);
  Level nl = L_NOMINAL;
  if (cnt > 0) nl = L_WARNING;
  if (critEvent || nm >= 2) nl = L_CRITICAL;           // RULE: high-confidence event OR 2+ modules agree
  if (mode == MODE_LOCKDOWN) nl = L_CRITICAL;
  if (isolated) nl = L_CRITICAL;

  String reason = "";
  for (int m = 0; m < M_COUNT; m++)
    if (mask & (1 << m)) { if (reason.length()) reason += " + "; reason += MOD_NAME[m]; }
  reason = (nm >= 2 ? "Correlated: " : "High-confidence: ") + reason;

  if (nl == L_CRITICAL && !isolated && mode == MODE_ARMED) {
    doIsolate(reason);
    tg("[ARGUS] CRITICAL - " + reason + ". Protected device ISOLATED.");
  }
  if (nl == level) return;
  Level old = level;
  level = nl;
  if (nl == L_CRITICAL && !isolated) {                  // monitor-only mode: alert but do not cut the load
    alarmUntil = now + 8000;
    logRaw("CRITICAL (monitor-only, not isolating) " + reason);
    tg("[ARGUS] CRITICAL (monitor-only mode, relay not used) - " + reason);
  } else if (nl == L_WARNING && old == L_NOMINAL) {
    beep(200);
    if (now - lastWarnTg > 20000) { lastWarnTg = now; tg("[ARGUS] Warning: " + lastMsg); }
  } else if (nl == L_NOMINAL) {
    logRaw("Status back to NOMINAL");
  }
}

void raiseEvent(Module m, Sev s, const String& msgIn, bool sim) {
  unsigned long now = millis();
  if (moduleOff[m]) return;
  if (!sim && s < SEV_CRIT && moduleSeen[m] && now - moduleLast[m] < DEBOUNCE_MS) return;
  moduleSeen[m] = true;
  moduleLast[m] = now;
  moduleCnt[m]++;
  lastEventMs = now;
  String msg = clean(msgIn);
  lastMsg = msg;
  Alert& a = alerts[alertHead];
  a.ms = now; a.mod = m; a.sev = s;
  strncpy(a.msg, msg.c_str(), sizeof(a.msg) - 1);
  a.msg[sizeof(a.msg) - 1] = 0;
  alertHead = (alertHead + 1) % MAX_ALERTS;
  if (alertCount < MAX_ALERTS) alertCount++;
  logRaw(String("[") + MOD_NAME[m] + "] sev=" + String((int)s) + " " + msg);
  evaluate();
}

// Records one raw, individual contributing event (e.g. "this one wrong PIN",
// "this one unknown URL") so it is visible on its own in the alert feed,
// timeline and forensic log - without the per-module rate limit that
// raiseEvent() uses. Because correlation (evaluate()) escalates to CRITICAL
// only when 2+ DIFFERENT modules agree, a burst of detail lines from the
// same module never triggers isolation by itself - only the module's own
// raiseEvent() call does that, once a real threshold is crossed. These
// detail lines are the evidence trail leading up to that verdict, and they
// do nudge the displayed threat score up while they are recent, which is
// the point: the dashboard should visibly climb as a burst is under way.
void logDetail(Module m, Sev s, const String& msgIn) {
  if (moduleOff[m]) return;
  unsigned long now = millis();
  moduleCnt[m]++;
  String msg = clean(msgIn);
  Alert& a = alerts[alertHead];
  a.ms = now; a.mod = m; a.sev = s;
  strncpy(a.msg, msg.c_str(), sizeof(a.msg) - 1);
  a.msg[sizeof(a.msg) - 1] = 0;
  alertHead = (alertHead + 1) % MAX_ALERTS;
  if (alertCount < MAX_ALERTS) alertCount++;
  logRaw(String("[") + MOD_NAME[m] + "] detail " + msg);
}

int threatScore() {
  unsigned long now = millis();
  float sc = 0;
  for (int i = 0; i < alertCount; i++) {
    int idx = (alertHead - 1 - i + MAX_ALERTS) % MAX_ALERTS;
    float age = (now - alerts[idx].ms) / 1000.0f;
    if (age > 120) continue;
    static const float W[5] = {0, 6, 12, 22, 40};
    sc += W[alerts[idx].sev] * (1.0f - age / 120.0f);
  }
  if (sc > 100) sc = 100;
  if (isolated && sc < 85) sc = 85;
  if (level == L_CRITICAL && sc < 70) sc = 70;
  return (int)sc;
}

void healTick() {
  if (!isolated || mode != MODE_ARMED) return;
  unsigned long now = millis();
  if (now - lastEventMs < QUIET_MS || now - lastHealTry < 5000) return;
  lastHealTry = now;
  bool fwGood = fwOK && (String(prefs.getString("fw", "")) == fwNow);
  bool gwGood = true;
  uint8_t m[6];
  if (gwKnown && readGwMac(m)) gwGood = (memcmp(m, gwMac, 6) == 0);
  bool linkGood = (WiFi.status() == WL_CONNECTED);
  if (fwGood && gwGood && linkGood) {
    doRelease("self-heal: firmware, gateway MAC and Wi-Fi link re-verified");
    tg("[ARGUS] Self-heal: re-verification passed, protected device restored.");
    evaluate();
  } else {
    Serial.println("[Self-heal] checks failed, staying isolated");
  }
}

// ============================ MODULE 3: FIRMWARE =======================
String firmwareHash() {
  const esp_partition_t* p = esp_ota_get_running_partition();
  if (!p) return sha256Hex(ESP.getSketchMD5());
  size_t total = ESP.getSketchSize();
  if (total == 0 || total > p->size) total = p->size;
  static uint8_t buf[4096];
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts(&ctx, 0);
  size_t off = 0;
  while (off < total) {
    size_t n = total - off;
    if (n > sizeof(buf)) n = sizeof(buf);
    if (esp_partition_read(p, off, buf, n) != ESP_OK) break;
    mbedtls_sha256_update(&ctx, buf, n);
    off += n;
  }
  uint8_t out[32];
  mbedtls_sha256_finish(&ctx, out);
  mbedtls_sha256_free(&ctx);
  String hex = "";
  char b[3];
  for (int i = 0; i < 32; i++) { snprintf(b, sizeof(b), "%02x", out[i]); hex += b; }
  return hex;
}

void checkFirmwareAtBoot() {
  fwNow = firmwareHash();
  String base = prefs.getString("fw", "");
  if (base == "") {
    prefs.putString("fw", fwNow);
    fwOK = true;
    logRaw("FW baseline stored " + fwNow.substring(0, 16));
  } else if (base != fwNow) {
    fwOK = false;
    raiseEvent(M_FW, SEV_CRIT, "Firmware image CHANGED - tamper suspected");
  } else {
    fwOK = true;
    logRaw("FW verified " + fwNow.substring(0, 16));
  }
}

// ============================ MODULE 10: UNEXPECTED REBOOT =============
void checkResetReasonAtBoot() {
  esp_reset_reason_t r = esp_reset_reason();
  bool abnormal = (r == ESP_RST_PANIC || r == ESP_RST_INT_WDT || r == ESP_RST_TASK_WDT ||
                    r == ESP_RST_WDT || r == ESP_RST_BROWNOUT);
  const char* name = (r == ESP_RST_PANIC) ? "panic/crash" :
                      (r == ESP_RST_INT_WDT || r == ESP_RST_TASK_WDT || r == ESP_RST_WDT) ? "watchdog timeout" :
                      (r == ESP_RST_BROWNOUT) ? "brownout (power glitch)" :
                      (r == ESP_RST_POWERON) ? "normal power-on" :
                      (r == ESP_RST_SW) ? "software/operator reset" : "other";
  logRaw(String("Reset reason: ") + name);
  if (abnormal) raiseEvent(M_REBOOT, SEV_HIGH, String("Unexpected reset (") + name + ") - possible crash or fault attack");
}

// ============================ MODULE 9: MEMORY / DoS ====================
void heapTick() {
  uint32_t h = ESP.getFreeHeap();
  if (heapHigh == 0 || h > heapHigh) { heapHigh = h; return; }        // still warming up / new high-water mark
  unsigned long now = millis();
  bool low = (h < 15000) || (heapHigh > 40000 && h < heapHigh / 2);
  if (low && now - lastHeapAlert > 15000) {
    lastHeapAlert = now;
    raiseEvent(M_DOS, SEV_HIGH, "Free heap collapsed to " + String(h) + " bytes (high-water " + String(heapHigh) + ") - possible memory exhaustion");
  }
}

// ============================ MODULE 4: SENSOR =========================
void histPush(float t, float h, int rssi) {
  hT[histHead] = isnan(t) ? -9999 : (int16_t)lroundf(t * 10);
  hH[histHead] = isnan(h) ? -9999 : (int16_t)lroundf(h * 10);
  hR[histHead] = (int16_t)rssi;
  histHead = (histHead + 1) % HIST_N;
  if (histN < HIST_N) histN++;
}

void sensorTick() {
  float t = dht.getTemperature();
  float h = dht.getHumidity();
  int rssi = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : -100;
  histPush(t, h, rssi);
  if (isnan(t) || isnan(h)) {
    if (++dhtFail >= 3) { raiseEvent(M_SENS, SEV_MED, "Sensor not responding (disconnected?)"); dhtFail = 0; }
    return;
  }
  dhtFail = 0;
  if (t < -10 || t > 60 || h < 0 || h > 100) raiseEvent(M_SENS, SEV_MED, "Implausible sensor value");
  if (!isnan(lastT) && fabsf(t - lastT) > 5.0f) raiseEvent(M_SENS, SEV_MED, "Temperature jumped over 5C in 5s");
  if (!isnan(lastH) && fabsf(h - lastH) > 25.0f) raiseEvent(M_SENS, SEV_MED, "Humidity jumped over 25% in 5s");
  lastT = t;
  lastH = h;
  if (mqtt.connected()) {
    String key = deviceKeyFor("sensor1");                 // this node signs its own readings with its device key
    selfSeq++;                                            // ever-increasing sequence, so a captured packet cannot be replayed
    lastSelfTemp = key + "|" + String(selfSeq) + "|" + String(t, 1);
    lastSelfHum = key + "|" + String(selfSeq) + "|" + String(h, 1);
    lastSelfPubMs = millis();
    mqtt.publish(TOPIC_TEMP, lastSelfTemp.c_str());
    mqtt.publish(TOPIC_HUM, lastSelfHum.c_str());
  }
}

// ============================ MODULE 2: MQTT POLICY (zero-trust) =======
// Devices on argus/dev/<id>/... must prefix every payload with "<key>|" where
// <key> is their entry in DEVICE_KEYS. No key, wrong key, or an id that is
// not in DEVICE_KEYS at all is treated as an unauthenticated / spoofed device.
// An optional middle field "<key>|<seq>|<value>" is checked for replay (module 8).
void onMqtt(char* topic, byte* payload, unsigned int len) {
  unsigned long now = millis();
  String t(topic), p;
  for (unsigned i = 0; i < len && i < 40; i++) p += (char)payload[i];
  bool isTemp = (t == TOPIC_TEMP), isHum = (t == TOPIC_HUM);
  if ((isTemp || isHum) && now - lastSelfPubMs < 800 && p == (isTemp ? lastSelfTemp : lastSelfHum)) return;   // our own echo
  mqttCount++;
  if (now < mqttArmedAt) return;                                    // ignore retained msgs right after connect

  if (len > 220) {                                                  // close to the 256-byte MQTT buffer - overflow probe
    raiseEvent(M_MQTT, SEV_HIGH, "Oversized MQTT payload (" + String(len) + " bytes) on " + t);
    return;
  }

  if (t.startsWith("argus/dev/")) {
    int a = t.indexOf('/', 10);
    String id = (a > 0) ? t.substring(10, a) : t.substring(10);
    String want = deviceKeyFor(id);
    int bar = p.indexOf('|');
    String gotKey = (bar > 0) ? p.substring(0, bar) : "";
    String rest = (bar > 0) ? p.substring(bar + 1) : p;
    uint32_t seq = 0;
    String val = rest;
    int bar2 = rest.indexOf('|');
    if (bar2 > 0) {
      String seqStr = rest.substring(0, bar2);
      bool numeric = seqStr.length() > 0;
      for (size_t k = 0; k < seqStr.length() && numeric; k++) if (!isdigit((unsigned char)seqStr[k])) numeric = false;
      if (numeric) { seq = (uint32_t)seqStr.toInt(); val = rest.substring(bar2 + 1); }
    }

    if (want == "") {
      touchDevice(id, false);
      raiseEvent(M_MQTT, SEV_MED, "Unknown device id: " + id);
    } else if (gotKey != want) {
      touchDevice(id, false);
      raiseEvent(M_MQTT, SEV_HIGH, "Zero-trust: " + id + " sent wrong or missing device key");
    } else if (id == "sensor1") {
      int di = touchDevice(id, true);
      if (!replayCheck(di, seq, id)) return;
      if (isTemp || isHum) {                                        // only this node may publish here
        float v = val.toFloat();
        float ref = isTemp ? lastT : lastH;
        float lim = isTemp ? 3.0f : 15.0f;
        if (!isnan(ref) && fabsf(v - ref) > lim) raiseEvent(M_SENS, SEV_HIGH, "Fake sensor value injected: " + val);
        else raiseEvent(M_SENS, SEV_MED, "Foreign publisher on sensor topic");
      } else raiseEvent(M_MQTT, SEV_MED, "sensor1 used forbidden topic " + t);
    } else {
      int di = touchDevice(id, true);
      replayCheck(di, seq, id);                                      // authenticated device, no extra rule defined
    }
  } else {
    raiseEvent(M_MQTT, SEV_MED, "Unauthorised topic: " + t);
  }
}

void mqttRateTick() {
  static unsigned long last = 0;
  if (millis() - last >= 1000) {
    last = millis();
    int c = mqttCount;
    mqttCount = 0;
    if (c > FLOOD_PER_SEC) {
      logDetail(M_MQTT, SEV_LOW, String(c) + " MQTT messages in the last second (limit " + String(FLOOD_PER_SEC) + ")");
      raiseEvent(M_MQTT, SEV_HIGH, "MQTT flood " + String(c) + " msg/s");
    }
  }
}

void mqttEnsure() {
  if (strlen(MQTT_HOST) == 0 || WiFi.status() != WL_CONNECTED || mqtt.connected()) return;
  if (millis() - lastMqttTry < 5000) return;
  lastMqttTry = millis();
  if (mqtt.connect("argus-monitor")) { mqtt.subscribe("#"); mqttArmedAt = millis() + 3000; Serial.println("[MQTT] connected"); }
  else Serial.println("[MQTT] connect failed (is Mosquitto running? port 1883 open?)");
}

// ============================ MODULE 1: ARP / MITM =====================
bool readGwMac(uint8_t* out) {
#if USE_ARP_CHECK
  if (!netif_default) return false;
  ip4_addr_t gw;
  gw.addr = (uint32_t)WiFi.gatewayIP();
  struct eth_addr* e = nullptr;
  const ip4_addr_t* ipRet = nullptr;
  if (etharp_find_addr(netif_default, &gw, &e, &ipRet) >= 0 && e) { memcpy(out, e->addr, 6); return true; }
#endif
  return false;
}

void kickGateway() {                     // makes lwIP (re)resolve the gateway so the cache entry exists
  udp.beginPacket(WiFi.gatewayIP(), 9);
  udp.write((uint8_t)0);
  udp.endPacket();
}

void learnGateway() {
  gwKnown = false;
#if USE_ARP_CHECK
  kickGateway();
  for (int i = 0; i < 30 && !gwKnown; i++) {
    delay(100);
    uint8_t m[6];
    if (readGwMac(m)) { memcpy(gwMac, m, 6); gwKnown = true; }
  }
#endif
  if (!gwKnown) moduleOff[M_ARP] = !USE_ARP_CHECK;
  logRaw(gwKnown ? String("Gateway MAC baseline ") + macStr(gwMac) : String("Gateway MAC baseline NOT learned (ARP module idle)"));
}

void arpTick() {
#if USE_ARP_CHECK
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastKick > 20000) { lastKick = millis(); kickGateway(); }
  uint8_t m[6];
  if (!readGwMac(m)) return;
  if (!gwKnown) { memcpy(gwMac, m, 6); gwKnown = true; return; }
  if (memcmp(m, gwMac, 6) != 0) raiseEvent(M_ARP, SEV_HIGH, "ARP spoof: gateway MAC now " + macStr(m));
#endif
}

// ============================ MODULE 0: ROGUE AP =======================
void apTick() {
  if (WiFi.status() != WL_CONNECTED || trustedBssid == "") return;
  if (!scanning && millis() - lastScan > AP_SCAN_EVERY) { WiFi.scanNetworks(true, false); scanning = true; lastScan = millis(); }
  if (!scanning) return;
  int n = WiFi.scanComplete();
  if (n >= 0) {
    scoutN = 0;
    for (int i = 0; i < n; i++) {
      if (WiFi.SSID(i) == WIFI_SSID && !bssidTrusted(WiFi.BSSIDstr(i))) {
        bool open = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
        raiseEvent(M_AP, SEV_HIGH, String(open ? "Open evil twin " : "Evil twin ") + WiFi.BSSIDstr(i) + " ch" + String(WiFi.channel(i)) + " " + String(WiFi.RSSI(i)) + "dBm");
      }
      if (scoutN < MAX_SCOUT) {                                      // "scout" snapshot: every nearby network, for visibility
        ApSeen& a = scout[scoutN++];
        strncpy(a.ssid, clean(WiFi.SSID(i), 22).c_str(), sizeof(a.ssid) - 1); a.ssid[sizeof(a.ssid) - 1] = 0;
        strncpy(a.bssid, WiFi.BSSIDstr(i).c_str(), sizeof(a.bssid) - 1); a.bssid[sizeof(a.bssid) - 1] = 0;
        a.rssi = (int8_t)WiFi.RSSI(i);
        a.ch = (uint8_t)WiFi.channel(i);
        a.open = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
        a.self = (WiFi.SSID(i) == WIFI_SSID) && bssidTrusted(WiFi.BSSIDstr(i));
      }
    }
    scoutAt = millis();

    // ---- MODULE 12: SSID FLOOD / JAMMING SIGNATURE ----
    // A sudden, large jump in the TOTAL number of visible networks (not just
    // copies of our own SSID) looks like someone flooding the air with fake
    // beacons, or jamming - either way, not normal Wi-Fi background noise.
    if (apBaseline < 0) {
      apBaseline = n;                                                 // first scan: learn the starting point
    } else {
      if (n >= AP_FLOOD_MIN && (n - apBaseline) >= AP_FLOOD_JUMP) {
        raiseEvent(M_JAM, SEV_HIGH, String(n) + " networks visible, up from a normal " + String((int)apBaseline) + " - possible Wi-Fi flood/jamming");
      }
      apBaseline = (apBaseline * 3.0f + n) / 4.0f;                     // slow-moving average so normal variation doesn't drift it too fast
    }

    WiFi.scanDelete();
    scanning = false;
  } else if (n == -2) scanning = false;
}

// ============================ MODULE 5: WI-FI LINK =====================
void onWifiEvent(arduino_event_id_t event, arduino_event_info_t info) {
  if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
    discT[discI % 6] = millis();
    discI = (discI + 1) % 6;
    lastReason = info.wifi_sta_disconnected.reason;
    logDetail(M_LINK, SEV_LOW, "Wi-Fi disconnected (reason " + String((int)lastReason) + ")");
  }
}

void linkTick() {
  unsigned long now = millis();
  int recent = 0;
  for (int i = 0; i < 6; i++) if (discT[i] != 0 && now - discT[i] < 60000) recent++;
  if (recent >= 3) {
    raiseEvent(M_LINK, SEV_HIGH, "Repeated Wi-Fi drops (" + String(recent) + "/min, reason " + String((int)lastReason) + ") - deauth or jamming?");
    for (int i = 0; i < 6; i++) discT[i] = 0;
  }
  if (WiFi.status() == WL_CONNECTED) {
    wifiLostAt = 0;
    int r = WiFi.RSSI();
    if (rssiN < 12) { rssiEma = (rssiN == 0) ? r : rssiEma * 0.8f + r * 0.2f; rssiN++; }
    else if (r < rssiEma - 18) {
      if (++rssiBad >= 3) { raiseEvent(M_LINK, SEV_MED, "Wi-Fi signal dropped " + String((int)(rssiEma - r)) + " dB"); rssiBad = 0; }
    } else { rssiBad = 0; rssiEma = rssiEma * 0.95f + r * 0.05f; }
  } else {
    if (wifiLostAt == 0) wifiLostAt = now;
    if (now - wifiLostAt > 10000) { wifiLostAt = now; WiFi.reconnect(); }
  }
}

// ============================ MODULE 6: TAMPER WIRE ====================
void learnTamper() {
#if USE_TAMPER
  float s = 0;
  for (int i = 0; i < 10; i++) { s += touchRead(PIN_TAMPER); delay(20); }
  touchBase = s / 10;
  if (touchBase < 8) { moduleOff[M_TAMPER] = true; Serial.println("[Tamper] pad reads too low, module disabled"); }
  else Serial.printf("[Tamper] baseline %.0f\n", touchBase);
#else
  moduleOff[M_TAMPER] = true;
#endif
}

void tamperTick() {
#if USE_TAMPER
  if (moduleOff[M_TAMPER] || touchBase <= 0) return;
  float v = touchRead(PIN_TAMPER);
  if (v < touchBase * 0.6f) {
    if (++touchHits >= 3) { raiseEvent(M_TAMPER, SEV_HIGH, "Tamper wire touched (case opened?)"); touchHits = 0; }
  } else touchHits = 0;
#endif
}

// ============================ BUTTON (BOOT) ============================
void buttonTick() {
  static unsigned long down = 0;
  bool pressed = (digitalRead(PIN_BOOT) == LOW);
  if (pressed && down == 0) down = millis();
  if (!pressed && down != 0) {
    unsigned long d = millis() - down;
    down = 0;
    if (d > 3000) { doRelease("manual release (BOOT button)"); evaluate(); }
    else if (d > 40) { alarmUntil = 0; beepUntil = 0; Serial.println("[Button] alarm silenced"); }
  }
}

// ============================ SELF-TEST / SIM ==========================
String hardwareSelfTest() {
  bool wasIso = isolated;
  setLed(true, false, false); delay(250);
  setLed(false, true, false); delay(250);
  setLed(false, false, true); delay(250);
  digitalWrite(PIN_BUZZER, muted ? LOW : HIGH); delay(150); digitalWrite(PIN_BUZZER, LOW);
  float t = dht.getTemperature();
  String r = "LED ok, buzzer " + String(muted ? "muted" : "ok") + ", DHT " + (isnan(t) ? String("NOT READING") : String(t, 0) + "C");
  r += ", Wi-Fi " + String(WiFi.status() == WL_CONNECTED ? "up" : "DOWN");
  r += ", MQTT " + String(mqtt.connected() ? "up" : "down");
  r += ", storage " + String(fsOK ? "ok" : "FAIL");
  if (!wasIso) { relayCoil(true); delay(400); relayCoil(false); r += ", relay clicked"; }
  logRaw("Self-test: " + r);
  return r;
}

void simulate(int m) {
  if (m < 0 || m >= M_COUNT) return;
  Sev s = (m == M_AP || m == M_ARP || m == M_FW || m == M_LINK || m == M_TAMPER) ? SEV_HIGH : SEV_MED;
  raiseEvent((Module)m, s, String("[SIM] ") + MOD_NAME[m] + " test event", true);
}

void startScenario(int n) {
  if (n == 1) simulate(M_AP);
  else if (n == 2) { simulate(M_ARP); scenarioStep = 1; scenarioAt = millis() + 4000; }
}

void rebase() {
  learnGateway();
  learnTamper();
  rssiN = 0; rssiBad = 0;
  trustedBssid = (WiFi.status() == WL_CONNECTED) ? WiFi.BSSIDstr() : trustedBssid;
  logRaw("Baselines re-learned by operator");
}

void clearAlerts() {
  alertCount = 0; alertHead = 0;
  for (int i = 0; i < M_COUNT; i++) { moduleSeen[i] = false; moduleCnt[i] = 0; }
  level = isolated ? L_CRITICAL : L_NOMINAL;
  logRaw("Alert list cleared by operator");
}

// ============================ COMMANDS (web / telegram / serial) =======
String statusText() {
  return String("[ARGUS] ") + (level == L_NOMINAL ? "NOMINAL" : level == L_WARNING ? "WARNING" : "CRITICAL") +
         (isolated ? " (device isolated)" : "") + " | mode " + (mode == MODE_ARMED ? "armed" : mode == MODE_MONITOR ? "monitor" : "lockdown") +
         " | score " + String(threatScore()) + " | temp " + (isnan(lastT) ? String("--") : String(lastT, 0) + "C") +
         " | RSSI " + String(WiFi.RSSI()) + " | dashboard http://" + WiFi.localIP().toString();
}

void setMode(int m) {
  if (m < 0 || m > 2) return;
  mode = (Mode)m;
  prefs.putUChar("mode", (uint8_t)m);
  logRaw(String("Mode set to ") + (m == 0 ? "ARMED" : m == 1 ? "MONITOR-ONLY" : "LOCKDOWN"));
  if (mode == MODE_LOCKDOWN) doIsolate("manual LOCKDOWN");
  else if (isolated && mode == MODE_MONITOR) doRelease("mode changed to monitor-only");
  evaluate();
}

void handlePending() {
  int c = pendingCmd;
  if (!c) return;
  pendingCmd = 0;
  switch (c) {
    case 1: tg(statusText()); break;
    case 2: doIsolate("manual isolate (Telegram)"); evaluate(); tg("[ARGUS] Device isolated by command."); break;
    case 3: doRelease("manual release (Telegram)"); evaluate(); tg("[ARGUS] Device released by command."); break;
    case 4: muted = true; tg("[ARGUS] Buzzer muted."); break;
    case 5: muted = false; tg("[ARGUS] Buzzer unmuted."); break;
    case 6: setMode(0); tg("[ARGUS] Mode: ARMED (auto-isolate)."); break;
    case 7: setMode(1); tg("[ARGUS] Mode: MONITOR ONLY."); break;
    case 8: tg("[ARGUS] Commands: /status /isolate /release /mute /unmute /armed /monitor /help"); break;
  }
}

// ============================ WEB DASHBOARD ============================
#include "dashboard_html.h"

String statusJson() {
  unsigned long now = millis();
  String j;
  j.reserve(6000);
  j += "{\"v\":3,\"ver\":\"" ARGUS_VERSION "\",\"level\":" + String((int)level) + ",\"mode\":" + String((int)mode);
  j += ",\"iso\":" + String(isolated ? 1 : 0) + ",\"muted\":" + String(muted ? 1 : 0) + ",\"score\":" + String(threatScore());
  j += ",\"up\":" + String(now / 1000) + ",\"clock\":\"" + nowStr() + "\"";
  j += ",\"ip\":\"" + WiFi.localIP().toString() + "\",\"ssid\":\"" + clean(WiFi.SSID()) + "\",\"bssid\":\"" + WiFi.BSSIDstr() + "\"";
  j += ",\"rssi\":" + String(WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : -100);
  j += ",\"gw\":\"" + String(gwKnown ? macStr(gwMac) : String("n/a")) + "\"";
  j += ",\"fw\":\"" + fwNow.substring(0, 12) + "\",\"fwok\":" + String(fwOK ? 1 : 0);
  j += ",\"head\":\"" + chainHead.substring(0, 12) + "\",\"seq\":" + String(chainSeq);
  j += ",\"heap\":" + String(ESP.getFreeHeap());
  j += ",\"free\":" + String(fsOK ? (unsigned long)(LittleFS.totalBytes() - LittleFS.usedBytes()) : 0UL);
  j += ",\"mqtt\":" + String(mqtt.connected() ? 1 : 0) + ",\"tg\":" + String(strlen(BOT_TOKEN) > 0 ? 1 : 0);
  j += ",\"boots\":" + String(bootCount);
  j += ",\"temp\":" + (isnan(lastT) ? String("null") : String(lastT, 1));
  j += ",\"hum\":" + (isnan(lastH) ? String("null") : String(lastH, 1));
  j += ",\"mods\":[";
  for (int m = 0; m < M_COUNT; m++) {
    if (m) j += ",";
    j += "{\"age\":" + (moduleSeen[m] ? String((now - moduleLast[m]) / 1000) : String("-1")) + ",\"cnt\":" + String(moduleCnt[m]) + ",\"off\":" + String(moduleOff[m] ? "true" : "false") + "}";
  }
  j += "],\"alerts\":[";
  int shown = 0;
  for (int k = 0; k < alertCount && shown < 40; k++) {
    int idx = (alertHead - 1 - k + MAX_ALERTS) % MAX_ALERTS;
    if (shown++) j += ",";
    j += "{\"ago\":" + String((now - alerts[idx].ms) / 1000) + ",\"m\":" + String(alerts[idx].mod) + ",\"s\":" + String(alerts[idx].sev) + ",\"x\":\"" + alerts[idx].msg + "\"}";
  }
  j += "],\"inc\":[";
  for (int k = 0; k < incCount; k++) {
    int idx = (incHead - 1 - k + MAX_INC) % MAX_INC;
    if (k) j += ",";
    long d = incs[idx].end ? (long)((incs[idx].end - incs[idx].start) / 1000) : -1;
    j += "{\"ago\":" + String((now - incs[idx].start) / 1000) + ",\"dur\":" + String(d) + ",\"x\":\"" + incs[idx].why + "\"}";
  }
  j += "]}";
  return j;
}

String historyJson() {
  String j = "{\"t\":[";
  for (int pass = 0; pass < 3; pass++) {
    int16_t* a = pass == 0 ? hT : pass == 1 ? hH : hR;
    for (int i = 0; i < histN; i++) {
      int idx = (histHead - histN + i + HIST_N) % HIST_N;
      if (i) j += ",";
      j += String((int)a[idx]);
    }
    if (pass == 0) j += "],\"h\":[";
    else if (pass == 1) j += "],\"r\":[";
  }
  j += "]}";
  return j;
}

String devicesJson() {
  unsigned long now = millis();
  String j = "{\"devices\":[";
  for (int i = 0; i < devCount; i++) {
    if (i) j += ",";
    j += "{\"id\":\"" + String(devs[i].id) + "\",\"ago\":" + String((now - devs[i].lastMs) / 1000) +
         ",\"trusted\":" + String(devs[i].trusted ? 1 : 0) + ",\"everBad\":" + String(devs[i].everBad ? 1 : 0) +
         ",\"msgs\":" + String(devs[i].msgs) + ",\"seq\":" + String(devs[i].lastSeq) + "}";
  }
  j += "],\"known\":\"" + clean(String(DEVICE_KEYS), 120) + "\"}";
  return j;
}

String scoutJson() {
  unsigned long now = millis();
  String j = "{\"age\":" + String(scoutAt ? (now - scoutAt) / 1000 : -1) + ",\"nets\":[";
  for (int i = 0; i < scoutN; i++) {
    if (i) j += ",";
    j += "{\"ssid\":\"" + String(scout[i].ssid) + "\",\"bssid\":\"" + String(scout[i].bssid) + "\",\"rssi\":" +
         String(scout[i].rssi) + ",\"ch\":" + String(scout[i].ch) + ",\"open\":" + String(scout[i].open ? 1 : 0) +
         ",\"self\":" + String(scout[i].self ? 1 : 0) + "}";
  }
  j += "]}";
  return j;
}

String logTailJson(int n) {
  if (n < 1) n = 1;
  if (n > 40) n = 40;
  String lines[40];
  int count = 0, head = 0;
  if (fsOK) {
    File f = LittleFS.open(LOG_PATH, FILE_READ);
    if (f) {
      while (f.available()) {
        String ln = f.readStringUntil('\n');
        ln.trim();
        if (!ln.length()) continue;
        lines[head] = ln;
        head = (head + 1) % n;
        if (count < n) count++;
      }
      f.close();
    }
  }
  String j = "{\"lines\":[";
  for (int i = 0; i < count; i++) {
    int idx = (head - 1 - i + n * 2) % n;     // newest first
    if (i) j += ",";
    j += "\"" + lines[idx] + "\"";
  }
  j += "]}";
  return j;
}

bool authed() {
  unsigned long now = millis();
  if (now < pinLockUntil) { server.send(429, "application/json", "{\"ok\":false,\"msg\":\"locked\"}"); return false; }
  if (server.header("X-Pin") == String(ADMIN_PIN)) { pinFails = 0; return true; }
  pinFails++;
  logDetail(M_SCAN, SEV_LOW, "Wrong PIN attempt (" + String(pinFails) + "/5) from " + server.client().remoteIP().toString());
  if (pinFails >= 5) { pinLockUntil = now + 60000; pinFails = 0; raiseEvent(M_SCAN, SEV_HIGH, "Dashboard brute-force: 5 wrong PIN attempts"); }
  server.send(401, "application/json", "{\"ok\":false,\"msg\":\"bad pin\"}");
  return false;
}

void sendCmd(bool ok, const String& msg) {
  server.send(200, "application/json", String("{\"ok\":") + (ok ? "true" : "false") + ",\"msg\":\"" + clean(msg, 150) + "\"}");
}

// ============================ MODULE 11: SESSION / CONTROL HIJACK ======
void sessionCheck() {
  String ip = server.client().remoteIP().toString();
  unsigned long now = millis();
  if (lastCmdIp != "" && ip != lastCmdIp && now - lastCmdMs < 120000) {
    raiseEvent(M_SESSION, SEV_HIGH, "Dashboard commanded from a new IP (" + ip + ") within 2 min of " + lastCmdIp + " - possible session hijack");
  }
  lastCmdIp = ip;
  lastCmdMs = now;
}

void handleCmd() {
  if (!authed()) return;
  sessionCheck();
  String c = server.arg("c");
  int v = server.arg("v").toInt();
  if (c == "isolate") { doIsolate("manual isolate (dashboard)"); evaluate(); sendCmd(true, "Protected device isolated"); }
  else if (c == "release") { doRelease("manual release (dashboard)"); lastEventMs = millis(); evaluate(); sendCmd(true, "Protected device restored"); }
  else if (c == "mute") { muted = (v != 0); sendCmd(true, muted ? "Alarm muted" : "Alarm unmuted"); }
  else if (c == "mode") { setMode(v); sendCmd(true, "Mode changed"); }
  else if (c == "sim") { simulate(v); sendCmd(true, "Simulated event injected"); }
  else if (c == "scenario") { startScenario(v); sendCmd(true, v == 1 ? "Warning demo started" : "Critical demo started"); }
  else if (c == "clear") { clearAlerts(); sendCmd(true, "Alerts cleared"); }
  else if (c == "rebase") { rebase(); sendCmd(true, "Baselines re-learned"); }
  else if (c == "trust") { fwNow = firmwareHash(); prefs.putString("fw", fwNow); fwOK = true; logRaw("FW baseline re-trusted by operator " + fwNow.substring(0, 16)); sendCmd(true, "Firmware trusted"); }
  else if (c == "relaytest") { if (isolated) sendCmd(false, "Already isolated"); else { relayCoil(true); delay(500); relayCoil(false); sendCmd(true, "Relay clicked"); } }
  else if (c == "selftest") { sendCmd(true, hardwareSelfTest()); }
  else sendCmd(false, "Unknown command");
}

void setupWeb() {
  const char* hk[] = {"X-Pin"};
  server.collectHeaders(hk, 1);
  server.on("/", []() { server.send_P(200, "text/html", PAGE); });
  server.on("/api/status", []() { server.sendHeader("Cache-Control", "no-store"); server.send(200, "application/json", statusJson()); });
  server.on("/api/history", []() { server.sendHeader("Cache-Control", "no-store"); server.send(200, "application/json", historyJson()); });
  server.on("/api/log", []() { server.send(200, "application/json", logTailJson(server.arg("n").toInt())); });
  server.on("/api/log.txt", []() {
    if (!fsOK) { server.send(404, "text/plain", "no storage"); return; }
    File f = LittleFS.open(LOG_PATH, FILE_READ);
    if (!f) { server.send(404, "text/plain", "no log"); return; }
    server.streamFile(f, "text/plain");
    f.close();
  });
  server.on("/api/verify", []() { server.send(200, "application/json", "{\"r\":\"" + verifyChain() + "\"}"); });
  server.on("/api/devices", []() { server.send(200, "application/json", devicesJson()); });
  server.on("/api/scan", []() { server.send(200, "application/json", scoutJson()); });
  server.on("/api/cmd", handleCmd);
  server.onNotFound([]() { probeHit(server.uri()); server.send(404, "text/plain", "not found"); });
  server.begin();
}

// ============================ SERIAL COMMANDS ==========================
void serialTick() {
  if (!Serial.available()) return;
  String c = Serial.readStringUntil('\n');
  c.trim();
  c.toLowerCase();
  if (c == "status") { Serial.println(statusText()); }
  else if (c == "verify") { Serial.println("Chain: " + verifyChain()); }
  else if (c == "isolate") { doIsolate("manual isolate (serial)"); evaluate(); }
  else if (c == "release") { doRelease("manual release (serial)"); lastEventMs = millis(); evaluate(); }
  else if (c == "trust") { fwNow = firmwareHash(); prefs.putString("fw", fwNow); fwOK = true; logRaw("FW baseline re-trusted by operator"); Serial.println("Firmware trusted."); }
  else if (c == "rebase") { rebase(); }
  else if (c == "wipe") {
    if (fsOK) { LittleFS.remove(LOG_PATH); LittleFS.remove(LOG_OLD); }
    chainHead = "GENESIS"; chainSeq = 0;
    prefs.putString("head", chainHead); prefs.putUInt("seq", 0);
    Serial.println("Log wiped, chain reset.");
  }
  else if (c.startsWith("sim")) { simulate(c.substring(3).toInt()); }
  else Serial.println("commands: help | status | verify | sim <0-12> | isolate | release | trust | rebase | wipe");
}

// ============================ SETUP / LOOP =============================
void setup() {
  Serial.begin(115200);
  delay(300);
  tgMux = xSemaphoreCreateMutex();
  pinMode(PIN_LED_R, OUTPUT); pinMode(PIN_LED_G, OUTPUT); pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT); pinMode(PIN_ONBOARD, OUTPUT);
  pinMode(PIN_BOOT, INPUT_PULLUP);
  restoreLoad();
  setLed(false, false, true);                                    // blue = booting

  dht.setup(PIN_DHT, DHT_TYPE);
  prefs.begin("argus", false);
  chainHead = prefs.getString("head", "GENESIS");
  chainSeq = prefs.getUInt("seq", 0);
  bootCount = prefs.getUInt("boots", 0) + 1;
  prefs.putUInt("boots", bootCount);
  mode = (Mode)prefs.getUChar("mode", 0);
  if (mode > MODE_LOCKDOWN) mode = MODE_ARMED;
  fsOK = LittleFS.begin(true);
  Serial.printf("[Storage] LittleFS %s\n", fsOK ? "ready" : "FAILED");

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.onEvent(onWifiEvent);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) { delay(250); setLed(false, false, ((millis() / 250) % 2) == 0); }
  if (WiFi.status() == WL_CONNECTED) {
    trustedBssid = WiFi.BSSIDstr();
    configTime(19800, 0, "pool.ntp.org", "time.google.com");     // IST (UTC+5:30)
    if (MDNS.begin("argus")) MDNS.addService("http", "tcp", 80);
    Serial.println("[WiFi] " + WiFi.localIP().toString() + "  trusted BSSID " + trustedBssid);
  } else Serial.println("[WiFi] NOT connected - network modules idle");

  if (strlen(MQTT_HOST) == 0) moduleOff[M_MQTT] = true;
  Serial.println("Learning baselines - keep the network quiet and do not touch the tamper wire (~5 s)");
  learnTamper();
  if (WiFi.status() == WL_CONNECTED) learnGateway();

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqtt);
  mqtt.setBufferSize(256);
  mqtt.setSocketTimeout(3);

  logRaw("BOOT v" ARGUS_VERSION " ip=" + WiFi.localIP().toString() + " boot#" + String(bootCount));
  checkFirmwareAtBoot();
  checkResetReasonAtBoot();
  heapHigh = ESP.getFreeHeap();
  setupWeb();
  xTaskCreatePinnedToCore(tgTask, "tg", 10240, nullptr, 1, nullptr, 0);

  tg("[ARGUS] Online. FW " + fwNow.substring(0, 16) + " | log head " + chainHead.substring(0, 16) + " | dashboard http://" + WiFi.localIP().toString() + " | send /help for commands");
  Serial.println("Argus ready. Dashboard: http://" + WiFi.localIP().toString() + "  (or http://argus.local)");
  if (mode == MODE_LOCKDOWN) doIsolate("LOCKDOWN mode active at boot");
  evaluate();
}

void loop() {
  static unsigned long tSens = 0, tArp = 0, tEval = 0, tLink = 0, tTamper = 0, tHeap = 0;
  unsigned long now = millis();
  server.handleClient();
  mqttEnsure();
  mqtt.loop();
  serialTick();
  buttonTick();
  handlePending();
  if (now - tSens >= 5000) { tSens = now; sensorTick(); }
  if (now - tArp >= 2000) { tArp = now; arpTick(); }
  if (now - tLink >= 2000) { tLink = now; linkTick(); }
  if (now - tTamper >= 200) { tTamper = now; tamperTick(); }
  if (now - tHeap >= 4000) { tHeap = now; heapTick(); }
  apTick();
  mqttRateTick();
  if (scenarioStep == 1 && now >= scenarioAt) { scenarioStep = 0; simulate(M_MQTT); }
  if (now - tEval >= 500) { tEval = now; evaluate(); }
  healTick();
  outputsTick();
}
