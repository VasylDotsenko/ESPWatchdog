#pragma once

#include <Arduino.h>

#include "HealthCheckResult.h"

//=============================================================================
// Health Check Information
//=============================================================================
//
// Поточний стан та накопичена статистика сервісу HealthCheck.
//
//=============================================================================

struct HealthCheckInfo
{
    //---------------------------------------------------------------------
    // Current availability
    //---------------------------------------------------------------------

    bool available = false;

    HealthCheckStatus lastStatus = HealthCheckStatus::Error;

    uint32_t responseTime = 0;

    //---------------------------------------------------------------------
    // Statistics
    //---------------------------------------------------------------------

    uint32_t sent = 0;

    uint32_t received = 0;

    uint32_t lost = 0;

    uint32_t consecutiveSuccess = 0;

    uint32_t consecutiveFails = 0;

    //---------------------------------------------------------------------
    // Response time statistics
    //---------------------------------------------------------------------

    uint32_t minResponseTime = UINT32_MAX;

    uint32_t maxResponseTime = 0;

    //---------------------------------------------------------------------
    // Timestamps (milliseconds from system uptime)
    //---------------------------------------------------------------------

    uint64_t lastCheck = 0;

    uint64_t lastSuccess = 0;

    uint64_t lastFail = 0;

    //---------------------------------------------------------------------
    // Availability state
    //---------------------------------------------------------------------

    uint64_t availabilityChanged = 0;

    //---------------------------------------------------------------------
    // Reset statistics
    //---------------------------------------------------------------------

    void reset()
    {
        available = false;

        lastStatus = HealthCheckStatus::Error;

        responseTime = 0;

        sent = 0;

        received = 0;

        lost = 0;

        consecutiveSuccess = 0;

        consecutiveFails = 0;

        minResponseTime = UINT32_MAX;

        maxResponseTime = 0;

        lastCheck = 0;

        lastSuccess = 0;

        lastFail = 0;

        availabilityChanged = 0;
    }
};