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
#include "Driver.h"
#include "GamePort.h"
#include "GrIP.h"
#include "HidDevice.h"
#include "Log.h"

class HidGrIP : public Driver {
public:
  template <GrIP::Model M>
  struct HidType;
  using HidGamePadPro = HidDevice<HidType<GrIP::Model::GRIP_GAMEPAD_PRO>>;

  void init() override {
    m_grip.reset();
    switch (m_grip.getModel()) {
      case GrIP::Model::GRIP_GAMEPAD_PRO:
        log("Detected Gravis GamePad Pro");
        HidGamePadPro::activate();
        break;
      case GrIP::Model::GRIP_UNKNOWN:
        log("Unknown or not GrIP input device");
        break;
    }
  }

  void update() override {
    const auto state = m_grip.readState();
    switch (m_grip.getModel()) {
      case GrIP::Model::GRIP_GAMEPAD_PRO:
        sendGamePadPro(state);
        break;
      case GrIP::Model::GRIP_UNKNOWN:
        log("Unknown or not GrIP input device");
        break;
    }
  }

private:
  static void sendGamePadPro(const GrIP::State &state) {
    const uint16_t data = state.buttons << 4 | state.axis[0] << 2 | state.axis[1];
    HidGamePadPro::send(&data, sizeof(data));
  }

  GrIP m_grip;
};

template <>
const byte HidGrIP::HidGamePadPro::description[] = {
    0x05, 0x01, // Usage Page (Generic Desktop)
    0x09, 0x04, // Usage (Joystick)
    0xa1, 0x01, // Collection (Application)
    0x85, id,   //   Report ID (id)
    0x05, 0x01, //   Usage Page (Generic Desktop)
    0x09, 0x30, //   Usage (X)
    0x09, 0x31, //   Usage (Y)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x02, //   Logical Maximum (2)
    0x75, 0x02, //   Report Size (2)
    0x95, 0x02, //   Report Count (2)
    0x81, 0x02, //   Input (Data,Var,Abs)
    0x05, 0x09, //   Usage Page (Button)
    0x19, 0x01, //   Usage Minimum (1)
    0x29, 0x0A, //   Usage Maximum (10)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x0A, //   Report Count (10)
    0x81, 0x02, //   Input (Data,Var,Abs)
    0x75, 0x02, //   Report Size (2)
    0x95, 0x01, //   Report Count (1)
    0x81, 0x03, //   Input (Const,Var,Abs)
    0xc0,       // End Collection
};
