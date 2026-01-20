#include "Networking.h"

#include "DebugLog.h"

#include <TelnetStream.h>

NetworkManager::NetworkManager(Display &display, int buttonPin, const char *eapSsid, const char *fallbackSsid,
                               const char *thingspeakUrl, unsigned long wifiTimeout, String firmwareUrl, String version)
    : _display(display), _buttonPin(buttonPin), _eapSsid(eapSsid), _fallbackSsid(fallbackSsid),
      _thingspeakUrl(thingspeakUrl), _wifiTimeout(wifiTimeout), _firmwareUrl(firmwareUrl), _currentVersion(version),
      _useEAP(true) {}

void NetworkManager::begin(UserCredentials &creds) {
    _creds = creds;
    pinMode(_buttonPin, INPUT_PULLUP);
    Debug.debug("NetworkManager using EAP: %s", _useEAP ? "true" : "false");
}

void NetworkManager::handleInput() { checkButtonInterrupt(); }

bool NetworkManager::isConnected() { return WiFi.status() == WL_CONNECTED; }

void NetworkManager::connect(bool reconnect) {
    Debug.debug("Attempting to connect using %s", _useEAP ? "EAP" : "WiFiManager");
    _display.modeSelector(_useEAP);

    if (_useEAP) {
        connectEduroam(reconnect);
    } else {
        connectWifiManager(reconnect);
    }

    if (isConnected()) {
        _display.wifiConnected(WiFi.SSID());
        _display.modeSelector(_useEAP);
    }
}

void NetworkManager::connectEduroam(bool reconnect) {
    WiFi.begin(_eapSsid, WPA2_AUTH_PEAP, _creds.identity.c_str(), _creds.username.c_str(), _creds.password.c_str());
    connectionTimer(reconnect);
}

void NetworkManager::connectWifiManager(bool reconnect) {
    if (attemptLastNetwork(reconnect))
        return;

    _display.waitToStartWM(15);
    unsigned long start = millis();

    while (millis() - start < 15000) {
        if (checkButtonInterrupt())
            return;
        delay(500);
    }

    _wm.setConnectTimeout(5);
    _wm.setConfigPortalTimeout(300);

    _display.wmInstructions(_fallbackSsid, WM_IP);
    Debug.debug("Starting WiFiManager...");

    if (!_wm.autoConnect(_fallbackSsid)) {
        Debug.error("WiFiManager failed, rebooting...");
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
    if (!preferences.begin("wifiCreds", true))
        return false;

    String ssid = preferences.getString("ssid", "");
    String pass = preferences.getString("pass", "");
    preferences.end();

    if (ssid.isEmpty())
        return false;

    Debug.info("Attempting to connect to last known network: %s", ssid.c_str());
    WiFi.begin(ssid.c_str(), pass.c_str());
    connectionTimer(reconnect);

    return isConnected();
}

void NetworkManager::connectionTimer(bool reconnect) {
    int multiplier = reconnect ? 4 : 1;
    unsigned long startAttemptTime = millis();

    while (!isConnected() && millis() - startAttemptTime < _wifiTimeout * multiplier) {
        delay(500);

        if (checkButtonInterrupt())
            return;
    }
}

void NetworkManager::sendHydroData(const HydroValues &data) {
    if (!isConnected())
        return;

    char urlBuffer[256];

    snprintf(urlBuffer, sizeof(urlBuffer), "%s?api_key=%s&field1=%.2f&field2=%.2f&field3=%.2f&field4=%.2f",
             _thingspeakUrl, _creds.apiKey.c_str(), data.temperature, data.tds, data.ph, data.conductivity);

    HTTPClient http;
    http.begin(urlBuffer);
    int httpCode = http.GET();

    if (httpCode > 0 && httpCode != 200) {
        Debug.error("ThingSpeak Error: %s", http.errorToString(httpCode).c_str());
    } else if (httpCode <= 0) {
        Debug.error("Error sending data: %s", http.errorToString(httpCode).c_str());
    }
    http.end();
}

void NetworkManager::sendAirData(const AirValues &data) {
    if (!isConnected())
        return;

    char urlBuffer[256];

    snprintf(
        urlBuffer, sizeof(urlBuffer),
        "%s?api_key=%s&field1=%.2f&field2=%.2f&field3=%.2f&field4=%.2f&field5=%.2f&field6=%.2f&field7=%.2f&field8=%.2f",
        _thingspeakUrl, _creds.apiKey.c_str(), data.pm2_5, data.pm10_0, data.toluene, data.co, data.co2, data.alcohol,
        data.nh4, data.acetone);

    HTTPClient http;
    http.begin(urlBuffer);
    int httpCode = http.GET();

    if (httpCode > 0 && httpCode != 200) {
        Debug.error("ThingSpeak Error: %s", http.errorToString(httpCode).c_str());
    } else if (httpCode <= 0) {
        Debug.error("Error sending data: %s", http.errorToString(httpCode).c_str());
    }
    http.end();
}

void NetworkManager::handleUpdates() {
    String newVersion;
    if (checkNewUpdate(newVersion)) {
        _display.updating(_currentVersion, newVersion);
        updateFirmware();
    }
}

bool NetworkManager::checkNewUpdate(String &newVersion) {
    if (!isConnected() || _firmwareUrl.isEmpty())
        return false;

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

    Debug.info("Current version: %s, Remote version: %s", _currentVersion.c_str(), newVersion.c_str());
    return newVersion != _currentVersion;
}

void NetworkManager::updateFirmware() {
    if (!isConnected())
        return;

    HTTPClient http;
    int contentLength = downloadFirmware(http);

    if (contentLength > 0) {
        if (installFirmware(http, contentLength)) {
            ESP.restart();
        }
    }
    http.end();
}

int NetworkManager::downloadFirmware(HTTPClient &http) {
    Debug.info("Downloading firmware...");
    http.begin(_firmwareUrl + "/firmware.bin");
    int httpCode = http.GET();

    if (httpCode != 200) {
        Debug.error("Firmware download failed: %s", http.errorToString(httpCode).c_str());
        return -1;
    }
    return http.getSize();
}

bool NetworkManager::installFirmware(HTTPClient &http, int contentLength) {
    Debug.info("Installing new firmware...");
    WiFiClient *stream = http.getStreamPtr();

    if (!Update.begin(contentLength)) {
        Debug.error("Not enough space for OTA");
        return false;
    }

    size_t written = Update.writeStream(*stream);
    if (written != contentLength) {
        Debug.error("Write mismatch: written %d, expected %d", written, contentLength);
        return false;
    }

    if (Update.end() && Update.isFinished()) {
        Debug.info("Update Success!");
        return true;
    }

    Debug.error("Update failed: %s", Update.errorString());
    return false;
}

bool NetworkManager::checkButtonInterrupt() {
    if (digitalRead(_buttonPin) == LOW) {
        static unsigned long lastPress = 0;
        if (millis() - lastPress > 200) {
            _useEAP = !_useEAP;
            WiFi.disconnect(true);
            _display.clear();

            lastPress = millis();
            return true;
        }
    }
    return false;
}