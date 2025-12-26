#include <Arduino.h>
#include <WiFi.h>

#include "Networking.h"
#include "HydroSensors.h"
#include "config.h"
#include "Display.h"

HydroSensorsManager HydroSensors(TEMPERATURE_PIN, TDS_PIN, PH_PIN);
NetworkManager Network(BUTTON_PIN, API_KEY, FIRMWARE_URL, VERSION);

HydroValues currentValues;

unsigned long currentTime = 0;
unsigned long lastReadTime = 0;
unsigned long lastUpdateCheckTime = 0;
unsigned long lastSendTime = 0;

void startTimers() {
  lastSendTime = millis() - SEND_INTERVAL;
  lastReadTime = millis() - READ_INTERVAL;
  lastUpdateCheckTime = millis() - UPDATE_CHECK_INTERVAL;
}

void setup() {
  Serial.begin(115200);
  loadCredentials();

  display.setup();
  HydroSensors.begin();
  Network.begin();
  delay(1000);

  Network.connect(false);
  startTimers();
}

void loop() {
  currentTime = millis();
  Network.handleInput(); // Check for button presses

  // Read sensor data
  if (currentTime - lastReadTime >= READ_INTERVAL) {
    currentValues = HydroSensors.readAll();
    display.sensorValues(currentValues.temperature, currentValues.tds, currentValues.ph);
    lastReadTime = currentTime;
  }

  // Ensure WiFi connection
  if (!Network.isConnected()) {
    Network.connect(true);
  }

  // Firmware update check
  if (currentTime - lastUpdateCheckTime >= UPDATE_CHECK_INTERVAL) {
    Network.handleUpdates();
    lastUpdateCheckTime = currentTime;
  }

  // Send data to server
  if (currentTime - lastSendTime >= SEND_INTERVAL 
      && Network.isConnected() 
      && currentValues.temperature > -100.0) {
    
    Network.sendHydroData(currentValues.temperature, currentValues.tds, currentValues.ph);
    lastSendTime = currentTime;
  }

  delay(100); 
}