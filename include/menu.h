#ifndef MENU_H
#define MENU_H

const char *weights[] = { "退出", "50克", "100克", "200克", "500克", "1000克" };
const float weight_values[] = { 0.0, 50.0, 100.0, 200.0, 500.0, 1000.0 };
long t_actionMessage = 0;
int t_actionMessageDelay = 1000;
String actionMessage = "Default";
String actionMessage2 = "Default";
// template<typename T> int getArraySize(T &menu) {
//   return sizeof(menu) / sizeof(menu[0]);
// }
template<typename T, size_t N>
constexpr size_t getArraySize(T (&arr)[N]) {
    return N;
}

// Menu structure
struct Menu {
  const char *name;  //menu name
  void (*action)();  //what to do NULL for submenu
  Menu *subMenu;     //submenu NULL for none
  Menu *parentMenu;  //parentmenu NULL for root menu
};

// Function prototypes
void exitMenu();
#ifdef BUZZER

void buzzerOn();
void buzzerOff();
#endif
void calibrate();
void wifiUpdate();
void showAbout();
void showMenu();
void calibrateVoltage();
void navigateMenu(int direction);
void selectMenu();
void enableDebug();
void setContainerWeight();
void drawButtonBox();
void autoSleepOn();
void autoSleepOff();
void quickBootOn();
void quickBootOff();
void driftCompOff();
void driftComp0050();
void driftComp0075();
void driftComp0100();
void driftComp0200();

// Top-level menu options
// 1/5 define the 1st level menu
Menu menuExit = { "退出", exitMenu, NULL, NULL };
#ifdef BUZZER

Menu menuBuzzer = { "蜂鸣器设置", NULL, NULL, NULL };
#endif
Menu menuContainer = { "自动扣除设置", NULL, NULL, NULL };
Menu menuCalibration = { "称重校准", NULL, NULL, NULL };
Menu menuWiFiUpdate = { "WiFi升级", NULL, NULL, NULL };
Menu menuAbout = { "设备信息", showAbout, NULL, NULL };
Menu menuFactory = { "开发选项", NULL, NULL, NULL };
Menu menuAutoSleep = { "自动休眠", NULL, NULL, NULL };
Menu menuQuickBoot = { "快速开机", NULL, NULL, NULL };
Menu menuDriftComp = { "漂移补偿", NULL, NULL, NULL };
// Menu menuHolder1 = { "menuHolder1", NULL, NULL, NULL };
// Menu menuHolder2 = { "menuHolder2", NULL, NULL, NULL };
// Menu menuHolder3 = { "menuHolder3", NULL, NULL, NULL };
// Menu menuHolder4 = { "menuHolder4", NULL, NULL, NULL };
// Menu menuHolder5 = { "menuHolder5", NULL, NULL, NULL };
// Menu menuHolder6 = { "menuHolder6", NULL, NULL, NULL };

// Buzzer submenu
// 2/5 define the 2st level menu
#ifdef BUZZER

Menu menuBuzzerBack = { "上一级", NULL, NULL, &menuBuzzer };
Menu menuBuzzerOn = { "开启蜂鸣器", buzzerOn, NULL, &menuBuzzer };
Menu menuBuzzerOff = { "关闭蜂鸣器", buzzerOff, NULL, &menuBuzzer };
Menu *buzzerMenu[] = { &menuBuzzerBack, &menuBuzzerOn, &menuBuzzerOff };
#endif
// Auto minus container submenu
Menu menuContainerBack = { "上一级", NULL, NULL, &menuContainer };
Menu menuContainerStart = { "设置容器重量", setContainerWeight, NULL, &menuContainer };
Menu *containerMenu[] = { &menuContainerBack, &menuContainerStart };

// Calibration submenu
Menu menuCalibrationBack = { "上一级", NULL, NULL, &menuCalibration };
Menu menuCalibrate = { "开始校准", calibrate, NULL, &menuCalibration };
Menu *calibrationMenu[] = { &menuCalibrationBack, &menuCalibrate };

// WiFi Update submenu
Menu menuWiFiUpdateBack = { "上一级", NULL, NULL, &menuWiFiUpdate };
Menu menuWiFiUpdateOption = { "开始WiFi升级", wifiUpdate, NULL, &menuWiFiUpdate };
Menu *wifiUpdateMenu[] = { &menuWiFiUpdateBack, &menuWiFiUpdateOption };

