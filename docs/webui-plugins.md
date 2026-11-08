# Web UI 插件开发

Web UI 的插件是**普通的 JavaScript 文件**，存放在设备的 LittleFS `/plugins/` 目录中，
打开网页时自动加载。插件只能操作网页（DOM / `ScaleWeb` API），不会在设备上执行任意代码。

## 1. 上传 / 删除

1. 设备菜单 `连接 → 网页控制` 启动 AP（SSID `D.R.S`，密码 `12345678`）。
2. 浏览器打开 `http://192.168.1.1`。
3. 页面底部 `Plugins`：
   - 选择 `.js` 文件 → `Upload JS plugin`；
   - 列表中的 `Delete` 删除对应插件；
   - 刷新页面后插件自动加载。

文件大小限制 64KB，文件名会被限制为字母/数字/`.`/`_`/`-`。

## 2. ScaleWeb API

插件脚本加载时页面已就绪，可直接使用全局 `ScaleWeb`：

| 成员 | 说明 |
| --- | --- |
| `ScaleWeb.status` | 最近一次状态对象（`type:"status"` 的 JSON） |
| `ScaleWeb.api(path, params)` | 发送 POST 请求（params 作为查询参数），返回 Promise<string> |
| `ScaleWeb.onStatus(fn)` | 订阅状态更新（约 10Hz；校准阻塞时依靠轮询） |
| `ScaleWeb.onEvent(fn)` | 订阅事件：`cal_result`、`container_result` |
| `ScaleWeb.addPanel(title, html)` | 在插件区添加卡片，返回卡片 DOM 元素 |
| `ScaleWeb.addButton(label, fn)` | 在插件区添加按钮，返回按钮元素 |

状态对象字段（示例）：

```json
{"type":"status","g":57.0,"mode":1,"ext":0,"timer":0,"secs":0,"ble":1,
 "menu":0,"cal":0,"calStep":0,"calIdx":0,"calStatus":"ok",
 "container":18.5,"lang":0,"web":1,"samples":1}
```

常用 API：

```js
ScaleWeb.api('/api/tare');
ScaleWeb.api('/api/extraction', {value: 'on'});  // on/off/toggle
ScaleWeb.api('/api/timer', {action: 'start'});   // start/stop/reset (extraction mode only)
ScaleWeb.api('/api/mode', {value: 'toggle'});    // espresso/pour/toggle
ScaleWeb.api('/api/rotate');
ScaleWeb.api('/api/menu', {show: '1'});
ScaleWeb.api('/api/calibrate', {weight: 500});
ScaleWeb.api('/api/container/record');
ScaleWeb.api('/api/container/clear');
ScaleWeb.api('/api/cal/start', {index: 4});
ScaleWeb.api('/api/cal/confirm');
ScaleWeb.api('/api/cal/cancel');
```

## 3. 示例

`docs/examples/plugin-example.js`：

```js
const card = ScaleWeb.addPanel('Example Plugin',
  '<p>Live weight: <b class="ep-weight">--</b> g</p>');
ScaleWeb.addButton('Tare (plugin)', () => ScaleWeb.api('/api/tare'));
ScaleWeb.onStatus(s => {
  const w = card.querySelector('.ep-weight');
  if (w) w.textContent = (Number(s.g) || 0).toFixed(2);
});
```

## 4. 注意事项

- AP 是开放局域网（密码固定），任何能连接 AP 的设备都可以操作秤和上传插件；仅在可信环境使用。
- 插件在浏览器中运行，不能直接访问设备内存或执行固件代码，但可以调用所有 Web API。
- 删除插件后刷新页面生效；插件加载失败会在浏览器控制台输出错误，不影响核心页面。
