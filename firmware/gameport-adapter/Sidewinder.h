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
#include "Log.h"

/// Class to for communication with Sidewinder joysticks.
/// @remark This is a green field implementation, but it was heavily
///         inspired by Linux Sidewinder driver implementation. See
///         https://github.com/torvalds/linux/blob/master/drivers/input/joystick/sidewinder.c
class Sidewinder {
public:
  /// Supported Sidewinder model types.
  enum class Model {
    /// Unknown model.
    SW_UNKNOWN,

    /// Sidewinder GamePad
    SW_GAMEPAD,

    /// Sidewinder 3D Pro
    SW_3D_PRO,

    /// Sidewinder Precision Pro
    SW_PRECISION_PRO,

    /// Sidewinder Force Feedback Wheel
    SW_FORCE_FEEDBACK_WHEEL
  };

  /// Joystick state.
  struct State {
    uint16_t buttons{0};
    uint16_t axis[4]{0};
    uint16_t hat;
  };

  /// Gets the detected model.
  /// @returns the detected joystick model
  Model getModel() const {
    return m_model;
  }

  /// Resets the joystick and tries to detect the model.
  void reset() {
    log("Trying to reset...");
    cooldown();
    m_model = guessModel(readPacket());
    while (m_model == Model::SW_UNKNOWN) {
      // No data. 3d Pro analog mode?
      enableDigitalMode();
      m_model = guessModel(readPacket());
    }
    log("Detected model %d", m_model);
  }

  /// Reads joystick state.
  /// @returns the state of axis, buttons etc.
  /// @remark if reading the state fails, the last known state is
  ///         returned and the joystick reset is executed.
  State readState() {
    const auto packet = readPacket();
    State state;
    if (decode(packet, state)) {
      m_state = state;
      m_errors = 0;
    } else {
      m_errors++;
      log("Packet decoding failed %d time(s)", m_errors);
      if (m_errors > MAX_ERRORS) {
        reset();
      }
    }
    return m_state;
  }

  /// Joystick cool down timeout.
  ///
  /// This is need to settle the signals between the reads.
  void cooldown() const {
    m_trigger.setLow();
    delayMicroseconds(2000);
  }

private:
  enum {
    PULSE_DURATION = 60,
    MAX_ERRORS = 3,
  };

  /// Interrupt guard (RAII).
  ///
  /// This class is used to deactivate the interrupts in performance
  /// critical sections. The interrupt is reactivated as soon as this
  /// guard runs out of scope.
  struct InterruptStopper {
    InterruptStopper() {
      noInterrupts();
    }
    ~InterruptStopper() {
      interrupts();
    }
  };

  /// Internal bit structure which is filled by reading from the joystick.
  struct Packet {
    byte bits[128] {0};
    uint16_t length{0u};

    // Prints the 64 bits of the packet data
    // Used mainly for debugging
    void print() const {
      uint64_t result{0};
      for (auto i = 0u; i < length; i++) {
        result |= uint64_t(bits[i] & 0b111) << (i * 3);
      }

      Serial.print("Data Packet: ");
      Serial.print(String(uint32_t((result & 0xFFFFFFFF00000000) >> 32), BIN));
      Serial.println(String(uint32_t((result & 0x00000000FFFFFFFF)), BIN));
    }
  };
  /// Model specific status decoder function.
  template <Model M>
  struct Decoder {
    static bool decode(const Packet &packet, State &state);
  };

  /// Guesses joystick model from the size of the packet.
  static Model guessModel(const Packet &packet) {
    switch (packet.length) {
      case 15:
        return Model::SW_GAMEPAD;
      case 16:
        return Model::SW_PRECISION_PRO;
      case 11:
        return Model::SW_FORCE_FEEDBACK_WHEEL;
      case 64:
        return Model::SW_3D_PRO;
      default:
        return Model::SW_UNKNOWN;
    }
  }

  DigitalInput<GamePort<2>::pin, true> m_clock;
  DigitalInput<GamePort<7>::pin, true> m_data0;
  DigitalInput<GamePort<10>::pin, true> m_data1;
  DigitalInput<GamePort<14>::pin, true> m_data2;
  DigitalOutput<GamePort<3>::pin> m_trigger;
  Model m_model{Model::SW_UNKNOWN};
  State m_state;
  int m_errors{0u};

