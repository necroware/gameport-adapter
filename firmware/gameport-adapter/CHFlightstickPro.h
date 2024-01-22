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

class CHFlightstickPro : public Joystick {
public:
  const Description &getDescription() const override {
    static const Description description{"CH FlightStick Pro", 4, 4, 1};
    return description;
  }

  const State &getState() const override {
    return m_state;
  }

  bool init() override {
    return true;
  }

  bool update() override {

    // Unfortunately I had no real CHFlighstickPro joystick to test, but
    // Sidewinder 3D Pro has an emulation for CHFlighstickPro. So, this
    // implementation was made using that emulation and could be wrong.
    // CHFlighstickPro seems to be a very interesting joystick. It doesn't
    // allow the user to press multiple buttons simultaneously and uses
    // combined buttons invocations as hat switch codes instead. So, every
    // time a multiple buttons seem to be pressed, means that the user is
    // actually using the hat switch.

    const auto decode = [](byte code) -> byte {
      static const byte table[16] = {0, 0, 0, 7, 0, 6, 0, 5, 0, 4, 0, 3, 0, 2, 0, 1};
      return (code < sizeof(table)) ? table[code] : 0u;
    };

    for (auto i = 0u; i < 4; i++) {
      m_state.axes[i] = m_joystick.getAxis(i);
    }

    const auto code = m_joystick.getButtons();
    m_state.hat = decode(code);
    m_state.buttons = m_state.hat ? 0u : code;

    return true;
  }

private:
  AnalogJoystick m_joystick;
  State m_state;
};

