#include "Display.h"
#include "config.h"

Display display;

Display::Display() : oledReady(false) {}

void Display::setup() {
    if(!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) return;

    oledReady = true;
    oled.clearDisplay();
    oled.setRotation(2);
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
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

void Display::sensorValues(float temperatureC, float tds, float pH) {
    if (!oledReady) return;

    writeLine("Temp: " + String(temperatureC) + " C", TEMPERATURE_ROW);
    writeLine("TDS: " + String(tds) + " ppm", TDS_ROW);
    writeLine("pH: " + String(pH), PH_ROW);
    writeLine("Cond: " + String(tds*TDS_TO_COND_REF) + " uS/cm", CONDUCTIVITY_ROW);
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