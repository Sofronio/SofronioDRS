// Example SofronioDRS Web UI plugin.
// Upload through the Web UI -> Plugins section (stored in LittleFS /plugins/).
// It demonstrates status subscription, API calls and adding UI.

const card = ScaleWeb.addPanel(
  'Example Plugin',
  '<p>Live weight: <b class="ep-weight">--</b> g</p>' +
  '<p class="ep-mode"></p>'
);

const tareButton = ScaleWeb.addButton('Tare (plugin)', () => {
  ScaleWeb.api('/api/tare');
});

ScaleWeb.onStatus(status => {
  const w = card.querySelector('.ep-weight');
  if (w) w.textContent = (Number(status.g) || 0).toFixed(2);
  const m = card.querySelector('.ep-mode');
  if (m) m.textContent = status.mode ? 'Espresso' : 'Pour-over';
});

ScaleWeb.onEvent(event => {
  if (event.type === 'cal_result') {
    console.log('calibration result', event);
  }
});
