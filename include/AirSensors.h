#ifndef AIR_SENSORS_H
#define AIR_SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "PMS.h" 
#include <MQUnifiedsensor.h>

#include "Types.h"

#define BOARD_TYPE "ESP-32" 
#define VOLTAGE_RESOLUTION 3.3
#define ADC_BIT_RESOLUTION 12
#define RATIO_MQ135_CLEAN_AIR 3.6

class AirSensorsManager {
public:
    AirSensorsManager(int mqPin, int pmsRxPin, int pmsTxPin);
    void begin();
    AirValues readAll();

private:
    int _mqPin;
    int _pmsRx;
    int _pmsTx;
    float _mqRZero = 10.0;

    AirValues _data;
    
    MQUnifiedsensor* _mq135;
    Adafruit_BME680 bme;
    HardwareSerial _pmsSerial;
    PMS _pms;

    void readMQ135(AirValues &data);
    void readBME680(AirValues &data);
    void readPMS5003(AirValues &data);
};

extern AirSensorsManager AirSensors;

#endif