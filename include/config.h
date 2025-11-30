#ifndef CONFIG_H
#define CONFIG_H

// BLE UUIDs

#define CUUID_DECENTSCALE_READ "0000FFF4-0000-1000-8000-00805F9B34FB"
#define CUUID_DECENTSCALE_WRITE "000036F5-0000-1000-8000-00805F9B34FB"
#define CUUID_DECENTSCALE_WRITEBACK "83CDC3D4-3BA2-13FC-CC5E-106C351A9352"
#define SUUID_DECENTSCALE "0000FFF0-0000-1000-8000-00805F9B34FB"

//#define ESPNOW
#define V7_2
#define THREE_LINE
#define WIFIOTA

#if defined(V0)
#define TRIGGER_LEVEL HIGH
#else
#define TRIGGER_LEVEL LOW
#endif
//#define SH1116
//#define ADS1232ADC

//#define OLD_PIN  //use old pins on hx711, buzzer. Notice: It'll cover FIVE_BUTTON pin

//#define ROTATION_180

#define TWO_BUTTON
//#define THREE_LINE
//#define FIVE_BUTTON

//POWER DISPLAY
#define SHOWBATTERY
//#define CHECKBATTERY

//SCALE CONFIG
#define LINE1 (char*)"FW: 6.0.0"
#define LINE2 (char*)"Built-date(YYYYMMDD): 20251130"
#define LINE3 (char*)"S/N: DRS073"  //序列号 073
#define VERSION /*版本号 version*/ LINE1, /*编译日期*/ LINE2, /*序列号*/ LINE3
//About info
#define FIRMWARE_VER LINE1
//#define WELCOME1 (char*)"Lian"
#define WELCOME1 (char*)"soso D.R.S."
#define WELCOME2 (char*)"w2"
#define WELCOME3 (char*)"w3"
#define WELCOME WELCOME1, FONT_EXTRACTION

//Language
//#define ENGLISH
#define CHINESE
#ifdef ENGLISH
#define TEXT_ESPRESSO "Espresso"
#define TEXT_ESPRESSO_AUTO_TARE_CONTAINER "Espresso *"
#define TEXT_ESPRESSO_MANUAL_TARE_CONTAINER "Espresso *"
#define TEXT_POUROVER "Pour Over"
#define TEXT_POUROVER_AUTO_TARE_CONTAINER "Pour Over *"
#define TEXT_POUROVER_MANUAL_TARE_CONTAINER "Pour Over *"
#endif
#ifdef CHINESE
#define TEXT_ESPRESSO "意式模式"
#define TEXT_ESPRESSO_AUTO_TARE_CONTAINER "意式模式 *"
#define TEXT_ESPRESSO_MANUAL_TARE_CONTAINER "意式模式 *"
#define TEXT_POUROVER "手冲模式"
#define TEXT_POUROVER_AUTO_TARE_CONTAINER "手冲模式 *"
#define TEXT_POUROVER_MANUAL_TARE_CONTAINER "手冲模式 *"


#endif


#define PositiveTolerance 25  // positive tolerance range in grams
#define NegativeTolerance 5   // negative tolerance range in grams
#define OledTolerance 0.09


//ntc
//#define THERMISTOR_PIN 39
#define SERIESRESISTOR 10000
#define NOMINAL_RESISTANCE 10000
#define NOMINAL_TEMPERATURE 25
#define BCOEFFICIENT 3950
#define FILTER_CONSTANT 0.1

//#define CAL //both button down during welcome text, start calibration
#define BT
//#define DEBUG
//#define DEBUG_BT
//#define DEBUG_BATTERY

//ADC BIT DEPTH
#define ADC_BIT 12

//BUTTON

#define ACEBUTTON  //ACEBUTTON ACEBUTTONT
#define DEBOUNCE 200
#define LONGCLICK 1500
#define DOUBLECLICK 800
#define BUTTON_KEY_DELAY 150

//DISPLAY
//#define HW_I2C        //HW_I2C  HW_SPI  SW_I2C  SW_SPI   //oled连接方式
#define Margin_Top 0  //显示边框
#define Margin_Bottom 0
#define Margin_Left 0
#define Margin_Right 0

#ifdef ESP32



#ifdef V8_1
#define PCB_VER (char*)"PCB: 8.1"
#define HW_SPI
#define SH1106
#define ADS1232ADC
#define ADS1115ADC
#define ROTATION_180

