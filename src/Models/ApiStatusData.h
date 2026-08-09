#pragma once

#include <Arduino.h>

#include "Models/HealthStatusData.h"
#include "Models/NetworkStatusData.h"
#include "Models/PowerStatusData.h"
#include "Models/SystemStatusData.h"
#include "Models/WatchdogStatusData.h"

struct ApiStatusData
{
    SystemStatusData system;
    NetworkStatusData network;
    HealthStatusData health;
    WatchdogStatusData watchdog;
    PowerStatusData power;
};
