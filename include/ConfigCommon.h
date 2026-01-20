#pragma once
#include <Arduino.h>

#define ENABLE_DEBUG 1
#define DEBUG_LEVEL 3 // 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG

namespace Config {

    namespace Wifi {
        constexpr const char *EAP_SSID = "eduroam";
        constexpr const char *FALLBACK_SSID = "LSA-UBI-AP";
        constexpr unsigned long TIMEOUT_MS = 10000; // 10 secs
    }

    namespace Cloud {
        constexpr const char *THINGSPEAK_URL = "http://api.thingspeak.com/update";
    }

    namespace Display {
        constexpr int SDA_PIN = 21;
        constexpr int SCL_PIN = 22;
        constexpr int RESET_PIN = -1;
        constexpr int ADDRESS = 0x3C;
        constexpr int WIDTH = 128;
        constexpr int HEIGHT = 64;
    }

    namespace Timers {
        constexpr unsigned long READ_INTERVAL = 20000UL;                // 20 secs
        constexpr unsigned long SEND_INTERVAL = 15 * 60 * 1000UL;       // 15 mins
        constexpr unsigned long CHECK_INTERVAL = 24 * 60 * 60 * 1000UL; // 24h
    }
}