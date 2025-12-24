/*
  2021-07-25 新增开机时校准，双击咖啡录入关机
  2021-07-26 新增开机按住清零校准，取消双击录入关机
  2021-08-02 新增开机修改sample，显示版本信息，手柄录入功能
  2021-08-07 v1.1 新增手柄录入功能
  2021-08-15 v1.1 去掉手柄录入（因为双头手柄含水量不一定），修复进入意式模式时未恢复参数，新增电量检测
  2021-09-01 v1.2 重新加入手柄录入 修复sample不可更改bug
  2021-09-03 v1.3 修复切换到意式模式直接计时问题 修复录入可能产生负值问题
  2021-10-02 v1.4 修复切换到意式模式 下液计时不清零问题
  2021-10-10 v1.5 修复手柄录入产生的计时问题 新增显示旋转功能
  2022-02-03 v1.6 二按钮模式
  2022-03-04 v1.7 流速计
  2022-04-12 v1.8 优化电量显示为0时闪屏
  2022-08-01 v1.9 尝试支持3.3v芯片
  2022-08-06 v2.0 换用rp2040，支持无操作自动关机
  2022-09-11 v2.1 支持双模式，支持六轴传感器侧放关机
  2022-11-02 v2.2 使用ESP32 wemos lite，支持esp32 休眠，支持esp32电容按钮开关机，去掉5v充放一体单元，去掉了电量显示
  2023-02-11 v2.3 倾斜时不开机，避免误触
    bug fix:  setsamplesinuse只能缩小原来config.h中的SAMPLES，不能增加。
  2023-03-06 v2.4 大幅改进显示稳定性
  2023-03-11 v3.0 WiFi OTA，更换字体大小
  2023-06-24 v3.1 去掉WiFi功能，加入蓝牙串口，可自定义所有参数。
  2023-12-11 v3.2 ESPNow无线传输参数显示
  2023-12-23 v3.3 ESPNow左键开启，和蓝牙一样，加入ssd1312
  2024-01-27 v3.4 加入静音后代替蜂鸣器的LED
  2024-03-25 v3.5 Add early verion of English translation
  2024-04-06 v4.0 Add BLE and uuid.
  2024-04-28 v4.1 fix - wrong button oled indicator on rotated display
                  fix - "disabled weight in serial" works but "auto tare on espresso mode" doesn't
                  fix - when ble app connected, scale can't power off via both button down
  2024-05-13 v4.2 fix - wrong map() usage. map only use long for input, not float.
  2024-06-03 v4.3 fix - put back serial command.
                  fix - wrong screen rotation when powering off at extration mode.
  2024-07-25 v4.4 fix - wrong removal of FIVE_BUTTON
  2024-07-25 v5.0 add - ADS1232 support
                  add - #ifdef SMOOTH_ADC 
  2024-09-04 v5.1 New calibraion(100g,200g,500g,1000g) via BLE
  2024-11-03 v5.2 add - charging UI.
                  add - now deepsleep in esp_sleep_enable_ext1_wakeup_io mode in tead of esp_sleep_enable_ext0_wakeup to provide gpio wakeup identification.
                  add - left button power with ble, right button power without ble.
                  add - touch button more than 1s to power on.
  2024-11-15 v5.3 add - New extration UI
  2024-12-13 v5.4 fix - None block tare
                  add - Menu system
  2024-12-19 v5.5 fix - Removed delay() for bettery double-click
                  fix - None block buzzer
  2025-08-21 v5.6 add - v8.1 PCB support
  2025-08-21 v5.7 fix - charging ui
                  fix - battery reading frequncy
  2025-12-24 v6.1 add - drift compensation
                  add - menu option of autosleep, quickboot, drift compensation
                  fix - tare triggered by buttonTare_Released()
                  fix - removed button suppress
                  fix - battery refresh too much cause t_battery was wrong
                  fix - universal template for getArrarySize


  todo
  开机M进入菜单
  //2023-02-11 关闭蜂鸣器 DONE(2023-06-25)
  2023-03-06 使用enum菜单
  2024-03-23 Impliment ble function using service uuid and charactoristic uuid to let other apps/devices to get the scale data, or have it tared.(done)
  2024-05-05 Fix when in extration mode, screen rotated, back to pure scale, xor button indicating wrong.(done)
  2024-10-15
*/

//include

#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "parameter.h"
#include "power.h"
#include "gyro.h"
#include "display.h"
#include "declare.h"
#include "wifi_ota.h"
#include "espnow.h"
#include "menu.h"

//unsupported by later esp32s3
// #include <BluetoothSerial.h>
// BluetoothSerial SerialBT;

// #ifndef BT
// #ifdef DEBUG_BT
// #include <BluetoothSerial.h>
// BluetoothSerial SerialBT;
// #endif
// #endif

//functions 函数

// Reads a boolean value from EEPROM with validation.
// If the stored value is not 0 or 1 (i.e., invalid or uninitialized data),
// it will be replaced with the provided default value.
bool readBoolEEPROMWithValidation(int addr, bool defaultVal) {
  uint8_t val;
  EEPROM.get(addr, val);  // Read raw byte from EEPROM
  if (val == 0 || val == 1) {
    // Valid boolean value found
    return val;
  }
  // Invalid value, overwrite with default
  EEPROM.put(addr, (uint8_t)defaultVal);
  EEPROM.commit();
  return defaultVal;
}

//ble
// Function to calculate XOR for validation (assuming this might still be needed)
uint8_t calculateXOR(uint8_t *data, size_t len) {
  uint8_t xorValue = 0x03;                // Starting value for XOR as per your example
  for (size_t i = 1; i < len - 1; i++) {  // Start from 1 to len - 1 assuming last byte is XOR value
    xorValue ^= data[i];
  }
  return xorValue;
}

// Encode weight into two bytes, big endian
void encodeWeight(float weight, byte &byte1, byte &byte2) {
  int weightInt = (int)(weight * 10);  // Convert to grams * 10
  byte1 = (byte)((weightInt >> 8) & 0xFF);
  byte2 = (byte)(weightInt & 0xFF);
}

// This callback will be invoked when a device connects or disconnects
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  }

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected, restarting advertising...");
    // 断开后重新开始广告
    pServer->startAdvertising();
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  uint8_t calculateChecksum(uint8_t *data, size_t len) {
    uint8_t xorSum = 0;
    // Iterate over each byte in the data, excluding the last one assumed to be the checksum
    for (size_t i = 0; i < len - 1; i++) {
      xorSum ^= data[i];
    }
    return xorSum;
  }

  // Validate the checksum of the data
  bool validateChecksum(uint8_t *data, size_t len) {
    if (len < 2) {  // Need at least 1 byte of data and 1 byte of checksum
      return false;
    }
    uint8_t expectedChecksum = data[len - 1];
    uint8_t calculatedChecksum = calculateChecksum(data, len);
    return expectedChecksum == calculatedChecksum;
  }
  /*  
        Weight received on	FFF4 (0000FFF4-0000-1000-8000-00805F9B34FB)

        Firmware v1.0 and v1.1 sends weight as a 7 byte message:
        03CE 0000 0000 CD = 0.0 grams
        03CE 0065 0000 A8 = 10.1 grams
        03CE 0794 0000 5E = 194.0 grams
        03CE 1B93 0000 5E = 705.9 grams
        03CE 2BAC 0000 4A = 1118.0 grams

        Firmware v1.2 and newer sends weight with a timestamp as a 10 byte message:
        03CE 0000 010203 0000 CD = 0.0 grams - (1 minute, 2 seconds, 3 milliseconds)
        03CE 0065 010204 0000 A8 = 10.1 grams - (1 minute, 2 seconds, 4 milliseconds)
        03CE 0794 010205 0000 5E = 194.0 grams - (1 minute, 2 seconds, 5 milliseconds)
        03CE 1B93 010206 0000 5E = 705.9 grams - (1 minute, 2 seconds, 6 milliseconds)
        03CE 2BAC 010207 0000 4A = 1118.0 grams - (1 minute, 2 seconds, 7 milliseconds)

        030A LED and Power
        LED on [requires v1.1 firmware]
        030A 0101 000009 (grams)
        030A 0101 010008 (ounces) 
        LED off	
        030A 0000 000009
        Power off (new in v1.2 firmware)
        030A 0200 00000B

        030B Timer
        Timer start	
        030B 0300 00000B
        Timer stop	
        030B 0000 000008
        Timer zero	
        030B 0200 00000A

        030F Tare (set weight to zero)	
        030F 0000 00000C
        030F B900 0000B5
        
        sofronio edit 2024-08-31
        031A Calibration
        031B WIFIOTA
        
*/

  void onWrite(BLECharacteristic *pWriteCharacteristic) {
    Serial.print("Timer");
    Serial.print(millis());
    Serial.print(" onWrite counter:");
    Serial.println(i_onWrite_counter++);
    if (pWriteCharacteristic != nullptr) {                         // Check if the characteristic is valid
      size_t len = pWriteCharacteristic->getLength();              // Get the data length
      uint8_t *data = (uint8_t *)pWriteCharacteristic->getData();  // Get the data pointer

      // Optionally print the received HEX for verification or debugging
      Serial.print("Received HEX: ");
      for (size_t i = 0; i < len; i++) {
        if (data[i] < 0x10) {  // Check if the byte is less than 0x10
          Serial.print("0");   // Print a leading zero
        }
        Serial.print(data[i], HEX);  // Print the byte in HEX
      }
      Serial.println();  // New line for readability
      if (data[0] == 0x03) {
        if (data[1] == 0x0F) {
          if (validateChecksum(data, len)) {
            Serial.println("Valid checksum for tare operation. Taring");
          } else {
            Serial.println("Invalid checksum for tare operation.");
          }
          scale.tareNoDelay();
        } else if (data[1] == 0x0A) {
          if (data[2] == 0x00) {
            Serial.println("LED off detected.");
          } else if (data[2] == 0x01) {
            Serial.println("LED on detected.");
          } else if (data[2] == 0x02) {
            Serial.println("Power off detected.");
            shut_down_now_nobeep();
          }
        } else if (data[1] == 0x0B) {
          if (data[2] == 0x03) {
            Serial.println("Timer start detected.");
            stopWatch.start();
          } else if (data[2] == 0x00) {
            Serial.println("Timer stop detected.");
            stopWatch.stop();
          } else if (data[2] == 0x02) {
            Serial.println("Timer zero detected.");
            stopWatch.reset();
          }
        } else if (data[1] == 0x1A) {
          Serial.println("Calibration via BLE");
          i_button_cal_status = 1;
          b_calibration = true;
        }
#ifdef WIFI
        else if (data[1] == 0x1B) {
          b_ota = true;
        }
#endif
      }
    }
  }
};

//buttons

void aceButtonHandleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState) {
  power_off(-1);  //reset power off timer
  if (b_u8g2Sleep) {
    u8g2.setPowerSave(0);  //wake up oled when button is pressed
    b_u8g2Sleep = false;
  }
  u8g2.setContrast(255);  //set oled brightness to max when button is pressed  int pin = button->getPin();
  int pin = button->getPin();
  switch (eventType) {
    case AceButton::kEventPressed:
      //these will be triggered once the button is touched.
      if (GPIO_power_on_with != BATTERY_CHARGING)
#ifdef BUZZER
        buzzer.beep(1, 50);
#endif
      switch (pin) {
        case BUTTON_SET:
          buttonSet_Pressed();
          break;
        case BUTTON_TARE:
          buttonTare_Pressed();
          break;
#if defined(FOUR_BUTTON) || defined(FIVE_BUTTON)
        case BUTTON_PLUS:
          if (!deviceConnected)
            buttonPlus_Pressed();
          else
            sendBleButton(1, 0);
          Serial.println("Right button short pressed");
          break;
        case BUTTON_MINUS:
          buttonMinus_Pressed();
          break;
#endif
#ifdef FIVE_BUTTON
        case BUTTON_POWER:
          Serial.println("BUTTON_POWER Clicked");
          shut_down_now_nobeep();
          break;
#endif  //FIVE_BUTTON
      }
      break;
    case AceButton::kEventDoubleClicked:
      switch (pin) {
        case BUTTON_SET:
          buttonSet_DoubleClicked();
          break;
        case BUTTON_TARE:
          buttonTare_DoubleClicked();
          break;
      }
      break;
    case AceButton::kEventLongPressed:
      switch (pin) {
        case BUTTON_SET:
          buttonSet_LongPressed();
          break;
#if defined(FOUR_BUTTON) || defined(FIVE_BUTTON)
        case BUTTON_PLUS:
          break;
#endif
        case BUTTON_TARE:
          buttonTare_LongPressed();
          break;
      }
      break;
    case AceButton::kEventReleased:
      switch (pin) {
        case BUTTON_TARE:
          buttonTARE_Released();
          break;
      }
      break;
    case AceButton::kEventRepeatPressed:
      switch (pin) {
#if defined(FOUR_BUTTON) || defined(FIVE_BUTTON)
        case BUTTON_PLUS:
          buttonPlus_Pressed();
          break;
        case BUTTON_MINUS:
          buttonMinus_Pressed();
          break;
#endif
      }
      break;
  }
}