Menu menuFactoryBack = { "上一级", NULL, NULL, &menuFactory };
Menu menuCalibrateVoltage = { "校准电压4.2v", calibrateVoltage, NULL, &menuFactory };
Menu menuFactoryDebug = { "Debug信息", enableDebug, NULL, &menuFactory };
Menu *factoryMenu[] = { &menuFactoryBack, &menuCalibrateVoltage, &menuFactoryDebug };

// Auto sleep function
Menu menuAutoSleepBack = { "上一级", NULL, NULL, &menuAutoSleep };
Menu menuAutoSleepOn = { "开启自动关机", autoSleepOn, NULL, &menuAutoSleep };
Menu menuAutoSleepOff = { "禁止自动关机", autoSleepOff, NULL, &menuAutoSleep };
Menu *autoSleepMenu[] = { &menuAutoSleepBack, &menuAutoSleepOn, &menuAutoSleepOff };

// Quick boot function(aka no delay when pressing the button to boot the scale)
Menu menuQuickBootBack = { "上一级", NULL, NULL, &menuQuickBoot };
Menu menuQuickBootOn = { "短按开机", quickBootOn, NULL, &menuQuickBoot };
Menu menuQuickBootOff = { "长按开机", quickBootOff, NULL, &menuQuickBoot };
Menu *quickBootMenu[] = { &menuQuickBootBack, &menuQuickBootOn, &menuQuickBootOff };

Menu menuDriftCompBack = { "上一级", NULL, NULL, &menuDriftComp };
Menu menuDriftCompOff = { "关闭漂移补偿", driftCompOff, NULL, &menuDriftComp };
Menu menuQuickBoot0050 = { "0.05g", driftComp0050, NULL, &menuDriftComp };
Menu menuQuickBoot0075 = { "0.075g", driftComp0075, NULL, &menuDriftComp };
Menu menuQuickBoot0100 = { "0.1g", driftComp0100, NULL, &menuDriftComp };
Menu menuQuickBoot0200 = { "0.2g", driftComp0200, NULL, &menuDriftComp };
Menu *driftCompMenu[] = { &menuDriftCompBack, &menuDriftCompOff, &menuQuickBoot0050, &menuQuickBoot0075, &menuQuickBoot0100, &menuQuickBoot0200 };


// Main menu
// 3/5 write all the 1st menu to mainMenu
Menu *mainMenu[] = { &menuExit,
#ifdef BUZZER
                     &menuBuzzer,
#endif
                     &menuContainer, &menuCalibration, &menuWiFiUpdate, &menuAbout, &menuFactory,
                     &menuAutoSleep, &menuQuickBoot, &menuDriftComp, };
//  &menuHolder1, &menuHolder2, &menuHolder3, &menuHolder4,
//  &menuHolder5, &menuHolder6};
Menu **currentMenu = mainMenu;
Menu *currentSelection = mainMenu[0];
int currentMenuSize = getArraySize(mainMenu);  // Top-level menu size
int currentIndex = 0;
const int linesPerPage = 4;                           // Maximum number of lines that can fit on the display
int currentPage = 0;                                  // Determine the current page
int totalPages = currentMenuSize / linesPerPage + 1;  // Calculate total pages

// 4/5 link all the submenus
void linkSubmenus() {
  // Link submenus
#ifdef BUZZER
  menuBuzzer.subMenu = buzzerMenu[0];
#endif
  menuContainer.subMenu = containerMenu[0];
  menuCalibration.subMenu = calibrationMenu[0];
  menuWiFiUpdate.subMenu = wifiUpdateMenu[0];
  menuFactory.subMenu = factoryMenu[0];
  menuAutoSleep.subMenu = autoSleepMenu[0];
  menuQuickBoot.subMenu = quickBootMenu[0];
  menuDriftComp.subMenu = driftCompMenu[0];
}

// Menu actions
void exitMenu() {
  u8g2.setFont(FONT_M);
  u8g2.firstPage();
  do {
    u8g2.drawUTF8(AC((char *)"退出设置"), AM() - 5, (char *)"退出设置");
  } while (u8g2.nextPage());
#ifdef BUZZER

  buzzer.off();
#endif
  delay(1000);
  b_menu = false;
  // Optionally reset or perform an exit action
}
#ifdef BUZZER

void buzzerOn() {
  if (b_beep == false) {
    b_beep = true;
    buzzer.beep(1, 50);
  }
  actionMessage = "蜂鸣器已开启";
  t_actionMessage = millis();
  EEPROM.put(i_addr_beep, b_beep);
  EEPROM.commit();
}

