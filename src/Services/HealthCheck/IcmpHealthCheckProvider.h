#pragma once

#include <Arduino.h>

#include "Network/Icmp/IcmpSession.h"
#include "Services/HealthCheck/IHealthCheckProvider.h"

// Adapts the asynchronous ICMP network session to the HealthCheck contract.
class IcmpHealthCheckProvider final : public IHealthCheckProvider
{
public:
    bool begin() override;

    bool start(const char* host, uint32_t timeoutMs) override;

    void loop() override;

    bool running() const override;

    bool finished() const override;

    void cancel() override;

    const HealthCheckResult& result() const override;

private:
    static HealthCheckStatus toHealthStatus(NetworkResult::Status status);
    void setResult(const NetworkResult& networkResult);
    void reset();

    IcmpSession m_session;
    HealthCheckResult m_result;
    bool m_running = false;
    bool m_finished = false;
};

extern IcmpHealthCheckProvider IcmpProvider;
