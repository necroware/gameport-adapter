#pragma once 

#include "Joystick.h"
#include "GamePort.h"
#include "DigitalPin.h"
#include "AnalogPin.h"
#include "HidDevice.h"

class GenericJoystickB2A2 : public Joystick {
public:
   using HidDeviceType = HidDevice<GenericJoystickB2A2>;

   void init() override {
       HidDeviceType::activate();
   }
   
   void update() override {

       DigitalInput button1{GamePort<2>::pin};
       DigitalInput button2{GamePort<7>::pin};
       AnalogInput axis1{GamePort<3>::pin};
       AnalogInput axis2{GamePort<6>::pin};

       const auto convert = [](int value) {
           static const int upper = 1023;
           static const int lower =  upper / 2;
           const auto constrained = constrain(value, lower, upper);
           return static_cast<uint8_t>(map(constrained, lower, upper, 255, 0));
       };

       const byte data[3] = {
           static_cast<byte>(button1.isLow() | button2.isLow() << 1),
           convert(axis1.get()),
           convert(axis2.get())
       };

       HidDeviceType::send(&data, sizeof(data));
   }
};

template <>
const byte GenericJoystickB2A2::HidDeviceType::description[] = {
  0x05, 0x01, // USAGE_PAGE (Generic Desktop)
  0x09, 0x04, // USAGE (Joystick)
  0xa1, 0x01, // COLLECTION (Application)
  0x85, id,   //   REPORT_ID (DEVICE_ID)  
  0x05, 0x09, //   USAGE_PAGE (Button)
  0x19, 0x01, //   USAGE_MINIMUM (Button 1)
  0x29, 0x02, //   USAGE_MAXIMUM (Button 2)
  0x15, 0x00, //   LOGICAL_MINIMUM (0)
  0x25, 0x01, //   LOGICAL_MAXIMUM (1)
  0x75, 0x01, //   REPORT_SIZE (1)  
  0x95, 0x02, //   REPORT_COUNT (2)
  0x81, 0x02, //   INPUT (Data,Var,Abs)
  0x75, 0x06, //   REPORT_SIZE (6)  
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

