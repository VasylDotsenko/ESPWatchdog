#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "Models/ApiStatusData.h"

class JsonStatusSerializer
{
public:
    static bool serialize(
        const ApiStatusData& status,
        char* output,
        size_t outputCapacity,
        size_t& outputLength);

private:
    static void writeSystem(
        JsonObject target,
        const SystemStatusData& status);

    static void writeNetwork(
        JsonObject target,
        const NetworkStatusData& status);

    static void writeHealth(
        JsonObject target,
        const HealthStatusData& status);

    static void writeWatchdog(
        JsonObject target,
        const WatchdogStatusData& status);

    static void writePower(
        JsonObject target,
        const PowerStatusData& status);

    static void writeRestartHistory(
        JsonObject target,
        const RestartHistoryStatus& history);

    static void writeIPv4(
        JsonObject target,
        const char* key,
        const IPv4Address& address);
};
