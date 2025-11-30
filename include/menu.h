#ifndef MENU_H
#define MENU_H

String actionMessage = "Default";
long t_actionMessage = 0;
template<typename T>
int getMenuSize(T &menu) {
  return sizeof(menu) / sizeof(menu[0]);
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

// Main menu
// 3/5 write all the 1st menu to mainMenu
Menu *mainMenu[] = { &menuExit,
#ifdef BUZZER
                     &menuBuzzer,
#endif
                     &menuContainer, &menuCalibration, &menuWiFiUpdate, &menuAbout, &menuFactory };
//  &menuHolder1, &menuHolder2, &menuHolder3, &menuHolder4,
//  &menuHolder5, &menuHolder6};
Menu **currentMenu = mainMenu;
Menu *currentSelection = mainMenu[0];
int currentMenuSize = getMenuSize(mainMenu);  // Top-level menu size
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
}

// Menu actions
void exitMenu() {
  u8g2.setFont(FONT_M);
  u8g2.firstPage();
  do {
    u8g2.drawUTF8(AC((char *)"退出设置"), AM(), (char *)"退出设置");
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
    int i_margin_top = 8;
    int i_margin_bottom = 8;
    char *c_calval = (char *)"";
    if (i_button_cal_status == 1) {
      scale.setSamplesInUse(16);
      Serial.println(F("***"));
      Serial.println(F("Start calibration:"));
      Serial.println(F("Select weight."));
      u8g2.firstPage();
      do {
#ifdef ROTATION_180
        u8g2.setDisplayRotation(U8G2_R2);
#else
        u8g2.setDisplayRotation(U8G2_R0);
#endif
        u8g2.setFontMode(1);
        u8g2.setDrawColor(1);
        u8g2.setFont(FONT_S);
        int x, y;
        x = 0;
        y = u8g2.getMaxCharHeight();
        u8g2.drawUTF8(x, y, "校准砝码重量");
        u8g2.setFont(FONT_M);
        x += 5;
        y += u8g2.getMaxCharHeight();
        u8g2.drawUTF8(x, y, weights[0]);
        x = 64;
        u8g2.drawUTF8(x, y, weights[2]);
        x = 5;
        y += u8g2.getMaxCharHeight();
        u8g2.drawUTF8(x, y, weights[1]);
        x = 64;
        u8g2.drawUTF8(x, y, weights[3]);
        if (i_cal_weight == 0 || i_cal_weight == 2)
          y = y - u8g2.getMaxCharHeight();
        if (i_cal_weight == 0 || i_cal_weight == 1)
          x = 0;
        else
          x = 64 - 5;
        int x0 = x;
        int x1 = x;
        int x2 = x0 + 4;
        int y0 = y - u8g2.getMaxCharHeight() + 6;
        int y1 = y;
        int y2 = y - (y1 - y0) / 2;
        u8g2.drawTriangle(x0, y0, x1, y1, x2, y2);

        u8g2.setDrawColor(2);
        drawButtonBox();  //cal
        //drawBleBox();
      } while (u8g2.nextPage());
    }
    if (i_button_cal_status == 2) {
      scale.update();

      u8g2.firstPage();
      u8g2.setFont(FONT_S);
      do {
        //2行
        //FONT_M = u8g2_font_fub14_tn;
        u8g2.drawUTF8(AC((char *)"请勿触碰"), u8g2.getMaxCharHeight() + i_margin_top, (char *)"请勿触碰");
        u8g2.drawUTF8(AC((char *)"3秒后清零"), LCDHeight - i_margin_bottom, (char *)"3秒后清零");
      } while (u8g2.nextPage());

#ifdef BUZZER
      buzzer.off();
#endif
      delay(1000);
      u8g2.firstPage();
      u8g2.setFont(FONT_S);
      do {
        //2行
        //FONT_M = u8g2_font_fub14_tn;
        u8g2.drawUTF8(AC((char *)"请勿触碰"), u8g2.getMaxCharHeight() + i_margin_top, (char *)"请勿触碰");
        u8g2.drawUTF8(AC((char *)"2秒后清零"), LCDHeight - i_margin_bottom, (char *)"2秒后清零");
      } while (u8g2.nextPage());
#ifdef BUZZER
      buzzer.off();
#endif
      delay(1000);

      u8g2.firstPage();
      u8g2.setFont(FONT_S);
      do {
        //2行
        //FONT_M = u8g2_font_fub14_tn;
        u8g2.drawUTF8(AC((char *)"请勿触碰"), u8g2.getMaxCharHeight() + i_margin_top, (char *)"请勿触碰");
        u8g2.drawUTF8(AC((char *)"1秒后清零"), LCDHeight - i_margin_bottom, (char *)"1秒后清零");
      } while (u8g2.nextPage());

#ifdef BUZZER
      buzzer.off();
#endif
      delay(1000);

      u8g2.firstPage();
      u8g2.setFont(FONT_S);
      do {
        //2行
        //FONT_M = u8g2_font_fub14_tn;
        u8g2.drawUTF8(AC((char *)"正在清零"), AM(), (char *)"正在清零");
      } while (u8g2.nextPage());

      scale.tare();
      Serial.println(F("Tare done"));
      u8g2.firstPage();
      u8g2.setFont(FONT_S);
      do {
        //2行
        //FONT_M = u8g2_font_fub14_tn;
        u8g2.drawUTF8(AC((char *)"清零完成"), AM(), (char *)"清零完成");
      } while (u8g2.nextPage());

      //buzzer.beep(2, 50);

#ifdef BUZZER
      buzzer.off();
#endif
      delay(1000);
      i_button_cal_status++;
    }

    if (i_button_cal_status == 3) {
      float known_mass = 0;
      scale.update();
      known_mass = weight_values[i_cal_weight];
      char buffer[50];
      snprintf(buffer, sizeof(buffer), "放置%s砝码", weights[i_cal_weight]);

      u8g2.firstPage();
      u8g2.setFont(FONT_S);
      do {
        //2行
        //FONT_M = u8g2_font_fub14_tn;
        u8g2.drawUTF8(AC((char *)trim(buffer)), u8g2.getMaxCharHeight() + i_margin_top, (char *)trim(buffer));
        u8g2.drawUTF8(AC((char *)"3秒后开始校准"), LCDHeight - i_margin_bottom, (char *)"3秒后开始校准");
      } while (u8g2.nextPage());
#ifdef BUZZER
      buzzer.off();
#endif
      delay(1000);

      u8g2.firstPage();
      u8g2.setFont(FONT_S);
      do {
        //2行
        //FONT_M = u8g2_font_fub14_tn;
        u8g2.drawUTF8(AC((char *)trim(buffer)), u8g2.getMaxCharHeight() + i_margin_top, (char *)trim(buffer));
        u8g2.drawUTF8(AC((char *)"2秒后开始校准"), LCDHeight - i_margin_bottom, (char *)"2秒后开始校准");
      } while (u8g2.nextPage());
#ifdef BUZZER
      buzzer.off();
#endif
      delay(1000);

      u8g2.firstPage();
      u8g2.setFont(FONT_S);
      do {
        //2行
        //FONT_M = u8g2_font_fub14_tn;
        u8g2.drawUTF8(AC((char *)trim(buffer)), u8g2.getMaxCharHeight() + i_margin_top, (char *)trim(buffer));
        u8g2.drawUTF8(AC((char *)"1秒后开始校准"), LCDHeight - i_margin_bottom, (char *)"1秒后开始校准");
      } while (u8g2.nextPage());
#ifdef BUZZER
      buzzer.off();
#endif
      delay(1000);

      u8g2.firstPage();
      u8g2.setFont(FONT_S);
      do {
        //2行
        //FONT_M = u8g2_font_fub14_tn;
        u8g2.drawUTF8(AC((char *)"请勿触碰"), u8g2.getMaxCharHeight() + i_margin_top, (char *)"请勿触碰");
        u8g2.drawUTF8(AC((char *)"正在校准"), LCDHeight - i_margin_bottom, (char *)"正在校准");
      } while (u8g2.nextPage());
#ifdef BUZZER
      buzzer.off();
#endif
      delay(1000);

      scale.refreshDataSet();                                     //refresh the dataset to be sure that the known mass is measured correct
      f_calibration_value = scale.getNewCalibration(known_mass);  //get the new calibration value
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
        //2行
        //FONT_M = u8g2_font_fub14_tn;
        u8g2.drawUTF8(AC((char *)"校准已完成"), u8g2.getMaxCharHeight() + i_margin_top, (char *)"校准已完成");
        u8g2.drawUTF8(AC((char *)trim(c_calval)), LCDHeight - i_margin_bottom, (char *)trim(c_calval));
      } while (u8g2.nextPage());
#ifdef BUZZER
      buzzer.off();
#endif
      delay(1000);

      //buzzer.beep(2, 50);
#ifdef BUZZER
      buzzer.off();
#endif
      delay(1000);
      b_calibration = false;
    }
    scale.setSamplesInUse(1);
  }
}

