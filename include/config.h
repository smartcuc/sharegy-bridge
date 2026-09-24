#pragma once

#include <Arduino.h>

// ==========================================
// ⚡ SHAREGY WAVESHARE ESP32-S3 PIN DEFINITIONS
// ==========================================

// 8x Digital Outputs (Relays RO1 - RO8)
#define PIN_RO1 1
#define PIN_RO2 2
#define PIN_RO3 41
#define PIN_RO4 42
#define PIN_RO5 45
#define PIN_RO6 46
#define PIN_RO7 47
#define PIN_RO8 48

// 8x Digital Inputs (Optocoupled DI1 - DI8)
#define PIN_DI1 4
#define PIN_DI2 5
#define PIN_DI3 6
#define PIN_DI4 7
#define PIN_DI5 15
#define PIN_DI6 16
#define PIN_DI7 17
#define PIN_DI8 18

// RS485 (UART2)
#define PIN_RS485_TX 14
#define PIN_RS485_RX 13
#define PIN_RS485_DE 21

// CAN Bus (TWAI)
#define PIN_CAN_TX 8
#define PIN_CAN_RX 9

// Status & Alarm LEDs / Buzzer
#define PIN_STATUS_LED 38

// Ethernet W5500 SPI Pins
#define PIN_ETH_MISO 12
#define PIN_ETH_MOSI 11
#define PIN_ETH_SCLK 10
#define PIN_ETH_CS   39
#define PIN_ETH_INT  40
#define PIN_ETH_RST  -1

// ==========================================
// ⚙️ DEFAULT TIMINGS & AUTH
// ==========================================
#define TELEMETRY_INTERVAL_MS 2000
#define WATCHDOG_TIMEOUT_MS   8000
#define CLOUD_FAILSAFE_TIMEOUT_MS 60000

#define DEFAULT_WIFI_PASS     "sharegy!26B"
#define DEFAULT_WEB_USER      "admin"
#define DEFAULT_WEB_PASS      "sharegy!26B"
#define BRIDGE_FIRMWARE_VERSION "1.0.0"
#define BRIDGE_HARDWARE_MODEL   "Waveshare ESP32-S3 Industrial PoE 8DI/8RO"
#define DEFAULT_MQTT_BROKER   "mqtt.sharegy.de"
#define DEFAULT_MQTT_PORT     8883

extern char deviceId[32];
extern char hostName[32];
extern bool ethConnected;
extern String ethIp;
extern bool wifiRadioEnabled;