void buttonSet_Pressed() {
  Serial.println("Set button short pressed");
  if (b_menu) {
    navigateMenu(1);  // Navigate to next menu item
  } else {
    if (b_set_container) {
      i_setContainerWeight = 1;
    } else if (b_calibration) {
      i_cal_weight++;
      if (i_cal_weight >= getArraySize(weight_values))
        i_cal_weight = 0;
    } else if (b_extraction) {
      if (stopWatch.isRunning() == false) {
        if (stopWatch.elapsed() == 0) {
          stopWatch.start();
        }
        if (stopWatch.elapsed() > 0) {
          stopWatch.reset();
        }
      } else {
        stopWatch.stop();
      }
    } else {
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
      // }
    }
  }
}

void buttonTARE_Released() {
  //trigger tare after released to avoid to judge where the tare should be placed.
  Serial.println("Tare button released");
  if (!deviceConnected || b_btnFuncWhileConnected) {
    if (b_set_container) {
      t_tareByButton = millis();
      b_tareByButton = true;
    } else if (b_extraction) {
      if (stopWatch.isRunning()){
      }
      else {
        t_tareByButton = millis();
        b_tareByButton = true;
      }
    } else {
      //普通归零
      b_weight_quick_zero = true;
      t_tareByButton = millis();
      b_tareByButton = true;
    }
  }
  Serial.println("Tare button released");
  if (deviceConnected) {
    sendBleButton(2, 1);
  }
}

void buttonTare_Pressed() {
  if (deviceConnected)
    sendBleButton(1, 0);
  Serial.println("Tare button short pressed");
  if (b_menu) {
    selectMenu();  // Select current menu item
  } else {
    if (deviceConnected && millis() - t_shutdownFailBle < 1000)
      shut_down_now_nobeep();
    if (!b_menu && !b_calibration && !deviceConnected) {
      //scaleTimer();
    }
    if (b_set_container) {
    } else if (b_calibration) {
      i_button_cal_status++;
      Serial.print("i_button_cal_status:");
      Serial.println(i_button_cal_status);
      //return;
    } else if (b_extraction) {
      if (stopWatch.isRunning())
        //在计时中 按一下则结束计时 停止冲煮
        //press button to stop the timer, and stop the brewing.
        stopWatch.stop();
      else {
        //萃取模式归零
        //不在计时中 时间归零
        //a tare on extracion mode.
        //timer not runing, zero the timer.
        b_weight_quick_zero = true;
        if (!b_extraction)
          f_weight_dose = 0;
        stopWatch.reset();
        t_extraction_begin = 0;
        t_extraction_first_drop = 0;
        t_extraction_first_drop_num = 0;
        t_extraction_last_drop = 0;
      }
    }
  }
}

void buttonPlus_Pressed() {
  if (millis() - t_button_pressed > 500) {
    t_button_pressed = millis();
    // if (b_set_sample) {
    //   //设置采样数
    //set sample number.
    //   i_sample++;
    // }
    if (b_show_info) {
      //显示信息
      //show some product info
      b_show_info = false;
    }
    if (b_extraction) {
      //萃取模式
      //extration mode
      f_weight_dose = f_weight_dose + 0.1;
      //four button version, to plus 0.1g ground weight.
    } else {
      //纯称重模式
      //pure weighing mode
      b_minus_container_button = !b_minus_container_button;  //是否减去咖啡手柄重量
    }
  }
}

void buttonMinus_Pressed() {
  if (millis() - t_button_pressed > 1000) {
    t_button_pressed = millis();
    // if (b_set_sample) {
    //   i_sample--;
    // } else
    if (b_show_info) {
      b_show_info = false;
    } else if (b_extraction) {
      //萃取模式 按一下-0.1g
      //four button version, to minus 0.1g ground weight.
      if (f_weight_dose - 0.1 > 0)
        f_weight_dose = f_weight_dose - 0.1;
    } else {
      if (i_decimal_precision == 1)
        i_decimal_precision = 2;
      else
        i_decimal_precision = 1;
    }
  }
}


void buttonSet_DoubleClicked() {
  Serial.println("Set button double clicked");
  if (!deviceConnected && !b_menu && !b_calibration) {
    Serial.println("Going to sleep now.");
    sendBlePowerOff(1);
    shut_down_now_nobeep();
  } else {
    if (deviceConnected) {
      sendBlePowerOff(1);
      Serial.println("BLE connected, going to sleep anyway.");
      shut_down_now_nobeep();
    }
    if (b_menu)
      Serial.println("Menu operating, not going to sleep.");
    if (!b_menu)
      sendBlePowerOff(0);
  }
}

void buttonTare_DoubleClicked() {
  Serial.println("Tare button double clicked");
  if (!deviceConnected && !b_menu && !b_calibration) {
    Serial.println("Going to sleep now.");
    sendBlePowerOff(2);
    shut_down_now_nobeep();
  } else {
    if (deviceConnected) {
      t_shutdownFailBle = millis();
      sendBlePowerOff(2);
      Serial.println("BLE connected, going to sleep anyway.");
      shut_down_now_nobeep();
    }
    if (b_menu)
      Serial.println("Menu operating, not going to sleep.");
    if (!b_menu)
      sendBlePowerOff(0);
  }
}

void buttonSet_LongPressed() {
  if (!b_menu) {
    Serial.println("Set button long pressed");
#ifdef BUZZER

    buzzer.beep(1, 200);

#endif
    if (GPIO_power_on_with == BATTERY_CHARGING) {
      //change GPIO_power_on_with from BATTERY_CHARGING to enter scale loop
      GPIO_power_on_with = BUTTON_SET;
      b_ble_enabled = true;
      ble_init();
    }
    if (b_extraction) {
      //萃取模式下 长按回归普通模式
      //if in extraction mode, long press to go to pure weighing mode.
      b_extraction = !b_extraction;
    } else {
      //in pure weighing mode, switch auto minus container function
      b_minus_container = !b_minus_container;
    }
    if (deviceConnected)
      sendBleButton(0, 1);
  }
}

void buttonTare_LongPressed() {
  if (!b_menu) {
    Serial.println("Tare button long pressed");
#ifdef BUZZER
    buzzer.beep(1, 200);
#endif
    if (b_calibration) {
      reset();
    } else {
      if (b_extraction) {
        i_display_rotation = !i_display_rotation;
      } else {
        if (b_mode != 0)
          b_mode = 0;
        else
          b_mode = 1;
        EEPROM.put(i_addr_mode, b_mode);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
        EEPROM.commit();
#endif
      }
    }
    if (GPIO_power_on_with == BATTERY_CHARGING) {
      //change GPIO_power_on_with from BATTERY_CHARGING to enter scale loop
      GPIO_power_on_with = BUTTON_TARE;
    }
    if (deviceConnected)
      sendBleButton(1, 1);
    b_debug = false;
  }
}

void initExtration() {
  stopWatch.reset();
  t_extraction_begin = 0;  //开始萃取打点
  //t_ for time stamp on:
  //extration begins
  t_extraction_first_drop = 0;  //下液第一滴打点
  //first drop
  t_extraction_first_drop_num = 0;
  t_extraction_last_drop = 0;  //下液结束打点
  //last drop
  tareCounter = 0;  //不稳计数器
  //if it's stable
  t_auto_tare = 0;  //自动归零打点
  //auto tare time stamp
  t_auto_stop = 0;  //下液停止打点
  //auto stop time stamp
  t_scale_stable = 0;  //稳定状态打点
  //scale stable time stamp
  t_time_out = 0;  //超时打点
  //time to long time stamp
  t_last_weight_adc = 0;  //最后一次重量输出打点
  //last adc reading time stamp
}


void ble_init() {
  //turn on ble
  BLEDevice::init("Decent Scale");
  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // Create BLE Service
  BLEService *pService = pServer->createService(SUUID_DECENTSCALE);
  pWriteCharacteristic = pService->createCharacteristic(
    CUUID_DECENTSCALE_WRITE,
    BLECharacteristic::PROPERTY_WRITE);
  pWriteCharacteristic->setCallbacks(new MyCallbacks());
  pReadCharacteristic = pService->createCharacteristic(
    CUUID_DECENTSCALE_READ,
    BLECharacteristic::PROPERTY_READ
      | BLECharacteristic::PROPERTY_NOTIFY);
  //pReadCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client connection to notify...");
}

void button_init() {
#if defined(V0)
  pinMode(BUTTON_SET, INPUT_PULLDOWN);
#if defined(FOUR_BUTTON) || defined(FIVE_BUTTON)
  pinMode(BUTTON_PLUS, INPUT_PULLDOWN);
  pinMode(BUTTON_MINUS, INPUT_PULLDOWN);
#endif
  pinMode(BUTTON_TARE, INPUT_PULLDOWN);

#ifdef FIVE_BUTTON
  pinMode(BUTTON_POWER, INPUT_PULLDOWN);
  buttonPower.init(BUTTON_POWER, !TRIGGER_LEVEL);
#endif  //FIVE_BUTTON


#else
  pinMode(BUTTON_SET, INPUT_PULLUP);
#if defined(FOUR_BUTTON) || defined(FIVE_BUTTON)
  pinMode(BUTTON_PLUS, INPUT_PULLUP);
  pinMode(BUTTON_MINUS, INPUT_PULLUP);
#endif
  pinMode(BUTTON_TARE, INPUT_PULLUP);

#ifdef FIVE_BUTTON
  pinMode(BUTTON_POWER, INPUT_PULLUP);
  buttonPower.init(BUTTON_POWER);
#endif  //FIVE_BUTTON
#endif

  buttonSet.init(BUTTON_SET, !TRIGGER_LEVEL);
#if defined(FOUR_BUTTON) || defined(FIVE_BUTTON)
  buttonPlus.init(BUTTON_PLUS, !TRIGGER_LEVEL);
  buttonMinus.init(BUTTON_MINUS, !TRIGGER_LEVEL);
#endif
  buttonTare.init(BUTTON_TARE, !TRIGGER_LEVEL);

  config1.setEventHandler(aceButtonHandleEvent);
  config1.setFeature(ButtonConfig::kFeatureClick);
  //config1.setFeature(ButtonConfig::kFeatureSuppressAfterClick);
  config1.setFeature(ButtonConfig::kFeatureDoubleClick);
  //config1.setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
  config1.setFeature(ButtonConfig::kFeatureLongPress);
  //config1.setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
  //config1.setFeature(ButtonConfig::kFeatureRepeatPress);
  //config1.setRepeatPressInterval(10);
  config1.setDoubleClickDelay(DOUBLECLICK);
  config1.setLongPressDelay(LONGCLICK);
}


void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  button_init();
  linkSubmenus();
#ifdef V0
  pinMode(BATTERY_CHARGING, INPUT_PULLDOWN);
#else
  pinMode(BATTERY_CHARGING, INPUT_PULLUP);
#endif
#if defined(V7_4) || defined(V7_5) || defined(V8_0) || defined(V8_1)
  pinMode(USB_DET, INPUT_PULLUP);
  // either esp32 rev change or diff in SDK? We get
  // warnings in logs about incorrect pinMode
  pinMode(OLED_CS, OUTPUT);
  pinMode(OLED_DC, OUTPUT);
  pinMode(SCALE_PDWN, OUTPUT);
  pinMode(SCALE_SCLK, OUTPUT);
  pinMode(ACC_PWR_CTRL, OUTPUT);
  pinMode(PWR_CTRL, OUTPUT);
#endif
  print_wakeup_reason();
  Serial.println("GPIO_power_on_with = " + String(GPIO_power_on_with));
  if (GPIO_power_on_with == BATTERY_CHARGING)
    b_is_charging = true;
  if (GPIO_power_on_with == BUTTON_SET)
    b_ble_enabled = true;
  else
    b_ble_enabled = false;
  while (true && GPIO_power_on_with > 0) {
    if (digitalRead(GPIO_power_on_with) == TRIGGER_LEVEL) {  // Button is pressed
      if (!b_button_pressed) {
        t_power_on_button = millis();
        b_button_pressed = true;  // Mark button as pressed
      }

      if (millis() - t_power_on_button >= 1000) {
        Serial.println("Button held for 1 second. Powering on...");
        // Execute power on logic
        break;  // Exit loop to continue with other code
      }
    } else {
      // If the button is released
      if (b_button_pressed) {
        Serial.println("Button released before 1 second.");
        Serial.println("Going to sleep now.");
        shut_down_now_nobeep();
        break;  // Exit loop to enter sleep mode
      }
      b_button_pressed = false;  // Reset mark
    }
  }
