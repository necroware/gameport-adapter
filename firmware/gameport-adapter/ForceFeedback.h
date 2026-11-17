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

#include <stdint.h>

#ifdef __AVR__
#include <avr/pgmspace.h>
#else
#define PROGMEM
#define pgm_read_word(address) (*(address))
#endif

/// USB HID PID (Physical Interface Device) force feedback.
///
/// Keeps track of the effects created by the host and calculates the
/// resulting force on the X axis. All PID effect types except custom forces
/// are offered to the host. Dampers, inertia and friction are not part of the
/// force, the wheel plays them itself, see coefficients().
/// This class has no hardware dependencies, so it can be unit tested on the
/// host.
/// See docs/adr/0001-sidewinder-ffb-wheel-force-feedback.md
class ForceFeedback {
public:
  /// Maximum number of effects the host can create.
  static const uint8_t MAX_EFFECTS{8};

  /// Strongest force, as reported by force().
  static const int16_t MAX_FORCE{10000};

  /// Report IDs of the PID reports. ID 3 is used by the joystick input report.
  enum ReportId : uint8_t {
    SET_EFFECT = 0x11,
    SET_ENVELOPE,
    SET_CONSTANT_FORCE,
    EFFECT_OPERATION,
    BLOCK_FREE,
    DEVICE_CONTROL,
    DEVICE_GAIN,
    CREATE_NEW_EFFECT,
    BLOCK_LOAD,
    POOL,
    SET_CONDITION,
    SET_PERIODIC,
    SET_RAMP_FORCE,
  };

  /// Effect types in the order of the descriptor.
  enum EffectType : uint8_t {
    CONSTANT_FORCE = 1,
    SPRING,
    DAMPER,
    INERTIA,
    FRICTION,
    RAMP,
    SQUARE,
    SINE,
    TRIANGLE,
    SAWTOOTH_UP,
    SAWTOOTH_DOWN,
  };

  /// Handles an output or feature report sent by the host.
  /// @param[in] data is the report, starting with the report ID
  /// @param[in] size is the size of the report in bytes
  /// @param[in] now is the current time in milliseconds
  void setReport(const uint8_t *data, uint8_t size, uint32_t now) {
    if (size < 2) {
      return;
    }
    switch (data[0]) {
      case CREATE_NEW_EFFECT:
        createEffect();
        return;
      case DEVICE_CONTROL:
        control(data[1]);
        return;
      case DEVICE_GAIN:
        m_gain = data[1];
        return;
    }

    // All other reports address an effect block
    if (data[1] < 1 || data[1] > MAX_EFFECTS || !m_effects[data[1] - 1].used) {
      return;
    }
    auto &effect = m_effects[data[1] - 1];
    switch (data[0]) {
      case SET_EFFECT:
        if (size >= 11) {
          effect.type = data[2];
          effect.duration = data[3] | data[4] << 8;
          effect.gain = data[7];
          effect.directionEnable = data[9] & 0b10;
          effect.direction = data[10];
        }
        break;
      case SET_CONDITION:
        if (size >= 7) {
          effect.offset = int8_t(data[2]);
          effect.positiveCoefficient = int8_t(data[3]);
          effect.negativeCoefficient = int8_t(data[4]);
          effect.saturation = data[5];
          effect.deadBand = data[6];
        }
        break;
      case SET_ENVELOPE:
        if (size >= 8) {
          effect.attackLevel = data[2];
          effect.fadeLevel = data[3];
          effect.attackTime = data[4] | data[5] << 8;
          effect.fadeTime = data[6] | data[7] << 8;
        }
        break;
      case SET_PERIODIC:
        if (size >= 9) {
          effect.magnitude = int16_t(data[2] | data[3] << 8);
          effect.periodicOffset = int16_t(data[4] | data[5] << 8);
          effect.phase = data[6];
          effect.period = data[7] | data[8] << 8;
        }
        break;
      case SET_RAMP_FORCE:
        if (size >= 6) {
          effect.magnitude = int16_t(data[2] | data[3] << 8);
          effect.rampEnd = int16_t(data[4] | data[5] << 8);
        }
        break;
      case SET_CONSTANT_FORCE:
        if (size >= 4) {
          effect.magnitude = int16_t(data[2] | data[3] << 8);
        }
        break;
      case EFFECT_OPERATION:
        if (size >= 4) {
          if (data[2] == OP_START_SOLO) {
            stopAll();
          }
          effect.playing = data[2] != OP_STOP;
          effect.start = now;
          effect.loops = data[3];
        }
        break;
      case BLOCK_FREE:
        effect = Effect{};
        break;
    }
  }

