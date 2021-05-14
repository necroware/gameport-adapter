#pragma once

#include "DigitalPin.h"
#include "Log.h"

class Sidewinder {
public:

   enum class Model {
       SW_UNKNOWN,
       SW_GAMEPAD,
       SW_3DPRO
   };

   struct State {
       int16_t axis[10]{0};
       int32_t buttons{0};
   };

   Model reset() {
       log("Resetting...");
       cooldown();       
       m_model = guessModel(readPacket());
       if (m_model == Model::SW_UNKNOWN) {
         // No data. 3d Pro analog mode?
         enableDigitalMode();
         cooldown();
         m_model = guessModel(readPacket());
       }
       cooldown();
       log("Detected model %d", m_model);          
       return m_model;
   }

   State readState() {
       const auto packet = readPacket();
       State state;
       if (!decode(packet, state)) {
           reset();
       }
       return state;
   }

   void cooldown() const {
       m_trigger.setLow();
       delayMicroseconds(6000);
   }

private:

   enum {
      PULSE_DURATION = 60,
   };

   struct InterruptStopper {
       InterruptStopper() { noInterrupts(); }
       ~InterruptStopper() { interrupts();}
   };

   struct Packet {
       byte bits[256];
       uint16_t length{0u};
   };

   template <Model M>
   struct Decoder {
       static bool decode(const Packet& packet, State& state);
   };

   static Model guessModel(const Packet& packet) {
       switch(packet.length) {
           case 15: return Model::SW_GAMEPAD;
           case 64: return Model::SW_3DPRO;
           default: return Model::SW_UNKNOWN;
       }
   }

   DigitalInput<GamePort<2>::pin, true> m_clock;
   DigitalInput<GamePort<7>::pin, true> m_data;
   DigitalOutput<GamePort<3>::pin> m_trigger;
   Model m_model{Model::SW_UNKNOWN};

   void enableDigitalMode() const {
      static const int magic = 200;
      static const int seq[] = { magic, magic + 725, magic + 300, 0 };
      log("Trying to enable digital mode");
      cooldown();
      InterruptStopper interruptStopper;
      for (auto i = 0u; seq[i]; i++) {
        m_trigger.pulse(PULSE_DURATION);
        delayMicroseconds(seq[i]);
      }      
      m_trigger.pulse(PULSE_DURATION);
   }

   Packet readPacket() const {
       InterruptStopper interruptStopper;
       const auto ready = m_clock.isHigh();
       m_trigger.setHigh();
       Packet packet;
       if (ready || m_clock.wait(Edge::rising, PULSE_DURATION * 10)) {
           while(packet.length < sizeof(packet.bits)) { 
               if (!m_clock.wait(Edge::rising, PULSE_DURATION)) {
                   break;
               }
               packet.bits[packet.length++] = m_data.get();
           }
       }
       m_trigger.setLow();
       delayMicroseconds(PULSE_DURATION);
       return packet;
   }

   bool decode(const Packet& packet, State& state) const;
};

template <>
class Sidewinder::Decoder<Sidewinder::Model::SW_GAMEPAD> {
public:
    static bool decode(const Sidewinder::Packet& packet, Sidewinder::State& state) {

        const auto checksum = [&](){
            byte result = 0u;
            for (auto i = 0u; i < packet.length; i++) {
               result ^= packet.bits[i] & 1;
            }
            return result;
        };

        if (packet.length != 15 || checksum() != 0) {
            return false;
        }

        // Bit 0-1: x-axis (10-left, 01-right, 11-middle)
        // Bit 2-3: y-axis (01-up, 10-down, 11-middle)
        // Bit 4-13: 10 buttons
        // Bit 14: checksum
        for (auto i = 0u; i < 10; i++) {                
            state.buttons |= (~packet.bits[i + 4] & 1) << i;
        }
        state.axis[0] = 1 + packet.bits[0] - packet.bits[1];
        state.axis[1] = 1 + packet.bits[3] - packet.bits[2];            

        return true;
    }    
};

template <>
class Sidewinder::Decoder<Sidewinder::Model::SW_3DPRO> {
public:
    static bool decode(const Sidewinder::Packet& packet, Sidewinder::State& state) {
        return false;
    }    
};

inline bool Sidewinder::decode(const Packet& packet, State& state) const {
    switch(m_model) {
        case Model::SW_GAMEPAD:
           return Decoder<Model::SW_GAMEPAD>::decode(packet, state);
        case Model::SW_3DPRO:
           return Decoder<Model::SW_3DPRO>::decode(packet, state);
        case Model::SW_UNKNOWN:
           break;
    }
    return false;
}

