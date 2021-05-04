#pragma once

class Driver {
public:
    virtual void init() = 0;
    virtual void update() = 0;

    Driver() = default;
    Driver(const Driver&) = delete;
    Driver(Driver&&) = delete;
    virtual ~Driver() = default;   
    Driver& operator=(const Driver&) = delete;
    Driver& operator=(Driver&&) = delete;
};

