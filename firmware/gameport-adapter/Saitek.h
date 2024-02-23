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

class Saitek : public Joystick {
public:
  const Description &getDescription() const override {
    static const Description description{"Saitek", 3, 6, 1};
    return description;
  }

  const State &getState() const override {
    return m_state;
  }

  bool init() override {
    return true;
  }

  bool update() override {

    const auto decode = [](byte code) -> byte {
      static const byte table[16] = {0, 0, 0, 7, 0, 0, 0, 5, 0, 0, 0, 3, 0, 0, 0, 1};
      return (code < sizeof(table)) ? table[code] : 0u;
    };

    m_state.axes[0] = m_joystick.getAxis(0);
    m_state.axes[1] = m_joystick.getAxis(1);
    m_state.axes[2] = 1023-m_joystick.getAxis(3);
      
    const auto code = m_joystick.getButtons();
    m_state.hats[0] = decode(code);
    if (m_state.hats[0]!=0)
    {
      m_state.buttons = 0u;
    }
    else if (code == 5) {
      m_state.buttons = 16u;
    }
    else if (code == 9) {
      m_state.buttons = 32u;
    }
    else
    {
      m_state.buttons = code;
    }

    return true;
  }

private:
  AnalogJoystick m_joystick;
  State m_state;
};
