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

#include "AnalogJoystick.h"
#include "Joystick.h"

class ThrustMaster : public Joystick {
public:

  const Description &getDescription() const override {
    static const Description description{"ThrustMaster", 3, 4, 1};
    return description;
  }

  const State &getState() const override {
    return m_state;
  }

  bool init() override {
    return true;
  }

  bool update() override {

    // Unfortunately I had no real ThrustMaster joystick to test, but
    // Sidewinder 3D Pro has an emulation for ThrustMaster. So, this
    // implementation was made using that emulation and could be wrong.
    // However the ThrustMaster seem to have a strange hat switch. It is
    // encoded using the 3rd axis with values between 0 and 512, where
    // 0-127 is up, 128-255 is right, 256-383 is down and 384-479 is left.
    // Everything above 480 considered to be middle.
    const auto hat = [](uint16_t value) -> uint8_t {
      if (value < 128) {
        return 1;
      }
      if (value < 256) {
        return 3;
      }
      if (value < 384) {
        return 5;
      }
      if (value < 480) {
        return 7;
      }
      return 0;
    };

    for (auto i = 0u; i < 3; i++) {
      m_state.axes[i] = m_joystick.getAxis(i);
    }
    m_state.hat = hat(m_joystick.getAxis(3));
    m_state.buttons = m_joystick.getButtons();

    return true;
  }

private:
  AnalogJoystick m_joystick;
  State m_state;
};

