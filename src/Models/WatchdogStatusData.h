#pragma once

#include <Arduino.h>

#include "Models/WatchdogData.h"

struct WatchdogStatusSummary
{
    WatchdogState state = WatchdogState::Idle;

    bool enabled = false;
    bool restartPending = false;
    bool restartRequired = false;
    bool lockedOut = false;
    bool cooldown = false;

    uint32_t consecutiveFailures = 0;
};

struct WatchdogStatusConfiguration
{
    uint8_t failureThreshold = 0;
    uint32_t bootDelay = 0;
    uint32_t powerOffTime = 0;
    uint8_t maxRestartPerDay = 0;
};

struct WatchdogStatusStatistics
{
    uint32_t restartCount = 0;

    uint64_t lastSuccess = 0;
    uint64_t lastFailure = 0;
    uint64_t lastRestart = 0;
    uint64_t lockedOutAt = 0;
};

struct WatchdogStatusData
{
    WatchdogStatusSummary summary;
    WatchdogStatusConfiguration configuration;
    WatchdogStatusStatistics statistics;
};
