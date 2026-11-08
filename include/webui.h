#ifndef WEBUI_H
#define WEBUI_H

#include <WiFi.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include "parameter.h"
#include "display.h"
#include "declare.h"
#include "webui_page.h"

// Provided by menu.h / SofronioDRS.cpp
void resetMenuToMain();
void calibrationFinish(bool returnToMenu);
void initExtration();

// ---------------------------------------------------------------------------
// Web action queue (consumed by the main loop; AsyncWebServer runs in its task)
// ---------------------------------------------------------------------------
enum WebAction {
  WEB_ACTION_NONE = 0,
  WEB_TARE,
  WEB_TIMER_START,
  WEB_TIMER_STOP,
  WEB_TIMER_RESET,
  WEB_MODE_ESPRESSO,
  WEB_MODE_POUR,
  WEB_MODE_TOGGLE,
  WEB_ROTATE,
  WEB_MENU_SHOW,
  WEB_MENU_HIDE,
  WEB_DEVICE_INFO,
  WEB_POWER_OFF,
  WEB_LOGO_SHOW,
  WEB_WEBUI_STOP,
  WEB_CONTAINER_CLEAR,
  WEB_CAL_START,
  WEB_CAL_SELECT,
  WEB_CAL_CONFIRM,
  WEB_CAL_CANCEL,
  WEB_EXTRACTION_ON,
  WEB_EXTRACTION_OFF,
  WEB_EXTRACTION_TOGGLE,
  WEB_LOGO_BUILTIN,
  WEB_WIFI_CONNECT,
  WEB_WIFI_HOTSPOT,
  WEB_WIFI_SCAN
};

static AsyncWebServer webServer(80);
static AsyncWebSocket webSocket("/ws");
static bool b_webRoutesAdded = false;
static unsigned long t_webLastBroadcast = 0;
static unsigned long t_webLastCleanup = 0;

// Logo upload staging
static uint8_t g_logoBuf[1024];
static size_t g_logoRecv = 0;
static bool g_logoOk = false;

// Plugin upload staging
static String g_pluginBody;
static String g_pluginName;
static bool g_pluginOk = false;
static bool g_pluginError = false;

// WiFi provisioning / STA state
static Preferences wifiPrefs;
static int i_wifiState = 0;  // 0=hotspot only, 1=connecting, 2=STA connected, 3=failed
static bool b_wifiApActive = false;
static unsigned long t_wifiConnectStart = 0;
static unsigned long t_wifiApGraceStart = 0;
static String g_wifiStaSsid;
static String g_wifiStaPass;
static String g_wifiError;
static String g_wifiScanJson = "[]";

// ---------------------------------------------------------------------------
// Filesystem
// ---------------------------------------------------------------------------
void webuiSetupFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed");
    return;
  }
  if (!LittleFS.exists("/plugins")) {
    LittleFS.mkdir("/plugins");
  }
  wifiPrefs.begin("wifi", false);
}

void webuiLoadWifiCreds() {
  g_wifiStaSsid = wifiPrefs.getString("ssid", "");
  g_wifiStaPass = wifiPrefs.getString("pass", "");
}

String webJsonEscape(const String &s) {
  String out;
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '"' || c == '\\') out += '\\';
    out += c;
  }
  return out;
}

void webuiScanNetworks() {
  int n = WiFi.scanNetworks();
  String json = "[";
  bool first = true;
  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.length() == 0) continue;  // hidden network
    if (!first) json += ",";
    first = false;
    json += "{\"ssid\":\"" + webJsonEscape(ssid) + "\",\"rssi\":" +
            String(WiFi.RSSI(i)) + ",\"enc\":" +
            (WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "0" : "1") + "}";
  }
  json += "]";
  g_wifiScanJson = json;
  WiFi.scanDelete();
  Serial.print("WiFi scan: ");
  Serial.println(n);
}

