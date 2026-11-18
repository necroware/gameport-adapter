# ADR 0001: Force feedback for the Sidewinder Force Feedback Wheel

## Status

Accepted (2026-11-16). All effect types verified on a real wheel
(2026-11-17).

## Context

The gameport version of the Microsoft Sidewinder Force Feedback Wheel reports
its axes and buttons through the regular Sidewinder digital protocol, which the
adapter already supports. Force feedback commands travel on a separate line:
the wheel listens to MIDI-like serial data (31250 baud, 8N1) on gameport pin 12
("MIDI out" of a sound card). On the adapter PCB, that pin is routed to Pro
Micro pin D5 (PC6), which has no hardware UART.

What is publicly known about the wheel protocol comes from the
[adapt-ffb-joy wiki](https://github.com/tloimu/adapt-ffb-joy/blob/wiki/SidewinderFFBWheel.md),
which was captured with Microsoft's Force Editor:

* initialization and auto-center on/off sequences (`f3 ...`, `f1 ...`),
* effect upload as SysEx (`f0 00 01 0a 15 20 <type> ... <checksum> f7`),
* effect modification (`f1 <checksum> <0x40|attribute> <id> <lsb> <msb>`),
* play, stop and delete commands (`f2 <op|checksum> <id>`).

Constant force is well documented. Spring, damper and inertia store their
parameters in several bytes whose encoding is not understood ("formula not
known"). How the wheel assigns effect IDs, and whether duration 0 means
"infinite", was never documented.

On the USB side, force feedback without custom drivers requires the USB HID
Physical Interface Device (PID) class. Windows (DirectInput) and Linux
(`hid-pidff`) both support it natively.

The ATmega32U4 has 2.5 KB RAM and 28 KB usable flash. Before this change the
firmware used 1358 bytes RAM and 19210 bytes flash.

## Decision

1. **USB:** When a Force Feedback Wheel is detected, the adapter appends a USB
   HID PID descriptor to its joystick descriptor. The descriptor lives in
   PROGMEM and declares all PID effect types except custom forces:
   constant force, ramp, square, sine, triangle, sawtooth up/down, spring,
   damper, inertia and friction, plus envelopes.
   Output and feature reports arrive over the control endpoint
   (SET_REPORT/GET_REPORT), so no additional USB endpoint is needed.
2. **Effect handling in the adapter:** The adapter keeps track of the PID
   effects itself (`ForceFeedback.h`) and calculates one net force from all
   playing effects, including effect gain, device gain, direction, duration
   and envelopes. Constant forces, ramps and periodic effects are calculated
   from the time since their start. Springs are calculated from the wheel
   position, with offset,
   coefficients, saturation and dead band as defined by PID. Dampers,
   inertia and friction are not part of the net force; the adapter only sums
   their coefficients per type.
3. **Wheel side:** The adapter plays four effects on the wheel
   (`SidewinderWheelFfb.h`): a constant force (effect ID 2), whose magnitude
   and direction are modified when the net force changes, and the wheel's
   own damper, inertia and friction (effect IDs 3-5), which are uploaded
   again when their summed coefficients change. While no host application
   holds an effect, the wheel's own auto-center spring is turned on again.
4. **MIDI output:** Bit-banged 8N1 at 31250 baud on D5, one byte at a time
   with interrupts disabled for ~320 µs. Pin D5 is only driven once a wheel
   has been detected, because some gameport devices tie pin 12 to ground.
5. **MIDI enable:** The wheel ignores all MIDI data while gameport pin 11
   (X2 axis) is high. A PC game port holds that line low; the adapter's
   100 kOhm pull-down resistor loses against the wheel's pull-up. So the
   adapter drives pin 11 (A1) low once a wheel has been detected.

6. **Wheel classification:** The wheel's input axes are wrapped in a
   physical collection with the usage "Automobile Simulation Device"
   (Simulation Controls page). DirectInput and Wine classify the device as
   driving device that way, so games like Forza Horizon find the wheel. The
   axes keep their usages, so Linux still reports the steering as X axis.

## Alternatives considered

* **Translate every PID effect into a native wheel effect.** This would
  let the wheel's own microcontroller render all effects, but it depends
  on the poorly understood condition-effect encoding and on the wheel's
  effect ID allocation for many simultaneous effects. Rejected. Only
  damper, inertia and friction are played natively (see below), as one
  effect each with a fixed ID.
* **Calculate dampers (and inertia, friction) in the adapter from the wheel
  velocity.** Tried for the damper
  first: the velocity is measured from 10 bit positions and reaches the
  wheel 15-20 ms late. With that delay a damper strong enough to be felt
  overshoots on every update and the wheel jitters. Rejected.
* **Use the hardware UART (D1/TX).** It is not routed to the gameport on the
  existing PCB. Rejected because it would require a hardware change.

## Consequences

* Only a few wheel features must work for force feedback: auto-center
  on/off, upload of a constant force and three condition effects, and the
  modify command. The rest of the logic runs on the host side of the adapter and is
  covered by host unit tests (`firmware/gameport-adapter/test`).
* Springs, ramps and periodic effects run through the constant force. Their
  quality is limited by the loop rate of the adapter (one position packet
  and at most two modify messages per iteration, roughly 150 Hz) and the 7
  bit force resolution, so vibrations above roughly 30 Hz get coarse. Linux
  plays rumble effects as sine effects, so rumble works too.
* On Linux, Proton's default input path (SDL) does not report the adapter
  as a wheel. With Proton's hidraw path, Wine reads the descriptor itself
  and sends the PID reports directly to the adapter like Windows does; the
  README describes the setup. Verified with Forza Horizon 6 (Proton Hotfix).
* Wine enables the direction of every effect, but leaves it at 0, when the
  effect has no axes, and gives left or right by the sign of the magnitude.
  Linux sends 90° or 270° instead. Direction 0 is therefore treated as no
  direction, because it would have no force on the wheel axis anyway.
* All force calculations use integer math and a sine table. Floating point
  math cost about 1.6 KB of the 28 KB flash.
* Condition coefficients were mapped on the wheel by timing a constant force
  push: in the damper and inertia SysEx the last byte sets the coefficient
  for turning right (`0x00` full, `0x3e` none, `0x7d` negative), the second
  parameter byte the one for turning left (`0x7d` full, `0x3e` none, `0x00`
  negative). Friction has a single coefficient byte (`0x7e` full, `0x3f`
  none, `0x00` negative), so the adapter uses the average of the positive
  and negative PID coefficients. Offset, saturation and dead band of these
  conditions are not supported. The wheel ignores most modify messages for
  the coefficients, so an effect is deleted and uploaded again (~9 ms of
  MIDI) whenever one of its 7 bit coefficients changes.
* The wheel's own inertia resonates (felt as jitter) at low turning speeds,
  at any strength, likely because it estimates the acceleration from its
  encoder. Changing the unmapped parameter bytes did not fix it: bytes 4/6
  of the spring's capture (`4e`/`2f`) made it worse, halving bytes 1/7
  (`1f`/`5e`) helped only slightly. The inertia is kept as the wheel plays
  it. A host side inertia would resonate more, for the same delay reason as
  the damper.
* The Set Condition report has no Negative Saturation field: Linux
  (`hid-pidff`) scales that field as a signed value, so a full saturation of
  `0xffff` would arrive as 0. Without the field, the kernel skips it and the
  adapter applies the positive saturation to both sides. It also has no
  Parameter Block Offset, because the wheel has only one axis.
* The MIDI link limits update rate to roughly one modify message (6 bytes,
  ~2 ms) per loop iteration.
* Verified on hardware: the first uploaded effect gets ID 2, the second ID 3,
  and IDs are reused after deleting effects. Duration 0 plays until the
  effect is deleted, direction `7d` turns the wheel clockwise
  (positive X) and `00` counter-clockwise.
* The `0x40` flag of the modify command must be set for non-zero values and
  cleared for zero values. Otherwise the wheel ignores the message. The wiki
  calls it a "default indicator".
* Envelopes are accepted (Linux requires the report) but ignored.
