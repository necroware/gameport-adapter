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
#include "GamePort.h"
#include "Joystick.h"
#include "Utilities.h"

/// Class to communicate with ThrustMaster DirectConnect (TMDC/BSP) joysticks.
/// @remark This is a green field implementation, but it was heavily
///         inspired by the Linux ThrustMaster DirectConnect driver. See
///         https://github.com/torvalds/linux/blob/master/drivers/input/joystick/tmdc.c
///
/// Protocol overview (port 0):
///   - GamePort pin 7 (Button 2): strobe line — falling edge marks each bit slot
///   - GamePort pin 2 (Button 1): data line   — sampled on each strobe falling edge
///   - GamePort pin 3 (Joy1-X):   trigger     — a pulse starts a 13-byte transmission
///
/// Each byte is framed as: [start: data HIGH] [8 data bits, LSB first, LOW=1] [stop: data LOW]
class TMDC : public Joystick {
public:
  bool init() override {
    uint8_t data[PACKET_LEN];

    while (!readPacket(data))
      delay(200);

    m_deviceType = static_cast<DeviceType>(data[BYTE_ID]);
    return true;
  }

  bool update() override {
    uint8_t data[PACKET_LEN];
    if (!readPacket(data) || static_cast<DeviceType>(data[BYTE_ID]) != m_deviceType) {
      return false;
    }
    parsePacket(data);
    return true;
  }

  const State &getState() const override {
    return m_state;
  }

  const Description &getDescription() const override {
    switch (m_deviceType) {
        // Note, only the Millennium 3D is tested. Button/axis mappings may need to be revised by folks with actual
        // controllers.
      case DeviceType::Millenium3D: {
        static const Description d{"ThrustMaster Millennium 3D Inceptor", 4, 6, true};
        return d;
      } // Verified
      case DeviceType::Rage3D: {
        static const Description d{"ThrustMaster Rage 3D Gamepad", 2, 10, false};
        return d;
      }
      case DeviceType::AttackThrottle: {
        static const Description d{"ThrustMaster Attack Throttle", 4, 10, true};
        return d;
      }
      case DeviceType::FragMaster: {
        static const Description d{"ThrustMaster FragMaster", 4, 10, false};
        return d;
      }
      case DeviceType::Fusion: {
        static const Description d{"Thrustmaster Fusion GamePad", 2, 10, false};
        return d;
      }
      default: {
        static const Description d{"ThrustMaster Generic TMDC", 2, 4, false};
        return d;
      }
    }
  }

private:
  static constexpr uint8_t PACKET_LEN = 13;
  static constexpr uint8_t BYTE_ID = 10;

  enum class DeviceType : uint8_t {
    // Device mode IDs reported in packet byte 10 (TMDC_BYTE_ID)
    Uninitialized = 0,
    Millenium3D = 1,
    Rage3D = 3,
    AttackThrottle = 4,
    FragMaster = 8,
    Fusion = 163
  };

  // GamePort pin 7 (Button 2): TMDC strobe/clock line for port 0
  // In the Linux driver: bit 1 of (gameport_read() >> 4); falling edge = ~v & u & 2
  DigitalInput<GamePort<7>::pin, true> m_strobe;

  // GamePort pin 2 (Button 1): TMDC serial data line for port 0
  // In the Linux driver: bit 0 of (gameport_read() >> 4); LOW=1, HIGH=0
  DigitalInput<GamePort<2>::pin, true> m_data;

  // GamePort pin 3 (Joy1-X): analog timing trigger — pulse starts transmission
  mutable DigitalOutput<GamePort<3>::pin> m_trigger;

  DeviceType m_deviceType{DeviceType::Uninitialized};
  State m_state{};

