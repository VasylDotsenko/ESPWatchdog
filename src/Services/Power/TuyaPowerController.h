#pragma once

#include "IPowerController.h"

class TuyaPowerController final : public IPowerController
{
public:
    bool begin() override;

    bool available() const override;

    bool powerOn() override;

    bool powerOff() override;

    const char* name() const override;
};

extern TuyaPowerController TuyaPower;
