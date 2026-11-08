#ifndef MENU_H
#define MENU_H

#include "calibration_validation.h"

//BLE 开关（定义在 SofronioDRS.cpp）
void bleSetEnabled(bool enabled);
void drawButtonBox();
void resetTracking();
void resetStableOutput();

//Web UI / WiFi（定义在 webui.h）
void startHotspot();
void joinWiFi();
void webuiStop();

//砝码选项（中英双语）
const char *weights_zh[] = { "退出", "50克", "100克", "200克", "500克", "1000克" };
const char *weights_en[] = { "Exit", "50g", "100g", "200g", "500g", "1000g" };
const float weight_values[] = { 0.0, 50.0, 100.0, 200.0, 500.0, 1000.0 };
inline const char* weightLabel(int i) {
  return (i_language == 0) ? weights_zh[i] : weights_en[i];
}

long t_actionMessage = 0;
int t_actionMessageDelay = 1000;
String actionMessage = "Default";
String actionMessage2 = "Default";

inline void menuActionMessage(const char *zh, const char *en) {
  actionMessage = T(zh, en);
  t_actionMessage = millis();
  t_actionMessageDelay = 1000;
}

// Menu structure
struct Menu {
  const char *name;  //中文名称（显示时按语言翻译）
  void (*action)();  //what to do NULL for submenu
  Menu *subMenu;     //submenu NULL for none
  Menu *parentMenu;  //parentmenu NULL for root menu
};

template<typename T, size_t N>
constexpr size_t getArraySize(T (&arr)[N]) {
  return N;
}

// Function prototypes
void exitMenu();
void toggleBuzzer();
void calibrate();
void wifiUpdate();
void showAbout();
void showDeviceInfo();
void showMenu();
void navigateMenu(int direction);
void selectMenu();
void resetMenuToMain();
void setContainerWeight();
void clearContainer();
void toggleAutoSubtract();
void cycleSamples();
void cycleDriftComp();
void toggleBle();
void toggleHeartbeat();
void toggleBleButtons();
void toggleFlipScreen();
void toggleTimeOnTop();
void toggleLanguage();
void toggleAutoSleep();
void toggleQuickBoot();

// ---------- Menu definitions ----------
Menu menuExit = { "退出", exitMenu, NULL, NULL };
Menu menuScale = { "称重", NULL, NULL, NULL };
Menu menuConnections = { "连接", NULL, NULL, NULL };
Menu menuDisplay = { "显示", NULL, NULL, NULL };
Menu menuPower = { "电源", NULL, NULL, NULL };
Menu menuInfo = { "信息", NULL, NULL, NULL };

Menu menuScaleBack = { "上一级", NULL, NULL, &menuScale };
Menu menuCalibrate = { "开始校准", calibrate, NULL, &menuScale };
Menu menuDriftComp = { "漂移补偿", cycleDriftComp, NULL, &menuScale };
Menu menuContainer = { "自动扣除设置", NULL, NULL, &menuScale };
Menu menuSamples = { "采样数", cycleSamples, NULL, &menuScale };

Menu menuContainerBack = { "上一级", NULL, NULL, &menuContainer };
Menu menuRecordContainer = { "记录容器重量", setContainerWeight, NULL, &menuContainer };
Menu menuClearContainer = { "清除容器", clearContainer, NULL, &menuContainer };
Menu menuAutoSubtract = { "自动扣除", toggleAutoSubtract, NULL, &menuContainer };

Menu menuConnectionsBack = { "上一级", NULL, NULL, &menuConnections };
Menu menuBle = { "蓝牙", toggleBle, NULL, &menuConnections };
Menu menuHeartbeat = { "心跳检测", toggleHeartbeat, NULL, &menuConnections };
Menu menuBleButtons = { "BLE按键", toggleBleButtons, NULL, &menuConnections };
Menu menuHotspot = { "新建热点", startHotspot, NULL, &menuConnections };
Menu menuJoinWiFi = { "加入WiFi", joinWiFi, NULL, &menuConnections };
Menu menuWiFiUpdate = { "WiFi升级", wifiUpdate, NULL, &menuConnections };

Menu menuDisplayBack = { "上一级", NULL, NULL, &menuDisplay };
Menu menuBuzzer = { "蜂鸣器", toggleBuzzer, NULL, &menuDisplay };
Menu menuFlipScreen = { "屏幕翻转", toggleFlipScreen, NULL, &menuDisplay };
Menu menuTimeOnTop = { "时间置顶", toggleTimeOnTop, NULL, &menuDisplay };
Menu menuLanguage = { "语言", toggleLanguage, NULL, &menuDisplay };

Menu menuPowerBack = { "上一级", NULL, NULL, &menuPower };
Menu menuAutoSleep = { "自动休眠", toggleAutoSleep, NULL, &menuPower };
Menu menuQuickBoot = { "快速开机", toggleQuickBoot, NULL, &menuPower };

Menu menuInfoBack = { "上一级", NULL, NULL, &menuInfo };
Menu menuDeviceInfo = { "设备信息", showDeviceInfo, NULL, &menuInfo };

Menu *scaleMenu[] = { &menuScaleBack, &menuCalibrate, &menuDriftComp, &menuContainer, &menuSamples };
Menu *containerMenu[] = { &menuContainerBack, &menuRecordContainer, &menuClearContainer, &menuAutoSubtract };
Menu *connectionsMenu[] = { &menuConnectionsBack, &menuBle, &menuHeartbeat, &menuBleButtons, &menuHotspot, &menuJoinWiFi, &menuWiFiUpdate };
Menu *displayMenu[] = { &menuDisplayBack, &menuBuzzer, &menuFlipScreen, &menuTimeOnTop, &menuLanguage };
Menu *powerMenu[] = { &menuPowerBack, &menuAutoSleep, &menuQuickBoot };
Menu *infoMenu[] = { &menuInfoBack, &menuDeviceInfo };

