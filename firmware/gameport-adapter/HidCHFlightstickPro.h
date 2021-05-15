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

#include "AnalogJoystick.h"
#include "Driver.h"
#include "HidDevice.h"

class HidCHFlightstickPro : public Driver {
public:

   using Device = HidDevice<HidCHFlightstickPro>;

   void init() override {
       Device::activate();
   }

   void update() override {

       // Unfortunately I had no real CHFlighstickPro joystick to test, but
       // Sidewinder 3D Pro has an emulation for CHFlighstickPro. So, this
       // implementation was made using that emulation and could be wrong.
       // CHFlighstickPro seems to be a very interesting joystick. It doesn't
       // allow the user to press multiple buttons simultaneously and uses
       // combined buttons invocations as hat switch codes instead. So, every
       // time a multiple buttons seem to be pressed, means that the user is
       // actually using the hat switch.
       
       const auto decode = [](byte code) -> byte {
           // upper 4 bits are buttons (16..128)
           // lower 4 bits are hat codes (1..7)
           static const byte table[16] = {
               0, 16, 32, 7, 64, 6, 0, 5, 128, 4, 0, 3, 0, 2, 0, 1
           };
           return (code < sizeof(table)) ? table[code] : 0u;
       };

       const byte data[5] = {
           m_joystick.getAxis(0), 
           m_joystick.getAxis(1), 
           m_joystick.getAxis(2), 
           m_joystick.getAxis(3),
           decode(m_joystick.getButtons()),
       };

       Device::send(&data, sizeof(data));
   }

private:
    AnalogJoystick m_joystick;
};

template <>
const byte HidCHFlightstickPro::Device::description[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x04,       // Usage (Joystick)
    0xa1, 0x01,       // Collection (Application)
    0x85, id,         //   Report ID (id)
    0x05, 0x01,       //   Usage Page (Generic Desktop)
    0x09, 0x30,       //   Usage (X)
    0x09, 0x31,       //   Usage (Y)
    0x09, 0x33,       //   Usage (Z)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0xff,       //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x03,       //   Report Count (3)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x05, 0x02,       //   Usage Page (Simulation Controls)
    0x09, 0xBB,       //   Usage (Throttle)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0xff,       //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x05, 0x01,       //   Usage Page (Generic Desktop)
    0x09, 0x39,       //   Usage (Hat switch)
    0x15, 0x01,       //   Logical Minimum (1)
    0x25, 0x08,       //   Logical Maximum (8)
    0x75, 0x04,       //   Report Size (4)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x42,       //   Input (Data,Var,Abs)
    0x05, 0x09,       //   Usage Page (Button)
    0x19, 0x01,       //   Usage Minimum (1)
    0x29, 0x04,       //   Usage Maximum (4)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x01,       //   Logical Maximum (1)
    0x75, 0x01,       //   Report Size (1)
    0x95, 0x04,       //   Report Count (4)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0xc0,             // End Collection
};

