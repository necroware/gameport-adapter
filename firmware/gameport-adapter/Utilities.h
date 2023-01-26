// This file is part of Necroware's GamePort adapter firmware.
// Copyright (C) 2021 Necroware
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>

/// Debug messages on serial port are turned off by default. Comment the following
/// line to enable logging to the serial port.
/// Arduino Micro seems somehow to share the serial port with the USB interface.
/// If the serial port will be activated, the operating system will no longer
/// recognize the USB device!
#define NDEBUG

#ifdef NDEBUG
#define initLog()
#define log(...)
#else
inline void initLog() {
    Serial.begin(9600);
    while(!Serial); 
}

inline void log(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buffer[512];
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  Serial.println(buffer);
}
#endif // !NDEBUG

/// Interrupt guard (RAII).
///
/// This class is used to deactivate the interrupts in performance
/// critical sections. The interrupt is reactivated as soon as this
/// guard runs out of scope.
struct InterruptStopper {
  InterruptStopper() { noInterrupts(); }
  ~InterruptStopper() { interrupts(); }
  InterruptStopper(const InterruptStopper&) = delete;
  InterruptStopper(InterruptStopper&&) = delete;
  InterruptStopper& operator=(const InterruptStopper&) = delete;
  InterruptStopper& operator=(InterruptStopper&&) = delete;
};
