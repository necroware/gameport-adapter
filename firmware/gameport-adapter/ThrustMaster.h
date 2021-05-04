#pragma once

#include "AnalogJoystick.h"
#include "Driver.h"
#include "HidDevice.h"

class ThrustMaster : public Driver {
public:
   using HidDeviceType = HidDevice<ThrustMaster>;

   void init() override {
       HidDeviceType::activate();
   }

   void update() override {

       auto hat = [](int value) -> byte {
           return value > 128 ? 15 : value / 32 * 2;
       };

       AnalogJoystick joystick;

       const byte buttons = joystick.getButtons() << 4 | hat(joystick.getAxis(3));

       const byte data[4] = {
           joystick.getAxis(0),
           joystick.getAxis(1),
           joystick.getAxis(2),
           buttons
       };

       HidDeviceType::send(&data, sizeof(data));
   }
};

template <>
const byte ThrustMaster::HidDeviceType::description[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x04,       // Usage (Joystick)
    0xa1, 0x01,       // Collection (Application)
    0x85, id,         //   Report ID (id)
    0x05, 0x01,       //   Usage Page (Generic Desktop)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0xff,       //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x03,       //   Report Count (3)
    0x09, 0x30,       //   Usage (X)
    0x09, 0x31,       //   Usage (Y)
    0x09, 0x32,       //   Usage (Z)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x09, 0x39,       //   Usage (Hat switch)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x07,       //   Logical Maximum (7)
    0x65, 0x14,       //   Unit (Degrees, EngRotation)
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

