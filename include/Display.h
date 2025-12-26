#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class Display {
public:
    Display();
    void setup();
    void clear();
    void modeSelector();
    void waitToStartWM();
    void wmInstructions();
    void wifiConnected(const String& ssid);
    void sensorValues(float temperatureC, float tds, float pH);
    void updating(const String& currentVersion, const String& newVersion);
    void debug(const String& message);

private:
    bool oledReady = false;
    void writeLine(const String& message, uint8_t row);
};

extern Display display;