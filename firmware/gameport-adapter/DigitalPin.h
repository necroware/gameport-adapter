#pragma once

#include <Arduino.h>

class DigitalOutput {
public:
    explicit DigitalOutput(byte id)
    : m_mask(digitalPinToBitMask(id))
    , m_output(*portOutputRegister(digitalPinToPort(id)))
    {
        *portModeRegister(digitalPinToPort(id)) |= m_mask;
    }

    void setHigh() {
      m_output |= m_mask;
    }

    void setLow() {
      m_output &= ~m_mask;
    }

    void set(bool value) {
      value ? setHigh() : setLow();
    }

private:
    const uint8_t m_mask;
    volatile uint8_t& m_output;
};

class DigitalInput {
public:
    explicit DigitalInput(byte id, bool pullup = true)
    : m_mask(digitalPinToBitMask(id))
    , m_input(*portInputRegister(digitalPinToPort(id)))
    {
        const auto pinPort = digitalPinToPort(id);
        *portModeRegister(pinPort) &= ~m_mask;
        if (pullup) {            
          *portOutputRegister(pinPort) |= m_mask;
        }
    }

    bool get() const {
      return m_input & m_mask;
    }

    bool isHigh() const {
      return get();
    }

    bool isLow() const {
      return !get();
    }

private:
    const uint8_t m_mask;
    volatile uint8_t& m_input;
};

