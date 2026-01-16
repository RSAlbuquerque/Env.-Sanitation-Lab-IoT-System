// DebugLog.h
#pragma once

#include <Arduino.h>
#include <TelnetStream.h>
#include <stdarg.h>

class DebugLog {
public:
  DebugLog(bool enabled = false) : _enabled(enabled) {}

  void setEnabled(bool enabled) { _enabled = enabled; }
  bool enabled() const { return _enabled; }

  // Print a string
  inline void print(const String &s) {
    if (!_enabled) return;
    Serial.print(s);
    TelnetStream.print(s);
  }
  inline void println(const String &s) {
    if (!_enabled) return;
    Serial.println(s);
    TelnetStream.println(s);
  }

  // Overloads for C-style strings
  inline void print(const char *s) {
    if (!_enabled) return;
    Serial.print(s);
    TelnetStream.print(s);
  }
  inline void println(const char *s) {
    if (!_enabled) return;
    Serial.println(s);
    TelnetStream.println(s);
  }

  // Print a newline
  inline void println() {
    if (!_enabled) return;
    Serial.println();
    TelnetStream.println();
  }

  // Formatted print
  inline void printf(const char *fmt, ...) {
    if (!_enabled) return;
    constexpr size_t BUF_SZ = 512;
    char buf[BUF_SZ];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, BUF_SZ, fmt, args);
    va_end(args);

    Serial.print(buf);
    TelnetStream.print(buf);
  }

private:
  bool _enabled;
};
