#pragma once

#include "AnalogAxis.h"
#include "DigitalPin.h"
#include "GamePort.h"

class AnalogJoystick
{
public:

    byte getAxis(int id) {
        switch(id) {
            case 0: return m_axis1.get();
            case 1: return m_axis2.get();
            case 2: return m_axis3.get();
            case 3: return m_axis4.get();
            default: return 0u;
        }
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
            default: return false;
        }
    }

private:
    DigitalInput<GamePort<2>::pin> m_button1;
    DigitalInput<GamePort<7>::pin> m_button2;
    DigitalInput<GamePort<10>::pin> m_button3;
    DigitalInput<GamePort<14>::pin> m_button4;
    AnalogAxis<GamePort<3>::pin> m_axis1;
    AnalogAxis<GamePort<6>::pin> m_axis2;
    AnalogAxis<GamePort<11>::pin> m_axis3;
    AnalogAxis<GamePort<13>::pin> m_axis4;
};
