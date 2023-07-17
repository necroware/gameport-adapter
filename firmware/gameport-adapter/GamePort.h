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

// Simple GamePort pins to Arduino pins mapper.
// GamePort pins <1>, <8>, <9> are already connected to "VCC" via the PCB
template <int I>
struct GamePort {
  enum { pin = -1 };
};

// Analog: Button #1
// Digital (e.g. SW3D): Clock
template <>
struct GamePort<2> {
  enum { pin = 10 };
};

// Analog: Joystick1 X-Axis 
// Digital (e.g. SW3D): Trigger
template <>
struct GamePort<3> {
  enum { pin = A0 };
};

// Analog: Joystick1 Y-Axis 
template <>
struct GamePort<6> {
  enum { pin = A7 };
};

// Analog: Button #2
// Digital (e.g. SW3D): Data 0
template <>
struct GamePort<7> {
  enum { pin = 8 };
};

// Analog: Button #3
// Digital (e.g. SW3D): Data 1
template <>
struct GamePort<10> {
  enum { pin = 16 };
};

// Analog: Joystick2 X-Axis 
template <>
struct GamePort<11> {
  enum { pin = A1 };
};

// Analog: Midi OUT
template <>
struct GamePort<12> {
  enum { pin = 5 };
};

// Analog: Joystick2 Y-Axis 
template <>
struct GamePort<13> {
  enum { pin = A6 };
};

// Analog: Button #4
// Digital (e.g. SW3D): Data 2
template <>
struct GamePort<14> {
  enum { pin = 7 };
};

// Analog: Midi IN
template <>
struct GamePort<15> {
  enum { pin = 9 };
};
