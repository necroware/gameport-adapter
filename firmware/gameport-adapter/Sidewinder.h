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

#include "Buffer.h"
#include "DigitalPin.h"
#include "Joystick.h"
#include "Utilities.h"

/// Class to for communication with Sidewinder joysticks.
/// @remark This is a green field implementation, but it was heavily
///         inspired by Linux Sidewinder driver implementation. See
///         https://github.com/torvalds/linux/blob/master/drivers/input/joystick/sidewinder.c
class Sidewinder : public Joystick {
public:
  /// Resets the joystick and tries to detect the model.
  bool init() override {
    log("Sidewinder init...");
    m_errors = 0;

    m_model = guessModel(readPacket());
    while (m_model == Model::SW_UNKNOWN) {
      // No data. 3d Pro analog mode?
      enableDigitalMode();
      m_model = guessModel(readPacket());
    }
    log("Detected model %d", m_model);
    return true;
  }

  bool update() override {
    const auto packet = readPacket();
    State state;
    if (decode(packet, state)) {
      m_state = state;
      m_errors = 0;
      return true;
    }

    m_errors++;
    log("Packet decoding failed %d time(s)", m_errors);
    if (m_errors > 5) {
      return init();
    }
    return false;
  }

  const State &getState() const override {
    return m_state;
  }

  const Description &getDescription() const override;

private:
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

    /// Sidewinder Force Feedback Pro
    SW_FORCE_FEEDBACK_PRO,

    /// Sidewinder Force Feedback Wheel
    SW_FORCE_FEEDBACK_WHEEL
  };

  /// Internal bit structure which is filled by reading from the joystick.
  using Packet = Buffer<128u>;

  /// Model specific status decoder function.
  template <Model M>
  struct Decoder {
    static const Description &getDescription();
    static bool decode(const Packet &packet, State &state);
  };

  /// Guesses joystick model from the size of the packet.
  Model guessModel(const Packet &packet) const {
    log("Guessing model by packet size of %d", packet.size);
    switch (packet.size) {
      case 15:
        return Model::SW_GAMEPAD;
      case 16:   // 3bit mode
      case 48: { // 1bit mode
          const auto id = readID(packet.size);
          log("Data packet size is ambiguous. Guessing by ID %d", id);
          if (id == 14) {
            return Model::SW_FORCE_FEEDBACK_PRO;
          }
          return Model::SW_PRECISION_PRO;
        }
      case 11: // 3bit mode
      case 33: // 1bit mode
        return Model::SW_FORCE_FEEDBACK_WHEEL;
      case 64:
        return Model::SW_3D_PRO;
      default:
        return Model::SW_UNKNOWN;
    }
  }

  void cooldown() const {
    m_trigger.setLow();
    delay(3);
  }

  void trigger() const {
    m_trigger.pulse(20);
  }

  DigitalInput<GamePort<2>::pin, true> m_clock;
  DigitalInput<GamePort<7>::pin, true> m_data0;
  DigitalInput<GamePort<10>::pin, true> m_data1;
  DigitalInput<GamePort<14>::pin, true> m_data2;
  DigitalOutput<GamePort<3>::pin> m_trigger;
  Model m_model{Model::SW_UNKNOWN};
  State m_state{};
  uint8_t m_errors{};

  /// Enables digital mode for 3D Pro.
  //
  /// The 3D Pro can work as legacy analog joystick or in digital mode.
  /// This mode has to be activated explicitly. In this function timing
  /// is very important. See Patent: US#5628686 (page 19) for details.
  void enableDigitalMode() const {
    static const uint16_t magic = 150;
    static const uint16_t seq[] = {magic, magic + 725, magic + 300, magic, 0};
    log("Trying to enable digital mode");
    cooldown();
    const InterruptStopper interruptStopper;
    for (auto i = 0u; seq[i]; i++) {
      trigger();
      delayMicroseconds(seq[i]);
    }
  }

  /// Read bits packet from the joystick.
  ///
  /// This part is extremely performance and timing critical. Change only, if
  /// you know, what you are doing.
  Packet readPacket() const {

    // Packet instantiation is a very expensive call, which zeros the memory.
    // The instantiation should therefore happen outside of the interrupt stopper
    // and before triggering the device. Otherwise the clock will come before
    // the packet was zeroed/instantiated.
    Packet packet;

    // We are reading into a byte array instead of an uint64_t, because of two
    // reasons. First, bits packets can be larger, than 64 bits. We are actually
    // not interested in packets, which are larger than that, but may be in the
    // future we'd need to handle them as well. Second, for reading into an
    // uint64_t we would need to shift between the clock impulses, which is
    // impossible to do in time. Unfortunately this shift is extremely slow on
    // an Arduino and it's just faster to write into an array. One bit per byte.
    packet.size = readBits(Packet::MAX_SIZE, [this, &packet](uint8_t pos) {
      const auto b1 = m_data0.read();
      const auto b2 = m_data1.read();
      const auto b3 = m_data2.read();
      packet.data[pos] = bool(b1) | bool(b2) << 1 | bool(b3) << 2;
    });

    return packet;
  }

  uint8_t readID(uint8_t dataPacketSize) const {

    const auto rise = dataPacketSize / 2 - 1;
    const auto fall = rise + 2;

    const auto count = readBits(255u, [this, rise, fall](uint8_t pos) {
      if (pos == rise) {
        m_trigger.setHigh();
      }
      else if (pos == fall) {
        m_trigger.setLow();
      }
    });
    return count < dataPacketSize ? 0 : count - dataPacketSize;
  }

  template <typename T>
  uint8_t readBits(uint8_t maxCount, T&& extract) const {
    static const uint8_t wait_duration = 100;
    uint8_t count{};
    cooldown();
    // WARNING: Here starts the timing critical section
    const InterruptStopper interruptStopper;
    trigger();
    if (m_clock.wait(true, wait_duration)) {
      while(count < maxCount && m_clock.wait(Edge::rising, wait_duration)) {
        extract(count++);
      }
    }
    return count;
  }

  /// Decodes bit packet into a state.
  bool decode(const Packet &packet, State &state) const;
};