  /// Reads a 13-byte TMDC packet from the joystick.
  ///
  /// The protocol uses a trigger-response model: we pulse the gameport timing
  /// line and the joystick responds with 13 bytes, each framed with a start
  /// and stop bit on the strobe line.
  ///
  /// Timeouts (wait-loop iterations at ~0.375µs each on a 16 MHz Arduino):
  ///   TMDC_MAX_START  = 600 µs  → ~1600 iterations (we use 2000 for margin)
  ///   TMDC_MAX_STROBE = 60 µs   → ~160 iterations  (we use 300 for margin)
  bool readPacket(uint8_t data[PACKET_LEN]) const {
    static constexpr uint16_t TIMEOUT_START = 2000;
    static constexpr uint16_t TIMEOUT_STROBE = 300;

    // Cooldown: hold trigger LOW and allow the joystick time to reset between
    // transmissions. The Linux driver polls at 20ms intervals; without this
    // delay rapid re-triggering may cause the joystick to miss the trigger.
    m_trigger.setLow();
    delay(3);

    // Disable interrupts BEFORE triggering so no ISR can fire in the gap
    // between the trigger pulse and the start of packet reception and cause
    // us to miss the first strobe (which must arrive within 600 µs).
    // This mirrors the approach used in Sidewinder.h.
    const InterruptStopper interruptStopper;
    /// Triggers the gameport timing circuit to start a TMDC data transmission.
    m_trigger.pulse(20);

    for (uint8_t i = 0; i < PACKET_LEN; i++) {
      // Wait for strobe falling edge — this is the start-bit slot
      const uint16_t timeout = (i == 0) ? TIMEOUT_START : TIMEOUT_STROBE;
      if (!m_strobe.wait(Edge::falling, timeout)) {
        return false;
      }
      // Start bit: data line must be HIGH (idle/inactive level)
      if (!m_data.isHigh()) {
        return false;
      }

      // Read 8 data bits, LSB first.
      // Each bit is clocked in on a strobe falling edge.
      // Data line LOW = bit value 1, data line HIGH = bit value 0.
      data[i] = 0;
      for (uint8_t bit = 0; bit < 8; bit++) {
        if (!m_strobe.wait(Edge::falling, TIMEOUT_STROBE)) {
          return false;
        }
        if (m_data.isLow()) {
          data[i] |= (1u << bit);
        }
      }

      // Stop bit: data line must be LOW (asserted level)
      if (!m_strobe.wait(Edge::falling, TIMEOUT_STROBE)) {
        return false;
      }
      if (m_data.isHigh()) {
        return false;
      }
    }

    return true;
  }

  /// Converts a HAT switch (x, y) vector to the HID HAT position encoding.
  /// HID encoding: 0=center, 1=up, 2=up-right, 3=right, ... 8=up-left (clockwise).
  static uint8_t hatXYtoHid(int8_t x, int8_t y) {
    if (x == 0 && y == 0)
      return 0; // center
    if (x == 0 && y == -1)
      return 1; // up
    if (x == 1 && y == -1)
      return 2; // up-right
    if (x == 1 && y == 0)
      return 3; // right
    if (x == 1 && y == 1)
      return 4; // down-right
    if (x == 0 && y == 1)
      return 5; // down
    if (x == -1 && y == 1)
      return 6; // down-left
    if (x == -1 && y == 0)
      return 7; // left
    if (x == -1 && y == -1)
      return 8; // up-left
    return 0;
  }