// Draw the uploaded logo, or the built-in image_logo when none is stored.
bool drawStoredLogo() {
  File f = LittleFS.open("/logo.xbm", "r");
#ifdef ROTATION_180
  u8g2.setDisplayRotation(b_screenFlipped ? U8G2_R0 : U8G2_R2);
#else
  u8g2.setDisplayRotation(b_screenFlipped ? U8G2_R2 : U8G2_R0);
#endif
  if (f && f.size() == 1024) {
    static uint8_t buf[1024];
    f.read(buf, 1024);
    f.close();
    u8g2.drawXBMP(0, 0, 128, 64, buf);
    return true;
  }
  if (f) f.close();
  u8g2.drawXBMP(0, 0, 128, 64, image_logo);
  return true;
}

// ---------------------------------------------------------------------------
// Status JSON
// ---------------------------------------------------------------------------
void webBuildStatus(char *buf, size_t len) {
  String staIp = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : String("");
  String apIp = b_wifiApActive ? WiFi.softAPIP().toString() : String("");
  String staSsid = (WiFi.status() == WL_CONNECTED) ? WiFi.SSID() : String("");
  snprintf(buf, len,
           "{\"type\":\"status\",\"g\":%.2f,\"mode\":%d,\"ext\":%d,\"timer\":%d,\"secs\":%lu,"
           "\"ble\":%d,\"menu\":%d,\"cal\":%d,\"calStep\":%d,\"calIdx\":%d,\"calStatus\":\"%s\","
           "\"container\":%.2f,\"lang\":%d,\"web\":%d,\"samples\":%d,\"logoCustom\":%d,"
           "\"wifiState\":%d,\"staIp\":\"%s\",\"staSsid\":\"%s\",\"apIp\":\"%s\","
           "\"wifiError\":\"%s\",\"mdns\":\"drs.local\"}",
           f_displayedValue, b_mode ? 1 : 0, b_extraction ? 1 : 0,
           stopWatch.isRunning() ? 1 : 0, (unsigned long)stopWatch.elapsed(),
           b_ble_enabled ? 1 : 0, b_menu ? 1 : 0, b_calibration ? 1 : 0,
           i_button_cal_status, i_cal_weight, c_calibrationStatus,
           f_weight_container, i_language, b_webuiEnabled ? 1 : 0, i_samplesInUse,
           LittleFS.exists("/logo.xbm") ? 1 : 0,
           i_wifiState, staIp.c_str(), webJsonEscape(staSsid).c_str(), apIp.c_str(),
           g_wifiError.c_str());
}

void webSendStatus(AsyncWebServerRequest *request) {
  char buf[768];
  webBuildStatus(buf, sizeof(buf));
  request->send(200, "application/json", buf);
}

void webuiAnnounceCalibration(int result, float measured, float newCal) {
  if (!b_webuiEnabled || webSocket.count() == 0) return;
  char buf[160];
  snprintf(buf, sizeof(buf),
           "{\"type\":\"cal_result\",\"status\":%d,\"measured\":%.2f,\"cal\":%.4f}",
           result, measured, newCal);
  webSocket.textAll(buf);
}

void webuiAnnounceContainer(int result, float measured, float container) {
  if (!b_webuiEnabled || webSocket.count() == 0) return;
  char buf[160];
  snprintf(buf, sizeof(buf),
           "{\"type\":\"container_result\",\"status\":%d,\"measured\":%.2f,\"container\":%.2f}",
           result, measured, container);
  webSocket.textAll(buf);
}

// ---------------------------------------------------------------------------
// Logo / plugin upload helpers
// ---------------------------------------------------------------------------
String webSanitizeName(String name) {
  name.trim();
  int slash = name.lastIndexOf('/');
  if (slash >= 0) name = name.substring(slash + 1);
  String out;
  for (size_t i = 0; i < name.length(); i++) {
    char c = name[i];
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
        c == '.' || c == '_' || c == '-') {
      out += c;
    } else {
      out += '_';
    }
  }
  if (!out.endsWith(".js")) out += ".js";
  return out;
}

