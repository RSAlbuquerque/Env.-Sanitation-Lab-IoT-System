#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Types.h"

class Display {
public:
    Display();
    void setup();
    void clear();
    void modeSelector();
    void waitToStartWM();
    void wmInstructions();
    void wifiConnected(const String& ssid);
    void hydroSensorValues(const HydroValues& hydroData);
    void airSensorValues(const AirValues& airData);
    void updating(const String& currentVersion, const String& newVersion);
    void debug(const String& message);

private:
    bool oledReady = false;
    void writeLine(const String& message, uint8_t row);

    int _currentPage;
    unsigned long _lastPageSwitch;
};

extern Display display;