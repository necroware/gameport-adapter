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

#include "HidCHFlightstickPro.h"
#include "HidJoystickB2A2.h"
#include "HidJoystickB4A2.h"
#include "HidJoystickB4A4.h"
#include "HidSidewinder.h"
#include "HidThrustMaster.h"
#include "DigitalPin.h"

static Driver* driver;

static Driver* createDriver(int sw) {
    switch(sw) {
        case 0b0001: return new HidJoystickB4A2;
        case 0b0010: return new HidJoystickB4A4;
        case 0b0011: return new HidCHFlightstickPro;
        case 0b0100: return new HidThrustMaster;
        case 0b1000: return new HidSidewinder;
        default: return new HidJoystickB2A2;
    }
}

void setup() {
  const auto sw1 = DigitalInput<14, true>{}.isLow();
  const auto sw2 = DigitalInput<15, true>{}.isLow();
  const auto sw3 = DigitalInput<20, true>{}.isLow(); 
  const auto sw4 = DigitalInput<21, true>{}.isLow();  
  Serial.begin(9600);
  while(!Serial);
  driver = createDriver(sw4 << 3 | sw3 << 2 | sw2 << 1 | sw1);
  driver->init();
}

void loop() {
  if (driver) {
    driver->update();
  }
}
