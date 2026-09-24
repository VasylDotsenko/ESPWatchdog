#include "RestartHistoryStorage.h"

#include <ArduinoJson.h>

#include "Services/Logger/Logger.h"
#include "Services/Storage/Storage.h"

namespace
{
    bool validResult(uint8_t value)
    {
        return value <= static_cast<uint8_t>(RestartResult::Failed);
    }

    bool validReason(uint8_t value)
    {
        return value <= static_cast<uint8_t>(RestartReason::Interrupted);
    }
}

bool RestartHistoryStorage::load(RestartHistoryData& history) const
{
    history = RestartHistoryData {};

    if (!Storage.exists(FILE_PATH))
    {
        return true;
    }

    JsonDocument document;

    if (!Storage.readJson(FILE_PATH, document))
    {
        Log.warning("RestartHistory: unable to read persistent history");
        return false;
    }

    JsonObjectConst root = document.as<JsonObjectConst>();

    if ((root["version"] | 0) != FORMAT_VERSION)
    {
        Log.warning("RestartHistory: unsupported persistent history version");
        return false;
    }

    const uint8_t capacity = RestartHistoryData::CAPACITY;
    const uint8_t storedHead = root["head"] | 0;
    const uint8_t storedCount = root["count"] | 0;

    history.head = storedHead < capacity ? storedHead : 0;
    history.count = storedCount <= capacity ? storedCount : 0;
    history.nextId = root["nextId"] | 1UL;
    history.total = root["total"] | 0UL;
    history.succeeded = root["succeeded"] | 0UL;
    history.failed = root["failed"] | 0UL;
    history.lastStartedAt = root["lastStartedAt"] | 0ULL;
    history.lastCompletedAt = root["lastCompletedAt"] | 0ULL;
    history.lastFailedAt = root["lastFailedAt"] | 0ULL;

    JsonArrayConst entries = root["entries"].as<JsonArrayConst>();
    uint32_t greatestId = 0;

    for (uint8_t index = 0; index < capacity; ++index)
    {
        JsonObjectConst source = entries[index].as<JsonObjectConst>();

        if (source.isNull())
        {
            continue;
        }

        RestartHistoryEntry& entry = history.entries[index];

        entry.id = source["id"] | 0UL;

        if (entry.id == 0)
        {
            continue;
        }

        const uint8_t reason = source["reason"] | 0;
        const uint8_t result = source["result"] | 0;

        entry.reason = validReason(reason)
            ? static_cast<RestartReason>(reason)
            : RestartReason::Unknown;
        entry.result = validResult(result)
            ? static_cast<RestartResult>(result)
            : RestartResult::None;
        entry.startedAt = source["startedAt"] | 0ULL;
        entry.completedAt = source["completedAt"] | 0ULL;
        entry.powerOffAt = source["powerOffAt"] | 0ULL;
        entry.powerOnAt = source["powerOnAt"] | 0ULL;
        entry.startedAtEpoch = source["startedAtEpoch"] | 0ULL;
        entry.completedAtEpoch = source["completedAtEpoch"] | 0ULL;
        entry.powerOffAtEpoch = source["powerOffAtEpoch"] | 0ULL;
        entry.powerOnAtEpoch = source["powerOnAtEpoch"] | 0ULL;
        entry.requestedPowerOffTime = source["requestedPowerOffTime"] | 0UL;
        entry.actualDuration = source["actualDuration"] | 0UL;
        entry.controllerAvailableAtStart =
            source["controllerAvailableAtStart"] | false;

        if (entry.id > greatestId)
        {
            greatestId = entry.id;
        }
    }

    if (history.nextId == 0 || history.nextId <= greatestId)
    {
        history.nextId = greatestId + 1;
    }

    if (history.total < history.count)
    {
        history.total = history.count;
    }

    Log.info(
        "RestartHistory: restored %u entries",
        static_cast<unsigned int>(history.count));

    return true;
}

bool RestartHistoryStorage::save(const RestartHistoryData& history) const
{
    JsonDocument document;
    JsonObject root = document.to<JsonObject>();

    root["version"] = FORMAT_VERSION;
    root["head"] = history.head;
    root["count"] = history.count;
    root["nextId"] = history.nextId;
    root["total"] = history.total;
    root["succeeded"] = history.succeeded;
    root["failed"] = history.failed;
    root["lastStartedAt"] = history.lastStartedAt;
    root["lastCompletedAt"] = history.lastCompletedAt;
    root["lastFailedAt"] = history.lastFailedAt;

    JsonArray entries = root["entries"].to<JsonArray>();

    for (uint8_t index = 0;
         index < RestartHistoryData::CAPACITY;
         ++index)
    {
        const RestartHistoryEntry& entry = history.entries[index];
        JsonObject target = entries.add<JsonObject>();

        target["id"] = entry.id;
        target["reason"] = static_cast<uint8_t>(entry.reason);
        target["result"] = static_cast<uint8_t>(entry.result);
        target["startedAt"] = entry.startedAt;
        target["completedAt"] = entry.completedAt;
        target["powerOffAt"] = entry.powerOffAt;
        target["powerOnAt"] = entry.powerOnAt;
        target["startedAtEpoch"] = entry.startedAtEpoch;
        target["completedAtEpoch"] = entry.completedAtEpoch;
        target["powerOffAtEpoch"] = entry.powerOffAtEpoch;
        target["powerOnAtEpoch"] = entry.powerOnAtEpoch;
        target["requestedPowerOffTime"] = entry.requestedPowerOffTime;
        target["actualDuration"] = entry.actualDuration;
        target["controllerAvailableAtStart"] =
            entry.controllerAvailableAtStart;
    }

    if (!Storage.writeJson(FILE_PATH, document))
    {
        Log.warning("RestartHistory: unable to save persistent history");
        return false;
    }

    return true;
}