#if !defined(V0)
  gpio_hold_dis((gpio_num_t)PWR_CTRL);  // Disable GPIO hold mode for the specified pin, allowing it to be controlled
  pinMode(PWR_CTRL, OUTPUT);            // Set the PWR_CTRL pin as an output pin
  digitalWrite(PWR_CTRL, HIGH);         // Set the PWR_CTRL pin to HIGH, turning on the connected device or circuit
#endif
#if defined(V7_3) || defined(V7_4) || defined(V7_5)
  gpio_hold_dis((gpio_num_t)MPU_PWR_CTRL);  // Disable GPIO hold mode for the specified pin, allowing it to be controlled
  pinMode(MPU_PWR_CTRL, OUTPUT);            // Set the PWR_CTRL pin as an output pin
  digitalWrite(MPU_PWR_CTRL, HIGH);         // Set the PWR_CTRL pin to HIGH, turning on the connected device or circuit
  Serial.println("MPU_PWR_CTRL = HIGH");
#endif

#ifdef ESP32
  Wire.begin(I2C_SDA, I2C_SCL);
#endif

#ifdef HW_SPI
  SPI.begin(OLED_SCLK, -1, OLED_SDIN, OLED_CS);
#endif
#ifdef ADS1115ADC
  ADS_init();
#endif
  delay(50);
  if (b_ble_enabled) {
    ble_init();
  }

#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040) || defined(ESP32C3)
  EEPROM.begin(512);
#endif
#ifdef DEBUG_BT
  SerialBT.begin("soso D.R.S");
#endif
  //delay(2000);
  Serial.println("Begin!");

#if defined(ACC_MPU6050) || defined(ACC_BMA400)

  ACC_init();
  Serial.println("ACC_init complete");
  if (b_gyroEnabled) {
#ifdef GYROFACEUP
    if (gyro_z() < 8) {
      Serial.print("gyro_z:");
      Serial.println(gyro_z());
      shut_down_now_accidentTouch();
    }
#endif
#ifdef GYROFACEDOWN
    if (gyro_z() > -8) {
      Serial.print("gyro_z:");
      Serial.println(gyro_z());
      shut_down_now_accidentTouch();
    }
#endif
  }
#endif
#if defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
  analogReadResolution(ADC_BIT);
#endif

#ifdef BUZZER
  pinMode(BUZZER, OUTPUT);
#endif
#if defined(FOUR_BUTTON) || defined(FIVE_BUTTON)
  // pinMode(BUTTON_KEY, INPUT_PULLUP);
  // pinMode(BUTTON_DEBUG, INPUT_PULLUP);
#endif
#ifdef CHECKBATTERY
  pinMode(BATTERY_LEVEL, INPUT);
  pinMode(USB_LEVEL, INPUT);
#endif  //CHECKBATTERY

#ifndef ESP32
#if defined(HW_SPI) || defined(SW_SPI)
  pinMode(OLED_CS, OUTPUT);
  pinMode(OLED_DC, OUTPUT);
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_CS, LOW);
  digitalWrite(OLED_DC, LOW);
  digitalWrite(OLED_RST, HIGH);
#endif  //defined(HW_SPI) || defined(SW_SPI)
#endif  //ndef ESP32
  if (GPIO_power_on_with != BATTERY_CHARGING) {
#ifdef BUZZER

    buzzer.beep(1, 100);
#endif
  }

  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setContrast(255);
  //char* welcome = "soso E.R.S"; //欢迎文字
  //refreshOLED(welcome);
  power_off(15);
#ifdef WELCOME
  EEPROM.get(i_addr_welcome, str_welcome);
  str_welcome.trim();

#ifdef ROTATION_180
  u8g2.setDisplayRotation(U8G2_R2);
#else
  u8g2.setDisplayRotation(U8G2_R0);
#endif
  // if (str_welcome.length() == 127)
  //   refreshOLED(WELCOME);
  // else
  //   refreshOLED((char *)str_welcome.c_str(), FONT_EXTRACTION);
  u8g2.firstPage();
  do {
    //u8g2.drawXBM(18, 9, 97, 46, image_logo);
    u8g2.drawXBM(0, 0, 128, 64, image_logo);
  } while (u8g2.nextPage());

  // if (GPIO_power_on_with != BATTERY_CHARGING) {
  //   delay(1000);
  // }
#endif  //WELCOME
  stopWatch.setResolution(StopWatch::SECONDS);
  stopWatch.start();
  stopWatch.reset();

  i_button_cal_status = 1;  //校准状态归1
  //calibration status goes to 1
  unsigned long stabilizingtime = 500;  //去皮时间(毫秒)，增加可以提高去皮精确度
  //taring duration. longer for better reading.
  boolean _tare = true;  //电子秤初始化去皮，如果不想去皮则设为false
  //whether the scale will tare on start.
  scale.begin();
  scale.start(stabilizingtime, _tare);

  EEPROM.get(INPUTCOFFEEPOUROVER_ADDRESS, INPUTCOFFEEPOUROVER);
  EEPROM.get(INPUTCOFFEEESPRESSO_ADDRESS, INPUTCOFFEEESPRESSO);
  EEPROM.get(i_addr_batteryCalibrationFactor, f_batteryCalibrationFactor);
  EEPROM.get(i_addr_beep, b_beep);
  Serial.print("f_batteryCalibrationFactor:");
  Serial.println(f_batteryCalibrationFactor);
  if (isnan(f_batteryCalibrationFactor)) {
    f_batteryCalibrationFactor = 1.06;
    EEPROM.put(i_addr_batteryCalibrationFactor, f_batteryCalibrationFactor);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
    EEPROM.commit();
#endif
  }
  if (isnan(INPUTCOFFEEPOUROVER)) {
    INPUTCOFFEEPOUROVER = 16.0;
    EEPROM.put(INPUTCOFFEEPOUROVER_ADDRESS, INPUTCOFFEEPOUROVER);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
    EEPROM.commit();
#endif
  }
  if (isnan(INPUTCOFFEEESPRESSO)) {
    INPUTCOFFEEESPRESSO = 18.0;
    EEPROM.put(INPUTCOFFEEESPRESSO_ADDRESS, INPUTCOFFEEESPRESSO);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
    EEPROM.commit();
#endif
  }
  if (b_beep != 0 && b_beep != 1) {
    b_beep = 1;
    EEPROM.put(i_addr_beep, b_beep);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
    EEPROM.commit();
#endif
  }
  //读取模式
  EEPROM.get(i_addr_mode, b_mode);
  if (b_mode > 1) {
    b_mode = 0;
    EEPROM.put(i_addr_mode, b_mode);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
    EEPROM.commit();
#endif
  }
  if (b_mode < 0) {
    b_mode = 0;
    EEPROM.put(i_addr_mode, b_mode);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
    EEPROM.commit();
#endif
  }

  //检查手柄重量合法性
  //check eeprom container value is valid.
  EEPROM.get(i_addr_container, f_weight_container);
  if (isnan(f_weight_container)) {
    f_weight_container = 0;  //手柄重量为0
                             //保存0值
                             //if container is 0 grams, then save it to eeprom.
    EEPROM.put(i_addr_container, f_weight_container);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
    EEPROM.commit();
#endif
  }

  //检查校准值合法性
  EEPROM.get(i_addr_calibration_value, f_calibration_value);
  if (isnan(f_calibration_value)) {
    f_calibration_value = 1000.0;
    EEPROM.put(i_addr_calibration_value, f_calibration_value);  //set to default value
    EEPROM.commit();
  } else
    scale.setCalFactor(f_calibration_value);  //设定校准值

  //检查sample值合法性
  //check sample number is valid
  // EEPROM.get(i_addr_sample, i_sample);
  // if (isnan(i_sample)) {
  //   b_set_sample = true;
  //   i_sample = 0;  //读取失败 默认值为3 对应sample为8
  //   i_sample_step = 0;
  //   setSample();
  // }

  if (digitalRead(BUTTON_SET) == TRIGGER_LEVEL && digitalRead(BUTTON_TARE) == TRIGGER_LEVEL) {

    b_menu = true;
    refreshOLED((char *)"设置选项", FONT_M);
    delay(1000);
  }


#ifdef DEBUG
  if (digitalRead(BUTTON_DEBUG) == TRIGGER_LEVEL && digitalRead(BUTTON_SET) == !TRIGGER_LEVEL)
    b_debug = true;
  if (digitalRead(BUTTON_DEBUG) == TRIGGER_LEVEL && digitalRead(BUTTON_SET) == TRIGGER_LEVEL)
    b_debug_battery = true;
#endif  //DEBUG

//重新校准
//recalibration
#ifdef CAL
  if (digitalRead(BUTTON_SET) == TRIGGER_LEVEL && digitalRead(BUTTON_TARE) == TRIGGER_LEVEL) {
    b_calibration = true;  //让按钮进入校准状态3
    i_button_cal_status = 1;
    calibration(0);
    //calibration value is not valid, go to calibration procedure.
  }
#endif

  //wifiota
  // #ifdef WIFIOTA
  //   if (digitalRead(BUTTON_SET) == TRIGGER_LEVEL) {
  //     wifiOta();
  //   }
  // #endif

#if DEBUG
  Serial.print("digitalRead(BUTTON_SET):");
  Serial.print(digitalRead(BUTTON_SET));
  Serial.print("\tdigitalRead(BUTTON_SET):");
  Serial.println(digitalRead(BUTTON_SET));
#endif

  if (digitalRead(BUTTON_SET) == TRIGGER_LEVEL) {
    //bluetooth serial init
    //SerialBT.begin("D.R.S.");
    //SerialBT.begin("Open Scale BTSerial");
//#ifdef WIFI
#ifdef ESPNOW
    WiFi.mode(WIFI_STA);
#if ESP_IDF_VERSION_MAJOR > 4
    while (!WiFi.STA.started()) {
      delay(100);
    }
#endif
    // Print MAC address
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());

    // Disconnect from WiFi
    WiFi.disconnect();
    // #endif
    // #ifdef ESPNOW
    if (esp_now_init() == ESP_OK) {
      Serial.println("ESPNow Init Success");
      esp_now_register_recv_cb(receiveCallback);
      esp_now_register_send_cb(sentCallback);
      b_espnow = true;
    } else {
      Serial.println("ESPNow Init Failed");
      // Retry InitESPNow, add a counte and then restart?
      // InitESPNow();
      // or Simply Restart
      delay(3000);
      ESP.restart();
    }
#endif
  }

//4按键模式时显示信息
#ifndef TWO_BUTTON
  if (digitalRead(BUTTON_MINUS) == TRIGGER_LEVEL) {
    b_show_info = true;
    showInfo();
    delay(2000);
  }
#endif
  Serial.print("Welcome: ");
  if (str_welcome.length() == 127) {
    Serial.print(WELCOME1);
  } else {
    Serial.print(str_welcome);
  }
  Serial.print("\t");
  Serial.print(WELCOME2);
  Serial.print("\t");
  Serial.println(WELCOME3);
  Serial.print("Info: ");
  Serial.print(LINE1);
  Serial.print("\t");
  Serial.print(LINE2);
  Serial.print("\t");
  Serial.print(LINE3);
  Serial.print("\tScale Type: ");
  if (b_mode) {
    Serial.println("ESPRESSO");
  } else {
    Serial.println("POUROVER");
  }
  Serial.print("Cal_Val: ");
  Serial.print(f_calibration_value);
  // Serial.print("\tSample[");
  // Serial.print(i_sample);
  // Serial.print("]: ");
  // Serial.print(sample[i_sample]);
  Serial.print(F("\tContainer: "));
  Serial.print(f_weight_container);
  Serial.print("g");
  Serial.print(F("\tDefaultPourOver: "));
  Serial.print(INPUTCOFFEEPOUROVER);
  Serial.print("g");
  Serial.print(F("\tDefaultEspresso: "));
  Serial.print(INPUTCOFFEEESPRESSO);
  Serial.println("g");

  Serial.println("Button:\tSET\tTare\tPower");
#if defined(FOUR_BUTTON) || defined(FIVE_BUTTON)
  Serial.println("Button:\tSET\tPlus\tMinus\tTare\tPower");
