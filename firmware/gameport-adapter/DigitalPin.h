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

/// Digital signal edge types.
enum class Edge { any, falling, rising };

/// Digital pin base constants.
///
/// This implementation was born out of the need for faster digital I/O. The
/// original digitalRead(...) function of Arduino suite needs about 2.7us per
/// call on a 16MHz MCU. Which makes it impossible to poll 5us pulses, not
/// talking about doing with the data something in between. This was a hard
/// requirement for this project, so following solution is up to 50% faster
/// and needs about 1.6us per call on the same hardware.

template <int Id>
struct DigitalPin {
  using RegType = uint8_t;
  const RegType mask;
  const RegType port;
  DigitalPin()
  : mask(digitalPinToBitMask(Id))
  , port(digitalPinToPort(Id)) {
  }
};

/// Digital Output class.
template <int Id>
class DigitalOutput {
public:
  /// Constructor.
  DigitalOutput()
  : m_output(*portOutputRegister(m_pin.port)) {
    *portModeRegister(m_pin.port) |= m_pin.mask;
  }

  /// Sets output high.
  void setHigh() const {
    m_output |= m_pin.mask;
  }

  /// Sets output low.
  void setLow() const {
    m_output &= ~m_pin.mask;
  }

  /// Sets output to the given value.
  void set(bool value) const {
    value ? setHigh() : setLow();
  }

  /// Toggles the output.
  void toggle() const {
    m_output ^= m_pin.mask;
  }

  /// Triggers a pulse of given duration.
  /// @param[in] duration is the duration in microseconds
  void pulse(uint16_t duration = 0) const {
    toggle();
    if (duration) {
      delayMicroseconds(duration);
    }
    toggle();
  }

private:
  const DigitalPin<Id> m_pin;
  volatile typename DigitalPin<Id>::RegType &m_output;
};

/// Digital input class.
template <int Id, bool Pullup = true>
class DigitalInput {
public:
  /// Constructor.
  DigitalInput()
  : m_input(*portInputRegister(m_pin.port)) {
    *portModeRegister(m_pin.port) &= ~m_pin.mask;
    if (Pullup) {
      *portOutputRegister(m_pin.port) |= m_pin.mask;
    }
  }

  operator bool() const {
      return isHigh();
  }

  /// Read raw bit data
  uint8_t read() const {
    return m_input & m_pin.mask;
  }

  /// Checks if the input is high.
  bool isHigh() const {
    return read();
  }

  /// Checks if the input is low
  bool isLow() const {
    return !read();
  }

  /// Waits for an edge with given timeout.
  /// @param[in] edge is the type of edge to wait for
  /// @param[in] timeount is the timeout in microseconds
  uint16_t wait(Edge edge, uint16_t timeout) const {
    if (edge == Edge::falling) {
      return waitImpl(timeout, [](uint8_t a, uint8_t) {return a;});
    }
    if (edge == Edge::rising) {
      return waitImpl(timeout, [](uint8_t, uint8_t b) {return b;});
    }
    // edge == Edge::rising
    return waitImpl(timeout, [](uint8_t a, uint8_t b) {return a|b;});
  }

  /// Waits for a state with given timeout.
  /// @param[in] state is the state to wait for
  /// @param[in] timeount is the timeout in microseconds
  uint16_t wait(bool state, uint16_t timeout) const {
    for (; state != isHigh() && timeout; timeout--)
      ;
    return timeout;
  }

private:
  DigitalPin<Id> m_pin;
  volatile typename DigitalPin<Id>::RegType &m_input;

  template <typename T>
  uint16_t waitImpl(uint16_t timeout, T compare) const {
    auto last = read();
    for (; timeout; timeout--) {
      const auto next = read();
      if (last != next) {
        if (compare(last, next)) {
          return timeout;
        }
        last = next;
      }
    }
    return 0u;
  }
};
