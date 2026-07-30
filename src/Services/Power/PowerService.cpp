#include "PowerService.h"

#include "Services/Logger/Logger.h"

PowerService Power;

bool PowerService::begin()
{
    if (m_controller == nullptr)
    {
        m_data.state = PowerState::Disabled;

        Log.warning("PowerService: controller not configured");

        return true;
    }

    if (!m_controller->begin())
    {
        fail("controller initialization failed");
        return false;
    }

    m_data.state = PowerState::Idle;

    Log.info(
        "PowerService: started, controller=%s",
        m_controller->name());

    return true;
}

void PowerService::loop()
{
    if (!m_data.runtime.restartInProgress)
    {
        return;
    }

    if (m_data.state != PowerState::WaitingPowerOn)
    {
        return;
    }

    if (!m_powerOffTimer.expired())
    {
        return;
    }

    if (!powerOn())
    {
        fail("powerOn failed");
        return;
    }

    m_data.runtime.restartInProgress = false;
    m_data.runtime.restartCompleted = true;
    m_data.runtime.lastOperationSucceeded = true;

    ++m_data.statistics.restartCount;

    m_data.statistics.lastRestart = millis();

    m_data.state = PowerState::Idle;

    Log.info("PowerService: restart completed");
}

void PowerService::setController(IPowerController& controller)
{
    m_controller = &controller;
}

bool PowerService::restart(uint32_t powerOffTime)
{
    if (m_controller == nullptr)
    {
        fail("controller not configured");
        return false;
    }

    if (m_data.runtime.restartInProgress)
    {
        return false;
    }

    if (!available())
    {
        fail("controller unavailable");
        return false;
    }

    m_data.runtime.restartCompleted = false;
    m_data.runtime.lastOperationSucceeded = false;
    m_data.runtime.powerOffTime = powerOffTime;

    if (!powerOff())
    {
        fail("powerOff failed");
        return false;
    }

    m_data.runtime.restartInProgress = true;
    m_data.state = PowerState::WaitingPowerOn;

    m_powerOffTimer.start(
        powerOffTime,
        TimerMode::OneShot);

    Log.warning(
        "PowerService: restart started, powerOffTime=%lu ms",
        static_cast<unsigned long>(powerOffTime));

    return true;
}

bool PowerService::restartInProgress() const
{
    return m_data.runtime.restartInProgress;
}

bool PowerService::restartCompleted() const
{
    return m_data.runtime.restartCompleted;
}

void PowerService::clearRestartCompleted()
{
    m_data.runtime.restartCompleted = false;
}

bool PowerService::available() const
{
    return m_controller != nullptr &&
           m_controller->available();
}

const PowerData& PowerService::data() const
{
    return m_data;
}

bool PowerService::powerOn()
{
    if (m_controller == nullptr)
    {
        return false;
    }

    if (!m_controller->powerOn())
    {
        return false;
    }

    m_data.statistics.lastPowerOn = millis();

    Log.info("PowerService: power ON");

    return true;
}

bool PowerService::powerOff()
{
    if (m_controller == nullptr)
    {
        return false;
    }

    if (!m_controller->powerOff())
    {
        return false;
    }

    m_data.statistics.lastPowerOff = millis();
    m_data.state = PowerState::PowerOff;

    Log.warning("PowerService: power OFF");

    return true;
}

void PowerService::fail(const char* reason)
{
    ++m_data.statistics.errorCount;

    m_data.statistics.lastError = millis();

    m_data.runtime.restartInProgress = false;
    m_data.runtime.lastOperationSucceeded = false;

    m_powerOffTimer.stop();

    m_data.state = PowerState::Error;

    Log.error(
        "PowerService: %s",
        reason != nullptr ? reason : "error");
}