Menu *mainMenu[] = { &menuExit, &menuScale, &menuConnections, &menuDisplay, &menuPower, &menuInfo };

Menu **currentMenu = mainMenu;
Menu *currentSelection = mainMenu[0];
int currentMenuSize = getArraySize(mainMenu);
int currentIndex = 0;
const int linesPerPage = 4;
int currentPage = 0;
int totalPages = currentMenuSize / linesPerPage + 1;

// 菜单名称翻译表
struct MenuNameText {
  const char *zh;
  const char *en;
};
const MenuNameText menuNameTexts[] = {
  { "退出", "Exit" },
  { "称重", "Scale" },
  { "连接", "Connections" },
  { "显示", "Display" },
  { "电源", "Power" },
  { "信息", "Info" },
  { "上一级", "Back" },
  { "开始校准", "Calibrate" },
  { "漂移补偿", "Drift" },
  { "自动扣除设置", "Auto Container" },
  { "记录容器重量", "Record Container" },
  { "清除容器", "Clear Container" },
  { "自动扣除", "Auto Subtract" },
  { "采样数", "Samples" },
  { "蓝牙", "Bluetooth" },
  { "心跳检测", "Heartbeat" },
  { "BLE按键", "BLE Buttons" },
  { "新建热点", "Start Hotspot" },
  { "加入WiFi", "Join WiFi" },
  { "WiFi升级", "WiFi Update" },
  { "蜂鸣器", "Buzzer" },
  { "屏幕翻转", "Flip Screen" },
  { "时间置顶", "Time on Top" },
  { "语言", "Language" },
  { "自动休眠", "Auto Sleep" },
  { "快速开机", "Quick Boot" },
  { "设备信息", "Device Info" },
  { "调试信息", "Debug Info" },
};

const char* trName(const char *zh) {
  if (i_language == 0) return zh;
  for (size_t i = 0; i < getArraySize(menuNameTexts); i++) {
    if (strcmp(menuNameTexts[i].zh, zh) == 0) return menuNameTexts[i].en;
  }
  return zh;
}

inline const char* onOffText(bool enabled) {
  return enabled ? T("开", "ON") : T("关", "OFF");
}

// 菜单项显示文本（开关/数值项为动态文本）
const char* menuItemLabel(Menu *item, char *buf, size_t len) {
  if (item == &menuDriftComp) {
    if (fabs(f_maxDriftCompensation) < 0.0001f)
      snprintf(buf, len, "%s", T("漂移:关闭", "Drift: Off"));
    else
      snprintf(buf, len, T("漂移:%.3gg", "Drift: %.3gg"), f_maxDriftCompensation);
    return buf;
  }
  if (item == &menuSamples) {
    snprintf(buf, len, T("采样:%d", "Samples: %d"), i_samplesInUse);
    return buf;
  }
  if (item == &menuAutoSubtract) {
    snprintf(buf, len, T("自动扣除:%s", "Auto Sub: %s"), onOffText(b_containerSubtractEnabled));
    return buf;
  }
  if (item == &menuBle) {
    snprintf(buf, len, T("蓝牙:%s", "Bluetooth: %s"), onOffText(b_ble_enabled));
    return buf;
  }
  if (item == &menuHeartbeat) {
    snprintf(buf, len, T("心跳:%s", "Heartbeat: %s"), onOffText(b_requireHeartBeat));
    return buf;
  }
  if (item == &menuBleButtons) {
    snprintf(buf, len, T("BLE按键:%s", "BLE Buttons: %s"), onOffText(b_btnFuncWhileConnected));
    return buf;
  }
  if (item == &menuBuzzer) {
    snprintf(buf, len, T("蜂鸣器:%s", "Buzzer: %s"), onOffText(b_beep));
    return buf;
  }
  if (item == &menuFlipScreen) {
    snprintf(buf, len, T("翻转屏幕:%s", "Flip: %s"), onOffText(b_screenFlipped));
    return buf;
  }
  if (item == &menuTimeOnTop) {
    snprintf(buf, len, T("时间置顶:%s", "Time Top: %s"), onOffText(b_timeOnTop));
    return buf;
  }
  if (item == &menuLanguage) {
    snprintf(buf, len, "%s", (i_language == 0) ? "语言:中文" : "Language: English");
    return buf;
  }
  if (item == &menuAutoSleep) {
    snprintf(buf, len, T("自动休眠:%s", "Auto Sleep: %s"), onOffText(b_autoSleep));
    return buf;
  }
  if (item == &menuQuickBoot) {
    snprintf(buf, len, T("快速开机:%s", "Quick Boot: %s"), onOffText(b_quickBoot));
    return buf;
  }
  return trName(item->name);
}

void linkSubmenus() {
  menuScale.subMenu = scaleMenu[0];
  menuContainer.subMenu = containerMenu[0];
  menuConnections.subMenu = connectionsMenu[0];
  menuDisplay.subMenu = displayMenu[0];
  menuPower.subMenu = powerMenu[0];
  menuInfo.subMenu = infoMenu[0];
}

// ---------- helpers ----------
void menuSaveBool(int addr, bool value) {
  EEPROM.put(addr, value);
  EEPROM.commit();
}
void menuSaveInt(int addr, int value) {
  EEPROM.put(addr, value);
  EEPROM.commit();
}

// ---------- menu actions ----------
void exitMenu() {
  u8g2.setFont(FONT_M);
  u8g2.firstPage();
  do {
    u8g2.drawUTF8(AC((char *)T("退出设置", "Exit Menu")), AM() - 5,
                  (char *)T("退出设置", "Exit Menu"));
  } while (u8g2.nextPage());
#ifdef BUZZER
  buzzer.off();
#endif
  delay(1000);
  b_menu = false;
  resetMenuToMain();
}

