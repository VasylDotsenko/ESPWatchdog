#pragma once

#include <Arduino.h>

//=============================================================================
// Health Check Protocol
//=============================================================================

enum class HealthProtocol : uint8_t
{
    ICMP = 0,

    TCP,

    HTTP,

    MQTT
};

//=============================================================================
// Health Check State
//=============================================================================

enum class HealthState : uint8_t
{
    Idle = 0,

    Running,

    Online,

    Offline,

    Error
};

//=============================================================================
// Health Statistics
//=============================================================================

struct HealthStatistics
{
    uint32_t totalChecks = 0;

    uint32_t successfulChecks = 0;

    uint32_t failedChecks = 0;

    uint32_t consecutiveFailures = 0;

    uint32_t consecutiveSuccesses = 0;

    uint32_t averageResponseTime = 0;

    uint32_t minimumResponseTime = 0;

    uint32_t maximumResponseTime = 0;
};

//=============================================================================
// Health Runtime
//=============================================================================

struct HealthRuntime
{
    bool online = false;

    uint32_t lastResponseTime = 0;

    uint32_t lastSuccessMillis = 0;

    uint32_t lastFailureMillis = 0;
};

//=============================================================================
// Health Configuration
//=============================================================================

struct HealthConfiguration
{
    HealthProtocol protocol = HealthProtocol::ICMP;

    uint32_t interval = 10000;

    uint32_t timeout = 1000;

    uint8_t maxFailures = 3;
};

//=============================================================================
// Health Check Data
//=============================================================================

struct HealthCheckData
{
    HealthState state = HealthState::Idle;

    HealthConfiguration configuration;

    HealthRuntime runtime;

    HealthStatistics statistics;
};