#endif
  Serial.print("Pin:");
  Serial.print("\t");
  Serial.print(BUTTON_SET);
  Serial.print("\t");
#if defined(FOUR_BUTTON) || defined(FIVE_BUTTON)
  Serial.print(BUTTON_PLUS);
  Serial.print("\t");
  Serial.print(BUTTON_MINUS);
  Serial.print("\t");
#endif
  Serial.print(BUTTON_TARE);
  Serial.print("\t");
  Serial.println(GPIO_NUM_BUTTON_POWER);
#ifdef ADS1232ADC
  Serial.println("Button:\tI2C_SDA\tI2C_SCK\tADC_DOUT\tADC_SCLK\tADC_PWDN\tBUZZER");
  Serial.print("Pin:");
  Serial.print("\t");
  Serial.print(I2C_SDA);
  Serial.print("\t");
  Serial.print(I2C_SCL);
  Serial.print("\t");
  Serial.print(SCALE_DOUT);
  Serial.print("\t");
  Serial.print(SCALE_SCLK);
  Serial.print("\t");
  Serial.print(SCALE_PDWN);
#ifdef BUZZER

  Serial.print("\t");
  Serial.println(BUZZER);
#endif
#endif
#ifdef HX711ADC
  Serial.println("Button:\tI2C_SDA\tI2C_SCK\t711SDA\t711SCK\tBUZZER");
  Serial.print("Pin:");
  Serial.print("\t");
  Serial.print(I2C_SDA);
  Serial.print("\t");
  Serial.print(I2C_SCL);
  Serial.print("\t");
  Serial.print(HX711_SDA);
  Serial.print("\t");
  Serial.print(HX711_SCL);
#ifdef BUZZER

  Serial.print("\t");
  Serial.println(BUZZER);
#endif
#endif

  scale.setCalFactor(f_calibration_value);  //设置偏移量
  //set the calibration value
  //scale.setSamplesInUse(sample[i_sample]);  //设置灵敏度
  scale.setSamplesInUse(1);  //设置灵敏度

#ifdef CHECKBATTERY
  f_up_battery = analogRead(BATTERY_LEVEL) * f_vref / (pow(2, ADC_BIT) - 1) * f_divider_factor;
  t_up_battery = millis();
#endif  //CHECKBATTERY
  scale.tareNoDelay();
  Serial.println("Setup complete...");
  // while (!b_ota)
  //   ;
  Serial.print("Woken up by GPIO: ");
  Serial.println(GPIO_reason);
  updateBattery(BATTERY_PIN);
}

void pourOverScale() {
  if (f_weight_dose < 3)
    f_weight_dose = INPUTCOFFEEPOUROVER;
  //checkBrew
  if (f_displayedValue > 1 && b_ready_to_brew == true && b_weight_quick_zero == false && (millis() - t_ready_to_brew > 3000)) {
    stopWatch.start();
#ifdef BUZZER
    buzzer.beep(1, 100);
#endif
    b_ready_to_brew = false;
  }
  static bool newDataReady = 0;
  static bool scaleStable = 0;
  if (scale.update()) newDataReady = true;
  if (newDataReady) {
    f_weight_adc = scale.getData();
    newDataReady = 0;
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

    if (millis() > t_flow_rate + 1000) {
      f_flow_rate = f_displayedValue - f_flow_rate_last_weight;
      if (f_flow_rate < 0)
        f_flow_rate = 0;
      dtostrf(f_flow_rate, 7, 1, c_flow_rate);
      f_flow_rate_last_weight = f_displayedValue;
      t_flow_rate = millis();
    }

    dtostrf(f_displayedValue, 7, 1, c_weight);
    if (b_weight_in_serial == true) {
      Serial.println(trim(c_weight));
    }
  }

  //记录咖啡粉时，将重量固定为0
  //during the coffee ground input, set the reading to 0 to prevent from misreading
  if (scale.getTareStatus()) {  //pourover
    //完成清零
    //buzzer.beep(2, 50);
    b_ready_to_brew = true;
    t_ready_to_brew = millis();
    b_weight_quick_zero = false;
    t_tareStatus = millis();
  }
  if (b_weight_quick_zero)
    f_displayedValue = 0.0;
  float ratio_temp = f_displayedValue / f_weight_dose;
  if (ratio_temp < 0)
    ratio_temp = 0.0;
  if (f_weight_dose < 0.1)
    ratio_temp = 0.0;
  dtostrf(ratio_temp, 7, i_decimal_precision, c_brew_ratio);
  dtostrf(f_displayedValue, 7, i_decimal_precision, c_weight);
#ifdef DEBUG_BT

#ifdef ENGLISH
  Serial.print("手冲模式 ");  //pourover mode
  Serial.print("原重:");      //raw weight
  Serial.print(f_weight_adc);
  Serial.print(",平滑:");  //smoothed weight
  Serial.print(f_weight_smooth);
  Serial.print(",显示:");  //displaed weight
  Serial.println(f_displayedValue);
  SerialBT.print("手冲模式 ");  //same for serialbt
  SerialBT.print("原重:");
  SerialBT.print(f_weight_adc);
  SerialBT.print(",平滑:");
  SerialBT.print(f_weight_smooth);
  SerialBT.print(",显示:");
  SerialBT.println(f_displayedValue);
#endif
#ifdef ENGLISH
  Serial.print("Pour Over mode ");  //pourover mode
  Serial.print("Raw:");             //raw weight
  Serial.print(f_weight_adc);
  Serial.print(",Smoothed:");  //smoothed weight
  Serial.print(f_weight_smooth);
  Serial.print(",Displayed:");  //displaed weight
  Serial.println(f_displayedValue);
  SerialBT.print("Pour Over mode ");  //same for serialbt
  SerialBT.print("Raw:");
  SerialBT.print(f_weight_adc);
  SerialBT.print(",Smoothed:");
  SerialBT.print(f_weight_smooth);
  SerialBT.print(",Displayed:");
  SerialBT.println(f_displayedValue);
#endif
#endif
}

void espressoScale() {
  if (f_weight_dose < 3)
    f_weight_dose = INPUTCOFFEEESPRESSO;
  static bool newDataReady = 0;
  static bool scaleStable = 0;
  if (scale.update()) newDataReady = true;
  if (newDataReady) {
    if (millis() > t_last_weight_adc + i_serial_print_interval) {
      f_weight_adc = scale.getData();
      newDataReady = 0;
      t_last_weight_adc = millis();
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
      if (millis() > t_flow_rate + 1000) {
        f_flow_rate = f_displayedValue - f_flow_rate_last_weight;
        if (f_flow_rate < 0)
          f_flow_rate = 0;
        dtostrf(f_flow_rate, 7, i_decimal_precision, c_flow_rate);
        f_flow_rate_last_weight = f_displayedValue;
        t_flow_rate = millis();
      }
      dtostrf(f_displayedValue, 7, 1, c_weight);
      if (b_weight_in_serial == true) {
        Serial.println(trim(c_weight));
      }
      if (millis() > t_scale_stable + scaleStableInterval) {
        //稳定判断
        t_scale_stable = millis();  //重量稳定打点
        //get the time stamp for a stable scale.
        if (abs(aWeight - f_weight_smooth) < aWeightDiff) {
          scaleStable = true;  //称已经稳定

#ifdef DEBUG_BT
          //scale is steady
          SerialBT.println("称已经稳定");
#endif
          aWeight = f_weight_smooth;  //稳定重量aWeight
          //smoothed data
          if (millis() > t_auto_tare + autoTareInterval) {
            if (t_extraction_begin > 0 && tareCounter > 3) {
              //t_extraction_begin>0 已经开始萃取 tareCounter>3 忽略前期tare时不稳定
              // if the stable reading number diviation is greater than 3
              if (t_extraction_last_drop == 0) {    //没有给t_extraction_last_drop计时过
                t_extraction_last_drop = millis();  //萃取完成打点
              }
              if (t_extraction_last_drop - t_extraction_first_drop < i_extraction_minimium_timer * 1000) {  //前7秒不判断停止计时 因此继续计时
                t_extraction_last_drop = 0;
              } else if (b_ready_to_brew) {
                //正常过程 最终下液到稳定时间大于5秒
                //normal brewing, the last drop to stalbe scale is greater than 5 second
                stopWatch.stop();
//萃取完成 单次固定液重
#ifdef DEBUG_BT
                Serial.println("萃取完成");
                SerialBT.println("萃取完成");
#endif
#ifdef BUZZER
                buzzer.beep(3, 50);
#endif
                b_ready_to_brew = false;
              }
            }
            if (stopWatch.elapsed() == 0) {
              //秒表没有运行
              t_auto_tare = millis();                                     //自动清零计时打点
              if (f_displayedValue > 30 && b_minus_container == false) {  //大于30g说明放了杯子 3g是纸杯
                //后面的判断避免手柄模式超过30g对放杯感应产生干扰
                scale.tare();
#ifdef BUZZER
                buzzer.beep(1, 100);
#endif
                tareCounter = 0;
                t_extraction_last_drop = 0;
                t_extraction_first_drop = 0;
                t_extraction_first_drop_num = 0;
                if (b_ready_to_brew) {
                  //已经准备冲煮状态 才开始计时
                  stopWatch.reset();
                  stopWatch.start();
                } else {
                  //没有准备好冲煮 则第一次清零不计时 并进入准备冲煮状态
                  b_ready_to_brew = true;
                }
                scaleStable = false;
                t_extraction_begin = millis();
                //Serial.println(F("正归零 开始计时 取消稳定"));
              }
              //时钟为零，负重量稳定后归零，时钟不变
              if (f_displayedValue < -0.5) {  //负重量状态
                scale.tare();
                Serial.println("tare() 负归零后 进入准备冲煮状态 【下次】放了杯子后 清零并计时");
                //负归零后 进入准备冲煮状态 【下次】放了杯子后 清零并计时
                b_ready_to_brew = true;
              }
            }
            atWeight = f_displayedValue;
          }
        } else {  //称不稳定
          scaleStable = false;
          if (t_extraction_begin > 0) {
            //过滤tare环节的不稳
            if (tareCounter <= 3)
              tareCounter++;  ///tare后 遇到前2次不稳 视为稳定
            else {
              //tareCounter > 3 //视为开始萃取
              //萃取开始 下液重量开始计算
              //w1 = f_weight_adc;
              if (t_extraction_first_drop == 0) {
                t_extraction_first_drop = millis();  //第一滴下液
              }
            }
          }
          aWeight = f_displayedValue;
          //不稳 为负 停止计时
          //stopWatch.stop();
        }
      }
    }
  }

  //记录咖啡粉时，将重量固定为0
  if (scale.getTareStatus()) {  //espresso
    b_minus_container = false;
    //buzzer.beep(2, 50);
    //Serial.println("beep2 espressoScale");
    b_ready_to_brew = true;
    b_weight_quick_zero = false;
    t_tareStatus = millis();
  }
  if (b_weight_quick_zero)
    f_displayedValue = 0.0;

  float ratio_temp = f_displayedValue / f_weight_dose;
  if (ratio_temp < 0)
    ratio_temp = 0.0;
  if (f_weight_dose < 0.1)
    ratio_temp = 0.0;

  dtostrf(f_displayedValue, 7, 1, c_weight);
  dtostrf(ratio_temp, 7, 1, c_brew_ratio);
#ifdef DEBUG_BT
#ifdef CHINESE
  Serial.print("意式模式 ");
  Serial.print("原重:");
  Serial.print(f_weight_adc);
  Serial.print(",平滑:");
  Serial.print(f_weight_smooth);
  Serial.print(",显示:");
  Serial.println(f_displayedValue);
  SerialBT.print("意式模式 ");
  SerialBT.print("原重:");
  SerialBT.print(f_weight_adc);
  SerialBT.print(",平滑:");
  SerialBT.print(f_weight_smooth);
  SerialBT.print(",显示:");
  SerialBT.println(f_displayedValue);
#endif
#ifdef ENGLISH
  Serial.print("Espresso Mode ");
  Serial.print("Raw:");
  Serial.print(f_weight_adc);
  Serial.print(",Smoothed:");
  Serial.print(f_weight_smooth);
  Serial.print(",Displayed:");
  Serial.println(f_displayedValue);
  SerialBT.print("Espresso Mode ");
  SerialBT.print("Raw:");
  SerialBT.print(f_weight_adc);
  SerialBT.print(",Smoothed:");
  SerialBT.print(f_weight_smooth);
  SerialBT.print(",Displayed:");
  SerialBT.println(f_displayedValue);
#endif
#endif
}


