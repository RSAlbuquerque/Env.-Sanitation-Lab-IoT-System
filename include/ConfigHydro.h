#pragma once
#include <Arduino.h>

namespace Config {
    namespace Hydro {
        constexpr const char* VERSION   = "2.0.2";
        constexpr const char* FW_URL    = "https://pub-f48ea50e587f4657ae2f43c545374bd1.r2.dev";

        constexpr float TDS_FACTOR = 1.56f;
        
        namespace Pins {
            constexpr int TEMP    = 4;
            constexpr int BUTTON  = 23;
            constexpr int TDS     = 34;
            constexpr int PH      = 36;
        }
    }
}