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
  void pulse(uint32_t duration = 0) const {
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

  /// Gets the value of the input.
  bool get() const {
    return m_input & m_pin.mask;
  }

  /// Checks if the input is high.
  bool isHigh() const {
    return get();
  }

  /// Checks if the input is low
  bool isLow() const {
    return !get();
  }

  /// Waits for an edge with given timeout.
  /// @param[in] edge is the type of edge to wait for
  /// @param[in] timeount is the timeout in microseconds
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

  /// Waits for a state with given timeout.
  /// @param[in] state is the state to wait for
  /// @param[in] timeount is the timeout in microseconds
  uint32_t wait(bool state, uint32_t timeout) const {
    for (; state != get() && timeout; timeout--)
      ;
    return timeout;
  }

private:
  DigitalPin<Id> m_pin;
  volatile typename DigitalPin<Id>::RegType &m_input;
};