/**
 * Enhanced adaptive tracking system
 * Tracks both zero and stable weights to prevent oscillation
 */
void updateAdaptiveTracking(float current_weight) {
  unsigned long current_time = millis();

  if (!b_tracking_enabled) {
    return;
  }

  // Calculate weight difference from current tracking target
  float weight_diff = current_weight - f_tracking_target;

  // Check if weight is stable (within tracking threshold)
  if (fabs(weight_diff) <= TRACKING_THRESHOLD) {
    i_stable_count++;

    // Update tracking target to slowly follow stable weights
    if (i_stable_count >= 3) {               // Start adjusting target after 3 stable readings
      float adjustment = weight_diff * 0.1;  // Slow adaptation

      // Limit maximum adjustment to prevent large jumps
      if (fabs(adjustment) > MAX_TRACKING_ADJUSTMENT) {
        adjustment = (adjustment > 0) ? MAX_TRACKING_ADJUSTMENT : -MAX_TRACKING_ADJUSTMENT;
      }

      f_tracking_target += adjustment;
    }

  } else {
    // Weight changed significantly - likely a real weight change
    i_stable_count = 0;
    b_tracking_active = false;

    // If weight change is large and persistent, update tracking target
    if (fabs(weight_diff) > TRACKING_THRESHOLD * 2) {
      // Consider this as a new stable weight after verification
      if (verifyWeightStability(current_weight)) {
        f_tracking_target = current_weight;
        b_tracking_active = true;
        if (b_weight_in_serial) {
          Serial.print("New weight target set: ");
          Serial.println(f_tracking_target, 4);
        }
      }
    }
  }

  // Perform tracking adjustment when conditions are met
  if (i_stable_count >= i_STABLE_COUNT_THRESHOLD) {
    if (current_time - t_last_tracking_update >= TRACKING_UPDATE_INTERVAL) {
      performTrackingAdjustment(current_weight);
    }
  }
}

/**
 * Perform the actual tracking adjustment
 */
void performTrackingAdjustment(float current_weight) {
  float old_offset = f_tracking_offset;

  // Calculate new offset based on current weight and target
  float calculated_offset = current_weight - f_tracking_target;

  // Apply slow adaptation to prevent sudden changes
  f_tracking_offset = f_tracking_offset * 0.8 + calculated_offset * 0.2;

  // Activate tracking if not already active
  if (!b_tracking_active) {
    b_tracking_active = true;
  }

  // Debug output
  if (b_weight_in_serial) {
    Serial.print("Tracking adjustment: Offset ");
    Serial.print(old_offset, 4);
    Serial.print("g -> ");
    Serial.print(f_tracking_offset, 4);
    Serial.print("g | Target: ");
    Serial.print(f_tracking_target, 4);
    Serial.print("g | Raw: ");
    Serial.print(current_weight, 4);
    Serial.println("g");
  }

  // Reset counters
  i_stable_count = i_STABLE_COUNT_THRESHOLD - 2;  // Keep near threshold for continuous tracking
  t_last_tracking_update = millis();
}


/**
 * Verify if a weight is stable enough to be considered a new target
 */
bool verifyWeightStability(float current_weight) {
  static float last_verified_weight = 0.0;
  static int verification_count = 0;

  if (fabs(current_weight - last_verified_weight) <= TRACKING_THRESHOLD) {
    verification_count++;
  } else {
    verification_count = 0;
  }

  last_verified_weight = current_weight;

  // Require 3 consecutive stable readings to verify new weight
  return (verification_count >= 3);
}

/**
 * Apply tracking compensation to raw weight
 */
float applyTrackingCompensation(float raw_weight) {
  if (b_tracking_active && b_tracking_enabled) {
    return raw_weight - f_tracking_offset;
  }
  return raw_weight;
}

/**
 * Apply stable output filtering
 * Returns the same value if change is below threshold
 */
float applyStableOutput(float current_value) {
  if (!b_stable_output_enabled) {
    return current_value;  // Bypass stable filtering if disabled
  }

  float change = fabs(current_value - f_previous_stable_value);

  // If change is significant, update the stable value
  if (change >= STABLE_OUTPUT_THRESHOLD) {
    f_previous_stable_value = current_value;
    t_last_stable_change = millis();

    // Debug output for significant changes
    if (b_weight_in_serial) {
      Serial.print("Output updated: ");
      Serial.print(current_value, 4);
      Serial.print("g (Change: ");
      Serial.print(change, 4);
      Serial.println("g)");
    }
  }

  // Always return the stable value (may be same as previous)
  return f_previous_stable_value;
}

void pureScale() {
  static bool b_newDataReady = 0;
  static bool b_scaleStable = 0;
  float f_weight_adc_raw = 0;
  if (scale.update()) b_newDataReady = true;

  if (b_newDataReady) {
    float raw_weight = scale.getData();
    f_current_raw_value = raw_weight;  // Store for status display

    // Continuous temperature drift detection and compensation
    // 1. Calculate difference between current raw and displayed value
    float current_diff = raw_weight - f_displayedValue - f_driftCompensation;
    
    // 2. If difference is small (0.01g-0.1g), accumulate to continuous compensation
    if (fabs(current_diff) > 0.01 && fabs(current_diff) < f_maxDriftCompensation) {
      static int f_similar_diff_count = 0;
      static float f_last_diff = current_diff;
      
      // Check if continuous same direction change
      if ((f_last_diff * current_diff) > 0) {  // Same direction
        f_similar_diff_count++;
        
        // If continuous micro changes, increase continuous compensation
        if (f_similar_diff_count >= 3) {
          // Increase continuous compensation (slowly)
          f_driftCompensation += current_diff * 0.3;  // Compensate 30% each time
          
          // Limit compensation range
          if (fabs(f_driftCompensation) > 2.0) {
            f_driftCompensation = (f_driftCompensation > 0) ? 2.0 : -2.0;
          }
          
          if (b_weight_in_serial) {
            Serial.print("TEMP-DRIFT-COMP: diff=");
            Serial.print(current_diff, 4);
            Serial.print("g, total_comp=");
            Serial.print(f_driftCompensation, 4);
            Serial.print("g, count=");
            Serial.println(f_similar_diff_count);
          }
          
          f_similar_diff_count = 2;  // Keep partial count for continued detection
        }
      } else {
        // Direction changed, reset
        f_similar_diff_count = 1;
      }
      
      f_last_diff = current_diff;
    } else {
      // Difference too large or too small, reset detection
      static int f_similar_diff_count = 0;
      f_similar_diff_count = 0;
    }
    
    // 3. Apply continuous temperature compensation
    float temperature_compensated = raw_weight - f_driftCompensation;
    
    // 4. Original processing pipeline
    float tracking_compensated = applyTrackingCompensation(temperature_compensated);
    float stable_output = applyStableOutput(tracking_compensated);
    
    if (stable_output >= -0.14 && stable_output <= 0.14) {
      f_displayedValue = 0.0;
    } else {
      // scale value is outside tolerance range, update displayed value
      f_displayedValue = stable_output;
    }
    // Update adaptive tracking (use temperature compensated value)
    updateAdaptiveTracking(tracking_compensated);

    if (!b_minus_container_button) {
      //自动减去容器重量
      if (f_weight_container >= 1 && f_displayedValue >= f_weight_container - NegativeTolerance && f_weight_smooth <= f_weight_container + PositiveTolerance) {
        // calculate difference between scale value and container value
        f_displayedValue = f_displayedValue - f_weight_container;
        b_minus_container = true;
      } else
        b_minus_container = false;
    } else if (f_weight_container >= 1) {
      //手动减去容器重量
      f_displayedValue = f_displayedValue - f_weight_container;
      b_minus_container = true;
    }

    if (f_displayedValue >= -0.14 && f_displayedValue <= 0.14) 
      f_displayedValue = 0.0;
    // print smoothed reading

    f_weight_before_input = f_displayedValue;

    dtostrf(f_displayedValue, 7, i_decimal_precision, c_weight);
    if (b_weight_in_serial == true) {
      unsigned long current_time = millis();
      if (current_time - t_last_status_display >= STATUS_DISPLAY_INTERVAL) {
        Serial.println("=== Temperature Drift Status ===");
        Serial.print("Raw: ");
        Serial.print(raw_weight, 4);
        Serial.print("g | TempComp: ");
        Serial.print(f_driftCompensation, 4);
        Serial.print("g | AfterTempComp: ");
        Serial.print(temperature_compensated, 4);
        Serial.println("g");
        
        Serial.print("Displayed: ");
        Serial.print(f_displayedValue, 4);
        Serial.print("g | Raw-Display Diff: ");
        Serial.print(raw_weight - f_displayedValue, 4);
        Serial.println("g");
        
        displayEnhancedStatus(temperature_compensated, tracking_compensated, stable_output);
        t_last_status_display = current_time;
      }
    }
#ifdef DEBUG_BT
#ifdef CHINESES
    Serial.print("称重模式 ");
    Serial.print("原重:");
    Serial.print(f_weight_adc);
    Serial.print(",平滑:");
    Serial.print(f_weight_smooth);
    Serial.print(",显示:");
    Serial.println(f_displayedValue);
    SerialBT.print("称重模式 ");
    SerialBT.print("原重:");
    SerialBT.print(f_weight_adc);
    SerialBT.print(",平滑:");
    SerialBT.print(f_weight_smooth);
    SerialBT.print(",显示:");
    SerialBT.println(f_displayedValue);
    Serial.print("Gyro Z:");
    Serial.println(a.acceleration.z);
#endif
#ifdef ENGLISH
    Serial.print("Weighing Mode ");
    Serial.print("Raw:");
    Serial.print(f_weight_adc);
    Serial.print(",Smoothed:");
    Serial.print(f_weight_smooth);
    Serial.print(",Displayed:");
    Serial.println(f_displayedValue);
    SerialBT.print("Weighing Mode ");
    SerialBT.print("Raw:");
    SerialBT.print(f_weight_adc);
    SerialBT.print(",Smoothed:");
    SerialBT.print(f_weight_smooth);
    SerialBT.print(",Displayed:");
    SerialBT.println(f_displayedValue);
    Serial.print("Gyro Z:");
    Serial.println(a.acceleration.z);
#endif
#endif
    b_newDataReady = false;
  }

  if (scale.getTareStatus()) {  //pure
    //buzzer.beep(2, 50);
    t_tareStatus = millis();
    b_weight_quick_zero = false;
    resetTracking();
    resetStableOutput();
    f_driftCompensation = 0.0;
    f_displayedValue = 0.0;
    if (b_weight_in_serial) {
      Serial.println("TARE: Temperature drift compensation reset");
    }
  }
  //记录咖啡粉时，将重量固定为0
  if (b_weight_quick_zero) {
    f_displayedValue = 0.0;
    f_driftCompensation = 0.0;
    resetTracking();
    resetStableOutput();
  }

  float ratio_temp = f_displayedValue / f_weight_dose;
  if (ratio_temp < 0)
    ratio_temp = 0.0;
  if (f_weight_dose < 0.1)
    ratio_temp = 0.0;
  dtostrf(ratio_temp, 7, i_decimal_precision, c_brew_ratio);
}

/**
 * Get current temperature compensation value
 */
float getTemperatureDriftCompensation() {
  return f_driftCompensation;
}

/**
 * Manually adjust temperature compensation
 */
void adjustTemperatureDriftCompensation(float amount) {
  f_driftCompensation += amount;
  Serial.print("Manual temp-comp adjust: ");
  Serial.print(amount, 4);
  Serial.print("g, total: ");
  Serial.println(f_driftCompensation, 4);
}

/**
 * Reset tracking system (for tare/zero operations)
 */
void resetTracking() {
  f_tracking_offset = 0.0;
  f_tracking_target = 0.0;
  i_stable_count = 0;
  b_tracking_active = false;
  t_last_tracking_update = millis();
  if (b_weight_in_serial) {
    Serial.println("Tracking system reset");
  }
}

/**
 * Reset stable output system
 */
void resetStableOutput() {
  f_previous_stable_value = 0.0;
  t_last_stable_change = millis();
  if (b_weight_in_serial) {
    Serial.println("Stable output reset");
  }
}

/**
 * Enable/disable stable output
 */
void setStableOutputEnabled(bool enabled) {
  b_stable_output_enabled = enabled;
  if (!enabled) {
    resetStableOutput();
  }
  Serial.print("Stable output ");
  Serial.println(enabled ? "enabled" : "disabled");
}

/**
 * Set stable output threshold
 */
void setStableOutputThreshold(float threshold) {
  STABLE_OUTPUT_THRESHOLD = threshold;
  Serial.print("Stable threshold set to: ");
  Serial.println(threshold, 4);
}

