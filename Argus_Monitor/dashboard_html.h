#pragma once
const char PAGE[] PROGMEM = R"=====(<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>ARGUS</title>
<style>
:root{--bg:#060912;--p:#10172c;--p2:#151f3c;--ln:#223058;--ln2:#2c3b6b;--tx:#eef2fd;--mu:#8fa0c4;--ok:#22c55e;--wa:#f59e0b;--cr:#ef4444;--ac:#38bdf8;--ac2:#a78bfa;--r:14px;--lvcol:#38bdf8}
*{box-sizing:border-box}
body{margin:0;position:relative;background:
  radial-gradient(900px 420px at 85% -8%,rgba(56,189,248,.16) 0%,transparent 60%),
  radial-gradient(700px 380px at 5% 0%,rgba(167,139,250,.12) 0%,transparent 55%),
  var(--bg);
  color:var(--tx);font:14px/1.45 system-ui,"Segoe UI",Roboto,Arial,sans-serif;min-height:100vh}
body::after{content:"";position:fixed;inset:0;pointer-events:none;z-index:50;opacity:.035;mix-blend-mode:overlay;
  background-image:url('data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg"><filter id="n"><feTurbulence type="fractalNoise" baseFrequency="0.9" numOctaves="2" stitchTiles="stitch"/></filter><rect width="100%25" height="100%25" filter="url(%23n)"/></svg>')}
button{font:inherit;cursor:pointer}
header{display:flex;align-items:center;gap:12px;padding:12px 20px;border-bottom:1px solid var(--ln);background:rgba(6,9,18,.78);position:sticky;top:0;z-index:5;backdrop-filter:blur(10px)}
.logo{display:flex;align-items:center;gap:10px;font-weight:800;letter-spacing:.16em}
.logo-ic{position:relative;width:34px;height:34px;display:flex;align-items:center;justify-content:center}
.logo-ic::before{content:"";position:absolute;inset:0;border-radius:50%;background:conic-gradient(from 0deg,rgba(56,189,248,.55),transparent 30%,transparent 100%);animation:radarSweep 2.6s linear infinite;-webkit-mask:radial-gradient(circle,transparent 58%,#000 60%);mask:radial-gradient(circle,transparent 58%,#000 60%)}
@keyframes radarSweep{to{transform:rotate(360deg)}}
.logo svg{position:relative;filter:drop-shadow(0 0 6px rgba(56,189,248,.65))}
.logo small{display:block;font-weight:400;letter-spacing:.02em;color:var(--mu);font-size:11px;margin-top:-2px}
.sp{flex:1}
.pill{padding:4px 11px;border-radius:99px;border:1px solid var(--ln);background:rgba(255,255,255,.03);color:var(--mu);font-size:12px;white-space:nowrap}
.dot{display:inline-block;width:8px;height:8px;border-radius:50%;background:var(--ok);margin-right:6px;box-shadow:0 0 9px currentColor;animation:pulseDot 2s ease-in-out infinite}
.dot.off{background:var(--cr);animation:none}
@keyframes pulseDot{0%,100%{opacity:1}50%{opacity:.45}}
main{max-width:1320px;margin:0 auto;padding:18px;display:grid;gap:16px}
.card{background:linear-gradient(165deg,rgba(21,31,60,.92),rgba(16,23,44,.92));border:1px solid var(--ln);border-radius:var(--r);padding:16px;backdrop-filter:blur(8px);box-shadow:0 1px 0 rgba(255,255,255,.03) inset,0 10px 30px -18px rgba(0,0,0,.6);transition:box-shadow .25s,transform .25s,border-color .25s;opacity:0;transform:translateY(10px);animation:cardIn .5s cubic-bezier(.2,.7,.3,1) forwards}
@keyframes cardIn{to{opacity:1;transform:translateY(0)}}
.card:hover{border-color:var(--ln2);box-shadow:0 1px 0 rgba(255,255,255,.04) inset,0 16px 40px -18px rgba(0,0,0,.7)}
.card h3{margin:0 0 12px;font-size:12px;letter-spacing:.13em;text-transform:uppercase;color:var(--mu);font-weight:700;display:flex;justify-content:space-between;align-items:center;gap:8px}
.card h3 .hic{font-size:14px;letter-spacing:0;text-transform:none;display:inline-flex;align-items:center;justify-content:center;width:24px;height:24px;border-radius:8px;background:rgba(255,255,255,.06);margin-right:1px}
.card[data-ac]{border-left:3px solid transparent}
.card[data-ac="cyan"]{border-left-color:#38bdf8}.card[data-ac="cyan"] h3 .hic{background:rgba(56,189,248,.18)}
.card[data-ac="amber"]{border-left-color:#f59e0b}.card[data-ac="amber"] h3 .hic{background:rgba(245,158,11,.18)}
.card[data-ac="green"]{border-left-color:#22c55e}.card[data-ac="green"] h3 .hic{background:rgba(34,197,94,.18)}
.card[data-ac="violet"]{border-left-color:#a78bfa}.card[data-ac="violet"] h3 .hic{background:rgba(167,139,250,.18)}
.card[data-ac="pink"]{border-left-color:#ec4899}.card[data-ac="pink"] h3 .hic{background:rgba(236,72,153,.18)}
.card[data-ac="red"]{border-left-color:#ef4444}.card[data-ac="red"] h3 .hic{background:rgba(239,68,68,.18)}
.card[data-ac="quiet"]{border-left-color:#2a3758;background:linear-gradient(165deg,rgba(15,21,42,.72),rgba(11,16,32,.72));box-shadow:none}
.card[data-ac="quiet"] h3{color:#64739c}.card[data-ac="quiet"] h3 .hic{background:rgba(255,255,255,.04)}
.card[data-ac="quiet"] .how b{font-size:12.5px}.card[data-ac="quiet"] .how{font-size:12px}
.card[data-ac="quiet"] .info div{font-size:12.5px}
.hero{position:relative;display:grid;grid-template-columns:260px 1fr 320px;gap:16px}
.hero::before{content:"";position:absolute;left:0;right:0;top:-40px;height:220px;z-index:-1;pointer-events:none;
  background:radial-gradient(540px 220px at 20% 0%,color-mix(in srgb,var(--lvcol) 22%,transparent) 0%,transparent 70%);
  transition:background .5s}
@media(max-width:980px){.hero{grid-template-columns:1fr}}
.gaugeBox{display:flex;flex-direction:column;align-items:center;justify-content:center}
#gauge{width:240px;height:200px}
#banner{border-radius:var(--r);padding:20px;display:flex;flex-direction:column;justify-content:center;gap:6px;border:1px solid var(--ln);position:relative;overflow:hidden;transition:box-shadow .4s}
#banner.ok{background:linear-gradient(135deg,#0d3522,#0b2420)}
#banner.wa{background:linear-gradient(135deg,#523406,#33230a)}
#banner.cr{background:linear-gradient(135deg,#601212,#350d12);animation:critPulse 1.6s ease-in-out infinite}
@keyframes critPulse{0%,100%{box-shadow:0 0 0 0 rgba(239,68,68,.55),0 10px 30px -14px rgba(0,0,0,.7)}50%{box-shadow:0 0 0 10px rgba(239,68,68,0),0 10px 30px -14px rgba(0,0,0,.7)}}
#banner h2{margin:0;font-size:34px;font-weight:800;letter-spacing:.02em;display:flex;align-items:center;gap:10px;text-shadow:0 0 22px rgba(0,0,0,.35)}
#banner p{margin:0;color:#dbe2f5;font-size:14.5px}
.btnrow{display:flex;flex-wrap:wrap;gap:8px;margin-top:10px}
.btn{background:rgba(255,255,255,.04);border:1px solid var(--ln);color:var(--tx);border-radius:99px;padding:8px 14px;font-weight:600;font-size:13px;transition:transform .15s,border-color .15s,background .15s,box-shadow .15s}
.btn:hover{border-color:var(--ac);transform:translateY(-1px);box-shadow:0 6px 16px -8px rgba(56,189,248,.5)}
.btn:active{transform:translateY(0)}
.btn.pri{background:linear-gradient(135deg,#0c4a6e,#0369a1);border-color:#0ea5e9}
.btn.dan{background:linear-gradient(135deg,#5b1620,#9f1d2c);border-color:#ef4444}
.btn.on{border-color:var(--ac);box-shadow:inset 0 0 0 1px var(--ac),0 0 12px -2px rgba(56,189,248,.6)}
.kpis{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.kpi{background:rgba(255,255,255,.035);border:1px solid var(--ln);border-radius:12px;padding:11px;transition:border-color .2s}
.kpi:hover{border-color:var(--ln2)}
.kpi b{display:block;font-size:26px;font-weight:800;letter-spacing:-.01em;font-variant-numeric:tabular-nums;white-space:nowrap}
.kpi span{color:var(--mu);font-size:11px;text-transform:uppercase;letter-spacing:.08em}
.kpi{position:relative}
.spark{display:block;width:100%;height:22px;margin-top:4px;opacity:.9}
.mods{display:grid;grid-template-columns:repeat(auto-fit,minmax(225px,1fr));gap:10px}
.mod{position:relative;background:rgba(255,255,255,.035);border:1px solid var(--ln);border-left:4px solid var(--ok);border-radius:12px;padding:11px 78px 11px 11px;transition:transform .18s,border-color .18s,background .18s}
.mod:hover{transform:translateY(-2px);background:rgba(255,255,255,.055)}
.mod.wa{border-left-color:var(--wa)}.mod.cr{border-left-color:var(--cr);animation:modGlow 1.8s ease-in-out infinite}.mod.na{border-left-color:#475569;opacity:.75}
@keyframes modGlow{0%,100%{box-shadow:0 0 0 0 rgba(239,68,68,.35)}50%{box-shadow:0 0 14px 0 rgba(239,68,68,.35)}}
.mod b{display:flex;align-items:center;font-size:13.5px;flex-wrap:wrap;row-gap:4px}
.mod b .nm{white-space:nowrap;overflow:hidden;text-overflow:ellipsis;max-width:120px}
.mod small{color:var(--mu);display:block;min-height:17px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.mod .st{position:absolute;top:12px;right:11px;font-size:10.5px;font-weight:700;padding:2px 9px;border-radius:99px;background:#14391f;color:#86efac;letter-spacing:.04em;white-space:nowrap}
.mod.wa .st{background:#4a3208;color:#fcd34d}.mod.cr .st{background:#5b1620;color:#fca5a5}.mod.na .st{background:#1e293b;color:#94a3b8}
.two{display:grid;grid-template-columns:1fr 1fr;gap:16px}
@media(max-width:980px){.two{grid-template-columns:1fr}}
canvas.ch{width:100%;height:170px;display:block}
canvas.tl{width:100%;height:280px;display:block}
.mod .ic{font-size:16px;margin-right:7px;filter:drop-shadow(0 0 3px rgba(0,0,0,.4))}
.howrow{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:10px;margin-top:2px}
.how{background:linear-gradient(160deg,rgba(255,255,255,.045),rgba(255,255,255,.015));border:1px solid var(--ln);border-radius:12px;padding:11px 13px;font-size:12.5px;color:#cfd9f2;transition:border-color .2s,transform .2s}
.how:hover{border-color:var(--ln2);transform:translateY(-2px)}
.how b{color:var(--tx);display:block;margin-bottom:4px;font-size:13px}
.cat{display:inline-block;font-size:9.5px;letter-spacing:.07em;text-transform:uppercase;color:var(--mu);background:rgba(255,255,255,.06);border-radius:6px;padding:2px 7px;margin-left:7px;font-weight:600}
.cat[data-c="Network"]{color:#7dd3fc;background:rgba(56,189,248,.14)}
.cat[data-c="Protocol"]{color:#c4b5fd;background:rgba(167,139,250,.14)}
.cat[data-c="Device"]{color:#fcd34d;background:rgba(245,158,11,.14)}
.cat[data-c="Data"]{color:#86efac;background:rgba(34,197,94,.14)}
.cat[data-c="Web"]{color:#f9a8d4;background:rgba(236,72,153,.14)}
.cat[data-c="Physical"]{color:#fca5a5;background:rgba(239,68,68,.14)}
.feed{max-height:330px;overflow:auto}
table{width:100%;border-collapse:collapse;font-size:13px}
th,td{padding:7px 7px;border-bottom:1px solid var(--ln);text-align:left;vertical-align:top}
tbody tr{transition:background .15s}
tbody tr:nth-child(even){background:rgba(255,255,255,.015)}
tbody tr:hover{background:rgba(56,189,248,.07)}
th{color:var(--mu);font-weight:700;font-size:10.5px;text-transform:uppercase;letter-spacing:.08em;position:sticky;top:0;background:#10172c}
.sev{display:inline-block;min-width:22px;text-align:center;border-radius:6px;padding:1px 7px;font-size:12px;font-weight:800}
.s1{background:#3b3b0d;color:#fde68a}.s2{background:#5a3b06;color:#fdba74}.s3{background:#6a2a0a;color:#fb923c}.s4{background:#6a1313;color:#fca5a5;box-shadow:0 0 8px -1px rgba(239,68,68,.6)}
.chips{display:flex;flex-wrap:wrap;gap:6px;margin-bottom:10px}
.chip{border:1px solid var(--ln);background:rgba(255,255,255,.03);color:var(--mu);border-radius:99px;padding:3px 11px;font-size:12px;font-weight:600;transition:all .15s}
.chip.on{color:#fff;border-color:var(--ac);background:linear-gradient(135deg,#0c4a6e,#0369a1);box-shadow:0 0 10px -2px rgba(56,189,248,.6)}
input,select{background:var(--p);border:1px solid var(--ln);color:var(--tx);border-radius:9px;padding:6px 9px;font:inherit}
input:focus,select:focus{outline:none;border-color:var(--ac)}
.mono{font-family:ui-monospace,Consolas,monospace;font-size:12px}
.logbox{background:#05070f;border:1px solid var(--ln);border-radius:10px;padding:10px;max-height:230px;overflow:auto;white-space:pre-wrap;word-break:break-all}
.info{display:grid;grid-template-columns:repeat(auto-fit,minmax(210px,1fr));gap:9px 18px}
.info div{display:flex;justify-content:space-between;gap:8px;border-bottom:1px dashed var(--ln);padding:4px 0}
.info span{color:var(--mu)}.info b{font-weight:600;text-align:right;overflow:hidden;text-overflow:ellipsis}
.seg{display:inline-flex;border:1px solid var(--ln);border-radius:99px;overflow:hidden;background:rgba(255,255,255,.02)}
.seg button{background:transparent;border:0;color:var(--mu);padding:7px 14px;font-weight:600;transition:all .2s}
.seg button.on{background:linear-gradient(135deg,#0c4a6e,#0369a1);color:#fff;box-shadow:0 0 10px -2px rgba(56,189,248,.5)}
#toast{position:fixed;right:14px;bottom:14px;display:grid;gap:8px;z-index:20}
.t{background:#0f172a;border:1px solid var(--ln);border-left:4px solid var(--ac);padding:10px 15px;border-radius:10px;box-shadow:0 10px 28px rgba(0,0,0,.5);animation:toastIn .25s ease-out}
@keyframes toastIn{from{opacity:0;transform:translateY(8px)}to{opacity:1;transform:translateY(0)}}
.t.ok{border-left-color:var(--ok)}.t.bad{border-left-color:var(--cr)}
#modal{position:fixed;inset:0;background:rgba(0,0,0,.65);display:none;align-items:center;justify-content:center;z-index:30;backdrop-filter:blur(3px)}
#modal div{background:linear-gradient(165deg,#151f3c,#10172c);border:1px solid var(--ln2);border-radius:14px;padding:22px;width:300px;display:grid;gap:10px;box-shadow:0 20px 60px rgba(0,0,0,.6)}
.legend{display:flex;gap:14px;color:var(--mu);font-size:12px;margin-top:6px;flex-wrap:wrap}
.legend i{display:inline-block;width:10px;height:10px;border-radius:3px;margin-right:5px}
</style></head><body>
<header>
 <div class="logo"><span class="logo-ic"><svg width="30" height="30" viewBox="0 0 32 32"><path d="M16 2 4 7v8c0 7.5 5 13 12 15 7-2 12-7.500 12-15V7z" fill="none" stroke="#38bdf8" stroke-width="2"/><circle cx="16" cy="15" r="5" fill="none" stroke="#38bdf8" stroke-width="2"/><circle cx="16" cy="15" r="1.800" fill="#38bdf8"/></svg></span><div>ARGUS<small>Autonomous IoT threat detection and response</small></div></div>
 <div class="sp"></div>
 <span class="pill" id="pConn"><span class="dot" id="dConn"></span><span id="tConn">connecting</span></span>
 <span class="pill" id="pClock">--:--:--</span>
 <span class="pill" id="pUp">up 0s</span>
 <span class="pill" id="pMode">ARMED</span>
</header>
<main>
 <section class="hero">
  <div class="card gaugeBox" id="gaugeCard" data-ac="cyan"><h3 style="width:100%"><span class="hic">🎯</span>&nbsp;Threat level</h3><canvas id="gauge" width="420" height="340"></canvas></div>
  <div id="banner" class="ok"><h2 id="bT">NOMINAL</h2><p id="bS">All detection modules quiet.</p>
   <div class="btnrow"><button class="btn dan" id="bIso">🛑 Isolate device now</button><button class="btn pri" id="bRel">✅ Release / restore</button><button class="btn" id="bMute">🔇 Mute alarm</button><button class="btn" id="bSnd">🔈 Dashboard sound: off</button></div></div>
  <div class="card" data-ac="amber"><h3><span class="hic">🌡️</span>&nbsp;Live readings</h3><div class="kpis">
   <div class="kpi"><span>Temperature</span><b id="kT">--</b><canvas class="spark" id="spT" width="90" height="22"></canvas></div>
   <div class="kpi"><span>Humidity</span><b id="kH">--</b><canvas class="spark" id="spH" width="90" height="22"></canvas></div>
   <div class="kpi"><span>Wi-Fi signal</span><b id="kR">--</b><canvas class="spark" id="spR" width="90" height="22"></canvas></div>
   <div class="kpi"><span>Protected device</span><b id="kP">--</b></div></div></div>
 </section>
 <section class="card" data-ac="quiet"><h3><span class="hic">🧭</span>&nbsp;How Argus decides</h3><div class="howrow">
   <div class="how"><b>🟢 One module alerts</b>Logged as WARNING. You're told, but nothing is cut off - could be a false alarm.</div>
   <div class="how"><b>🔴 Two or more agree</b>Escalates to CRITICAL: the relay cuts the protected device automatically, within 20 seconds of the first signal.</div>
   <div class="how"><b>🔑 Zero-trust devices</b>An MQTT device is only believed if its message carries its secret key - a plausible value with the wrong key is rejected, not just logged.</div>
   <div class="how"><b>♻️ Self-heal</b>After 30 quiet seconds, firmware and network baselines are re-checked and the device is automatically restored.</div>
  </div></section>
 <section class="card" data-ac="cyan"><h3><span class="hic">🛡️</span>&nbsp;Detection modules <span id="modSum" class="mu"></span></h3><div class="mods" id="mods"></div></section>
 <section class="two">
  <div class="card" data-ac="green"><h3><span class="hic">🔐</span>&nbsp;Zero-trust device registry <span id="devSum" class="mu"></span></h3>
   <table><thead><tr><th>Device id</th><th>Last seen</th><th>Status</th><th>Messages</th><th>Last seq</th></tr></thead><tbody id="devBody"></tbody></table>
   <div style="color:var(--mu);font-size:11px;margin-top:6px">Every MQTT message on argus/dev/&lt;id&gt;/... must carry that device's pre-shared key, or it's rejected. "Last seq" rising confirms each reading is fresh, not a replayed copy of an old one.</div></div>
  <div class="card" data-ac="violet"><h3><span class="hic">📶</span>&nbsp;Wi-Fi scout <span id="scoutSum" class="mu"></span></h3>
   <table><thead><tr><th>SSID</th><th>BSSID</th><th>Ch</th><th>RSSI</th><th>Security</th></tr></thead><tbody id="scoutBody"></tbody></table>
   <div style="color:var(--mu);font-size:11px;margin-top:6px">Every network Argus can currently see on this channel band, refreshed every scan. "This device" is the trusted access point.</div></div>
 </section>
 <section class="two">
  <div class="card" data-ac="amber"><h3><span class="hic">📈</span>&nbsp;Temperature and humidity (last 10 min)</h3><canvas class="ch" id="cTH"></canvas><div class="legend"><span><i style="background:#f97316"></i>Temp C</span><span><i style="background:#38bdf8"></i>Humidity %</span></div></div>
  <div class="card" data-ac="violet"><h3><span class="hic">📡</span>&nbsp;Wi-Fi signal (RSSI dBm)</h3><canvas class="ch" id="cR"></canvas><div class="legend"><span><i style="background:#a78bfa"></i>RSSI</span></div></div>
 </section>
 <section class="card" data-ac="pink"><h3><span class="hic">⏱️</span>&nbsp;Incident timeline (last 10 min)</h3><canvas class="tl" id="cTL"></canvas><div class="legend"><span><i style="background:#facc15"></i>Low</span><span><i style="background:#fb923c"></i>Medium</span><span><i style="background:#f97316"></i>High</span><span><i style="background:#ef4444"></i>Critical</span></div></section>
 <section class="two">
  <div class="card" data-ac="amber"><h3><span class="hic">📰</span>&nbsp;Alert feed <span id="fCount"></span></h3>
   <div class="chips" id="chips"></div>
   <div style="display:flex;gap:8px;margin-bottom:8px"><select id="fMod"></select><input id="fQ" placeholder="search text" style="flex:1"></div>
   <div class="feed"><table><thead><tr><th>Ago</th><th>Module</th><th>Sev</th><th>Message</th></tr></thead><tbody id="feed"></tbody></table></div></div>
  <div style="display:grid;gap:14px">
   <div class="card" data-ac="red"><h3><span class="hic">🧯</span>&nbsp;Incidents (isolations)</h3><table><thead><tr><th>Started</th><th>Duration</th><th>Cause</th></tr></thead><tbody id="inc"></tbody></table></div>
   <div class="card" data-ac="quiet"><h3><span class="hic">🧾</span>&nbsp;Forensic log <span><button class="btn" id="bVer">✔️ Verify chain</button> <button class="btn" id="bLog">↻ Refresh</button> <a class="btn" href="/api/log.txt" download="argus_log.txt" style="text-decoration:none">⬇️ Download</a></span></h3>
    <div id="verR" style="margin-bottom:8px;color:var(--mu)">Chain not verified yet.</div><div class="logbox mono" id="logBox">...</div></div>
  </div>
 </section>
 <section class="two">
  <div class="card" data-ac="cyan"><h3><span class="hic">🎛️</span>&nbsp;Control panel</h3>
   <div style="display:grid;gap:12px">
    <div>Mode&nbsp; <div class="seg" id="seg"><button data-v="0">Armed (auto-isolate)</button><button data-v="1">Monitor only</button><button data-v="2">Lockdown</button></div></div>
    <div class="btnrow" style="margin:0"><button class="btn" id="bST">🧪 Run hardware self-test</button><button class="btn" id="bRT">🔌 Test relay</button><button class="btn" id="bRB">📍 Re-learn baselines</button><button class="btn" id="bTR">🔏 Trust current firmware</button><button class="btn" id="bCL">🧹 Clear alerts</button><button class="btn" id="bRP">📄 Export incident report</button></div>
    <div><div style="color:var(--mu);margin-bottom:6px">Demo scenarios (marked SIMULATED in the log)</div>
     <div class="btnrow" style="margin:0"><button class="btn pri" id="bS1">🟡 Demo: single anomaly (warning)</button><button class="btn dan" id="bS2">🔴 Demo: coordinated attack (critical)</button><select id="simM"></select><button class="btn" id="bSim">⚡ Inject test event</button></div></div>
    <div style="color:var(--mu);font-size:12px">Controls ask for the admin PIN once per browser tab. Five wrong PINs lock controls for 60 s.</div>
   </div></div>
  <div class="card" data-ac="quiet"><h3><span class="hic">ℹ️</span>&nbsp;Device information</h3><div class="info" id="info"></div></div>
 </section>
</main>
<div id="toast"></div>
<div id="modal"><div><b>Admin PIN</b><input id="pinI" type="password" inputmode="numeric" placeholder="PIN"><div class="btnrow" style="margin:0"><button class="btn pri" id="pinOk">OK</button><button class="btn" id="pinNo">Cancel</button></div></div></div>
<script>
var NAMES=["Rogue AP","ARP / MITM","MQTT policy","Firmware","Sensor","Wi-Fi link","Tamper","Probe/Scan","Replay","Memory/DoS","Reboot","Session","Wi-Fi flood"];
var DESC=["Fake Wi-Fi using your network's name","Someone impersonating your router","Bad topic, wrong device key, huge payload","Program on the chip was changed","Impossible, jumpy or spoofed readings","Repeated disconnects or signal loss","Wire touched - case opened","Many unknown pages hit quickly","An old message replayed by an attacker","Memory draining - crash/DoS attempt","Chip reset itself unexpectedly","Dashboard used from 2 places at once","Sudden jump in visible networks"];
var CAT=["Network","Network","Protocol","Device","Data","Network","Physical","Web","Protocol","Device","Device","Web","Network"];
var ICON=["📶","🔀","📨","🔏","🌡️","📡","🖐️","🔍","🔁","🧠","🔄","🪪","🌊"];
var LV=["NOMINAL","WARNING","CRITICAL"],LC=["ok","wa","cr"],SC=["","#facc15","#fb923c","#f97316","#ef4444"];
var S=null,H=null,sound=false,prevLv=0,sevMin=0,logTxt="",verTxt="Not verified";
function $(i){return document.getElementById(i)}
function el(t,c,x){var e=document.createElement(t);if(c)e.className=c;if(x!==undefined&&x!==null)e.textContent=String(x);return e}
function dur(s){s=Math.max(0,Math.floor(s));if(s<60)return s+"s";if(s<3600)return Math.floor(s/60)+"m "+(s%60)+"s";return Math.floor(s/3600)+"h "+Math.floor(s%3600/60)+"m"}
function toast(m,k){var ic=k==="ok"?"✅ ":k==="bad"?"⚠️ ":"ℹ️ ";var d=el("div","t "+(k||""),ic+m);$("toast").appendChild(d);setTimeout(function(){d.style.opacity="0";d.style.transform="translateY(6px)";d.style.transition="all .25s";setTimeout(function(){d.remove()},260)},3600)}
function beep(f,ms){if(!sound)return;try{var a=new (window.AudioContext||window.webkitAudioContext)(),o=a.createOscillator(),g=a.createGain();o.frequency.value=f;g.gain.value=.08;o.connect(g);g.connect(a.destination);o.start();setTimeout(function(){o.stop();a.close()},ms)}catch(e){}}
function fit(c){var r=c.getBoundingClientRect(),d=window.devicePixelRatio||1;c.width=Math.max(10,Math.round(r.width*d));c.height=Math.max(10,Math.round(r.height*d));var g=c.getContext("2d");g.setTransform(d,0,0,d,0,0);return{g:g,w:r.width,h:r.height}}
function gauge(score,lv){
 var c=$("gauge"),f=fit(c),g=f.g,cx=f.w/2,cy=f.h*0.6,R=Math.min(f.w/2-14,f.h*0.56);
 var a0=Math.PI*0.8,a1=Math.PI*2.2;g.clearRect(0,0,f.w,f.h);g.lineWidth=18;g.lineCap="round";
 g.strokeStyle="#1b2747";g.beginPath();g.arc(cx,cy,R,a0,a1);g.stroke();
 for(var tk=0;tk<=4;tk++){var ta=a0+(a1-a0)*tk/4,x1=cx+Math.cos(ta)*(R-12),y1=cy+Math.sin(ta)*(R-12),x2=cx+Math.cos(ta)*(R+12),y2=cy+Math.sin(ta)*(R+12);
  g.strokeStyle="#0a1020";g.lineWidth=3;g.beginPath();g.moveTo(x1,y1);g.lineTo(x2,y2);g.stroke()}
 g.lineWidth=18;
 var col=lv==2?"#ef4444":lv==1?"#f59e0b":"#22c55e";
 g.save();g.shadowColor=col;g.shadowBlur=22;
 g.strokeStyle=col;g.beginPath();g.arc(cx,cy,R,a0,a0+(a1-a0)*Math.max(0.01,score/100));g.stroke();
 g.restore();
 var ea=a0+(a1-a0)*Math.max(0.01,score/100),ex=cx+Math.cos(ea)*R,ey=cy+Math.sin(ea)*R;
 g.fillStyle="#fff";g.beginPath();g.arc(ex,ey,5.5,0,6.3);g.fill();
 g.fillStyle="#e8eefc";g.font="800 54px system-ui";g.textAlign="center";g.fillText(String(Math.round(score)),cx,cy+12);
 g.fillStyle=col;g.font="800 15px system-ui";g.fillText(LV[lv],cx,cy+37);
 g.fillStyle="#6c7fa8";g.font="10.5px system-ui";g.fillText("score 0 - 100",cx,cy+56)}
var dispScore=0,dispLv=0,gaugeStarted=false;
function gaugeTick(){
 if(S){dispScore+=(S.score-dispScore)*0.09;dispLv=S.level;gauge(dispScore,dispLv)}
 requestAnimationFrame(gaugeTick)}
function line(id,series,lo,hi,unit){
 var c=$(id),f=fit(c),g=f.g,W=f.w,Hh=f.h,pl=34,pb=18,pt=8;g.clearRect(0,0,W,Hh);
 var vals=[];series.forEach(function(s){s.d.forEach(function(v){if(v!==null)vals.push(v)})});
 if(!vals.length){g.fillStyle="#8fa0c4";g.font="13px system-ui";g.fillText("no data yet",W/2-30,Hh/2);return}
 var mn=Math.min.apply(null,vals),mx=Math.max.apply(null,vals);
 if(lo!==null)mn=Math.min(mn,lo);if(hi!==null)mx=Math.max(mx,hi);if(mx-mn<4){mx+=2;mn-=2}
 var pad=(mx-mn)*0.1;mn-=pad;mx+=pad;
 g.strokeStyle="#24325a";g.fillStyle="#8fa0c4";g.font="10px system-ui";g.lineWidth=1;g.textAlign="right";
 for(var i=0;i<=4;i++){var y=pt+(Hh-pt-pb)*i/4;g.beginPath();g.moveTo(pl,y);g.lineTo(W,y);g.stroke();g.fillText((mx-(mx-mn)*i/4).toFixed(unit==="dBm"?0:1),pl-4,y+3)}
 g.textAlign="left";g.fillText("-10 min",pl,Hh-4);g.textAlign="right";g.fillText("now",W-2,Hh-4);
 series.forEach(function(s){var n=s.d.length,pts=[];
  s.d.forEach(function(v,i){if(v===null)return;var x=pl+(W-pl)*(i/Math.max(1,n-1)),y=pt+(Hh-pt-pb)*(1-(v-mn)/(mx-mn));pts.push([x,y])});
  if(pts.length>1){var grad=g.createLinearGradient(0,pt,0,Hh-pb);grad.addColorStop(0,s.c+"55");grad.addColorStop(1,s.c+"02");
   g.beginPath();g.moveTo(pts[0][0],Hh-pb);pts.forEach(function(p){g.lineTo(p[0],p[1])});g.lineTo(pts[pts.length-1][0],Hh-pb);g.closePath();g.fillStyle=grad;g.fill()}
  g.strokeStyle=s.c;g.lineWidth=2.2;g.shadowColor=s.c;g.shadowBlur=4;g.beginPath();
  pts.forEach(function(p,i){if(i==0)g.moveTo(p[0],p[1]);else g.lineTo(p[0],p[1])});g.stroke();g.shadowBlur=0})}
function timeline(al){
 var c=$("cTL"),f=fit(c),g=f.g,W=f.w,Hh=f.h,pl=98,n=NAMES.length,L=(Hh-16)/n;g.clearRect(0,0,W,Hh);g.font="11px system-ui";
 for(var i=0;i<n;i++){g.fillStyle="#8fa0c4";g.textAlign="left";g.fillText(NAMES[i],6,L*i+L/2+4);g.fillStyle="#1b2747";g.fillRect(pl,L*(i+1)-1,W-pl,1)}
 g.textAlign="left";g.fillStyle="#8fa0c4";g.fillText("-10 min",pl,Hh-3);g.textAlign="right";g.fillText("now",W-2,Hh-3);
 (al||[]).forEach(function(a){if(a.ago>600)return;var x=pl+(W-pl-8)*(1-a.ago/600),y=L*a.m+L/2;g.fillStyle=SC[a.s]||"#facc15";g.beginPath();g.arc(x,y,4+a.s,0,6.3);g.fill()})}
function modCards(){
 var m=$("mods");m.textContent="";var hot=0;
 S.mods.forEach(function(x,i){var cls="",st="OK";
  if(x.off){cls="na";st="OFF"}else if(x.age>=0&&x.age<30){cls="cr";st="ALERT";hot++}else if(x.age>=30&&x.age<300){cls="wa";st="recent"}
  if(i==3&&!S.fwok){cls="cr";st="TAMPERED"}
  var d=el("div","mod "+cls);d.appendChild(el("span","st",st));
  var h=el("b","");h.appendChild(el("span","ic",ICON[i]||""));h.appendChild(el("span","nm",NAMES[i]));var catEl=el("span","cat",CAT[i]||"");catEl.setAttribute("data-c",CAT[i]||"");h.appendChild(catEl);d.appendChild(h);
  d.appendChild(el("small","",x.off?"disabled in firmware":DESC[i]));d.appendChild(el("small","",x.cnt+" alerts total"));m.appendChild(d)});
 $("modSum").textContent=hot?hot+" active now":(S.level==0?"all quiet":"")}
function feed(){
 var t=$("feed");t.textContent="";var q=$("fQ").value.toLowerCase(),mf=$("fMod").value,n=0;
 S.alerts.forEach(function(a){if(a.s<sevMin)return;if(mf!=="all"&&String(a.m)!==mf)return;if(q&&a.x.toLowerCase().indexOf(q)<0)return;n++;
  var r=el("tr");r.appendChild(el("td","",dur(a.ago)));r.appendChild(el("td","",NAMES[a.m]));var s=el("td");s.appendChild(el("span","sev s"+a.s,a.s));r.appendChild(s);r.appendChild(el("td","",a.x));t.appendChild(r)});
 if(!n){var r=el("tr");var d=el("td","","No alerts match.");d.colSpan=4;r.appendChild(d);t.appendChild(r)}
 $("fCount").textContent=n+" shown"}
function devices(d){
 var t=$("devBody");t.textContent="";var bad=0;
 if(!d||!d.devices||!d.devices.length){var r=el("tr"),c=el("td","","No MQTT devices seen yet.");c.colSpan=5;r.appendChild(c);t.appendChild(r);$("devSum").textContent="";return}
 d.devices.forEach(function(x){if(x.everBad)bad++;var r=el("tr");r.appendChild(el("td","mono",x.id));r.appendChild(el("td","",dur(x.ago)+" ago"));
  var s=el("td"),b=el("span","sev "+(x.trusted?"s1":"s4"),x.trusted?"trusted":"UNTRUSTED");b.style.background=x.trusted?"#14391f":"";b.style.color=x.trusted?"#86efac":"";s.appendChild(b);r.appendChild(s);
  r.appendChild(el("td","",x.msgs));r.appendChild(el("td","mono",x.seq||"-"));t.appendChild(r)});
 $("devSum").textContent=bad?bad+" flagged":""}
function scout(d){
 var t=$("scoutBody");t.textContent="";
 if(!d||!d.nets||!d.nets.length){var r=el("tr"),c=el("td","","No scan yet.");c.colSpan=5;r.appendChild(c);t.appendChild(r);$("scoutSum").textContent="";return}
 d.nets.slice().sort(function(a,b){return b.rssi-a.rssi}).forEach(function(n){var r=el("tr");
  r.appendChild(el("td","",n.ssid+(n.self?" (this device)":"")));r.appendChild(el("td","mono",n.bssid));
  r.appendChild(el("td","",n.ch));r.appendChild(el("td","",n.rssi+" dBm"));r.appendChild(el("td","",n.open?"open":"secured"));t.appendChild(r)});
 $("scoutSum").textContent=d.age>=0?d.nets.length+" seen, "+d.age+"s ago":""}
function incs(){var t=$("inc");t.textContent="";
 if(!S.inc.length){var r=el("tr"),d=el("td","","No isolations yet.");d.colSpan=3;r.appendChild(d);t.appendChild(r);return}
 S.inc.forEach(function(i){var r=el("tr");r.appendChild(el("td","",dur(i.ago)+" ago"));r.appendChild(el("td","",i.dur<0?"ongoing":dur(i.dur)));r.appendChild(el("td","",i.x));t.appendChild(r)})}
function info(){
 var p=[["IP address",S.ip],["Wi-Fi network",S.ssid],["Access point",S.bssid],["Gateway MAC",S.gw],["Signal",S.rssi+" dBm"],["Firmware SHA-256",S.fw],["Log head",S.head+" #"+S.seq],["MQTT broker",S.mqtt?"connected":"offline"],["Telegram",S.tg?"enabled":"not set"],["Free heap",Math.round(S.heap/1024)+" KB"],["Log storage free",Math.round(S.free/1024)+" KB"],["Boots",S.boots],["Firmware",S.ver],["Local time",S.clock]];
 var b=$("info");b.textContent="";p.forEach(function(x){var d=el("div");d.appendChild(el("span","",x[0]));d.appendChild(el("b","",x[1]));b.appendChild(d)})}
function render(){
 var lv=S.level;$("banner").className=LC[lv];
 document.documentElement.style.setProperty("--lvcol",lv==2?"#ef4444":lv==1?"#f59e0b":"#22c55e");
 $("gaugeCard").setAttribute("data-ac",lv==2?"red":lv==1?"amber":"cyan");
 var iso=S.iso;$("bT").textContent=iso?"CRITICAL - DEVICE ISOLATED":LV[lv];
 var sub=lv==0?"All detection modules are quiet.":S.alerts.length?S.alerts[0].x:"";
 if(iso)sub="Protected device cut off by the relay. "+sub;if(lv==2&&!iso&&S.mode==1)sub="Monitor-only mode: alert raised, relay not used. "+sub;
 $("bS").textContent=sub;
 $("pMode").textContent=["ARMED","MONITOR ONLY","LOCKDOWN"][S.mode];$("pClock").textContent=S.clock;$("pUp").textContent="up "+dur(S.up);
 $("kT").textContent=S.temp===null?"--":S.temp.toFixed(1)+" C";$("kH").textContent=S.hum===null?"--":S.hum.toFixed(0)+" %";
 $("kR").textContent=S.rssi>-99?S.rssi+" dBm":"offline";$("kP").textContent=iso?"ISOLATED":"connected";$("kP").style.color=iso?"#fca5a5":"#86efac";
 $("bMute").textContent=S.muted?"🔈 Unmute alarm":"🔇 Mute alarm";
 var bs=document.querySelectorAll("#seg button");for(var i=0;i<bs.length;i++)bs[i].className=(Number(bs[i].getAttribute("data-v"))===S.mode)?"on":"";
 document.title=(lv==2?"(!) ":lv==1?"(~) ":"")+LV[lv]+" - ARGUS";
 modCards();feed();incs();info();timeline(S.alerts);
 if(lv==2&&prevLv<2)beep(880,500);else if(lv==1&&prevLv==0)beep(520,200);prevLv=lv}
async function tick(){
 try{var r=await fetch("/api/status",{cache:"no-store"});S=await r.json();$("dConn").className="dot";$("tConn").textContent="live";render()}
 catch(e){$("dConn").className="dot off";$("tConn").textContent="connection lost"}}
function sparkline(id,vals,color){
 var c=$(id);if(!c)return;var g=c.getContext("2d"),W=c.width,Hh=c.height;g.clearRect(0,0,W,Hh);
 var v2=vals.filter(function(v){return v!==null});if(v2.length<2)return;
 var mn=Math.min.apply(null,v2),mx=Math.max.apply(null,v2);if(mx-mn<0.5){mx+=0.5;mn-=0.5}
 var n=vals.length,pts=[];vals.forEach(function(v,i){if(v===null)return;var x=W*(i/Math.max(1,n-1)),y=Hh-2-(Hh-4)*(v-mn)/(mx-mn);pts.push([x,y])});
 if(pts.length<2)return;
 var grad=g.createLinearGradient(0,0,0,Hh);grad.addColorStop(0,color+"50");grad.addColorStop(1,color+"02");
 g.beginPath();g.moveTo(pts[0][0],Hh);pts.forEach(function(p){g.lineTo(p[0],p[1])});g.lineTo(pts[pts.length-1][0],Hh);g.closePath();g.fillStyle=grad;g.fill();
 g.beginPath();g.strokeStyle=color;g.lineWidth=1.6;pts.forEach(function(p,i){if(i==0)g.moveTo(p[0],p[1]);else g.lineTo(p[0],p[1])});g.stroke();
 var last=pts[pts.length-1];g.fillStyle=color;g.beginPath();g.arc(last[0],last[1],2,0,6.3);g.fill()}
async function hist(){
 try{var r=await fetch("/api/history",{cache:"no-store"});H=await r.json();
 var f=function(a,k){return a.map(function(v){return v<-9000?null:v/k})};
 var ft=f(H.t,10),fh=f(H.h,10),fr=f(H.r,1);
 line("cTH",[{d:ft,c:"#f97316"},{d:fh,c:"#38bdf8"}],null,null,"");
 line("cR",[{d:fr,c:"#a78bfa"}],-90,-30,"dBm");
 sparkline("spT",ft,"#f97316");sparkline("spH",fh,"#38bdf8");sparkline("spR",fr,"#a78bfa")}catch(e){}}
async function loadLog(){
 try{var r=await fetch("/api/log?n=30",{cache:"no-store"});var j=await r.json();logTxt=j.lines.join("\n");$("logBox").textContent=logTxt||"(log is empty)"}catch(e){$("logBox").textContent="could not load log"}}
async function loadDevices(){try{var r=await fetch("/api/devices",{cache:"no-store"});devices(await r.json())}catch(e){}}
async function loadScout(){try{var r=await fetch("/api/scan",{cache:"no-store"});scout(await r.json())}catch(e){}}
async function verify(){
 $("verR").textContent="Verifying...";
 try{var r=await fetch("/api/verify",{cache:"no-store"});var j=await r.json();verTxt=j.r;$("verR").textContent=j.r;$("verR").style.color=j.r.indexOf("OK")==0?"#86efac":"#fca5a5"}catch(e){$("verR").textContent="verify failed"}}
function askPin(){return new Promise(function(res){var m=$("modal"),i=$("pinI");m.style.display="flex";i.value="";i.focus();
 function done(v){m.style.display="none";$("pinOk").onclick=null;$("pinNo").onclick=null;i.onkeydown=null;res(v)}
 $("pinOk").onclick=function(){done(i.value)};$("pinNo").onclick=function(){done(null)};i.onkeydown=function(e){if(e.key==="Enter")done(i.value);if(e.key==="Escape")done(null)}})}
async function cmd(c,v){
 var pin=sessionStorage.getItem("pin");if(!pin){pin=await askPin();if(!pin)return;sessionStorage.setItem("pin",pin)}
 try{var r=await fetch("/api/cmd?c="+c+(v!==undefined?"&v="+v:""),{headers:{"X-Pin":pin}});
  if(r.status==401){sessionStorage.removeItem("pin");toast("Wrong PIN","bad");return}
  if(r.status==429){toast("Too many wrong PINs. Wait 60 s.","bad");return}
  var j=await r.json();toast(j.msg||"done",j.ok?"ok":"bad");tick();hist();if(c=="selftest"||c=="clear")loadLog()}
 catch(e){toast("Command failed (device unreachable)","bad")}}
function report(){
 if(!S){return}
 var L=[];L.push("ARGUS INCIDENT REPORT");L.push("Generated: "+new Date().toString());L.push("Device time: "+S.clock+"   Uptime: "+dur(S.up));L.push("");
 L.push("State: "+LV[S.level]+(S.iso?" (device isolated)":"")+"   Mode: "+["Armed","Monitor only","Lockdown"][S.mode]+"   Threat score: "+Math.round(S.score));
 L.push("Firmware SHA-256: "+S.fw+"   Log head: "+S.head+" #"+S.seq);L.push("Chain verification: "+verTxt);L.push("");
 L.push("MODULE SUMMARY");S.mods.forEach(function(m,i){L.push("  "+NAMES[i]+": "+m.cnt+" alerts"+(m.off?" (disabled)":""))});L.push("");
 L.push("INCIDENTS (isolations)");if(!S.inc.length)L.push("  none");S.inc.forEach(function(i){L.push("  "+dur(i.ago)+" ago, "+(i.dur<0?"ongoing":dur(i.dur))+" - "+i.x)});L.push("");
 L.push("RECENT ALERTS (newest first)");S.alerts.forEach(function(a){L.push("  "+dur(a.ago)+" ago  ["+NAMES[a.m]+"]  sev "+a.s+"  "+a.x)});L.push("");
 L.push("FORENSIC LOG (last 30 entries)");L.push(logTxt||"(not loaded)");
 var b=new Blob([L.join("\n")],{type:"text/plain"}),a=document.createElement("a");a.href=URL.createObjectURL(b);a.download="argus_incident_report.txt";a.click();setTimeout(function(){URL.revokeObjectURL(a.href)},2000)}
function init(){
 var cardEls=document.querySelectorAll(".card");for(var ci=0;ci<cardEls.length;ci++)cardEls[ci].style.animationDelay=(ci*0.035)+"s";
 var ms=$("simM"),fm=$("fMod");var o=el("option","","All modules");o.value="all";fm.appendChild(o);
 NAMES.forEach(function(n,i){var a=el("option","",n);a.value=i;ms.appendChild(a);var b=el("option","",n);b.value=i;fm.appendChild(b)});
 var chips=$("chips");[["All",0],["Medium+",2],["High+",3],["Critical",4]].forEach(function(x){var b=el("button","chip"+(x[1]==0?" on":""),x[0]);b.onclick=function(){sevMin=x[1];var k=chips.children;for(var i=0;i<k.length;i++)k[i].className="chip";b.className="chip on";if(S)feed()};chips.appendChild(b)});
 $("fMod").onchange=function(){if(S)feed()};$("fQ").oninput=function(){if(S)feed()};
 $("bIso").onclick=function(){if(confirm("Cut power to the protected device now?"))cmd("isolate")};
 $("bRel").onclick=function(){cmd("release")};
 $("bMute").onclick=function(){cmd("mute",S&&S.muted?0:1)};
 $("bSnd").onclick=function(){sound=!sound;$("bSnd").textContent=(sound?"🔊":"🔈")+" Dashboard sound: "+(sound?"on":"off");if(sound)beep(660,150)};
 var sb=document.querySelectorAll("#seg button");for(var i=0;i<sb.length;i++)sb[i].onclick=function(){cmd("mode",this.getAttribute("data-v"))};
 $("bST").onclick=function(){cmd("selftest")};$("bRT").onclick=function(){cmd("relaytest")};
 $("bRB").onclick=function(){cmd("rebase")};
 $("bTR").onclick=function(){if(confirm("Accept the running firmware as the new trusted image?"))cmd("trust")};
 $("bCL").onclick=function(){cmd("clear")};$("bRP").onclick=report;
 $("bS1").onclick=function(){cmd("scenario",1)};$("bS2").onclick=function(){cmd("scenario",2)};
 $("bSim").onclick=function(){cmd("sim",$("simM").value)};
 $("bVer").onclick=verify;$("bLog").onclick=loadLog;
 window.addEventListener("resize",function(){if(S){timeline(S.alerts)}hist()});
 tick();hist();loadLog();loadDevices();loadScout();setInterval(tick,1500);setInterval(hist,5000);setInterval(loadLog,15000);setInterval(loadDevices,4000);setInterval(loadScout,16000);
 requestAnimationFrame(gaugeTick)}
init();
</script></body></html>)=====";