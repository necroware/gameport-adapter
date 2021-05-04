#pragma once

#include <Arduino.h>

template <int ID>
class AnalogInput {
public:
    AnalogInput() {
        pinMode(ID, INPUT);
    }

    int get() const {
        return analogRead(ID);
    }
};

template <int ID>
class AnalogOutput {
public:
    AnalogOutput() {
        pinMode(ID, OUTPUT);
    }

    void set(int value) const {
        analogWrite(ID, value);
    }
};
