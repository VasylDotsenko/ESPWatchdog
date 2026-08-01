#pragma once

#include <Arduino.h>

#include "Core/IService.h"
#include "Core/Timer.h"
#include "IPowerController.h"
#include "Models/PowerData.h"

class PowerService final : public IService
{
public:
    bool begin() override;

    void loop() override;

    void setController(IPowerController& controller);

    bool restart(uint32_t powerOffTime);

    [[nodiscard]]
    bool restartInProgress() const;

    [[nodiscard]]
    bool restartCompleted() const;

    void clearRestartCompleted();

    [[nodiscard]]
    bool available() const;

    [[nodiscard]]
    const PowerData& data() const;

private:
    static constexpr uint32_t RESTART_RETRY_INTERVAL_MS = 30000;
    static constexpr uint32_t POWER_ON_RETRY_INTERVAL_MS = 2000;
    static constexpr uint32_t POWER_ON_WAIT_TIMEOUT_MS = 60000;

private:
    bool shouldAttemptPowerOn(uint64_t now);

    bool powerOn();

    bool powerOff();

    void fail(const char* reason);

private:
    IPowerController* m_controller = nullptr;

    PowerData m_data;

    Timer m_powerOffTimer;
};

extern PowerService Power;
