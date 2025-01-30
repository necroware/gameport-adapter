// This file is part of Necroware's GamePort adapter firmware.

// Emulation of the legacy NE558 gameport
// User for a digital simulation of signals NE558 as in QuickJoy M-5 SV-201
// Copyright (C) 2022 mamut-tme
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

#include "Joystick.h"

template <size_t Axes, size_t Buttons>
class NE558 : public Joystick {
  public:

    static_assert(Axes > 0 && Axes <= 4);

    bool init() override {
      zeroCaps();
            
      measurement(30000);
      
      log("Calibration values:");
      for(uint8_t i=0; i<Axes; i++)
        log("Axis %d: %d", i, axes[i].timeToPulse);

      //find the longer value
      timeout_cal=0;
      for(uint8_t i=0; i<Axes; i++)
        if(axes[i].found==true && timeout_cal<axes[i].timeToPulse) timeout_cal=axes[i].timeToPulse;
      log("Max value from measurement: %d", timeout_cal);

      //increase it by 200% (two times longer it should take from middle to left) +20%
      timeout_cal*=2.2;

      //safety if nothing connected, 6ms seems reasonable
      if(timeout_cal==0) timeout_cal=6000;
      
      log("Final calibration value for measurements: %d", timeout_cal);
      
      return true;
    }

    bool update() override {
      log("new measurement");

      zeroCaps();

      measurement(timeout_cal);
      log("Measured values:");
      for(uint8_t i=0; i<Axes; i++)
        log("Axis %d: %d", i, axes[i].timeToPulse);
      
      //scaling and saturation of the value on both axes
      for (int i = 0; i < Axes; i++) {
        axes[i].timeToPulse = axes[i].timeToPulse /(timeout_cal/1000);
        if(axes[i].timeToPulse > 1023)
          m_state.axes[i] = 1023;
        m_state.axes[i]= axes[i].timeToPulse;
      }

      //wait some time
      delay(2);

      //read the buttons
      m_state.buttons = m_button1.isLow() | m_button2.isLow() << 1 | m_button3.isLow() << 2 | m_button4.isLow() << 3;
      
      return true;
    }

    const State& getState() const override {
      return m_state;
    }

    const Description& getDescription() const override {
      static const Description description {
        "NE558 Gameport Emulation", Axes, Buttons, 0
      };
      return description;
    }

  private:
    DigitalInput<GamePort<3>::pin, false> m_x1;
    DigitalInput<GamePort<11>::pin, false> m_x2;
    DigitalInput<GamePort<6>::pin, false> m_y1;
    DigitalInput<GamePort<13>::pin, false> m_y2;
    DigitalInput<GamePort<2>::pin> m_button1;   
    DigitalInput<GamePort<7>::pin> m_button2;
    DigitalInput<GamePort<10>::pin> m_button3;
    DigitalInput<GamePort<14>::pin> m_button4;
    State m_state;
    uint16_t timeout_cal;

    struct axis {
      uint16_t timeToPulse;
      bool found;
    };
    struct axis axes[4];

    bool measurement(uint16_t timeout){
      int i=0;
      for(i=0; i<Axes; i++){
        axes[i].timeToPulse=0;
        axes[i].found=false;
      }
      unsigned long start = micros();

      while ((micros() - start) < timeout) {
        if (m_x1.isHigh() && (axes[0].found == false) && Axes>=1) {
          axes[0].timeToPulse = micros()-start;
          axes[0].found = true;
        }
        if (m_y1.isHigh() && (axes[1].found == false) && Axes>=2) {
          axes[1].timeToPulse = micros()-start;
          axes[1].found = true;
        }
        if (m_x2.isHigh() && (axes[2].found == false) && Axes>=3) {
          axes[2].timeToPulse = micros()-start;
          axes[2].found = true;
        }
        if (m_y2.isHigh() && (axes[3].found == false) && Axes==4) {
          axes[3].timeToPulse = micros()-start;
          axes[3].found = true;
        }
        
      }
      return(true);
    }

    bool zeroCaps(){
      //set all axes pins as output
      pinMode(GamePort<3>::pin, OUTPUT);
      pinMode(GamePort<11>::pin, OUTPUT);
      pinMode(GamePort<6>::pin, OUTPUT);
      pinMode(GamePort<13>::pin, OUTPUT);
      //set all axes pins to 0 - shorten all axes pins to gnd (discharging of the capacitor) 
      digitalWrite(GamePort<3>::pin, LOW);
      digitalWrite(GamePort<11>::pin, LOW);
      digitalWrite(GamePort<6>::pin, LOW);
      digitalWrite(GamePort<13>::pin, LOW);
      //give time to discharge
      delayMicroseconds(50);

      //set all axes pins back to input, no pullup.
      pinMode(GamePort<3>::pin, INPUT);
      pinMode(GamePort<11>::pin, INPUT);
      pinMode(GamePort<6>::pin, INPUT);
      pinMode(GamePort<13>::pin, INPUT); 
      return(true);
    }

};
