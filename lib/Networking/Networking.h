#ifndef NETWORKING_H
#define NETWORKING_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <Update.h>
#include <Preferences.h>
#include "Types.h" 
#include "Display.h"

class NetworkManager {
public:
    NetworkManager(Display& display, int buttonPin, 
                   const char* eapSsid, const char* fallbackSsid, 
                   const char* thingspeakUrl, unsigned long wifiTimeout,
                   String firmwareUrl, String version);

    void begin(UserCredentials &creds);
    void handleInput();

    void connect(bool reconnect);
    void sendHydroData(const HydroValues& data);
    void sendAirData(const AirValues& data);
    
    void handleUpdates();
    bool isConnected();
    
    bool usingEAP() const { return _useEAP; }

private:
    Display& _display; // Reference to the Display object

    const char* _eapSsid;
    const char* _fallbackSsid;
    const char* _thingspeakUrl;
    unsigned long _wifiTimeout;

    String _firmwareUrl;
    String _currentVersion;
    int _buttonPin;
    
    WiFiManager _wm;
    UserCredentials _creds; 
    bool _useEAP;

    static constexpr const char* WM_IP = "192.168.4.1";

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