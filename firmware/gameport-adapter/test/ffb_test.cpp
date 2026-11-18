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

// Host unit tests for the force feedback logic.
//
// Build and run from the repository root:
//   g++ -std=c++11 -Ifirmware/gameport-adapter firmware/gameport-adapter/test/ffb_test.cpp -o ffb_test

#include "ForceFeedback.h"
#include "SidewinderWheelFfb.h"

#include <cassert>
#include <cstdio>
#include <cstring>

using FFB = ForceFeedback;

// ---------------------------------------------------------------------------
// USB HID PID side
// ---------------------------------------------------------------------------

static uint8_t createEffect(FFB &ffb) {
  const uint8_t create[] = {FFB::CREATE_NEW_EFFECT, 1, 0, 0};
  ffb.setReport(create, sizeof(create), 0);
  uint8_t load[8]{};
  assert(ffb.getReport(FFB::BLOCK_LOAD, load) == 5);
  assert(load[0] == FFB::BLOCK_LOAD);
  return load[2] == 1 ? load[1] : 0;
}

static void setEffect(FFB &ffb, uint8_t block, uint16_t duration, uint8_t gain, bool dirEnable, uint8_t direction,
                      uint8_t type = FFB::CONSTANT_FORCE) {
  const uint8_t report[] = {FFB::SET_EFFECT,
                            block,
                            type,
                            uint8_t(duration),
                            uint8_t(duration >> 8),
                            0,
                            0,
                            gain,
                            0xff,
                            uint8_t(dirEnable ? 0b11 : 0b01),
                            direction};
  ffb.setReport(report, sizeof(report), 0);
}

static void setMagnitude(FFB &ffb, uint8_t block, int16_t magnitude) {
  const uint8_t report[] = {FFB::SET_CONSTANT_FORCE, block, uint8_t(magnitude), uint8_t(uint16_t(magnitude) >> 8)};
  ffb.setReport(report, sizeof(report), 0);
}

static void setCondition(FFB &ffb, uint8_t block, int8_t offset, int8_t positive, int8_t negative, uint8_t saturation,
                         uint8_t deadBand) {
  const uint8_t report[] = {FFB::SET_CONDITION, block,      uint8_t(offset), uint8_t(positive),
                            uint8_t(negative),  saturation, deadBand};
  ffb.setReport(report, sizeof(report), 0);
}

static void setPeriodic(FFB &ffb, uint8_t block, int16_t magnitude, int16_t offset, uint8_t phase, uint16_t period) {
  const uint8_t report[] = {FFB::SET_PERIODIC,
                            block,
                            uint8_t(magnitude),
                            uint8_t(uint16_t(magnitude) >> 8),
                            uint8_t(offset),
                            uint8_t(uint16_t(offset) >> 8),
                            phase,
                            uint8_t(period),
                            uint8_t(period >> 8)};
  ffb.setReport(report, sizeof(report), 0);
}

static void setEnvelope(FFB &ffb, uint8_t block, uint8_t attackLevel, uint8_t fadeLevel, uint16_t attackTime,
                        uint16_t fadeTime) {
  const uint8_t report[] = {FFB::SET_ENVELOPE,   block,
                            attackLevel,         fadeLevel,
                            uint8_t(attackTime), uint8_t(attackTime >> 8),
                            uint8_t(fadeTime),   uint8_t(fadeTime >> 8)};
  ffb.setReport(report, sizeof(report), 0);
}

static void setRamp(FFB &ffb, uint8_t block, int16_t start, int16_t end) {
  const uint8_t report[] = {FFB::SET_RAMP_FORCE,           block,        uint8_t(start),
                            uint8_t(uint16_t(start) >> 8), uint8_t(end), uint8_t(uint16_t(end) >> 8)};
  ffb.setReport(report, sizeof(report), 0);
}

static void operate(FFB &ffb, uint8_t block, uint8_t op, uint8_t loops, uint32_t now) {
  const uint8_t report[] = {FFB::EFFECT_OPERATION, block, op, loops};
  ffb.setReport(report, sizeof(report), now);
}

static void control(FFB &ffb, uint8_t command) {
  const uint8_t report[] = {FFB::DEVICE_CONTROL, command};
  ffb.setReport(report, sizeof(report), 0);
}

