#include "Networking.h"
#include "Display.h"      
#include "config.h"       

NetworkManager::NetworkManager(int buttonPin, String apiKey, String firmwareUrl, String version) 
    : _buttonPin(buttonPin), _apiKey(apiKey), _firmwareUrl(firmwareUrl), _currentVersion(version) {}

void NetworkManager::begin() {
    pinMode(_buttonPin, INPUT_PULLUP);
}

void NetworkManager::handleInput() {
    checkButtonInterrupt();
}

bool NetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void NetworkManager::connect(bool reconnect) {
    display.modeSelector();
  
    if (useEAP) {
        connectEduroam(reconnect);
    } else {
        connectWifiManager(reconnect);
    }

    if (isConnected()) {
        display.wifiConnected(WiFi.SSID());
        display.modeSelector();
    }
}

void NetworkManager::connectEduroam(bool reconnect) {
    WiFi.begin(EAP_SSID, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);
    connectionTimer(reconnect);
}

void NetworkManager::connectWifiManager(bool reconnect) {
    if (attemptLastNetwork(reconnect)) return;

    display.waitToStartWM();
    unsigned long start = millis();
    
    while (millis() - start < 15000) {
        if (checkButtonInterrupt()) return;
        delay(500);
    }

    _wm.setConnectTimeout(5);
    _wm.setConfigPortalTimeout(300);

    display.wmInstructions();
    Serial.println("Starting WiFiManager...");

    if (!_wm.autoConnect(FALLBACK_SSID_AP)) {
        Serial.println("WiFiManager failed, rebooting...");
        ESP.restart();
    }

    if (isConnected()) {
        Preferences preferences;
        preferences.begin("wifiCreds", false);
        preferences.putString("ssid", _wm.getWiFiSSID().c_str());
        preferences.putString("pass", _wm.getWiFiPass().c_str());
        preferences.end();
    }
}

bool NetworkManager::attemptLastNetwork(bool reconnect) {
    Preferences preferences;
    if (!preferences.begin("wifiCreds", true)) return false;
    
    String ssid = preferences.getString("ssid", "");
    String pass = preferences.getString("pass", "");
    preferences.end();

    if (ssid.isEmpty()) return false;

    Serial.println("Attempting saved network: " + ssid);
    WiFi.begin(ssid.c_str(), pass.c_str());
    connectionTimer(reconnect);

    return isConnected();
}

void NetworkManager::connectionTimer(bool reconnect) {
    int multiplier = reconnect ? 4 : 1;
    unsigned long startAttemptTime = millis();

    while (!isConnected() && millis() - startAttemptTime < WIFI_TIMEOUT * multiplier) {
        delay(500);
        Serial.print(".");
        
        if (checkButtonInterrupt()) return;
    }
    Serial.println();
}

void NetworkManager::sendHydroData(float temp, float tds, float ph) {
    if (!isConnected()) return;

    float conductivity = tds * TDS_TO_COND_REF; 
    
    snprintf(urlBuffer, URL_BUFFER_SIZE,
             "%s?api_key=%s&field1=%.2f&field2=%.2f&field3=%.2f&field4=%.2f",
             THINGSPEAK_URL, _apiKey.c_str(), temp, tds, ph, conductivity);

    HTTPClient http;
    http.begin(urlBuffer);
    int httpCode = http.GET();
    
    if (httpCode > 0 && httpCode != 200) {
        Serial.printf("ThingSpeak Error: %d\n", httpCode);
    } else if (httpCode <= 0) {
        Serial.printf("Error sending data: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

void NetworkManager::handleUpdates() {
    String newVersion;
    if (checkNewUpdate(newVersion)) {
        display.updating(_currentVersion, newVersion);
        updateFirmware();
    }
}

bool NetworkManager::checkNewUpdate(String &newVersion) {
    if (!isConnected() || _firmwareUrl.isEmpty()) return false;

    HTTPClient http;
    http.begin(_firmwareUrl + "/version.txt");
    int httpCode = http.GET();

    if (httpCode != 200) {
        http.end();
        return false;
    }

    newVersion = http.getString();
    newVersion.trim();
    http.end();

    Serial.println("Current: " + _currentVersion + " | Remote: " + newVersion);
    return newVersion != _currentVersion;
}

void NetworkManager::updateFirmware() {
    if (!isConnected()) return;

    HTTPClient http;
    int contentLength = downloadFirmware(http);
    
    if (contentLength > 0) {
        if (installFirmware(http, contentLength)) {
            ESP.restart();
        }
    }
    http.end();
}

int NetworkManager::downloadFirmware(HTTPClient& http) {
    Serial.println("Downloading firmware...");
    http.begin(_firmwareUrl + "/firmware.bin");
    int httpCode = http.GET();
    
    if (httpCode != 200) {
        Serial.printf("Download Error: %s\n", http.errorToString(httpCode).c_str());
        return -1;
    }
    return http.getSize();
}

bool NetworkManager::installFirmware(HTTPClient& http, int contentLength) {
    Serial.println("Installing...");
    WiFiClient* stream = http.getStreamPtr();

    if (!Update.begin(contentLength)) {
        Serial.println("Not enough space for OTA");
        return false;
    }

    size_t written = Update.writeStream(*stream);
    if (written != contentLength) {
        Serial.println("Write mismatch");
        return false;
    }

    if (Update.end() && Update.isFinished()) {
        Serial.println("Update Success!");
        return true;
    } 

    Serial.printf("Update Error: %s\n", Update.errorString());
    return false;
}

bool NetworkManager::checkButtonInterrupt() {
    if (digitalRead(_buttonPin) == LOW) {
        static unsigned long lastPress = 0;
        if (millis() - lastPress > 200) {
            useEAP = !useEAP;      
            WiFi.disconnect(true); 
            display.clear();       
            
            lastPress = millis();
            return true; 
        }
    }
    return false;
}