  /// Enables digital mode for 3D Pro.
  //
  /// The 3D Pro can work as legacy analog joystick or in digital mode.
  /// This mode has to be activated explicitly. In this function timing
  /// is very important. See Patent: US#5628686 (page 19) for details.
  void enableDigitalMode() const {
    static const int magic = 200;
    static const int seq[] = {magic, magic + 725, magic + 300, 0};
    log("Trying to enable digital mode");
    cooldown();
    InterruptStopper interruptStopper;
    for (auto i = 0u; seq[i]; i++) {
      m_trigger.pulse(PULSE_DURATION);
      delayMicroseconds(seq[i]);
    }
    m_trigger.pulse(PULSE_DURATION);
    cooldown();
  }

  /// Read bits packet from the joystick.
  ///
  /// This part is extremely performance and timing critical. Change only, if
  /// you know, what you are doing.
  Packet readPacket() const {

    InterruptStopper interruptStopper;
    const auto ready = m_clock.isHigh();
    m_trigger.setHigh();
    // We are reading into a byte array instead of an uint64_t, because of two
    // reasons. First, bits packets can be larger, than 64 bits. We are actually
    // not interested in packets, which are larger than that, but may be in the
    // future we'd need to handle them as well. Second, for reading into an
    // uint64_t we would need to shift between the clock impulses, which is
    // impossible to do in time. Unfortunately this shift is extremely slow on
    // an Arduino and it's just faster to write into an array. One bit per byte.
    Packet packet;
    if (ready || m_clock.wait(Edge::rising, PULSE_DURATION * 10)) {
      while (packet.length < sizeof(packet.bits)) {
        if (!m_clock.wait(Edge::rising, PULSE_DURATION)) {
          break;
        }
        packet.bits[packet.length++] = m_data0.get() | (m_data1.get() << 1) | (m_data2.get() << 2);
      }
    }
    m_trigger.setLow();
    delayMicroseconds(PULSE_DURATION);
    return packet;
  }

  /// Decodes bit packet into a state.
  bool decode(const Packet &packet, State &state) const;
};

/// Bit decoder for Sidewinder GamePad.
template <>
class Sidewinder::Decoder<Sidewinder::Model::SW_GAMEPAD> {
public:
  static bool decode(const Sidewinder::Packet &packet, Sidewinder::State &state) {

    const auto checksum = [&]() {
      byte result = 0u;
      for (auto i = 0u; i < packet.length; i++) {
        result ^= packet.bits[i] & 1;
      }
      return result;
    };

    if (packet.length != 15 || checksum() != 0) {
      return false;
    }

    // Bit 0-1: x-axis (10-left, 01-right, 11-middle)
    // Bit 2-3: y-axis (01-up, 10-down, 11-middle)
    // Bit 4-13: 10 buttons
    // Bit 14: checksum
    for (auto i = 0u; i < 10; i++) {
      state.buttons |= (~packet.bits[i + 4] & 1) << i;
    }
    state.axis[0] = 1 + packet.bits[3] - packet.bits[2];
    state.axis[1] = 1 + packet.bits[0] - packet.bits[1];

    return true;
  }
};

/// Bit decoder for Sidewinder 3D Pro.
template <>
class Sidewinder::Decoder<Sidewinder::Model::SW_3D_PRO> {
public:
  static bool decode(const Sidewinder::Packet &packet, Sidewinder::State &state) {

    const auto value = [&]() {
      uint64_t result{0u};
      for (auto i = 0u; i < packet.length; i++) {
        result |= uint64_t(packet.bits[i] & 1) << i;
      }
      return result;
    }();

    const auto bits = [&](uint8_t start, uint8_t length) {
      const auto mask = (1 << length) - 1;
      return (value >> start) & mask;
    };

    if (packet.length != 64 || !checkSync(value) || checksum(value)) {
      return false;
    }

    // bit 38: button 8 + bit 8-15: buttons 1-7 (low active)
    state.buttons = ~(bits(8, 7) | (bits(38, 1) << 7));

    // bit 3-5 + bit 16-22: x-axis (value 0-1023)
    state.axis[0] = bits(3, 3) << 7 | bits(16, 7);

    // bit 0-2 + bit 24-30: y-axis (value 0-1023)
    state.axis[1] = bits(0, 3) << 7 | bits(24, 7);

    // bit 35-36 + bit 40-46: z-axis (value 0-511)
    state.axis[2] = bits(35, 2) << 7 | bits(40, 7);

    // bit 32-34 + bit 48-54: throttle-axis (value 0-1023)
    state.axis[3] = bits(32, 3) << 7 | bits(48, 7);

    // bit 6-7 + bit 60-62 (9 pos, 0 center, 1-8 clockwise)
    state.hat = bits(6, 1) << 3 | bits(60, 3);

    return true;
  }

private:
  /// Checks sync bits.
  ///
  /// This code was taken from Linux driver as is.
  static bool checkSync(uint64_t value) {
    return !((value & 0x8080808080808080ULL) ^ 0x80);
  }