/// Creates and starts an infinite constant force pushing to the right (90°).
static uint8_t playConstant(FFB &ffb, int16_t magnitude) {
  const auto block = createEffect(ffb);
  setEffect(ffb, block, 0xffff, 255, true, 64);
  setMagnitude(ffb, block, magnitude);
  operate(ffb, block, 1, 1, 0);
  return block;
}

/// Creates and starts an infinite spring or damper.
static uint8_t playCondition(FFB &ffb, uint8_t type, int8_t offset, int8_t positive, int8_t negative,
                             uint8_t saturation, uint8_t deadBand) {
  const auto block = createEffect(ffb);
  setEffect(ffb, block, 0xffff, 255, true, 64, type);
  setCondition(ffb, block, offset, positive, negative, saturation, deadBand);
  operate(ffb, block, 1, 1, 0);
  return block;
}

/// Creates and starts an infinite periodic effect pushing to the right (90°).
static uint8_t playPeriodic(FFB &ffb, uint8_t type, int16_t magnitude, int16_t offset, uint8_t phase, uint16_t period) {
  const auto block = createEffect(ffb);
  setEffect(ffb, block, 0xffff, 255, true, 64, type);
  setPeriodic(ffb, block, magnitude, offset, phase, period);
  operate(ffb, block, 1, 1, 0);
  return block;
}

static void testPool() {
  FFB ffb;
  uint8_t pool[8]{};
  assert(ffb.getReport(FFB::POOL, pool) == 5);
  assert(pool[0] == FFB::POOL);
  assert(pool[3] == FFB::MAX_EFFECTS);
  assert(pool[4] == 1); // device managed pool
  assert(ffb.getReport(0x42, pool) == 0);
}

static void testAllocation() {
  FFB ffb;
  for (uint8_t i = 1; i <= FFB::MAX_EFFECTS; i++) {
    assert(createEffect(ffb) == i);
  }
  assert(createEffect(ffb) == 0); // pool full

  const uint8_t freeBlock[] = {FFB::BLOCK_FREE, 3};
  ffb.setReport(freeBlock, sizeof(freeBlock), 0);
  assert(createEffect(ffb) == 3);
}

static void testDirection() {
  FFB ffb;
  const auto block = playConstant(ffb, 5000);
  assert(ffb.force(10, 0) == 5000);

  setEffect(ffb, block, 0xffff, 255, true, 192); // 270°, pushing left
  assert(ffb.force(10, 0) == -5000);

  // Wine (Proton with hidraw) enables the direction, but leaves it at 0, if
  // the effect has no axes, and puts left/right into the magnitude's sign.
  setEffect(ffb, block, 0xffff, 255, true, 0);
  assert(ffb.force(10, 0) == 5000);
  setMagnitude(ffb, block, -5000);
  assert(ffb.force(10, 0) == -5000);
  setMagnitude(ffb, block, 5000);

  setEffect(ffb, block, 0xffff, 255, true, 32); // 45°
  assert(ffb.force(10, 0) == 3535);

  setEffect(ffb, block, 0xffff, 255, false, 0); // direction disabled: signed magnitude
  assert(ffb.force(10, 0) == 5000);
  setMagnitude(ffb, block, -3000);
  assert(ffb.force(10, 0) == -3000);
}

static void testGain() {
  FFB ffb;
  const auto block = playConstant(ffb, 10000);
  setEffect(ffb, block, 0xffff, 51, true, 64);
  assert(ffb.force(10, 0) == 2000);

  const uint8_t gain[] = {FFB::DEVICE_GAIN, 128};
  ffb.setReport(gain, sizeof(gain), 0);
  assert(ffb.force(10, 0) == 1003);
}

static void testSumIsClamped() {
  FFB ffb;
  playConstant(ffb, 8000);
  playConstant(ffb, 8000);
  assert(ffb.force(10, 0) == 10000);
}

