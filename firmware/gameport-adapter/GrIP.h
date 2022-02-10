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

#include "DigitalPin.h"
#include "Joystick.h"

/// Class to communicate with Gravis joysticks using GrIP.
/// @remark This is a green field implementation, but it was heavily
///         inspired by Linux Sidewinder driver implementation. See
class GrIP : public Joystick {
  ///         https://github.com/torvalds/linux/blob/master/drivers/input/joystick/grip.c
public:
  /// Resets the joystick and tries to detect the model.
  bool init() override {
    while (!readPacket())
      ;
    return true;
  }

  /// Reads the joystick state.
  /// @returns the state of axis, buttons etc.
  /// @remark if reading the state fails, the last known state is
  ///         returned and the joystick reset is executed.
  bool update() override {

    const auto packet = readPacket();
    if (packet == 0) {
      return false;
    }

    const auto getBit = [&](uint8_t pos) { return uint8_t(packet >> pos) & 1; };

    m_state.axes[0] = map(1 + getBit(15) - getBit(16), 0, 2, 0, 1023);
    m_state.axes[1] = map(1 + getBit(13) - getBit(12), 0, 2, 0, 1023);

    m_state.buttons = getBit(8);
    m_state.buttons |= getBit(3) << 1;
    m_state.buttons |= getBit(7) << 2;
    m_state.buttons |= getBit(6) << 3;
    m_state.buttons |= getBit(10) << 4;
    m_state.buttons |= getBit(11) << 5;
    m_state.buttons |= getBit(5) << 6;
    m_state.buttons |= getBit(2) << 7;
    m_state.buttons |= getBit(0) << 8;
    m_state.buttons |= getBit(1) << 9;

    return true;
  }

  const State &getState() const override {
    return m_state;
  }

  const Description &getDescription() const override {
    static Description desc{"Gravis GamePad Pro", 2, 10, 0};
    return desc;
  }

private:
  /// Supported Gravis model types.
  ///
  /// @remark currently inly GamePad Pro is supported and
  //          this enum is not used yet
  enum class Model {

    /// Unknown model
    GRIP_UNKNOWN,

    /// GamePad Pro
    GRIP_GAMEPAD_PRO,
  };

  DigitalInput<GamePort<2>::pin, true> m_clock;
  DigitalInput<GamePort<7>::pin, true> m_data;
  State m_state;

  /// Read bits packet from the joystick.
  uint32_t readPacket() const {

    // Gravis GamePad Pro sends 24 bits long packages of data all the
    // time. Every package starts with a binary tag sequence 011111.

    static const auto length = 24u;
    uint32_t result = 0u;

    // read a package of 24 bits
    for (auto i = 0u; i < length; i++) {
      if (!m_clock.wait(Edge::falling, 100)) {
        return 0u;
      }
      result |= uint32_t(m_data.isHigh()) << i;
    }

    // alighn the bits to have the binary tag in front. This code
    // was taken almost unchanged from the linux kernel.
    for (auto i = 0u; i < length; i++) {
      result = (result >> 1) | (result & 1) << (length - 1u);
      if ((result & 0xfe4210) == 0x7c0000) {
        return result;
      }
    }
    return 0u;
  }
};
