#include "IcmpHealthCheckProvider.h"

#include "Services/Logger/Logger.h"

IcmpHealthCheckProvider IcmpProvider;

bool IcmpHealthCheckProvider::begin()
{
    reset();

    if (!m_session.begin())
    {
        Log.error(F("ICMP provider: session initialization failed"));
        return false;
    }

    return true;
}

bool IcmpHealthCheckProvider::start(const char* host, uint32_t timeoutMs)
{
    if (m_session.running())
    {
        Log.warning(F("ICMP provider: previous check is still running"));
        return false;
    }

    reset();

    if (!m_session.start(host, 0, timeoutMs))
    {
        // DNS and SDK startup failures are valid health-check results. Report
        // them as a completed operation so HealthCheckService records them.
        setResult(m_session.result());
        m_finished = true;
        return true;
    }

    m_running = true;
    return true;
}

void IcmpHealthCheckProvider::loop()
{
    m_session.loop();

    if (m_finished || !m_session.finished())
    {
        return;
    }

    setResult(m_session.result());
    m_running = false;
    m_finished = true;
}

bool IcmpHealthCheckProvider::running() const
{
    return m_running;
}

bool IcmpHealthCheckProvider::finished() const
{
    return m_finished;
}

void IcmpHealthCheckProvider::cancel()
{
    if (!m_running)
    {
        return;
    }

    // The ESP8266 lwIP ping API provides no cancellation primitive. Keep the
    // underlying session alive until its callback, but publish Cancelled now.
    m_running = false;
    m_finished = true;
    m_result.success = false;
    m_result.status = HealthCheckStatus::Cancelled;
    m_result.responseTime = 0;
}

const HealthCheckResult& IcmpHealthCheckProvider::result() const
{
    return m_result;
}

HealthCheckStatus IcmpHealthCheckProvider::toHealthStatus(
    NetworkResult::Status status)
{
    switch (status)
    {
        case NetworkResult::Status::Success:
            return HealthCheckStatus::Success;

        case NetworkResult::Status::Timeout:
            return HealthCheckStatus::Timeout;

        case NetworkResult::Status::HostUnreachable:
            return HealthCheckStatus::HostUnreachable;

        case NetworkResult::Status::NetworkUnavailable:
            return HealthCheckStatus::NetworkUnavailable;

        case NetworkResult::Status::DnsFailed:
            return HealthCheckStatus::DnsFailed;

        case NetworkResult::Status::Cancelled:
            return HealthCheckStatus::Cancelled;

        case NetworkResult::Status::Error:
        default:
            return HealthCheckStatus::Error;
    }
}

void IcmpHealthCheckProvider::setResult(const NetworkResult& networkResult)
{
    m_result.success = networkResult.success;
    m_result.status = toHealthStatus(networkResult.status);
    m_result.responseTime = networkResult.responseTime;
}

void IcmpHealthCheckProvider::reset()
{
    m_running = false;
    m_finished = false;
    m_result.success = false;
    m_result.status = HealthCheckStatus::Error;
    m_result.responseTime = 0;
}