void buzzerOff() {
  b_beep = false;
  actionMessage = "蜂鸣器已关闭";
  t_actionMessage = millis();
  EEPROM.put(i_addr_beep, b_beep);
  EEPROM.commit();
}
#endif

void setContainerWeight() {
  b_menu = false;
  i_setContainerWeight = 0;
  b_set_container = true;
  float containerWeight = 0;
  scale.setSamplesInUse(4);
  while (b_set_container) {
    buttonTare.check();
    buttonSet.check();
#ifdef FIVE_BUTTON
    buttonPower.check();
#endif  //FIVE_BUTTON
    switch (i_setContainerWeight) {
      case 0:
        if (b_tareByButton && millis() - t_tareByButton > i_tareDelay) {
          scale.tareNoDelay();
          b_tareByButton = false;  // reset status
        }
        if (scale.getTareStatus()) {
          //完成清零
#ifdef BUZZER
          buzzer.beep(2, 50);
#endif
        }
        static boolean newDataReady = 0;
        static boolean scaleStable = 0;
        if (scale.update()) newDataReady = true;
        if (newDataReady) {
          f_weight_adc = scale.getData();
          circularBuffer[bufferIndex] = f_weight_adc;
          bufferIndex = (bufferIndex + 1) % windowLength;
          // calculate moving average
          f_weight_smooth = 0;
          for (int i = 0; i < windowLength; i++) {
            f_weight_smooth += circularBuffer[i];
          }
          f_weight_smooth /= windowLength;
          if (f_weight_smooth >= f_displayedValue - OledTolerance && f_weight_smooth <= f_displayedValue + OledTolerance) {
            // scale value is within tolerance range, do nothing
            // or weight is around 0, then set to 0.
            if (f_weight_smooth > -0.1 && f_weight_smooth < 0.1)
              f_displayedValue = 0.0;
          } else {
            // scale value is outside tolerance range, update displayed value
            f_displayedValue = f_weight_smooth;
            // print result to serial monitor
          }
          // print smoothed reading
          //Serial.print(f_displayedValue);
          f_weight_container = f_displayedValue;
          char c_temp[10];
          dtostrf(f_weight_container, 7, i_decimal_precision, c_temp);
#ifdef CHINESE
          refreshOLED((char *)"设定容器重量", (char *)"按录入按钮保存", (char *)trim(c_temp));
#endif
#ifdef ENGLISH
          refreshOLED((char *)"Set container weight", (char *)"Press INPUT to save", (char *)trim(c_temp));
#endif
        }
        break;
      case 1:
        delay(500);
        EEPROM.put(i_addr_container, f_weight_container);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
        EEPROM.commit();
#endif
        // #ifdef CHINESE
        //         refreshOLED((char *)"装豆容器重量", (char *)"已保存");
        // #endif
        // #ifdef ENGLISH
        //         refreshOLED((char *)"Container weight", (char *)"Saved");
        // #endif
        b_set_container = false;
        b_menu = true;
        break;
    }
  }
  Serial.print(F("ContainerWeight set to: "));
  Serial.print(f_weight_container);
  char containerInfo[20];
  snprintf(containerInfo, sizeof(containerInfo), "已录入%.1fg", f_weight_container);
  actionMessage = containerInfo;
  t_actionMessage = millis();
}

void calibrate() {
  b_menu = false;
  b_calibration = true;  //让按钮进入校准状态3
  i_calibration = 0;
  //calibration(0);
  //the calibration if is after the showMenu() if, so it should exit menu to do calibration
}

