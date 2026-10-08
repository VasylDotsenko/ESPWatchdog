#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "Services/HealthCheck/IHealthCheckProvider.h"

//=============================================================================
// TCP Health Check Provider
//=============================================================================
//
// Checks host availability by opening a TCP connection to the configured
// watchdog target port. The ESP8266 Arduino WiFiClient implementation owns
// the socket lifecycle. Every probe has a strictly bounded 250 ms connect
// slice, so an unavailable target cannot stall the web runtime for seconds.
//
// For SSH-based health checks use:
//
//   watchdog.targetPort = 22
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
    static constexpr uint32_t MAX_CONNECT_SLICE_MS = 250;

    void reset();

private:
    WiFiClient m_client;

    HealthCheckResult m_result;

    bool m_running = false;

    bool m_finished = false;
};

//=============================================================================

extern TcpHealthCheckProvider TcpProvider;
