#include "Display.h"

// Constructor: Initialize the internal Adafruit Object
Display::Display(uint8_t w, uint8_t h, uint8_t resetPin, uint8_t i2cAddr) 
    : _oled(w, h, &Wire, resetPin),
      _i2cAddr(i2cAddr),
      _oledReady(false), 
      _currentPage(0), 
      _lastPageSwitch(0) 
{
}

bool Display::begin(String firmwareVersion) {
    _version = firmwareVersion;

    if(!_oled.begin(SSD1306_SWITCHCAPVCC, _i2cAddr)) {
        return false;
    }

    _oledReady = true;
    _oled.clearDisplay();
    _oled.setRotation(2);
    _oled.setTextSize(1);
    _oled.setTextColor(SSD1306_WHITE);
    _oled.display();
    return true;
}

void Display::clear() {
    if (!_oledReady) return;
    _oled.clearDisplay();
}

void Display::writeLine(const String& message, uint8_t row) {
    if (!_oledReady) return;
    int16_t y = row * Layout::ROW_HEIGHT;
    
    _oled.fillRect(0, y, 128, Layout::ROW_HEIGHT, SSD1306_BLACK);
    _oled.setCursor(0, y);
    _oled.println(message);
    _oled.display();
}

void Display::modeSelector(bool useEAP) {
    if (!_oledReady) return;
    _useEAP = useEAP;

    // Use the passed 'useEAP' boolean and the stored '_version' string
    auto message = useEAP ? String("Edu[X] Hts[ ] v") + _version
                          : String("Edu[ ] Hts[X] v") + _version;
    
    writeLine(message, Layout::ROW_MODE);
}

void Display::waitToStartWM(int seconds) {
    if (!_oledReady) return;
    clear();
    writeLine("WifiManager will", 2);
    writeLine("start in " + String(seconds) + "s", 3);
    writeLine("Press button to", 4);
    writeLine("CANCEL", 5);
}

void Display::wmInstructions(String ssid, String ip) {
    if (!_oledReady) return;
    clear();
    writeLine("Network Setup!", 0);
    writeLine("Connect Wifi:", 1);
    writeLine(ssid, 2);
    writeLine("Go to:", 3);
    writeLine(ip, 4);
}

void Display::wifiConnected(const String& ssid) {
    if (!_oledReady) return;
    clear();

    _displayedSSID = ssid;
    if (ssid.length() > 14) {
        _displayedSSID = ssid.substring(0, 11) + "...";
    }

    writeLine("Wifi: " + _displayedSSID, Layout::ROW_WIFI);
}

void Display::hydroSensorValues(const HydroValues& data) {
    if (!_oledReady) return;

    writeLine("Temp: " + String(data.temperature) + " C", Layout::ROW_TEMP);
    writeLine("TDS: " + String(data.tds) + " ppm", Layout::ROW_TDS);
    writeLine("pH: " + String(data.ph), Layout::ROW_PH);
    writeLine("Cond: " + String(data.conductivity) + " uS", Layout::ROW_COND);
}

void Display::airSensorValues(const AirValues& data) {
    if (!_oledReady) return;

    if (millis() - _lastPageSwitch > 3000) {
        _currentPage++;
        if (_currentPage > 3) _currentPage = 0; // Reset after Page 3
        _lastPageSwitch = millis();
        
        clear();
        modeSelector(_useEAP); // Redraw Header
        writeLine("Wifi: " + _displayedSSID, Layout::ROW_WIFI); // Redraw Wifi
    }

    // PAGE 0: ENVIRONMENT
    if (_currentPage == 0) {
        writeLine("Temp: " + String(data.temperature, 1) + " C",   Layout::ROW_ENV_TEMP);
        writeLine("Hum:  " + String(data.humidity, 0) + " %",      Layout::ROW_ENV_HUM);
        writeLine("Pres: " + String(data.pressure, 0) + " hPa",    Layout::ROW_ENV_PRES);
        writeLine("VOC:  " + String(data.gasResistance, 0) + " K", Layout::ROW_ENV_VOC);
    }
    // PAGE 1: GASES (Part 1)
    else if (_currentPage == 1) {
        writeLine("CO:  " + String(data.co),      Layout::ROW_GAS_CO);
        writeLine("Alc: " + String(data.alcohol), Layout::ROW_GAS_ALC);
        writeLine("CO2: " + String(data.co2),     Layout::ROW_GAS_CO2);
    }
    // PAGE 2: GASES (Part 2)
    else if (_currentPage == 2) {
        writeLine("NH4: " + String(data.nh4),     Layout::ROW_GAS_NH4);
        writeLine("Ace: " + String(data.acetone), Layout::ROW_GAS_ACE);
        writeLine("Tol: " + String(data.toluene), Layout::ROW_GAS_TOL);
        writeLine("ADC: " + String(data.adc_mq135), Layout::ROW_GAS_ADC_MQ135);
    }
    // PAGE 3: PARTICLES
    else if (_currentPage == 3) {
        writeLine("PM 1.0:  " + String(data.pm1_0) + " ug",  Layout::ROW_PM_1_0);
        writeLine("PM 2.5:  " + String(data.pm2_5) + " ug",  Layout::ROW_PM_2_5);
        writeLine("PM 10.0: " + String(data.pm10_0) + " ug", Layout::ROW_PM_10_0);
    }
}

void Display::updating(const String& currentVersion, const String& newVersion) {
    if (!_oledReady) return;
    clear();
    writeLine("Updating...", 2);
    writeLine("v" + currentVersion + " -> v" + newVersion, 3);
}

void Display::debug(const String& message) {
    if (!_oledReady) return;
    clear();
    writeLine(message, 2);
}