#define I2C_SCL 4
#define I2C_SDA 5
#define BATTERY_PIN 6 //wasn't used but to keep getVoltage(battery_pin) working. Any number is good for that.
#define CHRG_CTRL 6 //Introduced in PCB v8.3.1 to control TP4056 charging for bettery battery voltage sampling.
#define OLED_SDIN 7
#define OLED_SCLK 15
#define OLED_DC 16
#define OLED_RST 17
#define OLED_CS 18
#define USB_DET 8
#define PWR_CTRL 3
//#define NTC 9
#define BATTERY_CHARGING 10
#define SCALE_DOUT 11
#define SCALE_SCLK 12
#define SCALE_PDWN 13
#define SCALE2_DOUT 47
#define SCALE2_SCLK 48
#define SCALE2_PDWN 9
#define ACC_PWR_CTRL 14
#define BUTTON_SET 1  //33
#define BUTTON_TARE 2
#if defined(TWO_BUTTON) || defined(FOUR_BUTTON)
#define GPIO_NUM_BUTTON_POWER GPIO_NUM_1
#endif

#define SCALE_A0 -1
#define HX711_SCL 12
#define HX711_SDA 11
#endif


#ifdef V8_0
#define PCB_VER (char*)"PCB: 8.0.0"
#define HW_SPI
#define SH1106
#define ADS1232ADC
#define ADS1115ADC
#define ACC_BMA400
#define ROTATION_180
#define GYROFACEDOWN  //GYRO //#define GYROFACEUP

#define I2C_SCL 4
#define I2C_SDA 5
#define BATTERY_PIN 6 //wasn't used but to keep getVoltage(battery_pin) working. Any number is good for that.
#define OLED_SDIN 7
#define OLED_SCLK 15
#define OLED_DC 16
#define OLED_RST 17
#define OLED_CS 18
#define USB_DET 8
#define PWR_CTRL 3
//#define NTC 9
#define BATTERY_CHARGING 10
#define SCALE_DOUT 11
#define SCALE_SCLK 12
#define SCALE_PDWN 13
#define ACC_PWR_CTRL 14
#define ACC_INT 21
#define SCALE2_DOUT 47
#define SCALE2_SCLK 48
#define SCALE2_PDWN 9
#define BUTTON_SET 1  //33
#define BUTTON_TARE 2
#if defined(TWO_BUTTON) || defined(FOUR_BUTTON)
#define GPIO_NUM_BUTTON_POWER GPIO_NUM_1
#endif
#define BUZZER 38

#define SCALE_A0 -1
#define HX711_SCL 12
#define HX711_SDA 11
#endif


#ifdef V7_5
#define HW_SPI
#define SH1106
#define ADS1232ADC
#define ROTATION_180
#define GYROFACEDOWN  //GYRO //#define GYROFACEUP

#define I2C_SCL 4
#define I2C_SDA 5
#define BATTERY_PIN 6
#define OLED_SDIN 7
#define OLED_SCLK 15
#define OLED_DC 16
#define OLED_RST 17
#define OLED_CS 18
#define USB_DET 8
#define PWR_CTRL 3
#define NTC 9
#define BATTERY_CHARGING 10
#define SCALE_DOUT 11
#define SCALE_SCLK 12
#define SCALE_PDWN 13
#define MPU_PWR_CTRL 14
#define MPU_INT 21
#define SCALE2_DOUT 47
#define SCALE2_SCLK 48
#define SCALE2_PDWN 39
#define BUTTON_SET 1  //33
#define BUTTON_TARE 2
#if defined(TWO_BUTTON) || defined(FOUR_BUTTON)
#define GPIO_NUM_BUTTON_POWER GPIO_NUM_1
#endif
#define BUZZER 38

#define SCALE_A0 -1
#define BUZZER_LED 38
#define HX711_SCL 12
#define HX711_SDA 11
#endif

#ifdef V7_3
#define HW_SPI
#define SH1106
#define ADS1232ADC
#define ROTATION_180
#define GYROFACEDOWN  //GYRO //#define GYROFACEUP

#define I2C_SCL 4
#define I2C_SDA 5
#define BATTERY_CHARGING 6
#define BATTERY_PIN 7
#define OLED_DC 15
#define OLED_RST 16
#define SCALE_DOUT 8
#define PWR_CTRL 3
#define SCALE_SCLK 9
#define OLED_CS 10
#define OLED_SDIN 11
#define OLED_SCLK 12
#define MPU_PWR_CTRL 13
#define NTC 14
#define MPU_INT 21
#define SCALE2_DOUT 47
#define SCALE2_SCLK 48
#define BUTTON_CIRCLE 1  //33
#define BUTTON_SQUARE 2
#define BUTTON_SET BUTTON_CIRCLE
#define BUTTON_TARE BUTTON_SQUARE
#if defined(TWO_BUTTON) || defined(FOUR_BUTTON)
#define GPIO_NUM_BUTTON_POWER GPIO_NUM_1
#endif
#define BUZZER 38