void toggleBuzzer() {
  b_beep = !b_beep;
#ifdef BUZZER
  if (b_beep) buzzer.beep(1, 50);
#endif
  menuSaveBool(i_addr_beep, b_beep);
  menuActionMessage(b_beep ? "蜂鸣器已开启" : "蜂鸣器已关闭",
                    b_beep ? "Buzzer ON" : "Buzzer OFF");
}

void toggleAutoSleep() {
  b_autoSleep = !b_autoSleep;
  menuSaveBool(i_addr_autoSleep, b_autoSleep);
  menuActionMessage(b_autoSleep ? "已启用自动关机" : "已禁用自动关机",
                    b_autoSleep ? "Auto sleep ON" : "Auto sleep OFF");
}

void toggleQuickBoot() {
  b_quickBoot = !b_quickBoot;
  menuSaveBool(i_addr_quickBoot, b_quickBoot);
  menuActionMessage(b_quickBoot ? "已设为短按开机" : "已设为长按开机",
                    b_quickBoot ? "Quick boot ON" : "Quick boot OFF");
}

void cycleDriftComp() {
  const float values[] = { 0.0f, 0.05f, 0.075f, 0.1f, 0.2f };
  int index = 0;
  for (int i = 0; i < (int)getArraySize(values); i++) {
    if (fabsf(f_maxDriftCompensation - values[i]) < 0.0001f) index = i;
  }
  f_maxDriftCompensation = values[(index + 1) % getArraySize(values)];
  EEPROM.put(i_addr_driftCompensation, f_maxDriftCompensation);
  EEPROM.commit();
  if (fabsf(f_maxDriftCompensation) < 0.0001f) {
    actionMessage = T("已关闭漂移补偿", "Drift compensation OFF");
    t_actionMessage = millis();
    t_actionMessageDelay = 1000;
  } else {
    char msg[32];
    snprintf(msg, sizeof(msg), T("已设为%.3gg", "Set to %.3gg"), f_maxDriftCompensation);
    actionMessage = msg;
    t_actionMessage = millis();
    t_actionMessageDelay = 1000;
  }
}

void cycleSamples() {
  const int values[] = { 1, 2, 4 };
  int index = 0;
  for (int i = 0; i < (int)getArraySize(values); i++) {
    if (i_samplesInUse == values[i]) index = i;
  }
  i_samplesInUse = values[(index + 1) % getArraySize(values)];
  scale.setSamplesInUse(i_samplesInUse);
  menuSaveInt(i_addr_samplesInUse, i_samplesInUse);
  char msg[32];
  snprintf(msg, sizeof(msg), T("采样数已设为%d", "Samples set to %d"), i_samplesInUse);
  actionMessage = msg;
  t_actionMessage = millis();
  t_actionMessageDelay = 1000;
}

void toggleAutoSubtract() {
  b_containerSubtractEnabled = !b_containerSubtractEnabled;
  menuSaveBool(i_addr_containerSubtract, b_containerSubtractEnabled);
  menuActionMessage(b_containerSubtractEnabled ? "自动扣除已开启" : "自动扣除已关闭",
                    b_containerSubtractEnabled ? "Auto subtract ON" : "Auto subtract OFF");
}

void clearContainer() {
  f_weight_container = 0.0f;
  EEPROM.put(i_addr_container, f_weight_container);
  EEPROM.commit();
  menuActionMessage("容器重量已清除", "Container cleared");
}

void toggleBle() {
  bleSetEnabled(!b_ble_enabled);
  menuActionMessage(b_ble_enabled ? "蓝牙已开启" : "蓝牙已关闭",
                    b_ble_enabled ? "Bluetooth ON" : "Bluetooth OFF");
}

void toggleHeartbeat() {
  b_requireHeartBeat = !b_requireHeartBeat;
  menuSaveBool(i_addr_requireHeartBeat, b_requireHeartBeat);
  menuActionMessage(b_requireHeartBeat ? "心跳检测已开启" : "心跳检测已关闭",
                    b_requireHeartBeat ? "Heartbeat ON" : "Heartbeat OFF");
}

void toggleBleButtons() {
  b_btnFuncWhileConnected = !b_btnFuncWhileConnected;
  menuSaveBool(i_addr_btnFuncWhileConnected, b_btnFuncWhileConnected);
  menuActionMessage(b_btnFuncWhileConnected ? "BLE按键已开启" : "BLE按键已关闭",
                    b_btnFuncWhileConnected ? "BLE buttons ON" : "BLE buttons OFF");
}

void toggleFlipScreen() {
  b_screenFlipped = !b_screenFlipped;
  menuSaveBool(i_addr_screenFlipped, b_screenFlipped);
  menuActionMessage(b_screenFlipped ? "屏幕已翻转" : "屏幕已恢复",
                    b_screenFlipped ? "Screen flipped" : "Screen normal");
}

void toggleTimeOnTop() {
  b_timeOnTop = !b_timeOnTop;
  menuSaveBool(i_addr_timeOnTop, b_timeOnTop);
  menuActionMessage(b_timeOnTop ? "时间显示在顶部" : "时间显示在底部",
                    b_timeOnTop ? "Time on top" : "Time at bottom");
}

void toggleLanguage() {
  i_language = (i_language == 0) ? 1 : 0;
  menuSaveInt(i_addr_language, i_language);
  actionMessage = (i_language == 0) ? "已切换为中文" : "Language: English";
  t_actionMessage = millis();
  t_actionMessageDelay = 1000;
}

void showDeviceInfo() {
  b_aboutFromMenu = true;  //退出时回到当前菜单（信息子菜单）
  b_menu = false;
  b_debug = false;
  b_show_about = true;
}

void showAbout() {
  showDeviceInfo();
}