void webHandleLogoBody(AsyncWebServerRequest *request, uint8_t *data, size_t len,
                       size_t index, size_t total) {
  if (index == 0) {
    g_logoRecv = 0;
    g_logoOk = false;
    Serial.print("Logo upload start, content-length: ");
    Serial.println(total);
  }
  if (index + len > sizeof(g_logoBuf)) {
    Serial.println("Logo upload too large");
    return;
  }
  memcpy(g_logoBuf + index, data, len);
  g_logoRecv = index + len;
  if (g_logoRecv == 1024) {
    File f = LittleFS.open("/logo.xbm", "w");
    if (f) {
      f.write(g_logoBuf, 1024);
      f.close();
      g_logoOk = true;
      Serial.println("Logo saved: 1024 bytes");
    } else {
      Serial.println("Logo save failed: LittleFS open error");
    }
  }
}

void webHandlePluginBody(AsyncWebServerRequest *request, uint8_t *data, size_t len,
                         size_t index, size_t total) {
  if (index == 0) {
    g_pluginBody = "";
    g_pluginOk = false;
    g_pluginError = false;
    g_pluginName = webSanitizeName(request->arg("name"));
  }
  if (total > 65536 || g_pluginBody.length() + len > 65536) {
    g_pluginError = true;
    return;
  }
  g_pluginBody.concat((const char *)data, len);
  if (index + len >= total) {
    File f = LittleFS.open(String("/plugins/") + g_pluginName, "w");
    if (f) {
      f.write((const uint8_t *)g_pluginBody.c_str(), g_pluginBody.length());
      f.close();
      g_pluginOk = true;
    }
    g_pluginBody = "";
  }
}

void webListPlugins(AsyncWebServerRequest *request) {
  String json = "[";
  bool first = true;
  File dir = LittleFS.open("/plugins");
  if (dir) {
    File f = dir.openNextFile();
    while (f) {
      if (!f.isDirectory()) {
        String n = f.name();
        int slash = n.lastIndexOf('/');
        if (slash >= 0) n = n.substring(slash + 1);
        if (!first) json += ",";
        json += "\"" + n + "\"";
        first = false;
      }
      f = dir.openNextFile();
    }
  }
  json += "]";
  request->send(200, "application/json", json);
}

