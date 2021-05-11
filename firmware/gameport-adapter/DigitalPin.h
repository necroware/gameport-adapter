#pragma once

#include <Arduino.h>

enum class Edge {
    any, falling, rising
};

template <int Id>
struct DigitalPin {
    using RegType = uint8_t;
    const RegType mask;
    const RegType port;
    DigitalPin()
    : mask(digitalPinToBitMask(Id))
    , port(digitalPinToPort(Id))
    {}
};

template <int Id>
class DigitalOutput {
public:
    DigitalOutput()
    : m_output(*portOutputRegister(m_pin.port)) {
        *portModeRegister(m_pin.port) |= m_pin.mask;
    }

    void setHigh() const {
      m_output |= m_pin.mask;
    }

    void setLow() const {
      m_output &= ~m_pin.mask;
    }

    void set(bool value) const {
      value ? setHigh() : setLow();
    }

    void toggle() const {
        m_output ^= m_pin.mask;
    }

    void pulse(uint32_t microseconds = 0) const {
        toggle();
        if (microseconds) {
            delayMicroseconds(microseconds);
        }
        toggle();
    }

private:
    const DigitalPin<Id> m_pin;
    volatile typename DigitalPin<Id>::RegType& m_output;
};

template <int Id, bool Pullup = true>
class DigitalInput {
public:
    DigitalInput()
    : m_input(*portInputRegister(m_pin.port)) {
        *portModeRegister(m_pin.port) &= ~m_pin.mask;
        if (Pullup) {
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

    uint32_t wait(Edge edge, uint32_t timeout) const {
      auto last = get();
      for (; timeout; timeout--) {
        const auto next = get();
        if (last == next) {
          continue;
        }
        switch (edge) {
            case Edge::falling:
              if (last > next) {
                return timeout;
              }
              break;
            case Edge::rising:
              if (last < next) {
                return timeout;
              }
              break;
            case Edge::any:
              return timeout;
        }
        last = next;
      }
      return 0u;
    }

    uint32_t wait(bool state, uint32_t timeout) const {
        for (; state != get() && timeout; timeout--);
        return timeout;
    }

private:
    DigitalPin<Id> m_pin;
    volatile typename DigitalPin<Id>::RegType& m_input;
};

