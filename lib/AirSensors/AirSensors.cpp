#include "AirSensors.h"

// Constructor
AirSensorsManager::AirSensorsManager(int mqPin, int pmsRxPin, int pmsTxPin, uint8_t bmeAddr, uint8_t bmeAddrFb) 
    : _mqPin(mqPin), 
      _pmsRx(pmsRxPin), 
      _pmsTx(pmsTxPin), 
      _bmeAddr(bmeAddr),
      _bmeAddrFb(bmeAddrFb),
      _pmsSerial(2),
      _pms(_pmsSerial)
{
    _mq135 = new MQUnifiedsensor(BOARD_TYPE, VOLTAGE_RESOLUTION, ADC_BIT_RESOLUTION, _mqPin, MQ_TYPE);
}

void AirSensorsManager::begin() {
    // MQ-135 Initialization and Calibration
    pinMode(_mqPin, INPUT); 
    _mq135->setRegressionMethod(1);
    _mq135->init();

    Serial.print("Calibrating MQ-135");
    float calcR0 = 0;
    for(int i = 1; i <= MQ_CALIB_SAMPLES; i++) {
        _mq135->update(); 
        calcR0 += _mq135->calibrate(RATIO_MQ135_CLEAN_AIR);
        Serial.print(".");
        delay(1000); 
    }
    Serial.println(" Done!");
    
    _mq135->setR0(calcR0 / MQ_CALIB_SAMPLES);

    if(isinf(calcR0)) { Serial.println(F("Warning: MQ135 Connection issue (Open Circuit)")); }
    if(calcR0 == 0)   { Serial.println(F("Warning: MQ135 Connection issue (Short Circuit)")); }
    
    // BME680 Initialization
    bool bmeFound = false;

    if (_bme.begin(_bmeAddr)) {
        Serial.println("Found at Primary Address.");
        bmeFound = true;
    } 
    else if (_bme.begin(_bmeAddrFb)) {
        Serial.println("Found at Fallback Address.");
        bmeFound = true;
    } 
    else {
        Serial.println("Error: BME680 not found!");
    }

    if (bmeFound) {
        _bme.setTemperatureOversampling(BME680_OS_8X);
        _bme.setHumidityOversampling(BME680_OS_2X);
        _bme.setPressureOversampling(BME680_OS_4X);
        _bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
        _bme.setGasHeater(320, 150);
    }

    // PMS5003 Initialization
    _pmsSerial.begin(9600, SERIAL_8N1, _pmsRx, _pmsTx);
    _pms.activeMode(); 
}

AirValues AirSensorsManager::readAll() {
    AirValues data;
    
    // Initialize with safe defaults (-1 indicates error/no-read)
    data = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    readMQ135(data);
    readBME680(data);
    readPMS5003(data);

    return data;
}

void AirSensorsManager::readMQ135(AirValues &data) {
    _mq135->update();
    data.adc_mq135 = analogRead(_mqPin);

    _mq135->setA(605.18); _mq135->setB(-3.937);
    float testCO = _mq135->readSensor(false, MQ_CORRECTION_FACTOR);

    if (isinf(testCO) || isnan(testCO) || testCO <= 0.0 || testCO > 1000000.0) {
        data.co = -2;
        return; 
    }

    data.co = testCO;

    _mq135->setA(77.255); _mq135->setB(-3.18);
    data.alcohol = _mq135->readSensor(false, MQ_CORRECTION_FACTOR);

    _mq135->setA(110.47); _mq135->setB(-2.862);
    data.co2 = _mq135->readSensor() + 400; // Offset for atmospheric CO2

    _mq135->setA(44.947); _mq135->setB(-3.445);
    data.toluene = _mq135->readSensor(false, MQ_CORRECTION_FACTOR);

    _mq135->setA(102.2);  _mq135->setB(-2.473);
    data.nh4 = _mq135->readSensor(false, MQ_CORRECTION_FACTOR);

    _mq135->setA(34.668); _mq135->setB(-3.369);
    data.acetone = _mq135->readSensor(false, MQ_CORRECTION_FACTOR);
}

void AirSensorsManager::readBME680(AirValues &data) {
    if (_bme.performReading()) {
        data.temperature = _bme.temperature;
        data.humidity = _bme.humidity;
        data.pressure = _bme.pressure / 100.0;
        data.gasResistance = _bme.gas_resistance / 1000.0;
    }
}

void AirSensorsManager::readPMS5003(AirValues &data) {
    PMS::DATA pmsData;

    if (_pms.read(pmsData)) {
        data.pm1_0  = pmsData.PM_AE_UG_1_0;
        data.pm2_5  = pmsData.PM_AE_UG_2_5;
        data.pm10_0 = pmsData.PM_AE_UG_10_0;
    }
}