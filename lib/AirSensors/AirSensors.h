#ifndef AIR_SENSORS_H
#define AIR_SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "PMS.h" 
#include <MQUnifiedsensor.h>

#include "Types.h"

class AirSensorsManager {
public:
    AirSensorsManager(int mqPin, int pmsRxPin, int pmsTxPin, uint8_t bmeAddr, uint8_t bmeAddrFb);
    
    void begin();
    AirValues readAll();

private:
    int _mqPin;
    int _pmsRx;
    int _pmsTx;
    uint8_t _bmeAddr;
    uint8_t _bmeAddrFb;
    
    // Sensor Objects
    MQUnifiedsensor* _mq135;
    Adafruit_BME680 _bme;
    HardwareSerial _pmsSerial;
    PMS _pms;

    // Internal Constants
    static constexpr const char* BOARD_TYPE = "ESP-32";
    static constexpr const char* MQ_TYPE = "MQ-135";
    static constexpr float VOLTAGE_RESOLUTION = 3.3;
    static constexpr int   ADC_BIT_RESOLUTION = 12;
    static constexpr float RATIO_MQ135_CLEAN_AIR = 3.6;
    static constexpr int MQ_CALIB_SAMPLES = 10;
    static constexpr int MQ_CORRECTION_FACTOR = 0;

    // Helper functions
    void readMQ135(AirValues &data);
    void readBME680(AirValues &data);
    void readPMS5003(AirValues &data);
};

#endif