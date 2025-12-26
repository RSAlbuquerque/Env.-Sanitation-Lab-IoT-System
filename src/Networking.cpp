#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <Update.h>
#include <Preferences.h>

#include "Networking.h"
#include "Sensors.h"
#include "Display.h"
#include "config.h"

WiFiManager wm;

bool isWifiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void connectionTimer(bool reconnect) {
  int multiplier = reconnect ? 4 : 1;
  unsigned long startAttemptTime = millis();

  while (!isWifiConnected() && millis() - startAttemptTime < WIFI_TIMEOUT*multiplier) {
      delay(500);
      Serial.print(".");
      if (checkButtonState())
        return;
  }
  Serial.println();

  if (!isWifiConnected()) {
    Serial.println("Failed with error: " + String(WiFi.status()));
  }
}

void connectEduroam(bool reconnect) {
  // In some cases, CA certificates may be needed. In UBI, they are not required.
  WiFi.begin(EAP_SSID, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);
  connectionTimer(reconnect);
}

bool attemptLastNetwork(bool reconnect) {
  Preferences preferences;
  
  if (!preferences.begin("wifiCreds", true)) {
    Serial.println("Failed to open preferences");
    return false;
  }
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("pass", "");
  preferences.end();

  Serial.println(ssid);

  if (ssid.isEmpty() || pass.isEmpty())
    return false;

  WiFi.begin(ssid.c_str(), pass.c_str());
  connectionTimer(reconnect);

  return isWifiConnected();
}

void connectWifiManager(bool reconnect) {
  if (attemptLastNetwork(reconnect))
    return;

  display.waitToStartWM();
  auto start = millis();
  // Wait 15 seconds
  while (millis() - start < 15000) {
    if (checkButtonState())
      return;
    delay(500);
  }

  wm.setConnectTimeout(5);
  wm.setConfigPortalTimeout(300);

  // Write instructions to OLED display
  display.wmInstructions();

  // Starts an access point and a configuration portal
  // Connection is handled automatically by WiFiManager
  Serial.println("Starting WiFiManager...");
  if (!wm.autoConnect(FALLBACK_SSID_AP)) {
    Serial.println("WiFiManager failed, rebooting...");
    ESP.restart();
  }

  // This will be used for reconnection attempts
  if (isWifiConnected()) {
    Preferences preferences;
    preferences.begin("wifiCreds", false);
    preferences.putString("ssid", wm.getWiFiSSID().c_str());
    preferences.putString("pass", wm.getWiFiPass().c_str());
    preferences.end();
  }
}

void connectWifi(bool reconnect) {
  display.modeSelector();
  
  if (useEAP)
    connectEduroam(reconnect);

  if (!useEAP)
    connectWifiManager(reconnect);

  if (isWifiConnected()) {
    display.wifiConnected(WiFi.SSID());
    display.modeSelector();
  }
}

void sendThingSpeakData(float temperatureC, float tds, float pH, const char* apiKey) {
  if (!isWifiConnected())
    return;

  float conductivity = tds * TDS_TO_COND_REF;
  HTTPClient http;
  // Build the URL for the request
  snprintf(urlBuffer, URL_BUFFER_SIZE,
                     "%s?api_key=%s&field1=%.2f&field2=%.2f&field3=%.2f&field4=%.2f",
                     THINGSPEAK_URL,        // URL for ThingSpeak
                     apiKey,                // API key for ThingSpeak
                     temperatureC,          // Temperature reading (field1)
                     tds,                   // TDS reading (field2)
                     pH,                    // pH reading (field3)
                     conductivity);         // Conductivity reading (field4)            

  http.begin(urlBuffer);
  int httpCode = http.GET();
  
  if (httpCode > 0 && httpCode != 200) {
    Serial.print("ThingSpeak returned HTTP code ");
    Serial.println(httpCode);
    Serial.print("Response: ");
    Serial.println(http.getString());
  } 
  else if (httpCode <= 0) {
    Serial.print("Error sending data: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }

  http.end();
}

bool checkNewUpdate() {
  if (!isWifiConnected()) return false;

  if (!FIRMWARE_URL || strlen(FIRMWARE_URL) == 0) {
    Serial.println("FIRMWARE_URL is not set.");
    return false;
  }

  HTTPClient http;
  http.begin(String(FIRMWARE_URL) + "/version.txt");
  int httpCode = http.GET();
  if (httpCode != 200) {
    Serial.print("Error checking for update: ");
    Serial.println(http.errorToString(httpCode).c_str());
    http.end();
    return false;
  }

  String newVersion = http.getString();
  newVersion.trim();
  Serial.println("Current version: " + String(VERSION));
  Serial.println("Latest version: " + newVersion);
  http.end();

  if (newVersion != String(VERSION))
    display.updating(String(VERSION), newVersion);

  return newVersion != String(VERSION);
}

int downloadFirmware(HTTPClient& http) {
  Serial.println("Downloading firmware...");
  http.begin(String(FIRMWARE_URL) + "/firmware.bin");
  int httpCode = http.GET();
  if (httpCode != 200) {
    Serial.print("Error downloading update: ");
    Serial.println(http.errorToString(httpCode).c_str());
    return -1;
  }

  int contentLength = http.getSize();
  if (contentLength <= 0) {
    Serial.println("Invalid content length");
    return -1;
  }

  return contentLength;
}

bool installFirmware(HTTPClient& http, int contentLength) {
  Serial.println("Installing firmware...");
  WiFiClient* stream = http.getStreamPtr();

    if (!Update.begin(contentLength)) {
        Serial.println("Not enough space for OTA");
        return false;
    }

    size_t written = Update.writeStream(*stream);
    if (written != contentLength) {
        Serial.printf("Write mismatch: %d/%d\n", written, contentLength);
        return false;
    }

    if (!Update.end()) {
        Serial.printf("Update error: %s\n", Update.errorString());
        return false;
    }

    if (Update.isFinished()) {
        Serial.println("Update successful! Rebooting...");
        ESP.restart();
        return true;
    }
    else {
        Serial.println("Update not finished?");
        return false;
    }
}

void updateFirmware() {
  if (!isWifiConnected()) return;

  HTTPClient http;
  int contentLength = downloadFirmware(http);
  if (contentLength <= 0) {
    http.end();
    return;
  }

  if (installFirmware(http, contentLength))
    ESP.restart();
  
  http.end();
}