void showMenu() {
#ifdef ROTATION_180
  u8g2.setDisplayRotation(b_screenFlipped ? U8G2_R0 : U8G2_R2);
#else
  u8g2.setDisplayRotation(b_screenFlipped ? U8G2_R2 : U8G2_R0);
#endif
  u8g2.setFont(FONT_XS);
  u8g2.firstPage();
  do {
    if (millis() - t_actionMessage < t_actionMessageDelay) {
      u8g2.setFont(FONT_M);
      if (AC((char *)actionMessage.c_str()) < 0)
        u8g2.setFont(FONT_S);
      u8g2.drawUTF8(AC((char *)actionMessage.c_str()), AM() - 5,
                    (char *)actionMessage.c_str());
    } else {
      u8g2.setFont(FONT_XS);
      currentPage = currentIndex / linesPerPage + 1;
      totalPages = (currentMenuSize + linesPerPage - 1) / linesPerPage;
      char pageInfo[10];
      snprintf(pageInfo, sizeof(pageInfo), "%d/%d", currentPage, totalPages);
      if (totalPages > 1)
        u8g2.drawUTF8(AR(pageInfo), u8g2.getMaxCharHeight(), pageInfo);
      for (int i = 0; i < currentMenuSize; i++) {
        if (currentMenu[i] == currentSelection) {
          u8g2.drawUTF8(0, (u8g2.getMaxCharHeight() + 2) * (i % linesPerPage + 1) - 2, ">");
        }
        if (i >= (currentPage - 1) * linesPerPage && i < currentPage * linesPerPage) {
          char label[32];
          const char *text = menuItemLabel(currentMenu[i], label, sizeof(label));
          u8g2.drawUTF8(10, (u8g2.getMaxCharHeight() + 2) * (i % linesPerPage + 1) - 2, (char *)text);
        }
      }
    }
  } while (u8g2.nextPage());
}

void navigateMenu(int direction) {
  currentIndex = (currentIndex + direction + currentMenuSize) % currentMenuSize;
  currentSelection = currentMenu[currentIndex];
  Serial.print("currentIndex ");
  Serial.println(currentIndex);
}

//回到主菜单（BLE 0x1E 显隐菜单时使用）
void resetMenuToMain() {
  currentMenu = mainMenu;
  currentMenuSize = getArraySize(mainMenu);
  currentIndex = 0;
  currentSelection = mainMenu[0];
  currentPage = 0;
}

void backToParent() {
  Menu *origin = currentSelection->parentMenu;
  currentMenu = mainMenu;
  currentMenuSize = getArraySize(mainMenu);
  currentIndex = 0;
  for (int i = 0; i < currentMenuSize; i++) {
    if (mainMenu[i] == origin) currentIndex = i;
  }
  currentSelection = currentMenu[currentIndex];
}

void selectMenu() {
  if (currentSelection->subMenu) {
    if (currentSelection == &menuScale) {
      currentMenu = scaleMenu;
      currentMenuSize = getArraySize(scaleMenu);
    } else if (currentSelection == &menuContainer) {
      currentMenu = containerMenu;
      currentMenuSize = getArraySize(containerMenu);
    } else if (currentSelection == &menuConnections) {
      currentMenu = connectionsMenu;
      currentMenuSize = getArraySize(connectionsMenu);
    } else if (currentSelection == &menuDisplay) {
      currentMenu = displayMenu;
      currentMenuSize = getArraySize(displayMenu);
    } else if (currentSelection == &menuPower) {
      currentMenu = powerMenu;
      currentMenuSize = getArraySize(powerMenu);
    } else if (currentSelection == &menuInfo) {
      currentMenu = infoMenu;
      currentMenuSize = getArraySize(infoMenu);
    }
    currentIndex = 0;
    currentSelection = currentMenu[currentIndex];
  } else if (currentSelection->action) {
    currentSelection->action();
  } else if (currentSelection->parentMenu) {
    backToParent();
  }
}

// ---------- container weight recording ----------
void setContainerWeight() {
  b_menu = false;
  i_setContainerWeight = 0;
  b_set_container = true;
  int samplesBefore = scale.getSamplesInUse();
  scale.setSamplesInUse(4);
  while (b_set_container) {
    buttonTare.check();
    buttonSet.check();
#ifdef FIVE_BUTTON
    buttonPower.check();
#endif
    switch (i_setContainerWeight) {
      case 0: {
        if (b_tareByButton && millis() - t_tareByButton > i_tareDelay) {
          scale.tareNoDelay();
          b_tareByButton = false;
        }
        scale.getTareStatus();
        static boolean newDataReady = 0;
        if (scale.update()) newDataReady = true;
        if (newDataReady) {
          f_weight_adc = scale.getData();
          circularBuffer[bufferIndex] = f_weight_adc;
          bufferIndex = (bufferIndex + 1) % windowLength;
          f_weight_smooth = 0;
          for (int i = 0; i < windowLength; i++) f_weight_smooth += circularBuffer[i];
          f_weight_smooth /= windowLength;
          f_displayedValue = f_weight_smooth;
          f_weight_container = f_displayedValue;
          char c_temp[10];
          dtostrf(f_weight_container, 7, i_decimal_precision, c_temp);
          refreshOLED((char *)T("设定容器重量", "Set container"),
                      (char *)T("按录入按钮保存", "Press INPUT to save"),
                      (char *)trim(c_temp));
        }
        break;
      }
      case 1:
        delay(300);
        EEPROM.put(i_addr_container, f_weight_container);
        EEPROM.commit();
        b_set_container = false;
        b_menu = true;
        scale.setSamplesInUse(samplesBefore);
        break;
    }
  }
  Serial.print(F("ContainerWeight set to: "));
  Serial.println(f_weight_container);
  char containerInfo[24];
  snprintf(containerInfo, sizeof(containerInfo), T("已录入%.1fg", "Saved %.1fg"), f_weight_container);
  actionMessage = containerInfo;
  t_actionMessage = millis();
}

