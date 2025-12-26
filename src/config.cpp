#include "config.h"
#include <Preferences.h>

// Buffer for URL building (accessed by Networking.cpp)
char urlBuffer[URL_BUFFER_SIZE];

// Global Configuration Flags
bool useEAP = true;

// OLED Display Object
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Credential Storage
String EAP_IDENTITY;
String EAP_USERNAME;
String EAP_PASSWORD;
String API_KEY;

void saveCredentials(const char* identity, const char* username, const char* pass, const char* apiKey) {
  Preferences preferences;
  preferences.begin("credentials", false);
  preferences.putString("identity", identity);
  preferences.putString("username", username);
  preferences.putString("password", pass);
  preferences.putString("apiKey", apiKey);
  preferences.end();
}

void loadCredentials() {
  Preferences preferences;
  preferences.begin("credentials", true);
  EAP_IDENTITY = preferences.getString("identity", "");
  EAP_USERNAME = preferences.getString("username", "");
  EAP_PASSWORD = preferences.getString("password", "");
  API_KEY = preferences.getString("apiKey", "");
  preferences.end();
}