static void testDuration() {
  FFB ffb;
  const auto block = createEffect(ffb);
  setEffect(ffb, block, 100, 255, true, 64);
  setMagnitude(ffb, block, 4000);
  operate(ffb, block, 1, 2, 1000); // two loops of 100ms each, started at 1000ms
  assert(ffb.force(1199, 0) == 4000);
  assert(ffb.force(1200, 0) == 0);
  assert(ffb.active()); // still allocated by the host

  operate(ffb, block, 1, 255, 5000); // infinite loops
  assert(ffb.force(100000, 0) == 4000);
}

static void testStopAndFree() {
  FFB ffb;
  assert(!ffb.active());
  const auto block = playConstant(ffb, 5000);
  assert(ffb.active());

  operate(ffb, block, 3, 0, 0); // stop
  assert(ffb.force(10, 0) == 0);
  assert(ffb.active());

  operate(ffb, block, 1, 1, 0);
  assert(ffb.force(10, 0) == 5000);

  const uint8_t freeBlock[] = {FFB::BLOCK_FREE, block};
  ffb.setReport(freeBlock, sizeof(freeBlock), 0);
  assert(ffb.force(10, 0) == 0);
  assert(!ffb.active());
}

static void testStartSolo() {
  FFB ffb;
  playConstant(ffb, 3000);
  const auto block = playConstant(ffb, 2000);
  assert(ffb.force(10, 0) == 5000);
  operate(ffb, block, 2, 1, 0); // start solo
  assert(ffb.force(10, 0) == 2000);
}

static void testDeviceControl() {
  FFB ffb;
  playConstant(ffb, 5000);

  control(ffb, 2); // disable actuators
  assert(ffb.force(10, 0) == 0);
  control(ffb, 1); // enable actuators
  assert(ffb.force(10, 0) == 5000);

  control(ffb, 5); // pause
  assert(ffb.force(10, 0) == 0);
  control(ffb, 6); // continue
  assert(ffb.force(10, 0) == 5000);

  control(ffb, 3); // stop all
  assert(ffb.force(10, 0) == 0);
  assert(ffb.active());

  control(ffb, 4); // reset
  assert(!ffb.active());
  assert(createEffect(ffb) == 1);
}

static void testInvalidReportsAreIgnored() {
  FFB ffb;
  const uint8_t badBlock[] = {FFB::SET_CONSTANT_FORCE, FFB::MAX_EFFECTS + 1, 0x10, 0x27};
  ffb.setReport(badBlock, sizeof(badBlock), 0);
  const uint8_t truncated[] = {FFB::SET_EFFECT, 1};
  ffb.setReport(truncated, sizeof(truncated), 0);
  ffb.setReport(nullptr, 0, 0);
  assert(ffb.force(10, 0) == 0);
  assert(!ffb.active());
}

static void testSpringPullsTowardsCenter() {
  FFB ffb;
  playCondition(ffb, FFB::SPRING, 0, 127, 127, 255, 0);
  assert(ffb.force(10, 0) == 0);
  assert(ffb.force(10, 5000) == -5000);
  assert(ffb.force(10, -5000) == 5000);
}

static void testSpringOffsetAndCoefficients() {
  FFB ffb;
  const auto block = playCondition(ffb, FFB::SPRING, -127, 127, 127, 255, 0);
  assert(ffb.force(10, 0) == -10000); // center at the left end

  setCondition(ffb, block, 0, 127, 0, 255, 0); // no force on the left side
  assert(ffb.force(10, -5000) == 0);
  assert(ffb.force(10, 5000) == -5000);

  setCondition(ffb, block, 0, -127, -127, 255, 0); // pushes away from the center
  assert(ffb.force(10, 5000) == 5000);
}

static void testSpringDeadBandAndSaturation() {
  FFB ffb;
  playCondition(ffb, FFB::SPRING, 0, 127, 127, 51, 51); // 20% saturation and dead band
  assert(ffb.force(10, 1500) == 0);
  assert(ffb.force(10, -1500) == 0);
  assert(ffb.force(10, 3000) == -1000);
  assert(ffb.force(10, 9000) == -2000);
  assert(ffb.force(10, -9000) == 2000);
}

