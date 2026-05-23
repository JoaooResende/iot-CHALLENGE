#pragma once


#define WIFI_SSID     "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL  6


#define PIN_SERVO         19
#define PIN_BTN_OPEN      33
#define PIN_BTN_CLOSE     25
#define PIN_BUZZER        32
#define PIN_LED_OPEN      26
#define PIN_LED_CLOSED    27
#define PIN_DHT           18

#define LCD_ADDR  0x27
#define LCD_COLS  16
#define LCD_ROWS  2


#define SERVO_ANGLE_OPEN    170
#define SERVO_ANGLE_CLOSED  10
#define SERVO_STEP_MS       15   // ms entre cada grau de movimento (não bloqueante)


#define INTERVAL_AUTO_FEED     20000UL   // 20s em demo; em prod seria horas
#define INTERVAL_DOOR_OPEN     8000UL    // porta fica aberta por 8s
#define INTERVAL_ALARM         3000UL    // buzzer 3s antes de fechar
#define INTERVAL_BTN_DEBOUNCE  250UL
#define INTERVAL_DHT_READ      5000UL


#define FOOD_LEVEL_INITIAL   100
#define FOOD_LEVEL_LOW       25


#define LOG_MAX      10
#define LOG_MSG_LEN  48
#define LOG_TIME_LEN 10