// ---------- WiFi update ----------
void wifiUpdate() {
  u8g2.setFont(FONT_M);
  u8g2.firstPage();
  do {
    u8g2.drawUTF8(AC((char *)T("WiFi升级", "WiFi Update")), AM() - 5,
                  (char *)T("WiFi升级", "WiFi Update"));
  } while (u8g2.nextPage());
#ifdef BUZZER
  buzzer.off();
#endif
  delay(1000);
  if (b_webuiEnabled) webuiStop();  //两个服务都用 80 端口
  wifiOta();
  b_menu = false;
}

// ==========================================================================
// Calibration (ported from openscale)
// ==========================================================================
struct CalibrationRawCapture {
  long raw = 0;
  long firstRaw = 0;
  long secondRaw = 0;
  long spread = 0;
  uint8_t validSamples = 0;
  bool signalTimeout = false;
  bool dataOutOfRange = false;
};

static bool b_calibrationSampleWindowActive = false;
static bool b_calibrationZeroCaptured = false;
static CalibrationRawCapture calibrationZeroCapture;

void calibrationSetStatus(CalibrationRejectReason reason) {
  snprintf(c_calibrationStatus, sizeof(c_calibrationStatus), "%s",
           calibrationRejectReasonText(reason));
  b_calibrationInvalid = reason != CAL_REJECT_NONE;
}

void calibrationResetLastDiagnostics() {
  f_lastCalibrationCandidate = 0.0f;
  f_lastCalibrationVerifiedWeight = 0.0f;
  i_lastCalibrationZeroRaw = 0;
  i_lastCalibrationLoadRaw = 0;
  i_lastCalibrationRawDelta = 0;
  i_lastCalibrationSpread = 0;
}

void calibrationEnsureSampleWindow() {
  if (!b_calibrationSampleWindowActive) {
    scale.setSamplesInUse(16);
    b_calibrationSampleWindowActive = true;
  }
}

void calibrationRestoreSampleWindow() {
  if (b_calibrationSampleWindowActive) {
    scale.setSamplesInUse(i_samplesInUse);
    b_calibrationSampleWindowActive = false;
  }
}

void calibrationFinish(bool returnToMenu) {
  const bool restoreMenu = returnToMenu || calibrationReturnToMenu;
  calibrationReturnToMenu = false;
  i_button_cal_status = 0;
  b_calibration = false;
  b_calibrationZeroCaptured = false;
  scale.getTareStatus();
  calibrationRestoreSampleWindow();
  if (restoreMenu) {
    b_menu = true;
    resetMenuToMain();
  }
}

void calibrationPrintDiagnostics(CalibrationRejectReason reason,
                                 float previousFactor,
                                 float candidateFactor,
                                 const CalibrationRawCapture *zeroCapture,
                                 const CalibrationRawCapture *loadCapture,
                                 float verifiedWeight) {
  long zeroRaw = zeroCapture != nullptr ? zeroCapture->raw : 0;
  long loadRaw = loadCapture != nullptr ? loadCapture->raw : 0;
  long rawDelta = loadRaw - zeroRaw;
  Serial.print(F("Calibration "));
  Serial.print(reason == CAL_REJECT_NONE ? F("accepted") : F("failed"));
  Serial.print(F(": reason="));
  Serial.print(calibrationRejectReasonText(reason));
  Serial.print(F(" prev="));
  Serial.print(previousFactor, 6);
  Serial.print(F(" candidate="));
  Serial.print(candidateFactor, 6);
  Serial.print(F(" verified_g="));
  Serial.print(verifiedWeight, 4);
  Serial.print(F(" zero_raw="));
  Serial.print(zeroRaw);
  Serial.print(F(" load_raw="));
  Serial.print(loadRaw);
  Serial.print(F(" delta="));
  Serial.print(rawDelta);
  Serial.print(F(" zero_spread="));
  Serial.print(zeroCapture != nullptr ? zeroCapture->spread : 0);
  Serial.print(F(" load_spread="));
  Serial.println(loadCapture != nullptr ? loadCapture->spread : 0);
}

void calibrationRecordDiagnostics(CalibrationRejectReason reason,
                                  float candidateFactor,
                                  const CalibrationRawCapture *zeroCapture,
                                  const CalibrationRawCapture *loadCapture,
                                  float verifiedWeight) {
  calibrationSetStatus(reason);
  f_lastCalibrationCandidate = candidateFactor;
  f_lastCalibrationVerifiedWeight = verifiedWeight;
  i_lastCalibrationZeroRaw = zeroCapture != nullptr ? zeroCapture->raw : 0;
  i_lastCalibrationLoadRaw = loadCapture != nullptr ? loadCapture->raw : 0;
  i_lastCalibrationRawDelta = i_lastCalibrationLoadRaw - i_lastCalibrationZeroRaw;
  long zeroSpread = zeroCapture != nullptr ? zeroCapture->spread : 0;
  long loadSpread = loadCapture != nullptr ? loadCapture->spread : 0;
  i_lastCalibrationSpread = zeroSpread > loadSpread ? zeroSpread : loadSpread;
}

