#ifndef WEBUI_PAGE_H
#define WEBUI_PAGE_H

static const char WEBUI_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SofronioDRS</title>
<style>
:root{color-scheme:dark;--bg:#101418;--card:#1a2027;--fg:#e8eef5;--muted:#8fa1b3;--accent:#4cc2ff;--ok:#4ade80;--err:#f87171}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--fg);font:15px/1.45 system-ui,-apple-system,Segoe UI,sans-serif}
header{display:flex;align-items:center;justify-content:space-between;padding:14px 18px;background:#151b22;border-bottom:1px solid #26303a}
h1{font-size:18px;margin:0}h2{font-size:15px;margin:0 0 10px;color:var(--muted);font-weight:600}
main{max-width:760px;margin:0 auto;padding:14px;display:grid;gap:14px}
.card{background:var(--card);border:1px solid #26303a;border-radius:14px;padding:16px}
.weightCard{display:flex;align-items:baseline;gap:10px}
#weight{font-size:64px;font-weight:700;letter-spacing:-2px;line-height:1}
#unit{font-size:20px;color:var(--muted)}
.chips{display:flex;flex-wrap:wrap;gap:8px;margin-top:10px}
.chip{font-size:12px;padding:4px 9px;border-radius:999px;background:#232c36;color:var(--muted);border:1px solid #2e3a46}
.chip b{color:var(--fg);font-weight:600}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(130px,1fr));gap:8px}
button,select,input[type=file]{font:inherit;border-radius:10px;border:1px solid #34414f;background:#222b35;color:var(--fg);padding:9px 12px}
button{background:#25313d;cursor:pointer}button:hover{border-color:var(--accent)}
button:disabled{opacity:.4;cursor:not-allowed}
button.primary{background:#0f5f86;border-color:#1487bd}button.danger{background:#5c2020;border-color:#8f3232}
input[type=number],input[type=password]{font:inherit;border-radius:10px;border:1px solid #34414f;background:#161d24;color:var(--fg);padding:9px 12px;width:150px}
.row{display:flex;flex-wrap:wrap;gap:8px;align-items:center;margin:8px 0}
label{color:var(--muted);font-size:13px}
.msg{font-size:13px;color:var(--muted);min-height:18px;margin-top:8px}
.ok{color:var(--ok)}.err{color:var(--err)}
canvas{background:#000;border-radius:8px;border:1px solid #2e3a46;max-width:100%;image-rendering:pixelated}
#conn{font-size:12px;color:var(--muted)}#conn.on{color:var(--ok)}
.pluginRow{display:flex;justify-content:space-between;align-items:center;gap:8px;padding:6px 0;border-bottom:1px solid #26303a;font-size:13px}
footer{padding:10px 18px 26px;text-align:center}
small{color:var(--muted)}
</style>
</head>
<body>
<header><h1>SofronioDRS <small>Web UI</small></h1><span id="conn">connecting…</span></header>
<main>

<section class="card">
  <div class="weightCard"><div id="weight">--.-</div><div id="unit">g</div></div>
  <div class="chips" id="chips"></div>
</section>

<section class="card">
  <h2>Mode</h2>
  <div class="row">
    <button id="extBtn" class="primary" onclick="toggleExtraction()">Enter extraction</button>
    <span id="extState" class="msg"></span>
  </div>
  <small>Pure weighing ↔ extraction. The extraction timer below only works in extraction mode.</small>
</section>

<section class="card">
  <h2>Extraction timer</h2>
  <div class="row">
    <button class="timerBtn" onclick="api('/api/timer',{action:'start'})">Start</button>
    <button class="timerBtn" onclick="api('/api/timer',{action:'stop'})">Stop</button>
    <button class="timerBtn" onclick="api('/api/timer',{action:'reset'})">Reset</button>
  </div>
  <div class="msg" id="timerState"></div>
</section>

<section class="card">
  <h2>Controls</h2>
  <div class="grid">
    <button onclick="api('/api/tare')">Tare 去皮</button>
    <button onclick="api('/api/mode',{value:'toggle'})">Espresso / Pour-over</button>
    <button onclick="api('/api/rotate')">Rotate 旋转</button>
    <button onclick="api('/api/menu',{show:'1'})">Show Menu</button>
    <button onclick="api('/api/menu',{show:'0'})">Hide Menu</button>
    <button onclick="api('/api/device-info')">Device Info</button>
    <button class="danger" onclick="if(confirm('Power off?'))api('/api/power')">Power Off</button>
  </div>
</section>

<section class="card">
  <h2>Calibrate with actual weight</h2>
  <div class="row">
    <input id="calW" type="number" step="0.1" placeholder="Actual weight (g)">
    <button class="primary" onclick="calibrateWeight()">Calibrate</button>
  </div>
  <div class="msg" id="calMsg"></div>
</section>

<section class="card">
  <h2>Guided calibration (no buttons needed)</h2>
  <div class="row">
    <select id="calIdx">
      <option value="1">50 g</option>
      <option value="2">100 g</option>
      <option value="3">200 g</option>
      <option value="4" selected>500 g</option>
      <option value="5">1000 g</option>
    </select>
    <button class="primary" onclick="calStart()">Start</button>
    <button onclick="calConfirm()">Continue</button>
    <button class="danger" onclick="api('/api/cal/cancel')">Cancel</button>
  </div>
  <div class="msg" id="calState"></div>
  <small>Start → remove all weight → Continue. Place the selected weight when the device countdown starts. Results are validated on the device.</small>
</section>

<section class="card">
  <h2>Container</h2>
  <div class="row">
    <button onclick="api('/api/container/record')">Record current weight</button>
    <button onclick="api('/api/container/clear')">Clear container</button>
  </div>
  <div class="msg" id="container"></div>
</section>

<section class="card">
  <h2>OLED logo (128×64)</h2>
  <div class="row">
    <input type="file" id="logoFile" accept="image/*">
    <label>Threshold <input type="range" id="thr" min="0" max="255" value="128"></label>
    <label><input type="checkbox" id="inv"> Invert</label>
  </div>
  <div class="row">
    <div><small>Current logo on scale</small><br><canvas id="currentPreview" width="256" height="128"></canvas></div>
    <div><small>New image preview</small><br><canvas id="preview" width="256" height="128"></canvas></div>
  </div>
  <div class="row">
    <button onclick="downloadLogo()">Download current (.xbm)</button>
    <button onclick="downloadCurrentPng()">Download current (.png)</button>
    <button onclick="downloadPreviewPng()">Download preview (.png)</button>
  </div>
  <div class="row">
    <button class="danger" onclick="restoreLogo()">Restore built-in logo</button>
    <button class="primary" onclick="uploadLogo()">Upload & Show</button>
    <button onclick="api('/api/logo/show')">Show on scale</button>
  </div>
  <div class="msg" id="logoMsg"></div>
  <small>PNG/JPG are converted in the browser to a 128×64 1-bit XBM (1024 bytes). Uploaded logos override the firmware logo until you restore it.</small>
</section>

<section class="card">
  <h2>WiFi</h2>
  <div class="msg" id="wifiStatus"></div>
  <div class="row">
    <select id="wifiSsid"></select>
    <input id="wifiPass" type="password" placeholder="Password">
  </div>
  <div class="row">
    <button class="primary" onclick="wifiConnect()">Join WiFi</button>
    <button onclick="wifiRescan()">Rescan</button>
    <button onclick="api('/api/wifi/hotspot')">Switch to hotspot</button>
  </div>
  <div class="msg" id="wifiMsg"></div>
  <small>Joining keeps the hotspot during the test. On success the hotspot closes after 10 s; open http://drs.local or the shown IP. Rescan may briefly disconnect the page.</small>
</section>

<section class="card">
  <h2>Plugins</h2>
  <div id="pluginList"></div>
  <div class="row">
    <input type="file" id="pluginFile" accept=".js,text/javascript">
    <button onclick="uploadPlugin()">Upload JS plugin</button>
  </div>
  <div class="msg" id="pluginMsg"></div>
  <div id="pluginArea"></div>
  <small>Plugins are ordinary JS files stored on the scale and loaded into this page.</small>
</section>

</main>
<footer><button class="danger" onclick="if(confirm('Stop Web UI and WiFi?'))api('/api/webui/stop')">Stop Web UI</button></footer>

<script>
const $ = id => document.getElementById(id);
let status = {}, ws, logoBytes = null, currentLogoBytes = null, currentImg = null, currentImgUrl = null;
const statusListeners = [], eventListeners = [];

function api(path, params) {
  let url = path;
  if (params) url += '?' + new URLSearchParams(params).toString();
  return fetch(url, {method: 'POST'}).then(r => r.text());
}
function fmt(n, d = 1) { return (Number(n) || 0).toFixed(d); }

function render() {
  if (!status || status.type !== 'status') return;
  $('weight').textContent = fmt(status.g, 1);
  const chips = [];
  chips.push(`<span class="chip">Mode <b>${status.mode ? 'Espresso' : 'Pour-over'}</b></span>`);
  chips.push(`<span class="chip">${status.ext ? 'Extraction' : 'Weighing'}</span>`);
  chips.push(`<span class="chip">Timer <b>${status.timer ? status.secs + 's' : 'idle'}</b></span>`);
  chips.push(`<span class="chip">BLE <b>${status.ble ? 'on' : 'off'}</b></span>`);
  chips.push(`<span class="chip">Lang <b>${status.lang ? 'EN' : '中文'}</b></span>`);
  chips.push(`<span class="chip">Samples <b>${status.samples}</b></span>`);
  $('chips').innerHTML = chips.join('');
  const ext = !!status.ext;
  $('extBtn').textContent = ext ? 'Exit extraction' : 'Enter extraction';
  $('extState').textContent = ext ? 'Extraction mode' : 'Pure weighing';
  document.querySelectorAll('.timerBtn').forEach(b => { b.disabled = !ext; });
  $('timerState').textContent = ext
    ? (status.timer ? 'Running — ' + status.secs + ' s' : 'Stopped')
    : 'Enter extraction mode to use the timer.';
  $('container').textContent = 'Container: ' + fmt(status.container, 1) + ' g';
  const wsMap = {0: 'Hotspot only', 1: 'Connecting…', 2: 'STA connected', 3: 'Failed'};
  let wifiInfo = `<span class="chip">WiFi <b>${wsMap[status.wifiState] || '—'}</b></span>`;
  if (status.apIp) wifiInfo += `<span class="chip">AP <b>${status.apIp}</b></span>`;
  if (status.staIp) wifiInfo += `<span class="chip">STA <b>${status.staSsid || ''} ${status.staIp}</b></span>`;
  wifiInfo += `<span class="chip">mDNS <b>${status.mdns || 'drs.local'}</b></span>`;
  $('wifiStatus').innerHTML = wifiInfo;
  if (status.staIp) {
    $('wifiMsg').textContent = 'Open http://drs.local or http://' + status.staIp +
      (status.apIp ? ' — hotspot closes about 10 s after connecting.' : '');
    $('wifiMsg').className = 'msg ok';
  } else if (status.wifiError) {
    $('wifiMsg').textContent = status.wifiError;
    $('wifiMsg').className = 'msg err';
  }
  if (status.cal) {
    const steps = {1: 'select weight', 2: 'zero capture', 3: 'load capture'};
    $('calState').textContent = 'Calibrating — ' + (steps[status.calStep] || ('step ' + status.calStep)) +
      (status.calStatus && status.calStatus !== 'ok' ? ' — ' + status.calStatus : '');
  } else if (status.calStatus && status.calStatus !== 'ok') {
    $('calState').textContent = 'Last calibration: ' + status.calStatus;
  } else if (status.calStatus === 'ok') {
    $('calState').textContent = 'Last calibration: ok';
  }
  statusListeners.forEach(fn => { try { fn(status); } catch (e) {} });
}

function handleEvent(m) {
  if (m.type === 'cal_result') {
    $('calMsg').textContent = `Calibration status ${m.status} — measured ${fmt(m.measured, 2)} g, new factor ${fmt(m.cal, 2)}`;
    $('calMsg').className = 'msg ' + (m.status === 0 ? 'ok' : 'err');
  } else if (m.type === 'container_result') {
    $('container').textContent = `Container status ${m.status} — ${fmt(m.container, 2)} g`;
  }
  eventListeners.forEach(fn => { try { fn(m); } catch (e) {} });
}

function connect() {
  ws = new WebSocket(`ws://${location.host}/ws`);
  ws.onopen = () => { $('conn').textContent = 'connected'; $('conn').className = 'on'; };
  ws.onclose = () => { $('conn').textContent = 'disconnected'; $('conn').className = ''; setTimeout(connect, 1500); };
  ws.onerror = () => {};
  ws.onmessage = e => {
    try {
      const m = JSON.parse(e.data);
      if (m.type === 'status') { status = m; render(); }
      else handleEvent(m);
    } catch (err) {}
  };
}
async function poll() {
  try { const r = await fetch('/api/status'); status = await r.json(); render(); } catch (e) {}
}
setInterval(poll, 1200);

function toggleExtraction() {
  api('/api/extraction', {value: status.ext ? 'off' : 'on'});
}

async function loadWifiScan() {
  try {
    const list = await (await fetch('/api/wifi/scan')).json();
    const sel = $('wifiSsid');
    const current = sel.value;
    sel.innerHTML = '';
    for (const n of list) {
      const o = document.createElement('option');
      o.value = n.ssid;
      o.textContent = n.ssid + ' (' + n.rssi + ' dBm)' + (n.enc ? ' 🔒' : '');
      sel.appendChild(o);
    }
    if (current) sel.value = current;
  } catch (e) {}
}
function wifiConnect() {
  const ssid = $('wifiSsid').value;
  const password = $('wifiPass').value;
  if (!ssid) { $('wifiMsg').textContent = 'Select an SSID first.'; return; }
  $('wifiMsg').textContent = 'Connecting… keep this page open. On success the hotspot closes in ~10 s; use drs.local or the shown IP.';
  $('wifiMsg').className = 'msg';
  api('/api/wifi/connect', {ssid, password});
}
function wifiRescan() {
  $('wifiMsg').textContent = 'Scanning… the page may briefly disconnect.';
  api('/api/wifi/rescan');
  setTimeout(loadWifiScan, 5000);
}

function calibrateWeight() {
  const w = $('calW').value;
  $('calMsg').className = 'msg';
  $('calMsg').textContent = 'Calibrating…';
  api('/api/calibrate', {weight: w}).then(t => {
    try { const j = JSON.parse(t); if (j.error) { $('calMsg').textContent = j.error; $('calMsg').className = 'msg err'; } } catch (e) {}
  });
}
function calStart() { api('/api/cal/start', {index: $('calIdx').value}); $('calState').textContent = 'Started — remove all weight, then Continue.'; }
function calConfirm() { api('/api/cal/confirm'); $('calState').textContent = 'Continue sent — place the weight during the device countdown.'; }

$('logoFile').onchange = e => {
  const f = e.target.files[0];
  if (!f) return;
  if (currentImgUrl) URL.revokeObjectURL(currentImgUrl);
  currentImgUrl = URL.createObjectURL(f);
  currentImg = new Image();
  currentImg.onload = () => convertLogo();
  currentImg.src = currentImgUrl;
};
$('thr').oninput = () => { if (currentImg) convertLogo(); };
$('inv').onchange = () => { if (currentImg) convertLogo(); };

function convertLogo() {
  const c = document.createElement('canvas'); c.width = 128; c.height = 64;
  const ctx = c.getContext('2d');
  ctx.fillStyle = '#000'; ctx.fillRect(0, 0, 128, 64);
  const s = Math.min(128 / currentImg.width, 64 / currentImg.height);
  const w = currentImg.width * s, h = currentImg.height * s;
  ctx.drawImage(currentImg, (128 - w) / 2, (64 - h) / 2, w, h);
  const d = ctx.getImageData(0, 0, 128, 64).data;
  const bytes = new Uint8Array(1024);
  const thr = +$('thr').value, inv = $('inv').checked;
  // U8g2 XBMP: row-major, 16 bytes per row, LSB = leftmost pixel
  for (let y = 0; y < 64; y++) {
    for (let x = 0; x < 128; x++) {
      const i = (y * 128 + x) * 4;
      const lum = 0.299 * d[i] + 0.587 * d[i + 1] + 0.114 * d[i + 2];
      let on = lum >= thr;
      if (inv) on = !on;
      if (on) bytes[y * 16 + (x >> 3)] |= 1 << (x & 7);
    }
  }
  logoBytes = bytes;
  drawLogoTo('preview', logoBytes);
}
function drawLogoTo(id, bytes) {
  const c = $(id); if (!c) return;
  const ctx = c.getContext('2d');
  ctx.fillStyle = '#000'; ctx.fillRect(0, 0, c.width, c.height);
  ctx.fillStyle = '#fff';
  const sx = c.width / 128, sy = c.height / 64;
  for (let y = 0; y < 64; y++) {
    for (let x = 0; x < 128; x++) {
      if (bytes[y * 16 + (x >> 3)] & (1 << (x & 7))) ctx.fillRect(x * sx, y * sy, sx, sy);
    }
  }
}
async function loadCurrentLogo() {
  try {
    const r = await fetch('/api/logo/current');
    const ab = await r.arrayBuffer();
    if (ab.byteLength === 1024) {
      currentLogoBytes = new Uint8Array(ab);
      drawLogoTo('currentPreview', currentLogoBytes);
    }
  } catch (e) {}
}
function downloadLogo() {
  if (!currentLogoBytes) return;
  const blob = new Blob([currentLogoBytes], {type: 'application/octet-stream'});
  const a = document.createElement('a');
  a.href = URL.createObjectURL(blob);
  a.download = 'logo.xbm';
  a.click();
  setTimeout(() => URL.revokeObjectURL(a.href), 1000);
}
function downloadCurrentPng() {
  if (!currentLogoBytes) return;
  const c = $('currentPreview');
  const a = document.createElement('a');
  a.href = c.toDataURL('image/png');
  a.download = 'logo-current.png';
  a.click();
}
function downloadPreviewPng() {
  if (!logoBytes) return;
  const c = $('preview');
  const a = document.createElement('a');
  a.href = c.toDataURL('image/png');
  a.download = 'logo-preview.png';
  a.click();
}
async function restoreLogo() {
  await api('/api/logo/reset');
  await loadCurrentLogo();
  await api('/api/logo/show');
  $('logoMsg').textContent = 'Built-in logo restored.';
  $('logoMsg').className = 'msg ok';
}
async function uploadLogo() {
  if (!logoBytes) { $('logoMsg').textContent = 'Choose an image first.'; return; }
  const r = await fetch('/api/logo', {method: 'POST', headers: {'Content-Type': 'application/octet-stream'}, body: logoBytes});
  const t = await r.text();
  $('logoMsg').textContent = r.ok ? 'Uploaded.' : t;
  $('logoMsg').className = 'msg ' + (r.ok ? 'ok' : 'err');
  if (r.ok) {
    await loadCurrentLogo();
    api('/api/logo/show');
  }
}

async function loadPlugins() {
  let list = [];
  try { list = await (await fetch('/api/plugins')).json(); } catch (e) {}
  const box = $('pluginList');
  box.innerHTML = '';
  for (const name of list) {
    const row = document.createElement('div');
    row.className = 'pluginRow';
    row.innerHTML = '<span></span>';
    row.firstChild.textContent = name;
    const del = document.createElement('button');
    del.textContent = 'Delete';
    del.onclick = async () => { await api('/api/plugin/delete', {name}); loadPlugins(); };
    row.appendChild(del);
    box.appendChild(row);
    const s = document.createElement('script');
    s.src = '/plugins/' + encodeURIComponent(name);
    s.onerror = () => console.log('plugin load failed', name);
    document.body.appendChild(s);
  }
}
async function uploadPlugin() {
  const f = $('pluginFile').files[0];
  if (!f) { $('pluginMsg').textContent = 'Choose a .js file.'; return; }
  const name = f.name.replace(/[^A-Za-z0-9._-]/g, '_');
  const text = await f.text();
  const r = await fetch('/api/plugin?name=' + encodeURIComponent(name), {
    method: 'POST', headers: {'Content-Type': 'application/octet-stream'}, body: text
  });
  $('pluginMsg').textContent = r.ok ? 'Uploaded ' + name : 'Upload failed';
  $('pluginMsg').className = 'msg ' + (r.ok ? 'ok' : 'err');
  if (r.ok) loadPlugins();
}

window.ScaleWeb = {
  get status() { return status; },
  api,
  onStatus(fn) { statusListeners.push(fn); if (status) try { fn(status); } catch (e) {} },
  onEvent(fn) { eventListeners.push(fn); },
  addPanel(title, html) {
    const card = document.createElement('section'); card.className = 'card';
    card.innerHTML = '<h2></h2><div></div>';
    card.querySelector('h2').textContent = title;
    card.querySelector('div').innerHTML = html;
    $('pluginArea').appendChild(card);
    return card;
  },
  addButton(label, fn) {
    const b = document.createElement('button');
    b.textContent = label; b.onclick = fn;
    $('pluginArea').appendChild(b);
    return b;
  }
};

connect();
loadPlugins();
loadCurrentLogo();
loadWifiScan();
poll();
</script>
</body>
</html>
)rawliteral";

#endif
