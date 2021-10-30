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

/// Class to read analog axis.
///
/// Usually PC joystick axes are specified to have 100 Ohm resistance, but
/// many analog joysticks have old, bad, or just wrong resistors. This ends
/// up in incorrect positions and is especially bad for games, which don't
/// have calibration features. This class takes care of this issue and
/// applies automatic calibration on every read.
template <int ID>
class AnalogAxis {
public:
  /// Constructor.
  ///
  /// The initial state of the joystick is considered as middle
  /// which is used for autocalibration.
  AnalogAxis() {
    pinMode(ID, INPUT);
    m_mid = analogRead(ID);
    m_min = m_mid - 100;
    m_max = m_mid + 100;
  }

  /// Gets the axis state.
  ///
  /// This function automatically recalculates the outer limits and
  /// readjusts the position of the joystick.
  /// @returns a value between 0 and 1023
  uint16_t get() {
    const auto value = analogRead(ID);
    if (value < m_min) {
      m_min = value;
    } else if (value > m_max) {
      m_max = value;
    }

    if (value < m_mid) {
      return map(value, m_min, m_mid, 1023, 512);
    }
    return map(value, m_mid, m_max, 511, 0);
  }

private:
  int m_mid;
  int m_min;
  int m_max;
};
