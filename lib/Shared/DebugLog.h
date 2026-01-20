#pragma once

#include "ConfigCommon.h"

#include <Arduino.h>
#include <stdarg.h>
#include <TelnetStream.h>

// Log levels
#define LOG_ERROR 1
#define LOG_WARN 2
#define LOG_INFO 3
#define LOG_DEBUG 4

class DebugLog {
  public:
#if ENABLE_DEBUG && DEBUG_LEVEL >= LOG_ERROR
    inline void error(const char *fmt, ...) {
        printPrefix("ERROR");

        va_list args;
        va_start(args, fmt);
        vprintfImpl(fmt, args);
        va_end(args);

        newline();
    }
#else
    inline void error(const char *, ...) {}
#endif

#if ENABLE_DEBUG && DEBUG_LEVEL >= LOG_WARN
    inline void warn(const char *fmt, ...) {
        printPrefix("WARN");

        va_list args;
        va_start(args, fmt);
        vprintfImpl(fmt, args);
        va_end(args);

        newline();
    }
#else
    inline void warn(const char *, ...) {}
#endif

#if ENABLE_DEBUG && DEBUG_LEVEL >= LOG_INFO
    inline void info(const char *fmt, ...) {
        printPrefix("INFO");

        va_list args;
        va_start(args, fmt);
        vprintfImpl(fmt, args);
        va_end(args);

        newline();
    }
#else
    inline void info(const char *, ...) {}
#endif

#if ENABLE_DEBUG && DEBUG_LEVEL >= LOG_DEBUG
    inline void debug(const char *fmt, ...) {
        printPrefix("DEBUG");

        va_list args;
        va_start(args, fmt);
        vprintfImpl(fmt, args);
        va_end(args);

        newline();
    }
#else
    inline void debug(const char *, ...) {}
#endif

  private:
#if ENABLE_DEBUG
    inline void printPrefix(const char *lvl) {
        Serial.print(lvl);
        Serial.print(": ");
        TelnetStream.print(lvl);
        TelnetStream.print(": ");
    }

    inline void newline() {
        Serial.println();
        TelnetStream.println();
    }

    inline void vprintfImpl(const char *fmt, va_list args) {
        constexpr size_t BUF_SZ = 256;
        char buf[BUF_SZ];
        vsnprintf(buf, BUF_SZ, fmt, args);
        Serial.print(buf);
        TelnetStream.print(buf);
    }
#endif
};

extern DebugLog Debug;
