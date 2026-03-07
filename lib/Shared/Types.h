#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

struct UserCredentials {
    String identity;
    String username;
    String password;
    String apiKey;
    bool valid;
};

struct HydroValues {
    float temperature;
    float tds;
    float ph;
    float conductivity;
};

struct AirValues {
    // MQ-135 Values
    float adc_mq135;
    float co;
    float co2;
    float nh4;
    float alcohol;
    float acetone;
    float toluene;

    // BME680 Values
    float temperature;
    float humidity;
    float pressure;
    float gasResistance;

    // SPS30 Values
    float pm1_0;
    float pm2_5;
    float pm10_0;

    bool isValid() const {
        return !isnan(pm2_5) && pm2_5 >= 0 && !isnan(pm10_0) && pm10_0 >= 0 && !isnan(toluene) && toluene >= 0 &&
               !isnan(co) && co >= 0 && !isnan(co2) && co2 >= 0 && !isnan(nh4) && nh4 >= 0 && !isnan(temperature) &&
               !isnan(humidity);
    }
};

#endif