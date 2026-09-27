#pragma once

#include <Arduino.h>

struct RuntimeGuardStatus
{
    bool enabled = true;
    bool degraded = false;
    bool restartScheduled = false;
    bool startupBaselineReady = false;

    uint32_t freeHeap = 0;
    uint32_t heapFragmentation = 0;

    uint32_t heapAtBoot = 0;
    uint32_t minFreeHeapSeen = 0;
    uint32_t heapDropFromBoot = 0;
    uint8_t maxHeapFragmentationSeen = 0;

    uint32_t minFreeHeap = 0;
    uint8_t maxHeapFragmentation = 0;

    uint32_t lastCheckAt = 0;
    uint32_t degradedSince = 0;
    uint32_t restartAt = 0;
};

class RuntimeGuardService
{
public:
    bool begin();

    // Arms deferred startup baseline capture. Call once after all application
    // services have started; the sample is taken after stable WiFi settles.
    void captureStartupBaseline();

    void loop();

    RuntimeGuardStatus status() const;

private:
    static constexpr uint32_t CHECK_INTERVAL_MS = 60000;
    static constexpr uint32_t STARTUP_BASELINE_SETTLE_MS = 15000;
    static constexpr uint32_t MIN_UPTIME_BEFORE_RECOVERY_MS = 3600000;
    static constexpr uint32_t DEGRADED_DURATION_MS = 600000;
    static constexpr uint32_t RESTART_DELAY_MS = 3000;
    static constexpr uint32_t ESP_RESTART_SETTLE_MS = 50;

    static constexpr uint32_t MIN_FREE_HEAP_BYTES = 8000;
    static constexpr uint8_t MAX_HEAP_FRAGMENTATION = 60;

private:
    void check();

    void updateStartupBaseline(
        uint32_t now);

    void updateObservedMetrics(
        uint32_t freeHeap,
        uint8_t heapFragmentation);

    void scheduleRestart(
        const char* reason);

    void resetDegradedState();

private:
    RuntimeGuardStatus m_status;

    uint32_t m_lastCheck = 0;
    uint32_t m_networkConnectedAt = 0;
    bool m_startupBaselinePending = false;
};

extern RuntimeGuardService RuntimeGuard;
