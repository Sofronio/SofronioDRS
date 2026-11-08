# AI Build Notes

## Environment

- PlatformIO project; default env `esp32s3_v8_1` (PCB 8.1). Other envs: `esp32s3_v8_0`,
  `esp32s3_v7_5`, `esp32s3_v7_2`, `esp32`.
- `platformio.ini` sets `core_dir = .pio/core`, `libdeps_dir = .pio/libdeps`.
- USB-serial is CH340 at **115200** baud; auto-reset is wired.

## Commands

```bash
pio run -e esp32s3_v8_1
pio run -e esp32s3_v8_1 -t upload --upload-port /dev/cu.wchusbserialXXX
pio device monitor --port /dev/cu.wchusbserialXXX --baud 115200
```

Opening the serial port with DTR/RTS asserted can hold the board in reset. When capturing
logs from Python, set `dtr = False`, `rts = False` after opening.

## Optional hardware test hooks

- BLE name `Decent Scale`; service `FFF0`, write `36F5`, notify `FFF4`.
- Weight notifications are 7-byte `0xCE` frames about 10x per second.
- Heartbeat: any write refreshes `t_heartBeat`; clients that write nothing for 5 s are disconnected.
- USB serial accepts newline-terminated ASCII commands (`calw`, `ctr`, `v`, `tare`, ...).

## Font subsetting (important)

- `tools/gen_subset_fonts.py` scans string literals in `src/` and `include/`, extracts the used
  glyphs from the full U8g2 wqy GB2312 arrays and writes `include/generated_fonts.h`.
- `tools/pio_gen_fonts.py` is the PlatformIO `pre:` hook; it runs on every build.
- If the U8g2 library is not installed yet, generation is skipped and the committed
  `generated_fonts.h` is used. After changing UI strings, rerun the script or rebuild.
- The script relies on `u8g2_fonts.c` under `.pio/libdeps/<env>/U8g2/src/clib/`.
- Font sizes: the subset is ~6.5/7.5/9 KB (12/14/16 px) vs ~200-300 KB for full GB2312.
- Verify with `arm-none-eabi-nm --print-size .pio/build/<env>/firmware.elf | grep wqy`:
  only `u8g2_font_wqy1x_subset` should remain; full `*_gb2312` symbols must be absent.

## Common pitfalls

- Do not enable `U8G2_USE_LARGE_FONTS`-only full fonts accidentally by referencing
  `u8g2_font_wqyXX_t_gb2312` directly; use `FONT_M/S/XS`.
- `#define WIFI` is not set; BLE `0x1B` WiFi-OTA is compiled out. WiFi OTA uses
  `WiFi Update` menu → `wifiOta()`.
- SAMPLES is 16 in `ADS1232_ADC_CONFIG.h`; boot tare explicitly sets 1 sample first to stay fast.
- EEPROM settings are versioned (`SETTINGS_VERSION == 1`); a mismatch resets only new settings.

## Focused checks

1. `pio run -e esp32s3_v8_1` succeeds.
2. Firmware size is ~1.36 MB with subset fonts (full fonts removed).
3. BLE advertising + 10 Hz weight notifications after USB reset.
4. Serial boot log shows `Cal_Val`, `Container`, `Samples: 1`, `Lang: 0`.
5. Chinese menu/calibration screens render (visual check on device).
6. Web UI: menu Connections -> Start Hotspot (or USB `web`) starts AP `D.R.S`; page at
   `http://192.168.1.1`; stop with the page button or USB `web`. Verify weight, calibration,
   container, logo upload and a plugin upload/load.
7. Join WiFi: menu Join WiFi or USB `joinwifi`; scan runs, AP starts, page lists SSIDs.
   After joining, verify `drs.local`/STA IP, the 10 s hotspot grace period and the AP-only
   fallback when the STA connection fails.
