#ifndef HYDRO_SENSORS_H
#define HYDRO_SENSORS_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <vector>
#include <algorithm>

#include "Types.h"


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

    static constexpr float V_REF = 3.3;
    static constexpr float ADC_RES = 4096.0;
    static constexpr int FILTER_SAMPLES = 20;
    static constexpr int PH_READS = 20;

    OneWire _oneWire;
    DallasTemperature _sensors;

    HydroValues _data;

    float readTDS(float temperatureC);
    float readPH();
    int getMedian(std::vector<int>& data);
};

extern HydroSensorsManager HydroSensors;

#endif