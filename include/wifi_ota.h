#ifdef WIFIOTA
#ifndef WIFI_OTA_H
#define WIFI_OTA_H
#include "display.h"
#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>

const char *ssid = "D.R.S";
const char *password = "12345678";
AsyncWebServer server(80);
unsigned long ota_progress_millis = 0;

void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 500) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
    Serial.printf("Progress: %u%%\n", (current * 100) / final);
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "上传进度: %u%%", (current * 100) / final);
    u8g2.firstPage();
    u8g2.setFont(FONT_S);
    do {
      u8g2.drawUTF8(AC((char *)trim(buffer)), AM(), (char *)trim(buffer));
    } while (u8g2.nextPage());
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
    u8g2.firstPage();
    u8g2.setFont(FONT_S);
    do {
      u8g2.drawUTF8(AC((char *)"升级成功"), AM(), (char *)"升级成功");
    } while (u8g2.nextPage());
    delay(1000);
  } else {
    Serial.println("There was an error during OTA update!");
    u8g2.firstPage();
    u8g2.setFont(FONT_S);
    do {
      u8g2.drawUTF8(AC((char *)"升级失败"), AM(), (char *)"升级失败");
    } while (u8g2.nextPage());
    delay(1000);
  }
  b_ota = false;
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  reset();
}

void wifiOta() {
  u8g2.firstPage();
  u8g2.setFont(FONT_S);
  do {
    u8g2.drawUTF8(AC((char *)"开始升级"), AM(), (char *)"开始升级");
  } while (u8g2.nextPage());

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);
  Serial.println("");

  Serial.print("WiFi Access Point: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Redirect to "/update"
    request->redirect("/update");
  });

  ElegantOTA.begin(&server);  // Start ElegantOTA
  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);

  server.begin();
  Serial.println("HTTP server started");
  b_ota = true;
  u8g2.firstPage();
  u8g2.setFont(FONT_S);
  do {

    char ver[50];
    sprintf(ver, "%s %s", FIRMWARE_VER, PCB_VER);
    //u8g2.drawUTF8(AC((char *)"Please connect WiFi"), u8g2.getMaxCharHeight() + i_margin_top - 5, (char *)"Please connect WiFi");

    u8g2.drawUTF8(AC((char *)"WiFi: D.R.S"), u8g2.getMaxCharHeight() + i_margin_top - 5, (char *)"WiFi: D.R.S");
    u8g2.drawUTF8(AC((char *)"密码: 12345678"), AM(), (char *)"密码: 12345678");
    u8g2.drawUTF8(AC(trim(ver)), LCDHeight - i_margin_bottom + 5, trim(ver));
  } while (u8g2.nextPage());
}
#endif  //WIFI_OTA_H
#endif