/**
 * Enable/disable tracking system
 */
void setTrackingEnabled(bool enabled) {
  b_tracking_enabled = enabled;
  if (!enabled) {
    resetTracking();
  }
  Serial.print("Tracking system ");
  Serial.println(enabled ? "enabled" : "disabled");
}

/**
 * Enhanced status display with all system info
 */
void displayEnhancedStatus(float raw_weight, float compensated_weight, float stable_weight) {
  Serial.println("=== Enhanced Scale Status ===");
  Serial.print("Raw Input: ");
  Serial.print(raw_weight, 4);
  Serial.print("g | Compensated: ");
  Serial.print(compensated_weight, 4);
  Serial.print("g | Stable Output: ");
  Serial.print(stable_weight, 4);
  Serial.println("g");

  Serial.print("Stable Output: ");
  Serial.print(b_stable_output_enabled ? "ON" : "OFF");
  Serial.print(" | Threshold: ±");
  Serial.print(STABLE_OUTPUT_THRESHOLD, 4);
  Serial.println("g");

  Serial.print("Last Stable Change: ");
  Serial.print((millis() - t_last_stable_change) / 1000);
  Serial.println("s ago");

  // Tracking status
  Serial.print("Tracking System: ");
  Serial.print(b_tracking_enabled ? "ON" : "OFF");
  Serial.print(" | Active: ");
  Serial.println(b_tracking_active ? "YES" : "NO");

  Serial.print("Tracking Offset: ");
  Serial.print(f_tracking_offset, 4);
  Serial.print("g | Target: ");
  Serial.print(f_tracking_target, 4);
  Serial.println("g");

  Serial.print("Stable Count: ");
  Serial.print(i_stable_count);
  Serial.print("/");
  Serial.println(i_STABLE_COUNT_THRESHOLD);

  Serial.println("=============================");
}

/**
 * Get current tracking offset
 */
float getTrackingOffset() {
  return f_tracking_offset;
}

/**
 * Get current stable output value
 */
float getStableOutputValue() {
  return f_previous_stable_value;
}

// Optional: Manual control functions
/**
 * Manual override - set specific tracking offset
 */
void setManualTrackingOffset(float offset) {
  f_tracking_offset = offset;
  b_tracking_active = true;
  Serial.print("Manual tracking offset set: ");
  Serial.println(offset, 4);
}

/**
 * Manual override - set specific stable value
 */
void setManualStableValue(float value) {
  f_previous_stable_value = value;
  t_last_stable_change = millis();
  Serial.print("Manual stable value set: ");
  Serial.println(value, 4);
}


#if defined(DEBUG) && defined(CHECKBATTERY)
//debug信息
char c_adc_bat[10] = "";
char c_adc_usb[10] = "";
char c_raw[10] = "";
char c_adc_voltage_bat[10] = "";
char c_adc_voltage_usb[10] = "";
char c_real_voltage_bat[10] = "";
char c_real_voltage_usb[10] = "";
char c_show_voltage_bat[10] = "";
char c_show_voltage_usb[10] = "";
char c_up_hr[10] = "";
char c_up_min[10] = "";
char c_up_sec[10] = "";
char c_battery_life_hr[10] = "";
char c_battery_life_min[10] = "";
char c_battery_life_sec[10] = "";
char c_up_battery[10] = "";
long t_adc_interval = 0;


void debugData() {
  if (millis() > t_oled_refresh) {
    //达到设定的oled刷新频率后进行刷新
    t_oled_refresh = millis();
    u8g2.firstPage();
    do {
      u8g2.setFontDirection(0);
#ifdef ROTATION_180
      u8g2.setDisplayRotation(U8G2_R2);
#else
      u8g2.setDisplayRotation(U8G2_R0);
#endif

      u8g2.setFont(FONT_S);
      int x = 0;
      int y = u8g2.getMaxCharHeight() - 3;
      dtostrf(f_up_battery, 7, 2, c_up_battery);
      dtostrf(analogRead(BATTERY_LEVEL), 7, 0, c_adc_bat);
      dtostrf(analogRead(USB_LEVEL), 7, 0, c_adc_usb);
      dtostrf(analogRead(BATTERY_LEVEL) * f_vref / (pow(2, ADC_BIT) - 1), 7, 2, c_adc_voltage_bat);
      dtostrf(analogRead(USB_LEVEL) * f_vref / (pow(2, ADC_BIT) - 1), 7, 2, c_adc_voltage_usb);
      dtostrf(analogRead(BATTERY_LEVEL) * f_vref / (pow(2, ADC_BIT) - 1) * f_divider_factor / 2, 7, 2, c_real_voltage_bat);
      dtostrf(analogRead(USB_LEVEL) * f_vref / (pow(2, ADC_BIT) - 1) * f_divider_factor / 2, 7, 2, c_real_voltage_usb);
      dtostrf(analogRead(BATTERY_LEVEL) * f_vref / (pow(2, ADC_BIT) - 1) * f_divider_factor, 7, 2, c_show_voltage_bat);
      dtostrf(analogRead(USB_LEVEL) * f_vref / (pow(2, ADC_BIT) - 1) * f_divider_factor, 7, 2, c_show_voltage_usb);

      if (millis() - t_adc_interval > 4000) {
        float f_now_battery = analogRead(BATTERY_LEVEL) * f_vref / (pow(2, ADC_BIT) - 1) * f_divider_factor;
        float f_v_drop_rate = (f_up_battery - f_now_battery) / (millis() - t_up_battery);
        long t_battery_life = (f_now_battery - 3.3) / f_v_drop_rate;
        int i_battery_life_hr = numberOfHours(t_battery_life / 1000);
        int i_battery_life_min = numberOfMinutes(t_battery_life / 1000);
        int i_battery_life_sec = numberOfSeconds(t_battery_life / 1000);
        dtostrf(i_battery_life_hr, 7, 0, c_battery_life_hr);
        dtostrf(i_battery_life_min, 7, 0, c_battery_life_min);
        dtostrf(i_battery_life_sec, 7, 0, c_battery_life_sec);
        dtostrf(numberOfHours(millis() / 1000), 7, 0, c_up_hr);
        // dtostrf(numberOfMinutes(millis() / 1000), 7, 0, c_up_min);
        // dtostrf(numberOfSeconds(millis() / 1000), 7, 0, c_up_sec);

        t_adc_interval = millis();
      }
      // if (scale.update()) {
      //   f_weight_adc = scale.getData();
      //   dtostrf(f_weight_adc, 7, 1, c_raw);
      // }
      u8g2.drawUTF8(AR(trim(c_up_battery)), y, trim(c_up_battery));
      u8g2.drawUTF8(x, y, trim(c_adc_bat));
      u8g2.drawUTF8(x, y * 2, trim(c_adc_voltage_bat));
      u8g2.drawUTF8(x, y * 3, trim(c_real_voltage_bat));
      u8g2.drawUTF8(x, y * 4, trim(c_show_voltage_bat));
      u8g2.drawUTF8(AC(trim(c_adc_usb)), y, trim(c_adc_usb));
      u8g2.drawUTF8(AC(trim(c_adc_usb)), y * 2, trim(c_adc_voltage_usb));
      u8g2.drawUTF8(AC(trim(c_adc_usb)), y * 3, trim(c_real_voltage_usb));
      u8g2.drawUTF8(AC(trim(c_adc_usb)), y * 4, trim(c_show_voltage_usb));
      //u8g2.drawUTF8(AR(trim(c_raw)), y, trim(c_raw));
      u8g2.drawUTF8(AR(trim(c_battery_life_hr)), y * 2, trim(c_battery_life_hr));
      u8g2.drawUTF8(AR(trim(c_battery_life_min)), y * 3, trim(c_battery_life_min));
      u8g2.drawUTF8(AR(trim(c_battery_life_sec)), y * 4, trim(c_battery_life_sec));
      u8g2.drawUTF8(90, y * 2, trim(c_up_hr));
      u8g2.drawUTF8(90, y * 3, trim(c_up_min));
      u8g2.drawUTF8(90, y * 4, trim(c_up_sec));

    } while (u8g2.nextPage());
  }
}
#endif  // defined(DEBUG) && defined(CHECKBATTERY)

void serialCommand() {
  if (Serial.available()) {
    String inputString = Serial.readStringUntil('\n');
    inputString.trim();

    if (inputString.startsWith("welcome ")) {
      //strcpy(str_welcome, inputString.substring(8).c_str());
      EEPROM.put(i_addr_welcome, inputString.substring(8));
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
      EEPROM.commit();
#endif
    }

    if (inputString.startsWith("cp ")) {  //set default pour over ground weight
      INPUTCOFFEEPOUROVER = inputString.substring(3).toFloat();
      EEPROM.put(INPUTCOFFEEPOUROVER_ADDRESS, INPUTCOFFEEPOUROVER);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
      EEPROM.commit();
#endif
    }

    if (inputString.startsWith("v")) {  //电压
      Serial.print("Battery Voltage:");
      Serial.print(f_batteryVoltage);
#ifndef ADS1115ADC
      int adcValue = analogRead(BATTERY_PIN);                              // Read the value from ADC
      float voltageAtPin = (adcValue / adcResolution) * referenceVoltage;  // Calculate voltage at ADC pin
      Serial.print("\tADC Voltage:");
      Serial.print(voltageAtPin);
      Serial.print("\tbatteryCalibrationFactor: ");
      Serial.print(f_batteryCalibrationFactor);
#endif
      Serial.print("\tlowBatteryCounterTotal: ");
      Serial.print(i_lowBatteryCountTotal);
    }

    if (inputString.startsWith("vf ")) {                                                 // Command to set the battery voltage calibration factor
      int adcValue = analogRead(BATTERY_PIN);                                            // Read the ADC value from the battery pin
      float voltageAtPin = (adcValue / adcResolution) * referenceVoltage;                // Calculate the voltage at the ADC pin
      float batteryVoltage = voltageAtPin * dividerRatio;                                // Calculate the actual battery voltage using the voltage divider ratio
      f_batteryCalibrationFactor = inputString.substring(3).toFloat() / batteryVoltage;  // Calculate the calibration factor from user input
      EEPROM.put(i_addr_batteryCalibrationFactor, f_batteryCalibrationFactor);           // Store the calibration factor in EEPROM

#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
      EEPROM.commit();  // Commit changes to EEPROM to save the calibration factor
#endif

      Serial.print("Battery Voltage Factor set to: ");  // Output the new calibration factor to the Serial Monitor
      Serial.println(f_batteryCalibrationFactor);
    }

    if (inputString.startsWith("ce ")) {  //set default espresso ground weight
      INPUTCOFFEEESPRESSO = inputString.substring(3).toFloat();
      EEPROM.put(INPUTCOFFEEESPRESSO_ADDRESS, INPUTCOFFEEESPRESSO);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
      EEPROM.commit();
#endif
    }

    if (inputString.startsWith("ct ")) {  //set container weight
      f_weight_container = inputString.substring(3).toFloat();
      EEPROM.put(i_addr_container, f_weight_container);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
      EEPROM.commit();
#endif
    }

    if (inputString.startsWith("cv ")) {  //set scale calibration value
      f_calibration_value = inputString.substring(3).toFloat();
      EEPROM.put(i_addr_calibration_value, f_calibration_value);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
      EEPROM.commit();
#endif
    }


    if (inputString.startsWith("reset")) {  //重启
      reset();
    }

    // if (inputString.startsWith("sample")) {
    //   b_set_sample = true;
    //   i_sample = 0;  //读取失败 默认值为3 对应sample为8
    //   i_sample_step = 0;
    //   setSample();
    // }

    if (inputString.startsWith("cal")) {  //校准
      b_calibration = true;               //让按钮进入校准状态3
      //cal();                              //无有效读取，进入校准模式
    }


    if (inputString.startsWith("ota")) {  //WiFi ota
      b_ota = true;
    }

    if (inputString.startsWith("tare")) {
      buttonTare_Pressed();
      buttonTARE_Released();
    }

    if (inputString.startsWith("set")) {
      buttonSet_Pressed();
    }

    if (inputString.startsWith("beep")) {  //蜂鸣器
      b_beep = !b_beep;
      EEPROM.put(i_addr_beep, b_beep);
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)
      EEPROM.commit();
#endif
    }


// Send the updated values via USB serial
#ifdef CHINESE
    Serial.print("手冲默认粉重:");
    Serial.print(INPUTCOFFEEPOUROVER);
    Serial.print("g\t意式默认粉重:");
    Serial.print(INPUTCOFFEEESPRESSO);
    Serial.print("g\t容器重量:");
    Serial.print(f_weight_container);
    Serial.print("g\t蜂鸣器状态:");
    if (b_beep) {
      Serial.println("打开");
    } else {
      Serial.println("关闭");
    }
#endif
#ifdef ENGLISH
    Serial.print("Pour Over default ground weight:");
    Serial.print(INPUTCOFFEEPOUROVER);
    Serial.print("g\tEspresso default ground weight:");
    Serial.print(INPUTCOFFEEESPRESSO);
    Serial.print("g\tContainer Weight:");
    Serial.print(f_weight_container);
#ifdef BUZZER

    Serial.print("g\tBuzzer:");
    if (b_beep) {
      Serial.println("On");
    } else {
      Serial.println("Off");
    }
#endif
#endif
  }
}


