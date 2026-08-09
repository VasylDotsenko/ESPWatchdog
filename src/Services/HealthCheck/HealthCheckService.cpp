#include "HealthCheckService.h"

#include "Services/Config/Config.h"
#include "Services/Logger/Logger.h"

HealthCheckService HealthCheck;

//=============================================================================
// Dependency Injection
//=============================================================================

void HealthCheckService::setProvider(IHealthCheckProvider& provider)
{
    m_provider = &provider;
}

//=============================================================================
// IService
//=============================================================================

bool HealthCheckService::begin()
{
    if (m_provider == nullptr)
    {
        Log.error(F("HealthCheck: provider not configured"));
        return false;
    }

    if (!m_provider->begin())
    {
        Log.error(F("HealthCheck: provider initialization failed"));
        return false;
    }

    reset();

    Log.info(F("HealthCheck: started"));

    return true;
}

//=============================================================================

void HealthCheckService::loop()
{
    if (m_provider == nullptr)
    {
        return;
    }

    m_provider->loop();

    switch (m_state)
    {
        case State::Idle:
        {
            if (m_timer.expired())
            {
                startCheck();
            }

            break;
        }

        case State::Waiting:
        {
            if (m_provider->finished())
            {
                m_state = State::Idle;
                processResult();
            }

            break;
        }

        default:
        {
            m_state = State::Idle;
            break;
        }
    }
}

//=============================================================================

void HealthCheckService::reset()
{
    m_info.reset();

    m_state = State::Idle;

    if (m_provider != nullptr && m_provider->running())
    {
        m_provider->cancel();
    }

    m_timer.start(
        Config.data().watchdog.pingInterval,
        TimerMode::Periodic);
}

//=============================================================================
// Current state
//=============================================================================

bool HealthCheckService::online() const
{
    return m_info.available;
}

bool HealthCheckService::running() const
{
    return m_provider != nullptr &&
           m_provider->running();
}

const HealthCheckInfo& HealthCheckService::info() const
{
    return m_info;
}

HealthStatusData HealthCheckService::status() const
{
    HealthStatusData status;

    status.summary.available = m_info.available;
    status.summary.running = running();
    status.summary.lastStatus = m_info.lastStatus;
    status.summary.responseTime = m_info.responseTime;

    status.statistics.sent = m_info.sent;
    status.statistics.received = m_info.received;
    status.statistics.lost = m_info.lost;
    status.statistics.consecutiveSuccess =
        m_info.consecutiveSuccess;
    status.statistics.consecutiveFails =
        m_info.consecutiveFails;
    status.statistics.minResponseTime =
        m_info.minResponseTime == UINT32_MAX
            ? 0
            : m_info.minResponseTime;
    status.statistics.maxResponseTime =
        m_info.maxResponseTime;

    status.timestamps.lastCheck = m_info.lastCheck;
    status.timestamps.lastSuccess = m_info.lastSuccess;
    status.timestamps.lastFail = m_info.lastFail;
    status.timestamps.availabilityChanged =
        m_info.availabilityChanged;

    return status;
}

//=============================================================================
// Start new health check
//=============================================================================

void HealthCheckService::startCheck()
{
    if (m_provider == nullptr)
    {
        return;
    }

    if (m_provider->running())
    {
        Log.warning(F("HealthCheck: previous check still running"));
        return;
    }

    const auto& watchdog = Config.data().watchdog;

    if (watchdog.targetHost[0] == '\0')
    {
        Log.error(F("HealthCheck: target host is not configured"));
        return;
    }

    if (!m_provider->start(
            watchdog.targetHost,
            watchdog.pingTimeout))
    {
        Log.error(F("HealthCheck: unable to start provider"));
        return;
    }

    m_state = State::Waiting;

    Log.verbose(
        F("HealthCheck: started (%s)"),
        watchdog.targetHost);
}

//=============================================================================
// Process completed health check
//=============================================================================

void HealthCheckService::processResult()
{
    const HealthCheckResult& result = m_provider->result();

    const auto& watchdog = Config.data().watchdog;

    const uint64_t now = millis();

    const bool previousAvailable = m_info.available;

    m_info.lastCheck = now;
    m_info.lastStatus = result.status;
    m_info.responseTime = result.responseTime;

    ++m_info.sent;

    if (result.success)
    {
        ++m_info.received;

        ++m_info.consecutiveSuccess;
        m_info.consecutiveFails = 0;

        m_info.lastSuccess = now;

        if (result.responseTime < m_info.minResponseTime)
        {
            m_info.minResponseTime = result.responseTime;
        }

        if (result.responseTime > m_info.maxResponseTime)
        {
            m_info.maxResponseTime = result.responseTime;
        }

        m_info.available = true;
    }
    else
    {
        ++m_info.lost;

        ++m_info.consecutiveFails;
        m_info.consecutiveSuccess = 0;

        m_info.lastFail = now;

        if (m_info.consecutiveFails >= watchdog.failCount)
        {
            m_info.available = false;
        }
    }

    if (previousAvailable != m_info.available)
    {
        m_info.availabilityChanged = now;

        Log.info(
            F("HealthCheck: %s"),
            m_info.available ? "ONLINE" : "OFFLINE");
    }

        Log.verbose(
        F("HealthCheck: status=%u available=%u sent=%lu received=%lu lost=%lu "
          "success=%lu fails=%lu rtt=%lu ms"),
        static_cast<uint8_t>(result.status),
        m_info.available,
        m_info.sent,
        m_info.received,
        m_info.lost,
        m_info.consecutiveSuccess,
        m_info.consecutiveFails,
        m_info.responseTime);
}

//=============================================================================
// End of file
//=============================================================================
