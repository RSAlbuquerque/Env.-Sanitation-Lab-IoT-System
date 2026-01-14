#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "Types.h"

class Display {
public:
    Display(uint8_t w, uint8_t h, uint8_t resetPin, uint8_t i2cAddr);

    bool begin(String firmwareVersion);

    void clear();
    
    void modeSelector(bool useEAP);
    
    void waitToStartWM(int seconds);
    void wmInstructions(String ssid, String ip);
    void wifiConnected(const String& ssid);

    void hydroSensorValues(const HydroValues& data);
    void airSensorValues(const AirValues& data);
    
    void updating(const String& currentVersion, const String& newVersion);
    void debug(const String& message);

private:
    Adafruit_SSD1306 _oled;
    
    String _version;        
    uint8_t _i2cAddr;       
    bool _oledReady;
    bool _useEAP;
    String _displayedSSID;
    
    uint8_t _currentPage;
    unsigned long _lastPageSwitch;
    void writeLine(const String& message, uint8_t row);

    struct Layout {
        static constexpr uint8_t ROW_HEIGHT = 10;

        // --- Common Layout ---
        static constexpr uint8_t ROW_MODE   = 0;
        static constexpr uint8_t ROW_WIFI   = 1;

        // --- Hydro Layout ---
        static constexpr uint8_t ROW_TEMP   = 2;
        static constexpr uint8_t ROW_TDS    = 3;
        static constexpr uint8_t ROW_PH     = 4;
        static constexpr uint8_t ROW_COND   = 5;
        
        // --- Air Layout ---
        
        // Page 0: Environment
        static constexpr uint8_t ROW_ENV_TEMP    = 2;
        static constexpr uint8_t ROW_ENV_HUM     = 3;
        static constexpr uint8_t ROW_ENV_PRES    = 4;
        static constexpr uint8_t ROW_ENV_VOC     = 5;

        // Page 1: Gases (Part 1)
        static constexpr uint8_t ROW_GAS_CO      = 2;
        static constexpr uint8_t ROW_GAS_ALC     = 3;
        static constexpr uint8_t ROW_GAS_CO2     = 4;
        static constexpr uint8_t ROW_GAS_ADC_MQ135 = 5;

        // Page 2: Gases (Part 2)
        static constexpr uint8_t ROW_GAS_NH4     = 2;
        static constexpr uint8_t ROW_GAS_ACE     = 3;
        static constexpr uint8_t ROW_GAS_TOL     = 4;

        // Page 3: Particles
        static constexpr uint8_t ROW_PM_1_0      = 2;
        static constexpr uint8_t ROW_PM_2_5      = 3;
        static constexpr uint8_t ROW_PM_10_0     = 4;
    };
};

#endif