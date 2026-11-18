#!/usr/bin/env python3
"""End-to-end force feedback test with a real Sidewinder Force Feedback Wheel.

Needs Linux, python-evdev, the adapter (with the wheel attached) plugged in
and read/write access to its /dev/input/event* device. Run:

    python3 firmware/gameport-adapter/test/ffb_e2e.py

The script checks that the kernel accepted the force feedback descriptor and
plays effects, while it watches the wheel's X axis. Keep your hands off the
wheel while it runs.
"""
import time

import evdev
from evdev import ecodes, ff

devices = [evdev.InputDevice(path) for path in evdev.list_devices()]
wheel = next((d for d in devices if ecodes.EV_FF in d.capabilities()), None)
assert wheel, "no force feedback device found, check dmesg for hid-pidff errors"
effects = wheel.capabilities()[ecodes.EV_FF]
print(f"{wheel.name} ({wheel.path}), max effects: {wheel.ff_effects_count}")
for effect_type in ("FF_CONSTANT", "FF_PERIODIC", "FF_RAMP", "FF_SPRING", "FF_DAMPER", "FF_INERTIA", "FF_FRICTION",
                    "FF_GAIN"):
    assert getattr(ecodes, effect_type) in effects, f"{effect_type} missing in {effects}"

CENTER = 512
RIGHT, LEFT = 0x4000, 0xC000


def x():
    return wheel.absinfo(ecodes.ABS_X).value


def watch(seconds):
    """Returns the X axis positions during the given time."""
    positions = []
    end = time.time() + seconds
    while time.time() < end:
        positions.append(x())
        time.sleep(0.01)
    return positions


def constant(direction):
    force = ff.Constant(0x5000, ff.Envelope(0, 0, 0, 0))
    return ff.Effect(ecodes.FF_CONSTANT, -1, direction, ff.Trigger(0, 0), ff.Replay(0, 0),
                     ff.EffectType(ff_constant_effect=force))


def periodic(waveform, period_ms, length_ms):
    force = ff.Periodic(waveform, period_ms, 0x5000, 0, 0, ff.Envelope(0, 0, 0, 0))
    return ff.Effect(ecodes.FF_PERIODIC, -1, RIGHT, ff.Trigger(0, 0), ff.Replay(length_ms, 0),
                     ff.EffectType(ff_periodic_effect=force))


def ramp(start, end, length_ms):
    force = ff.Ramp(start, end, ff.Envelope(0, 0, 0, 0))
    return ff.Effect(ecodes.FF_RAMP, -1, RIGHT, ff.Trigger(0, 0), ff.Replay(length_ms, 0),
                     ff.EffectType(ff_ramp_effect=force))


def condition(effect_type, coefficient):
    conditions = (ff.Condition * 2)()
    conditions[0] = ff.Condition(0xFFFF, 0xFFFF, coefficient, coefficient, 0, 0)
    return ff.Effect(effect_type, -1, RIGHT, ff.Trigger(0, 0), ff.Replay(0, 0),
                     ff.EffectType(ff_condition_effect=conditions))


def upload(effect):
    return wheel.upload_effect(effect)


def play(effect_id, on=True):
    wheel.write(ecodes.EV_FF, effect_id, int(on))


def recenter():
    """Without effects the wheel's own auto-centering is on again."""
    time.sleep(2)
    assert abs(x() - CENTER) < 100, f"wheel did not re-center, X = {x()}"


def test_constant_force():
    for name, direction, sign in (("right", RIGHT, 1), ("left", LEFT, -1)):
        start = x()
        effect_id = upload(constant(direction))
        play(effect_id)
        positions = watch(2)
        wheel.erase_effect(effect_id)
        deflection = max(positions) - start if sign > 0 else min(positions) - start
        print(f"Constant force {name}: X moved by {deflection}")
        assert sign * deflection >= 200, f"wheel did not turn {name}"
        recenter()


def test_spring():
    spring = upload(condition(ecodes.FF_SPRING, 0x7FFF))  # holds auto-centering off
    push = upload(constant(RIGHT))
    play(push)
    watch(1.5)
    wheel.erase_effect(push)
    released = x()
    play(spring)
    positions = watch(2)
    wheel.erase_effect(spring)
    print(f"Spring: X {released} -> {positions[-1]}")
    assert released - CENTER > 200, "constant force did not turn the wheel"
    assert abs(positions[-1] - CENTER) < 100, "spring did not pull the wheel back to the center"
    recenter()


def test_ramp():
    """A ramp from left to right turns the wheel left first, then back to the
    right (it does not reach the right end before the ramp ends)."""
    effect_id = upload(ramp(-0x6000, 0x6000, 2000))
    play(effect_id)
    first, second = watch(1), watch(1)
    wheel.erase_effect(effect_id)
    print(f"Ramp: X down to {min(first)}, then up to {max(second)}")
    assert min(first) < CENTER - 200, "ramp did not start on the left"
    assert max(second) > min(first) + 300, "ramp did not turn the wheel back to the right"
    recenter()


def test_periodic():
    """A slow sine swings the wheel back and forth. The wheel's speed follows
    the force, so it swings between its start and one side, not around the
    center."""
    effect_id = upload(periodic(ecodes.FF_SINE, 1000, 3000))
    play(effect_id)
    positions = watch(3)
    wheel.erase_effect(effect_id)
    reversals, extreme, rising = 0, positions[0], True
    for position in positions:
        if (position - extreme) * (1 if rising else -1) > 0:
            extreme = position
        elif abs(position - extreme) > 100:
            reversals, extreme, rising = reversals + 1, position, not rising
    print(f"Sine: X {min(positions)}..{max(positions)}, {reversals} reversals")
    assert reversals >= 4, "sine did not swing the wheel"
    recenter()


def time_to_turn(effect_type=None):
    """Time in seconds for a constant force to turn the wheel to the right,
    while an optional condition effect with full coefficients plays."""
    holder = upload(constant(RIGHT))  # same number of effects in both runs
    condition_id = upload(condition(effect_type, 0x7FFF)) if effect_type else None
    if condition_id is not None:
        play(condition_id)
    wheel.erase_effect(holder)
    push = upload(constant(RIGHT))
    start = time.time()
    play(push)
    while x() < CENTER + 300 and time.time() - start < 3:
        time.sleep(0.005)
    duration = time.time() - start
    wheel.erase_effect(push)
    if condition_id is not None:
        wheel.erase_effect(condition_id)
    recenter()
    return duration


def test_conditions_slow_down_the_wheel():
    """Damper, inertia and friction are played by the wheel itself."""
    free = time_to_turn()
    for name in ("FF_DAMPER", "FF_INERTIA", "FF_FRICTION"):
        slowed = time_to_turn(getattr(ecodes, name))
        print(f"{name}: turning took {free:.2f} s free, {slowed:.2f} s with it")
        assert slowed > 1.3 * free, f"{name} did not slow down the wheel"


wheel.write(ecodes.EV_FF, ecodes.FF_GAIN, 0xFFFF)
test_constant_force()
test_spring()
test_ramp()
test_periodic()
test_conditions_slow_down_the_wheel()
print("OK")
