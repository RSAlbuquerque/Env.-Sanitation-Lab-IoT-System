#include "ConfigAir.h"
#include "ConfigCommon.h"

#include <AirSensors.h>
#include <Arduino.h>
#include <DebugLog.h>
#include <Display.h>
#include <Networking.h>
#include <Storage.h>
#include <TelnetStream.h>
#include <Types.h>
#include <WiFi.h>

Display display(Config::Display::WIDTH, Config::Display::HEIGHT, Config::Display::RESET_PIN, Config::Display::ADDRESS);

AirSensorsManager AirSensors(Config::Air::Pins::MQ135, Config::Air::Pins::PMS_RX, Config::Air::Pins::PMS_TX,
                             Config::Air::Pins::BME680_ADDR, Config::Air::Pins::BME680_ADDR_FB);

NetworkManager Network(display, Config::Air::Pins::BUTTON, Config::Wifi::EAP_SSID, Config::Wifi::FALLBACK_SSID,
                       Config::Cloud::THINGSPEAK_URL, Config::Wifi::TIMEOUT_MS, Config::Air::FW_URL,
                       Config::Air::VERSION);

DebugLog debugLog();

AirValues currentValues;
UserCredentials credentials;

// Timers
unsigned long currentTime = 0;
unsigned long lastReadTime = 0;
unsigned long lastUpdateCheckTime = 0;
unsigned long lastSendTime = 0;

void startTimers() {
    lastSendTime = millis() - Config::Timers::SEND_INTERVAL;
    lastReadTime = millis() - Config::Timers::READ_INTERVAL;
    lastUpdateCheckTime = millis() - Config::Timers::CHECK_INTERVAL;
}

void warmupSensors() {
    // Used to warmup the MQ-135
    unsigned long startWarmup = millis();
    unsigned long warmupTime = 20000;
    while (millis() - startWarmup < warmupTime) {
        Network.handleInput();
        if ((millis() - startWarmup) % 1000 == 0) {
            Serial.print(".");
        }
        delay(10);
    }
}

void setup() {
    Serial.begin(9600);
    credentials = storage.loadCredentials();

    display.begin(Config::Air::VERSION);
    AirSensors.begin();

    display.debug("Warming Up...");
    warmupSensors();

    Network.begin(credentials);
    Network.connect(false);
    delay(5000);

    // debugLog.setEnabled(true);
    if (debugLog.isEnabled()) {
        display.debug(WiFi.localIP().toString());
        delay(10000);
        TelnetStream.begin();
    }

    startTimers();
}

void loop() {
    currentTime = millis();
    Network.handleInput();

    // --- READ SENSORS ---
    if (currentTime - lastReadTime >= Config::Timers::READ_INTERVAL) {
        currentValues = AirSensors.readAll();

        display.airSensorValues(currentValues);

        lastReadTime = currentTime;
    }

    // --- NETWORK CHECK ---
    if (!Network.isConnected()) {
        Network.connect(true);
    }

    // --- FIRMWARE UPDATES ---
    if (currentTime - lastUpdateCheckTime >= Config::Timers::CHECK_INTERVAL) {
        Network.handleUpdates();
        lastUpdateCheckTime = currentTime;
    }

    // --- SEND DATA ---
    if (currentTime - lastSendTime >= Config::Timers::SEND_INTERVAL && Network.isConnected()) {
        Network.sendAirData(currentValues);

        lastSendTime = currentTime;
    }

    delay(100);
}