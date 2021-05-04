#pragma once 

#include "AnalogJoystick.h"
#include "Driver.h"
#include "HidDevice.h"

class GenericJoystickB2A2 : public Driver {
public:
   using HidDeviceType = HidDevice<GenericJoystickB2A2>;

   void init() override {
       HidDeviceType::activate();
   }
   
   void update() override {
       const byte data[3] = {
           m_joystick.getAxis(0), 
           m_joystick.getAxis(1),
           m_joystick.getButtons(),
       };
       HidDeviceType::send(&data, sizeof(data));
   }

private:
   AnalogJoystick m_joystick;
};

template <>
const byte GenericJoystickB2A2::HidDeviceType::description[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x04,       // Usage (Joystick)
    0xa1, 0x01,       // Collection (Application)
    0x85, id,         //   Report ID (id)
    0x05, 0x01,       //   Usage Page (Generic Desktop)
    0x09, 0x30,       //   Usage (X)
    0x09, 0x31,       //   Usage (Y)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0xff,       //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x02,       //   Report Count (2)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x05, 0x09,       //   Usage Page (Button)
    0x19, 0x01,       //   Usage Minimum (1)
    0x29, 0x02,       //   Usage Maximum (2)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x01,       //   Logical Maximum (1)
    0x75, 0x01,       //   Report Size (1)
    0x95, 0x02,       //   Report Count (2)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0xc0,             // End Collection
};

