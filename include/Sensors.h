#pragma once

#include <Arduino.h>

void setupSensors();
bool isTemperatureValid(float temperatureC);
float readTemperature();
float readTDS(float temperatureC);
float readPH();
bool checkButtonState();