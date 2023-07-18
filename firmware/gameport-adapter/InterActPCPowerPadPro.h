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

class InterActPCPowerPadPro : public Joystick {
public:
  const Description &getDescription() const override {
    static const Description description{"InterAct PC PowerPad Pro", 3, 6, 0};
    return description;
  }

  const State &getState() const override {
    return m_state;
  }

  bool init() override {
    return true;
  }

  bool update() override {

    m_state.axes[0] = m_joystick.getAxis(0);
    m_state.axes[1] = m_joystick.getAxis(1);
    m_state.axes[2] = m_joystick.getAxis(3);

    m_state.buttons = m_joystick.getButtons() | (m_joystick.getAxis(2)<80)<<4 | (m_joystick.getAxis(3)<80)<<5;


    return true;
  }

private:
  AnalogJoystick m_joystick;
  State m_state;
};
