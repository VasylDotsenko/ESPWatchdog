#pragma once

#include <Arduino.h>

enum class RestartResult : uint8_t
{
    None = 0,
    InProgress,
    Success,
    Failed
};

enum class RestartReason : uint8_t
{
    Unknown = 0,
    WatchdogFailure,
    ManualCommand,
    ControllerUnavailable,
    PowerOffFailed,
    PowerOnFailed,
    PowerOnTimeout
};

struct RestartHistoryEntry
{
    uint32_t id = 0;

    RestartReason reason = RestartReason::Unknown;
    RestartResult result = RestartResult::None;

    uint64_t startedAt = 0;
    uint64_t completedAt = 0;

    uint64_t powerOffAt = 0;
    uint64_t powerOnAt = 0;

    uint32_t requestedPowerOffTime = 0;
    uint32_t actualDuration = 0;

    bool controllerAvailableAtStart = false;
};

struct RestartHistoryData
{
    static constexpr uint8_t CAPACITY = 10;

    RestartHistoryEntry entries[CAPACITY] {};

    uint8_t head = 0;
    uint8_t count = 0;

    uint32_t nextId = 1;

    uint32_t total = 0;
    uint32_t succeeded = 0;
    uint32_t failed = 0;

    uint64_t lastStartedAt = 0;
    uint64_t lastCompletedAt = 0;
    uint64_t lastFailedAt = 0;
};
