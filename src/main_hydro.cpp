#include <Arduino.h>
#include <TelnetStream.h>
#include <WiFi.h>

// Modules
#include "ConfigCommon.h"
#include "ConfigHydro.h"

#include <Display.h>
#include <HydroSensors.h>
#include <Networking.h>
#include <Storage.h>
#include <Types.h>

Display display(Config::Display::WIDTH, Config::Display::HEIGHT, Config::Display::RESET_PIN, Config::Display::ADDRESS);

HydroSensorsManager HydroSensors(Config::Hydro::Pins::TEMP, Config::Hydro::Pins::TDS, Config::Hydro::Pins::PH);

NetworkManager Network(display, Config::Hydro::Pins::BUTTON, Config::Wifi::EAP_SSID, Config::Wifi::FALLBACK_SSID,
                       Config::Cloud::THINGSPEAK_URL, Config::Wifi::TIMEOUT_MS, Config::Hydro::FW_URL,
                       Config::Hydro::VERSION);

// Data Containers
HydroValues currentValues;
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

void setup() {
    Serial.begin(9600);
    credentials = storage.loadCredentials();

    display.begin(Config::Hydro::VERSION);
    HydroSensors.begin();
    Network.begin(credentials);
    delay(1000);

    Network.connect(false);
    delay(5000);

    // display.debug(WiFi.localIP().toString());
    // delay(10000);
    TelnetStream.begin();

    startTimers();
}

void loop() {
    currentTime = millis();
    Network.handleInput();

    // --- READ SENSORS ---
    if (currentTime - lastReadTime >= Config::Timers::READ_INTERVAL) {
        currentValues = HydroSensors.readAll();

        currentValues.conductivity = currentValues.tds * Config::Hydro::TDS_FACTOR;
        display.hydroSensorValues(currentValues);

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
    if (currentTime - lastSendTime >= Config::Timers::SEND_INTERVAL && Network.isConnected() &&
        currentValues.temperature > -100.0) {

        Network.sendHydroData(currentValues);

        lastSendTime = currentTime;
    }

    delay(100);
}