void calibration(int input) {
  if (b_calibration == true) {
    bool newDataReady = false;
    char c_calval[25];
    if (i_button_cal_status == 1) {
      if (input == 0) {
        scale.setSamplesInUse(16);
        u8g2.firstPage();
        do {
          if (b_screenFlipped)
            u8g2.setDisplayRotation(U8G2_R0);
          else
            u8g2.setDisplayRotation(U8G2_R2);
          u8g2.setFontMode(1);
          u8g2.setDrawColor(1);
          u8g2.setFont(FONT_S);
          if (b_is_charging) {
            u8g2.drawUTF8(AC((char *)"请拔出USB线"),
                          u8g2.getMaxCharHeight() + i_margin_top,
                          (char *)"请拔出USB线");
            u8g2.drawUTF8(AC((char *)"来开始校准"),
                          LCDHeight - i_margin_bottom,
                          (char *)"来开始校准");
          } else {
            int x, y;
            x = 0;
            y = u8g2.getMaxCharHeight() - 3;
            u8g2.drawUTF8(x, y, "校准砝码重量");
            // u8g2.setFont(FONT_M);
            x += 5;
            y += u8g2.getMaxCharHeight() - 1;
            u8g2.drawUTF8(x, y, weights[0]);
            x = 64;
            u8g2.drawUTF8(x, y, weights[3]);
            x = 5;
            y += u8g2.getMaxCharHeight() - 3;
            u8g2.drawUTF8(x, y, weights[1]);
            x = 64;
            u8g2.drawUTF8(x, y, weights[4]);
            x = 5;
            y += u8g2.getMaxCharHeight() - 3;
            u8g2.drawUTF8(x, y, weights[2]);
            x = 64;
            u8g2.drawUTF8(x, y, weights[5]);
            if (i_cal_weight == 0 || i_cal_weight == 3)
              y = y - (u8g2.getMaxCharHeight() - 3) * 2;
            if (i_cal_weight == 1 || i_cal_weight == 4)
              y = y - (u8g2.getMaxCharHeight() - 3);
            if (i_cal_weight == 0 || i_cal_weight == 1 || i_cal_weight == 2)
              x = 0;
            else
              x = 64 - 5;
            int x0 = x;
            int x1 = x;
            int x2 = x0 + 4;
            int y0 = y - (u8g2.getMaxCharHeight() - 3) + 6;
            int y1 = y;
            int y2 = y - (y1 - y0) / 2;
            u8g2.drawTriangle(x0, y0, x1, y1, x2, y2);
          }

          u8g2.setDrawColor(2);
          drawButtonBox();
          // drawBleBox();
        } while (u8g2.nextPage());
      }
      if (input == 1) {
        scale.setSamplesInUse(16);
        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          // 2行
          // FONT_M = u8g2_font_fub14_tn;
          u8g2.drawUTF8(AC((char *)"请拿走所有砝码"),
                        u8g2.getMaxCharHeight() + i_margin_top,
                        (char *)"请拿走所有砝码");
          u8g2.drawUTF8(AC((char *)"来开始校准"),
                        LCDHeight - i_margin_bottom,
                        (char *)"来开始校准");
        } while (u8g2.nextPage());
      }
    }
    if (i_button_cal_status == 2) {
      Serial.println("Before if check, i_cal_weight = " + String(i_cal_weight));

      if (i_cal_weight == 0 || b_is_charging) {
        // exit was selected, exit the calibration.
        i_button_cal_status = 0;
        b_calibration = false;
        b_menu = true;
        return;
      }
      // scale.update();
      if (input == 0) {
        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)"请移开砝码"), AM() - 5, (char *)"请移开砝码");
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(2000);
      }
      u8g2.firstPage();
      u8g2.setFont(FONT_S);
      do {
        u8g2.drawUTF8(AC((char *)"0g校准中"),
                      u8g2.getMaxCharHeight() + i_margin_top,
                      (char *)"0g校准中");
        u8g2.drawUTF8(AC((char *)"倒计时: 3"), LCDHeight - i_margin_bottom,
                      (char *)"倒计时: 3");
      } while (u8g2.nextPage());
#ifdef BUZZER
      buzzer.off();
#endif
      delay(1000);
      u8g2.firstPage();
      u8g2.setFont(FONT_S);
      do {
        u8g2.drawUTF8(AC((char *)"0g校准中"),
                      u8g2.getMaxCharHeight() + i_margin_top,
                      (char *)"0g校准中");
        u8g2.drawUTF8(AC((char *)"倒计时: 2"), LCDHeight - i_margin_bottom,
                      (char *)"倒计时: 2");
      } while (u8g2.nextPage());
#ifdef BUZZER
      buzzer.off();
#endif
      delay(1000);

      u8g2.firstPage();
      u8g2.setFont(FONT_S);
      do {
        u8g2.drawUTF8(AC((char *)"0g校准中"),
                      u8g2.getMaxCharHeight() + i_margin_top,
                      (char *)"0g校准中");
        u8g2.drawUTF8(AC((char *)"倒计时: 1"), LCDHeight - i_margin_bottom,
                      (char *)"倒计时: 1");
      } while (u8g2.nextPage());
#ifdef BUZZER
      buzzer.off();