void wifiUpdate() {
  u8g2.setFont(FONT_M);
  u8g2.firstPage();
  do {
    u8g2.drawUTF8(AC((char *)"WiFi升级"), AM(), (char *)"WiFi升级");
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
    u8g2.drawUTF8(AC((char *)"Debug信息"), AM(), (char *)"Debug信息");
  } while (u8g2.nextPage());
#ifdef BUZZER
  buzzer.off();
#endif
  delay(1000);
  b_debug = true;
  b_menu = false;
  // Optionally reset or perform an exit action
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
      currentMenuSize = getMenuSize(buzzerMenu);
    } else
#endif
      if (currentSelection == &menuContainer) {
      currentMenu = containerMenu;
      currentMenuSize = getMenuSize(containerMenu);
    } else if (currentSelection == &menuCalibration) {
      currentMenu = calibrationMenu;
      currentMenuSize = getMenuSize(calibrationMenu);
    } else if (currentSelection == &menuWiFiUpdate) {
      currentMenu = wifiUpdateMenu;
      currentMenuSize = getMenuSize(wifiUpdateMenu);
    } else if (currentSelection == &menuFactory) {
      currentMenu = factoryMenu;
      currentMenuSize = getMenuSize(factoryMenu);
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
    currentMenuSize = getMenuSize(mainMenu);
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
    if (millis() - t_actionMessage < 1000) {
      u8g2.setFont(FONT_M);
      if (AC(actionMessage.c_str()) < 0)
        u8g2.setFont(FONT_S);
      u8g2.drawUTF8(AC(actionMessage.c_str()), AM(), actionMessage.c_str());
    } else {
      u8g2.setFont(FONT_XS);
      currentPage = currentIndex / linesPerPage + 1;
      //currentMenuSize = getMenuSize(currentMenu);
      totalPages = (currentMenuSize + linesPerPage - 1) / linesPerPage;  //Calculate total pages
      char pageInfo[10];
      snprintf(pageInfo, sizeof(pageInfo), "%d/%d", currentPage, totalPages);
      if (totalPages > 1)
        u8g2.drawUTF8(AR(pageInfo), u8g2.getMaxCharHeight(), pageInfo);  // Show on top-right of the screen if more than one page is needed.
      for (int i = 0; i < currentMenuSize; i++) {
        if (currentMenu[i] == currentSelection) {
          u8g2.drawUTF8(0, u8g2.getMaxCharHeight() * (i % linesPerPage + 1), ">");  // Highlight current selection
        }
        if (i >= (currentPage - 1) * linesPerPage && i < currentPage * linesPerPage)
          u8g2.drawUTF8(10, u8g2.getMaxCharHeight() * (i % linesPerPage + 1), currentMenu[i]->name);
      }
    }
  } while (u8g2.nextPage());
}


#endif