  /// Calculates checksum.
  ///
  /// This code was taken from Linux driver as is.
  static byte checksum(uint64_t value) {
    auto result = 0u;
    while (value) {
      result += value & 0xf;
      value >>= 4;
    }
    return result & 0xf;
  }
};

/// Bit decoder for Sidewinder Precision Pro
template <>
class Sidewinder::Decoder<Sidewinder::Model::SW_PRECISION_PRO> {
public:
  static bool decode(const Sidewinder::Packet &packet, Sidewinder::State &state) {

    const auto value = [&]() {
      uint64_t result{0u};
      for (auto i = 0u; i < packet.length; i++) {
        result |= uint64_t(packet.bits[i] & 0b111) << (i * 3);
      }
      return result;
    }();

    // TODO shared code with 3D Pro?
    const auto bits = [&](uint8_t start, uint8_t length) {
      const auto mask = (1 << length) - 1;
      return (value >> start) & mask;
    };

    // TODO shared code with GP?
    const auto parity = [](uint64_t t) {
      uint32_t x = t ^ (t >> 32);
      x ^= x >> 16;
      x ^= x >> 8;
      x ^= x >> 4;
      x ^= x >> 2;
      x ^= x >> 1;
      return x & 1;
    };

    if (packet.length != 16 || !parity(value)) {
      return false;
    }

    state.axis[0] = bits(9, 10);
    state.axis[1] = bits(19, 10);
    state.axis[2] = bits(36, 6);
    state.axis[3] = bits(29, 7);
    state.hat = bits(42, 4);
    state.buttons = ~bits(0, 9);
    return true;
  }
};

/// Bit decoder for Sidewinder Force Feedback Wheel.
template <>
class Sidewinder::Decoder<Sidewinder::Model::SW_FORCE_FEEDBACK_WHEEL> {
public:
  static bool decode(const Sidewinder::Packet &packet, Sidewinder::State &state) {

    const auto value = [&]() {
      uint64_t result{0u};
      for (auto i = 0u; i < packet.length; i++) {
        result |= uint64_t(packet.bits[i] & 0b111) << (i*3);
      }
      return result;
    }();

    const auto parity = [](uint64_t t) {
      uint32_t x = t ^ (t >> 32);
      x ^= x >> 16;
      x ^= x >> 8;
      x ^= x >> 4;
      x ^= x >> 2;
      x ^= x >> 1;
      return x & 1;
    };

    const auto bits = [&](uint8_t start, uint8_t length) {
      const auto mask = (1 << length) - 1;
      return (value >> start) & mask;
    };

    if (packet.length != 11 || !parity(value)) {
      return false;
    }

    // bit 0-9: RX
    state.axis[0] = bits(0, 10);

    // bit 10-16: Rudder
    state.axis[1] = bits(10, 6);

    // bit 16-21: Throttle
    state.axis[2] = bits(16, 6);

    // bit 22-29: buttons 1-8
    state.buttons = ~bits(22, 8);

    return true;
  }
};


inline bool Sidewinder::decode(const Packet &packet, State &state) const {
  switch (m_model) {
    case Model::SW_GAMEPAD:
      return Decoder<Model::SW_GAMEPAD>::decode(packet, state);
    case Model::SW_3D_PRO:
      return Decoder<Model::SW_3D_PRO>::decode(packet, state);
    case Model::SW_PRECISION_PRO:
      return Decoder<Model::SW_PRECISION_PRO>::decode(packet, state);
    case Model::SW_FORCE_FEEDBACK_WHEEL:
      return Decoder<Model::SW_FORCE_FEEDBACK_WHEEL>::decode(packet, state);
    case Model::SW_UNKNOWN:
      break;
  }
  return false;
}
