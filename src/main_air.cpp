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

AirSensorsManager AirSensors(Config::Air::Pins::MQ135, Config::Air::Pins::BME680_ADDR,
                             Config::Air::Pins::BME680_ADDR_FB);

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
bool calibrateMQ135 = false;

void startTimers() {
    lastSendTime = millis() - Config::Timers::SEND_INTERVAL;
    lastReadTime = millis() - Config::Timers::READ_INTERVAL;
    lastUpdateCheckTime = millis() - Config::Timers::CHECK_INTERVAL;
}

void setup() {
    Serial.begin(9600);
    credentials = storage.loadCredentials();

    display.begin(Config::Air::VERSION);

    Debug.info("Warming up sensors, reads won't be available for %d seconds.", Config::Air::SENSOR_WARMUP_MS / 1000);
    warmupStartTime = millis();
    display.warmingUpAirSensors(Config::Air::SENSOR_WARMUP_MS / 1000);

    Network.begin(credentials);
    Network.connect(false);
    delay(5000);

    startTimers();
}

void loop() {
    currentTime = millis();
    Network.handleInput();

    // --- SENSOR WARMUP ---
    if (!sensorsWarmedUp) {
        if (currentTime - warmupStartTime >= Config::Air::SENSOR_WARMUP_MS) {
            sensorsWarmedUp = true;
            AirSensors.begin();
            Debug.info("Sensors warmed up, starting readings.");
        } else {
            int secondsLeft = (Config::Air::SENSOR_WARMUP_MS - (currentTime - warmupStartTime)) / 1000;
            display.warmingUpAirSensors(secondsLeft);
        }
    } else {
        display.airSensorValues(currentValues);
    }

    // --- READ SENSORS ---
    if (sensorsWarmedUp && (currentTime - lastReadTime >= Config::Timers::READ_INTERVAL)) {
        currentValues = AirSensors.readAll();

        if (currentValues.isValid()) {
            lastReadTime = currentTime;
            Debug.debug("Valid values are: Temp=%.2f°C Humidity=%.2f%% Pressure=%.2f hPa GasRes=%.2f kOhms CO=%.2f ppm "
                        "CO2=%.2f ppm NH4=%.2f ppm Alcohol=%.2f ppm Acetone=%.2f ppm Toluene=%.2f ppm PM1.0=%.2f µg/m³ "
                        "PM2.5=%.2f µg/m³ PM10=%.2f µg/m³",
                        currentValues.temperature, currentValues.humidity, currentValues.pressure,
                        currentValues.gasResistance, currentValues.co, currentValues.co2, currentValues.nh4,
                        currentValues.alcohol, currentValues.acetone, currentValues.toluene, currentValues.pm1_0,
                        currentValues.pm2_5, currentValues.pm10_0);
        }
    }

    // --- NETWORK CHECK ---
    if (!Network.isConnected()) {
        Network.connect(true);

#if ENABLE_DEBUG
        while (!Network.isConnected()) {
            delay(500);
        }

        display.debug(WiFi.localIP().toString());
        TelnetStream.begin();
        delay(5000);
#endif
    }

    // --- FIRMWARE UPDATES ---
    if (currentTime - lastUpdateCheckTime >= Config::Timers::CHECK_INTERVAL) {
        Network.handleUpdates();

        lastUpdateCheckTime = currentTime;
    }

    // --- SEND DATA ---
    if (sensorsWarmedUp && (currentTime - lastSendTime >= Config::Timers::SEND_INTERVAL) && Network.isConnected()) {
        if (currentValues.isValid()) {
            Network.sendAirData(currentValues);
            lastSendTime = currentTime;
        } else {
            Debug.warn("Current sensor values are invalid, skipping data send.");
        }
    }

    delay(1000);
}