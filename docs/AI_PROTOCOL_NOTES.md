# AI Protocol Notes

## BLE

- Device name `Decent Scale`; service `FFF0`, write characteristic `36F5`,
  read/notify characteristic `FFF4`.
- Frames start with model byte `0x03`. Incoming checksums are XOR of all preceding bytes;
  outgoing weight/button/power frames follow the Decent Scale convention (see `calculateXOR`).
- Weight notifications: 7 bytes `03 CE <weightHi> <weightLo> 00 00 XOR`, value = grams × 10.
- Button events: `03 AA <button 0|1> <long|short> 00 00 XOR`.
- Power-off events: `03 2A <reason> ...`.

### Standard Decent commands (write to `36F5`)

| Bytes | Meaning |
| --- | --- |
| `03 0F 00 00 00 00 0C` | Tare |
| `03 0A <00/01/02>` | Display off/on, power off |
| `03 0B <03/00/02>` | Timer start/stop/zero |
| `03 1A <mode>` | Calibration: 0 manual, 1 smart (disabled → failure screen) |
| `03 1C <00/01/02>` | Buzzer off/on/beep |
| `03 1D <00/01/03>` | Samples 1/2/4 |
| `03 1E <group> <show>` | group 0 menu, 1 device info, 2 debug; show 0/1 |
| `03 1F` | Reset (`ESP.restart()`) |

### Vendor commands (custom range)

| Bytes | Meaning | Response |
| --- | --- | --- |
| `03 30 <float32 LE grams> XOR` | Calibrate using a user-supplied actual weight | `03 B0 <status> <measured f32> <newCal f32> XOR` |
| `03 31 XOR` | Record current weight as container | `03 B1 <status> <measured f32> <container f32> XOR` |

Status codes for B0/B1: `0` ok, `1` invalid packet, `2` no weight, `3` unstable, `4` invalid factor / EEPROM failure (see `calibration.md`).

### Heartbeat

- On connect `t_firstConnect = millis()`. Any write updates `t_heartBeat`.
- If connected with `b_requireHeartBeat` and both timestamps are older than 5 s, the
  client is disconnected. Keep clients writing regularly.

## USB serial (CH340, 115200, newline-terminated)

`calw <g>`, `ctr`, `ct <g>`, `cv <factor>`, `cp <g>`, `ce <g>`, `vf <V>`, `v`,
`tare`, `set`, `cal`, `ota`, `beep`, `reset`. Calibration/container commands share the
BLE validation logic.

## Calibration state machine (device screen)

- Triggered by menu or `0x1A 00`; `i_button_cal_status`: 1 select weight, 2 zero capture,
  3 load capture. Set key cycles weight, Tare confirms/advances.
- Smart calibration (`0x1A 01`) is intentionally disabled and returns
  `CAL_REJECT_SMART_CAL_DISABLED`.
- Failure restores the previous factor and does not write EEPROM.

## Web UI (manual start: menu Connections -> Web UI, or USB `web`)

- SoftAP SSID `D.R.S`, password `12345678`, IP `192.168.1.1`; stop via page/USB.
- `GET /api/status` JSON; `GET /ws` WebSocket status at ~10Hz.
- POST APIs: `/api/tare`, `/api/extraction?value=on|off|toggle`,
  `/api/timer?action=start|stop|reset` (extraction mode only),
  `/api/mode?value=espresso|pour|toggle`, `/api/rotate`, `/api/menu?show=0|1`,
  `/api/device-info`, `/api/power`, `/api/webui/stop`,
  `/api/calibrate?weight=<g>`, `/api/container/record`, `/api/container/clear`,
  `/api/cal/start?index=N`, `/api/cal/select?index=N`, `/api/cal/confirm`, `/api/cal/cancel`,
  `/api/logo` (raw 1024 bytes), `/api/logo/show`, `GET /api/logo/current`,
  `/api/logo/reset`.
- Plugins: `GET /api/plugins`, `POST /api/plugin?name=x.js` (raw body),
  `POST /api/plugin/delete?name=x.js`; files served from `/plugins/`.
- Charging status: `03 32 XOR` → `03 B2 <charging> <USB_DET> <BATTERY_CHARGING> <batteryV f32> <usbV f32> XOR`
  (14 bytes). On V8.1 the USB_DET/CHRG digital pins do not change on this unit; firmware uses a hybrid:
  CHRG low, or USB_DET low + battery-rail voltage >4.4 V / USB_DET analog >3 V. Battery sampled every 500 ms.
- WiFi: menu `Start Hotspot` (AP only) or `Join WiFi` (scan → AP → AP+STA provisioning).
  APIs: `GET /api/wifi/scan`, `POST /api/wifi/rescan`, `POST /api/wifi/connect?ssid=&password=`,
  `POST /api/wifi/hotspot`. Status JSON adds `wifiState`, `staIp`, `staSsid`, `apIp`,
  `wifiError`, `mdns` (`drs.local`). On STA success the AP closes after 10 s; on failure the AP
  stays and the page shows the error. Credentials live in NVS Preferences namespace `wifi`.
- Actions are queued (`i_webAction`) and handled by the main loop.

## Checks

- `03 1D 00 1E` (standard sample command with checksum) must be ignored, not treated as a
  vendor command. Vendor IDs are `0x30/0x31` for this reason.
- `0x9C/0x9D` response IDs are unused by the standard protocol; current firmware uses
  `0xB0/0xB1`.
