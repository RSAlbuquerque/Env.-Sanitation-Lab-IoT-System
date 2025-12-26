#ifndef HYDRO_SENSORS_H
#define HYDRO_SENSORS_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <vector>
#include <algorithm> // For std::sort

// Default Constants (can be overridden in constructor)
#define DEFAULT_TDS_VREF 3.3
#define DEFAULT_ADC_RES 4096.0

struct HydroValues {
    float temperature;
    float tds;
    float ph;
};

class HydroSensorsManager {
public:
    HydroSensorsManager(int tempPin, int tdsPin, int phPin);
    
    void begin();
    HydroValues readAll();

    float getTemperature();
    bool isTemperatureValid(int temperatureC);

private:
    int _tempPin;
    int _tdsPin;
    int _phPin;

    const float _vRef = DEFAULT_TDS_VREF;
    const float _adcRes = DEFAULT_ADC_RES;

    OneWire _oneWire;
    DallasTemperature _sensors;

    HydroValues _data;

    float readTDS(float temperatureC);
    float readPH();
    int getMedian(std::vector<int>& data);
};

extern HydroSensorsManager HydroSensors;

#endif