#pragma once

#include <Arduino.h>

enum class WatchdogState : uint8_t
{
    Idle = 0,
    Monitoring,
    RestartRequired,
    Cooldown,
    LockedOut
};

struct WatchdogConfiguration
{
    bool enabled = true;
    uint8_t failureThreshold = 5;
    uint32_t bootDelay = 120000;
    uint32_t powerOffTime = 10000;
    uint8_t maxRestartPerDay = 20;
};

struct WatchdogRuntime
{
    uint32_t consecutiveFailures = 0;
    bool restartPending = false;
};

struct WatchdogStatistics
{
    uint32_t restartCount = 0;
    uint64_t lastSuccess = 0;
    uint64_t lastFailure = 0;
    uint64_t lastRestart = 0;
    uint64_t lockedOutAt = 0;
};

struct WatchdogData
{
    WatchdogState state = WatchdogState::Idle;
    WatchdogConfiguration configuration;
    WatchdogRuntime runtime;
    WatchdogStatistics statistics;
};
