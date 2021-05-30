// This file is part of Necroware's GamePort adapter firmware.
// Copyright (C) 2021 Necroware 
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "DigitalPin.h"
#include "Log.h"

/// Class to for communication with Sidewinder joysticks.
/// @remark This is a green field implementation, but it was heavily
///         inspired by Linux Sidewinder driver implementation. See
//          https://github.com/torvalds/linux/blob/master/drivers/input/joystick/sidewinder.c
class Sidewinder {
public:

   /// Supported Sidewinder model types.
   enum class Model {
       /// Unknown model.
       SW_UNKNOWN,

       /// Sidewinder GamePad
       SW_GAMEPAD,

       /// Sidewinder 3D Pro
       SW_3DPRO
   };

   /// Joystick state.
   struct State {
       uint16_t buttons{0};
       uint8_t axis[5]{0};
       uint8_t hat;
   };

   /// Gets the detected model.
   /// @returns the detected joystick model
   Model getModel() const {
       return m_model;
   }

   /// Resets the joystick and tries to detect the model.
   void reset() {
       log("Trying to reset...");
       cooldown();
       m_model = guessModel(readPacket());
       while (m_model == Model::SW_UNKNOWN) {
         // No data. 3d Pro analog mode?
         enableDigitalMode();
         m_model = guessModel(readPacket());
       }
       log("Detected model %d", m_model);
   }

   /// Reads joystick state.
   /// @returns the state of axis, buttons etc.
   /// @remark if reading the state fails, the last known state is
   ///         returned and the joystick reset is executed.
   State readState() {
       const auto packet = readPacket();
       State state;
       if (decode(packet, state)) {
         m_state = state;
         m_errors = 0;
       } else {
         m_errors++;
         log("Packet decoding failed %d time(s)", m_errors);
         if (m_errors > MAX_ERRORS) {
           reset();
         }
       }
       return m_state;
   }

   /// Joystick cool down timeout.
   /// 
   /// This is need to settle the signals between the reads.
   void cooldown() const {
       m_trigger.setLow();
       delayMicroseconds(2000);
   }

private:

   enum {
      PULSE_DURATION = 60,
      MAX_ERRORS = 3,
   };

   /// Interrupt guard (RAII).
   ///
   /// This class is used to deactivate the interrupts in performance
   /// critical sections. The interrupt is reactivated as soon as this
   /// guard runs out of scope.
   struct InterruptStopper {
       InterruptStopper() { noInterrupts(); }
       ~InterruptStopper() { interrupts();}
   };

   /// Internal bit structure which is filled by reading from the joystick.
   struct Packet {
       byte bits[256];
       uint16_t length{0u};
   };

   /// Model specific status decoder function.
   template <Model M>
   struct Decoder {
       static bool decode(const Packet& packet, State& state);
   };

   /// Guesses joystick model from the size of the packet.
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
   State m_state;
   int m_errors{0u};

   /// Enables digital mode for 3D Pro.
   //
   /// The 3D Pro can work as legacy analog joystick or in digital mode.
   /// This mode has to be activated explicitly. In this function timing
   /// is very important. See Patent: US#5628686 (page 19) for details.
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
      cooldown();
   }

   /// Read bits packet from the joystick.
   ///
   /// This part is extremely performance and timing critical. Change only, if
   /// you know, what you are doing. 
   Packet readPacket() const {

       InterruptStopper interruptStopper;
       const auto ready = m_clock.isHigh();
       m_trigger.setHigh();
       // We are reading into a byte array instead of an uint64_t, because of two
       // reasons. First, bits packets can be larger, than 64 bits. We are actually
       // not interested in packets, which are larger than that, but may be in the
       // future we'd need to handle them as well. Second, for reading into an 
       // uint64_t we would need to shift between the clock impulses, which is 
       // impossible to do in time. Unfortunately this shift is extremely slow on
       // an Arduino and it's just faster to write into an array. One bit per byte.
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

   /// Decodes bit packet into a state.
   bool decode(const Packet& packet, State& state) const;
};

/// Bit decoder for Sidewinder GamePad.
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

/// Bit decoder for Sidewinder 3D Pro.
template <>
class Sidewinder::Decoder<Sidewinder::Model::SW_3DPRO> {
public:
    static bool decode(const Sidewinder::Packet& packet, Sidewinder::State& state) {

        const auto value = [&]() {
            uint64_t result{0u};
            for (auto i = 0u; i < packet.length; i++) {
                result |= static_cast<uint64_t>(packet.bits[i]) << i;
            }
            return result;            
        }();

        const auto bits = [&](uint8_t start, uint8_t length) {
            const auto mask = (1 << length) - 1;
            return (value >> start) & mask;
        };

        if (packet.length != 64 || !checkSync(value) || checksum(value)) {
            return false;
        }

        // bit 38: button 8 + bit 8-15: buttons 1-7 (low active)
        state.buttons = ~(bits(8, 7) | (bits(38, 1) << 7));

        // bit 3-5 + bit 16-22: x-axis (value 0-1023)
        state.axis[0] = map(bits(3, 3) << 7 | bits(16, 7), 0, 1023, 0, 255);
        
        // bit 0-2 + bit 24-30: y-axis (value 0-1023)        
        state.axis[1] = map(bits(0, 3) << 7 | bits(24, 7), 0, 1023, 0, 255);

        // bit 35-36 + bit 40-46: rz-axis (value 0-512)        
        state.axis[2] = map(bits(35, 2) << 7 | bits(40, 7), 0, 512, 0, 255);

        // bit 32-34 + bit 48-54: throttle-axis (value 0-1023)
        state.axis[3] = map(bits(32, 3) << 7 | bits(48, 7), 0, 1023, 0, 255);

        // bit 6-7 + bit 60-62 (9 pos, 0 center, 1-8 clockwise)
        state.hat = bits(6, 1) << 3 | bits(60, 3); 

        return true;
    }

private:
    /// Checks sync bits.
    /// 
    /// This code was taken from Linux driver as is.
    static bool checkSync(uint64_t value) {
        return !((value & 0x8080808080808080ULL) ^ 0x80);
    }

    /// Calculates checksum.
    /// 
    /// This code was taken from Linux driver as is.
    static byte checksum(uint64_t value) {
        auto result = 0u;
        while (value) {
          result += value & 0xf;
          value >>= 4;
        }
        return result & 0xf;
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

