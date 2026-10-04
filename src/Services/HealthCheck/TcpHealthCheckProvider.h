#pragma once

#include <Arduino.h>

extern "C"
{
#include <espconn.h>
#include <lwip/ip4_addr.h>
}

#include "Services/HealthCheck/IHealthCheckProvider.h"

//=============================================================================
// TCP Health Check Provider
//=============================================================================
//
// Checks host availability by opening a TCP connection to the configured
// watchdog target port. The connection and DNS lookup run through espconn
// callbacks, so a failed target never blocks the Arduino loop.
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
    static constexpr size_t HOST_CAPACITY = 64;

    static TcpHealthCheckProvider* providerFromCallback(void* argument);

    static void onConnected(void* argument);

    static void onReconnect(void* argument, sint8 error);

    static void onDisconnected(void* argument);

    static void onDnsFound(
        const char* name,
        ipv4_addr_t* address,
        void* argument);

    void beginConnection(const ipv4_addr_t& address);

    void handleConnected(espconn* connection);

    void handleReconnect(
        espconn* connection,
        sint8 error);

    void handleDisconnected(espconn* connection);

    void completeSuccess();

    void completeFailure(HealthCheckStatus status);

    void abortConnection();

    HealthCheckStatus statusFromError(sint8 error) const;

    void reset();

private:
    espconn m_connection {};

    esp_tcp m_tcp {};

    ipv4_addr_t m_resolvedAddress {};

    char m_host[HOST_CAPACITY] {};

    HealthCheckResult m_result;

    uint32_t m_startedAt = 0;

    uint32_t m_timeoutMs = 0;

    bool m_running = false;

    bool m_finished = false;

    bool m_dnsPending = false;

    bool m_connectionActive = false;

    // espconn gives callbacks a runtime connection descriptor. It can differ
    // from m_connection, therefore socket teardown must use this pointer.
    espconn* m_activeConnection = nullptr;
};

//=============================================================================

extern TcpHealthCheckProvider TcpProvider;