// ---------------------------------------------------------------------------
// HTTP routes
// ---------------------------------------------------------------------------
void webRegisterRoutes() {
  if (b_webRoutesAdded) return;

  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *resp =
      request->beginResponse_P(200, "text/html", WEBUI_HTML);
    resp->addHeader("Cache-Control", "no-store, no-cache, must-revalidate");
    request->send(resp);
  });
  webServer.on("/api/status", HTTP_GET, webSendStatus);

  webServer.on("/api/tare", HTTP_POST, [](AsyncWebServerRequest *r) {
    i_webAction = WEB_TARE;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/extraction", HTTP_POST, [](AsyncWebServerRequest *r) {
    String v = r->arg("value");
    if (v == "on") i_webAction = WEB_EXTRACTION_ON;
    else if (v == "off") i_webAction = WEB_EXTRACTION_OFF;
    else i_webAction = WEB_EXTRACTION_TOGGLE;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/timer", HTTP_POST, [](AsyncWebServerRequest *r) {
    if (!b_extraction) {
      r->send(409, "application/json", "{\"error\":\"not in extraction mode\"}");
      return;
    }
    String a = r->arg("action");
    if (a == "start") i_webAction = WEB_TIMER_START;
    else if (a == "stop") i_webAction = WEB_TIMER_STOP;
    else if (a == "reset") i_webAction = WEB_TIMER_RESET;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/mode", HTTP_POST, [](AsyncWebServerRequest *r) {
    String v = r->arg("value");
    if (v == "espresso") i_webAction = WEB_MODE_ESPRESSO;
    else if (v == "pour") i_webAction = WEB_MODE_POUR;
    else i_webAction = WEB_MODE_TOGGLE;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/rotate", HTTP_POST, [](AsyncWebServerRequest *r) {
    i_webAction = WEB_ROTATE;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/menu", HTTP_POST, [](AsyncWebServerRequest *r) {
    i_webAction = (r->arg("show") == "0") ? WEB_MENU_HIDE : WEB_MENU_SHOW;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/device-info", HTTP_POST, [](AsyncWebServerRequest *r) {
    i_webAction = WEB_DEVICE_INFO;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/power", HTTP_POST, [](AsyncWebServerRequest *r) {
    i_webAction = WEB_POWER_OFF;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/webui/stop", HTTP_POST, [](AsyncWebServerRequest *r) {
    i_webAction = WEB_WEBUI_STOP;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/calibrate", HTTP_POST, [](AsyncWebServerRequest *r) {
    float w = r->arg("weight").toFloat();
    if (w <= 0) {
      r->send(400, "application/json", "{\"error\":\"invalid weight\"}");
      return;
    }
    f_weightCalibrationTarget = w;
    i_weightCalibrationSource = 2;
    b_weightCalibrationPending = true;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/container/record", HTTP_POST, [](AsyncWebServerRequest *r) {
    i_containerRecordSource = 2;
    b_containerRecordPending = true;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/container/clear", HTTP_POST, [](AsyncWebServerRequest *r) {
    i_webAction = WEB_CONTAINER_CLEAR;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/cal/start", HTTP_POST, [](AsyncWebServerRequest *r) {
    i_webActionParam = constrain(r->arg("index").toInt(), 0, 5);
    i_webAction = WEB_CAL_START;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/cal/select", HTTP_POST, [](AsyncWebServerRequest *r) {
    i_webActionParam = constrain(r->arg("index").toInt(), 0, 5);
    i_webAction = WEB_CAL_SELECT;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/cal/confirm", HTTP_POST, [](AsyncWebServerRequest *r) {
    i_webAction = WEB_CAL_CONFIRM;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/cal/cancel", HTTP_POST, [](AsyncWebServerRequest *r) {
    i_webAction = WEB_CAL_CANCEL;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/logo/show", HTTP_POST, [](AsyncWebServerRequest *r) {
    i_webAction = WEB_LOGO_SHOW;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/logo/current", HTTP_GET, [](AsyncWebServerRequest *r) {
    if (LittleFS.exists("/logo.xbm")) {
      r->send(LittleFS, "/logo.xbm", "application/octet-stream", true);
    } else {
      r->send(200, "application/octet-stream", image_logo, 1024);
    }
  });
  webServer.on("/api/logo/reset", HTTP_POST, [](AsyncWebServerRequest *r) {
    bool ok = true;
    if (LittleFS.exists("/logo.xbm")) ok = LittleFS.remove("/logo.xbm");
    r->send(ok ? 200 : 500, "application/json",
            ok ? "{\"ok\":true}" : "{\"error\":\"remove failed\"}");
  });

  webServer.on("/api/logo", HTTP_POST,
    [](AsyncWebServerRequest *r) {
      if (g_logoOk) r->send(200, "application/json", "{\"ok\":true}");
      else r->send(400, "application/json", "{\"error\":\"expected 1024 bytes\"}");
    }, nullptr, webHandleLogoBody);

  webServer.on("/api/plugin", HTTP_POST,
    [](AsyncWebServerRequest *r) {
      if (g_pluginOk) r->send(200, "application/json", "{\"ok\":true}");
      else r->send(400, "application/json", "{\"error\":\"upload failed\"}");
    }, nullptr, webHandlePluginBody);

  webServer.on("/api/plugin/delete", HTTP_POST, [](AsyncWebServerRequest *r) {
    String name = webSanitizeName(r->arg("name"));
    bool ok = LittleFS.remove(String("/plugins/") + name);
    r->send(ok ? 200 : 404, "application/json", ok ? "{\"ok\":true}" : "{\"error\":\"not found\"}");
  });
  webServer.on("/api/plugins", HTTP_GET, webListPlugins);

  webServer.on("/api/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *r) {
    r->send(200, "application/json", g_wifiScanJson);
  });
  webServer.on("/api/wifi/rescan", HTTP_POST, [](AsyncWebServerRequest *r) {
    i_webAction = WEB_WIFI_SCAN;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/wifi/connect", HTTP_POST, [](AsyncWebServerRequest *r) {
    String ssid = r->arg("ssid");
    String pass = r->arg("password");
    if (ssid.length() == 0) {
      r->send(400, "application/json", "{\"error\":\"missing ssid\"}");
      return;
    }
    g_wifiStaSsid = ssid;
    g_wifiStaPass = pass;
    g_wifiError = "";
    i_webAction = WEB_WIFI_CONNECT;
    r->send(200, "application/json", "{\"ok\":true}");
  });
  webServer.on("/api/wifi/hotspot", HTTP_POST, [](AsyncWebServerRequest *r) {
    i_webAction = WEB_WIFI_HOTSPOT;
    r->send(200, "application/json", "{\"ok\":true}");
  });

  webServer.serveStatic("/plugins/", LittleFS, "/plugins/");
  webServer.addHandler(&webSocket);

  webSocket.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                       AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      char buf[768];
      webBuildStatus(buf, sizeof(buf));
      client->text(buf);
    }
  });

  b_webRoutesAdded = true;
}

