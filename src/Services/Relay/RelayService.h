#pragma once

#include <Arduino.h>

#include "Core/IService.h"
#include "Core/Timer.h"
#include "Models/RelayData.h"

class RelayService final : public IService
{
public:
    bool begin() override;

    void loop() override;

    void powerOn();

    void powerOff();

    bool restart(uint32_t powerOffTime);

    [[nodiscard]]
    bool restartInProgress() const;

    [[nodiscard]]
    bool restartCompleted() const;

    void clearRestartCompleted();

    [[nodiscard]]
    const RelayData& data() const;

private:
    void configureFromConfig();

    void writeRelay(bool powerEnabled);

private:
    RelayData m_data;

    Timer m_powerOffTimer;
};

extern RelayService Relay;
