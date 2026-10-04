#pragma once

#include <Arduino.h>

#include "Core/IService.h"
#include "Core/Timer.h"
#include "Models/WatchdogData.h"
#include "Models/WatchdogStatusData.h"
#include "Services/HealthCheck/HealthCheckInfo.h"
#include "WatchdogRestartQuotaStorage.h"

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

    [[nodiscard]]
    WatchdogStatusData status() const;

private:
    void configureFromConfig();

    void processOnline(const HealthCheckInfo& health);

    void processOffline(const HealthCheckInfo& health);

    void requestRestart();

    void refreshRestartQuota();

    void recordWatchdogRestart();

    [[nodiscard]]
    bool restartLimitReached() const;

    [[nodiscard]]
    bool canRestart() const;

private:
    WatchdogData m_data;
    WatchdogRestartQuotaData m_restartQuota;
    WatchdogRestartQuotaStorage m_restartQuotaStorage;

    Timer m_cooldownTimer;
};

extern WatchdogService Watchdog;
