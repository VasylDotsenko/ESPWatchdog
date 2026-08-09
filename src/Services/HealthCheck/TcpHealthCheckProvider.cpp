#include "TcpHealthCheckProvider.h"

#include "Services/Config/Config.h"
#include "Services/Logger/Logger.h"

TcpHealthCheckProvider TcpProvider;

//=============================================================================

bool TcpHealthCheckProvider::begin()
{
    reset();

    m_client.setNoDelay(true);

    return true;
}

//=============================================================================

bool TcpHealthCheckProvider::start(
    const char* host,
    uint32_t timeoutMs)
{
    if (m_running)
    {
        Log.warning("TCP provider: previous check is still running");
        return false;
    }

    reset();

    if (host == nullptr ||
        host[0] == '\0')
    {
        m_result.success = false;
        m_result.status = HealthCheckStatus::Error;
        m_finished = true;
        return true;
    }

    if (!WiFi.isConnected())
    {
        m_result.success = false;
        m_result.status = HealthCheckStatus::NetworkUnavailable;
        m_finished = true;
        return true;
    }

    const auto& watchdog = Config.data().watchdog;

    if (watchdog.targetPort == 0)
    {
        m_result.success = false;
        m_result.status = HealthCheckStatus::Error;
        m_finished = true;
        return true;
    }

    const uint32_t startedAt = millis();

    m_running = true;

    m_client.setTimeout(timeoutMs);

    const bool connected =
        m_client.connect(
            host,
            watchdog.targetPort);

    const uint32_t elapsed =
        millis() - startedAt;

    if (connected)
    {
        m_client.stop();

        m_result.success = true;
        m_result.status = HealthCheckStatus::Success;
        m_result.responseTime = elapsed;

        Log.verbose(
            "TCP provider: connected to %s:%u in %lu ms",
            host,
            watchdog.targetPort,
            static_cast<unsigned long>(elapsed));
    }
    else
    {
        m_client.stop();

        m_result.success = false;
        m_result.responseTime = elapsed;

        if (elapsed >= timeoutMs)
        {
            m_result.status = HealthCheckStatus::Timeout;
        }
        else
        {
            m_result.status = HealthCheckStatus::HostUnreachable;
        }

        Log.verbose(
            "TCP provider: failed to connect to %s:%u, status=%u, time=%lu ms",
            host,
            watchdog.targetPort,
            static_cast<uint8_t>(m_result.status),
            static_cast<unsigned long>(elapsed));
    }

    m_running = false;
    m_finished = true;

    return true;
}

//=============================================================================

void TcpHealthCheckProvider::loop()
{
}

//=============================================================================

bool TcpHealthCheckProvider::running() const
{
    return m_running;
}

//=============================================================================

bool TcpHealthCheckProvider::finished() const
{
    return m_finished;
}

//=============================================================================

void TcpHealthCheckProvider::cancel()
{
    if (m_client.connected())
    {
        m_client.stop();
    }

    m_running = false;
    m_finished = true;

    m_result.success = false;
    m_result.status = HealthCheckStatus::Cancelled;
    m_result.responseTime = 0;
}

//=============================================================================

const HealthCheckResult& TcpHealthCheckProvider::result() const
{
    return m_result;
}

//=============================================================================

void TcpHealthCheckProvider::reset()
{
    if (m_client.connected())
    {
        m_client.stop();
    }

    m_running = false;
    m_finished = false;

    m_result.success = false;
    m_result.status = HealthCheckStatus::Error;
    m_result.responseTime = 0;
}
