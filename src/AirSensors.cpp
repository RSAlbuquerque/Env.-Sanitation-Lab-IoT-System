#include "AirSensors.h"

#define MQ_PIN 36 
#define PMS_RX_PIN 16 
#define PMS_TX_PIN 17 

AirSensorsManager AirSensors(MQ_PIN, PMS_RX_PIN, PMS_TX_PIN);

AirSensorsManager::AirSensorsManager(int mqPin, int pmsRxPin, int pmsTxPin) 
    : _mqPin(mqPin), _pmsRx(pmsRxPin), _pmsTx(pmsTxPin), _pmsSerial(2), _pms(_pmsSerial)
{
    _data = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    _mq135 = new MQUnifiedsensor(BOARD_TYPE, VOLTAGE_RESOLUTION, ADC_BIT_RESOLUTION, _mqPin, "MQ-135");
}

void AirSensorsManager::begin() {
    // Initialize MQ-135
    pinMode(_mqPin, INPUT); 
    _mq135->setRegressionMethod(1);
    _mq135->init();

    Serial.print(F("Calibrating MQ-135..."));
    float calcR0 = 0;
    for(int i = 1; i <= 10; i++) {
        _mq135->update(); 
        calcR0 += _mq135->calibrate(RATIO_MQ135_CLEAN_AIR);
        delay(100); 
    }
    _mq135->setR0(calcR0 / 10);
    Serial.println(F(" Done!"));

    if(isinf(calcR0)) { Serial.println(F("Warning: Connection issue (Open Circuit)")); }
    if(calcR0 == 0)   { Serial.println(F("Warning: Connection issue (Short Circuit)")); }

    // Initialize BME680
    if (!bme.begin(0x77)) {
        if (!bme.begin(0x76)) {
            Serial.println("Error: Could not find BME680 sensor!");
        }
    }

    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150);

    // Initialize PMS5003
    _pmsSerial.begin(9600, SERIAL_8N1, _pmsRx, _pmsTx);
    _pms.activeMode();
}

AirValues AirSensorsManager::readAll() {
    readMQ135(_data);
    readBME680(_data);
    readPMS5003(_data);

    return _data;
}

void AirSensorsManager::readMQ135(AirValues &data) {
    _mq135->update();

    // Read CO
    _mq135->setA(605.18); _mq135->setB(-3.937);
    data.co = _mq135->readSensor();

    // Read Alcohol
    _mq135->setA(77.255); _mq135->setB(-3.18);
    data.alcohol = _mq135->readSensor();

    // Read CO2
    _mq135->setA(110.47); _mq135->setB(-2.862);
    // Note: The library author suggests adding +400 offset for CO2
    data.co2 = _mq135->readSensor() + 400; 

    // Read Toluen
    _mq135->setA(44.947); _mq135->setB(-3.445);
    data.toluene = _mq135->readSensor();

    // Read NH4 (Ammonium)
    _mq135->setA(102.2);  _mq135->setB(-2.473);
    data.nh4 = _mq135->readSensor();

    // Read Acetone
    _mq135->setA(34.668); _mq135->setB(-3.369);
    data.acetone = _mq135->readSensor();
}

void AirSensorsManager::readBME680(AirValues &data) {
    if (bme.performReading()) {
        data.temperature = bme.temperature;
        data.humidity = bme.humidity;
        data.pressure = bme.pressure / 100.0;
        data.gasResistance = bme.gas_resistance / 1000.0;
    }
}

void AirSensorsManager::readPMS5003(AirValues &data) {
    PMS::DATA pmsData;

    int temp = _pms.read(pmsData);
    data.pm1_0 = pmsData.PM_AE_UG_1_0;
    data.pm2_5 = pmsData.PM_AE_UG_2_5;
    data.pm10_0 = pmsData.PM_AE_UG_10_0;
}