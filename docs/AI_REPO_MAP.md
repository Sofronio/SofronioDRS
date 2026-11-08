# AI Repo Map

Task routing for AI agents working on this repository. Keep it short and verify facts in code.

## Project shape

ESP32-S3 Arduino/PlatformIO firmware for Decent HDS (Half Decent Scale) hardware.
Default env `esp32s3_v8_1`; firmware version 6.1.3. No WiFi web UI or plugins yet
(only ElegantOTA). Bluetooth is the Decent Scale BLE protocol plus vendor commands.

## Key files

| Path | Responsibility |
| --- | --- |
| `src/SofronioDRS.cpp` | Main loop, BLE callbacks, scale modes, OLED drawing, serial commands, weight calibration, container record, power, input handling |
| `include/menu.h` | Two-level menu, CN/EN strings, guided calibration (openscale logic), container menu, actions |
| `include/parameter.h` | Globals and EEPROM address derivation (`i_addr_*`), language/settings flags |
| `include/display.h` | OLED driver object, fonts (`FONT_M/S/XS` = generated subsets), layout macros |
| `include/ADS1232_ADC.h`, `src/ADS1232_ADC.cpp` | ADS1232 driver: 16-sample window, `getDebugInfo()` snapshot |
| `include/calibration_validation.h` | Calibration candidate/verification validation (range, sign, tolerance) |
| `include/power.h` | Sleep, shutdown, charging wake |
| `include/wifi_ota.h` | ElegantOTA SoftAP (SSID `D.R.S`) |
| `include/declare.h` | `scale`, BLE server/characteristics, buzzer, buttons |
| `include/config.h` | Hardware profiles per PCB revision, strings, feature macros |
| `include/webui.h` | AsyncWebServer/WebSocket, HTTP APIs, Web actions, logo/plugin storage |
| `include/webui_page.h` | Embedded 128x64 web page HTML/CSS/JS (PROGMEM) |
| `tools/gen_subset_fonts.py` | Build-time font subsetter (used via `pre:tools/pio_gen_fonts.py`) |
| `docs/` | Calibration protocol, menu structure, behaviour notes, AI notes, web UI/plugin docs |

## Task routing

- BLE/UART protocol or commands → `docs/AI_PROTOCOL_NOTES.md` + `docs/calibration.md`
- Menu/UI/layout/font/clipping → `docs/AI_DISPLAY_NOTES.md` + `include/menu.h`
- Calibration flow or validation → `include/menu.h` (`calibration()`), `calibration_validation.h`
- EEPROM/settings/migration → `docs/AI_STORAGE_NOTES.md` + `include/parameter.h`
- Build/flash/serial/font tooling → `docs/AI_BUILD_NOTES.md`
- Power-off, tare, mode-switch input rules → `docs/behavior-notes.md`
- Web UI / HTTP / WebSocket / plugins / logo → `docs/webui-plan.md`, `docs/webui-plugins.md`, `include/webui.h`

## Invariants

- Do not reorder or move existing EEPROM addresses; append new ones and bump `SETTINGS_VERSION`.
- Keep calibration (address 0) and container (address 4) compatible with the factory backup.
- BLE command bytes `0x1C/0x1D/0x1E/0x1F` are standard Decent protocol; custom commands use `0x30/0x31`.
- After changing Chinese UI strings, rebuild so `tools/gen_subset_fonts.py` regenerates the font subset.
- Web UI actions are queued in `i_webAction` and executed in the main loop; never call UI/ADC directly
  from AsyncWebServer callbacks.
- Logo and plugins live in LittleFS (`/logo.xbm`, `/plugins/*.js`); EEPROM layout is unrelated.
- Always compile with `pio run -e esp32s3_v8_1` and, when hardware is attached, flash + verify BLE.
