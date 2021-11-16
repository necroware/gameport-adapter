# Necroware's GamePort Adapter

Many people from the retro community still have their beloved joysticks and
gamepads from the early days. These devices often live their lives somewhere in
the dark corners of our basements and we don't dare to throw them away, because
of nostalgic reasons. They remind us so much of our childhoods, where we played
our Wing Commanders, X-Wings, Descents and many other games. These old joysticks
were all made to be connected to the game port, usually on a sound card. But
by the end of 90's and beginning of 2000's game ports vanished from our
computers and were replaced by USB and our old joysticks disappeared into the
past. Today not everybody has a full retro PC and many people are using their
modern computer with  DOSBox to play the old games, sometimes with a modern
USB joystick. But wouldn't it be great to play the old games with the same
joystick which we used back then? And this is where this adapter comes into
play. It can be used to connect gameport joysticks to a USB port.

![GamePort Adapter](./photo.jpg)

## How does it work?

The adapter is built around Arduino Pro Micro, which uses the same ATmega32U4
microcontroller as Leonardo. This microcontroller has built-in USB HID
capabilities and can be used to build HID input devices, like joysticks. The
adapter itself is super simple, the main brainwork was invested into the
software. Very much simplified, it reads the joystick states and sends the data,
via USB, to the computer, which thinks that it is communicating with a USB 
joystick.

## What is special about this one?

There are already plenty of gameport to USB adapters for basic analog joysticks
and, as far as I know, there were some efforts to communicate with Sidewinder
joysticks by Microsoft as well. But there is no universal adapter, which would
work with different types of joysticks. This adapter implements multiple drivers
for various analog and digital joysticks, with an option to add more, in the 
future.

Features overview:
* Support for generic analog joysticks with 2/4 buttons and 2/4 axes
* Four switches to select joystick type
* Autodetection for various digital protocol joysticks
* Auto-Calibration
* Very low input lag

## What is the difference between analog and digital joystick?

Many people call button-only joysticks or gamepads digital. This is kind of
right, because A button is either pressed or not. You can't have an analog
values in between. However by digital, in this case, we mean something different.
A gameport contains 15 pins, 8 of which are used for joystick communication. 4
pins are for buttons and carry digital values in sense of on/off and 4 pins are
for analog axes, which deliver voltage somewhere between 0V and 5V. Joysticks
which were made in the early days used this pinout. They could have a
maximum of 4 buttons and 4 axes and were DOS compatible. Later, in the time of
Windows 95/98 many joysticks were made to be plugged into a gameport as well,
but they were not limited to 4 buttons and 4 axes. They had a lot more exciting
features, like hat switches and throttle controls. But how did this work? Well
the manufacturers implemented their drivers to communicate with the joystick via
gameport using a proprietary communication protocol. For example, by using one
pin of the gameport as clock and another one as data, the possibilities were
almost limitless. Such joysticks are called digital as well, because they used
digital protocols to communicate with the PC. And suddenly, many features were
possible, but the price of these features was the lost compatibility to DOS. You
couldn't just plug such a joystick it into the gameport and expect it to work in
old DOS games. The plug was the same, but the signaling was completely different.

## Which joysticks does this adapter support?

Currently, the following drivers are implemented. To select the right driver, you
have to use four switches, as shown in the table. The switches may be changed or
extended in the future, so please pay attention to the updates.

Joystick Model           | Buttons | Axes  |  Hat | SW1-4 | Comments
-------------------------|---------|-------|------|-------|-------------------------------------
Generic Analog           | 2       | 2     | 0    | 0000  | 
Generic Analog           | 4       | 2     | 0    | 1000  | 
Generic Analog           | 4       | 3     | 0    | 0100  | 3rd Axis is throttle
Generic Analog           | 4       | 4     | 0    | 1100  | 
CH FlightStick           | 4       | 4     | 1    | 0010  | Analog, DOS-compatible
ThrustMaster             | 4       | 3     | 1    | 1010  | Analog, DOS-compatible 
Sidewinder GamePad       | 10      | 2     | 0    | 1110  | Digital protocol
Sidewinder 3D Pro        | 8       | 4     | 1    | 1110  | Digital protocol
Sidewinder 3D Pro Plus   | 9       | 4     | 1    | 1110  | First version of Precision Pro
Sidewinder Precision Pro | 9       | 4     | 1    | 1110  | Digital protocol
Sidewinder FFB Wheel     | 8       | 3     | 0    | 1110  | Digital, FFB not yet implemented
Gravis GamePad Pro       | 10      | 2     | 0    | 0001  | Digital protocol (GrIP)
Logitech WingMan Extreme | 6       | 3     | 1    | 1001  | Digital protocol (ADI)
Logitech CyberMan 2      | 8       | 6     | 0    | 1001  | Digital proticol (ADI)

*Remarks:*

- Please pay attention to how the same switches are used for different families
of digital devices. This is possible due to fully digital communication. Using
this method, the adapter implements autodetection as soon as it knows, that it
is connected to a digital joystick.
- Currently, only the drivers for the listed Sidewinder devices are implemented,
since I have no other models at hand. The Precision Pro works natively on USB
as well, but was still implemented, because we can.
- Gravis used their GrIP protocol, which is currently implemented only for the
Gravis GamePad Pro, but without daisy chaining possibility so far.
- The implementation of the ADI protocol used by Logitech should work with all
the devices which support that protocol. However only the listed Logitech devices 
were tested so far.

