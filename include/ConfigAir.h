#pragma once
#include <Arduino.h>

namespace Config {
    namespace Air {
        constexpr const char *VERSION = "1.1.3";
        constexpr const char *FW_URL = "https://pub-d6aabefbfe154c3587f0e92652c96737.r2.dev";

        namespace Pins {
            constexpr int BUTTON = 23;
            constexpr int PMS_RX = 16;
            constexpr int PMS_TX = 17;
            constexpr int MQ135 = 36;
            constexpr uint8_t BME680_ADDR = 0x77;
            constexpr uint8_t BME680_ADDR_FB = 0x76;
        }
    }
}