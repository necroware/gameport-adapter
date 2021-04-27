#pragma once

class Joystick {
public:
    virtual void init() = 0;
    virtual void update() = 0;

    Joystick() = default;
    Joystick(const Joystick&) = delete;
    Joystick(Joystick&&) = delete;
    virtual ~Joystick() = default;   
    Joystick& operator=(const Joystick&) = delete;
    Joystick& operator=(Joystick&&) = delete;
};
