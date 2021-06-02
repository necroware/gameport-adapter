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
#include "HidDevice.h"
#include "Sidewinder.h"

class HidSidewinder : public Driver {
public:
  template <Sidewinder::Model M>
  struct HidType;
  using HidGamePad = HidDevice<HidType<Sidewinder::Model::SW_GAMEPAD>>;
  using Hid3DPro = HidDevice<HidType<Sidewinder::Model::SW_3D_PRO>>;
  using HidPrecisionPro =
      HidDevice<HidType<Sidewinder::Model::SW_PRECISION_PRO>>;

  void init() override {
    m_sw.reset();
    switch (m_sw.getModel()) {
      case Sidewinder::Model::SW_GAMEPAD:
        log("Detected Sidewinder GamePad");
        HidGamePad::activate();
        break;
      case Sidewinder::Model::SW_3D_PRO:
        log("Detected Sidewinder 3D Pro");
        Hid3DPro::activate();
        break;
      case Sidewinder::Model::SW_PRECISION_PRO:
        log("Detected Sidewinder Precision Pro");
        HidPrecisionPro::activate();
        break;
      case Sidewinder::Model::SW_UNKNOWN:
        log("Unknown input device");
        break;
    }
  }

  void update() override {
    const auto state = m_sw.readState();
    switch (m_sw.getModel()) {
      case Sidewinder::Model::SW_GAMEPAD:
        sendGamePad(state);
        break;
      case Sidewinder::Model::SW_3D_PRO:
        send3DPro(state);
        break;
      case Sidewinder::Model::SW_PRECISION_PRO:
        sendPrecisionPro(state);
        break;
      case Sidewinder::Model::SW_UNKNOWN:
        log("Unknown input device");
        break;
    }
    m_sw.cooldown();
  }

private:
  static void sendGamePad(const Sidewinder::State &state) {
    const uint16_t data =
        state.buttons << 4 | state.axis[0] << 2 | state.axis[1];
    HidGamePad::send(&data, sizeof(data));
  }

  static void send3DPro(const Sidewinder::State &state) {
    const struct {
      uint32_t x : 10;
      uint32_t y : 10;
      uint32_t z : 9;
      uint16_t throttle;
      uint8_t hat, buttons;
    } data = {
        uint32_t(state.axis[0]),
        uint32_t(state.axis[1]),
        uint32_t(state.axis[2]),
        uint16_t(state.axis[3]),
        uint8_t(state.hat),
        uint8_t(state.buttons),
    };
    Hid3DPro::send(&data, sizeof(data));
  }

  static void sendPrecisionPro(const Sidewinder::State &state) {
    const struct {
      uint32_t x : 10;
      uint32_t y : 10;
      uint32_t z : 6; 
      uint8_t throttle, hat;
      uint16_t buttons;
    } data = {
        uint32_t(state.axis[0]),
        uint32_t(state.axis[1]),
        uint32_t(state.axis[2]),
        uint8_t(state.axis[3]), 
        uint8_t(state.hat), 
        uint16_t(state.buttons),
    };
    HidPrecisionPro::send(&data, sizeof(data));
  }

  Sidewinder m_sw;
};

template <>
const byte HidSidewinder::HidGamePad::description[] = {
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

template <>
const byte HidSidewinder::Hid3DPro::description[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x04,       // Usage (Joystick)
    0xa1, 0x01,       // Collection (Application)
    0x85, id,         //   Report ID (id)
    0x05, 0x01,       //   Usage Page (Generic Desktop)
    0x09, 0x30,       //   Usage (X)
    0x09, 0x31,       //   Usage (Y)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xff, 0x03, //   Logical Maximum (1023)
    0x75, 0x0A,       //   Report Size (10)
    0x95, 0x02,       //   Report Count (2)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x09, 0x32,       //   Usage (Z)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xff, 0x01, //   Logical Maximum (511)
    0x75, 0x09,       //   Report Size (9)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x75, 0x03,       //   Report Size (3)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x03,       //   Input (Const,Var,Abs)
    0x05, 0x02,       //   Usage Page (Simulation Controls)
    0x09, 0xBB,       //   Usage (Throttle)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xff, 0x03, //   Logical Maximum (1023)
    0x75, 0x10,       //   Report Size (16)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x05, 0x01,       //   Usage Page (Generic Desktop)
    0x09, 0x39,       //   Usage (Hat switch)
    0x15, 0x01,       //   Logical Minimum (1)
    0x25, 0x08,       //   Logical Maximum (8)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x42,       //   Input (Data,Var,Abs)
    0x05, 0x09,       //   Usage Page (Button)
    0x19, 0x01,       //   Usage Minimum (1)
    0x29, 0x08,       //   Usage Maximum (8)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x01,       //   Logical Maximum (1)
    0x75, 0x01,       //   Report Size (1)
    0x95, 0x08,       //   Report Count (8)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0xc0,             // End Collection
};

template <>
const byte HidSidewinder::HidPrecisionPro::description[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x04,       // Usage (Joystick)
    0xa1, 0x01,       // Collection (Application)
    0x85, id,         //   Report ID (id)
    0x05, 0x01,       //   Usage Page (Generic Desktop)
    0x09, 0x30,       //   Usage (X)
    0x09, 0x31,       //   Usage (Y)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xff, 0x03, //   Logical Maximum (1023)
    0x75, 0x0A,       //   Report Size (10)
    0x95, 0x02,       //   Report Count (2)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x09, 0x32,       //   Usage (Z)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0x3f, 0x00, //   Logical Maximum (63)
    0x75, 0x06,       //   Report Size (6)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x75, 0x06,       //   Report Size (6)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x03,       //   Input (Const,Var,Abs)
    0x05, 0x02,       //   Usage Page (Simulation Controls)
    0x09, 0xBB,       //   Usage (Throttle)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0x7f, 0x00, //   Logical Maximum (127)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x05, 0x01,       //   Usage Page (Generic Desktop)
    0x09, 0x39,       //   Usage (Hat switch)
    0x15, 0x01,       //   Logical Minimum (1)
    0x25, 0x08,       //   Logical Maximum (8)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x42,       //   Input (Data,Var,Abs)
    0x05, 0x09,       //   Usage Page (Button)
    0x19, 0x01,       //   Usage Minimum (1)
    0x29, 0x09,       //   Usage Maximum (9)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x01,       //   Logical Maximum (1)
    0x75, 0x01,       //   Report Size (1)
    0x95, 0x09,       //   Report Count (9)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x75, 0x07,       //   Report Size (7)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x03,       //   Input (Const,Var,Abs)
    0xc0,             // End Collection
};
