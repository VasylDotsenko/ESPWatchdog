#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "Services/HealthCheck/IHealthCheckProvider.h"

//=============================================================================
// TCP Health Check Provider
//=============================================================================
//
// Checks host availability by opening a TCP connection to the configured
// watchdog target port.
//
// For SSH-based health checks use:
//
//   watchdog.targetPort = 22
//
// This is intentionally not a full SSH protocol implementation. A successful
// TCP connect means that the host is reachable and the SSH service accepts
// connections.
//
//=============================================================================

class TcpHealthCheckProvider final : public IHealthCheckProvider
{
public:
    bool begin() override;

    bool start(
        const char* host,
        uint32_t timeoutMs) override;

    void loop() override;

    bool running() const override;

    bool finished() const override;

    void cancel() override;

    const HealthCheckResult& result() const override;

private:
    void reset();

private:
    WiFiClient m_client;

    HealthCheckResult m_result;

    bool m_running = false;

    bool m_finished = false;
};

//=============================================================================

extern TcpHealthCheckProvider TcpProvider;