#endif
      delay(1000);

      u8g2.firstPage();
      u8g2.setFont(FONT_S);
      do {
        u8g2.drawUTF8(AC((char *)"0g校准中"), AM() - 5,
                      (char *)"0g校准中");
      } while (u8g2.nextPage());

      scale.tare();
      Serial.println(F("0g calibration done"));
      u8g2.firstPage();
      u8g2.setFont(FONT_S);
      do {
        u8g2.drawUTF8(AC((char *)"0g校准完成"), AM() - 5,
                      (char *)"0g校准完成");
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
        float known_mass = 0;
        scale.update();
        known_mass = weight_values[i_cal_weight];
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "请放上 %s 砝码",
                 weights[i_cal_weight]);

        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)trim(buffer)),
                        u8g2.getMaxCharHeight() + i_margin_top,
                        (char *)trim(buffer));
          u8g2.drawUTF8(AC((char *)"倒计时: 3"), LCDHeight - i_margin_bottom,
                        (char *)"倒计时: 3");
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(1000);

        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)trim(buffer)),
                        u8g2.getMaxCharHeight() + i_margin_top,
                        (char *)trim(buffer));
          u8g2.drawUTF8(AC((char *)"倒计时: 2"), LCDHeight - i_margin_bottom,
                        (char *)"倒计时: 2");
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(1000);

        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)trim(buffer)),
                        u8g2.getMaxCharHeight() + i_margin_top,
                        (char *)trim(buffer));
          u8g2.drawUTF8(AC((char *)"倒计时: 1"), LCDHeight - i_margin_bottom,
                        (char *)"倒计时: 1");
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(1000);

        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {

          u8g2.drawUTF8(AC((char *)"正在校准"), AM() - 5, (char *)"正在校准");
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(1000);
        double d_weight;
        for (int i = 0; i < DATA_SET; i++) {
          if (scale.update())
            newDataReady = true;
          if (newDataReady) {
            d_weight = scale.getData();
            Serial.println(d_weight);
            newDataReady = false;
            delay(100);
          }
        }
        Serial.print("weight is ");
        Serial.println(d_weight);
        if (abs(d_weight) < 5) {
          u8g2.firstPage();
          u8g2.setFont(FONT_S);
          do {
            // 2行
            // FONT_M = u8g2_font_fub14_tn;
            u8g2.drawUTF8(AC((char *)"没有检测到砝码"), AM() - 5,
                          (char *)"没有检测到砝码");
          } while (u8g2.nextPage());
#ifdef BUZZER
          buzzer.off();
#endif
          delay(1000);
          // reject the weight and exit
          i_button_cal_status = 0;
          b_calibration = false;
          b_menu = true;
          return;
        }
        scale.refreshDataSet();  // refresh the dataset to be sure that the known
                                 // mass is measured correct

        f_calibration_value = scale.getNewCalibration(
          known_mass);  // get the new calibration value
        Serial.print(F("New calibration value f: "));
        Serial.println(f_calibration_value);
        // #if defined(ESP8266) || defined(ESP32) ||
        // defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
        //     EEPROM.begin(512);
        // #endif
        EEPROM.put(i_addr_calibration_value, f_calibration_value);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
        EEPROM.commit();
#endif
        dtostrf(f_calibration_value, 10, 2, c_calval);
        Serial.print(F("New calibration value c: "));
        Serial.println(trim(c_calval));

        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          // 2行
          // FONT_M = u8g2_font_fub14_tn;
          u8g2.drawUTF8(AC((char *)"校准已完成"), AM() - 5,
                        (char *)"校准已完成");
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
        b_calibration = false;
      }
      if (input == 1) {
        scale.update();
        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)"请放上任意砝码"),
                        u8g2.getMaxCharHeight() + i_margin_top,
                        (char *)"请放上任意砝码");
          u8g2.drawUTF8(AC((char *)"倒计时: 3"), LCDHeight - i_margin_bottom,
                        (char *)"倒计时: 3");
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(1000);

        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)"请放上任意砝码"),
                        u8g2.getMaxCharHeight() + i_margin_top,
                        (char *)"请放上任意砝码");
          u8g2.drawUTF8(AC((char *)"倒计时: 2"), LCDHeight - i_margin_bottom,
                        (char *)"倒计时: 2");
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(1000);

        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)"请放上任意砝码"),
                        u8g2.getMaxCharHeight() + i_margin_top,
                        (char *)"请放上任意砝码");
          u8g2.drawUTF8(AC((char *)"倒计时: 1"), LCDHeight - i_margin_bottom,
                        (char *)"倒计时: 1");
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(1000);
        scale.update();
        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)"正在读取砝码"), AM() - 5,
                        (char *)"正在读取砝码");
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(1000);
        i_button_cal_status++;
      }
    }
    if (i_button_cal_status == 4) {
      float known_mass = 0;
      if (scale.update())
        newDataReady = true;
      if (newDataReady) {
        float current_weight = scale.getData();
        Serial.println(current_weight);

        char buffer[50];
        snprintf(buffer, sizeof(buffer), "检测到 %.0fg 砝码", known_mass);

        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)trim(buffer)),
                        u8g2.getMaxCharHeight() + i_margin_top,
                        (char *)trim(buffer));
          u8g2.drawUTF8(AC((char *)"倒计时: 3"), LCDHeight - i_margin_bottom,
                        (char *)"倒计时: 3");
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(1000);

        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)trim(buffer)),
                        u8g2.getMaxCharHeight() + i_margin_top,
                        (char *)trim(buffer));
          u8g2.drawUTF8(AC((char *)"倒计时: 2"), LCDHeight - i_margin_bottom,
                        (char *)"倒计时: 2");
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(1000);

        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)trim(buffer)),
                        u8g2.getMaxCharHeight() + i_margin_top,
                        (char *)trim(buffer));
          u8g2.drawUTF8(AC((char *)"倒计时: 1"), LCDHeight - i_margin_bottom,
                        (char *)"倒计时: 1");
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(1000);

        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)"正在校准"), AM() - 5, (char *)"正在校准");
        } while (u8g2.nextPage());
