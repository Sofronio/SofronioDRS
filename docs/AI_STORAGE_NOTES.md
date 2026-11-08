# AI Storage Notes

## Source of truth

EEPROM addresses are derived at compile time in `include/parameter.h` (`i_addr_*`).
`EEPROM.begin(512)` is called once in `setup()`.

## Layout (do not move existing entries)

| Address | Type | Meaning |
| --- | --- | --- |
| 0 | float | Calibration factor (`f_calibration_value`) |
| 4 | float | Container weight (`f_weight_container`) |
| 8 | int | Mode: 0 pour-over, 1 espresso |
| 12 | float | Default pour-over dose |
| 16 | float | Default espresso dose |
| 20 | int | Buzzer enable |
| 24 | String | Welcome text (`String` object) |
| ... | float/bool | Battery calibration, heartbeat, screen flip, time-on-top, BLE buttons, WiFi-on-boot, auto sleep, quick boot, drift max |
| appended | int | `i_addr_language` |
| appended | bool | `i_addr_bleEnabled` |
| appended | bool | `i_addr_containerSubtract` |
| appended | int | `i_addr_samplesInUse` |
| appended | int | `i_addr_settingsVersion` (`SETTINGS_VERSION = 1`) |

Use the `i_addr_*` expressions rather than hard-coded numbers. Existing addresses must never
be reordered; append new settings after `i_addr_settingsVersion` and bump the version if
defaults are needed.

## First-run migration

If `settingsVersion != 1`, the firmware writes defaults for the appended settings
(language Chinese, BLE on, auto-subtract on, samples 1) and stores version 1. Calibration and
container values are not touched.

## Calibration / container backup

`docs/device-calibration-backup.md` records the known values for the current device. Do not
overwrite calibration while performing unrelated work; the menu calibration validates and
only writes on success.

## WiFi credentials (NVS)

- Stored in Preferences namespace `wifi`: keys `ssid`, `pass`.
- Not part of the EEPROM layout; do not log the password.
- The Web UI join flow writes these keys; the hotspot/provisioning flow reads them.

## LittleFS

- Partition `spiffs` (default_16MB.csv) is mounted as LittleFS with `LittleFS.begin(true)`.
- `/logo.xbm`: 128x64 1-bit XBM, exactly 1024 bytes (web upload).
- `/plugins/*.js`: web UI JS plugins (max 64 KB each).
- LittleFS failures are non-fatal; logo display and the plugin page degrade gracefully.

## Checks

- After flashing, serial boot log prints `Cal_Val`, `Container`, `Samples`, `Lang`.
- New settings persist across reset; factory calibration/container remain unchanged.
- `SETTINGS_VERSION` guards against uninitialized EEPROM bytes being interpreted as settings.
