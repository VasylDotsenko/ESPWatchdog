#include "WatchdogRestartQuotaStorage.h"

#include <ArduinoJson.h>

#include "Services/Logger/Logger.h"
#include "Services/Storage/Storage.h"

bool WatchdogRestartQuotaStorage::load(
    WatchdogRestartQuotaData& quota) const
{
    quota = WatchdogRestartQuotaData {};

    if (!Storage.exists(FILE_PATH))
    {
        return true;
    }

    JsonDocument document;

    if (!Storage.readJson(FILE_PATH, document))
    {
        Log.warning("WatchdogQuota: unable to read persistent quota");
        return false;
    }

    const JsonObjectConst root = document.as<JsonObjectConst>();

    if ((root["version"] | 0) != FORMAT_VERSION)
    {
        Log.warning("WatchdogQuota: unsupported persistent quota version");
        return false;
    }

    const uint8_t capacity = WatchdogRestartQuotaData::CAPACITY;
    const uint8_t storedHead = root["head"] | 0;
    const uint8_t storedCount = root["count"] | 0;

    quota.head = storedHead < capacity ? storedHead : 0;
    quota.count = storedCount <= capacity ? storedCount : 0;

    const JsonArrayConst timestamps =
        root["completedAtEpoch"].as<JsonArrayConst>();

    for (uint8_t index = 0; index < capacity; ++index)
    {
        quota.completedAtEpoch[index] = timestamps[index] | 0ULL;
    }

    Log.info(
        "WatchdogQuota: restored %u entries",
        static_cast<unsigned int>(quota.count));

    return true;
}

bool WatchdogRestartQuotaStorage::save(
    const WatchdogRestartQuotaData& quota) const
{
    JsonDocument document;
    JsonObject root = document.to<JsonObject>();

    root["version"] = FORMAT_VERSION;
    root["head"] = quota.head;
    root["count"] = quota.count;

    JsonArray timestamps = root["completedAtEpoch"].to<JsonArray>();

    for (uint8_t index = 0;
         index < WatchdogRestartQuotaData::CAPACITY;
         ++index)
    {
        timestamps.add(quota.completedAtEpoch[index]);
    }

    if (!Storage.writeJson(FILE_PATH, document))
    {
        Log.warning("WatchdogQuota: unable to save persistent quota");
        return false;
    }

    return true;
}