#ifdef BUZZER
        buzzer.off();
#endif
        delay(1000);

        scale.setSamplesInUse(16);
        scale.refreshDataSet();  // refresh the dataset to be sure that the known
                                 // mass is measured correct
        f_calibration_value = scale.getNewCalibration(
          known_mass);  // get the new calibration value
        Serial.print(F("New calibration value f: "));
        Serial.println(f_calibration_value);
        EEPROM.put(i_addr_calibration_value, f_calibration_value);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
        EEPROM.commit();
#endif
        dtostrf(f_calibration_value, 10, 2, c_calval);
        Serial.print(F("New calibration value c: "));
        Serial.println(trim(c_calval));

        u8g2.firstPage();
        u8g2.setFont(FONT_S);
        do {
          u8g2.drawUTF8(AC((char *)"校准已完成"), AM() - 5,
                        (char *)"校准已完成");
          // u8g2.drawUTF8(AC((char *)trim(c_calval)), LCDHeight -
          // i_margin_bottom, (char *)trim(c_calval));
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
        b_calibration = false;
      }
    }
    scale.setSamplesInUse(1);
  }
}

void wifiUpdate() {
  u8g2.setFont(FONT_M);
  u8g2.firstPage();
  do {
    u8g2.drawUTF8(AC((char *)"WiFi升级"), AM() - 5, (char *)"WiFi升级");
  } while (u8g2.nextPage());
#ifdef BUZZER
  buzzer.off();
#endif
  delay(1000);
  wifiOta();
  b_menu = false;
}

void showAbout() {
  actionMessage = LINE1;
  t_actionMessage = millis();
}

void enableDebug() {
  u8g2.setFont(FONT_M);
  u8g2.firstPage();
  do {
    u8g2.drawUTF8(AC((char *)"Debug信息"), AM() - 5, (char *)"Debug信息");
  } while (u8g2.nextPage());
#ifdef BUZZER
  buzzer.off();
#endif
  delay(1000);
  b_debug = true;
  b_menu = false;
  // Optionally reset or perform an exit action
}

void autoSleepOn() {
  b_autoSleep = true;
  actionMessage = "已启用自动关机";
  t_actionMessage = millis();
  t_actionMessageDelay = 1000;
  EEPROM.put(i_addr_autoSleep, b_autoSleep);
  EEPROM.commit();
  Serial.println("Autosleep on stored in EEPROM.");
}

void autoSleepOff() {
  b_autoSleep = false;
  actionMessage = "已禁用自动关机";
  t_actionMessage = millis();
  t_actionMessageDelay = 1000;
  EEPROM.put(i_addr_autoSleep, b_autoSleep);
  EEPROM.commit();
  Serial.println("Autosleep off stored in EEPROM.");
}

