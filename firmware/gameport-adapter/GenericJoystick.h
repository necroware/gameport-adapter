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

#include "Joystick.h"
#include "AnalogJoystick.h"

template <size_t Axes, size_t Buttons>
class GenericJoystick : public Joystick {
public:

    static_assert(Axes > 0 && Axes <= 4);

    bool init() override {
        return true;
    }

    bool update() override {
        for (auto i = 0u; i < Axes; i++) {
            m_state.axes[i] = m_joystick.getAxis(i);
        }
        m_state.buttons = m_joystick.getButtons();
        return true;
    }

    const State& getState() const override {
        return m_state;
    }

    const Description& getDescription() const override {
        static const Description description {
            "Generic Joystick", Axes, Buttons, 0
        };
        return description;
    }

private:
  AnalogJoystick m_joystick;
  State m_state;
};
