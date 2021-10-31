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

class Joystick {
public:
  struct Description {
    char name[32];
    uint8_t numAxes;
    uint8_t numButtons;
    uint8_t numHats;
  };

  struct State {
    static const auto MAX_AXES{8u};
    static const auto MAX_HATS{1u};

    uint16_t axes[MAX_AXES]{};
    uint8_t hats[MAX_HATS]{};
    uint16_t buttons{};
  };

  virtual bool init() = 0;

  virtual bool update() = 0;

  virtual const State &getState() const = 0;

  virtual const Description &getDescription() const = 0;

  Joystick() = default;
  virtual ~Joystick() = default;
  Joystick(const Joystick &) = delete;
  Joystick(Joystick &&) = delete;
  Joystick &operator=(const Joystick &) = delete;
  Joystick &operator=(Joystick &&) = delete;
};
