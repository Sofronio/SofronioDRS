# SofronioDRS

ESP32-S3 coffee scale firmware with BLE, two weighing modes and a bilingual (English / Chinese) menu.

This firmware runs on the **Decent HDS (Half Decent Scale) hardware** — the same board as the
Decent Open Scale (ESP32-S3N16, ADS1232 24-bit ADC, SH1106 OLED, BMA400 accelerometer, TP4056
charger, CH340K USB-serial, REF5025 reference, 2 kg load cell). The default PlatformIO
environment targets **PCB v8.1**; other board revisions (v8.0 / v7.5 / v7.2 and the original
ESP32 board) are available as separate environments.

Current firmware version: **6.1.3** · Default PCB: **8.1**.

---

## Features

- **BLE Decent Scale protocol** — weight notifications, tare, timer, LED/power, calibration;
  plus vendor commands for weight-based calibration and container recording.
- **Two weighing modes** — pour-over and espresso, with extraction timer UI.
- **Two-level menu** with runtime **Chinese / English** switch (persisted in EEPROM).
- **Guided calibration** using openscale-style stability capture, range/sign validation and
  post-calibration verification; also calibration from a user-supplied actual weight over BLE/USB.
- **Container weight** record, clear and automatic subtraction.
- **Continuous drift compensation** (off / 0.05 / 0.075 / 0.1 / 0.2 g selectable).
- **Selectable ADS1232 averaging** (1 / 2 / 4 samples) and 16-sample calibration capture.
- **WiFi OTA** (ElegantOTA access point).
- **Pour-over auto-stop** — after the weight is stable, removing the cup (> 50 g drop) stops the timer.
- **BLE controls** — runtime BLE on/off, client heartbeat monitoring, physical-button behaviour
  while connected, screen flip, time-on-top.

---

## Hardware compatibility

- **Decent HDS / Half Decent Scale / Decent Open Scale** hardware.
- Default environment: `esp32s3_v8_1` (PCB v8.1).
- Other environments in `platformio.ini`: `esp32s3_v8_0`, `esp32s3_v7_5`, `esp32s3_v7_2`, `esp32`.
- USB-serial: CH340 at **115200** baud.

---

## Build and flash

