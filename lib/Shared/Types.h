#ifndef TYPES_H
#define TYPES_H

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

    // PMS5003 Values
    int pm1_0;
    int pm2_5;
    int pm10_0;
};

#endif