static void testDamperCoefficients() {
  FFB ffb;
  const auto block = playCondition(ffb, FFB::DAMPER, 0, 127, 64, 255, 0);
  assert(ffb.force(10, 5000) == 0); // the wheel plays dampers itself
  assert(ffb.coefficients(FFB::DAMPER, 10).positive == 10000);
  assert(ffb.coefficients(FFB::DAMPER, 10).negative == 5039);

  setEffect(ffb, block, 0xffff, 51, true, 64, FFB::DAMPER); // 20% effect gain
  assert(ffb.coefficients(FFB::DAMPER, 10).positive == 2000);

  playCondition(ffb, FFB::DAMPER, 0, 127, 127, 255, 0);        // dampers add up
  assert(ffb.coefficients(FFB::DAMPER, 10).positive == 10000); // clamped

  control(ffb, 3); // stop all
  assert(ffb.coefficients(FFB::DAMPER, 10).positive == 0);
}

static void testFrictionAndInertiaCoefficients() {
  FFB ffb;
  playCondition(ffb, FFB::FRICTION, 0, 64, 64, 255, 0);
  playCondition(ffb, FFB::INERTIA, 0, -127, 127, 255, 0);
  assert(ffb.force(10, 5000) == 0); // the wheel plays them itself
  assert(ffb.coefficients(FFB::FRICTION, 10).positive == 5039);
  assert(ffb.coefficients(FFB::INERTIA, 10).positive == -10000);
  assert(ffb.coefficients(FFB::INERTIA, 10).negative == 10000);
  assert(ffb.coefficients(FFB::DAMPER, 10).positive == 0);
}

static void testPeriodicWaveforms() {
  // Forces at 0, 1/4, 1/2 and 3/4 of the period
  const struct {
    uint8_t type;
    int16_t forces[4];
  } waveforms[] = {
      {FFB::SINE, {0, 10000, 0, -10000}},
      {FFB::SQUARE, {10000, 10000, -10000, -10000}},
      {FFB::TRIANGLE, {0, 10000, 0, -10000}},
      {FFB::SAWTOOTH_UP, {-10000, -5000, 0, 5000}},
      {FFB::SAWTOOTH_DOWN, {10000, 5000, 0, -5000}},
  };
  for (const auto &waveform : waveforms) {
    FFB ffb;
    playPeriodic(ffb, waveform.type, 10000, 0, 0, 1000);
    for (auto i = 0u; i < 4; i++) {
      assert(ffb.force(1000 + i * 250, 0) == waveform.forces[i]);
    }
  }
}

static void testPeriodicOffsetPhaseAndDirection() {
  FFB ffb;
  const auto block = playPeriodic(ffb, FFB::SINE, 5000, 2000, 0, 1000);
  assert(ffb.force(250, 0) == 7000);
  assert(ffb.force(750, 0) == -3000);

  setPeriodic(ffb, block, 5000, 0, 64, 1000); // starts at the top
  assert(ffb.force(0, 0) == 5000);

  setEffect(ffb, block, 0xffff, 255, true, 192, FFB::SINE); // 270°, pushing left
  assert(ffb.force(0, 0) == -5000);

  setPeriodic(ffb, block, 5000, 0, 64, 0); // no period: stays at the phase
  assert(ffb.force(123, 0) == -5000);
}

static void testRamp() {
  FFB ffb;
  const auto block = createEffect(ffb);
  setEffect(ffb, block, 1000, 255, true, 64, FFB::RAMP);
  setRamp(ffb, block, -10000, 10000);
  operate(ffb, block, 1, 1, 0);
  assert(ffb.force(0, 0) == -10000);
  assert(ffb.force(250, 0) == -5000);
  assert(ffb.force(500, 0) == 0);
  assert(ffb.force(1000, 0) == 0); // finished

  setEffect(ffb, block, 0xffff, 255, true, 64, FFB::RAMP); // infinite: start level
  operate(ffb, block, 1, 1, 0);
  assert(ffb.force(500, 0) == -10000);
}

static void testEnvelope() {
  FFB ffb;
  const auto block = createEffect(ffb);
  setEffect(ffb, block, 1000, 255, true, 64);
  setMagnitude(ffb, block, -8000);
  setEnvelope(ffb, block, 51, 0, 200, 400); // from 2000 in 200 ms, to 0 in the last 400 ms
  operate(ffb, block, 1, 1, 0);
  assert(ffb.force(0, 0) == -2000);
  assert(ffb.force(100, 0) == -5000);
  assert(ffb.force(200, 0) == -8000);
  assert(ffb.force(600, 0) == -8000);
  assert(ffb.force(800, 0) == -4000);
}