// 校准失败原因（中文/英文）
const char* calibrationReasonDisplay(CalibrationRejectReason reason) {
  if (i_language == 1) return calibrationRejectReasonDisplayText(reason);
  switch (reason) {
    case CAL_REJECT_NONE: return "正常";
    case CAL_REJECT_ADC_TIMEOUT: return "ADC超时";
    case CAL_REJECT_INSUFFICIENT_SAMPLES: return "采样不足";
    case CAL_REJECT_ADC_RANGE: return "ADC超量程";
    case CAL_REJECT_UNSTABLE_ZERO: return "零点不稳";
    case CAL_REJECT_UNSTABLE_LOAD: return "加载不稳";
    case CAL_REJECT_UNKNOWN_MASS: return "未知砝码";
    case CAL_REJECT_RAW_DELTA_TOO_SMALL: return "重量变化太小";
    case CAL_REJECT_FACTOR_NEAR_ZERO: return "系数接近0";
    case CAL_REJECT_FACTOR_TOO_LARGE: return "系数过大";
    case CAL_REJECT_FACTOR_SIGN: return "系数符号错误";
    case CAL_REJECT_VERIFY_INVERTED: return "重量反向";
    case CAL_REJECT_VERIFY_TOLERANCE: return "复核失败";
    case CAL_REJECT_SMART_CAL_DISABLED: return "智能校准未启用";
    case CAL_REJECT_UNPLUG_USB: return "请拔出USB";
    default: return "未知错误";
  }
}

void calibrationShowFailure(CalibrationRejectReason reason) {
  const char *displayText = calibrationReasonDisplay(reason);
  Serial.print(F("Calibration failed: "));
  Serial.println(calibrationRejectReasonText(reason));
  u8g2.firstPage();
  u8g2.setFont(FONT_S);
  do {
    u8g2.drawUTF8(AC((char *)T("校准失败", "Calibration failed")),
                  u8g2.getMaxCharHeight() + i_margin_top,
                  (char *)T("校准失败", "Calibration failed"));
    u8g2.drawUTF8(AC((char *)displayText), LCDHeight - i_margin_bottom,
                  (char *)displayText);
  } while (u8g2.nextPage());
#ifdef BUZZER
  buzzer.off();
#endif
  delay(1000);
}

void calibrationShowUsbWarning() {
  Serial.println(F("Calibration warning: USB connected"));
  u8g2.firstPage();
  u8g2.setFont(FONT_S);
  do {
    u8g2.drawUTF8(AC((char *)T("请拔出USB线", "Unplug USB")),
                  u8g2.getMaxCharHeight() + i_margin_top,
                  (char *)T("请拔出USB线", "Unplug USB"));
    u8g2.drawUTF8(AC((char *)T("5秒后校准", "Cal in 5s")),
                  LCDHeight - i_margin_bottom,
                  (char *)T("5秒后校准", "Cal in 5s"));
  } while (u8g2.nextPage());
#ifdef BUZZER
  buzzer.off();
#endif
  delay(5000);
}

long calibrationRawSpread(long firstRaw, long secondRaw) {
  long spread = secondRaw - firstRaw;
  return spread < 0 ? -spread : spread;
}

void calibrationStoreRawCapture(CalibrationRawCapture &capture,
                                const ADS1232DebugInfo &firstInfo,
                                const ADS1232DebugInfo &secondInfo,
                                long spread) {
  capture.firstRaw = firstInfo.smoothedValue;
  capture.secondRaw = secondInfo.smoothedValue;
  capture.spread = spread;
  capture.raw = (capture.firstRaw + capture.secondRaw) / 2;
  capture.validSamples = firstInfo.validSamples < secondInfo.validSamples
                           ? firstInfo.validSamples
                           : secondInfo.validSamples;
  capture.signalTimeout = firstInfo.signalTimeout || secondInfo.signalTimeout;
  capture.dataOutOfRange = firstInfo.dataOutOfRange || secondInfo.dataOutOfRange;
}

bool calibrationCaptureRaw(CalibrationRawCapture &capture,
                           CalibrationRejectReason unstableReason,
                           CalibrationRejectReason &failureReason) {
  calibrationEnsureSampleWindow();
  capture = CalibrationRawCapture();
  unsigned long start = millis();
  unsigned long lastUpdate = start;
  float stabilityLimit = calibrationStabilityRawLimit(f_calibration_value);
  int previousReadIndex = -1;
  bool hasPrevious = false;
  uint8_t stableReads = 0;
  unsigned long stableStartedAt = 0;
  long stableMin = 0;
  long stableMax = 0;
  ADS1232DebugInfo previousInfo;

  while (millis() - start < CALIBRATION_CAPTURE_TIMEOUT_MS) {
    if (scale.update()) {
      lastUpdate = millis();
      ADS1232DebugInfo currentInfo = scale.getDebugInfo();
      if (currentInfo.signalTimeout) {
        capture.signalTimeout = true;
        failureReason = CAL_REJECT_ADC_TIMEOUT;
        return false;
      }
      if (currentInfo.dataOutOfRange) {
        capture.dataOutOfRange = true;
        failureReason = CAL_REJECT_ADC_RANGE;
        return false;
      }
      if (currentInfo.validSamples >= CALIBRATION_MIN_VALID_SAMPLES &&
          currentInfo.readIndex != previousReadIndex) {
        previousReadIndex = currentInfo.readIndex;
        if (hasPrevious) {
          long spread = calibrationRawSpread(previousInfo.smoothedValue,
                                             currentInfo.smoothedValue);
          calibrationStoreRawCapture(capture, previousInfo, currentInfo, spread);
          if ((float)spread <= stabilityLimit) {
            if (stableReads == 0) {
              stableStartedAt = millis();
              stableMin = previousInfo.smoothedValue;
              stableMax = previousInfo.smoothedValue;
            }
            if (currentInfo.smoothedValue < stableMin) stableMin = currentInfo.smoothedValue;
            if (currentInfo.smoothedValue > stableMax) stableMax = currentInfo.smoothedValue;
            stableReads++;
          } else {
            stableReads = 0;
            stableStartedAt = 0;
          }
          long stableWindowSpread = stableMax - stableMin;
          if (stableReads >= CALIBRATION_STABLE_READS_REQUIRED &&
              millis() - stableStartedAt >= CALIBRATION_STABLE_HOLD_MS &&
              (float)stableWindowSpread <= stabilityLimit) {
            failureReason = CAL_REJECT_NONE;
            return true;
          }
        } else {
          capture.raw = currentInfo.smoothedValue;
          capture.validSamples = currentInfo.validSamples;
          hasPrevious = true;
        }
        previousInfo = currentInfo;
      }
    } else if (millis() - lastUpdate > 1500 || scale.getSignalTimeoutFlag()) {
      capture.signalTimeout = true;
      failureReason = CAL_REJECT_ADC_TIMEOUT;
      return false;
    }
    delay(2);
    yield();
  }

  ADS1232DebugInfo info = scale.getDebugInfo();
  failureReason = capture.validSamples < CALIBRATION_MIN_VALID_SAMPLES &&
                          info.validSamples < CALIBRATION_MIN_VALID_SAMPLES
                    ? CAL_REJECT_INSUFFICIENT_SAMPLES
                    : unstableReason;
  return false;
}

