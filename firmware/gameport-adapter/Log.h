#pragma once

#include <Arduino.h>
#include <stdio.h>
#include <stdarg.h>

inline void log(const char* fmt, ...) {
    va_list args;
    va_start (args, fmt);
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    Serial.println(buffer);
    va_end (args);
}