## Which joysticks were tested?

Many joysticks in the wild are using the same digital protocol or are backwards
compatible to the analog joysticks as they were used back in the days in DOS.
Following list contains all the devices wich were reported by others as working
so far:

* Gravis Analog Pro
* Gravis PC GamePad
* Gravis GamePad Pro
* QuickShot QS-201 "Super Warrior"
* QuickShot QS-203 "Avenger"
* Sidewinder GamePad
* Sidewinder 3D Pro
* Sidewinder 3D Pro Plus
* Sidewinder Precision Pro
* Sidewinder ForceFeedBack Wheel
* Logitech WingMan Extreme Digital
* Logitech CyberMan 2

Sidewinder 3D Pro can be switched between analog and digital mode and in analog
mode it can emulate the ThrustMaster and CH FlightStick. That's why you see them in
the table above. Unfortunately I don't possess those joysticks in reality, so it may
be that the implementation is not quite correct.

## What is auto calibration?

Old analog joysticks have resistors inside, which are specified to be 100 kOhm.
Unfortunately, these resistors are either worn out, bad quality or were wrong from
the beginning. Therefore most of the generic analog joysticks had adjustment
screws to correct the center point of the joystick. Also many games had
calibration options in their settings to readjust the joystick. With USB and new
digital solutions the calibration was not required anymore and was completely
implemented in the joysticks and/or drivers. Many modern games have no option
to re-calibrate the joystick anymore. If we try to play such newer games with
an old analog joystick through this adapter, the joystick center point would be
totally offset. That's why the adapter implements auto calibration internally and
presents already corrected values to the operation system. 

__ATTENTION__: a hard requirement for using the analog joysticks is that during 
plugging into the USB port all axes must be in their middle state, because all
the subsequent calibration happens based on the initial state.

## Technical insights into implementation

The code is well documented, so if you are interested in the details, feel free
to take a look into the driver implementation. All the analog joysticks were
actually a straight forward task, but the Sidewinder digital protocol was kind 
of tough. There are already some implementations of Sidewinder protocol made for
Arduino. However, they are mostly only for the Sidewinder GamePad and not really 
universally made. In the end I was heavily inspired by the Linux Sidewinder 
driver implementation. The code you can see in this project is a complete rewrite, 
I just took the Linux driver implementation as a reference to understand how it 
works. Also the Sidewinder patent US#5628686A helped a lot, especially with 
switching between digital and analog mode for Sidewinder 3D Pro.

In opposition to the already mentioned Sidewinder for Arduino implementations,
this one doesn't rely on interrupts. This implementation is from a similare idea 
to what the Linux driver does. It polls the port and makes a lot of things
simpler due to synchronous process. The biggest problem was that the Sidwinder
devices send the data incredibly fast, with a clock pulse of only 5us.  It was
not possible to use Arduino's digitalRead(...) function for that. It was too
slow with about 2.7us per call on an Arduino Pro Micro of 16MHz.  It simply
made it impossible to poll 5us pulses, with such a slow function, not even
considering doing something with the data in between. So out of that need, my 
own implementation came about, which is up to 50% faster and needs only about 
1.6us per call, on the same hardware. The custom I/O functions made it possible
to read the data with the speed which a Sidewinder joystick requires. The best 
part is that the code is written in pure C++. It is very simple to read and to 
use. There are no macros, no assembler or any dirty hacks, just a lot of
optimization.

## Bill of materials (BOM)

The hardware is super simple. To build an adapter you'll need the PCB from this
project and following parts:

Part    | Count | LCSC #  | Digikey #                | Comment
--------|-------|---------|--------------------------|------------------------------------------
CONN1   |   1   | C77835  | 609-5371-ND              | DB15 female connector
R1..R4  |   4   | C172965 | 13-MFR-25FTE52-100KCT-ND | 100 kOhm resistors
SW1     |   1   | C15781  | 2449-KG04ET-ND           | DIP-4 switch
U1      |   1   | C72120  | ED3024-ND                | DIP24 Socket (optional)
U1      |   1   | ---     | 1568-1060-ND             | Arduino Pro Micro (ATmega32U4 16MHz, 5V)

## Known issues

* *Some axes on an analog joystick are offset*

Auto calibration requires all the axes to be in the center position during 
initialization. Please see the paragraph about auto calibration.

* *Joystick doesn't work*

Make sure that you are using one of supported joysticks or a joystick which can
work in legacy analog mode

* *MicroUSB port on the Arduino is not stable enough*

This can be a problem indeed, since it is very easy to rip off the USB port
from the PCB. May be there will be other solution in the future. To use an 
Arduino Pro Micro was just the cheapest way so far and they all have these
SMD microUSB ports.

## How to help the project?

The best way is to implement more drivers. Since I have only the joysticks
mentioned above, I can't contribute more than what is currently included.

## Special thanks

I would like to give some special thanks to *Creopard* from the German 
dosreloaded.de community for providing me the mentioned joysticks. Without that
donation this project wouldn't be possible. Especially dealing with Sidewinder
3D Pro was a very exciting task.

## Links
* [Linux Sidewinder Driver](https://github.com/torvalds/linux/blob/master/drivers/input/joystick/sidewinder.c)
* [Sidewinder patent](https://patents.google.com/patent/US5628686A/en)
* [Creopard Retro Site](https://www.creopard.de/)

