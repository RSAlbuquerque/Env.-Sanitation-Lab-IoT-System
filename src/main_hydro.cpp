#include <Arduino.h>

#include "Networking.h"
#include "Sensors.h"
#include "config.h"
#include "Display.h"

// Last execution timestamps
unsigned long currentTime = 0;
unsigned long lastReadTime = 0;
unsigned long lastUpdateCheckTime = 0;
unsigned long lastSendTime = 0;

// Sensor values
float temperatureC;
float tds;
float pH;

void startTimers() {
  lastSendTime = millis() - SEND_INTERVAL;
  lastReadTime = millis() - READ_INTERVAL;
  lastUpdateCheckTime = millis() - UPDATE_CHECK_INTERVAL;
}

void firmwareUpdate() {
  if (currentTime - lastUpdateCheckTime >= UPDATE_CHECK_INTERVAL) {
    lastUpdateCheckTime = currentTime;

    if (checkNewUpdate())
      updateFirmware();
  } 
}

void readData() {
  if (currentTime - lastReadTime >= READ_INTERVAL) {
    temperatureC = readTemperature();
    tds = readTDS(temperatureC);
    pH = readPH();

    display.sensorValues(temperatureC, tds, pH);

    lastReadTime = currentTime;
  }
}

void sendData() {
  if (currentTime - lastSendTime >= SEND_INTERVAL
      && isWifiConnected() && isTemperatureValid(temperatureC)) {
    lastSendTime = currentTime;
    sendThingSpeakData(temperatureC, tds, pH, API_KEY.c_str());
  }
}

void setup() {
  Serial.begin(9600);
  loadCredentials();
  delay(3000);
  setupSensors();
  connectWifi(false);
  startTimers();
}

void loop() {
  checkButtonState();
  currentTime = millis();
  readData();

  if (!isWifiConnected())
    connectWifi(true);

  firmwareUpdate();
  sendData();
  delay(5000);
}