static void testPeriodicEnvelopeScalesTheMagnitude() {
  FFB ffb;
  const auto block = createEffect(ffb);
  setEffect(ffb, block, 2000, 255, true, 64, FFB::SINE);
  setPeriodic(ffb, block, 10000, 1000, 0, 1000);
  setEnvelope(ffb, block, 0, 0, 500, 0);
  operate(ffb, block, 1, 1, 0);
  assert(ffb.force(250, 0) == 6000); // offset 1000 + half of the magnitude
}

// ---------------------------------------------------------------------------
// Wheel MIDI side
// ---------------------------------------------------------------------------

struct Recorder {
  uint8_t bytes[256]{};
  uint16_t size{};

  void write(const uint8_t *data, uint8_t count) {
    memcpy(bytes + size, data, count);
    size += count;
  }

  template <size_t N>
  bool equals(const uint8_t (&expected)[N]) const {
    if (size != N || memcmp(bytes, expected, N) != 0) {
      for (auto i = 0u; i < size; i++) {
        printf("%02x ", bytes[i]);
      }
      printf("\n");
      return false;
    }
    return true;
  }

  void clear() {
    size = 0;
  }
};

using Wheel = SidewinderWheelFfb<Recorder>;

// Expected bytes in the following tests were captured from Microsoft Force
// Editor, see https://github.com/tloimu/adapt-ffb-joy/blob/wiki/SidewinderFFBWheel.md

static void testModifyChecksumMatchesCapture() {
  Wheel wheel;
  wheel.sendModify(1, 3, 0x7d << 7);
  wheel.sendModify(1, 5, 0x2f << 7 | 0x3e);
  wheel.sendModify(1, 6, 0x7d);
  wheel.sendModify(0, 0, 0x7f);
  const uint8_t captured[] = {
      0xf1, 0x0e, 0x43, 0x01, 0x00, 0x7d, // init
      0xf1, 0x1c, 0x45, 0x01, 0x3e, 0x2f, // init
      0xf1, 0x0b, 0x46, 0x01, 0x7d, 0x00, // init
      0xf1, 0x10, 0x40, 0x00, 0x7f, 0x00, // auto center off
  };
  assert(wheel.midi.equals(captured));
}

static void testSysExChecksumMatchesCapture() {
  Wheel wheel;
  const uint8_t spring[] = {0x20, 0x08, 0x7f, 0x6e, 0x1e, 0x00, 0x00, 0x7d, 0x3e, 0x4e, 0x3e, 0x2f, 0x7d, 0x00};
  wheel.sendEffect(spring, sizeof(spring));
  const uint8_t capturedSpring[] = {0xf0, 0x00, 0x01, 0x0a, 0x15, 0x20, 0x08, 0x7f, 0x6e, 0x1e, 0x00,
                                    0x00, 0x7d, 0x3e, 0x4e, 0x3e, 0x2f, 0x7d, 0x00, 0x5a, 0xf7};
  assert(wheel.midi.equals(capturedSpring));

  wheel.midi.clear();
  const uint8_t damper[] = {0x20, 0x09, 0x7f, 0x6e, 0x1e, 0x00, 0x00, 0x7d, 0x3e, 0x3f, 0x3e, 0x3f, 0x7d, 0x00};
  wheel.sendEffect(damper, sizeof(damper));
  const uint8_t capturedDamper[] = {0xf0, 0x00, 0x01, 0x0a, 0x15, 0x20, 0x09, 0x7f, 0x6e, 0x1e, 0x00,
                                    0x00, 0x7d, 0x3e, 0x3f, 0x3e, 0x3f, 0x7d, 0x00, 0x58, 0xf7};
  assert(wheel.midi.equals(capturedDamper));
}