  /// Parses a received 13-byte packet into the joystick state.
  ///
  /// Packet layout (from the Linux tmdc driver):
  ///   Analog axis bytes:  {0, 1, 3, 4, 6, 7}   (tmdc_byte_a)
  ///   Digital/btn bytes:  {2, 5, 8, 9}          (tmdc_byte_d)
  ///   Byte 10: device ID, byte 11: revision, byte 12: capabilities
  ///
  /// Per-mode button layout [btnc = count, btno = start bit offset]:
  ///   Millenium3D    (id 1):  btnc={4,2}, btno={4,6}   (HAT in d[0] bits 0-3)
  ///   Rage3D         (id 3):  btnc={8,2}, btno={0,0}
  ///   AttackThrottle (id 4):  btnc={4,6}, btno={4,2}   (HAT from axis byte a[3])
  ///   FragMaster     (id 8):  btnc={8,2}, btno={0,0}
  ///   Fusion         (id163): btnc={8,2}, btno={0,0}
  void parsePacket(const uint8_t data[PACKET_LEN]) {
    static constexpr uint8_t BYTE_A[6] = {0, 1, 3, 4, 6, 7};
    static constexpr uint8_t BYTE_D[4] = {2, 5, 8, 9};

    uint8_t numAxes = 0;
    bool hasHat = false;
    uint8_t btnc[4] = {};
    uint8_t btno[4] = {};

    switch (m_deviceType) {
      case DeviceType::Millenium3D:
        numAxes = 4;
        hasHat = true;
        btnc[0] = 4;
        btno[0] = 4;
        btnc[1] = 2;
        btno[1] = 6;
        break;
      case DeviceType::Rage3D:
        numAxes = 2;
        btnc[0] = 8;
        btno[0] = 0;
        btnc[1] = 2;
        btno[1] = 0;
        break;
      case DeviceType::AttackThrottle:
        // axis index 3 (packet byte 4) carries the HAT, not an analog axis
        numAxes = 5;
        hasHat = true;
        btnc[0] = 4;
        btno[0] = 4;
        btnc[1] = 6;
        btno[1] = 2;
        break;
      case DeviceType::FragMaster:
        numAxes = 4;
        btnc[0] = 8;
        btno[0] = 0;
        btnc[1] = 2;
        btno[1] = 0;
        break;
      case DeviceType::Fusion:
        numAxes = 2;
        btnc[0] = 8;
        btno[0] = 0;
        btnc[1] = 2;
        btno[1] = 0;
        break;
      default:
        return;
    }

    // Analog axes: scale 8-bit device values (0-255) to the adapter's 0-1023 range.
    uint8_t axisIdx = 0;
    for (uint8_t i = 0; i < numAxes; i++) {
      if (m_deviceType == DeviceType::AttackThrottle && i == 3) {
        continue; // byte 4 encodes the AttackThrottle HAT switch, not a joystick axis
      }
      m_state.axes[axisIdx++] = (uint16_t)data[BYTE_A[i]] * 4;
    }

    // HAT switch
    m_state.hat = 0;
    if (hasHat) {
      if (m_deviceType == DeviceType::Millenium3D) {
        // HAT direction bits packed into the lower nibble of button byte d[0]:
        //   bit 0 = up, bit 1 = right, bit 2 = down, bit 3 = left
        const uint8_t d = data[BYTE_D[0]];
        const int8_t x = (int8_t)((d >> 3) & 1) - (int8_t)((d >> 1) & 1); // left - right
        const int8_t y = (int8_t)((d >> 2) & 1) - (int8_t)(d & 1);        // down - up
        m_state.hat = hatXYtoHid(x, y);
      } else if (m_deviceType == DeviceType::AttackThrottle) {
        // HAT encoded as an analog value in axis byte a[3] (packet byte 4).
        // Five positions spaced ~25 counts apart starting at 141.
        // Index: 0=center, 1=right, 2=up, 3=left, 4=down
        static constexpr int8_t HAT_X[5] = {0, 1, 0, -1, 0};
        static constexpr int8_t HAT_Y[5] = {0, 0, -1, 0, 1};
        const uint8_t raw = data[BYTE_A[3]];
        if (raw >= 141) {
          const uint8_t idx = (raw - 141) / 25;
          if (idx < 5) {
            m_state.hat = hatXYtoHid(HAT_X[idx], HAT_Y[idx]);
          }
        }
      }
    }

    // Buttons: each digital byte contributes btnc[k] buttons starting at bit btno[k].
    m_state.buttons = 0;
    uint8_t btnIdx = 0;
    for (uint8_t k = 0; k < 4; k++) {
      for (uint8_t i = 0; i < btnc[k]; i++) {
        if ((data[BYTE_D[k]] >> (btno[k] + i)) & 1) {
          m_state.buttons |= (1u << btnIdx);
        }
        btnIdx++;
      }
    }
  }
};
