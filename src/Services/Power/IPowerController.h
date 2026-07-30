#pragma once

#include <Arduino.h>

class IPowerController
{
public:
    virtual ~IPowerController() = default;

    virtual bool begin() = 0;

    virtual bool available() const = 0;

    virtual bool powerOn() = 0;

    virtual bool powerOff() = 0;

    virtual const char* name() const = 0;
};
