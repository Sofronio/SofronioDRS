# AI Display Notes

## Hardware / rendering

- 128x64 SH1106 over hardware SPI, U8g2 page buffer (`u8g2.firstPage()/nextPage()`).
- `FONT_M` = 16 px, `FONT_S` = 14 px, `FONT_XS` = 12 px; all are generated subset fonts from
  `tools/gen_subset_fonts.py` (see `AI_BUILD_NOTES.md`).
- `FONT_L` (logisoso24) and `FONT_EXTRACTION` (fub14) are ASCII-only U8g2 fonts.

## Measured ink metrics (host-verified)

| Font | Chinese ink | Latin ink (with g) | maxCharHeight |
| --- | --- | --- | --- |
| wqy12 | baseline-10 … baseline | baseline-8 … baseline+1 | 13 |
| wqy14 | baseline-12 … baseline | baseline-10 … baseline+2 | 18 |
| wqy16 | baseline-14 … baseline | baseline-12 … baseline+2 | 18 |

Do **not** use `getMaxCharHeight()` as line spacing for multi-line Chinese layouts; it includes
padding. Compute with the ink metrics above.

## Layout rules

- Four-line layouts on 64 px height: use line height 14 and baselines 14/28/42/56
  (verified: ink rows 2..56, 7 px bottom margin). Used by the calibration weight screen.
- Menu: `FONT_XS`, step `getMaxCharHeight()+2` (=15), baselines 13/28/43/58; safe.
- English labels: keep ≤ ~19 ASCII chars at 6 px/char (menu width ≈ 118 px). If a string can
  exceed the width, shorten it or fall back to `FONT_S`.
- Action messages use `FONT_M`; `showMenu()` falls back to `FONT_S` when `AC()` is negative.

## Menu / screens

- Two-level menu; dynamic labels for toggles/values via `menuItemLabel()`.
- `b_screenFlipped` flips UI rotation (menu, pure/extraction scale, about/debug, calibration).
- Device info screen: title + firmware/PCB/serial; any button returns to the Info submenu.
- Debug screen remains available via BLE `03 1E 02 01` (menu entry removed).
- Charging and power-off screens use `FONT_M`; power-off text is localised.

## XBM / logo format (critical)

- `u8g2.drawXBMP()` uses **row-major, 16 bytes per row (for width 128), LSB = leftmost pixel**
  (`mask = 1 << (x & 7)`); this differs from `u8g2_DrawBitmap`/horizontal bitmaps which use MSB.
- Web logo conversion must pack: `bytes[y*16 + (x>>3)] |= 1 << (x&7)`; 1024 bytes total.
- The built-in `image_logo` in `display.h` uses this format (round-trip verified).
- Wrong layouts (column-major or MSB) produce garbled OLED output.

## WiFi icon

- When the Web UI AP is active (`b_webuiEnabled`), `drawWifi()` renders the openscale
  `u8g2_font_open_iconic_www_1x_t` glyph `0x0051` at x=96 (2 px left of the BLE icon).
- y baseline 10 in pure weighing (first row), 34 in extraction (second row, aligned with BLE).
- The icon follows the active display rotation; it is drawn after `drawBle()` in `updateOled()`.

## Checks

- Host-render clipping test: build a small U8g2 host program with the generated fonts and
  measure ink bounds for each 4-line layout before changing spacing.
- Visual check on device for CN/EN menu, calibration selection, device info, debug.
- After changing UI strings, rebuild to regenerate the subset fonts; missing glyphs render blank.