#define SCALE_PDWN 18
#define SCALE2_PDWN 35
#define SCALE_A0 -1
#define BUZZER_LED 38
#define HX711_SCL 9
#define HX711_SDA 8
#endif

#ifdef V7_2
#define PCB_VER (char*)"PCB: 7.2"
#define ACC_MPU6050
#define HW_SPI
#define SH1106
#define ADS1232ADC
#define ROTATION_180
#define GYROFACEDOWN  //GYRO //#define GYROFACEUP
#define PWR_CTRL 3
#define OLED_CS 10
#define OLED_DC 15
#define OLED_RST 16
#define OLED_SDIN 11
#define OLED_SCLK 12
#define SCALE_DOUT 8
#define SCALE_SCLK 9
#define SCALE_PDWN 13
#define SCALE_A0 -1
#define I2C_SCL 4
#define I2C_SDA 5
#define ROTATION_180

#define BATTERY_PIN 7
#define BATTERY_CHARGING 6  //low is charging
#define BUTTON_SET 1        //33
#define BUTTON_TARE 2
#if defined(TWO_BUTTON) || defined(FOUR_BUTTON)
//#define GPIO_NUM_BUTTON_POWER GPIO_NUM_1
#define GPIO_NUM_BUTTON_POWER (gpio_num_t) BUTTON_SET
#endif

#define BUZZER 38
#define BUZZER_LED 38
#define HX711_SCL 9
#define HX711_SDA 8
#endif

#ifdef V6
#define HW_SPI
#define SH1116
#define ADS1232ADC
#define ROTATION_180
#define GYROFACEDOWN
#define OLED_CS 10
#define OLED_DC 15
#define OLED_RST 16
#define OLED_SDIN 13
#define OLED_SCLK 12
#define SCALE_DOUT 8
#define SCALE_SCLK 9
#define SCALE_PDWN 11
#define SCALE_A0 -1
#define I2C_SCL 4
#define I2C_SDA 5
#define ROTATION_180

#define BATTERY_PIN 7
#define BATTERY_CHARGING 6  //low is charging
#define BUTTON_SET 1        //33
#define BUTTON_TARE 2
#if defined(TWO_BUTTON) || defined(FOUR_BUTTON)
#define GPIO_NUM_BUTTON_POWER GPIO_NUM_1
#endif


#define BUZZER 38
#define BUZZER_LED 38
#define HX711_SCL 9
#define HX711_SDA 8

#endif


#ifdef V5
#define SW_SPI
#define SH1116
#define ADS1232ADC
#define ROTATION_180
#define GYROFACEDOWN
#define OLED_CS 8
#define OLED_DC 15
#define OLED_RST 16
#define OLED_SDIN 6
#define OLED_SCLK 7
#define SCALE_DOUT 9
#define SCALE_SCLK 10
#define SCALE_PDWN 11
#define SCALE_A0 -1
#define I2C_SCL 4
#define I2C_SDA 5


#define BATTERY_PIN 3
#define USB_PIN 12
#define BUTTON_SET 1  //33
#define BUTTON_TARE 2
#if defined(TWO_BUTTON) || defined(FOUR_BUTTON)
#define GPIO_NUM_BUTTON_POWER GPIO_NUM_1
#endif


#define BUZZER 13
#define BUZZER_LED 13
#define HX711_SCL 18
#define HX711_SDA 5

#ifdef FIVE_BUTTON
#define BUTTON_PLUS 26
#define BUTTON_MINUS 25
#define BUTTON_POWER 27
#define GPIO_NUM_BUTTON_POWER GPIO_NUM_27
//#define BUTTON_DEBUG 17
#endif
#endif

#ifdef V0
#define ACC_MPU6050
#define PCB_VER (char*)"PCB: V0"
#define HW_I2C
#define SSD1306
#define HX711ADC
#define GYROFACEUP
#define BATTERY_PIN 36
#define BATTERY_CHARGING 4  //HIGH is charging
#define BUTTON_SET 33  //33
#define BUTTON_TARE 32
#if defined(TWO_BUTTON) || defined(FOUR_BUTTON)
#define GPIO_NUM_BUTTON_POWER GPIO_NUM_33
#endif


#define BUZZER 22
#define BUZZER_LED 27
#define I2C_SCL 19
#define I2C_SDA 23
#define HX711_SCL 18
#define HX711_SDA 5
#define OLED_RST 17

#ifdef FIVE_BUTTON
#define BUTTON_PLUS 26
#define BUTTON_MINUS 25
#define BUTTON_POWER 27
#define GPIO_NUM_BUTTON_POWER GPIO_NUM_27
//#define BUTTON_DEBUG 17
#endif
#endif

#endif
#endif