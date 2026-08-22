#pragma once

#include <Arduino.h>

#include "Core/IService.h"

class TimeService final : public IService
{
public:
    bool begin() override;

    void loop() override;

    bool synchronized() const;

private:
    static constexpr uint32_t CHECK_INTERVAL_MS = 5000;
    static constexpr uint32_t VALID_EPOCH = 1600000000UL;

private:
    void configure();

    bool isTimeValid() const;

private:
    bool m_configured = false;

    bool m_synchronized = false;

    uint32_t m_lastCheck = 0;
};

extern TimeService TimeSync;
