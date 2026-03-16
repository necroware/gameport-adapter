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

#include "DigitalPin.h"
#include "HidJoystick.h"

#include "CHFlightstickPro.h"
#include "CHF16CombatStick.h"
#include "GenericJoystick.h"
#include "GrIP.h"
#include "Logitech.h"
#include "Sidewinder.h"
#include "ThrustMaster.h"
#include "InterActPCPowerPadPro.h"

static Joystick *createJoystick() {

  const auto sw1 = DigitalInput<14, true>{};
  const auto sw2 = DigitalInput<15, true>{};
  const auto sw3 = DigitalInput<20, true>{};
  const auto sw4 = DigitalInput<21, true>{};

  // Give some time to setup the input
  delay(1);

  const auto sw = !sw4 << 3 | !sw3 << 2 | !sw2 << 1 | !sw1;

  switch (sw) {
    case 0b0001:
      return new GenericJoystick<2,4>;
    case 0b0010:
      return new GenericJoystick<3,4>;
    case 0b0011:
      return new GenericJoystick<4,4>;
    case 0b0100:
      return new CHFlightstickPro;
    case 0b0101:
      return new ThrustMaster;
    case 0b0110:
      return new CHF16CombatStick;
    case 0b0111:
      return new Sidewinder;
    case 0b1000:
      return new GrIP;
    case 0b1001:
      return new Logitech;
    case 0b1101:
      return new InterActPCPowerPadPro;
    default:
      return new GenericJoystick<2,2>;
  }
}

void setup() {
    // DEBUG information: Debugging is turned off by default
    // Comment the "NDEBUG" line in "Utilities.h" to enable logging to the serial monitor
    initLog();
}

void loop() {

  static auto hidJoystick = [] {
      HidJoystick hidJoystick;
      hidJoystick.init(createJoystick());
      return hidJoystick;
  }();

  hidJoystick.update();
}
