// DebugLog.h
#pragma once

#include <Arduino.h>
#include <stdarg.h>
#include <TelnetStream.h>

class DebugLog {
  public:
    // Overloads for Arduino String
    inline void print(const String &s) {
        Serial.print(s);
        TelnetStream.print(s);
    }

    inline void println(const String &s) {
        Serial.println(s);
        TelnetStream.println(s);
    }

    // Overloads for C-style strings
    inline void print(const char *s) {
        Serial.print(s);
        TelnetStream.print(s);
    }
    inline void println(const char *s) {
        Serial.println(s);
        TelnetStream.println(s);
    }

    // Print a newline
    inline void println() {
        Serial.println();
        TelnetStream.println();
    }

    // Formatted print
    inline void printf(const char *fmt, ...) {
        constexpr size_t BUF_SZ = 512;
        char buf[BUF_SZ];

        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, BUF_SZ, fmt, args);
        va_end(args);

        Serial.print(buf);
        TelnetStream.print(buf);
    }
};
