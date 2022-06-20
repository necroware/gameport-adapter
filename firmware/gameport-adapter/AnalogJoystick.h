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

#include "AnalogAxis.h"
#include "DigitalPin.h"
#include "GamePort.h"

/// A common class for all analog joysticks.
class AnalogJoystick {
public:
  /// Gets axis value.
  ///
  /// @param[in] id is the axes ID
  /// @returns a value between 0 and 255
  uint16_t getAxis(int id) {
    switch (id) {
      case 0:
        return m_axis1.get();
      case 1:
        return m_axis2.get();
      case 2:
        return m_axis3.get();
      case 3:
        return m_axis4.get();
      default:
        return 0u;
    }
  }

  /// Gets the buttons state as one byte.
  ///
  /// @returns a byte every bit represents a button
  byte getButtons() {
    return m_button1.isLow() | m_button2.isLow() << 1 | m_button3.isLow() << 2 | m_button4.isLow() << 3;
  }

private:
  DigitalInput<GamePort<2>::pin> m_button1;
  DigitalInput<GamePort<7>::pin> m_button2;
  DigitalInput<GamePort<10>::pin> m_button3;
  DigitalInput<GamePort<14>::pin> m_button4;
  AnalogAxis<GamePort<3>::pin> m_axis1;
  AnalogAxis<GamePort<6>::pin> m_axis2;
  AnalogAxis<GamePort<11>::pin> m_axis3;
  AnalogAxis<GamePort<13>::pin> m_axis4;
};
