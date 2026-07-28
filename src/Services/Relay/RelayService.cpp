#include "RelayService.h"

#include "Services/Config/Config.h"
#include "Services/Logger/Logger.h"

RelayService Relay;

bool RelayService::begin()
{
    configureFromConfig();

    if (!m_data.configuration.enabled)
    {
        m_data.state = RelayState::Disabled;
        Log.info("Relay: disabled");
        return true;
    }

    pinMode(m_data.configuration.pin, OUTPUT);

    powerOn();

    Log.info(
        "Relay: started, pin=%u, activeHigh=%u",
        m_data.configuration.pin,
        m_data.configuration.activeHigh);

    return true;
}

void RelayService::loop()
{
    if (!m_data.runtime.restartInProgress)
    {
        return;
    }

    if (m_data.state != RelayState::PowerOff)
    {
        return;
    }

    if (!m_powerOffTimer.expired())
    {
        return;
    }

    powerOn();

    m_data.runtime.restartInProgress = false;
    m_data.runtime.restartCompleted = true;

    ++m_data.statistics.restartCount;

    Log.info("Relay: restart cycle completed");
}

void RelayService::powerOn()
{
    if (!m_data.configuration.enabled)
    {
        return;
    }

    writeRelay(true);

    m_data.state = RelayState::PowerOn;
    m_data.statistics.lastPowerOn = millis();

    Log.info("Relay: power ON");
}

void RelayService::powerOff()
{
    if (!m_data.configuration.enabled)
    {
        return;
    }

    writeRelay(false);

    m_data.state = RelayState::PowerOff;
    m_data.statistics.lastPowerOff = millis();

    Log.warning("Relay: power OFF");
}

bool RelayService::restart(uint32_t powerOffTime)
{
    if (!m_data.configuration.enabled)
    {
        return false;
    }

    if (m_data.runtime.restartInProgress)
    {
        return false;
    }

    m_data.runtime.restartCompleted = false;
    m_data.runtime.restartInProgress = true;
    m_data.runtime.powerOffTime = powerOffTime;

    powerOff();

    m_powerOffTimer.start(
        powerOffTime,
        TimerMode::OneShot);

    Log.warning(
        "Relay: restart cycle started, powerOffTime=%lu ms",
        static_cast<unsigned long>(powerOffTime));

    return true;
}

bool RelayService::restartInProgress() const
{
    return m_data.runtime.restartInProgress;
}

bool RelayService::restartCompleted() const
{
    return m_data.runtime.restartCompleted;
}

void RelayService::clearRestartCompleted()
{
    m_data.runtime.restartCompleted = false;
}

const RelayData& RelayService::data() const
{
    return m_data;
}

void RelayService::configureFromConfig()
{
    const auto& relay = Config.data().relay;

    m_data.configuration.enabled = relay.enabled;
    m_data.configuration.pin = relay.pin;
    m_data.configuration.activeHigh = relay.activeHigh;
}

void RelayService::writeRelay(bool powerEnabled)
{
    const bool level = m_data.configuration.activeHigh
        ? powerEnabled
        : !powerEnabled;

    digitalWrite(
        m_data.configuration.pin,
        level ? HIGH : LOW);
}
