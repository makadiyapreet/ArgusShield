/*
  ====================================================================
  ARGUS v3.1 - ATTACK SIMULATOR (second ESP32, no extra hardware needed)
  ====================================================================
  FOR YOUR OWN ISOLATED LAB NETWORK ONLY (your phone hotspot / your own
  router). It generates harmless test traffic so the Argus monitor has
  something to detect during the demo. Nothing here touches any device
  you don't own, and it never scans or floods anything outside your own
  monitor node.

  v3.1: MQTT sensor messages use the monitor's zero-trust "key|value"
  format now, a Probe/Scan test was added, and this board now serves
  its own web dashboard at http://<this-board-ip>/ (or argus-attacker
  .local) so you can run the whole demo from a phone/laptop browser
  instead of the Serial Monitor. Serial commands still work too.

  Library needed: PubSubClient. Board: "ESP32 Dev Module". 115200 baud.
  Fill in ARGUS_IP below with the monitor's IP before using the probe
  test (shown on the monitor's Serial Monitor / dashboard at boot).

  Serial Monitor commands (same actions as the dashboard buttons):
    1  toggle EVIL TWIN       (fake Wi-Fi AP, same SSID, different BSSID)
    2  toggle MQTT FLOOD      (~100 msgs/sec to a junk topic)
    3  UNAUTHORISED TOPIC     (publish to argus/admin/relay)
    4  WRONG KEY              (spoof sensor1 with a bad zero-trust key)
    5  FAKE VALUE             (correct key, implausible 55.0 C reading)
    6  UNKNOWN DEVICE         (publish as 'sensor9', not in DEVICE_KEYS)
    7  PROBE / SCAN           (six quick requests to unknown dashboard URLs)
    8  ENUMERATE              (publish as sensor10-14 in a row)
    9  PIN BRUTE-FORCE        (5 wrong X-Pin attempts on the dashboard API)
    0  stop everything

  IMPORTANT - tests 3, 4 and 6 all trip the monitor's single "MQTT policy"
  module, which only raises one non-critical alert every 3 s (anti-spam
  debounce by design). Firing two of them back-to-back makes the SECOND
  one silently disappear - that's almost certainly why 3/4/6 can look
  "broken" when tried right after each other. Space them out, or run one,
  wait ~4s, then the next. Test 5 lands on the separate SENSOR module
  (not MQTT policy) - look for it there, and if the monitor has just
  booted and hasn't taken its first DHT reading yet it will read as
  "Foreign publisher on sensor topic" instead of "Fake sensor value
  injected", which is still a correct alert, just a different label.

  (ARP spoofing and Wi-Fi deauth bursts are real attacks on other radios
  / the LAN's own address resolution and are done from your laptop with
  arpspoof / aireplay-ng, as in the guide, not from this board.)
  ====================================================================
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <PubSubClient.h>

const char* WIFI_SSID = "YOUR_WIFI_SSID";        // same as the monitor
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char* MQTT_HOST = "";          // same broker as the monitor
const uint16_t MQTT_PORT = 1883;
const char* ARGUS_IP   = "XXX.XXX.XXX.XXX";          // the MONITOR's IP (for the probe test)
const char* GOOD_KEY   = "CHANGE_ME";             // sensor1's key from the monitor's DEVICE_KEYS
const char* BAD_KEY    = "wrongkey";

WiFiClient net;
PubSubClient mqtt(net);
WebServer server(80);

bool twin = false, flood = false;
unsigned long lastPub = 0;
unsigned long floodSent = 0, twinSince = 0;
unsigned long mqttPolicyLast = 0, sensorLast = 0;     // last time THIS board fired something in that alert category
String lastAction = "Idle";

// Tests 3, 4 and 6 all land on the monitor's single "MQTT policy" module, and the
// monitor only allows one non-critical alert per module every 3 s (anti-spam
// debounce) - firing two of these back-to-back makes the second one vanish with
// no error. This just warns you in the dashboard when that's about to happen.
String cooldownNote(unsigned long &catLast) {
  unsigned long now = millis();
  String note = (catLast != 0 && now - catLast < 3200)
    ? " [monitor may debounce this - same alert category fired <3s ago, wait and retry]" : "";
  catLast = now;
  return note;
}

// ============================ ACTIONS ===================================
void mqttEnsure() {
  if (mqtt.connected() || WiFi.status() != WL_CONNECTED) return;
  static unsigned long last = 0;
  if (millis() - last < 3000) return;
  last = millis();
  if (mqtt.connect("argus-attacker")) Serial.println("[MQTT] connected");
  else Serial.println("[MQTT] connect failed");
}

void setTwin(bool on) {
  twin = on;
  if (on) {
    WiFi.mode(WIFI_AP_STA);
    bool ok = WiFi.softAP(WIFI_SSID);
    twinSince = millis();
    lastAction = ok ? "Evil twin ON" : "Evil twin FAILED";
    Serial.printf("[Evil twin] %s  BSSID %s\n", ok ? "ON" : "FAILED", WiFi.softAPmacAddress().c_str());
  } else {
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    lastAction = "Evil twin OFF";
    Serial.println("[Evil twin] OFF");
  }
}

void setFlood(bool on) {
  flood = on;
  floodSent = 0;
  lastAction = flood ? "MQTT flood ON" : "MQTT flood OFF";
  Serial.println(lastAction);
}

void doBadTopic() {
  if (!mqtt.connected()) { lastAction = "Bad topic FAILED - MQTT not connected"; return; }
  mqtt.publish("argus/admin/relay", "OFF");
  lastAction = "Sent to argus/admin/relay (unauthorised topic)" + cooldownNote(mqttPolicyLast);
  Serial.println("[Bad topic] " + lastAction);
}

void doWrongKey() {
  if (!mqtt.connected()) { lastAction = "Wrong key FAILED - MQTT not connected"; return; }
  mqtt.publish("argus/dev/sensor1/temp", (String(BAD_KEY) + "|99.9").c_str());
  lastAction = "Spoofed sensor1 with a wrong key" + cooldownNote(mqttPolicyLast);
  Serial.println("[Wrong key] " + lastAction);
}

void doFakeValue() {
  if (!mqtt.connected()) { lastAction = "Fake value FAILED - MQTT not connected"; return; }
  mqtt.publish("argus/dev/sensor1/temp", (String(GOOD_KEY) + "|55.0").c_str());
  // Lands on the SENSOR module (not MQTT policy), and only reads as "Fake sensor value
  // injected" once the monitor has taken its first real DHT reading - on a very fresh
  // boot it shows up as "Foreign publisher on sensor topic" instead. Look in the Sensor
  // alert list either way, not MQTT policy.
  lastAction = "sensor1 correct key, implausible 55.0 C - check the SENSOR alerts" + cooldownNote(sensorLast);
  Serial.println("[Fake value] " + lastAction);
}

void doUnknownDevice() {
  if (!mqtt.connected()) { lastAction = "Unknown device FAILED - MQTT not connected"; return; }
  mqtt.publish("argus/dev/sensor9/temp", (String(GOOD_KEY) + "|24.0").c_str());
  lastAction = "Published as sensor9 (not in DEVICE_KEYS)" + cooldownNote(mqttPolicyLast);
  Serial.println("[Unknown device] " + lastAction);
}

void doEnumerate() {
  // Registers several fake device ids one after another, to fill up the monitor's
  // live device registry and show what reconnaissance / device enumeration looks
  // like, rather than a single spoofed id.
  if (!mqtt.connected()) { lastAction = "Enumerate FAILED - MQTT not connected"; return; }
  for (int i = 10; i <= 14; i++) {
    mqtt.publish(("argus/dev/sensor" + String(i) + "/temp").c_str(), (String(GOOD_KEY) + "|21.0").c_str());
    delay(150);
  }
  lastAction = "Enumerated sensor10-14 (none registered) - see Devices panel" + cooldownNote(mqttPolicyLast);
  Serial.println("[Enumerate] " + lastAction);
}

// Sends one GET with up to 2 retries if the monitor answers 503 (Service
// Unavailable). The monitor's single-client WebServer can return 503 when
// it is already busy answering the dashboard's own polling (status/history/
// log/devices/scan) or the Python proxy - this is normal contention, not a
// firmware bug, so a short backoff-and-retry clears it in almost all cases.
int getWithRetry(const String& url, const char* pinHeader = nullptr) {
  for (int attempt = 0; attempt < 3; attempt++) {
    HTTPClient http;
    http.begin(url);
    if (pinHeader) http.addHeader("X-Pin", pinHeader);
    int code = http.GET();
    http.end();
    if (code != 503) return code;
    Serial.println("  -> 503 from monitor (busy), retrying in " + String(250 * (attempt + 1)) + "ms ...");
    delay(250 * (attempt + 1));                      // 250ms, then 500ms
  }
  return 503;                                        // still busy after 3 tries
}

void doPinBrute() {
  // The monitor's dashboard locks out after 5 wrong X-Pin headers in a row and
  // raises its own Probe/Scan alert ("Dashboard brute-force"). This hits that
  // path directly instead of the generic unknown-path probe.
  if (WiFi.status() != WL_CONNECTED) { lastAction = "PIN brute-force failed: Wi-Fi not connected"; return; }
  if (String(ARGUS_IP).indexOf('X') >= 0) { lastAction = "PIN brute-force failed: set ARGUS_IP first"; return; }
  int busyCount = 0;
  for (int i = 0; i < 5; i++) {
    int code = getWithRetry(String("http://") + ARGUS_IP + "/api/cmd?c=status", "0000");
    if (code == 503) busyCount++;
    delay(400);                                      // widened from 200ms to reduce collisions
  }
  lastAction = "Sent 5 wrong-PIN attempts to the monitor's dashboard API";
  if (busyCount) lastAction += " (" + String(busyCount) + " still 503 after retries - monitor is overloaded, close the dashboard tab/proxy and retry)";
  Serial.println("[PIN brute-force] " + lastAction);
}

void runProbe() {
  if (WiFi.status() != WL_CONNECTED) { lastAction = "Probe failed: Wi-Fi not connected"; return; }
  if (String(ARGUS_IP).indexOf('X') >= 0) { lastAction = "Probe failed: set ARGUS_IP first"; return; }
  const char* paths[] = {"/a", "/b", "/c", "/d", "/e", "/f"};
  Serial.println("[Probe] sending 6 unknown-path requests ...");
  int busyCount = 0;
  for (int i = 0; i < 6; i++) {
    String url = String("http://") + ARGUS_IP + paths[i];
    int code = getWithRetry(url);
    if (code == 503) busyCount++;
    Serial.println("  -> " + url + "  [" + String(code) + "]");
    delay(500);                                      // widened from 300ms, still under the 8s window
  }
  lastAction = "Probe sent (6 unknown paths to " + String(ARGUS_IP) + ")";
  if (busyCount) lastAction += " (" + String(busyCount) + " still 503 after retries - monitor is overloaded, close the dashboard tab/proxy and retry)";
  Serial.println("[Probe] done - check the monitor for a Probe/Scan alert");
}

void stopAll() {
  setFlood(false);
  if (twin) setTwin(false);
  lastAction = "All stopped";
  Serial.println("All stopped");
}

// ============================ WEB DASHBOARD =============================
const char PAGE[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Argus Attack Simulator</title>
<style>
:root{--bg:#0f1420;--card:#171e2e;--line:#273047;--text:#e8edf7;--dim:#8793ac;
--red:#ef4444;--amber:#f59e0b;--green:#22c55e;--blue:#3b82f6}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--text);font-family:system-ui,-apple-system,Segoe UI,Roboto,sans-serif;padding:20px}
h1{font-size:1.25rem;margin:0 0 2px}
.sub{color:var(--dim);font-size:.85rem;margin-bottom:18px}
.badge{display:inline-block;padding:3px 10px;border-radius:999px;font-size:.75rem;font-weight:600;margin-left:8px;vertical-align:middle}
.b-on{background:rgba(239,68,68,.18);color:var(--red)}
.b-off{background:rgba(34,197,94,.15);color:var(--green)}
.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(220px,1fr));gap:12px}
.card{background:var(--card);border:1px solid var(--line);border-radius:14px;padding:16px}
.card h3{margin:0 0 4px;font-size:.95rem}
.card p{margin:0 0 12px;color:var(--dim);font-size:.8rem;line-height:1.3}
button{width:100%;padding:10px;border-radius:9px;border:1px solid var(--line);background:#1f2740;color:var(--text);
font-size:.85rem;font-weight:600;cursor:pointer}
button:active{transform:scale(.98)}
.danger{background:rgba(239,68,68,.15);border-color:rgba(239,68,68,.4);color:#fca5a5}
.status{margin-top:18px;background:var(--card);border:1px solid var(--line);border-radius:14px;padding:14px 16px;font-size:.85rem;color:var(--dim)}
.status b{color:var(--text)}
.stopbar{margin-top:16px}
.stopbar button{background:var(--red);color:#fff;border:none;font-size:.95rem;padding:13px}
</style></head>
<body>
<h1>Argus Attack Simulator</h1>
<div class="sub">Lab-only test traffic for the Argus monitor demo &middot; targets your own network only</div>

<div class="grid">
  <div class="card"><h3>Evil Twin <span id="twinBadge" class="badge b-off">OFF</span></h3>
    <p>Fake AP, same SSID as the real network, different BSSID.</p>
    <button onclick="cmd('twin')">Toggle</button></div>

  <div class="card"><h3>MQTT Flood <span id="floodBadge" class="badge b-off">OFF</span></h3>
    <p>~100 junk messages/sec to argus/junk/flood.</p>
    <button onclick="cmd('flood')">Toggle</button></div>

  <div class="card"><h3>Unauthorised Topic</h3>
    <p>Publishes to argus/admin/relay, which is off the MQTT allow-list.</p>
    <button onclick="cmd('badtopic')">Send</button></div>

  <div class="card"><h3>Wrong Key</h3>
    <p>Spoofs sensor1 with an invalid zero-trust key &rarr; marked untrusted.</p>
    <button onclick="cmd('wrongkey')">Send</button></div>

  <div class="card"><h3>Fake Value</h3>
    <p>Correct key, implausible 55.0&deg;C reading on sensor1.</p>
    <button onclick="cmd('fakevalue')">Send</button></div>

  <div class="card"><h3>Unknown Device</h3>
    <p>Publishes as sensor9, which isn't in the monitor's DEVICE_KEYS.</p>
    <button onclick="cmd('unknown')">Send</button></div>

  <div class="card"><h3>Probe / Scan</h3>
    <p>6 quick requests to unknown dashboard paths on the monitor.</p>
    <button onclick="cmd('probe')">Run</button></div>

  <div class="card"><h3>Device Enumeration</h3>
    <p>Publishes as 5 different unknown ids (sensor10-14) in a row.</p>
    <button onclick="cmd('enumerate')">Run</button></div>

  <div class="card"><h3>PIN Brute-force</h3>
    <p>5 wrong X-Pin attempts at the monitor's dashboard API &rarr; lockout alert.</p>
    <button onclick="cmd('pinbrute')">Run</button></div>
</div>
<p style="color:var(--dim);font-size:.75rem;margin-top:10px">Space tests at least ~3-4s apart where possible: the monitor allows only one non-critical alert per module every 3s, so firing two MQTT-policy tests back to back can silently drop the second.</p>

<div class="stopbar"><button onclick="cmd('stop')">STOP EVERYTHING</button></div>

<div class="status">
  Wi-Fi: <b id="wifi">-</b> &middot; MQTT: <b id="mqttState">-</b> &middot; Flood sent: <b id="floodCount">0</b><br>
  Last action: <b id="lastAction">-</b>
</div>

<script>
function cmd(c){
  fetch('/api/cmd?c='+c).then(r=>r.json()).then(refresh);
}
function refresh(){
  fetch('/api/status').then(r=>r.json()).then(d=>{
    document.getElementById('twinBadge').textContent = d.twin ? 'ON' : 'OFF';
    document.getElementById('twinBadge').className = 'badge ' + (d.twin ? 'b-on' : 'b-off');
    document.getElementById('floodBadge').textContent = d.flood ? 'ON' : 'OFF';
    document.getElementById('floodBadge').className = 'badge ' + (d.flood ? 'b-on' : 'b-off');
    document.getElementById('wifi').textContent = d.wifi;
    document.getElementById('mqttState').textContent = d.mqtt ? 'connected' : 'disconnected';
    document.getElementById('floodCount').textContent = d.floodSent;
    document.getElementById('lastAction').textContent = d.lastAction;
  });
}
refresh();
setInterval(refresh, 2000);
</script>
</body></html>
)HTML";

String statusJson() {
  String j = "{";
  j += "\"twin\":" + String(twin ? "true" : "false") + ",";
  j += "\"flood\":" + String(flood ? "true" : "false") + ",";
  j += "\"floodSent\":" + String(floodSent) + ",";
  j += "\"mqtt\":" + String(mqtt.connected() ? "true" : "false") + ",";
  j += "\"wifi\":\"" + (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String("disconnected")) + "\",";
  j += "\"lastAction\":\"" + lastAction + "\"";
  j += "}";
  return j;
}

void handleCmd() {
  String c = server.arg("c");
  if (c == "twin") setTwin(!twin);
  else if (c == "flood") setFlood(!flood);
  else if (c == "badtopic") doBadTopic();
  else if (c == "wrongkey") doWrongKey();
  else if (c == "fakevalue") doFakeValue();
  else if (c == "unknown") doUnknownDevice();
  else if (c == "probe") runProbe();
  else if (c == "enumerate") doEnumerate();
  else if (c == "pinbrute") doPinBrute();
  else if (c == "stop") stopAll();
  server.send(200, "application/json", statusJson());
}

void setupWeb() {
  server.on("/", []() { server.send_P(200, "text/html", PAGE); });
  server.on("/api/status", []() { server.sendHeader("Cache-Control", "no-store"); server.send(200, "application/json", statusJson()); });
  server.on("/api/cmd", handleCmd);
  server.begin();
}

// ============================ SETUP / LOOP ==============================
void setup() {
  Serial.begin(115200);
  delay(300);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting");
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) { delay(300); Serial.print("."); }
  Serial.println(WiFi.status() == WL_CONNECTED ? "\nConnected " + WiFi.localIP().toString() : "\nWi-Fi failed");
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setSocketTimeout(3);
  if (WiFi.status() == WL_CONNECTED && MDNS.begin("argus-attacker")) MDNS.addService("http", "tcp", 80);
  setupWeb();
  Serial.println("Dashboard: http://" + WiFi.localIP().toString() + "  (or http://argus-attacker.local)");
  Serial.println("Menu: 1 twin|2 flood|3 bad topic|4 wrong key|5 fake value|6 unknown device|7 probe/scan|8 enumerate|9 PIN brute-force|0 stop");
}

void loop() {
  mqttEnsure();
  mqtt.loop();
  server.handleClient();

  if (Serial.available()) {
    char c = Serial.read();
    if (c == '1') setTwin(!twin);
    else if (c == '2') setFlood(!flood);
    else if (c == '3') doBadTopic();
    else if (c == '4') doWrongKey();
    else if (c == '5') doFakeValue();
    else if (c == '6') doUnknownDevice();
    else if (c == '7') runProbe();
    else if (c == '8') doEnumerate();
    else if (c == '9') doPinBrute();
    else if (c == '0') stopAll();
  }

  if (flood && millis() - lastPub >= 10) { lastPub = millis(); mqtt.publish("argus/junk/flood", "x"); floodSent++; }
}
