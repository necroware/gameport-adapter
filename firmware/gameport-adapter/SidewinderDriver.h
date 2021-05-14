#pragma once 

#include "DigitalPin.h"
#include "Driver.h"
#include "GamePort.h"
#include "HidDevice.h"
#include "Sidewinder.h"

class SidewinderDriver : public Driver {
public:

   template <Sidewinder::Model M> struct HidType;
   using HidGamePad = HidDevice<HidType<Sidewinder::Model::SW_GAMEPAD>>;
   using Hid3DPro = HidDevice<HidType<Sidewinder::Model::SW_3DPRO>>;

   void init() override {
       m_sw.reset();
       switch(m_sw.getModel()) {
           case Sidewinder::Model::SW_GAMEPAD:
               Serial.println("Detected Sidewinder GamePad");
               HidGamePad::activate();
               break;
           case Sidewinder::Model::SW_3DPRO:
               Serial.println("Detected Sidewinder 3D Pro");
               Hid3DPro::activate();
               break;
           case Sidewinder::Model::SW_UNKNOWN:
               Serial.println("Unknown input device");
               break;
       }
   }

   void update() override {
       const auto state = m_sw.readState();
       switch(m_sw.getModel()) {
           case Sidewinder::Model::SW_GAMEPAD:
               sendGamePad(state);
               break;
           case Sidewinder::Model::SW_3DPRO:
               send3DPro(state);
               break;
           case Sidewinder::Model::SW_UNKNOWN:
               Serial.println("Unknown input device");
               break;
       }
       m_sw.cooldown();
   }

private:
   static void sendGamePad(const Sidewinder::State& state) {
       const uint16_t data = state.buttons << 4 | state.axis[0] << 2 | state.axis[1];
       HidGamePad::send(&data, sizeof(data));
   }

   static void send3DPro(const Sidewinder::State& state) {
       struct {
           byte x,y,rz,throttle,hat,buttons;
       } data = {
          state.axis[0],
          state.axis[1],
          state.axis[2],
          state.axis[3],
          state.hat,
          static_cast<byte>(state.buttons)
       };
       Hid3DPro::send(&data, sizeof(data));
   }

   Sidewinder m_sw;
};

template <>
const byte SidewinderDriver::HidGamePad::description[] = {
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

template <>
const byte SidewinderDriver::Hid3DPro::description[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x04,       // Usage (Joystick)
    0xa1, 0x01,       // Collection (Application)
    0x85, id,         //   Report ID (id)
    0x05, 0x01,       //   Usage Page (Generic Desktop)
    0x09, 0x30,       //   Usage (X)
    0x09, 0x31,       //   Usage (Y)
    0x09, 0x35,       //   Usage (Rz)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0xff,       //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x03,       //   Report Count (3)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x05, 0x02,       //   Usage Page (Simulation Controls)
    0x09, 0xBB,       //   Usage (Throttle)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0xff,       //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x05, 0x01,       //   Usage Page (Generic Desktop)
    0x09, 0x39,       //   Usage (Hat switch)
    0x15, 0x01,       //   Logical Minimum (1)
    0x25, 0x08,       //   Logical Maximum (8)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x01,       //   Report Count (1)
    0x81, 0x42,       //   Input (Data,Var,Abs)
    0x05, 0x09,       //   Usage Page (Button)
    0x19, 0x01,       //   Usage Minimum (1)
    0x29, 0x08,       //   Usage Maximum (8)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x01,       //   Logical Maximum (1)
    0x75, 0x01,       //   Report Size (1)
    0x95, 0x08,       //   Report Count (8)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0xc0,             // End Collection
};