// ---------------------------------------------------------------------------
// Start / stop
// ---------------------------------------------------------------------------
void webuiRaiseAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1),
                    IPAddress(255, 255, 255, 0));
  WiFi.softAP("D.R.S", "12345678");
  b_wifiApActive = true;
}

void webuiStartAP() {
  webuiRaiseAP();
  webRegisterRoutes();
  webServer.begin();
  b_webuiEnabled = true;
  if (i_wifiState != 1) i_wifiState = 0;
  if (MDNS.begin("drs")) {
    MDNS.addService("http", "tcp", 80);
  } else {
    Serial.println("mDNS start failed");
  }
  Serial.print("Web UI AP IP: ");
  Serial.println(WiFi.softAPIP());
}

void startHotspot() {
  webuiStartAP();
  u8g2.setFont(FONT_XS);
  u8g2.firstPage();
  do {
    u8g2.drawUTF8(AC((char *)T("新建热点", "Start Hotspot")),
                  u8g2.getMaxCharHeight() + i_margin_top,
                  (char *)T("新建热点", "Start Hotspot"));
    u8g2.drawUTF8(AC((char *)"WiFi: D.R.S"),
                  LCDHeight / 2 + 2, (char *)"WiFi: D.R.S");
    u8g2.drawUTF8(AC((char *)"192.168.1.1"),
                  LCDHeight - i_margin_bottom - 2, (char *)"192.168.1.1");
  } while (u8g2.nextPage());
  delay(3000);
  b_menu = false;
  resetMenuToMain();
}

