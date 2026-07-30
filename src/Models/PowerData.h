#pragma once

#include <Arduino.h>

enum class PowerState : uint8_t
{
    Disabled = 0,
    Idle,
    PowerOff,
    WaitingPowerOn,
    Error
};

struct PowerRuntime
{
    bool restartInProgress = false;
    bool restartCompleted = false;
    bool lastOperationSucceeded = false;
    uint32_t powerOffTime = 0;
};

struct PowerStatistics
{
    uint32_t restartCount = 0;
    uint32_t errorCount = 0;
    uint64_t lastPowerOn = 0;
    uint64_t lastPowerOff = 0;
    uint64_t lastRestart = 0;
    uint64_t lastError = 0;
};

struct PowerData
{
    PowerState state = PowerState::Disabled;
    PowerRuntime runtime;
    PowerStatistics statistics;
};
