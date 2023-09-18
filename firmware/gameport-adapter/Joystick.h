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

/// Base class for all joysticks.
class Joystick {
public:
  static const auto MAX_AXES{16u};

  /// Device description.
  ///
  /// This structure is used to generate the HID description
  /// and USB data packages
  struct Description {

    /// Human readable name.
    const char* name;

    /// Number of supported axes.
    uint8_t numAxes;

    /// Number of supported buttons.
    uint8_t numButtons;

    /// Has HAT.
    bool hasHat;
  };

  /// Joystick state.
  struct State {

    /// Axes.
    ///
    /// To keep it simple all axes are unified to 10 bits
    /// resolution. Every joystick has to map it's values
    /// to the values 0..1023 independent from the internal
    /// resolution of the device.
    uint16_t axes[MAX_AXES]{};

    /// Hats.
    ///
    /// HAT is represented through values 0..8 1 is up, 2 is
    /// 45° clockwise, 3 is right etc. Center is represented
    /// with zero.
    uint8_t hat{};

    /// Buttons.
    ///
    /// Every bit is a button. 1 is pressed, 0 is released
    uint16_t buttons{};
  };

  /// Initialize joystick.
  ///
  /// @returns True on successful initialization
  virtual bool init() = 0;

  /// Update state.
  ///
  /// This function is called every time the joystick
  /// state has to be updated.
  ///
  /// @returns True on successful initialization
  virtual bool update() = 0;

  /// Gets the State of the Joystick.
  virtual const State &getState() const = 0;

  /// Gets the Description of the Joystick.
  virtual const Description &getDescription() const = 0;

  Joystick() = default;
  virtual ~Joystick() = default;
  Joystick(const Joystick &) = delete;
  Joystick(Joystick &&) = delete;
  Joystick &operator=(const Joystick &) = delete;
  Joystick &operator=(Joystick &&) = delete;
};
