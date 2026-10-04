#pragma once

#include "Models/WatchdogRestartQuotaData.h"

class WatchdogRestartQuotaStorage final
{
public:
    bool load(WatchdogRestartQuotaData& quota) const;

    bool save(const WatchdogRestartQuotaData& quota) const;

private:
    static constexpr const char* FILE_PATH = "/watchdog-restart-quota.json";
    static constexpr uint8_t FORMAT_VERSION = 1;
};