void joinWiFi() {
  webuiLoadWifiCreds();
  u8g2.setFont(FONT_XS);
  u8g2.firstPage();
  do {
    u8g2.drawUTF8(AC((char *)T("加入WiFi", "Join WiFi")),
                  u8g2.getMaxCharHeight() + i_margin_top,
                  (char *)T("加入WiFi", "Join WiFi"));
    u8g2.drawUTF8(AC((char *)T("扫描网络中...", "Scanning...")),
                  LCDHeight - i_margin_bottom - 2,
                  (char *)T("扫描网络中...", "Scanning..."));
  } while (u8g2.nextPage());
  if (g_wifiScanJson == "[]") webuiScanNetworks();  // scan before raising AP
  webuiStartAP();
  if (g_wifiStaSsid.length() > 0) {
    i_webAction = WEB_WIFI_CONNECT;
  }
  u8g2.setFont(FONT_XS);
  u8g2.firstPage();
  do {
    u8g2.drawUTF8(AC((char *)T("加入WiFi", "Join WiFi")),
                  u8g2.getMaxCharHeight() + i_margin_top,
                  (char *)T("加入WiFi", "Join WiFi"));
    u8g2.drawUTF8(AC((char *)"WiFi: D.R.S"),
                  LCDHeight / 2 + 2, (char *)"WiFi: D.R.S");
    u8g2.drawUTF8(AC((char *)"192.168.1.1"),
                  LCDHeight - i_margin_bottom - 2, (char *)"192.168.1.1");
  } while (u8g2.nextPage());
  delay(3000);
  b_menu = false;
  resetMenuToMain();
}

void webuiStop() {
  if (!b_webuiEnabled) return;
  webSocket.closeAll();
  webServer.end();
  MDNS.end();
  WiFi.disconnect(true);
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  b_webuiEnabled = false;
  b_wifiApActive = false;
  i_wifiState = 0;
  Serial.println("Web UI stopped");
}

// ---------------------------------------------------------------------------
// Main-loop integration
// ---------------------------------------------------------------------------
void webuiProcessActions() {
  int action = i_webAction;
  if (!action) return;
  i_webAction = 0;
  int param = i_webActionParam;

  switch (action) {
    case WEB_TARE:
      scale.tareNoDelay();
      break;
    case WEB_TIMER_START:
      stopWatch.start();
      break;
    case WEB_TIMER_STOP:
      stopWatch.stop();
      break;
    case WEB_TIMER_RESET:
      stopWatch.reset();
      break;
    case WEB_MODE_ESPRESSO:
      b_mode = 1;
      EEPROM.put(i_addr_mode, b_mode);
      EEPROM.commit();
      break;
    case WEB_MODE_POUR:
      b_mode = 0;
      EEPROM.put(i_addr_mode, b_mode);
      EEPROM.commit();
      break;
    case WEB_MODE_TOGGLE:
      b_mode = b_mode ? 0 : 1;
      EEPROM.put(i_addr_mode, b_mode);
      EEPROM.commit();
      break;
    case WEB_ROTATE:
      i_display_rotation = i_display_rotation ? 0 : 1;
      break;
    case WEB_MENU_SHOW:
      resetMenuToMain();
      b_show_about = false;
      b_debug = false;
      b_menu = true;
      break;
    case WEB_MENU_HIDE:
      b_menu = false;
      resetMenuToMain();
      break;
    case WEB_DEVICE_INFO:
      b_menu = false;
      b_aboutFromMenu = false;
      b_debug = false;
      b_show_about = true;
      break;
    case WEB_POWER_OFF:
      b_powerOff = true;
      break;
    case WEB_LOGO_SHOW:
      b_show_logo = true;
      t_show_logo = millis();
      break;
    case WEB_WEBUI_STOP:
      webuiStop();
      break;
    case WEB_CONTAINER_CLEAR:
      f_weight_container = 0.0f;
      EEPROM.put(i_addr_container, f_weight_container);
      EEPROM.commit();
      break;
    case WEB_CAL_START:
      calibrationReturnToMenu = false;
      b_menu = false;
      b_show_about = false;
      b_debug = false;
      i_cal_weight = param;
      i_calibration = 0;
      i_button_cal_status = 1;
      b_calibration = true;
      break;
    case WEB_CAL_SELECT:
      if (b_calibration && i_button_cal_status == 1) i_cal_weight = param;
      break;
    case WEB_CAL_CONFIRM:
      if (b_calibration && (i_button_cal_status == 1 || i_button_cal_status == 2))
        i_button_cal_status++;
      break;
    case WEB_CAL_CANCEL:
      if (b_calibration) {
        scale.setCalFactor(f_calibration_value);
        calibrationFinish(false);
      }
      break;
    case WEB_WIFI_CONNECT:
      wifiPrefs.putString("ssid", g_wifiStaSsid);
      wifiPrefs.putString("pass", g_wifiStaPass);
      WiFi.mode(WIFI_AP_STA);
      WiFi.begin(g_wifiStaSsid.c_str(), g_wifiStaPass.c_str());
      i_wifiState = 1;
      t_wifiConnectStart = millis();
      g_wifiError = "";
      Serial.print("Connecting WiFi: ");
      Serial.println(g_wifiStaSsid);
      break;
    case WEB_WIFI_HOTSPOT:
      WiFi.disconnect(false, true);
      i_wifiState = 0;
      if (!b_wifiApActive) webuiRaiseAP();
      else WiFi.mode(WIFI_AP);
      Serial.println("Switched to hotspot mode");
      break;
    case WEB_WIFI_SCAN:
      webuiScanNetworks();
      break;
    case WEB_EXTRACTION_ON:
    case WEB_EXTRACTION_OFF:
    case WEB_EXTRACTION_TOGGLE: {
      bool want = action == WEB_EXTRACTION_ON ||
                  (action == WEB_EXTRACTION_TOGGLE && !b_extraction);
      if (want && !b_extraction) {
        f_weight_dose = f_displayedValue;
        initExtration();
        b_ready_to_brew = false;
        stopWatch.stop();
        stopWatch.reset();
        if (b_mode) {
          b_weight_quick_zero = true;
          t_tareByButton = millis();
          b_tareByButton = true;
        }
        b_extraction = true;
      } else if (!want) {
        b_extraction = false;
      }
      break;
    }
    default:
      break;
  }
}

