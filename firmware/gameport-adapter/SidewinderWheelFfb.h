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

#include "ForceFeedback.h"

#include <stdint.h>
#include <string.h>

/// Force feedback protocol of the Sidewinder Force Feedback Wheel.
///
/// The wheel receives MIDI-like messages on gameport pin 12. The protocol was
/// reverse engineered by the adapt-ffb-joy project, see
/// https://github.com/tloimu/adapt-ffb-joy/blob/wiki/SidewinderFFBWheel.md
///
/// The adapter uploads a constant force effect and the wheel's own damper,
/// inertia and friction effects. It modifies the magnitude and direction of
/// the constant force and uploads the other effects again, when their
/// coefficients change, see docs/adr/0001-sidewinder-ffb-wheel-force-feedback.md
///
/// @tparam Midi provides write(const uint8_t *data, uint8_t size)
template <typename Midi>
class SidewinderWheelFfb {
public:
  using Coefficients = ForceFeedback::Coefficients;

  Midi midi;

  /// Initializes force feedback mode, auto centering stays enabled.
  void init() {
    static const uint8_t init[] = {
        0xf3, 0x1d,                         // auto center on
        0xf1, 0x0e, 0x43, 0x01, 0x00, 0x7d, // modify effect 1, attribute 3
        0xf1, 0x7e, 0x04, 0x01, 0x3e, 0x4e, // modify effect 1, attribute 4
        0xf1, 0x1c, 0x45, 0x01, 0x3e, 0x2f, // modify effect 1, attribute 5
        0xf1, 0x0b, 0x46, 0x01, 0x7d, 0x00, // modify effect 1, attribute 6
    };
    midi.write(init, sizeof(init));
    // After an adapter reset, the wheel may still hold our effects
    deleteEffects();
  }

  /// Updates the forces of the wheel.
  ///
  /// While an application uses force feedback, the wheel's auto centering is
  /// turned off, the force is played by a constant force effect and the
  /// conditions by the wheel's own condition effects.
  ///
  /// @param[in] active is true while an application uses force feedback
  /// @param[in] force is between -10000 (left) and 10000 (right)
  /// @param[in] damper, inertia and friction are the coefficients of the
  ///            conditions. The wheel's friction has only one coefficient,
  ///            the average of both is used.
  void update(bool active, int16_t force, Coefficients damper = {}, Coefficients inertia = {},
              Coefficients friction = {}) {
    const bool clockwise = force >= 0;
    const uint8_t magnitude = uint32_t(clockwise ? force : -force) * 127 / 10000;
    const uint8_t frictionByte = limit(0x3f + (int32_t(friction.positive) + friction.negative) * 0x3f / 20000, 0x7e);
    const uint8_t conditions[CONDITIONS][2] = {
        {negativeByte(damper.negative), positiveByte(damper.positive)},
        {negativeByte(inertia.negative), positiveByte(inertia.positive)},
        {frictionByte, frictionByte},
    };

    if (active != m_active) {
      m_active = active;
      m_clockwise = clockwise;
      m_magnitude = magnitude;
      memcpy(m_conditions, conditions, sizeof(conditions));
      if (active) {
        start();
      } else {
        stop();
      }
      return;
    }

    if (!active) {
      return;
    }
    if (clockwise != m_clockwise) {
      m_clockwise = clockwise;
      sendModify(CONSTANT_FORCE_ID, DIRECTION, clockwise ? CLOCKWISE : COUNTER_CLOCKWISE);
    }
    if (magnitude != m_magnitude) {
      m_magnitude = magnitude;
      sendModify(CONSTANT_FORCE_ID, MAGNITUDE, magnitude);
    }
    for (auto i = 0u; i < CONDITIONS; i++) {
      if (memcmp(conditions[i], m_conditions[i], 2) != 0) {
        // The wheel ignores most coefficient modifications, so the effect is
        // uploaded again. This needs a rate limit, if games change conditions
        // continuously.
        memcpy(m_conditions[i], conditions[i], 2);
        sendCommand(DELETE, FIRST_CONDITION_ID + i);
        sendCondition(i);
      }
    }
  }

  /// Uploads an effect to the wheel.
  /// @param[in] data is the effect data starting with 0x20, without checksum
  void sendEffect(const uint8_t *data, uint8_t size) {
    static const uint8_t header[] = {0xf0, 0x00, 0x01, 0x0a, 0x15};
    midi.write(header, sizeof(header));
    midi.write(data, size);
    uint16_t sum = 0;
    for (auto i = 0u; i < size; i++) {
      sum += data[i];
    }
    const uint8_t footer[] = {checksum(sum), 0xf7};
    midi.write(footer, sizeof(footer));
  }

