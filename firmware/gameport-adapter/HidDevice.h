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

#include "HID.h"
#if !defined(_USING_HID)
#error "Incompatible USB HID device"
#endif

template <typename T>
class HidDevice {
public:
   static void activate() {
      static HidDevice instance;
   }

   static void send(const void* data, size_t size) {
      HID().SendReport(id, data, size);
   }

private:
   static const byte id{0x03};
   static const byte description[] PROGMEM;

   explicit HidDevice() {
      static HIDSubDescriptor node(description, sizeof(description));
      HID().AppendDescriptor(&node);
   }
};

