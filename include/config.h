#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

constexpr const char* VERSION = "1.2.0";

// Pins
constexpr int TEMPERATURE_PIN = 4;
constexpr int BUTTON_PIN = 23;
constexpr int TDS_PIN = 34;
constexpr int PH_PIN = 36;
constexpr int OLED_SDA = 21;
constexpr int OLED_SCL = 22;

// Timeouts & Intervals
constexpr unsigned long READ_INTERVAL = 20000UL; // 20 seconds
constexpr unsigned long SEND_INTERVAL = 15 * 60 * 1000UL; // 15 minutes
constexpr unsigned long UPDATE_CHECK_INTERVAL = 24 * 60 * 60 * 1000UL; // 24 hours
constexpr int WIFI_TIMEOUT = 15000; // 15 seconds

// Wifi credentials
constexpr const char* EAP_SSID = "eduroam";
extern bool useEAP;
extern String EAP_IDENTITY;
extern String EAP_USERNAME;
extern String EAP_PASSWORD;
constexpr const char* FALLBACK_SSID_AP = "LSA-Wifi-Setup";
constexpr const char* WIFI_MANAGER_IP = "192.168.4.1";

// ThingSpeak configuration
extern String API_KEY;
constexpr const char* THINGSPEAK_URL = "https://api.thingspeak.com/update";

// Networking Conversions
constexpr float TDS_TO_COND_REF = 1.56f;

// Buffer for URL (Used in Networking.cpp)
constexpr size_t URL_BUFFER_SIZE = 200;
extern char urlBuffer[URL_BUFFER_SIZE];

// OLED Display setup
constexpr int OLED_RESET = -1;
constexpr int OLED_ADDRESS = 0x3C;
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
extern Adafruit_SSD1306 oled;

// OLED Display rows
constexpr int MODE_ROW = 0;
constexpr int CONNECTED_WIFI_ROW = 1;
constexpr int TEMPERATURE_ROW = 3;
constexpr int TDS_ROW = 4;
constexpr int PH_ROW = 5;
constexpr int CONDUCTIVITY_ROW = 6;

// OTA Updates
constexpr const char* FIRMWARE_URL = "https://pub-f48ea50e587f4657ae2f43c545374bd1.r2.dev";

// Save and Load sensitive data to/from flash memory
void saveCredentials(const char* identity, const char* username, const char* pass, const char* apiKey);
void loadCredentials();