void sendBleWeight() {
  if (deviceConnected) {
    unsigned long currentMillis = millis();

    if (currentMillis - lastWeightNotifyTime >= weightNotifyInterval) {
      // Save the last time you sent the weight notification
      lastWeightNotifyTime = currentMillis;

      byte data[7];
      float weight = scale.getData();
      byte weightByte1, weightByte2;

      encodeWeight(weight, weightByte1, weightByte2);

      data[0] = modelByte;
      data[1] = 0xCE;  // Type byte for weight stable
      data[2] = weightByte1;
      data[3] = weightByte2;
      // Fill the rest with dummy data or real data as needed
      data[4] = 0x00;
      data[5] = 0x00;
      data[6] = calculateXOR(data, 6);  // Last byte is XOR validation

      pReadCharacteristic->setValue(data, 7);
      pReadCharacteristic->notify();
    }
  }
}

void sendBleButton(int buttonNumber, int buttonShortPress) {
  if (b_ble_enabled) {
    //buttonNumber 0 for button O, 1 for button[]
    //isShortPress ture for short press, false for long press
    byte data[7];
    float weight = scale.getData();
    byte weightByte1, weightByte2;

    encodeWeight(weight, weightByte1, weightByte2);

    data[0] = modelByte;
    data[1] = 0xAA;  // Type byte for weight stable
    data[2] = buttonNumber;
    data[3] = buttonShortPress;
    // Fill the rest with dummy data or real data as needed
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = calculateXOR(data, 6);  // Last byte is XOR validation

    pReadCharacteristic->setValue(data, 7);
    pReadCharacteristic->notify();
  }
}


void sendBlePowerOff(int i_reason) {
  if (b_ble_enabled && deviceConnected) {
    byte data[7];
    data[0] = modelByte;
    data[1] = 0x2A;
    switch (i_reason) {
      case 0:
        data[2] = 0x00;  //power off failed because it's disabled.
        break;
      case 1:
        data[2] = 0x10;  //power off from from "O" button double-click.
        break;
      case 2:
        data[2] = 0x11;  //power off from from "[]" button double-click.
        break;
      case 3:
        data[3] = 0x20;  //power off low battery.
        break;
      case 4:
        data[2] = 0x30;  //power off from gyro.
        break;
      default:
        data[2] = 0x00;  //Don't power off because i_reason is not valid.
        break;
    }
    data[3] = 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = calculateXOR(data, 6);  // Last byte is XOR validation

    pReadCharacteristic->setValue(data, 7);
    pReadCharacteristic->notify();
  }
}

void loop() {
  if (b_powerOff) {
    shut_down_now_nobeep();
    return;
  }

  // if (bleState == CONNECTED && b_requireHeartBeat) {
  //   if (millis() - t_heartBeat > HEARTBEAT_TIMEOUT) {
  //     disconnectBLE();
  //     t_heartBeat = millis() + 10000;
  //   }
  // }

  if (deviceConnected) {
    power_off(-1);  //reset power off timer
  } else {
    power_off(15);  //power off after 15 minutes
  }
  serialCommand();
  if (b_ble_enabled)
    sendBleWeight();
  buttonSet.check();
  buttonTare.check();
#ifdef BUZZER

  buzzer.check();
#endif

  // measureTemperature();
#if defined(FOUR_BUTTON) || defined(FIVE_BUTTON)
  buttonPlus.check();
  buttonMinus.check();
#endif  //extra button for four and five buttons
#ifdef FIVE_BUTTON
  buttonPower.check();
#endif  //FIVE_BUTTON

#if defined(ACC_MPU6050) || defined(ACC_BMA400)
  if (b_gyroEnabled) {
#ifdef GYROFACEUP
    if (gyro_z() > 5)
      power_off_gyro(-1);
#endif
#ifdef GYROFACEDOWN
    if (gyro_z() < -5)
      power_off_gyro(-1);
#endif
    power_off_gyro(10);
  }
#endif

#if defined(DEBUG) && defined(CHECKBATTERY)
  debugData();
#endif  //DEBUG \
        // #ifdef CHECKBATTERY \
        //   checkBattery(); \
        // #endif

  if (millis() - t_batteryRefresh > i_batteryRefreshTareInterval) {
    updateBattery(BATTERY_PIN);
    t_batteryRefresh = millis();
  }
  checkBattery();
  if (b_menu) {
    showMenu();
  } else if (GPIO_power_on_with == BATTERY_CHARGING) {
    if (digitalRead(BATTERY_CHARGING) == TRIGGER_LEVEL) {
      //read voltage
      // float voltage = f_batteryVoltage;
      // float perc = map(voltage * 1000, lowBatteryThreshold * 1000, batteryMaxVoltage * 1000, 0, 100);  //map funtion doesn't take float as input.
      // chargingOLED((int)perc, voltage);                                                                //show charging ui
      u8g2.firstPage();
      do {
        int x_battery = 121;
        int y_battery = 0;
        u8g2.drawXBM(x_battery, y_battery, 7, 12, image_battery_charging);  // charging icon
      } while (u8g2.nextPage());
    } else {
      //read voltage
      float voltage = f_batteryVoltage;
      if (voltage > 4.1) {
        //charging complete
        Serial.println("Charging compelete.");
      } else {
        //charging not complete, but the serial maynot be ouput cause usb unplugged.
        Serial.println("USB Unplugged, charging not compelete.");
      }
      shut_down_now_nobeep();  //deepsleep
    }
  } else {
    if (b_ota == true) {
#ifdef WIFI
      ElegantOTA.loop();
#endif
    } else if (b_calibration == true) {
      calibration(i_calibration);
    } else if (b_usbLinked == true) {
      //showing charging animation when powered off
      //charging();
    } else {
      if (b_bootTare) {
        //tare after boot
        if (millis() - t_bootTare > i_bootTareDelay) {
          scale.tareNoDelay();
          b_bootTare = false;
        }
      } else if (b_tareByButton) {
        // Tare by button, ensure 500ms delay to avoid touch interference
        if (millis() - t_tareByButton > i_tareDelay) {
          scale.tareNoDelay();
          b_tareByButton = false;  // reset status
          Serial.println("Tare by button");
        }
      } else if (b_tareByBle) {
        // Tare by BLE, performed instantly without delay
        scale.tareNoDelay();
        b_tareByBle = false;  // reset status
        Serial.println("Tare by BLE");
      }
      if (b_extraction) {
        if (b_mode)
          espressoScale();
        else
          pourOverScale();
      } else {
        pureScale();
      }
      updateOled();
    }
#ifdef ESPNOW
    if (b_espnow) {
      updateEspnow();
    }
#endif
  }
}

void chargingOLED(int perc, float voltage) {
  if (millis() > t_oled_refresh + 1000) {
    // Refresh the OLED at the specified interval
    t_oled_refresh = millis();
    u8g2.firstPage();
    do {
#ifdef ROTATION_180
      u8g2.setDisplayRotation(U8G2_R2);
#else
      u8g2.setDisplayRotation(U8G2_R0);
#endif

      // Get the display width and height
      int16_t displayWidth = u8g2.getDisplayWidth();
      int16_t displayHeight = u8g2.getDisplayHeight();

      // Set the size of the battery outline and inner rectangle
      int16_t width = 100, height = 30;
      int16_t innerWidth = width - 2;    // Width of the inner rectangle
      int16_t innerHeight = height - 2;  // Height of the inner rectangle

      // Calculate the centered position for the battery outline and inner rectangle
      int16_t x = (displayWidth - width) / 2;
      int16_t y = (displayHeight - height) / 2 - 6;

      u8g2.setFontMode(1);
      u8g2.setDrawColor(1);
      // Draw the battery outline (rounded rectangle)
      u8g2.drawRFrame(x, y, width, height, 5);  // Outline with a corner radius of 5

      // Calculate the height for the inner rectangle based on percentage
      int16_t innerY = y + 1 + innerHeight * (1 - (perc / 100.0));  // Y position of the inner rectangle
      //u8g2.drawBox(x + 1, y + 1, innerWidth * (perc / 100.0), innerHeight);  // Inner rectangle, filled to show charge level
      u8g2.drawVLine(x + width + 2, y + 7, height - 7 * 2);
      for (int i = 0; i < innerWidth; i = i + 2) {
        if (i < (innerWidth * (perc / 100.0))) {
          u8g2.drawVLine(x + 2 + i, y + 2, innerHeight - 2);
        }
      }

      u8g2.setDrawColor(2);
      // Display the battery percentage
      char batteryText[10];
      snprintf(batteryText, sizeof(batteryText), "%d%%", (perc > 100) ? 100 : perc);
      if (perc > 100) {
        strcat(batteryText, "+");
      }
      u8g2.setFont(u8g2_font_ncenB12_tr);                                                                 // Set font
      u8g2.drawStr(x + (width / 4) - (u8g2.getStrWidth(batteryText) / 2), y + height + 15, batteryText);  // Center the percentage text

      // Display the voltage
      char voltageText[10];
      snprintf(voltageText, sizeof(voltageText), "%.2fV", voltage);
      u8g2.drawStr(x + (width / 4) + (width / 2) - (u8g2.getStrWidth(voltageText) / 2), y + height + 15, voltageText);  // Center the voltage text

    } while (u8g2.nextPage());
  }
}


void updateOled() {
  if (millis() > t_oled_refresh + i_oled_print_interval) {
    //达到设定的oled刷新频率后进行刷新
    t_oled_refresh = millis();
    u8g2.firstPage();
    do {
      if (b_debug)
        drawDebug();
      else if (b_extraction) {
        drawExtrationScale();
      } else {
        drawPureScale();
      }
      drawBattery();
      drawButtonBox();
      drawBle();
      drawTare();
      //drawDriftCompensationInfo();
    } while (u8g2.nextPage());
  }
}

void drawPureScale() {
  //纯称重
  u8g2.setFontMode(1);
  u8g2.setDrawColor(1);
#ifdef ROTATION_180
  u8g2.setDisplayRotation(U8G2_R2);
#else
  u8g2.setDisplayRotation(U8G2_R0);
#endif
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);
  int x = 0;
  int y = 12;
  if (b_mode) {
    if (b_minus_container) {
      if (!b_minus_container_button)
        u8g2.drawUTF8(x, y, TEXT_ESPRESSO_AUTO_TARE_CONTAINER);
      else
        u8g2.drawUTF8(x, y, TEXT_ESPRESSO_MANUAL_TARE_CONTAINER);
    } else
      u8g2.drawUTF8(x, y, TEXT_ESPRESSO);
  } else {
    if (b_minus_container) {
      if (!b_minus_container_button)
        u8g2.drawUTF8(x, y, TEXT_POUROVER_AUTO_TARE_CONTAINER);
      else
        u8g2.drawUTF8(x, y, TEXT_POUROVER_MANUAL_TARE_CONTAINER);
    } else
      u8g2.drawUTF8(x, y, TEXT_POUROVER);
  }

  u8g2.setFont(FONT_L);
  //FONT_L_HEIGHT = u8g2.getMaxCharHeight();
  x = AC(trim(c_weight));
  y = AM();
  u8g2.drawUTF8(x, y + 3, trim(c_weight));
}