static void testActivationUploadsConstantForceAndDamper() {
  Wheel wheel;
  wheel.update(false, 0);
  assert(wheel.midi.size == 0);

  wheel.update(true, 10000);
  const uint8_t expected[] = {
      0xf3, 0x1d, 0xf1, 0x10, 0x40, 0x00, 0x7f, 0x00, 0xf3, 0x6a, // auto center off (captured)
      0xf0, 0x00, 0x01, 0x0a, 0x15,                               // SysEx header
      0x20, 0x06, 0x7f, 0x00, 0x00,                               // constant force, infinite duration
      0x00, 0x7f, 0x7f, 0x00, 0x00,                               // direction, ?, envelope y1, x1
      0x7f, 0x00, 0x00, 0x7f, 0x7d,                               // force, envelope x2, y2, clockwise
      0x62, 0xf7,                                                 // checksum, end of SysEx
      0xf2, 0x2d, 0x02,                                           // play effect 2
      0xf0, 0x00, 0x01, 0x0a, 0x15,                               // SysEx header
      0x20, 0x09, 0x7f, 0x00, 0x00, 0x00,                         // damper, infinite duration, direction
      0x00, 0x3e, 0x3e, 0x3f, 0x3e, 0x3f, 0x7d, 0x3e,             // no damping (coefficients 0x3e)
      0x65, 0xf7,                                                 // checksum, end of SysEx
      0xf2, 0x2c, 0x03,                                           // play effect 3
      0xf0, 0x00, 0x01, 0x0a, 0x15,                               // SysEx header
      0x20, 0x0a, 0x7f, 0x00, 0x00, 0x00,                         // inertia, infinite duration, direction
      0x00, 0x3e, 0x3e, 0x3f, 0x3e, 0x3f, 0x7d, 0x3e,             // no inertia (coefficients 0x3e)
      0x64, 0xf7,                                                 // checksum, end of SysEx
      0xf2, 0x2b, 0x04,                                           // play effect 4
      0xf0, 0x00, 0x01, 0x0a, 0x15,                               // SysEx header
      0x20, 0x0b, 0x7f, 0x00, 0x00, 0x00,                         // friction, infinite duration, direction
      0x3f,                                                       // no friction
      0x17, 0xf7,                                                 // checksum, end of SysEx
      0xf2, 0x2a, 0x05,                                           // play effect 5
  };
  assert(wheel.midi.equals(expected));
}

static void testDamperChangesReuploadTheDamper() {
  Wheel wheel;
  wheel.update(true, 0);
  wheel.midi.clear();

  wheel.update(true, 0, {10000, 10000});
  const uint8_t full[] = {
      0xf2, 0x1f, 0x03,                                           // delete effect 3
      0xf0, 0x00, 0x01, 0x0a, 0x15, 0x20, 0x09, 0x7f, 0x00, 0x00, // damper header
      0x00, 0x00, 0x7d, 0x3e, 0x3f, 0x3e, 0x3f, 0x7d, 0x00,       // full damping both ways
      0x64, 0xf7,                                                 //
      0xf2, 0x2c, 0x03,                                           // play effect 3
  };
  assert(wheel.midi.equals(full));

  wheel.midi.clear();
  wheel.update(true, 0, {10000, 10000}); // unchanged
  assert(wheel.midi.size == 0);

  wheel.update(true, 0, {5000, -10000});
  const uint8_t half[] = {
      0xf2, 0x1f, 0x03,                                           // delete effect 3
      0xf0, 0x00, 0x01, 0x0a, 0x15, 0x20, 0x09, 0x7f, 0x00, 0x00, // damper header
      0x00, 0x00, 0x00, 0x3e, 0x3f, 0x3e, 0x3f, 0x7d, 0x1f,       // half right, pushes left
      0x42, 0xf7,                                                 //
      0xf2, 0x2c, 0x03,                                           // play effect 3
  };
  assert(wheel.midi.equals(half));
}

