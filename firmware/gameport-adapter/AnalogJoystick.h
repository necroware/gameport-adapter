#pragma once

#include "AnalogPin.h"
#include "DigitalPin.h"
#include "GamePort.h"

class AnalogJoystick
{
public:

    byte getAxis(int id) {

        const auto translate = [](int value) -> byte {
            static const int upper = 1023;
            static const int lower = 430;
            static const int centr = 600;
            const auto constrained = constrain(value, lower, upper);
            if (constrained < centr) {
                return map(constrained, lower, 600, 255, 127);
            }
            return map(constrained, 600, upper, 126, 0);
        };

        switch(id) {
            case 0: return translate(m_axis1.get());
            case 1: return translate(m_axis2.get());
            case 2: return translate(m_axis3.get());
            case 3: return translate(m_axis4.get());
        }

        return 0u;
    }

    byte getButtons() {
        return m_button1.isLow() 
            | m_button2.isLow() << 1
            | m_button3.isLow() << 2 
            | m_button4.isLow() << 3;
    }

    bool isPressed(int id) {
        switch(id) {
            case 0: return m_button1.isLow();
            case 1: return m_button2.isLow();
            case 2: return m_button3.isLow();
            case 3: return m_button4.isLow();
        }
        return false;
    }

private:
    DigitalInput<GamePort<2>::pin> m_button1;
    DigitalInput<GamePort<7>::pin> m_button2;
    DigitalInput<GamePort<10>::pin> m_button3;
    DigitalInput<GamePort<14>::pin> m_button4;
    AnalogInput<GamePort<3>::pin> m_axis1;
    AnalogInput<GamePort<6>::pin> m_axis2;
    AnalogInput<GamePort<11>::pin> m_axis3;
    AnalogInput<GamePort<13>::pin> m_axis4;
};