void quickBootOn() {
  b_quickBoot = true;
  actionMessage = "已设为短按开机";
  t_actionMessage = millis();
  t_actionMessageDelay = 1000;
  EEPROM.put(i_addr_quickBoot, b_quickBoot);
  EEPROM.commit();
  Serial.println("Quick boot on stored in EEPROM.");
}

void quickBootOff() {
  b_quickBoot = false;
  actionMessage = "已设为长按开机";
  t_actionMessage = millis();
  t_actionMessageDelay = 1000;
  EEPROM.put(i_addr_quickBoot, b_quickBoot);
  EEPROM.commit();
  Serial.println("Quick boot off stored in EEPROM.");
}

void driftCompOff() {
  f_maxDriftCompensation = 0.0;
  actionMessage = "已关闭漂移补偿";
  t_actionMessage = millis();
  t_actionMessageDelay = 1000;
  EEPROM.put(i_addr_driftCompensation, f_maxDriftCompensation);
  EEPROM.commit();
  Serial.println("Drift Comp Off stored in EEPROM.");
}

void driftComp0050() {
  f_maxDriftCompensation = 0.05;
  actionMessage = "漂移补偿 0.05g";
  t_actionMessage = millis();
  t_actionMessageDelay = 1000;
  EEPROM.put(i_addr_driftCompensation, f_maxDriftCompensation);
  EEPROM.commit();
  Serial.println("Drift Comp 0.05g stored in EEPROM.");
}

void driftComp0075() {
  f_maxDriftCompensation = 0.075;
  actionMessage = "漂移补偿 0.075g";
  t_actionMessage = millis();
  t_actionMessageDelay = 1000;
  EEPROM.put(i_addr_driftCompensation, f_maxDriftCompensation);
  EEPROM.commit();
  Serial.println("Drift Comp 0.075g stored in EEPROM.");
}

void driftComp0100() {
  f_maxDriftCompensation = 0.1;
  actionMessage = "漂移补偿 0.1g";
  t_actionMessage = millis();
  t_actionMessageDelay = 1000;
  EEPROM.put(i_addr_driftCompensation, f_maxDriftCompensation);
  EEPROM.commit();
  Serial.println("Drift Comp 0.1g stored in EEPROM.");
}

void driftComp0200() {
  f_maxDriftCompensation = 0.2;
  actionMessage = "漂移补偿 0.2g";
  t_actionMessage = millis();
  t_actionMessageDelay = 1000;
  EEPROM.put(i_addr_driftCompensation, f_maxDriftCompensation);
  EEPROM.commit();
  Serial.println("Drift Comp 0.2g stored in EEPROM.");
}

// void calibrateVoltage() {
//   actionMessage = "Calibrate 4.2v";
//   t_actionMessage = millis();
//   int adcValue = analogRead(BATTERY_PIN);                                   // Read the ADC value from the battery pin
//   float voltageAtPin = (adcValue / adcResolution) * referenceVoltage;       // Calculate the voltage at the ADC pin
//   float batteryVoltage = voltageAtPin * dividerRatio;                       // Calculate the actual battery voltage using the voltage divider ratio
//   f_batteryCalibrationFactor = 4.2 / batteryVoltage;                        // Calculate the calibration factor from user input
//   EEPROM.put(i_addr_batteryCalibrationFactor, f_batteryCalibrationFactor);  // Store the calibration factor in EEPROM

// #if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
//   EEPROM.commit();  // Commit changes to EEPROM to save the calibration factor
// #endif
//   Serial.print("Battery Voltage Factor set to: ");  // Output the new calibration factor to the Serial Monitor
//   Serial.println(f_batteryCalibrationFactor);
// }

void calibrateVoltage() {
  actionMessage = "校准电压4.2v";
  t_actionMessage = millis();

  const int numReadings = 50;  // Number of readings to average
  long adcSum = 0;

  // Take multiple ADC readings and calculate their sum
  for (int i = 0; i < numReadings; i++) {
    adcSum += analogRead(BATTERY_PIN);
    delay(10);  // Optional: Add a small delay between readings for stability
  }

  // Calculate the average ADC value
  float adcValue = adcSum / (float)numReadings;

  // Calculate the voltage at the pin and the actual battery voltage
  float voltageAtPin = (adcValue / adcResolution) * referenceVoltage;
  float batteryVoltage = voltageAtPin * dividerRatio;

  // Calculate the calibration factor
  f_batteryCalibrationFactor = 4.2 / batteryVoltage;

  // Store the calibration factor in EEPROM
  EEPROM.put(i_addr_batteryCalibrationFactor, f_batteryCalibrationFactor);

#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
  EEPROM.commit();  // Commit changes to EEPROM
#endif

  // Output the new calibration factor to the Serial Monitor
  Serial.print("Battery Voltage Factor set to: ");
  Serial.println(f_batteryCalibrationFactor);
}

