#include "WatchdogService.h"

#include <time.h>

#include "Services/Config/Config.h"
#include "Services/Logger/Logger.h"

WatchdogService Watchdog;

namespace
{
    constexpr uint64_t MIN_VALID_EPOCH = 1600000000ULL;
    constexpr uint64_t ROLLING_WINDOW_SECONDS = 24ULL * 60ULL * 60ULL;

    uint64_t wallTimeEpoch()
    {
        const time_t now = time(nullptr);

        return now >= static_cast<time_t>(MIN_VALID_EPOCH)
            ? static_cast<uint64_t>(now)
            : 0;
    }

    void appendQuotaTimestamp(
        WatchdogRestartQuotaData& quota,
        uint64_t completedAtEpoch)
    {
        const uint8_t capacity = WatchdogRestartQuotaData::CAPACITY;

        quota.completedAtEpoch[quota.head] = completedAtEpoch;
        quota.head = static_cast<uint8_t>((quota.head + 1) % capacity);

        if (quota.count < capacity)
        {
            ++quota.count;
        }
    }
}

bool WatchdogService::begin()
{
    configureFromConfig();

    if (!m_restartQuotaStorage.load(m_restartQuota))
    {
        Log.warning("WatchdogQuota: starting with an empty RAM quota");
        m_restartQuota = WatchdogRestartQuotaData {};
    }

    refreshRestartQuota();
    reset();

    Log.info("Watchdog: started");

    return true;
}

void WatchdogService::loop()
{
    refreshRestartQuota();

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

    recordWatchdogRestart();

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
    status.summary.restartLimitReached =
        restartLimitReached();
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
    status.statistics.restartsLast24Hours =
        m_data.statistics.restartsLast24Hours;
    status.statistics.quotaTimeSynchronized =
        m_data.statistics.quotaTimeSynchronized;
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

    refreshRestartQuota();

    if (restartLimitReached())
    {
        if (m_data.state != WatchdogState::LockedOut)
        {
            m_data.statistics.lockedOutAt = millis();
            m_data.state = WatchdogState::LockedOut;

            Log.error(
                "Watchdog: rolling restart limit reached, restartsLast24h=%u limit=%u",
                m_data.statistics.restartsLast24Hours,
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

void WatchdogService::refreshRestartQuota()
{
    const uint64_t nowEpoch = wallTimeEpoch();

    m_data.statistics.quotaTimeSynchronized = nowEpoch != 0;

    if (nowEpoch == 0)
    {
        // Before NTP is valid, retain a conservative per-boot fallback.
        m_data.statistics.restartsLast24Hours =
            m_data.statistics.restartCount > UINT8_MAX
                ? UINT8_MAX
                : static_cast<uint8_t>(m_data.statistics.restartCount);
        return;
    }

    WatchdogRestartQuotaData retained;
    const uint8_t capacity = WatchdogRestartQuotaData::CAPACITY;
    const uint8_t start = m_restartQuota.count < capacity
        ? 0
        : m_restartQuota.head;

    for (uint8_t index = 0; index < m_restartQuota.count; ++index)
    {
        const uint8_t sourceIndex = static_cast<uint8_t>(
            (start + index) % capacity);
        const uint64_t timestamp =
            m_restartQuota.completedAtEpoch[sourceIndex];

        if (timestamp == 0)
        {
            continue;
        }

        // A future value is retained conservatively if wall time was adjusted.
        if (timestamp > nowEpoch ||
            (nowEpoch - timestamp) < ROLLING_WINDOW_SECONDS)
        {
            appendQuotaTimestamp(retained, timestamp);
        }
    }

    const bool changed =
        retained.count != m_restartQuota.count ||
        retained.head != m_restartQuota.head;

    m_restartQuota = retained;
    m_data.statistics.restartsLast24Hours = retained.count;

    if (changed && !m_restartQuotaStorage.save(m_restartQuota))
    {
        Log.warning("WatchdogQuota: failed to prune persistent quota");
    }
}

void WatchdogService::recordWatchdogRestart()
{
    const uint64_t nowEpoch = wallTimeEpoch();

    if (nowEpoch == 0)
    {
        refreshRestartQuota();

        Log.warning(
            "WatchdogQuota: NTP unavailable; using per-boot restart quota");
        return;
    }

    appendQuotaTimestamp(m_restartQuota, nowEpoch);

    if (!m_restartQuotaStorage.save(m_restartQuota))
    {
        Log.warning("WatchdogQuota: failed to persist watchdog restart");
    }

    refreshRestartQuota();
}

bool WatchdogService::restartLimitReached() const
{
    return m_data.statistics.restartsLast24Hours >=
        m_data.configuration.maxRestartPerDay;
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

    if (restartLimitReached())
    {
        return false;
    }

    return true;
}
