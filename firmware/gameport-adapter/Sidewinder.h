#pragma once

#include "DigitalPin.h"

class Sidewinder {
public:

   struct Bits {
       uint64_t data{0u};
       uint16_t count{0u};
   };

   Bits readBits() const {
       InterruptStopper interruptStopper;
       const auto ready = m_clock.isHigh();
       m_trigger.setHigh();
       Bits bits;
       if (ready || m_clock.wait(Edge::rising, PULSE_DURATION * 10)) {
           bits = readData();
       }
       m_trigger.setLow();
       log("Packet length: ", bits.count);
       return bits;
   }

   bool validate(Bits bits) const {
       auto checksum = 0u;
       while (bits.data) {
           checksum ^= bits.data & 1;
           bits.data >>= 1;
       }
       return checksum == 0;
   }

   bool reset() const {
       log("Resetting...");
       cooldown();
       auto bits = readBits();
       if (!bits.count) { // No data. 3d Pro analog mode?
         enableDigitalMode();
         cooldown();
         bits = readBits(); // Retry reading packet
         if (bits.count == 0) { // No data -> FAIL
           log("Failed to init driver");
           return false;
         }
       }

       bits = readBits();
       bits = readBits();
       cooldown();
       return true;
   }

private:

   static void cooldown() {
       delayMicroseconds(6000);
   }

   template <typename T, typename ...Args>
   static void log(const T& head, Args... tail) {
       Serial.print(head);
       log(tail...);
   }

   template <typename T>
   static void log(const T& head) {
       Serial.println(head);
   }

   enum {
      PULSE_DURATION = 60,
   };

   struct InterruptStopper {
       InterruptStopper() { noInterrupts(); }
       ~InterruptStopper() { interrupts();}
   };

   DigitalInput<GamePort<2>::pin, true> m_clock;
   DigitalInput<GamePort<7>::pin, true> m_data;
   DigitalOutput<GamePort<3>::pin> m_trigger;

   void enableDigitalMode() const {
      static const int magic = 200;
      static const int seq[] = { magic, magic + 725, magic + 300, 0 };
      log("Trying to enable digital mode");
      m_trigger.setLow();
      cooldown();
      InterruptStopper interruptStopper;
      for (auto i = 0u; seq[i]; i++) {
        m_trigger.pulse(PULSE_DURATION);
        delayMicroseconds(seq[i]);
      }      
      m_trigger.pulse(PULSE_DURATION);
   }

   Bits readData() const {
       Bits bits;
       for (auto i = 0u; i < 256; i++) {             
           if (!m_clock.wait(Edge::rising, PULSE_DURATION * 2)) {
               break;
           }
           bits.data = (bits.data << 1) | m_data.get();
           bits.count++;
       }
       return bits;
   }
};
