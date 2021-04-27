#pragma once

#include <Arduino.h>

class AnalogInput {
public:
    explicit AnalogInput(int id)
    : m_id(id) {
        pinMode(id, INPUT);
    }

    int get() const {
        return analogRead(m_id);
    }

private:
    int m_id;
};

class AnalogOutput {
public:
    explicit AnalogOutput(int id)
    : m_id(id) {
        pinMode(m_id, OUTPUT);
    }

    void set(int value) const {
        analogWrite(m_id, value);
    }

private:
    int m_id;
};
