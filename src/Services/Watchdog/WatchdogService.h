#pragma once

#include <Arduino.h>

#include "Core/IService.h"
#include "Core/Timer.h"
#include "Models/WatchdogData.h"
#include "Services/HealthCheck/HealthCheckInfo.h"

class WatchdogService final : public IService
{
public:
    bool begin() override;

    void loop() override;

    void enable(bool enabled);

    [[nodiscard]]
    bool enabled() const;

    void update(const HealthCheckInfo& health);

    void reset();

    [[nodiscard]]
    bool restartRequired() const;

    void restartCompleted();

    [[nodiscard]]
    const WatchdogData& data() const;

private:
    void configureFromConfig();

    void processOnline(const HealthCheckInfo& health);

    void processOffline(const HealthCheckInfo& health);

    void requestRestart();

    [[nodiscard]]
    bool canRestart() const;

private:
    WatchdogData m_data;

    Timer m_cooldownTimer;
};

extern WatchdogService Watchdog;
