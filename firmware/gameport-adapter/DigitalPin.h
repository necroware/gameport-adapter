#pragma once

#include <Arduino.h>

template <int ID>
struct DigitalPin {
    using RegType = uint8_t;
    const RegType mask;
    const RegType port;
    DigitalPin()
    : mask(digitalPinToBitMask(ID))
    , port(digitalPinToPort(ID))
    {}
};

template <int ID>
class DigitalOutput {
public:
    DigitalOutput()
    : m_output(*portOutputRegister(m_pin.port)) {
        *portModeRegister(m_pin.port) |= m_pin.mask;
    }

    void setHigh() {
      m_output |= m_pin.mask;
    }

    void setLow() {
      m_output &= ~m_pin.mask;
    }

    void set(bool value) {
      value ? setHigh() : setLow();
    }

    void toggle() {
        m_output ^= m_pin.mask;
    }

    void pulse(size_t microseconds = 0) {
        toggle();
        if (microseconds) {
            delayMicroseconds(microseconds);
        }
        toggle();
    }

private:
    const DigitalPin<ID> m_pin;
    volatile typename DigitalPin<ID>::RegType& m_output;
};

template <int ID, bool pullup = true>
class DigitalInput {
public:
    DigitalInput()
    : m_input(*portInputRegister(m_pin.port)) {
        *portModeRegister(m_pin.port) &= ~m_pin.mask;
        if (pullup) {            
          *portOutputRegister(m_pin.port) |= m_pin.mask;
        }
    }

    bool get() const {
      return m_input & m_pin.mask;
    }

    bool isHigh() const {
      return get();
    }

    bool isLow() const {
      return !get();
    }

private:
    DigitalPin<ID> m_pin;
    volatile typename DigitalPin<ID>::RegType& m_input;
};