  /// Modifies a parameter of an effect, which was already uploaded.
  /// @param[in] value is a 14 bit value
  void sendModify(uint8_t effectId, uint8_t attribute, uint16_t value) {
    const uint8_t lsb = value & 0x7f;
    const uint8_t msb = (value >> 7) & 0x7f;
    // The wheel ignores non-zero values without the 0x40 flag and zero
    // values with it.
    const uint8_t flag = value ? 0x40 : 0x00;
    const uint8_t message[] = {
        0xf1, checksum(0xf1 + attribute + effectId + lsb + msb), uint8_t(flag | attribute), effectId, lsb, msb};
    midi.write(message, sizeof(message));
  }

private:
  /// The wheel assigns IDs in upload order, starting at 2, and reuses the
  /// IDs of deleted effects. The condition effects follow the constant force.
  static const uint8_t CONSTANT_FORCE_ID{2};
  static const uint8_t FIRST_CONDITION_ID{3};
  /// Damper, inertia and friction, in upload order.
  static const uint8_t CONDITIONS{3};
  static const uint8_t FRICTION_INDEX{2};
  /// Duration of an effect, which plays until it gets deleted.
  static const uint16_t INFINITE_DURATION{0};

  static const uint8_t CONSTANT_FORCE{0x06};
  static const uint8_t DAMPER{0x09};
  static const uint8_t MAGNITUDE{6};
  static const uint8_t DIRECTION{9};
  static const uint8_t CLOCKWISE{0x7d};
  static const uint8_t COUNTER_CLOCKWISE{0x00};
  static const uint8_t DELETE{1};
  static const uint8_t PLAY{2};

  bool m_active{};
  bool m_clockwise{};
  uint8_t m_magnitude{};
  /// Coefficient bytes of the conditions: turning left, turning right
  uint8_t m_conditions[CONDITIONS][2]{};

  static uint8_t checksum(uint16_t sum) {
    return (0x80 - (sum & 0x7f)) & 0x7f;
  }

  static uint8_t limit(int16_t value, uint8_t max) {
    return value < 0 ? 0 : value > max ? max : value;
  }

  /// Coefficient bytes of damper and inertia, measured on the wheel: 0x3e is
  /// no effect, 0x00 (turning right) and 0x7d (turning left) the full effect.
  static uint8_t positiveByte(int16_t coefficient) {
    return limit(0x3e - int32_t(coefficient) * 0x3e / 10000, 0x7d);
  }
  static uint8_t negativeByte(int16_t coefficient) {
    return limit(0x3e + int32_t(coefficient) * 0x3f / 10000, 0x7d);
  }

  void start() {
    static const uint8_t autoCenterOff[] = {0xf3, 0x1d, 0xf1, 0x10, 0x40, 0x00, 0x7f, 0x00, 0xf3, 0x6a};
    midi.write(autoCenterOff, sizeof(autoCenterOff));

    const uint8_t effect[] = {
        0x20,                                                 // always 0x20
        CONSTANT_FORCE,                                       // effect type
        0x7f,                                                 // always 0x7f
        INFINITE_DURATION & 0x7f,                             // duration LSB
        INFINITE_DURATION >> 7,                               // duration MSB
        0x00,                                                 // direction, unused
        0x7f,                                                 // always 0x7f
        0x7f,                                                 // envelope start level
        0x00,                                                 // envelope start time LSB
        0x00,                                                 // envelope start time MSB
        m_magnitude,                                          // force
        0x00,                                                 // envelope end time LSB
        0x00,                                                 // envelope end time MSB
        0x7f,                                                 // envelope end level
        uint8_t(m_clockwise ? CLOCKWISE : COUNTER_CLOCKWISE), // force direction
    };
    sendEffect(effect, sizeof(effect));
    sendCommand(PLAY, CONSTANT_FORCE_ID);
    for (auto i = 0u; i < CONDITIONS; i++) {
      sendCondition(i);
    }
  }

  /// Uploads and plays a damper (0), inertia (1) or friction (2).
  void sendCondition(uint8_t index) {
    const uint8_t *bytes = m_conditions[index];
    // Damper and inertia share the layout, friction ends after its only
    // coefficient.
    const bool friction = index == FRICTION_INDEX;
    const uint8_t effect[] = {
        0x20,                                // always 0x20
        uint8_t(DAMPER + index),             // effect type: damper, inertia, friction
        0x7f,                                // always 0x7f
        INFINITE_DURATION & 0x7f,            // duration LSB
        INFINITE_DURATION >> 7,              // duration MSB
        0x00,                                // direction, unused
        uint8_t(friction ? bytes[0] : 0x00), // friction coefficient or saturation (as captured)
        bytes[0],                            // coefficient turning left
        0x3e,                                // dead band, 4 bytes (as captured)
        0x3f,                                //
        0x3e,                                //
        0x3f,                                //
        0x7d,                                // saturation (as captured)
        bytes[1],                            // coefficient turning right
    };
    sendEffect(effect, friction ? 7 : sizeof(effect));
    sendCommand(PLAY, FIRST_CONDITION_ID + index);
  }

  void stop() {
    static const uint8_t autoCenterOn[] = {0xf3, 0x1d};
    deleteEffects();
    midi.write(autoCenterOn, sizeof(autoCenterOn));
  }

  void deleteEffects() {
    for (uint8_t id = CONSTANT_FORCE_ID; id < FIRST_CONDITION_ID + CONDITIONS; id++) {
      sendCommand(DELETE, id);
    }
  }

  void sendCommand(uint8_t command, uint8_t effectId) {
    const uint8_t sum = 0xf ^ 0x2 ^ command ^ (effectId >> 4) ^ (effectId & 0xf);
    const uint8_t message[] = {0xf2, uint8_t(command << 4 | sum), effectId};
    midi.write(message, sizeof(message));
  }
};