void drawExtrationScale() {
  int x = 0;
  int y = 0;
  char c_oled_ratio[30];
  sprintf(c_oled_ratio, "1:%s", trim(c_brew_ratio));
  char c_coffee_powder[30];
  dtostrf(f_weight_dose, 7, i_decimal_precision, c_coffee_powder);
  u8g2.setFontMode(1);
  u8g2.setDrawColor(1);
  if (i_display_rotation == 0) {
    u8g2.setFontDirection(0);
#ifdef ROTATION_180
    u8g2.setDisplayRotation(U8G2_R2);
#else
    u8g2.setDisplayRotation(U8G2_R0);
#endif
#ifdef THREE_LINE
    u8g2.setFont(FONT_EXTRACTION);

    //实时重量
    x = Margin_Left;
    y = u8g2.getMaxCharHeight() + Margin_Top - 5;
    u8g2.drawUTF8(x, y, trim(c_weight));

    //时钟
    if (b_mode) {
      u8g2.drawUTF8(AR(sec2sec(stopWatch.elapsed())), y, sec2sec(stopWatch.elapsed()));
      if (t_extraction_first_drop > 0 && t_extraction_first_drop - t_extraction_begin > 0) {  //有下液了
        t_extraction_first_drop_num = (t_extraction_first_drop - t_extraction_begin) / 1000;
        u8g2.drawUTF8(70, y, sec2sec(t_extraction_first_drop_num));
      }
    } else {
      u8g2.drawUTF8(AR(sec2minsec(stopWatch.elapsed())), y, sec2minsec(stopWatch.elapsed()));
    }
    y = y + u8g2.getMaxCharHeight() + 2;
    u8g2.drawUTF8(0, y + 1, trim(c_flow_rate));

    //咖啡粉
    x = Margin_Left;
    y = LCDHeight - Margin_Bottom;
    u8g2.drawUTF8(x, y, trim(c_coffee_powder));
    //粉水比
    u8g2.drawUTF8(AR(trim(c_oled_ratio)), y, trim(c_oled_ratio));

#else
    //时钟
    u8g2.setFont(FONT_L);
    x = Margin_Left;
    y = u8g2.getMaxCharHeight() + Margin_Top - 5;
    if (b_mode) {
      u8g2.drawUTF8(x - 2, y, trim(sec2sec(stopWatch.elapsed())));
      if (t_extraction_first_drop > 0 && t_extraction_first_drop - t_extraction_begin > 0) {  //有下液了
        t_extraction_first_drop_num = (t_extraction_first_drop - t_extraction_begin) / 1000;
        u8g2.drawUTF8(40, y, sec2sec(t_extraction_first_drop_num));
      }
    } else {
      u8g2.drawUTF8(x - 2, y, trim(sec2minsec(stopWatch.elapsed())));
    }
    //粉水比
    u8g2.setFont(FONT_EXTRACTION);
    y = u8g2.getMaxCharHeight() + Margin_Top - 5 - 2;
    u8g2.drawUTF8(AR(trim(c_oled_ratio)), y, trim(c_oled_ratio));

    //实时重量
    u8g2.setFont(FONT_L);
    y = LCDHeight - Margin_Bottom;
    u8g2.drawUTF8(AR(trim(c_weight)) + 2, y, trim(c_weight));

    //流速
    u8g2.setFont(FONT_EXTRACTION);
    x = Margin_Left;
    u8g2.drawUTF8(x, y, trim(c_flow_rate));
#endif
  }
  //////////////////旋转效果
  if (i_display_rotation == 1) {
#ifdef ROTATION_180
    u8g2.setDisplayRotation(U8G2_R3);
#else
    u8g2.setDisplayRotation(U8G2_R1);
#endif
    u8g2.setFontDirection(0);
    u8g2.setFont(FONT_S);
    x = Margin_Left;
    y = u8g2.getMaxCharHeight() + Margin_Top - 5 + 1;
    //00 ESP
    if (b_mode)
      u8g2.drawUTF8(0, y, TEXT_ESPRESSO);
    else
      u8g2.drawUTF8(0, y, TEXT_POUROVER);

    u8g2.setFont(FONT_EXTRACTION);
    //01 粉重
    y = y + u8g2.getMaxCharHeight() + 1;
    u8g2.drawUTF8(x, y, trim(c_coffee_powder));
    //02 重量
    y = y + u8g2.getMaxCharHeight() + 1;
    u8g2.drawUTF8(0, y, trim(c_weight));
    //03 时间
    y = y + u8g2.getMaxCharHeight() + 1;
    if (b_mode) {
      u8g2.drawUTF8(0, y, sec2sec(stopWatch.elapsed()));

      if (t_extraction_first_drop > 0 && t_extraction_first_drop - t_extraction_begin > 0) {  //有下液了
        t_extraction_first_drop_num = (t_extraction_first_drop - t_extraction_begin) / 1000;
        u8g2.drawUTF8(AR(sec2sec(t_extraction_first_drop_num)), y, sec2sec(t_extraction_first_drop_num));
      }
    } else {
      u8g2.drawUTF8(0, y, sec2minsec(stopWatch.elapsed()));
    }

    y = y + u8g2.getMaxCharHeight() + 1;
    u8g2.drawUTF8(0, y, trim(c_flow_rate));

    //04 粉水比
    y = y + u8g2.getMaxCharHeight();

    //u8g2.setFont(FONT_M);
    x = Margin_Left;
    y = LCDHeight - Margin_Bottom;

    u8g2.drawUTF8(0, y, trim(c_oled_ratio));
  }
}

//button pressed box, left and right, added xor
void drawButtonBox() {
  u8g2.setDrawColor(2);
  int button_width = 2;
  int button_length = 40;
  if (i_display_rotation == 1 && b_extraction == true) {
    if (digitalRead(BUTTON_SET) == TRIGGER_LEVEL)
      u8g2.drawBox((LCDWidth - button_length) / 2, LCDHeight - button_width, button_length, button_width);
    if (digitalRead(BUTTON_TARE) == TRIGGER_LEVEL)
      u8g2.drawBox((LCDWidth - button_length) / 2, 0, button_length, button_width);
  } else {
    if (digitalRead(BUTTON_SET) == TRIGGER_LEVEL)
      u8g2.drawBox(0, (LCDHeight - button_length) / 2, button_width, button_length);
    if (digitalRead(BUTTON_TARE) == TRIGGER_LEVEL)
      u8g2.drawBox(LCDWidth - button_width, (LCDHeight - button_length) / 2, button_width, button_length);
  }
}


//some quick variable
long t_ble_box = 0;
bool b_drawBle = false;
void drawBle() {
  int x_ble = 106;
  int y_ble = 0;
  if (b_extraction)
    y_ble = 26;
  if (b_ble_enabled) {
    if (deviceConnected) {
      u8g2.drawXBM(x_ble + 3, y_ble, 5, 13, image_ble_enabled);
    } else {
      if (millis() - t_ble_box > 1000) {
        b_drawBle = !b_drawBle;
        t_ble_box = millis();
      }
      if (b_drawBle)
        u8g2.drawXBM(x_ble + 3, y_ble, 5, 13, image_ble_enabled);
    }
  } else {
    u8g2.drawXBM(x_ble, y_ble, 10, 13, image_ble_disabled);
  }
}

void drawTare() {
  if (millis() - t_tareStatus < 500) {
    if (i_display_rotation == 1 && b_extraction == true) {
      u8g2.drawBox(LCDWidth - 2, 30, 2, 128 - 30 * 2);
    } else {
      u8g2.drawBox(30, LCDHeight - 2, 128 - 30 * 2, 2);
    }
  }
}


void drawBattery() {
  int x_battery = 121;
  int y_battery = 0;
  if (b_extraction)
    y_battery = 26;
#if defined(V7_4) || defined(V7_5) || defined(V8_0) || defined(V8_1)
  //if (getUsbVoltage(USB_DET) > 4.0) {
  if (digitalRead(USB_DET) == TRIGGER_LEVEL) {
#else
  if (digitalRead(BATTERY_CHARGING) == TRIGGER_LEVEL) {
#endif
    b_is_charging = true;
    u8g2.drawXBM(x_battery, y_battery, 7, 12, image_battery_charging);  // charging icon
    // Serial.println("Battery is charging");
  } else {
    b_is_charging = false;
    int i_batteryPercent = map(f_batteryVoltage * 1000, showEmptyBatteryBelowVoltage * 1000, showFullBatteryAboveVoltage * 1000, 0, 100);
    if (i_batteryPercent <= 5) {
      u8g2.drawXBM(x_battery, y_battery, 7, 12, image_battery_0);  // 0% or very low battery
    } else if (i_batteryPercent > 5 && i_batteryPercent <= 25) {
      u8g2.drawXBM(x_battery, y_battery, 7, 12, image_battery_1);  // 5-25% battery
    } else if (i_batteryPercent > 25 && i_batteryPercent <= 50) {
      u8g2.drawXBM(x_battery, y_battery, 7, 12, image_battery_2);  // 25-50% battery
    } else if (i_batteryPercent > 50 && i_batteryPercent <= 75) {
      u8g2.drawXBM(x_battery, y_battery, 7, 12, image_battery_3);  // 50-75% battery
    } else if (i_batteryPercent > 75) {
      u8g2.drawXBM(x_battery, y_battery, 7, 12, image_battery_4);  // 75-100% battery
    }
  }
}


long t_debug = 0;
void drawDebug() {
  if (b_debug) {
    char bleText[20];
    if (b_ble_enabled) {
      if (deviceConnected) {
        snprintf(bleText, sizeof(bleText), "BLE connected");
      } else {
        snprintf(bleText, sizeof(bleText), "BLE enabled");
      }
    } else {
      snprintf(bleText, sizeof(bleText), "BLE disabled");
    }

    char chargingText[20];
    if (digitalRead(BATTERY_CHARGING) == TRIGGER_LEVEL)  //low for charging
      snprintf(chargingText, sizeof(chargingText), "Charging");
    else
      snprintf(chargingText, sizeof(chargingText), "Not charging");

    char batteryText[10];
    float voltage = f_batteryVoltage;
    int perc = map(voltage * 1000, showEmptyBatteryBelowVoltage * 1000, showFullBatteryAboveVoltage * 1000, 0, 100);  //map funtion doesn't take float as input.
    snprintf(batteryText, sizeof(batteryText), "%d%%", (perc > 100) ? 100 : perc);

    char voltageText[10];
    if (perc > 100) {
      strcat(batteryText, "+");
    }
    snprintf(voltageText, sizeof(voltageText), "%.1fV", voltage);

    char gpioText[10];
    snprintf(gpioText, sizeof(gpioText), "GPIO:%d", i_wakeupPin);
#if defined(ACC_MPU6050) || defined(ACC_BMA400)
    char gyroText[10];
    snprintf(gyroText, sizeof(gyroText), "Gyro:%.1f", gyro_z());
#endif
    char weightText[10];
    snprintf(weightText, sizeof(weightText), "%.1fg", f_displayedValue);

    //display
    u8g2.setFont(u8g2_font_6x13_tr);
    int lineHeight = 12;
    u8g2.drawStr(-54, lineHeight, LINE1);
    if (b_gyroEnabled) {
      u8g2.drawStr(0, lineHeight * 2, (char *)trim(gpioText));
    }
#if defined(ACC_MPU6050) || defined(ACC_BMA400)
    u8g2.drawStr(0, lineHeight * 3, (char *)trim(gyroText));
#endif
    u8g2.drawStr(0, lineHeight * 4, (char *)trim(chargingText));
    u8g2.drawStr(0, lineHeight * 5, (char *)trim(bleText));

    u8g2.drawStr(55, lineHeight, (char *)trim(batteryText));
    u8g2.drawStr(55, lineHeight * 2, (char *)trim(voltageText));

    u8g2.drawStr(AR((char *)trim(weightText)), lineHeight * 2, (char *)trim(weightText));
    u8g2.drawStr(AR(sec2sec(stopWatch.elapsed())), lineHeight * 3, sec2sec(stopWatch.elapsed()));
  }
}

void drawDriftCompensationInfo() {
  char factorText[20];
  u8g2.setFont(u8g2_font_6x13_tr);
  
  snprintf(factorText, sizeof(factorText), "TUI:%lums", TRACKING_UPDATE_INTERVAL);
  u8g2.drawStr(0, 13, (char *)trim(factorText));
  snprintf(factorText, sizeof(factorText), "TT:%.2f", TRACKING_THRESHOLD);
  u8g2.drawStr(AR((char *)trim(factorText)), 13, (char *)trim(factorText));

  snprintf(factorText, sizeof(factorText), "%.3f", f_maxDriftCompensation);
  u8g2.drawStr(0, 26, (char *)"MDC");
  u8g2.drawStr(0, 39, (char *)trim(factorText));

  snprintf(factorText, sizeof(factorText), "TDC:%.2f", f_driftCompensation * -1);
  u8g2.drawStr(AR((char *)trim(factorText)), 26, (char *)trim(factorText));

  snprintf(factorText, sizeof(factorText), "RAW:%.2f", f_current_raw_value);
  u8g2.drawStr(12, 64, (char *)trim(factorText));
  snprintf(factorText, sizeof(factorText), "%.2f", f_displayedValue);
  u8g2.drawStr(80, 64, (char *)trim(factorText));
}
