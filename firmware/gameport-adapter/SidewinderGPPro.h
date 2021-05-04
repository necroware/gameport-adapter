#pragma once 

#include "DigitalPin.h"
#include "Driver.h"
#include "GamePort.h"
#include "HidDevice.h"

class SidewinderGPPro : public Driver {
public:
   using HidDeviceType = HidDevice<SidewinderGPPro>;

   void init() override {
       HidDeviceType::activate();
   }
   
   void update() override {
       const auto bits = readBits(15);
       if (validate(bits)) {
           const byte decode[16] = { 
               5, 5, 5, 5, 5, 8, 0, 4, 5, 10, 2, 6, 5, 9, 1, 5
           };
           uint16_t data = (~bits >> 4) & 0b1111111111;
           data |= decode[bits & 0b1111] << 10;
           HidDeviceType::send(&data, sizeof(data));
       }
   }

private:

   bool validate(uint16_t bits) const {
       uint8_t checksum = 0;
       while (bits) {
           checksum ^= bits & 1;
           bits >>= 1;
       }
       return checksum == 0;
   }

   uint16_t readBits(size_t count) const {
       DigitalOutput<GamePort<3>::pin> trigger;
       DigitalInput<GamePort<2>::pin, true> clock;
       DigitalInput<GamePort<7>::pin, true> data;
       trigger.setLow();
       delayMicroseconds(250);
       while(clock.isLow());
       int timeout = 1000;
       uint16_t bits = 0;
       auto last = clock.get();
       auto next = last;
       noInterrupts();
       trigger.setHigh();
       for (auto i = 0u; (i < count) && timeout; timeout--) {
           next = clock.get();
           if (last < next) {
               bits |= data.get() << i;
               timeout = 1000;
               i++;
           }
           last = next;
       }
       interrupts();
       return bits;
   }
};

template <>
const byte SidewinderGPPro::HidDeviceType::description[] = {
  0x05, 0x01, // USAGE_PAGE (Generic Desktop)
  0x09, 0x05, // USAGE (Gamepad)
  0xa1, 0x01, // COLLECTION (Application)
  0x85, id,   //   REPORT_ID (DEVICE_ID)  
  0x05, 0x09, //   USAGE_PAGE (Button)
  0x19, 0x01, //   USAGE_MINIMUM (Button 1)
  0x29, 0x0A, //   USAGE_MAXIMUM (Button 10)
  0x15, 0x00, //   LOGICAL_MINIMUM (0)
  0x25, 0x01, //   LOGICAL_MAXIMUM (1)
  0x75, 0x01, //   REPORT_SIZE (1)  
  0x95, 0x0A, //   REPORT_COUNT (10)
  0x81, 0x02, //   INPUT (Data,Var,Abs)
  0x05, 0x01, //   USAGE_PAGE (Generic Desktop)
  0x09, 0x01, //   USAGE (Pointer)
  0xa1, 0x00, //   COLLECTION (Physical)
  0x09, 0x30, //     USAGE (X)
  0x09, 0x31, //     USAGE (Y)
  0x15, 0x00, //     LOGICAL_MINIMUM (0)
  0x25, 0x02, //     LOGICAL_MAXIMUM (2)
  0x75, 0x02, //     REPORT_SIZE (2)
  0x95, 0x02, //     REPORT_COUNT (2)
  0x81, 0x02, //     INPUT (Data,Var,Abs)
  0x75, 0x06, //     REPORT_SIZE (2)  
  0x95, 0x01, //     REPORT_COUNT (1)
  0x81, 0x03, //     INPUT (Cnst,Var,Abs) 
  0xc0,       //   END_COLLECTION
  0xc0,       // END_COLLECTION
};

