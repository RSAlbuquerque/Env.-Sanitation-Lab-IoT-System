#ifndef NETWORKING_H
#define NETWORKING_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <Update.h>
#include <Preferences.h>

class NetworkManager {
public:
    NetworkManager(int buttonPin, String apiKey, String firmwareUrl, String version);

    void begin();
    void handleInput();

    void connect(bool reconnect);
    void sendHydroData(float temp, float tds, float ph);
    void handleUpdates();
    
    bool isConnected();

private:
    String _apiKey;
    String _firmwareUrl;
    String _currentVersion;
    WiFiManager _wm;
    int _buttonPin;

    void connectEduroam(bool reconnect);
    void connectWifiManager(bool reconnect);
    bool attemptLastNetwork(bool reconnect);
    void connectionTimer(bool reconnect);
    
    bool checkNewUpdate(String &newVersion);
    void updateFirmware();
    int downloadFirmware(HTTPClient& http);
    bool installFirmware(HTTPClient& http, int contentLength);

    bool checkButtonInterrupt();
};

extern NetworkManager Network;

#endif