void webuiLoop() {
  if (!b_webuiEnabled) return;
  unsigned long now = millis();

  // WiFi provisioning / STA state machine
  if (i_wifiState == 1) {
    if (WiFi.status() == WL_CONNECTED) {
      i_wifiState = 2;
      g_wifiError = "";
      t_wifiApGraceStart = now;
      MDNS.end();
      if (MDNS.begin("drs")) MDNS.addService("http", "tcp", 80);
      Serial.print("WiFi connected, IP: ");
      Serial.println(WiFi.localIP());
    } else if (now - t_wifiConnectStart > 15000) {
      i_wifiState = 3;
      g_wifiError = "connect failed";
      Serial.println("WiFi connect failed");
    }
  } else if (i_wifiState == 2) {
    if (WiFi.status() != WL_CONNECTED) {
      i_wifiState = 3;
      g_wifiError = "connection lost";
      if (!b_wifiApActive) webuiRaiseAP();
      Serial.println("WiFi STA lost; hotspot restored");
    } else if (b_wifiApActive && now - t_wifiApGraceStart > 10000) {
      WiFi.softAPdisconnect(true);
      WiFi.mode(WIFI_STA);
      b_wifiApActive = false;
      Serial.println("STA up; hotspot closed");
    }
  } else if (i_wifiState == 3 && WiFi.status() == WL_CONNECTED) {
    i_wifiState = 2;
    g_wifiError = "";
    t_wifiApGraceStart = now;
  }

  if (now - t_webLastCleanup > 1000) {
    t_webLastCleanup = now;
    webSocket.cleanupClients();
  }
  if (now - t_webLastBroadcast >= 250) {
    t_webLastBroadcast = now;
    // Skip a frame if any client cannot accept more messages; this prevents
    // "Too many messages queued" disconnects with slow phones/browsers.
    if (webSocket.count() > 0 && webSocket.availableForWriteAll()) {
      char buf[768];
      webBuildStatus(buf, sizeof(buf));
      webSocket.textAll(buf);
    }
  }
}

// ---------------------------------------------------------------------------
// Embedded web page
// ---------------------------------------------------------------------------
// WEBUI_HTML is defined in webui_page.h


#endif
