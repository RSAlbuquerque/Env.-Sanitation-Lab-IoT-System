#pragma once
#include <Arduino.h>

namespace Config {
    namespace Air {
        constexpr const char *VERSION = "1.2.2";
        constexpr const char *FW_URL = "https://pub-d6aabefbfe154c3587f0e92652c96737.r2.dev";
        constexpr unsigned long SENSOR_WARMUP_MS = 60000;

        namespace Pins {
            constexpr int BUTTON = 4;
            constexpr int PMS_RX = 16;
            constexpr int PMS_TX = 17;
            constexpr int MQ135 = 36;
            constexpr uint8_t BME680_ADDR = 0x77;
            constexpr uint8_t BME680_ADDR_FB = 0x76;
        }
    }
}