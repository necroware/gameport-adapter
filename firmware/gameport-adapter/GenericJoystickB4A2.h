#pragma once 

#include "AnalogJoystick.h"
#include "Driver.h"
#include "HidDevice.h"

class GenericJoystickB4A2 : public Driver {
public:
   using HidDeviceType = HidDevice<GenericJoystickB4A2>;

   void init() override {
       HidDeviceType::activate();
   }
   
   void update() override {

       AnalogJoystick joystick;

       const byte buttons =
           joystick.isPressed(0) 
           | joystick.isPressed(1) << 1
           | joystick.isPressed(2) << 2
           | joystick.isPressed(3) << 3;

       const byte data[3] = {
           buttons, 
           joystick.getAxis(0), 
           joystick.getAxis(1)
       };

       HidDeviceType::send(&data, sizeof(data));
   }
};

template <>
const byte GenericJoystickB4A2::HidDeviceType::description[] = {
  0x05, 0x01, // USAGE_PAGE (Generic Desktop)
  0x09, 0x04, // USAGE (Joystick)
  0xa1, 0x01, // COLLECTION (Application)
  0x85, id,   //   REPORT_ID (DEVICE_ID)  
  0x05, 0x09, //   USAGE_PAGE (Button)
  0x19, 0x01, //   USAGE_MINIMUM (Button 1)
  0x29, 0x04, //   USAGE_MAXIMUM (Button 4)
  0x15, 0x00, //   LOGICAL_MINIMUM (0)
  0x25, 0x01, //   LOGICAL_MAXIMUM (1)
  0x75, 0x01, //   REPORT_SIZE (1)  
  0x95, 0x04, //   REPORT_COUNT (4)
  0x81, 0x02, //   INPUT (Data,Var,Abs)
  0x75, 0x04, //   REPORT_SIZE (4)  
  0x95, 0x01, //   REPORT_COUNT (1)
  0x81, 0x03, //   INPUT (Cnst,Var,Abs) 
  0x05, 0x01, //   USAGE_PAGE (Generic Desktop)
  0x09, 0x01, //   USAGE (Pointer)
  0xa1, 0x00, //   COLLECTION (Physical)
  0x09, 0x30, //     USAGE (X)
  0x09, 0x31, //     USAGE (Y)
  0x15, 0x00, //     LOGICAL_MINIMUM (0)
  0x25, 0xff, //     LOGICAL_MAXIMUM (255)
  0x75, 0x08, //     REPORT_SIZE (8)
  0x95, 0x02, //     REPORT_COUNT (2)
  0x81, 0x02, //     INPUT (Data,Var,Abs)
  0xc0,       //   END_COLLECTION
  0xc0,       // END_COLLECTION
};

