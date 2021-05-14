#pragma once 

#include "DigitalPin.h"
#include "Driver.h"
#include "GamePort.h"
#include "HidDevice.h"
#include "Sidewinder.h"

class SidewinderGPPro : public Driver {
public:

using HidDeviceType = HidDevice<SidewinderGPPro>;
   void init() override {
       HidDeviceType::activate();
       m_sw.reset();
   }

   void update() override {
       const auto state = m_sw.readState();
       const uint16_t data = state.buttons << 4 | state.axis[0] << 2 | state.axis[1];
       HidDeviceType::send(&data, sizeof(data));
       m_sw.cooldown();
   }

private:
   Sidewinder m_sw;
};

template <>
const byte SidewinderGPPro::HidDeviceType::description[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x04,       // Usage (Joystick)
    0xa1, 0x01,       // Collection (Application)
    0x85, id,         //   Report ID (id)
    0x05, 0x01,       //   Usage Page (Generic Desktop)
    0x09, 0x30,       //   Usage (X)
    0x09, 0x31,       //   Usage (Y)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x02,       //   Logical Maximum (2)
    0x75, 0x02,       //   Report Size (2)
    0x95, 0x02,       //   Report Count (2)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x05, 0x09,       //   Usage Page (Button)
    0x19, 0x01,       //   Usage Minimum (1)
    0x29, 0x0A,       //   Usage Maximum (10)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x01,       //   Logical Maximum (1)
    0x75, 0x01,       //   Report Size (1)
    0x95, 0x0A,       //   Report Count (10)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0xc0,             // End Collection
};

