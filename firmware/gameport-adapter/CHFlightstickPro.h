#pragma once

#include "AnalogJoystick.h"
#include "Driver.h"
#include "HidDevice.h"

class CHFlightstickPro : public Driver {
public:

   using HidType = HidDevice<CHFlightstickPro>;

   void init() override {
       HidType::activate();
   }

   void update() override {

       auto buttons = [](byte code) -> byte {
           static const byte table[16] = {
               0, 1, 2, 0, 4, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0
           };
           return (code < sizeof(table)) ? table[code] : byte{0};
       };

       auto hat = [](byte code) -> byte {
           static const byte table[16] = {
               0, 0, 0, 7, 0, 6, 0, 5, 0, 4, 0, 3, 0, 2, 0, 1
           };
           return (code < sizeof(table)) ? table[code] : byte{0};
       };

       const byte code = m_joystick.getButtons();
       const byte data[5] = {
           m_joystick.getAxis(0), 
           m_joystick.getAxis(1), 
           m_joystick.getAxis(2), 
           m_joystick.getAxis(3),
           static_cast<byte>(buttons(code) << 4 | hat(code))
       };

       HidType::send(&data, sizeof(data));
   }

private:
    AnalogJoystick m_joystick;
};

template <>
const byte HidDevice<CHFlightstickPro>::description[] = {
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

