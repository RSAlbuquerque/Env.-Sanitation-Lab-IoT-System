#include <cstdint>
#include <WiFi.h>

#include "config.h"
#include "Display.h"

bool oledReady = false;

int getMedianNum(const int* bArray){
  std::array<int, FILTER_SAMPLE_COUNT> bTab;
  for (uint8_t i = 0; i<FILTER_SAMPLE_COUNT; i++)
    bTab[i] = bArray[i];
  
  int bTemp;
  for (u_int8_t j = 0; j < FILTER_SAMPLE_COUNT - 1; j++) {
    for (u_int8_t i = 0; i < FILTER_SAMPLE_COUNT - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }

  if ((FILTER_SAMPLE_COUNT & 1) > 0){
    bTemp = bTab[(FILTER_SAMPLE_COUNT - 1) / 2];
  }
  else {
    bTemp = (bTab[FILTER_SAMPLE_COUNT / 2] + bTab[FILTER_SAMPLE_COUNT / 2 - 1]) / 2;
  }

  return bTemp;
}

void setupSensors() {
  display.setup();
  sensors.begin();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(TDS_PIN, INPUT);
  pinMode(PH_PIN, INPUT);
}

bool isTemperatureValid(float temperatureC) {
  return temperatureC != DEVICE_DISCONNECTED_C && temperatureC < 84.0f;
}

float readTemperature() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}

float readTDS(float temperatureC) {
  for (int i = 0; i < FILTER_SAMPLE_COUNT; i++)
  {
    tdsAnalogBuffer[i] = analogRead(TDS_PIN);
    delay(2);
  }

  float averageVoltage = (getMedianNum(tdsAnalogBuffer.data())) * VREF / 4096.0f;
  float compensationVoltage = averageVoltage / (1.0f + 0.02f * (temperatureC - 25.0f));
  return ((133.42f * (compensationVoltage * compensationVoltage * compensationVoltage) - 255.86f
          * (compensationVoltage * compensationVoltage) + 857.39f * compensationVoltage) * 0.5f);
}

float readPH() {
  float voltageSum = 0;

  for (int i = 0; i < PH_NUM_READS; i++) {
    float raw = analogRead(PH_PIN);
    voltageSum += (raw * VREF) / 4095.0f;
    delay(100);
  }
  
  return 7 + (((VREF / 2) - (voltageSum / PH_NUM_READS)) / 0.1841f);
}

bool checkButtonState() {
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && currentButtonState == LOW) {
    useEAP = !useEAP;
    WiFi.disconnect(true);
    display.clear();
    delay(50); // Debounce delay
    return true;
  }

  lastButtonState = currentButtonState;
  return false;
}