// Navigate menu
void navigateMenu(int direction) {
  currentIndex = (currentIndex + direction + currentMenuSize) % currentMenuSize;
  currentSelection = currentMenu[currentIndex];
  Serial.print("currentIndex ");
  Serial.println(currentIndex);
  //showMenu();
}

// Select menu
// 5/5 count submenu items
void selectMenu() {
  //use the static way to avoid get size of dynamic array.
  if (currentSelection->subMenu) {
    // Enter the submenu
#ifdef BUZZER

    if (currentSelection == &menuBuzzer) {
      currentMenu = buzzerMenu;
      currentMenuSize = getArraySize(buzzerMenu);
    } else
#endif
      if (currentSelection == &menuContainer) {
      currentMenu = containerMenu;
      currentMenuSize = getArraySize(containerMenu);
    } else if (currentSelection == &menuCalibration) {
      currentMenu = calibrationMenu;
      currentMenuSize = getArraySize(calibrationMenu);
    } else if (currentSelection == &menuWiFiUpdate) {
      currentMenu = wifiUpdateMenu;
      currentMenuSize = getArraySize(wifiUpdateMenu);
    } else if (currentSelection == &menuFactory) {
      currentMenu = factoryMenu;
      currentMenuSize = getArraySize(factoryMenu);
    } else if (currentSelection == &menuAutoSleep) {
      currentMenu = autoSleepMenu;
      currentMenuSize = getArraySize(autoSleepMenu);
    } else if (currentSelection == &menuQuickBoot) {
      currentMenu = quickBootMenu;
      currentMenuSize = getArraySize(quickBootMenu);
    } else if (currentSelection == &menuDriftComp) {
      currentMenu = driftCompMenu;
      currentMenuSize = getArraySize(driftCompMenu);
    }
    currentIndex = 0;
    currentSelection = currentMenu[currentIndex];
    //showMenu();
  } else if (currentSelection->action) {
    // Execute the action if available
    currentSelection->action();
  } else if (currentSelection->parentMenu) {
    // Go back to the parent menu
    currentMenu = mainMenu;
    currentMenuSize = getArraySize(mainMenu);
    currentIndex = 0;  // Reset to the first item in the parent menu
    currentSelection = currentMenu[currentIndex];
    //showMenu();
  }
}

// Display menu
void showMenu() {
#ifdef ROTATION_180
  u8g2.setDisplayRotation(U8G2_R2);
#else
  u8g2.setDisplayRotation(U8G2_R0);
#endif

  u8g2.setFont(FONT_XS);
  u8g2.firstPage();
  do {
    if (millis() - t_actionMessage < t_actionMessageDelay) {
      u8g2.setFont(FONT_M);
      if (AC(actionMessage.c_str()) < 0)
        u8g2.setFont(FONT_S);
      u8g2.drawUTF8(AC(actionMessage.c_str()), AM() - 5, actionMessage.c_str());
    } else {
      u8g2.setFont(FONT_XS);
      currentPage = currentIndex / linesPerPage + 1;
      //currentMenuSize = getArraySize(currentMenu);
      totalPages = (currentMenuSize + linesPerPage - 1) / linesPerPage;  //Calculate total pages
      char pageInfo[10];
      snprintf(pageInfo, sizeof(pageInfo), "%d/%d", currentPage, totalPages);
      if (totalPages > 1)
        u8g2.drawUTF8(AR(pageInfo), u8g2.getMaxCharHeight(), pageInfo);  // Show on top-right of the screen if more than one page is needed.
      for (int i = 0; i < currentMenuSize; i++) {
        if (currentMenu[i] == currentSelection) {
          u8g2.drawUTF8(0, (u8g2.getMaxCharHeight() + 2) * (i % linesPerPage + 1) - 2, ">");  // Highlight current selection
        }
        if (i >= (currentPage - 1) * linesPerPage && i < currentPage * linesPerPage)
          u8g2.drawUTF8(10, (u8g2.getMaxCharHeight() + 2) * (i % linesPerPage + 1) - 2, currentMenu[i]->name);
      }
    }
  } while (u8g2.nextPage());
}


#endif