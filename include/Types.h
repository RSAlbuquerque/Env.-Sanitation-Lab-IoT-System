#ifndef TYPES_H
#define TYPES_H

struct HydroValues {
    float temperature;
    float tds;
    float ph;
};

struct AirValues {
    // MQ-135 Values
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