Requires [PlatformIO](https://platformio.org/).

```bash
# build and flash the default PCB v8.1 target
pio run -e esp32s3_v8_1 -t upload --upload-port /dev/cu.wchusbserialXXX

# serial monitor (115200)
pio device monitor --port /dev/cu.wchusbserialXXX --baud 115200
```

The firmware prints boot diagnostics (calibration value, container, samples, language) on the
serial console. Note that the scale must be powered on for the console to answer; if you open the
port with DTR/RTS asserted the board may be held in reset — use `dtr = false`, `rts = false`
when capturing logs programmatically.

---

## Using the scale

Two touch buttons are used:

| Button | Label on board | Code name |
| --- | --- | --- |
| Left | `O` (circle) | `SET` |
| Right | `[]` (square) | `TARE` |

### Power on (and the boot-time BLE switch)

Hold one button for about 1 second to power on:

| Action | Result |
| --- | --- |
| Hold **left `O`** ~1 s | Power on with **BLE ON** |
| Hold **right `[]`** ~1 s | Power on with **BLE OFF** |
| Short press | Power on only if **Quick Boot** is enabled (`Power → Quick Boot`) |
| USB / reset / charging wake | BLE uses the saved menu setting (default **ON**) |

The menu toggle `Connections → Bluetooth` changes the saved setting at any time.

### Weighing ↔ extraction

| Action | Result |
| --- | --- |
| Short press **left `O`** in weighing mode | Enter **extraction mode** |
| Long press **left `O`** in extraction mode | Return to **weighing mode** |
| Long press **right `[]`** in weighing mode | Switch **Espresso ↔ Pour-over** (saved) |
| Short press **right `[]`** in weighing mode | **Tare** |

In extraction mode:

| Action | Result |
| --- | --- |
| Short press **left `O`** | Timer start / stop / reset |
| Short press **right `[]`** | Stop the timer (or reset it when stopped) |
| Long press **right `[]`** | Rotate the display 180° |
| Remove the cup after the weight is stable (pour-over) | Timer stops automatically when the reading drops > 50 g |

### Other button behaviour

- **Double-click either button** → power off (disabled while in menu, calibration, device info or
  debug screens).
- Long press **left `O`** in weighing mode toggles container subtraction mode.
- Long presses never trigger a tare.

### Enter the menu

**Power on while holding both buttons** and keep holding until the menu appears. The menu is
entered at boot only.

| Action | Result |
| --- | --- |
| Left `O` | Next item |
| Right `[]` | Select / enter |
| `Back` item | Return to the previous menu level |
| `Exit` item | Leave the menu |

Toggle items show their current state (e.g. `Auto Sleep: ON`); selecting one flips and saves it.

---

## Menu reference

```
Exit
Scale
├─ Calibrate            Guided calibration: choose weight → remove all weight (0 g capture)
│                       → place weight → stability/range/sign validation → verification
├─ Drift                Drift compensation: Off / 0.05 / 0.075 / 0.1 / 0.2 g
├─ Auto Container
│  ├─ Record Container  Save the current reading as container weight
│  ├─ Clear Container   Set container weight to 0
│  └─ Auto Subtract     Enable/disable container subtraction
└─ Samples              Averaging window: 1 / 2 / 4 (saved)

Connections
├─ Bluetooth            BLE on/off (saved; also selectable at power-on)
├─ Heartbeat            Require client writes; disconnect after 5 s of silence
├─ BLE Buttons          Allow physical button functions while a BLE client is connected
└─ WiFi Update          Start the OTA access point (SSID D.R.S / password 12345678)

Display
├─ Buzzer               Buzzer on/off
├─ Flip Screen          Rotate the UI 180°
├─ Time on Top          Timer position on the weighing screen
└─ Language             中文 / English

Power
├─ Auto Sleep           Automatic power-off after inactivity
└─ Quick Boot           Short-press power-on (instead of holding ~1 s)

Info
└─ Device Info          Firmware / PCB / serial number; any button returns to the menu
```

Menu settings are stored in EEPROM. The calibration value (address 0) and container weight
(address 4) keep their original locations; new settings are appended (language, BLE state,
auto-subtract, samples, settings version).

---

## BLE interface

- Service `FFF0`, write characteristic `36F5`, notify characteristic `FFF4`.
- Weight notifications (`0xCE`) at 10 Hz.
- Heartbeat: after connecting, a client must write something at least every **5 seconds**.

| Command | Meaning |
| --- | --- |
| `03 0F ...` | Tare |
| `03 0A ...` | LED / power off |
| `03 0B ...` | Timer start / stop / zero |
| `03 1A <mode>` | Calibration (0 = manual; 1 = smart, disabled by default) |
| `03 1C <mode>` | Buzzer: `00` off, `01` on, `02` beep |
| `03 1D <n>` | Samples: `00` = 1, `01` = 2, `03` = 4 |
| `03 1E <group> <show>` | Menu / Device Info / Debug show-hide |
| `03 1F` | Reset |
| `03 30 <float32 LE grams> XOR` | Calibrate with a user-supplied actual weight (response `0xB0`) |
| `03 31 XOR` | Record current weight as container (response `0xB1`) |

Frame checksums, examples and response formats are documented in
[calibration.md](docs/calibration.md) (Chinese).

---

## USB serial commands

Connect at **115200** baud and send newline-terminated ASCII commands:

| Command | Meaning |
| --- | --- |
| `calw <g>` | Calibrate using the provided actual weight (e.g. `calw 500`) |
| `ctr` | Record the current reading as container weight |
| `ct <g>` | Set container weight directly (`ct 0` clears) |
| `cv <factor>` | Write calibration factor directly |
| `cp <g>` / `ce <g>` | Default pour-over / espresso dose |
| `vf <V>` | Battery voltage calibration |
| `v` | Print battery voltage |
| `tare` / `set` / `cal` / `ota` / `beep` / `reset` | Scale controls |

See [calibration.md](docs/calibration.md) for details.

---

## Documentation

- [Calibration & container commands (BLE / USB)](docs/calibration.md) — **Chinese**
- [Menu structure](docs/menu-structure.md) — **Chinese**
- [Behaviour notes & regression checklist](docs/behavior-notes.md) — **Chinese**
- [Device calibration backup](docs/device-calibration-backup.md) — **Chinese**

---

## Notes and limitations

- Calibration smart mode (`0x1A 01`) is intentionally disabled, matching openscale defaults.
- The Debug Info menu entry was removed; the BLE `0x1E group=2` debug screen is still available.
- `0x1B` BLE WiFi-OTA is not compiled in this build; use `Connections → WiFi Update` instead.
- The first boot after the settings version update initialises new menu settings to defaults
  (language Chinese, BLE on, auto-subtract on, samples 1) and preserves calibration/container.
