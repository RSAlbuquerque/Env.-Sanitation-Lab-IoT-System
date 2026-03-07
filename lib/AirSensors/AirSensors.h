#ifndef AIR_SENSORS_H
#define AIR_SENSORS_H

#include "Adafruit_BME680.h"
#include "Types.h"

#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <MQUnifiedsensor.h>
#include <SensirionI2cSps30.h>
#include <Wire.h>

class AirSensorsManager {
  public:
    AirSensorsManager(int mqPin, uint8_t bmeAddr, uint8_t bmeAddrFb);

    void begin();
    AirValues readAll();

  private:
    int _mqPin;
    uint8_t _bmeAddr;
    uint8_t _bmeAddrFb;

    // Sensor Objects
    MQUnifiedsensor *_mq135;
    Adafruit_BME680 _bme;
    SensirionI2cSps30 _sps30;

    // Internal Constants
    static constexpr const char *BOARD_TYPE = "ESP-32";
    static constexpr const char *MQ_TYPE = "MQ-135";
    static constexpr float VOLTAGE_RESOLUTION = 3.3;
    static constexpr int ADC_BIT_RESOLUTION = 12;
    static constexpr float RATIO_MQ135_CLEAN_AIR = 3.6;
    static constexpr int MQ_CALIB_SAMPLES = 10;
    static constexpr int MQ_CORRECTION_FACTOR = 0;

    // Helper functions
    void readMQ135(AirValues &data);
    void readBME680(AirValues &data);
    void readSPS30(AirValues &data);
};

#endif