/// Placeholder for Unknown Device
template <>
class Sidewinder::Decoder<Sidewinder::Model::SW_UNKNOWN> {
public:
  static const Description &getDescription() {
    static const Description desc{"Unknown", 0, 0, 0};
    return desc;
  }

  static bool decode(const Packet &, State &) {
    return false;
  }
};

/// Bit decoder for Sidewinder GamePad.
template <>
class Sidewinder::Decoder<Sidewinder::Model::SW_GAMEPAD> {
public:
  static const Description &getDescription() {
    static const Description desc{"MS Sidewinder GamePad", 2, 10, 0};
    return desc;
  }

  static bool decode(const Packet &packet, State &state) {

    const auto checksum = [&]() {
      byte result = 0u;
      for (auto i = 0u; i < packet.size; i++) {
        result ^= packet.data[i] & 1;
      }
      return result;
    };

    if (packet.size != 15 || checksum() != 0) {
      return false;
    }

    // Bit 0-1: x-axis (10-left, 01-right, 11-middle)
    // Bit 2-3: y-axis (01-up, 10-down, 11-middle)
    // Bit 4-13: 10 buttons
    // Bit 14: checksum
    for (auto i = 0u; i < 10; i++) {
      state.buttons |= (~packet.data[i + 4] & 1) << i;
    }
    state.axes[0] = map(1 + packet.data[3] - packet.data[2], 0, 2, 0, 1023);
    state.axes[1] = map(1 + packet.data[0] - packet.data[1], 0, 2, 0, 1023);

    return true;
  }
};

/// Bit decoder for Sidewinder 3D Pro.
template <>
class Sidewinder::Decoder<Sidewinder::Model::SW_3D_PRO> {
public:
  static const Description &getDescription() {
    static const Description desc{"MS Sidewinder 3D Pro", 4, 8, 1};
    return desc;
  }

