#include "TimeService.h"

#include <time.h>

#include "Services/Logger/Logger.h"
#include "Services/WiFi/WiFiService.h"

TimeService TimeSync;

namespace
{
    constexpr char TIMEZONE_KYIV[] =
        "EET-2EEST,M3.5.0/3,M10.5.0/4";
}

bool TimeService::begin()
{
    m_configured = false;
    m_synchronized = false;
    m_lastCheck = 0;

    Log.info("TimeService: ready");

    return true;
}

void TimeService::loop()
{
    if (Network.setupMode() ||
        !Network.isConnected())
    {
        return;
    }

    if (!m_configured)
    {
        configure();
    }

    if (m_synchronized)
    {
        return;
    }

    const uint32_t now =
        millis();

    if (m_lastCheck != 0 &&
        static_cast<uint32_t>(now - m_lastCheck) < CHECK_INTERVAL_MS)
    {
        return;
    }

    m_lastCheck = now;

    if (!isTimeValid())
    {
        return;
    }

    m_synchronized = true;

    Log.synchronizeWallTime();

    Log.info("TimeService: time synchronized");
}

bool TimeService::synchronized() const
{
    return m_synchronized;
}

void TimeService::configure()
{
    configTime(
        TIMEZONE_KYIV,
        "pool.ntp.org",
        "time.nist.gov",
        "time.google.com");

    m_configured = true;
    m_lastCheck = 0;

    Log.info("TimeService: NTP configured");
}

bool TimeService::isTimeValid() const
{
    const time_t now =
        time(nullptr);

    return now >= static_cast<time_t>(VALID_EPOCH);
}
