#include "HydroSensors.h"

HydroSensorsManager::HydroSensorsManager(int tempPin, int tdsPin, int phPin)
    : _tempPin(tempPin), 
      _tdsPin(tdsPin), 
      _phPin(phPin), 
      _oneWire(tempPin), 
      _sensors(&_oneWire) 
{
    _data = {0.0, 0.0, 7.0};
}

void HydroSensorsManager::begin() {
    pinMode(_tdsPin, INPUT);
    pinMode(_phPin, INPUT);
    _sensors.begin();
}

HydroValues HydroSensorsManager::readAll() {
    _data.temperature = getTemperature();
    _data.tds = readTDS(_data.temperature);
    _data.ph = readPH();

    return _data;
}

float HydroSensorsManager::getTemperature() {
    _sensors.requestTemperatures();
    float tempC = _sensors.getTempCByIndex(0);

    if (isTemperatureValid(tempC)) {
        return -1.0;
    }
    return tempC;
}

float HydroSensorsManager::readTDS(float temperatureC) {
    std::vector<int> analogBuffer;
    for (int i = 0; i < FILTER_SAMPLES; i++) {
        analogBuffer.push_back(analogRead(_tdsPin));
        delay(2);
    }

    float averageVoltage = (getMedian(analogBuffer)) * V_REF / ADC_RES;
    
    float safeTemp = (isTemperatureValid(temperatureC)) ? 25.0f : temperatureC;
    float compensationVoltage = averageVoltage / (1.0f + 0.02f * (safeTemp - 25.0f));
    
    float tdsValue = (133.42f * pow(compensationVoltage, 3) 
                     - 255.86f * pow(compensationVoltage, 2) 
                     + 857.39f * compensationVoltage) * 0.5f;

    return tdsValue;
}

float HydroSensorsManager::readPH() {
    float voltageSum = 0;

    for (int i = 0; i < PH_READS; i++) {
        float raw = analogRead(_phPin);
        voltageSum += (raw * V_REF) / 4095.0f;
        delay(10);
    }
    
    float avgVoltage = voltageSum / PH_READS;
    return 7 + (((V_REF / 2) - avgVoltage) / 0.1841f);
}

bool HydroSensorsManager::isTemperatureValid(int temperatureC) {
    return (temperatureC == DEVICE_DISCONNECTED_C || temperatureC > 84.0f);
}

int HydroSensorsManager::getMedian(std::vector<int>& data) {
    if (data.empty()) return 0;
  
    std::sort(data.begin(), data.end());

    size_t size = data.size();
    if (size % 2 == 0) {
        return (data[size / 2 - 1] + data[size / 2]) / 2;
    } else {
        return data[size / 2];
    }
}