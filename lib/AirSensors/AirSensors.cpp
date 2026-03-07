#include "AirSensors.h"

#include "DebugLog.h"

#include <TelnetStream.h>

#ifdef NO_ERROR
    #undef NO_ERROR
#endif
#define NO_ERROR 0

// Constructor
AirSensorsManager::AirSensorsManager(int mqPin, uint8_t bmeAddr, uint8_t bmeAddrFb)
    : _mqPin(mqPin), _bmeAddr(bmeAddr), _bmeAddrFb(bmeAddrFb) {
    _mq135 = new MQUnifiedsensor(BOARD_TYPE, VOLTAGE_RESOLUTION, ADC_BIT_RESOLUTION, _mqPin, MQ_TYPE);
}

void AirSensorsManager::begin() {
    // MQ-135 Initialization and Calibration
    pinMode(_mqPin, INPUT);
    _mq135->setRegressionMethod(1);
    _mq135->init();

    Debug.info("Calibrating MQ135...");
    float calcR0 = 0;
    for (int i = 1; i <= MQ_CALIB_SAMPLES; i++) {
        _mq135->update();
        calcR0 += _mq135->calibrate(RATIO_MQ135_CLEAN_AIR);
        delay(1000);
    }

    _mq135->setR0(calcR0 / MQ_CALIB_SAMPLES);

    if (isinf(calcR0)) {
        Debug.warn("MQ135 Connection issue (Open Circuit)");
    }
    if (calcR0 == 0) {
        Debug.warn("MQ135 Connection issue (Short Circuit)");
    } else {
        Debug.info("MQ135 R0 Calibrated: %.2f kOhms", _mq135->getR0());
    }

    // BME680 Initialization
    bool bmeFound = false;

    if (_bme.begin(_bmeAddr)) {
        bmeFound = true;
    } else if (_bme.begin(_bmeAddrFb)) {
        bmeFound = true;
    } else {
        Debug.error("BME680 not found at 0x%02X or 0x%02X", _bmeAddr, _bmeAddrFb);
    }

    if (bmeFound) {
        _bme.setTemperatureOversampling(BME680_OS_8X);
        _bme.setHumidityOversampling(BME680_OS_2X);
        _bme.setPressureOversampling(BME680_OS_4X);
        _bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
        _bme.setGasHeater(320, 150);
    }

    // SPS30 Initialization
    Wire.begin();
    _sps30.begin(Wire, SPS30_I2C_ADDR_69);
    _sps30.stopMeasurement();
    _sps30.startMeasurement(SPS30_OUTPUT_FORMAT_OUTPUT_FORMAT_FLOAT);
}

AirValues AirSensorsManager::readAll() {
    AirValues data;

    // Initialize with safe defaults (-1 indicates
    // error/no-read)
    data = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    Debug.debug("Reading Air Sensors...");
    readMQ135(data);
    readBME680(data);
    readSPS30(data);
    Debug.debug("Air Sensors reading complete.");

    return data;
}

void AirSensorsManager::readMQ135(AirValues &data) {
    _mq135->update();
    data.adc_mq135 = analogRead(_mqPin);

    _mq135->setA(605.18);
    _mq135->setB(-3.937);
    float testCO = _mq135->readSensor(false, MQ_CORRECTION_FACTOR);

    if (isinf(testCO) || isnan(testCO) || testCO <= 0.0 || testCO > 1000000.0) {
        Debug.error("MQ135 reading error");
        data.co = -2;
        return;
    }

    data.co = testCO;

    _mq135->setA(77.255);
    _mq135->setB(-3.18);
    data.alcohol = _mq135->readSensor(false, MQ_CORRECTION_FACTOR);

    _mq135->setA(110.47);
    _mq135->setB(-2.862);
    data.co2 = _mq135->readSensor(false, MQ_CORRECTION_FACTOR) + 400; // Offset for atmospheric CO2

    _mq135->setA(44.947);
    _mq135->setB(-3.445);
    data.toluene = _mq135->readSensor(false, MQ_CORRECTION_FACTOR);

    _mq135->setA(102.2);
    _mq135->setB(-2.473);
    data.nh4 = _mq135->readSensor(false, MQ_CORRECTION_FACTOR);

    _mq135->setA(34.668);
    _mq135->setB(-3.369);
    data.acetone = _mq135->readSensor(false, MQ_CORRECTION_FACTOR);

    Debug.debug("MQ135: ADC=%.2f CO=%.2fppm CO2=%.2fppm NH4=%.2fppm Alcohol=%.2fppm Acetone=%.2fppm Toluene=%.2fppm",
                data.adc_mq135, data.co, data.co2, data.nh4, data.alcohol, data.acetone, data.toluene);
}

void AirSensorsManager::readBME680(AirValues &data) {
    if (_bme.performReading()) {
        data.temperature = _bme.temperature;
        data.humidity = _bme.humidity;
        data.pressure = _bme.pressure / 100.0;
        data.gasResistance = _bme.gas_resistance / 1000.0;
        Debug.debug("BME680: T=%.2fC H=%.2f%% P=%.2fhPa G=%.2kOhm", data.temperature, data.humidity, data.pressure,
                    data.gasResistance);
    } else {
        Debug.error("BME680 reading error");
    }
}

void AirSensorsManager::readSPS30(AirValues &data) {
    uint16_t dataReadyFlag = 0;
    static char errorMessage[64];

    int16_t error = _sps30.readDataReadyFlag(dataReadyFlag);
    if (error != NO_ERROR) {
        errorToString(error, errorMessage, sizeof errorMessage);
        Debug.error("SPS30 readDataReadyFlag error: %s", errorMessage);
        return;
    }

    if (!dataReadyFlag) {
        Debug.warn("SPS30 data not ready yet");
        return;
    }

    float mc1p0 = 0, mc2p5 = 0, mc4p0 = 0, mc10p0 = 0;
    float nc0p5 = 0, nc1p0 = 0, nc2p5 = 0, nc4p0 = 0, nc10p0 = 0;
    float typicalParticleSize = 0;

    error = _sps30.readMeasurementValuesFloat(mc1p0, mc2p5, mc4p0, mc10p0, nc0p5, nc1p0, nc2p5, nc4p0, nc10p0,
                                              typicalParticleSize);
    if (error != NO_ERROR) {
        errorToString(error, errorMessage, sizeof errorMessage);
        Debug.error("SPS30 readMeasurementValuesFloat error: %s", errorMessage);
        return;
    }
    if (valuesAreInvalid(mc1p0, mc2p5, mc4p0, mc10p0)) {
        Debug.error("SPS30 returned invalid measurement values");
        return;
    }

    data.pm1_0 = mc1p0;
    data.pm2_5 = mc2p5;
    data.pm10_0 = mc10p0;

    Debug.debug("SPS30: PM1=%.2f PM2.5=%.2f PM4=%.2f PM10=%.2f µg/m³", mc1p0, mc2p5, mc4p0, mc10p0);
}

bool AirSensorsManager::valuesAreInvalid(float mc1p0, float mc2p5, float mc4p0, float mc10p0) {
    return isnan(mc1p0) || isnan(mc2p5) || isnan(mc4p0) || isnan(mc10p0) ||
           (mc1p0 == 0 && mc2p5 == 0 && mc4p0 == 0 && mc10p0 == 0);
}