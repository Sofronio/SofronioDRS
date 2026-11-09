# Changelog / 更新日志

All notable changes to this firmware are documented in this file.
本文件记录固件的重要变更。版本号与 `include/config.h` 中的 `FW:` 字段一致。

## [6.2.0] - 2026-11-09

First automated GitHub Actions firmware release.
首个由 GitHub Actions 自动构建并发布的固件版本。

### English

#### Added

- **Web UI** — start from `Connections → Start Hotspot` or USB `web`; live weight and
  status over WebSocket (HTTP polling fallback), weighing/extraction switch, extraction
  timer, actual-weight and guided calibration without using the scale buttons, container
  control, virtual buttons, logo upload/download/restore, and a JavaScript plugin API
  (`ScaleWeb`). The AP runs at `http://192.168.1.1` / `http://drs.local`.
- **WiFi provisioning** — `Connections → Join WiFi` scans networks, raises the `D.R.S`
  hotspot, then joins the selected network in AP+STA mode. Credentials are stored in NVS,
  the hotspot closes after a successful join, and the page falls back to the hotspot on
  failure. `Join WiFi` / `web` are available as USB serial helpers.
- **Two-level categorized menu** (`Scale` / `Connections` / `Display` / `Power` / `Info`)
  with a runtime Chinese/English switch persisted in EEPROM.
- **Build-time CJK font subsetting** — `tools/gen_subset_fonts.py` extracts only the glyphs
  used by the source. The three full GB2312 fonts (~759 KB) were replaced by ~23 KB of
  subsets, cutting the firmware from 2.09 MB to about 1.5 MB.
- **Calibration and container commands** — calibrate with a supplied actual weight
  (BLE `0x30`, USB `calw`); record the current reading as container (BLE `0x31`, USB `ctr`).
- **BLE charging/USB status query** — `03 32` returns `0xB2` with the charging flag, raw
  `USB_DET`/`BATTERY_CHARGING` levels, battery voltage, USB voltage and XOR checksum.
  USB serial `chg` prints the same diagnostics.
- **Automatic extraction control** — after zeroing, pour-over starts the timer automatically
  when pouring is detected and stops it after the cup is removed (> 50 g drop after a stable
  reading); espresso auto-tares and starts when the cup is placed, then stops when dripping
  ends and the reading stabilizes. Both extraction screens show live flow rate (g/s, updated
  every second) and brew ratio (`1:x`, beverage weight ÷ ground dose).
- **Boot-time BLE switch** — hold left `O` for BLE on, hold right `[]` for BLE off;
  reset/USB wake uses the saved menu setting.
- **Chinese README** and links between the English and Chinese documentation.
- WiFi indicator on the OLED (left of the BLE icon) while the Web UI AP is active.

#### Changed

- Custom BLE commands moved from `0x1C`/`0x1D` to `0x30`/`0x31` to avoid conflicts with
  standard Decent protocol commands. Standard commands `0x1C` (buzzer), `0x1D` (sample
  setting), `0x1E` (menu / device info / debug) and `0x1F` (reset) are available.
- Menu calibration ported from openscale: stable raw capture, range/sign validation and
  post-calibration verification; drift-compensation state handling aligned with openscale.
- Battery and charging state sampled every 500 ms instead of every 30 s, so USB plug/unplug
  changes are reflected promptly.
- Device info returns to the `Info` submenu; boot tare uses the current sample window.
- EEPROM settings version marker avoids garbage settings on first boot.
- Documentation expanded: calibration and container protocol, menu structure, behaviour
  notes/regression checklist, Web UI and plugin API, plus AI-oriented build/protocol/
  storage/display notes.

#### Fixed

- Extraction mode: long-press rotation no longer leaves the weight display stuck at 0;
  a timer that was running before the press is resumed.
- Weighing mode no longer shows the extraction timer.
- V8.1 charging detection now combines `USB_DET`/`CHRG` with battery-rail voltage, fixing
  detection on units whose digital charging pins do not change.
- Menu double-click / long-press can no longer tare or power off during menu, calibration,
  device info, debug screens or while the Web UI is active.
- XBM logo packing now matches `u8g2` `drawXBMP` (row-major, 16 bytes/row, LSB on the left);
  logo upload accepts chunked bodies and the current logo can be downloaded as PNG.
- WebSocket status throttled to 4 Hz with queue backpressure.
- Mode-switch and menu-selection button releases no longer trigger a tare.

#### Notes

- This release builds the default `esp32s3_v8_1` environment (PCB 8.1). The `esp32s3_v8_0`,
  `esp32s3_v7_5`, `esp32s3_v7_2` and `esp32` environments remain available for building
  other board revisions from source.
- BLE `0x1B` WiFi-OTA is not compiled in this build; use `Connections → WiFi Update`.
- The Web UI hotspot is a trusted-LAN interface (no authentication beyond the AP password).

### 中文

#### 新增