  /// Fills a feature report requested by the host.
  /// @param[in] id is the requested report ID
  /// @param[out] data receives the report, it must hold at least 5 bytes
  /// @returns the size of the report or 0 for unknown reports
  uint8_t getReport(uint8_t id, uint8_t *data) const {
    switch (id) {
      case BLOCK_LOAD:
        // ID, block index, load status, RAM pool available (unlimited)
        data[0] = BLOCK_LOAD;
        data[1] = m_loadedBlock;
        data[2] = m_loadedBlock ? LOAD_SUCCESS : LOAD_FULL;
        data[3] = data[4] = 0xff;
        return 5;
      case POOL:
        // ID, RAM pool size (unlimited), simultaneous effects, device managed pool
        data[0] = POOL;
        data[1] = data[2] = 0xff;
        data[3] = MAX_EFFECTS;
        data[4] = 1;
        return 5;
      default:
        return 0;
    }
  }

  /// Calculates the force of all playing effects.
  /// @param[in] now is the current time in milliseconds
  /// @param[in] position of the wheel between -MAX_FORCE (left) and MAX_FORCE (right)
  /// @returns force between -MAX_FORCE (left) and MAX_FORCE (right)
  int16_t force(uint32_t now, int16_t position) {
    if (!m_actuators || m_paused) {
      return 0;
    }
    int32_t total = 0;
    for (auto &effect : m_effects) {
      if (!effect.playing) {
        continue;
      }
      if (isFinished(effect, now)) {
        effect.playing = false;
        continue;
      }
      // 32 bit: int is 16 bits on AVR, magnitude * gain would overflow
      int32_t force;
      if (effect.type == SPRING) {
        force = condition(effect, position);
      } else if (effect.type == CONSTANT_FORCE || effect.type >= RAMP) {
        force = level(effect, now - effect.start);
        // Direction 0 (north) has no force on the wheel axis. Wine sends it,
        // when it has no direction for the effect and the magnitude's sign
        // gives left or right, so it is treated as no direction.
        if (effect.directionEnable && effect.direction) {
          force = force * sine(effect.direction) / MAX_FORCE;
        }
      } else {
        continue; // played by the wheel, see coefficients()
      }
      total += force * effect.gain / 255;
    }
    return clamp(total * m_gain / 255);
  }

  /// Condition effect coefficients between -MAX_FORCE and MAX_FORCE.
  struct Coefficients {
    int16_t positive; ///< when turning (or accelerating) right
    int16_t negative; ///< when turning (or accelerating) left
  };

  /// Sums the coefficients of all playing effects of the given type. The
  /// wheel plays dampers, inertia and friction itself, because the adapter
  /// measures the wheel's velocity too late to calculate stable forces.
  /// @param[in] type is DAMPER, INERTIA or FRICTION
  /// @param[in] now is the current time in milliseconds
  Coefficients coefficients(EffectType type, uint32_t now) const {
    if (!m_actuators || m_paused) {
      return {0, 0};
    }
    int32_t positive = 0;
    int32_t negative = 0;
    for (const auto &effect : m_effects) {
      if (effect.playing && effect.type == type && !isFinished(effect, now)) {
        positive += int32_t(effect.positiveCoefficient) * MAX_FORCE / 127 * effect.gain / 255;
        negative += int32_t(effect.negativeCoefficient) * MAX_FORCE / 127 * effect.gain / 255;
      }
    }
    return {clamp(positive * m_gain / 255), clamp(negative * m_gain / 255)};
  }

  /// Checks whether the host holds any effect, i.e. an application uses
  /// force feedback at the moment.
  bool active() const {
    for (const auto &effect : m_effects) {
      if (effect.used) {
        return true;
      }
    }
    return false;
  }

private:
  enum : uint8_t { OP_START = 1, OP_START_SOLO, OP_STOP };
  enum : uint8_t { LOAD_SUCCESS = 1, LOAD_FULL };
  enum : uint8_t { DC_ENABLE_ACTUATORS = 1, DC_DISABLE_ACTUATORS, DC_STOP_ALL, DC_RESET, DC_PAUSE, DC_CONTINUE };

  /// Durations of 0x7fff and above are infinite. Linux sends 0xffff for an
  /// infinite effect and adapt-ffb-joy treats 0x7fff as infinite for Windows.
  static const uint16_t INFINITE_DURATION{0x7fff};

  /// Loop count of an effect, which repeats forever.
  static const uint8_t INFINITE_LOOPS{0xff};

  struct Effect {
    bool used{};
    bool playing{};
    bool directionEnable{};
    uint8_t direction{};
    uint8_t gain{255};
    uint8_t loops{};
    uint16_t duration{0xffff};
    int16_t magnitude{};
    uint32_t start{};
    uint8_t type{CONSTANT_FORCE};
    int8_t offset{};
    int8_t positiveCoefficient{};
    int8_t negativeCoefficient{};
    uint8_t saturation{};
    uint8_t deadBand{};
    int16_t periodicOffset{};
    uint8_t phase{};
    uint16_t period{};
    int16_t rampEnd{}; ///< ramps start at magnitude
    uint8_t attackLevel{};
    uint8_t fadeLevel{};
    uint16_t attackTime{};
    uint16_t fadeTime{};
  };

  Effect m_effects[MAX_EFFECTS]{};
  uint8_t m_loadedBlock{};
  uint8_t m_gain{255};
  bool m_actuators{true};
  bool m_paused{};

