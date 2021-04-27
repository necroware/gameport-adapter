#pragma once

#include "HID.h"
#if !defined(_USING_HID)
#error "Incompatible USB HID device"
#endif

template <typename T>
class HidDevice {
public:
   static void activate() {
      static HidDevice instance;
   }

   static void send(const void* data, size_t size) {
      HID().SendReport(id, data, size);
   }

private:
   static const byte id{0x03};
   static const byte description[] PROGMEM;

   explicit HidDevice() {
      static HIDSubDescriptor node(description, sizeof(description));
      HID().AppendDescriptor(&node);
   }
};

