# Web UI 方案（v1 已实现）

目标：不碰设备按键即可完成校准、容器录入、状态查看和常用操作；后续扩展为插件系统。

## 1. 访问方式

- 菜单新增 `连接 → 网页控制 / Web UI`。
- 启动 SoftAP：SSID `D.R.S`，密码 `12345678`，IP `192.168.1.1`（与现有 WiFi 升级一致）。
- 手机/电脑连接 AP 后打开 `http://192.168.1.1`。
- 页面提供“关闭网页服务”按钮；重启/关机自动停止。WiFi 升级与网页控制不能同时运行（端口 80）。

## 2. 页面功能（v1）

### 实时状态
- 大字号当前重量、模式（意式/手冲）、是否计时、计时秒数、容器重量、蓝牙状态、校准状态。
- 数据通过 WebSocket `/ws` 推送（约 10Hz），校准阻塞期间页面改为 HTTP 轮询 `/api/status`。

### 校准
1. **按实际重量校准**：输入当前砝码实际重量 → 点击校准。
   复用 `applyWeightCalibration()`（与 BLE `0x30` / USB `calw` 完全一致），显示实测重量/新系数/状态。
2. **引导式校准（openscale 流程）**：
   - 选择砝码（50/100/200/500/1000g）→ Start；
   - Continue（此时清空秤盘）→ 设备自动完成 0g 捕获和 3 秒倒计时；
   - 倒计时内放上砝码 → 自动完成加载捕获、校验和复核；
   - 页面显示校准步骤和失败原因；Cancel 可退出。
   - 全程无需触碰设备按键，避免按键释放造成的去皮/模式切换副作用。

### 容器
- “记录当前重量为容器”、“清除容器”。

### 按钮操作
- 去皮、计时开始/停止/归零、切换意式/手冲、旋转屏幕、显示/隐藏菜单、设备信息、关机。

### Logo 上传
- 选择 PNG/JPG/任意浏览器可解码图片；
- JS Canvas 缩放/居中到 128x64，灰度化 + 阈值（可调）+ 反色，打包为 U8g2 XBM（128×64 / 1024 字节）；
- 预览（像素放大）→ 上传到 `/api/logo`；
- 设备保存到 LittleFS `/logo.xbm`，可“显示在秤上”预览，并作为开机 Logo 使用；
- JPG 不需要服务端处理，全部在浏览器 JS 完成。

## 3. API 草案

| 方法/路径 | 作用 |
| --- | --- |
| `GET /` | 网页 |
| `GET /ws` | WebSocket：重量/状态/事件 |
| `GET /api/status` | JSON 状态（轮询兜底） |
| `POST /api/tare` | 去皮 |
| `POST /api/extraction?value=on|off|toggle` | 纯称重 ↔ 萃取模式 |
| `POST /api/timer?action=start|stop|reset` | 提取计时（仅在萃取模式可用，否则 409） |
| `POST /api/mode?value=espresso|pour|toggle` | 模式 |
| `POST /api/rotate` | 旋转 |
| `POST /api/menu?show=0|1` | 菜单显隐 |
| `POST /api/device-info` | 设备信息 |
| `POST /api/power` | 关机 |
| `POST /api/calibrate?weight=57.0` | 按实际重量校准（web 结果事件） |
| `POST /api/container/record` | 记录容器 |
| `POST /api/container/clear` | 清除容器 |
| `POST /api/cal/start?index=N` | 引导式校准开始 |
| `POST /api/cal/confirm` | 引导流程继续（等价右键确认） |
| `POST /api/cal/cancel` | 取消校准 |
| `POST /api/logo` | 1024 字节 XBM 原始 body |
| `GET /api/logo/current` | 下载当前 Logo（自定义或固件内置，1024 字节 XBM） |
| `POST /api/logo/reset` | 删除自定义 Logo，恢复固件内置 Logo |
| `POST /api/logo/show` | 在 OLED 上预览（自定义或内置） |
| `POST /api/webui/stop` | 关闭 AP/网页服务 |

- 所有设备操作通过主循环消费的 `volatile` 动作队列执行，避免 AsyncWebServer 任务直接操作 UI/ADC。
- 校准/容器结果通过 WebSocket 事件返回（`type: "cal_result"` / `"container_result"`）。

## 4. 插件系统（后续）

- 网页提供 `/plugins` 页面，列出 LittleFS `/plugins/*.js` 文件；
- 插件通过 JS API 读取状态、注册按钮、订阅 WebSocket 事件；
- 上传/删除插件同样走网页；
- 需要签名/校验和开启开关，默认关闭，避免任意代码执行风险。

## 5. WiFi 双模式（v1 已实现）

- 菜单 `连接 → 新建热点`：纯 AP 模式（D.R.S / 192.168.1.1）。
- 菜单 `连接 → 加入WiFi`：先扫描 SSID，再开 AP；网页选择 SSID + 密码。
  设备以 AP+STA 连接，成功后约 10 秒关闭热点（只保留 STA），失败保留热点并显示错误。
  凭据存 NVS `wifi` 命名空间；mDNS 主机名 `drs.local`；STA 掉线自动恢复热点。
- API：`GET /api/wifi/scan`、`POST /api/wifi/rescan`、`POST /api/wifi/connect?ssid=&password=`、
  `POST /api/wifi/hotspot`；状态 JSON 增加 `wifiState/staIp/staSsid/apIp/wifiError/mdns`。

## 6. 已实现内容（v1）

- 菜单 `连接 → 网页控制`（手动启动）；USB 命令 `web` 可启停（开发辅助）。
- 内嵌网页（`include/webui_page.h`）与 AsyncWebServer/WebSocket（`include/webui.h`）。
- 实时状态、纯称重/萃取切换、萃取计时、实际重量校准、引导式校准、容器、虚拟按钮、
  Logo 上传/下载当前/恢复内置/预览、JS 插件上传/加载/删除。
- Logo 和插件存储在 LittleFS：`/logo.xbm`、`/plugins/*.js`。
- WiFi 开启时 OLED 显示 openscale 同款 WiFi 图标（BLE 图标左侧；萃取模式在第二排）。
- 插件 API 见 `webui-plugins.md`，示例见 `docs/examples/plugin-example.js`。

## 7. 风险与限制

- AP 为开放局域网环境，任何能连接 AP 的设备都能操作秤；仅在可信环境使用。
- 上传 Logo 会写 LittleFS（约 3.4MB 分区），校验固定 1024 字节。
- 引导式校准期间主循环阻塞数秒，HTTP 状态轮询仍可用，WebSocket 重量推送暂停。
- 网页控制与 WiFi 升级共用端口 80，启动一个前需停止另一个。
