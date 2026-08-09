#include "WatchdogService.h"

#include "Services/Config/Config.h"
#include "Services/Logger/Logger.h"

WatchdogService Watchdog;

bool WatchdogService::begin()
{
    configureFromConfig();
    reset();

    Log.info("Watchdog: started");

    return true;
}

void WatchdogService::loop()
{
    if (m_data.state != WatchdogState::Cooldown)
    {
        return;
    }

    if (m_cooldownTimer.expired())
    {
        Log.info("Watchdog: boot delay finished");

        m_data.state = m_data.configuration.enabled
            ? WatchdogState::Monitoring
            : WatchdogState::Idle;
    }
}

void WatchdogService::enable(bool enabled)
{
    if (m_data.configuration.enabled == enabled)
    {
        return;
    }

    m_data.configuration.enabled = enabled;
    reset();

    Log.info(
        "Watchdog: %s",
        enabled ? "enabled" : "disabled");
}

bool WatchdogService::enabled() const
{
    return m_data.configuration.enabled;
}

void WatchdogService::update(const HealthCheckInfo& health)
{
    if (!m_data.configuration.enabled)
    {
        return;
    }

    if (health.lastCheck == 0)
    {
        return;
    }

    if (m_data.state == WatchdogState::RestartRequired ||
        m_data.state == WatchdogState::Cooldown)
    {
        return;
    }

    if (health.available)
    {
        processOnline(health);
    }
    else
    {
        processOffline(health);
    }
}

void WatchdogService::reset()
{
    m_cooldownTimer.stop();

    m_data.runtime.restartPending = false;
    m_data.runtime.consecutiveFailures = 0;

    m_data.state = m_data.configuration.enabled
        ? WatchdogState::Monitoring
        : WatchdogState::Idle;
}

bool WatchdogService::restartRequired() const
{
    return m_data.runtime.restartPending;
}

void WatchdogService::restartCompleted()
{
    if (!m_data.runtime.restartPending)
    {
        return;
    }

    m_data.runtime.restartPending = false;
    m_data.runtime.consecutiveFailures = 0;

    ++m_data.statistics.restartCount;

    m_data.statistics.lastRestart = millis();

    m_cooldownTimer.start(
        m_data.configuration.bootDelay,
        TimerMode::OneShot);

    m_data.state = WatchdogState::Cooldown;

    Log.info(
        "Watchdog: restart completed, boot delay=%lu ms",
        static_cast<unsigned long>(m_data.configuration.bootDelay));
}

const WatchdogData& WatchdogService::data() const
{
    return m_data;
}

WatchdogStatusData WatchdogService::status() const
{
    WatchdogStatusData status;

    status.summary.state = m_data.state;
    status.summary.enabled = m_data.configuration.enabled;
    status.summary.restartPending =
        m_data.runtime.restartPending;
    status.summary.restartRequired =
        m_data.state == WatchdogState::RestartRequired;
    status.summary.lockedOut =
        m_data.state == WatchdogState::LockedOut;
    status.summary.cooldown =
        m_data.state == WatchdogState::Cooldown;
    status.summary.consecutiveFailures =
        m_data.runtime.consecutiveFailures;

    status.configuration.failureThreshold =
        m_data.configuration.failureThreshold;
    status.configuration.bootDelay =
        m_data.configuration.bootDelay;
    status.configuration.powerOffTime =
        m_data.configuration.powerOffTime;
    status.configuration.maxRestartPerDay =
        m_data.configuration.maxRestartPerDay;

    status.statistics.restartCount =
        m_data.statistics.restartCount;
    status.statistics.lastSuccess =
        m_data.statistics.lastSuccess;
    status.statistics.lastFailure =
        m_data.statistics.lastFailure;
    status.statistics.lastRestart =
        m_data.statistics.lastRestart;
    status.statistics.lockedOutAt =
        m_data.statistics.lockedOutAt;

    return status;
}

void WatchdogService::configureFromConfig()
{
    const auto& watchdog = Config.data().watchdog;

    m_data.configuration.enabled = true;
    m_data.configuration.failureThreshold = watchdog.failCount;
    m_data.configuration.bootDelay = watchdog.bootDelay;
    m_data.configuration.powerOffTime = watchdog.powerOffTime;
    m_data.configuration.maxRestartPerDay = watchdog.maxRestartPerDay;
}

void WatchdogService::processOnline(const HealthCheckInfo& health)
{
    m_data.runtime.consecutiveFailures = 0;
    m_data.statistics.lastSuccess = health.lastSuccess;

    if (m_data.state == WatchdogState::LockedOut)
    {
        Log.info("Watchdog: target recovered after lockout");
    }

    m_data.state = WatchdogState::Monitoring;
}

void WatchdogService::processOffline(const HealthCheckInfo& health)
{
    m_data.runtime.consecutiveFailures = health.consecutiveFails;
    m_data.statistics.lastFailure = health.lastFail;

    if (m_data.runtime.consecutiveFailures <
        m_data.configuration.failureThreshold)
    {
        return;
    }

    if (m_data.statistics.restartCount >=
        m_data.configuration.maxRestartPerDay)
    {
        if (m_data.state != WatchdogState::LockedOut)
        {
            m_data.statistics.lockedOutAt = millis();
            m_data.state = WatchdogState::LockedOut;

            Log.error(
                "Watchdog: restart limit reached, maxRestartPerDay=%u",
                m_data.configuration.maxRestartPerDay);
        }

        return;
    }

    if (!canRestart())
    {
        return;
    }

    requestRestart();
}

void WatchdogService::requestRestart()
{
    m_data.runtime.restartPending = true;
    m_data.state = WatchdogState::RestartRequired;

    Log.warning(
        "Watchdog: restart required, failures=%lu, powerOffTime=%lu ms",
        static_cast<unsigned long>(m_data.runtime.consecutiveFailures),
        static_cast<unsigned long>(m_data.configuration.powerOffTime));
}

bool WatchdogService::canRestart() const
{
    if (!m_data.configuration.enabled)
    {
        return false;
    }

    if (m_data.runtime.restartPending)
    {
        return false;
    }

    if (m_data.state == WatchdogState::Cooldown)
    {
        return false;
    }

    if (m_data.statistics.restartCount >=
        m_data.configuration.maxRestartPerDay)
    {
        return false;
    }

    return true;
}
