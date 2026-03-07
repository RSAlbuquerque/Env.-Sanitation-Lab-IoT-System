# IoT Environment Sanitation Monitor

A dual-module IoT monitoring system developed to track air and water quality in laboratory environments. The project leverages the **ESP32** ecosystem to provide a remotely maintainable solution for environmental sanitation.

## 🚀 Features

- **Dual Conectivity Support**: Support for both EAP (eduroam) and WPA2 networks (customizable).
- **Over-The-Air (OTA) Updates**: Support for remote firmware updates, facilitating maintenance and deployment.
- **Edge Visualization**: Local OLED display integration for real-time value monitoring at the source.
- **Cloud Integration**: Supports transmitting environmental telemetry to centralized cloud platforms.

## 🛠️ Tech Stack

- **Microcontroller**: ESP32
- **Language/Framework**: C++ / Arduino
- **Communication Protocols**: WiFi (LoRaWAN may be added on the future)

## 📂 System Overview

The system architecture is divided into two specialized modules:

### 💧 Water Quality Module

- **KS0429 (TDS)**: Measures Total Dissolved Solids to evaluate water purity.
- **PH4502 (pH)**: Monitors acidity or alkalinity levels.
- **DS18B20**: High-precision temperature sensor for liquid monitoring.

### 🌬️ Air Quality Module

- **MQ-135**: Monitors general air quality based on gas resistance (derives CO, CO2, NH4, alcohol, acetone and toluene values).
- **BME680**: Integrated environmental sensor for Temperature, Humidity, Pressure, and VOC (Volatile Organic Compounds) index.
- **SPS30**: Laser-based sensor for measuring particulate matter concentrations (PM1.0, PM2.5, and PM10).

## 📊 Data Access

You can view the sensor data collected by this project at the following links (temporary solution until a better system is developed):

- **Air Quality:** [Thingspeak Channel](https://thingspeak.mathworks.com/channels/3229374)
- **Hydro / Water Sensors:** [Thingspeak Channel](https://thingspeak.mathworks.com/channels/3058451)

---
