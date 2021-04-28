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

       const auto axis = [](AnalogInput& input) -> byte {
           static const int upper = 1023;
           static const int lower = 430;
           static const int centr = 600;
           const auto constrained = constrain(input.get(), lower, upper);
           if (constrained < centr) {
               return map(constrained, lower, 600, 255, 127);
           }
           return map(constrained, 600, upper, 126, 0);
       };

       auto buttons = [](byte code) -> byte {
           static const byte table[16] = {
               0, 1, 2, 0, 4, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0
           };
           return (code < sizeof(table)) ? table[code] : byte{0};
       };

       auto hat = [](byte code) -> byte {
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

       const byte data[4] = {
           axis(axis1),
           axis(axis2),
           axis(axis3),
           static_cast<byte>(buttons(code) << 4 | hat(code))
       };

       HidDeviceType::send(&data, sizeof(data));
   }
};

template <>
const byte CHFlightstickPro::HidDeviceType::description[] = {
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