void calibrationFail(CalibrationRejectReason reason,
                     float previousFactor,
                     float candidateFactor,
                     const CalibrationRawCapture *zeroCapture,
                     const CalibrationRawCapture *loadCapture,
                     float verifiedWeight) {
  f_calibration_value = previousFactor;
  scale.setCalFactor(f_calibration_value);
  calibrationRecordDiagnostics(reason, candidateFactor, zeroCapture, loadCapture,
                               verifiedWeight);
  calibrationPrintDiagnostics(reason, previousFactor, candidateFactor,
                              zeroCapture, loadCapture, verifiedWeight);
  calibrationShowFailure(reason);
  calibrationFinish(false);
}

void calibrationSave(float previousCalibrationValue,
                     float newCalibrationValue,
                     const CalibrationRawCapture &zeroCapture,
                     const CalibrationRawCapture &loadCapture,
                     float verifiedWeight) {
  f_calibration_value = newCalibrationValue;
  scale.setCalFactor(f_calibration_value);
  EEPROM.put(i_addr_calibration_value, f_calibration_value);
  EEPROM.commit();
  f_driftCompensation = 0.0f;
  resetTracking();
  resetStableOutput();
  calibrationRecordDiagnostics(CAL_REJECT_NONE, f_calibration_value,
                               &zeroCapture, &loadCapture, verifiedWeight);
  calibrationPrintDiagnostics(CAL_REJECT_NONE, previousCalibrationValue,
                              f_calibration_value, &zeroCapture, &loadCapture,
                              verifiedWeight);
}

void calibrate() {
  calibrationReturnToMenu = b_menu;
  b_menu = false;
  i_cal_weight = 0;
  i_button_cal_status = 1;
  b_calibration = true;
  i_calibration = 0;
}

void calibration(int input) {
  if (b_calibration == true) {
    char c_calval[25];
    if (input == 1) {  //智能校准（与openscale一致：默认禁用）
      calibrationFail(CAL_REJECT_SMART_CAL_DISABLED, f_calibration_value, 0.0f,
                      nullptr, nullptr, 0.0f);
      return;
    }
    if (i_button_cal_status == 1) {
      if (input == 0) {
        if (!b_calibrationSampleWindowActive) {
          calibrationResetLastDiagnostics();
          b_calibrationZeroCaptured = false;
        }
        calibrationEnsureSampleWindow();
        u8g2.firstPage();
        do {
          if (b_screenFlipped)
            u8g2.setDisplayRotation(U8G2_R0);
          else
            u8g2.setDisplayRotation(U8G2_R2);
          u8g2.setFontMode(1);
          u8g2.setDrawColor(1);
          //紧凑布局：标题 + 3 行选项，14px 字体，固定行距 14（实测墨迹 2..56，留 7px 底边）
          //wqy14 中文墨迹: 基线-12 ~ 基线；英文含 g: 基线-10 ~ 基线+2
          u8g2.setFont(FONT_S);
          const int lineH = 14;
          int yTitle = lineH;
          int yRow[3] = { lineH * 2, lineH * 3, lineH * 4 };
          u8g2.drawUTF8(0, yTitle, (char *)T("校准砝码重量", "Cal Weight"));
          u8g2.drawUTF8(5, yRow[0], (char *)weightLabel(0));
          u8g2.drawUTF8(64, yRow[0], (char *)weightLabel(3));
          u8g2.drawUTF8(5, yRow[1], (char *)weightLabel(1));
          u8g2.drawUTF8(64, yRow[1], (char *)weightLabel(4));
          u8g2.drawUTF8(5, yRow[2], (char *)weightLabel(2));
          u8g2.drawUTF8(64, yRow[2], (char *)weightLabel(5));
          int row = (i_cal_weight < 3) ? i_cal_weight : (i_cal_weight - 3);
          int markerX = (i_cal_weight < 3) ? 0 : 59;
          int my = yRow[row];
          int y0 = my - lineH + 4;
          int y2 = my - (my - y0) / 2;
          u8g2.drawTriangle(markerX, y0, markerX, my, markerX + 4, y2);
          u8g2.setDrawColor(2);
          drawButtonBox();
        } while (u8g2.nextPage());
      }
      if (input == 1) {
        calibrationEnsureSampleWindow();
        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)T("请拿走所有砝码", "Remove all weight")),
                        u8g2.getMaxCharHeight() + i_margin_top + 3,
                        (char *)T("请拿走所有砝码", "Remove all weight"));
          u8g2.drawUTF8(AC((char *)T("来开始校准", "to start")),
                        LCDHeight - i_margin_bottom - 3,
                        (char *)T("来开始校准", "to start"));
        } while (u8g2.nextPage());
      }
    }
    if (i_button_cal_status == 2) {
      if (i_cal_weight == 0) {
        calibrationFinish(true);
        return;
      }
      if (b_is_charging) {
        calibrationShowUsbWarning();
      }
      if (input == 0) {
        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)T("请移开砝码", "Remove weight")),
                        u8g2.getMaxCharHeight() + i_margin_top,
                        (char *)T("请移开砝码", "Remove weight"));
          u8g2.drawUTF8(AC((char *)T("离开秤盘", "from scale")),
                        LCDHeight - i_margin_bottom,
                        (char *)T("离开秤盘", "from scale"));
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(2000);
      }
      for (int i = 3; i >= 1; i--) {
        char waitText[16];
        snprintf(waitText, sizeof(waitText), T("倒计时:%d", "Wait: %ds"), i);
        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)T("0g校准中", "Calibrating 0g")),
                        u8g2.getMaxCharHeight() + i_margin_top,
                        (char *)T("0g校准中", "Calibrating 0g"));
          u8g2.drawUTF8(AC((char *)waitText), LCDHeight - i_margin_bottom, waitText);
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(1000);
      }
      float previousCalibrationValue = f_calibration_value;
      CalibrationRejectReason zeroFailureReason = CAL_REJECT_NONE;
      if (!calibrationCaptureRaw(calibrationZeroCapture,
                                 CAL_REJECT_UNSTABLE_ZERO,
                                 zeroFailureReason)) {
        calibrationFail(zeroFailureReason, previousCalibrationValue, 0.0f,
                        &calibrationZeroCapture, nullptr, 0.0f);
        return;
      }
      scale.tare();
      scale.getTareStatus();
      b_calibrationZeroCaptured = true;
      i_lastCalibrationZeroRaw = calibrationZeroCapture.raw;
      i_lastCalibrationLoadRaw = 0;
      i_lastCalibrationRawDelta = 0;
      i_lastCalibrationSpread = calibrationZeroCapture.spread;
      Serial.print(F("0g raw captured: "));
      Serial.print(calibrationZeroCapture.raw);
      Serial.print(F(" spread="));
      Serial.print(calibrationZeroCapture.spread);
      Serial.print(F(" valid="));
      Serial.println(calibrationZeroCapture.validSamples);
      u8g2.firstPage();
      u8g2.setFont(FONT_S);
      do {
        u8g2.drawUTF8(AC((char *)T("0g校准完成", "0g cal done")), AM(),
                      (char *)T("0g校准完成", "0g cal done"));
      } while (u8g2.nextPage());