static void testForceChangesAreSentAsModify() {
  Wheel wheel;
  wheel.update(true, 0);
  wheel.midi.clear();

  wheel.update(true, 5000);
  const uint8_t magnitude[] = {0xf1, 0x48, 0x46, 0x02, 0x3f, 0x00};
  assert(wheel.midi.equals(magnitude));

  wheel.midi.clear();
  wheel.update(true, 5010); // same 7 bit magnitude
  assert(wheel.midi.size == 0);

  wheel.update(true, -5000);
  // The wheel ignores zero values with the 0x40 flag (verified on hardware)
  const uint8_t direction[] = {0xf1, 0x04, 0x09, 0x02, 0x00, 0x00}; // counter clockwise
  assert(wheel.midi.equals(direction));

  wheel.midi.clear();
  wheel.update(true, -10000);
  const uint8_t magnitudeOnly[] = {0xf1, 0x08, 0x46, 0x02, 0x7f, 0x00};
  assert(wheel.midi.equals(magnitudeOnly));

  wheel.midi.clear();
  wheel.update(true, -10);
  const uint8_t zeroMagnitude[] = {0xf1, 0x07, 0x06, 0x02, 0x00, 0x00};
  assert(wheel.midi.equals(zeroMagnitude));
}

static void testInertiaAndFrictionChangesReuploadThem() {
  Wheel wheel;
  wheel.update(true, 0);
  wheel.midi.clear();

  wheel.update(true, 0, {}, {10000, 10000});
  const uint8_t inertia[] = {
      0xf2, 0x18, 0x04,                                           // delete effect 4
      0xf0, 0x00, 0x01, 0x0a, 0x15, 0x20, 0x0a, 0x7f, 0x00, 0x00, // inertia header
      0x00, 0x00, 0x7d, 0x3e, 0x3f, 0x3e, 0x3f, 0x7d, 0x00,       // full inertia both ways
      0x63, 0xf7,                                                 //
      0xf2, 0x2b, 0x04,                                           // play effect 4
  };
  assert(wheel.midi.equals(inertia));

  wheel.midi.clear();
  wheel.update(true, 0, {}, {10000, 10000}, {10000, 10000});
  const uint8_t friction[] = {
      0xf2, 0x19, 0x05,                                           // delete effect 5
      0xf0, 0x00, 0x01, 0x0a, 0x15, 0x20, 0x0b, 0x7f, 0x00, 0x00, // friction header
      0x00, 0x7e,                                                 // full friction
      0x58, 0xf7,                                                 //
      0xf2, 0x2a, 0x05,                                           // play effect 5
  };
  assert(wheel.midi.equals(friction));
}

static void testDeactivationRestoresAutoCenter() {
  Wheel wheel;
  wheel.update(true, 0);
  wheel.midi.clear();
  wheel.update(false, 0);
  const uint8_t expected[] = {
      0xf2, 0x1e, 0x02, 0xf2, 0x1f, 0x03, 0xf2, 0x18, 0x04, 0xf2, 0x19, 0x05, // delete effects 2-5
      0xf3, 0x1d,                                                             // auto center on
  };
  assert(wheel.midi.equals(expected));
}

static void testInitRemovesLeftoverEffects() {
  // After an adapter reset, the wheel may still hold effects 2-5
  Wheel wheel;
  wheel.init();
  const uint8_t deletes[] = {0xf2, 0x1e, 0x02, 0xf2, 0x1f, 0x03, 0xf2, 0x18, 0x04, 0xf2, 0x19, 0x05};
  assert(wheel.midi.size > sizeof(deletes));
  assert(memcmp(wheel.midi.bytes + wheel.midi.size - sizeof(deletes), deletes, sizeof(deletes)) == 0);
}

int main() {
  testPool();
  testAllocation();
  testDirection();
  testGain();
  testSumIsClamped();
  testDuration();
  testStopAndFree();
  testStartSolo();
  testDeviceControl();
  testInvalidReportsAreIgnored();
  testSpringPullsTowardsCenter();
  testSpringOffsetAndCoefficients();
  testSpringDeadBandAndSaturation();
  testDamperCoefficients();
  testFrictionAndInertiaCoefficients();
  testPeriodicWaveforms();
  testPeriodicOffsetPhaseAndDirection();
  testRamp();
  testEnvelope();
  testPeriodicEnvelopeScalesTheMagnitude();
  testModifyChecksumMatchesCapture();
  testSysExChecksumMatchesCapture();
  testActivationUploadsConstantForceAndDamper();
  testDamperChangesReuploadTheDamper();
  testInertiaAndFrictionChangesReuploadThem();
  testInitRemovesLeftoverEffects();
  testForceChangesAreSentAsModify();
  testDeactivationRestoresAutoCenter();
  printf("all force feedback tests passed\n");
  return 0;
}
