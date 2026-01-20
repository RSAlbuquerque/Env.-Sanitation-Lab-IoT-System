#include "ConfigAir.h"
#include "ConfigCommon.h"
#include "DebugLog.h"

#include <AirSensors.h>
#include <Arduino.h>
#include <DebugLog.h>
#include <Display.h>
#include <Networking.h>
#include <Storage.h>
#include <TelnetStream.h>
#include <Types.h>
#include <WiFi.h>

DebugLog Debug;

Display display(Config::Display::WIDTH, Config::Display::HEIGHT, Config::Display::RESET_PIN, Config::Display::ADDRESS);

AirSensorsManager AirSensors(Config::Air::Pins::MQ135, Config::Air::Pins::PMS_RX, Config::Air::Pins::PMS_TX,
                             Config::Air::Pins::BME680_ADDR, Config::Air::Pins::BME680_ADDR_FB);

NetworkManager Network(display, Config::Air::Pins::BUTTON, Config::Wifi::EAP_SSID, Config::Wifi::FALLBACK_SSID,
                       Config::Cloud::THINGSPEAK_URL, Config::Wifi::TIMEOUT_MS, Config::Air::FW_URL,
                       Config::Air::VERSION);

AirValues currentValues;

UserCredentials credentials;

// Timers
unsigned long currentTime = 0;
unsigned long lastReadTime = 0;
unsigned long lastUpdateCheckTime = 0;
unsigned long lastSendTime = 0;
unsigned long warmupStartTime = 0;
bool sensorsWarmedUp = false;

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
        delay(10);
    }
}

void setup() {
    Serial.begin(9600);
    credentials = storage.loadCredentials();

    display.begin(Config::Air::VERSION);
    AirSensors.begin();

    Debug.info("Warming up sensors, reads won't be available for %d seconds.", Config::Air::SENSOR_WARMUP_MS / 1000);

    Network.begin(credentials);
    Network.connect(false);
    delay(5000);

    display.debug(WiFi.localIP().toString());
    TelnetStream.begin();
    delay(5000);
#endif

    startTimers();
}

void loop() {
    currentTime = millis();
    Network.handleInput();

    // --- SENSOR WARMUP ---
    if (!sensorsWarmedUp) {
        if (currentTime - warmupStartTime >= Config::Air::SENSOR_WARMUP_MS) {
            sensorsWarmedUp = true;
            Debug.info("Sensors warmed up, starting readings.");
        }
    }

    // --- READ SENSORS ---
    if (sensorsWarmedUp && (currentTime - lastReadTime >= Config::Timers::READ_INTERVAL)) {
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
    if (sensorsWarmedUp && (currentTime - lastSendTime >= Config::Timers::SEND_INTERVAL) && Network.isConnected()) {
        Network.sendAirData(currentValues);

        lastSendTime = currentTime;
    }

    delay(100);
}