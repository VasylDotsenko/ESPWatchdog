#pragma once

#include <Arduino.h>

// Persistent ring buffer used only to enforce the rolling Watchdog quota.
// It is deliberately separate from the user-facing restart history, whose
// shorter capacity must not weaken the configured per-day safety limit.
struct WatchdogRestartQuotaData
{
    static constexpr uint8_t CAPACITY = 32;

    uint64_t completedAtEpoch[CAPACITY] {};

    uint8_t head = 0;
    uint8_t count = 0;
};
