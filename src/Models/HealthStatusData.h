#pragma once

#include <Arduino.h>

#include "Services/HealthCheck/HealthCheckResult.h"

struct HealthStatusSummary
{
    bool available = false;
    bool running = false;

    HealthCheckStatus lastStatus = HealthCheckStatus::Error;

    uint32_t responseTime = 0;
};

struct HealthStatusStatistics
{
    uint32_t sent = 0;
    uint32_t received = 0;
    uint32_t lost = 0;

    uint32_t consecutiveSuccess = 0;
    uint32_t consecutiveFails = 0;

    uint32_t minResponseTime = 0;
    uint32_t maxResponseTime = 0;
};

struct HealthStatusTimestamps
{
    uint64_t lastCheck = 0;
    uint64_t lastSuccess = 0;
    uint64_t lastFail = 0;
    uint64_t availabilityChanged = 0;
};

struct HealthStatusData
{
    HealthStatusSummary summary;
    HealthStatusStatistics statistics;
    HealthStatusTimestamps timestamps;
};