  static int16_t clamp(int32_t value) {
    return value > MAX_FORCE ? MAX_FORCE : value < -MAX_FORCE ? -MAX_FORCE : value;
  }

  /// sin(angle * 2π / 256) scaled to -MAX_FORCE..MAX_FORCE.
  static int16_t sine(uint8_t angle) {
    // round(MAX_FORCE * sin(i * π / 128)) for a quarter wave, i = 0..64
    static const int16_t QUARTER_WAVE[] PROGMEM = {
        0,    245,  491,  736,  980,  1224, 1467, 1710, 1951, 2191, 2430, 2667, 2903, 3137, 3369, 3599, 3827,
        4052, 4276, 4496, 4714, 4929, 5141, 5350, 5556, 5758, 5957, 6152, 6344, 6532, 6716, 6895, 7071, 7242,
        7410, 7572, 7730, 7883, 8032, 8176, 8315, 8449, 8577, 8701, 8819, 8932, 9040, 9142, 9239, 9330, 9415,
        9495, 9569, 9638, 9700, 9757, 9808, 9853, 9892, 9925, 9952, 9973, 9988, 9997, 10000};
    const uint8_t i = angle & 0x3f;
    const int16_t value = pgm_read_word(&QUARTER_WAVE[angle & 0x40 ? 64 - i : i]);
    return angle & 0x80 ? -value : value;
  }

  /// Force of a constant force, ramp or periodic effect before its direction
  /// is applied.
  /// @param[in] time since the effect was started in milliseconds
  static int32_t level(const Effect &effect, uint32_t time) {
    switch (effect.type) {
      case CONSTANT_FORCE:
        return envelope(effect, time);
      case RAMP:
        if (!hasDuration(effect)) {
          return effect.magnitude;
        }
        return effect.magnitude +
               (int32_t(effect.rampEnd) - effect.magnitude) * int32_t(time % effect.duration) / effect.duration;
      default: {
        const uint8_t angle = (effect.period ? time % effect.period * 256 / effect.period : 0) + effect.phase;
        return effect.periodicOffset + envelope(effect, time) * wave(effect.type, angle) / MAX_FORCE;
      }
    }
  }

  /// Ramps and envelopes need a duration, infinite effects have none.
  static bool hasDuration(const Effect &effect) {
    return effect.duration != 0 && effect.duration < INFINITE_DURATION;
  }

  /// Magnitude of a constant force or periodic effect shaped by its envelope.
  static int32_t envelope(const Effect &effect, uint32_t time) {
    const bool finite = hasDuration(effect);
    if (finite) {
      time %= effect.duration;
    }
    int32_t level = effect.magnitude < 0 ? -effect.magnitude : effect.magnitude;
    if (time < effect.attackTime) {
      const int32_t attack = int32_t(effect.attackLevel) * MAX_FORCE / 255;
      level = attack + (level - attack) * int32_t(time) / effect.attackTime;
    } else if (finite && time + effect.fadeTime > effect.duration) {
      const int32_t fade = int32_t(effect.fadeLevel) * MAX_FORCE / 255;
      level += (fade - level) * int32_t(time + effect.fadeTime - effect.duration) / effect.fadeTime;
    }
    return effect.magnitude < 0 ? -level : level;
  }

  /// Periodic waveform at the given angle (256 per period), scaled to
  /// -MAX_FORCE..MAX_FORCE.
  static int16_t wave(uint8_t type, uint8_t angle) {
    switch (type) {
      case SQUARE:
        return angle < 128 ? MAX_FORCE : -MAX_FORCE;
      case SINE:
        return sine(angle);
      case TRIANGLE:
        return (angle < 64 ? angle : angle < 192 ? 128 - angle : angle - 256) * int32_t(MAX_FORCE) / 64;
      case SAWTOOTH_UP:
        return (angle - 128) * int32_t(MAX_FORCE) / 128;
      default: // SAWTOOTH_DOWN
        return (128 - angle) * int32_t(MAX_FORCE) / 128;
    }
  }

  /// Force of a spring, all values are scaled to -MAX_FORCE..MAX_FORCE.
  static int32_t condition(const Effect &effect, int32_t metric) {
    const int32_t offset = int32_t(effect.offset) * MAX_FORCE / 127;
    const int32_t deadBand = int32_t(effect.deadBand) * MAX_FORCE / 255;
    int32_t force = 0;
    if (metric > offset + deadBand) {
      force = (offset + deadBand - metric) * effect.positiveCoefficient / 127;
    } else if (metric < offset - deadBand) {
      force = (offset - deadBand - metric) * effect.negativeCoefficient / 127;
    }
    const int32_t saturation = int32_t(effect.saturation) * MAX_FORCE / 255;
    return force > saturation ? saturation : force < -saturation ? -saturation : force;
  }

  static bool isFinished(const Effect &effect, uint32_t now) {
    return effect.duration < INFINITE_DURATION && effect.loops != INFINITE_LOOPS &&
           now - effect.start >= uint32_t(effect.duration) * effect.loops;
  }

