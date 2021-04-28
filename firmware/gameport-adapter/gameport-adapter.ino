#include "CHFlightstickPro.h"
#include "GenericJoystickB2A2.h"
#include "GenericJoystickB4A2.h"
#include "GenericJoystickB4A4.h"
#include "SidewinderGPPro.h"
#include "DigitalPin.h"

static Joystick* joystick;

static Joystick* create(int sw) {
    switch(sw) {
        case 0b0001: return new GenericJoystickB4A2;
        case 0b0010: return new GenericJoystickB4A4;
        case 0b0011: return new CHFlightstickPro;
        case 0b1000: return new SidewinderGPPro;
        default: return new GenericJoystickB2A2;
    }
}

void setup() {
  const auto sw1 = DigitalInput(14, true).isLow();
  const auto sw2 = DigitalInput(15, true).isLow();
  const auto sw3 = DigitalInput(20, true).isLow(); 
  const auto sw4 = DigitalInput(21, true).isLow();  
  joystick = create(sw4 << 3 | sw3 << 2 | sw2 << 1 | sw1);
  joystick->init();
  Serial.begin(9600);
}

void loop() {
  joystick->update();
}
