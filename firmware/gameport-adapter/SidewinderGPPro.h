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
       const auto bits = m_sw.readBits();
       if (m_sw.validate(bits) && bits.count == 15) {
           // Input data (MSB, low active):
           // Bit 0: checksum
           // Bit 1-11: 10 buttons
           // Bit 12-13: y-axis (01-up, 10-down, 11-middle)
           // Bit 14-15: x-axis (10-left, 01-right, 11-middle)
           //
           // Output data (LSB, high active):
           // Bit 0-1: x-axis (00-left, 01-middle, 10-right)
           // Bit 2-3: y-axis (00-up, 01-middle, 10-right)
           // Bit 4-14: 10 buttons
           const auto decode = [](uint16_t buttons) {
               switch(buttons) {
                   case 0b0101: return 0b0010; // down-left
                   case 0b0110: return 0b0000; // up-left,
                   case 0b0111: return 0b0001; // left
                   case 0b1001: return 0b1010; // down-right
                   case 0b1010: return 0b0100; // up-right
                   case 0b1011: return 0b1001; // right
                   case 0b1101: return 0b0110; // down
                   case 0b1110: return 0b0100; // up
                   default:     return 0b0101; // middle
               }              
           };

           uint16_t data = decode((bits.data >> 11) & 0b1111);
           for (auto i = 1; i < 11; i++) {
               data |= (~(bits.data >> i) & 1) << (14 - i);
           }
           HidDeviceType::send(&data, sizeof(data));
       }
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