#ifdef BUZZER
      buzzer.beep(1, BUZZER_DURATION);
      buzzer.off();
#endif
      delay(1000);
      i_button_cal_status++;
    }
    if (i_button_cal_status == 3) {
      if (input == 0) {
        float known_mass = weight_values[i_cal_weight];
        char buffer[50];
        snprintf(buffer, sizeof(buffer), T("请放上 %s 砝码", "Place %s weight"),
                 weightLabel(i_cal_weight));
        for (int i = 3; i >= 1; i--) {
          char waitText[16];
          snprintf(waitText, sizeof(waitText), T("倒计时:%d", "Wait: %ds"), i);
          u8g2.firstPage();
          u8g2.setFont(FONT_S);
          do {
            u8g2.drawUTF8(AC((char *)trim(buffer)),
                          u8g2.getMaxCharHeight() + i_margin_top,
                          (char *)trim(buffer));
            u8g2.drawUTF8(AC((char *)waitText), LCDHeight - i_margin_bottom, waitText);
          } while (u8g2.nextPage());
#ifdef BUZZER
          buzzer.off();
#endif
          delay(1000);
        }
        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)T("正在校准", "Calibrating")), AM(),
                        (char *)T("正在校准", "Calibrating"));
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(1000);
        float previousCalibrationValue = f_calibration_value;
        if (!b_calibrationZeroCaptured) {
          calibrationFail(CAL_REJECT_UNSTABLE_ZERO, previousCalibrationValue,
                          0.0f, nullptr, nullptr, 0.0f);
          return;
        }
        CalibrationRawCapture loadCapture;
        CalibrationRejectReason loadFailureReason = CAL_REJECT_NONE;
        if (!calibrationCaptureRaw(loadCapture, CAL_REJECT_UNSTABLE_LOAD,
                                   loadFailureReason)) {
          calibrationFail(loadFailureReason, previousCalibrationValue, 0.0f,
                          &calibrationZeroCapture, &loadCapture, 0.0f);
          return;
        }
        long rawDelta = loadCapture.raw - calibrationZeroCapture.raw;
        float candidateCalibrationValue = (float)rawDelta / known_mass;
        CalibrationRejectReason validationReason =
          validateCalibrationCandidateBasics(known_mass, rawDelta,
                                             candidateCalibrationValue);
        if (validationReason != CAL_REJECT_NONE) {
          calibrationFail(validationReason, previousCalibrationValue,
                          candidateCalibrationValue, &calibrationZeroCapture,
                          &loadCapture, 0.0f);
          return;
        }
        scale.setCalFactor(candidateCalibrationValue);
        float verifiedWeight = scale.getData();
        validationReason = validateCalibrationVerification(known_mass,
                                                           verifiedWeight);
        if (validationReason != CAL_REJECT_NONE) {
          calibrationFail(validationReason, previousCalibrationValue,
                          candidateCalibrationValue, &calibrationZeroCapture,
                          &loadCapture, verifiedWeight);
          return;
        }
        calibrationSave(previousCalibrationValue, candidateCalibrationValue,
                        calibrationZeroCapture, loadCapture, verifiedWeight);
        Serial.print(F("New calibration value f: "));
        Serial.println(f_calibration_value);
        dtostrf(f_calibration_value, 10, 2, c_calval);
        Serial.print(F("New calibration value c: "));
        Serial.println(trim(c_calval));
        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)T("校准已完成", "Calibration done")), AM(),
                        (char *)T("校准已完成", "Calibration done"));
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(1000);
#ifdef BUZZER
        buzzer.beep(1, BUZZER_DURATION);
        buzzer.off();
#endif
        delay(1000);
        calibrationFinish(false);
        return;
      }
    }
  }
}

#endif