  void createEffect() {
    m_loadedBlock = 0;
    for (auto i = 0u; i < MAX_EFFECTS; i++) {
      if (!m_effects[i].used) {
        m_effects[i] = Effect{};
        m_effects[i].used = true;
        m_loadedBlock = i + 1;
        return;
      }
    }
  }

  void stopAll() {
    for (auto &effect : m_effects) {
      effect.playing = false;
    }
  }

  void control(uint8_t command) {
    switch (command) {
      case DC_ENABLE_ACTUATORS:
        m_actuators = true;
        break;
      case DC_DISABLE_ACTUATORS:
        m_actuators = false;
        break;
      case DC_STOP_ALL:
        stopAll();
        break;
      case DC_RESET:
        for (auto &effect : m_effects) {
          effect = Effect{};
        }
        m_actuators = true;
        m_paused = false;
        break;
      case DC_PAUSE:
        m_paused = true;
        break;
      case DC_CONTINUE:
        m_paused = false;
        break;
    }
  }
};

/// HID PID report descriptor for ForceFeedback.
///
/// It has to be appended to a joystick application collection, which is still
/// open. The descriptor closes that collection with its last byte. Layout and
/// physical ranges follow the Windows tested descriptor of adapt-ffb-joy, see
/// https://github.com/tloimu/adapt-ffb-joy/blob/master/Descriptors.c
// clang-format off
static const uint8_t PID_DESCRIPTOR[] PROGMEM = {
    0x05, 0x0F,                               // Usage Page (Physical Interface)
    0x09, 0x21,                               // Usage (Set Effect Report)
    0xA1, 0x02,                               // Collection (Logical)
    0x85, ForceFeedback::SET_EFFECT,          //   Report ID
    0x09, 0x22,                               //   Usage (Effect Block Index)
    0x15, 0x01,                               //   Logical Minimum (1)
    0x25, ForceFeedback::MAX_EFFECTS,         //   Logical Maximum
    0x35, 0x01,                               //   Physical Minimum (1)
    0x45, ForceFeedback::MAX_EFFECTS,         //   Physical Maximum
    0x75, 0x08,                               //   Report Size (8)
    0x95, 0x01,                               //   Report Count (1)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x09, 0x25,                               //   Usage (Effect Type)
    0xA1, 0x02,                               //   Collection (Logical)
    0x09, 0x26,                               //     Usage (ET Constant Force)
    0x09, 0x40,                               //     Usage (ET Spring)
    0x09, 0x41,                               //     Usage (ET Damper)
    0x09, 0x42,                               //     Usage (ET Inertia)
    0x09, 0x43,                               //     Usage (ET Friction)
    0x09, 0x27,                               //     Usage (ET Ramp)
    0x09, 0x30,                               //     Usage (ET Square)
    0x09, 0x31,                               //     Usage (ET Sine)
    0x09, 0x32,                               //     Usage (ET Triangle)
    0x09, 0x33,                               //     Usage (ET Sawtooth Up)
    0x09, 0x34,                               //     Usage (ET Sawtooth Down)
    0x15, 0x01,                               //     Logical Minimum (1)
    0x25, 0x0B,                               //     Logical Maximum (11)
    0x35, 0x01,                               //     Physical Minimum (1)
    0x45, 0x0B,                               //     Physical Maximum (11)
    0x91, 0x00,                               //     Output (Data,Ary,Abs)
    0xC0,                                     //   End Collection
    0x09, 0x50,                               //   Usage (Duration)
    0x09, 0x54,                               //   Usage (Trigger Repeat Interval)
    0x15, 0x00,                               //   Logical Minimum (0)
    0x26, 0xFF, 0x7F,                         //   Logical Maximum (32767)
    0x35, 0x00,                               //   Physical Minimum (0)
    0x46, 0xFF, 0x7F,                         //   Physical Maximum (32767)
    0x66, 0x03, 0x10,                         //   Unit (Eng Lin: Time)
    0x55, 0xFD,                               //   Unit Exponent (-3)
    0x75, 0x10,                               //   Report Size (16)
    0x95, 0x02,                               //   Report Count (2)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x55, 0x00,                               //   Unit Exponent (0)
    0x66, 0x00, 0x00,                         //   Unit (None)
    0x09, 0x52,                               //   Usage (Gain)
    0x26, 0xFF, 0x00,                         //   Logical Maximum (255)
    0x46, 0x10, 0x27,                         //   Physical Maximum (10000)
    0x75, 0x08,                               //   Report Size (8)
    0x95, 0x01,                               //   Report Count (1)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x09, 0x53,                               //   Usage (Trigger Button)
    0x15, 0x01,                               //   Logical Minimum (1)
    0x25, 0x08,                               //   Logical Maximum (8)
    0x35, 0x01,                               //   Physical Minimum (1)
    0x45, 0x08,                               //   Physical Maximum (8)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x09, 0x55,                               //   Usage (Axes Enable)
    0xA1, 0x02,                               //   Collection (Logical)
    0x05, 0x01,                               //     Usage Page (Generic Desktop)
    0x09, 0x30,                               //     Usage (X)
    0x15, 0x00,                               //     Logical Minimum (0)
    0x25, 0x01,                               //     Logical Maximum (1)
    0x35, 0x00,                               //     Physical Minimum (0)
    0x45, 0x01,                               //     Physical Maximum (1)
    0x75, 0x01,                               //     Report Size (1)
    0x91, 0x02,                               //     Output (Data,Var,Abs)
    0xC0,                                     //   End Collection
    0x05, 0x0F,                               //   Usage Page (Physical Interface)
    0x09, 0x56,                               //   Usage (Direction Enable)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x95, 0x06,                               //   Report Count (6)
    0x91, 0x03,                               //   Output (Const,Var,Abs)
    0x09, 0x57,                               //   Usage (Direction)
    0xA1, 0x02,                               //   Collection (Logical)
    0x0B, 0x01, 0x00, 0x0A, 0x00,             //     Usage (Ordinal: Instance 1)
    0x26, 0xFF, 0x00,                         //     Logical Maximum (255)
    0x47, 0xA0, 0x8C, 0x00, 0x00,             //     Physical Maximum (36000)
    0x66, 0x14, 0x00,                         //     Unit (Eng Rot: Degrees)
    0x55, 0xFE,                               //     Unit Exponent (-2)
    0x75, 0x08,                               //     Report Size (8)
    0x95, 0x01,                               //     Report Count (1)
    0x91, 0x02,                               //     Output (Data,Var,Abs)
    0x55, 0x00,                               //     Unit Exponent (0)
    0x66, 0x00, 0x00,                         //     Unit (None)
    0xC0,                                     //   End Collection
    0xC0,                                     // End Collection
    0x09, 0x5A,                               // Usage (Set Envelope Report)
    0xA1, 0x02,                               // Collection (Logical)
    0x85, ForceFeedback::SET_ENVELOPE,        //   Report ID
    0x09, 0x22,                               //   Usage (Effect Block Index)
    0x15, 0x01,                               //   Logical Minimum (1)
    0x25, ForceFeedback::MAX_EFFECTS,         //   Logical Maximum
    0x35, 0x01,                               //   Physical Minimum (1)
    0x45, ForceFeedback::MAX_EFFECTS,         //   Physical Maximum
    0x75, 0x08,                               //   Report Size (8)
    0x95, 0x01,                               //   Report Count (1)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x09, 0x5B,                               //   Usage (Attack Level)
    0x09, 0x5D,                               //   Usage (Fade Level)
    0x15, 0x00,                               //   Logical Minimum (0)
    0x26, 0xFF, 0x00,                         //   Logical Maximum (255)
    0x35, 0x00,                               //   Physical Minimum (0)
    0x46, 0x10, 0x27,                         //   Physical Maximum (10000)
    0x95, 0x02,                               //   Report Count (2)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x09, 0x5C,                               //   Usage (Attack Time)
    0x09, 0x5E,                               //   Usage (Fade Time)
    0x26, 0xFF, 0x7F,                         //   Logical Maximum (32767)
    0x46, 0xFF, 0x7F,                         //   Physical Maximum (32767)
    0x66, 0x03, 0x10,                         //   Unit (Eng Lin: Time)
    0x55, 0xFD,                               //   Unit Exponent (-3)
    0x75, 0x10,                               //   Report Size (16)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x55, 0x00,                               //   Unit Exponent (0)
    0x66, 0x00, 0x00,                         //   Unit (None)
    0xC0,                                     // End Collection
    0x09, 0x73,                               // Usage (Set Constant Force Report)
    0xA1, 0x02,                               // Collection (Logical)
    0x85, ForceFeedback::SET_CONSTANT_FORCE,  //   Report ID
    0x09, 0x22,                               //   Usage (Effect Block Index)
    0x15, 0x01,                               //   Logical Minimum (1)
    0x25, ForceFeedback::MAX_EFFECTS,         //   Logical Maximum
    0x35, 0x01,                               //   Physical Minimum (1)
    0x45, ForceFeedback::MAX_EFFECTS,         //   Physical Maximum
    0x75, 0x08,                               //   Report Size (8)
    0x95, 0x01,                               //   Report Count (1)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x09, 0x70,                               //   Usage (Magnitude)
    0x16, 0xF0, 0xD8,                         //   Logical Minimum (-10000)
    0x26, 0x10, 0x27,                         //   Logical Maximum (10000)
    0x36, 0xF0, 0xD8,                         //   Physical Minimum (-10000)
    0x46, 0x10, 0x27,                         //   Physical Maximum (10000)
    0x75, 0x10,                               //   Report Size (16)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0xC0,                                     // End Collection
    0x09, 0x5F,                               // Usage (Set Condition Report)
    0xA1, 0x02,                               // Collection (Logical)
    0x85, ForceFeedback::SET_CONDITION,       //   Report ID
    0x09, 0x22,                               //   Usage (Effect Block Index)
    0x15, 0x01,                               //   Logical Minimum (1)
    0x25, ForceFeedback::MAX_EFFECTS,         //   Logical Maximum
    0x35, 0x01,                               //   Physical Minimum (1)
    0x45, ForceFeedback::MAX_EFFECTS,         //   Physical Maximum
    0x75, 0x08,                               //   Report Size (8)
    0x95, 0x01,                               //   Report Count (1)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x09, 0x60,                               //   Usage (CP Offset)
    0x09, 0x61,                               //   Usage (Positive Coefficient)
    0x09, 0x62,                               //   Usage (Negative Coefficient)
    0x15, 0x81,                               //   Logical Minimum (-127)
    0x25, 0x7F,                               //   Logical Maximum (127)
    0x36, 0xF0, 0xD8,                         //   Physical Minimum (-10000)
    0x46, 0x10, 0x27,                         //   Physical Maximum (10000)
    0x95, 0x03,                               //   Report Count (3)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    // No Negative Saturation: Linux scales it as a signed value, so full
    // saturation (0xffff) would arrive as 0. Positive Saturation is used for
    // both sides. No Parameter Block Offset: the wheel has only one axis.
    0x09, 0x63,                               //   Usage (Positive Saturation)
    0x09, 0x65,                               //   Usage (Dead Band)
    0x15, 0x00,                               //   Logical Minimum (0)
    0x26, 0xFF, 0x00,                         //   Logical Maximum (255)
    0x35, 0x00,                               //   Physical Minimum (0)
    0x46, 0x10, 0x27,                         //   Physical Maximum (10000)
    0x95, 0x02,                               //   Report Count (2)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0xC0,                                     // End Collection
    0x09, 0x6E,                               // Usage (Set Periodic Report)
    0xA1, 0x02,                               // Collection (Logical)
    0x85, ForceFeedback::SET_PERIODIC,        //   Report ID
    0x09, 0x22,                               //   Usage (Effect Block Index)
    0x15, 0x01,                               //   Logical Minimum (1)
    0x25, ForceFeedback::MAX_EFFECTS,         //   Logical Maximum
    0x35, 0x01,                               //   Physical Minimum (1)
    0x45, ForceFeedback::MAX_EFFECTS,         //   Physical Maximum
    0x75, 0x08,                               //   Report Size (8)
    0x95, 0x01,                               //   Report Count (1)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x09, 0x70,                               //   Usage (Magnitude)
    0x09, 0x6F,                               //   Usage (Offset)
    0x16, 0xF0, 0xD8,                         //   Logical Minimum (-10000)
    0x26, 0x10, 0x27,                         //   Logical Maximum (10000)
    0x36, 0xF0, 0xD8,                         //   Physical Minimum (-10000)
    0x46, 0x10, 0x27,                         //   Physical Maximum (10000)
    0x75, 0x10,                               //   Report Size (16)
    0x95, 0x02,                               //   Report Count (2)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x09, 0x71,                               //   Usage (Phase)
    0x15, 0x00,                               //   Logical Minimum (0)
    0x26, 0xFF, 0x00,                         //   Logical Maximum (255)
    0x35, 0x00,                               //   Physical Minimum (0)
    0x47, 0xA0, 0x8C, 0x00, 0x00,             //   Physical Maximum (36000)
    0x66, 0x14, 0x00,                         //   Unit (Eng Rot: Degrees)
    0x55, 0xFE,                               //   Unit Exponent (-2)
    0x75, 0x08,                               //   Report Size (8)
    0x95, 0x01,                               //   Report Count (1)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x09, 0x72,                               //   Usage (Period)
    0x26, 0xFF, 0x7F,                         //   Logical Maximum (32767)
    0x46, 0xFF, 0x7F,                         //   Physical Maximum (32767)
    0x66, 0x03, 0x10,                         //   Unit (Eng Lin: Time)
    0x55, 0xFD,                               //   Unit Exponent (-3)
    0x75, 0x10,                               //   Report Size (16)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x55, 0x00,                               //   Unit Exponent (0)
    0x66, 0x00, 0x00,                         //   Unit (None)
    0xC0,                                     // End Collection

    0x09, 0x74,                               // Usage (Set Ramp Force Report)
    0xA1, 0x02,                               // Collection (Logical)
    0x85, ForceFeedback::SET_RAMP_FORCE,      //   Report ID
    0x09, 0x22,                               //   Usage (Effect Block Index)
    0x15, 0x01,                               //   Logical Minimum (1)
    0x25, ForceFeedback::MAX_EFFECTS,         //   Logical Maximum
    0x35, 0x01,                               //   Physical Minimum (1)
    0x45, ForceFeedback::MAX_EFFECTS,         //   Physical Maximum
    0x75, 0x08,                               //   Report Size (8)
    0x95, 0x01,                               //   Report Count (1)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x09, 0x75,                               //   Usage (Ramp Start)
    0x09, 0x76,                               //   Usage (Ramp End)
    0x16, 0xF0, 0xD8,                         //   Logical Minimum (-10000)
    0x26, 0x10, 0x27,                         //   Logical Maximum (10000)
    0x36, 0xF0, 0xD8,                         //   Physical Minimum (-10000)
    0x46, 0x10, 0x27,                         //   Physical Maximum (10000)
    0x75, 0x10,                               //   Report Size (16)
    0x95, 0x02,                               //   Report Count (2)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0xC0,                                     // End Collection

    0x09, 0x77,                               // Usage (Effect Operation Report)
    0xA1, 0x02,                               // Collection (Logical)
    0x85, ForceFeedback::EFFECT_OPERATION,    //   Report ID
    0x09, 0x22,                               //   Usage (Effect Block Index)
    0x15, 0x01,                               //   Logical Minimum (1)
    0x25, ForceFeedback::MAX_EFFECTS,         //   Logical Maximum
    0x35, 0x01,                               //   Physical Minimum (1)
    0x45, ForceFeedback::MAX_EFFECTS,         //   Physical Maximum
    0x75, 0x08,                               //   Report Size (8)
    0x95, 0x01,                               //   Report Count (1)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0x09, 0x78,                               //   Usage (Effect Operation)
    0xA1, 0x02,                               //   Collection (Logical)
    0x09, 0x79,                               //     Usage (Op Effect Start)
    0x09, 0x7A,                               //     Usage (Op Effect Start Solo)
    0x09, 0x7B,                               //     Usage (Op Effect Stop)
    0x25, 0x03,                               //     Logical Maximum (3)
    0x45, 0x03,                               //     Physical Maximum (3)
    0x91, 0x00,                               //     Output (Data,Ary,Abs)
    0xC0,                                     //   End Collection
    0x09, 0x7C,                               //   Usage (Loop Count)
    0x15, 0x00,                               //   Logical Minimum (0)
    0x26, 0xFF, 0x00,                         //   Logical Maximum (255)
    0x35, 0x00,                               //   Physical Minimum (0)
    0x46, 0xFF, 0x00,                         //   Physical Maximum (255)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0xC0,                                     // End Collection
    0x09, 0x90,                               // Usage (PID Block Free Report)
    0xA1, 0x02,                               // Collection (Logical)
    0x85, ForceFeedback::BLOCK_FREE,          //   Report ID
    0x09, 0x22,                               //   Usage (Effect Block Index)
    0x15, 0x01,                               //   Logical Minimum (1)
    0x25, ForceFeedback::MAX_EFFECTS,         //   Logical Maximum
    0x35, 0x01,                               //   Physical Minimum (1)
    0x45, ForceFeedback::MAX_EFFECTS,         //   Physical Maximum
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0xC0,                                     // End Collection
    0x09, 0x96,                               // Usage (PID Device Control)
    0xA1, 0x02,                               // Collection (Logical)
    0x85, ForceFeedback::DEVICE_CONTROL,      //   Report ID
    0x09, 0x97,                               //   Usage (DC Enable Actuators)
    0x09, 0x98,                               //   Usage (DC Disable Actuators)
    0x09, 0x99,                               //   Usage (DC Stop All Effects)
    0x09, 0x9A,                               //   Usage (DC Device Reset)
    0x09, 0x9B,                               //   Usage (DC Device Pause)
    0x09, 0x9C,                               //   Usage (DC Device Continue)
    0x25, 0x06,                               //   Logical Maximum (6)
    0x45, 0x06,                               //   Physical Maximum (6)
    0x91, 0x00,                               //   Output (Data,Ary,Abs)
    0xC0,                                     // End Collection
    0x09, 0x7D,                               // Usage (Device Gain Report)
    0xA1, 0x02,                               // Collection (Logical)
    0x85, ForceFeedback::DEVICE_GAIN,         //   Report ID
    0x09, 0x7E,                               //   Usage (Device Gain)
    0x15, 0x00,                               //   Logical Minimum (0)
    0x26, 0xFF, 0x00,                         //   Logical Maximum (255)
    0x35, 0x00,                               //   Physical Minimum (0)
    0x46, 0x10, 0x27,                         //   Physical Maximum (10000)
    0x91, 0x02,                               //   Output (Data,Var,Abs)
    0xC0,                                     // End Collection
    0x09, 0xAB,                               // Usage (Create New Effect Report)
    0xA1, 0x02,                               // Collection (Logical)
    0x85, ForceFeedback::CREATE_NEW_EFFECT,   //   Report ID
    0x09, 0x25,                               //   Usage (Effect Type)
    0xA1, 0x02,                               //   Collection (Logical)
    0x09, 0x26,                               //     Usage (ET Constant Force)
    0x09, 0x40,                               //     Usage (ET Spring)
    0x09, 0x41,                               //     Usage (ET Damper)
    0x09, 0x42,                               //     Usage (ET Inertia)
    0x09, 0x43,                               //     Usage (ET Friction)
    0x09, 0x27,                               //     Usage (ET Ramp)
    0x09, 0x30,                               //     Usage (ET Square)
    0x09, 0x31,                               //     Usage (ET Sine)
    0x09, 0x32,                               //     Usage (ET Triangle)
    0x09, 0x33,                               //     Usage (ET Sawtooth Up)
    0x09, 0x34,                               //     Usage (ET Sawtooth Down)
    0x15, 0x01,                               //     Logical Minimum (1)
    0x25, 0x0B,                               //     Logical Maximum (11)
    0x35, 0x01,                               //     Physical Minimum (1)
    0x45, 0x0B,                               //     Physical Maximum (11)
    0xB1, 0x00,                               //     Feature (Data,Ary,Abs)
    0xC0,                                     //   End Collection
    0x05, 0x01,                               //   Usage Page (Generic Desktop)
    0x09, 0x3B,                               //   Usage (Byte Count)
    0x15, 0x00,                               //   Logical Minimum (0)
    0x26, 0xFF, 0x01,                         //   Logical Maximum (511)
    0x35, 0x00,                               //   Physical Minimum (0)
    0x46, 0xFF, 0x01,                         //   Physical Maximum (511)
    0x75, 0x0A,                               //   Report Size (10)
    0xB1, 0x02,                               //   Feature (Data,Var,Abs)
    0x75, 0x06,                               //   Report Size (6)
    0xB1, 0x03,                               //   Feature (Const,Var,Abs)
    0x05, 0x0F,                               //   Usage Page (Physical Interface)
    0xC0,                                     // End Collection
    0x09, 0x89,                               // Usage (PID Block Load Report)
    0xA1, 0x02,                               // Collection (Logical)
    0x85, ForceFeedback::BLOCK_LOAD,          //   Report ID
    0x09, 0x22,                               //   Usage (Effect Block Index)
    0x15, 0x01,                               //   Logical Minimum (1)
    0x25, ForceFeedback::MAX_EFFECTS,         //   Logical Maximum
    0x35, 0x01,                               //   Physical Minimum (1)
    0x45, ForceFeedback::MAX_EFFECTS,         //   Physical Maximum
    0x75, 0x08,                               //   Report Size (8)
    0xB1, 0x02,                               //   Feature (Data,Var,Abs)
    0x09, 0x8B,                               //   Usage (Block Load Status)
    0xA1, 0x02,                               //   Collection (Logical)
    0x09, 0x8C,                               //     Usage (Block Load Success)
    0x09, 0x8D,                               //     Usage (Block Load Full)
    0x09, 0x8E,                               //     Usage (Block Load Error)
    0x25, 0x03,                               //     Logical Maximum (3)
    0x45, 0x03,                               //     Physical Maximum (3)
    0xB1, 0x00,                               //     Feature (Data,Ary,Abs)
    0xC0,                                     //   End Collection
    0x09, 0xAC,                               //   Usage (RAM Pool Available)
    0x15, 0x00,                               //   Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0x00, 0x00,             //   Logical Maximum (65535)
    0x35, 0x00,                               //   Physical Minimum (0)
    0x47, 0xFF, 0xFF, 0x00, 0x00,             //   Physical Maximum (65535)
    0x75, 0x10,                               //   Report Size (16)
    0xB1, 0x02,                               //   Feature (Data,Var,Abs)
    0xC0,                                     // End Collection
    0x09, 0x7F,                               // Usage (PID Pool Report)
    0xA1, 0x02,                               // Collection (Logical)
    0x85, ForceFeedback::POOL,                //   Report ID
    0x09, 0x80,                               //   Usage (RAM Pool Size)
    0x15, 0x00,                               //   Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0x00, 0x00,             //   Logical Maximum (65535)
    0x35, 0x00,                               //   Physical Minimum (0)
    0x47, 0xFF, 0xFF, 0x00, 0x00,             //   Physical Maximum (65535)
    0x75, 0x10,                               //   Report Size (16)
    0x95, 0x01,                               //   Report Count (1)
    0xB1, 0x02,                               //   Feature (Data,Var,Abs)
    0x09, 0x83,                               //   Usage (Simultaneous Effects Max)
    0x26, 0xFF, 0x00,                         //   Logical Maximum (255)
    0x46, 0xFF, 0x00,                         //   Physical Maximum (255)
    0x75, 0x08,                               //   Report Size (8)
    0xB1, 0x02,                               //   Feature (Data,Var,Abs)
    0x09, 0xA9,                               //   Usage (Device Managed Pool)
    0x09, 0xAA,                               //   Usage (Shared Parameter Blocks)
    0x25, 0x01,                               //   Logical Maximum (1)
    0x45, 0x01,                               //   Physical Maximum (1)
    0x75, 0x01,                               //   Report Size (1)
    0x95, 0x02,                               //   Report Count (2)
    0xB1, 0x02,                               //   Feature (Data,Var,Abs)
    0x75, 0x06,                               //   Report Size (6)
    0x95, 0x01,                               //   Report Count (1)
    0xB1, 0x03,                               //   Feature (Const,Var,Abs)
    0xC0,                                     // End Collection
    0xC0,                                     // End Collection (Application) of the joystick
};
// clang-format on
