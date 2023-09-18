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

class CHF16CombatStick : public Joystick {
public:
  const Description &getDescription() const override {
    // CH F16 Combat Stick from 1995
    static const Description description{"CH F16 Combat Stick", 3, 10, 1};
    return description;
  }

  const State &getState() const override {
    return m_state;
  }

  bool init() override {  
    return true;
  }


  bool update() override {
    const auto decodeHat = [](byte code) -> byte {
      // Same as CH Flight Stick Pro. But the F16 stick has 4 positions on the hat only.
      //                                 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
      static constexpr byte table[16] = {0, 0, 0, 7, 0, 0, 0, 5, 0, 0, 0, 3, 0, 0, 0, 1};
      return (code < sizeof(table)) ? table[code] : 0u;
    };

    const auto decodeButtons = [](byte code) -> uint16_t {
      // Map to 10 buttons: 5 real ones, one hat with 4 positions. Thus we need uint16 instead of byte
      //                                     0  1  2  3  4   5    6  7   8  9   10 11   12 13  14 15
      static constexpr uint16_t table[16] = {0, 1, 8, 0, 4, 32, 256, 0, 16, 2, 128, 0, 512, 0, 64, 0};
      static constexpr auto tableSize = sizeof(table) / sizeof(table[0]);
      return (code < tableSize) ? table[code] : 0u;
    };

    // The 4th axis (index 2) is ignored, because there is a big jitter
    // and it reacts on movement of the other axes too. So you can not
    // assign the axes to a function in your game.
    m_state.axes[0] = m_joystick.getAxis(0);
    m_state.axes[1] = m_joystick.getAxis(1);
    m_state.axes[2] = m_joystick.getAxis(3); // Throttle
    
    const auto code = m_joystick.getButtons();

    m_state.hat = decodeHat(code);
    m_state.buttons = decodeButtons(code);
  
    log("Code %d : %d , A2 %d", code, m_state.buttons, m_state.axes[2] );
    return true;
  }

private:
  AnalogJoystick m_joystick;
  State m_state;
};
