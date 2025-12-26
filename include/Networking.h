#pragma once

#include <Arduino.h>

/**
 * Check if WiFi is currently connected.
 * @return true if connected, false otherwise
 */
bool isWifiConnected();

/**
 * Main WiFi connection function.
 * Based on the state of `useEAP`, it will attempt
 * to connect to eduroam or start WiFiManager.
 * Updates OLED display with connection status.
 */
void connectWifi(bool reconnect);

/**
 * Send sensor data to ThingSpeak cloud platform.
 * @param temperatureC Temperature in Celsius (Field 1)
 * @param tds Total Dissolved Solids value (Field 2)
 * @param pH pH value (Field 3)
 * @param apiKey ThingSpeak API key for authentication (16 characters)
 */
void sendThingSpeakData(float temperatureC, float tds, float pH, const char* apiKey);

/**
 * Check if a new firmware update is available
 * Downloads version.txt from firmware server and compares with current version
 * @return true if new version is available, false otherwise
 */
bool checkNewUpdate();

/**
 * Download and install firmware update via OTA (Over-The-Air)
 * Updates OLED display during the process and reboots device on success
 */
void updateFirmware();