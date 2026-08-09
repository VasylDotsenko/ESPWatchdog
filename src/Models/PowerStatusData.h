#pragma once

#include <Arduino.h>

#include "Models/PowerData.h"

struct PowerStatusSummary
{
    PowerState state = PowerState::Disabled;

    bool available = false;
    bool restartInProgress = false;
    bool restartCompleted = false;
    bool lastOperationSucceeded = false;

    uint32_t activeRestartId = 0;
    uint32_t powerOffTime = 0;
};

struct PowerStatusStatistics
{
    uint32_t restartCount = 0;
    uint32_t errorCount = 0;

    uint64_t lastPowerOn = 0;
    uint64_t lastPowerOff = 0;
    uint64_t lastRestart = 0;
    uint64_t lastError = 0;
};

struct RestartHistoryStatus
{
    uint8_t capacity = RestartHistoryData::CAPACITY;
    uint8_t count = 0;
    uint8_t head = 0;

    uint32_t total = 0;
    uint32_t succeeded = 0;
    uint32_t failed = 0;

    uint64_t lastStartedAt = 0;
    uint64_t lastCompletedAt = 0;
    uint64_t lastFailedAt = 0;

    RestartHistoryEntry entries[RestartHistoryData::CAPACITY] {};
};

struct PowerStatusData
{
    PowerStatusSummary summary;
    PowerStatusStatistics statistics;
    RestartHistoryStatus history;
};
