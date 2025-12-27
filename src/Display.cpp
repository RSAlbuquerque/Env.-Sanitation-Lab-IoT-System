#include "Display.h"
#include "config.h"

Display display;

Display::Display() 
    : oledReady(false), 
      _currentPage(0), 
      _lastPageSwitch(0) 
{
}

void Display::setup() {
    if(!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) return;

    oledReady = true;
    oled.clearDisplay();
    oled.setRotation(2);
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.display();
}

void Display::clear() {
    if (!oledReady) return;

    oled.clearDisplay();
}

void Display::writeLine(const String& message, uint8_t row) {
    if (!oledReady) return;

    oled.fillRect(0, row * 10, 128, 10, BLACK);
    oled.setCursor(0, row * 10);
    oled.println(message);
    oled.display();
}

void Display::modeSelector() {
    if (!oledReady) return;

    auto message = useEAP ? String("Edu[X] Hts[ ] v") + VERSION
                          : String("Edu[ ] Hts[X] v") + VERSION;
    writeLine(message, MODE_ROW);
}

void Display::waitToStartWM() {
    if (!oledReady) return;

    clear();
    writeLine("WifiManager will", 2);
    writeLine("start in 15 seconds.", 3);
    writeLine("If you wish to cancel", 4);
    writeLine("Press the button now", 5);
}

void Display::wmInstructions() {
    if (!oledReady) return;

    clear();
    writeLine("Network setup!", 0);
    writeLine("Connect to the Wifi:", 1);
    writeLine(String(FALLBACK_SSID_AP) + ". Then,", 2);
    writeLine("open your browser and", 3);
    writeLine("go to " + String(WIFI_MANAGER_IP) + ", to", 4);
    writeLine("complete Wifi setup.", 5);
}

void Display::wifiConnected(const String& ssid) {
    if (!oledReady) return;

    clear();
    writeLine("Wifi: " + ssid, CONNECTED_WIFI_ROW);
}

void Display::hydroSensorValues(const HydroValues& data) {
    if (!oledReady) return;

    float conductivity = data.tds * TDS_TO_COND_REF;
    writeLine("Temp: " + String(data.temperature) + " C", TEMPERATURE_ROW);
    writeLine("TDS: " + String(data.tds) + " ppm", TDS_ROW);
    writeLine("pH: " + String(data.ph), PH_ROW);
    writeLine("Cond: " + String(conductivity) + " uS/cm", CONDUCTIVITY_ROW);
}

void Display::airSensorValues(const AirValues& data) {
    if (!oledReady) return;

    if (millis() - _lastPageSwitch > 3000) {
        _currentPage++;
        if (_currentPage > 2) _currentPage = 0;
        _lastPageSwitch = millis();
        clear();
    }

    // PAGE 0: ENVIRONMENT (BME680)
    if (_currentPage == 0) {
        writeLine("-- ENVIRONMENT --", 0);
        writeLine("Temp: " + String(data.temperature, 1) + " C", 2);
        writeLine("Hum:  " + String(data.humidity, 0) + " %", 3);
        writeLine("Pres: " + String(data.pressure, 0) + " hPa", 4);
        writeLine("VOC:  " + String(data.gasResistance, 0) + " K", 5);
    }

    // PAGE 1: GASES (MQ-135)
    else if (_currentPage == 1) {
        // writeLine("-- GASES (PPM) --", 0);
        writeLine("CO: " + String(data.co) + " | Alc: " + String(data.alcohol), 1);
        writeLine("CO2: " + String(data.co2), 2);
        writeLine("Tol: " + String(data.toluene), 3);
        writeLine("NH4: " + String(data.nh4), 4);
        writeLine("Ace: " + String(data.acetone), 5);
    }

    // PAGE 2: PARTICLES (PMS5003)
    else if (_currentPage == 2) {
        writeLine("-- PARTICLES --", 0);
        writeLine("PM 1.0:  " + String(data.pm1_0) + " ug", 2);
        writeLine("PM 2.5:  " + String(data.pm2_5) + " ug", 3);
        writeLine("PM 10.0: " + String(data.pm10_0) + " ug", 4);
    }
}

void Display::updating(const String& currentVersion, const String& newVersion) {
    if (!oledReady) return;

    clear();
    writeLine("Updating...", 2);
    writeLine("From v" + currentVersion, 3);
    writeLine("To v" + newVersion, 4);
}

void Display::debug(const String& message) {
    if (!oledReady) return;

    clear();
    writeLine(message, 2);
}