- **Web UI 网页界面** — 菜单 `连接 → 启动热点` 或 USB 命令 `web` 启动；支持 WebSocket
  实时重量/状态(HTTP 轮询兜底)、称重/萃取模式切换、萃取计时、实际重量校准与引导式校准
  (无需按秤上按钮)、容器控制、虚拟按键、Logo 上传/下载/恢复，以及 JavaScript 插件 API
  (`ScaleWeb`)。热点地址为 `http://192.168.1.1` / `http://drs.local`。
- **WiFi 配网** — 菜单 `连接 → 加入 WiFi` 会扫描网络并开启 `D.R.S` 热点，然后以 AP+STA
  模式连接所选网络。凭据保存在 NVS；连接成功后热点自动关闭，失败则保持热点并显示错误。
  同时提供 USB 命令 `joinwifi` / `web` 用于调试。
- **两级分类菜单**(`称重` / `连接` / `显示` / `电源` / `信息`)，中英文运行时切换并保存到
  EEPROM。
- **构建期中文字体子集化** — `tools/gen_subset_fonts.py` 只提取源码实际用到的字形，将三个
  完整 GB2312 字体(约 759 KB)替换为约 23 KB 的子集，固件从 2.09 MB 降到约 1.5 MB。
- **校准与容器命令** — 可用实际重量校准(BLE `0x30`、USB `calw`)；把当前读数记录为容器
  重量(BLE `0x31`、USB `ctr`)。
- **BLE 充电/USB 状态查询** — `03 32` 返回 `0xB2`：充电标志、`USB_DET`/`BATTERY_CHARGING`
  原始电平、电池电压、USB 电压和 XOR 校验。USB 串口 `chg` 可打印相同诊断信息。
- **自动萃取控制** — 手冲清零后监测到注水自动开始计时，稳态后移出杯子(下降 > 50 g)自动停止；
  意式放杯自动清零并开始计时，停止落液且读数稳定后自动停止。两种萃取界面都显示实时
  流速(g/s，每秒刷新)和粉水比(`1:x`，液重 ÷ 粉重)。
- **开机 BLE 开关** — 长按左键 `O` 开机并开启 BLE，长按右键 `[]` 开机并关闭 BLE；
  复位/USB 唤醒使用菜单中保存的设置。
- **中文说明文档**，并与英文 README 互相链接。
- Web UI 热点开启时，OLED 在 BLE 图标左侧显示 WiFi 图标。

#### 变更

- 自定义 BLE 命令从 `0x1C`/`0x1D` 移到 `0x30`/`0x31`，避免与 Decent 标准协议冲突。
  标准命令 `0x1C`(蜂鸣器)、`0x1D`(采样设置)、`0x1E`(菜单/设备信息/调试)、`0x1F`(复位)
  可正常使用。
- 菜单校准移植 openscale 逻辑：稳定原始值捕获、范围/符号校验、校准后复核；漂移补偿状态
  处理与 openscale 对齐。
- 电池与充电状态改为每 500 ms 采样一次(原来 30 s)，USB 插拔状态能及时更新。
- 设备信息退出后返回 `信息` 子菜单；开机去皮使用当前采样窗口。
- EEPROM 设置版本标记，避免首次运行时读到无效设置。
- 文档扩充：校准与容器协议、菜单结构、行为说明/回归清单、Web UI 与插件 API，以及面向
  AI 的构建/协议/存储/显示笔记。

#### 修复

- 萃取模式下长按旋转屏幕不再导致重量显示卡在 0；按下前正在运行的计时器会恢复。
- 称重模式不再显示萃取计时器。
- V8.1 充电检测改为 `USB_DET`/`CHRG` + 电池轨电压混合判断，修复部分机器数字充电信号
  不翻转时无法识别充电的问题。
- 菜单、校准、设备信息、调试界面以及 Web UI 活动期间，双击/长按不会再误触去皮或关机。
- XBM Logo 打包与 `u8g2` `drawXBMP` 一致(行优先、每行 16 字节、LSB 在左)；Logo 上传支持
  chunked 请求，并可把当前 Logo 下载为 PNG。
- WebSocket 状态限制为 4 Hz，并加入队列背压。
- 模式切换和菜单选择时松开按键不再触发去皮。

#### 说明

- 本次发布构建默认 `esp32s3_v8_1` 环境(PCB 8.1)。其他硬件版本仍可使用 `esp32s3_v8_0`、
  `esp32s3_v7_5`、`esp32s3_v7_2`、`esp32` 环境从源码构建。
- 本固件未编译 BLE `0x1B` WiFi-OTA，请使用菜单 `连接 → WiFi升级`。
- Web UI 热点属于可信局域网接口(除热点密码外无额外认证)。

## [6.1.3] - 2026-07-07

### English

- Added proper GPIO power-off/shutdown handling.

### 中文

- 增加可靠的 GPIO 关机/断电处理。