  static bool decode(const Packet &packet, State &state) {
    const auto value = [&]() {
      uint64_t result{0u};
      for (auto i = 0u; i < packet.size; i++) {
        result |= uint64_t(packet.data[i] & 1) << i;
      }
      return result;
    }();

    const auto bits = [&](uint8_t start, uint8_t length) {
      const auto mask = (1 << length) - 1;
      return (value >> start) & mask;
    };

    if (packet.size != 64 || !checkSync(value) || checksum(value)) {
      return false;
    }

    // bit 38: button 8 + bit 8-15: buttons 1-7 (low active)
    state.buttons = ~(bits(8, 7) | (bits(38, 1) << 7));

    // bit 3-5 + bit 16-22: x-axis (value 0-1023)
    state.axes[0] = bits(3, 3) << 7 | bits(16, 7);

    // bit 0-2 + bit 24-30: y-axis (value 0-1023)
    state.axes[1] = bits(0, 3) << 7 | bits(24, 7);

    // bit 35-36 + bit 40-46: z-axis (value 0-511)
    state.axes[2] = map(bits(35, 2) << 7 | bits(40, 7), 0, 511, 0, 1023);

    // bit 32-34 + bit 48-54: throttle-axis (value 0-1023)
    state.axes[3] = bits(32, 3) << 7 | bits(48, 7);

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
  static const Description &getDescription() {
    static const Description desc{"MS Sidewinder Precision Pro", 4, 9, 1};
    return desc;
  }

  static bool decode(const Packet &packet, State &state) {

    // The packet can be either in 3bit or in 1bit mode
    if (packet.size != 16 && packet.size != 48) {
        return false;
    }

    const auto value = [&packet]() {
      uint64_t result{0u};
      const auto shift = 48 / packet.size;
      const auto mask = (shift == 3) ? 0b111 : 0b1;
      for (auto i = 0u; i < packet.size; i++) {
        result |= uint64_t(packet.data[i] & mask) << (i * shift);
      }
      return result;
    }();

    // TODO shared code with 3D Pro?
    const auto bits = [&value](uint8_t start, uint8_t length) {
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

    if (!parity(value)) {
      return false;
    }

    state.axes[0] = bits(9, 10);
    state.axes[1] = bits(19, 10);
    state.axes[2] = map(bits(36, 6), 0, 63, 0, 1023);
    state.axes[3] = map(bits(29, 7), 0, 127, 0, 1023);
    state.hat = bits(42, 4);
    state.buttons = ~bits(0, 9);

    return true;
  }
};

/// Descriptor for Sidewinder Force Feedback Pro.
/// (The bit decoder is identical to the Precision Pro.)
template <>
class Sidewinder::Decoder<Sidewinder::Model::SW_FORCE_FEEDBACK_PRO> {
public:
  static const Description &getDescription() {
    static const Description desc{"MS Sidewinder Force Feedback Pro", 4, 9, 1};
    return desc;
  }

  static bool decode(const Packet &packet, State &state) {
    // Decode is identical between the Force Feedback Pro and the Precision Pro.
    return Decoder<Model::SW_PRECISION_PRO>::decode(packet, state);
  }
};

/// Bit decoder for Sidewinder Force Feedback Wheel.
template <>
class Sidewinder::Decoder<Sidewinder::Model::SW_FORCE_FEEDBACK_WHEEL> {
public:
  static const Description &getDescription() {
    static const Description desc{"MS ForceFeedBack Wheel", 3, 8, 0};
    return desc;
  }

  static bool decode(const Packet &packet, State &state) {

    // The packet can be either in 3bit or in 1bit mode
    if (packet.size != 11 && packet.size != 33) {
        return false;
    }

    const auto value = [&packet]() {
      uint64_t result{0u};
      const auto shift = 33 / packet.size;
      const auto mask = (shift == 3) ? 0b111 : 0b1;
      for (auto i = 0u; i < packet.size; i++) {
        result |= uint64_t(packet.data[i] & mask) << (i * shift);
      }
      return result;
    }();

    // TODO shared code with 3D Pro?
    const auto bits = [&value](uint8_t start, uint8_t length) {
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

    if (!parity(value)) {
      return false;
    }

    // bit 0-9: RX
    state.axes[0] = bits(0, 10);

    // bit 10-16: Rudder
    state.axes[1] = map(bits(10, 6), 0, 63, 0, 1023);

    // bit 16-21: Throttle
    state.axes[2] = map(bits(16, 6), 0, 63, 0, 1023);

    // bit 22-29: buttons 1-8
    state.buttons = ~bits(22, 8);

    return true;
  }
};

inline const Joystick::Description &Sidewinder::getDescription() const {
  switch (m_model) {
    case Model::SW_GAMEPAD:
      return Decoder<Model::SW_GAMEPAD>::getDescription();
    case Model::SW_3D_PRO:
      return Decoder<Model::SW_3D_PRO>::getDescription();
    case Model::SW_PRECISION_PRO:
      return Decoder<Model::SW_PRECISION_PRO>::getDescription();
    case Model::SW_FORCE_FEEDBACK_PRO:
      return Decoder<Model::SW_FORCE_FEEDBACK_PRO>::getDescription();
    case Model::SW_FORCE_FEEDBACK_WHEEL:
      return Decoder<Model::SW_FORCE_FEEDBACK_WHEEL>::getDescription();
    default:
      return Decoder<Model::SW_UNKNOWN>::getDescription();
  }
}

inline bool Sidewinder::decode(const Packet &packet, State &state) const {
  switch (m_model) {
    case Model::SW_GAMEPAD:
      return Decoder<Model::SW_GAMEPAD>::decode(packet, state);
    case Model::SW_3D_PRO:
      return Decoder<Model::SW_3D_PRO>::decode(packet, state);
    case Model::SW_PRECISION_PRO:
      return Decoder<Model::SW_PRECISION_PRO>::decode(packet, state);
    case Model::SW_FORCE_FEEDBACK_PRO:
      return Decoder<Model::SW_FORCE_FEEDBACK_PRO>::decode(packet, state);
    case Model::SW_FORCE_FEEDBACK_WHEEL:
      return Decoder<Model::SW_FORCE_FEEDBACK_WHEEL>::decode(packet, state);
    default:
      return Decoder<Model::SW_UNKNOWN>::decode(packet, state);
  }
}
