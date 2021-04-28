#pragma once 

#include "Joystick.h"
#include "GamePort.h"
#include "DigitalPin.h"
#include "AnalogPin.h"
#include "HidDevice.h"

class CHFlightstickPro : public Joystick {
public:
   using HidDeviceType = HidDevice<CHFlightstickPro>;

   void init() override {
       HidDeviceType::activate();
   }
   
   void update() override {

       DigitalInput button1{GamePort<2>::pin};
       DigitalInput button2{GamePort<7>::pin};
       DigitalInput button3{GamePort<10>::pin};
       DigitalInput button4{GamePort<14>::pin};
       AnalogInput axis1{GamePort<3>::pin};
       AnalogInput axis2{GamePort<6>::pin};
       AnalogInput axis3{GamePort<11>::pin};

       const auto axis = [](int value) {
           static const int upper = 1023;
           static const int lower =  upper / 2;
           const auto constrained = constrain(value, lower, upper);
           return static_cast<uint8_t>(map(constrained, lower, upper, 255, 0));
       };

       auto buttons = [](byte code) {
           static const byte table[16] = {
               0, 1, 2, 0, 4, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0
           };
           return (code < sizeof(table)) ? table[code] : byte{0};
       };

       auto hat = [](byte code) {
           static const byte table[16] = {
               15, 15, 15, 6, 15, 5, 15, 4, 15, 3, 15, 2, 15, 1, 15, 0           
           };
           return (code < sizeof(table)) ? table[code] : byte{0};
       };

       const byte code = 
           button1.isLow() 
           | button2.isLow() << 1 
           | button3.isLow() << 2 
           | button4.isLow() << 3;

       const byte data[5] = {
           buttons(code),
           axis(axis1.get()),
           axis(axis2.get()),
           axis(axis3.get()),
           hat(code)
       };

       HidDeviceType::send(&data, sizeof(data));
   }
};

template <>
const byte CHFlightstickPro::HidDeviceType::description[] = {
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
  0x15, 0x00, //   LOGICAL_MINIMUM (0)
  0x25, 0xff, //   LOGICAL_MAXIMUM (255)
  0x75, 0x08, //   REPORT_SIZE (8)
  0x95, 0x03, //   REPORT_COUNT (3)
  0xa1, 0x00, //   COLLECTION (Physical)
  0x09, 0x30, //     USAGE (X)
  0x09, 0x31, //     USAGE (Y)
  0x09, 0x32, //     USAGE (Z)
  0x81, 0x02, //     INPUT (Data,Var,Abs)
  0xc0,       //   END_COLLECTION
  0x09, 0x39, //   USAGE (Hat Switch)
  0x15, 0x00, //   LOGICAL_MINIMUM (0)
  0x25, 0x07, //   LOGICAL_MAXIMUM (7)
  0x65, 0x14,  //  UNIT (Ent Rot: Angular Pos)
  0x75, 0x04, //   REPORT_SIZE (4)
  0x95, 0x01, //   REPORT_COUNT (1) 
  0x81, 0x02, //   INPUT (Data,Var,Abs)
  0x75, 0x04, //   REPORT_SIZE (4)  
  0x95, 0x01, //   REPORT_COUNT (1)
  0x81, 0x03, //   INPUT (Cnst,Var,Abs) 
  0xc0,       // END_COLLECTION
};

