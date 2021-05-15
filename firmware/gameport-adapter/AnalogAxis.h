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

template <int ID>
class AnalogAxis {
public:
    AnalogAxis() {
        pinMode(ID, INPUT);
        m_mid = analogRead(ID);
        m_min = m_mid - 100;
        m_max = m_mid + 100;
    }

    byte get() {
        const auto value = analogRead(ID);
        if (value < m_min) {
            m_min = value;
        } else if (value > m_max) {
            m_max = value;
        }

        if (value < m_mid) {
            return map(value, m_min, m_mid, 255, 127);
        }
        return map(value, m_mid, m_max, 126, 0);
    }

private:
    int m_mid;
    int m_min;
    int m_max;
};
