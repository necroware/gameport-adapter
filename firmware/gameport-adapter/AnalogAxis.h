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
