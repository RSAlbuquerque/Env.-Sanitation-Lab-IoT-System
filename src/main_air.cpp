#include <Arduino.h>
#include <Wire.h>

#include "AirSensors.h"
#include "Display.h"
#include "Types.h"

unsigned long lastAirRead = 0;
const unsigned long AIR_READ_INTERVAL = 3000;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("Initializing...");
  delay(5000);
  Wire.begin(21, 22);
  delay(1000);
  display.setup();
  AirSensors.begin();

  Serial.println("Warming up sensors (20 sec)...");
  delay(20000); 
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastAirRead >= AIR_READ_INTERVAL) {
    lastAirRead = currentMillis;

    AirValues air = AirSensors.readAll();

    Serial.print("Temp:     "); Serial.print(air.temperature); Serial.println(" C");
    Serial.print("Humidity: "); Serial.print(air.humidity);    Serial.println(" %");
    Serial.print("Pressure: "); Serial.print(air.pressure);    Serial.println(" hPa");
    Serial.print("VOC Res:  "); Serial.print(air.gasResistance); Serial.println(" KOhms");
    
    // MQ-135 Gases
    Serial.print("CO:       "); Serial.print(air.co);            Serial.println(" ppm");
    Serial.print("Alcohol:  "); Serial.print(air.alcohol);       Serial.println(" ppm");
    Serial.print("CO2:      "); Serial.print(air.co2);           Serial.println(" ppm");
    Serial.print("Toluen:   "); Serial.print(air.toluene);        Serial.println(" ppm");
    Serial.print("NH4:      "); Serial.print(air.nh4);           Serial.println(" ppm");
    Serial.print("Acetone:  "); Serial.print(air.acetone);       Serial.println(" ppm");

    // PMS5003 Particles
    Serial.print("PM1.0:    "); Serial.print(air.pm1_0);        Serial.println(" ug/m3");
    Serial.print("PM2.5:    "); Serial.print(air.pm2_5);        Serial.println(" ug/m3");
    Serial.print("PM10.0:   "); Serial.print(air.pm10_0);       Serial.println(" ug/m3");

    display.airSensorValues(air);
  }
}