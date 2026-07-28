#pragma once

#include <Arduino.h>

enum class RelayState : uint8_t
{
    Disabled = 0,
    PowerOn,
    PowerOff
};

struct RelayConfiguration
{
    bool enabled = false;
    uint8_t pin = 5;
    bool activeHigh = true;
};

struct RelayRuntime
{
    bool restartInProgress = false;
    bool restartCompleted = false;
    uint32_t powerOffTime = 0;
};

struct RelayStatistics
{
    uint32_t restartCount = 0;
    uint64_t lastPowerOn = 0;
    uint64_t lastPowerOff = 0;
};

struct RelayData
{
    RelayState state = RelayState::Disabled;
    RelayConfiguration configuration;
    RelayRuntime runtime;
    RelayStatistics statistics;
};
