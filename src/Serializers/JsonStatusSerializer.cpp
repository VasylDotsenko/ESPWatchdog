#include "JsonStatusSerializer.h"

#include <cstdio>

namespace
{
    const char* networkStateText(NetworkState state)
    {
        switch (state)
        {
            case NetworkState::Connecting:
                return "connecting";

            case NetworkState::Connected:
                return "connected";

            case NetworkState::Disconnected:
            default:
                return "disconnected";
        }
    }

    const char* healthStatusText(HealthCheckStatus status)
    {
        switch (status)
        {
            case HealthCheckStatus::Success:
                return "success";

            case HealthCheckStatus::Timeout:
                return "timeout";

            case HealthCheckStatus::HostUnreachable:
                return "host_unreachable";

            case HealthCheckStatus::NetworkUnavailable:
                return "network_unavailable";

            case HealthCheckStatus::DnsFailed:
                return "dns_failed";

            case HealthCheckStatus::Cancelled:
                return "cancelled";

            case HealthCheckStatus::Error:
            default:
                return "error";
        }
    }

    const char* watchdogStateText(WatchdogState state)
    {
        switch (state)
        {
            case WatchdogState::Monitoring:
                return "monitoring";

            case WatchdogState::RestartRequired:
                return "restart_required";

            case WatchdogState::Cooldown:
                return "cooldown";

            case WatchdogState::LockedOut:
                return "locked_out";

            case WatchdogState::Idle:
            default:
                return "idle";
        }
    }

    const char* powerStateText(PowerState state)
    {
        switch (state)
        {
            case PowerState::Idle:
                return "idle";

            case PowerState::PowerOff:
                return "power_off";

            case PowerState::WaitingPowerOn:
                return "waiting_power_on";

            case PowerState::Error:
                return "error";

            case PowerState::Disabled:
            default:
                return "disabled";
        }
    }

    const char* restartResultText(RestartResult result)
    {
        switch (result)
        {
            case RestartResult::InProgress:
                return "in_progress";

            case RestartResult::Success:
                return "success";

            case RestartResult::Failed:
                return "failed";

            case RestartResult::None:
            default:
                return "none";
        }
    }

    const char* restartReasonText(RestartReason reason)
    {
        switch (reason)
        {
            case RestartReason::WatchdogFailure:
                return "watchdog_failure";

            case RestartReason::ManualCommand:
                return "manual_command";

            case RestartReason::ControllerUnavailable:
                return "controller_unavailable";

            case RestartReason::PowerOffFailed:
                return "power_off_failed";

            case RestartReason::PowerOnFailed:
                return "power_on_failed";

            case RestartReason::PowerOnTimeout:
                return "power_on_timeout";

            case RestartReason::Unknown:
            default:
                return "unknown";
        }
    }
}

bool JsonStatusSerializer::serialize(
    const ApiStatusData& status,
    char* output,
    size_t outputCapacity,
    size_t& outputLength)
{
    outputLength = 0;

    if (output == nullptr ||
        outputCapacity == 0)
    {
        return false;
    }

    JsonDocument doc;

    JsonObject root =
        doc.to<JsonObject>();

    writeSystem(
        root["system"].to<JsonObject>(),
        status.system);

    writeNetwork(
        root["network"].to<JsonObject>(),
        status.network);

    writeHealth(
        root["health"].to<JsonObject>(),
        status.health);

    writeWatchdog(
        root["watchdog"].to<JsonObject>(),
        status.watchdog);

    writePower(
        root["power"].to<JsonObject>(),
        status.power);

    return finish(
        doc,
        output,
        outputCapacity,
        outputLength);
}

bool JsonStatusSerializer::serializeSystem(
    const SystemStatusData& status,
    char* output,
    size_t outputCapacity,
    size_t& outputLength)
{
    JsonDocument doc;

    writeSystem(
        doc.to<JsonObject>(),
        status);

    return finish(doc, output, outputCapacity, outputLength);
}

bool JsonStatusSerializer::serializeNetwork(
    const NetworkStatusData& status,
    char* output,
    size_t outputCapacity,
    size_t& outputLength)
{
    JsonDocument doc;

    writeNetwork(
        doc.to<JsonObject>(),
        status);

    return finish(doc, output, outputCapacity, outputLength);
}

bool JsonStatusSerializer::serializeHealth(
    const HealthStatusData& status,
    char* output,
    size_t outputCapacity,
    size_t& outputLength)
{
    JsonDocument doc;

    writeHealth(
        doc.to<JsonObject>(),
        status);

    return finish(doc, output, outputCapacity, outputLength);
}

bool JsonStatusSerializer::serializeWatchdog(
    const WatchdogStatusData& status,
    char* output,
    size_t outputCapacity,
    size_t& outputLength)
{
    JsonDocument doc;

    writeWatchdog(
        doc.to<JsonObject>(),
        status);

    return finish(doc, output, outputCapacity, outputLength);
}

bool JsonStatusSerializer::serializePower(
    const PowerStatusData& status,
    char* output,
    size_t outputCapacity,
    size_t& outputLength)
{
    JsonDocument doc;

    writePower(
        doc.to<JsonObject>(),
        status);

    return finish(doc, output, outputCapacity, outputLength);
}

bool JsonStatusSerializer::finish(
    JsonDocument& doc,
    char* output,
    size_t outputCapacity,
    size_t& outputLength)
{
    outputLength = 0;

    if (output == nullptr ||
        outputCapacity == 0)
    {
        return false;
    }

    outputLength =
        serializeJson(
            doc,
            output,
            outputCapacity);

    if (outputLength == 0 ||
        outputLength >= outputCapacity)
    {
        output[0] = '\0';
        outputLength = 0;
        return false;
    }

    return true;
}

void JsonStatusSerializer::writeSystem(
    JsonObject target,
    const SystemStatusData& status)
{
    JsonObject firmware =
        target["firmware"].to<JsonObject>();

    firmware["version"] = status.firmware.version;
    firmware["sdk"] = status.firmware.sdkVersion;
    firmware["core"] = status.firmware.coreVersion;
    firmware["buildDate"] = status.firmware.buildDate;
    firmware["buildTime"] = status.firmware.buildTime;
    firmware["resetReason"] = status.firmware.resetReason;

    JsonObject uptime =
        target["uptime"].to<JsonObject>();

    uptime["milliseconds"] =
        static_cast<uint64_t>(status.uptime.milliseconds);
    uptime["seconds"] =
        static_cast<uint64_t>(status.uptime.seconds);
    uptime["days"] = status.uptime.days;
    uptime["hours"] = status.uptime.hours;
    uptime["minutes"] = status.uptime.minutes;
    uptime["secondsPart"] = status.uptime.secondsPart;

    JsonObject memory =
        target["memory"].to<JsonObject>();

    memory["freeHeap"] = status.memory.freeHeap;
    memory["heapFragmentation"] =
        status.memory.heapFragmentation;
    memory["sketchSize"] = status.memory.sketchSize;
    memory["freeSketchSpace"] =
        status.memory.freeSketchSpace;
    memory["flashChipSize"] =
        status.memory.flashChipSize;

    JsonObject cpu =
        target["cpu"].to<JsonObject>();

    cpu["chipId"] = status.cpu.chipId;
    cpu["frequencyMHz"] = status.cpu.frequencyMHz;
}

void JsonStatusSerializer::writeNetwork(
    JsonObject target,
    const NetworkStatusData& status)
{
    JsonObject summary =
        target["summary"].to<JsonObject>();

    summary["connected"] = status.summary.connected;
    summary["state"] =
        static_cast<uint8_t>(status.summary.state);
    summary["stateText"] =
        networkStateText(status.summary.state);

    JsonObject configuration =
        target["configuration"].to<JsonObject>();

    configuration["hostname"] =
        status.configuration.hostname;
    configuration["ssid"] =
        status.configuration.ssid;

    JsonObject address =
        target["address"].to<JsonObject>();

    writeIPv4(address, "ip", status.address.ip);
    writeIPv4(address, "gateway", status.address.gateway);
    writeIPv4(address, "subnet", status.address.subnet);
    writeIPv4(address, "dns1", status.address.dns1);
    writeIPv4(address, "dns2", status.address.dns2);

    JsonObject signal =
        target["signal"].to<JsonObject>();

    signal["rssi"] = status.signal.rssi;
    signal["quality"] = status.signal.quality;

    JsonObject statistics =
        target["statistics"].to<JsonObject>();

    statistics["reconnectCount"] =
        status.statistics.reconnectCount;
    statistics["connectTime"] =
        status.statistics.connectTime;
    statistics["disconnectTime"] =
        status.statistics.disconnectTime;
}

void JsonStatusSerializer::writeHealth(
    JsonObject target,
    const HealthStatusData& status)
{
    JsonObject summary =
        target["summary"].to<JsonObject>();

    summary["available"] = status.summary.available;
    summary["running"] = status.summary.running;
    summary["lastStatus"] =
        static_cast<uint8_t>(status.summary.lastStatus);
    summary["lastStatusText"] =
        healthStatusText(status.summary.lastStatus);
    summary["responseTime"] =
        status.summary.responseTime;

    JsonObject statistics =
        target["statistics"].to<JsonObject>();

    statistics["sent"] = status.statistics.sent;
    statistics["received"] = status.statistics.received;
    statistics["lost"] = status.statistics.lost;
    statistics["consecutiveSuccess"] =
        status.statistics.consecutiveSuccess;
    statistics["consecutiveFails"] =
        status.statistics.consecutiveFails;
    statistics["minResponseTime"] =
        status.statistics.minResponseTime;
    statistics["maxResponseTime"] =
        status.statistics.maxResponseTime;

    JsonObject timestamps =
        target["timestamps"].to<JsonObject>();

    timestamps["lastCheck"] =
        static_cast<uint64_t>(status.timestamps.lastCheck);
    timestamps["lastSuccess"] =
        static_cast<uint64_t>(status.timestamps.lastSuccess);
    timestamps["lastFail"] =
        static_cast<uint64_t>(status.timestamps.lastFail);
    timestamps["availabilityChanged"] =
        static_cast<uint64_t>(
            status.timestamps.availabilityChanged);
}

void JsonStatusSerializer::writeWatchdog(
    JsonObject target,
    const WatchdogStatusData& status)
{
    JsonObject summary =
        target["summary"].to<JsonObject>();

    summary["state"] =
        static_cast<uint8_t>(status.summary.state);
    summary["stateText"] =
        watchdogStateText(status.summary.state);
    summary["enabled"] = status.summary.enabled;
    summary["restartPending"] =
        status.summary.restartPending;
    summary["restartRequired"] =
        status.summary.restartRequired;
    summary["lockedOut"] = status.summary.lockedOut;
    summary["cooldown"] = status.summary.cooldown;
    summary["consecutiveFailures"] =
        status.summary.consecutiveFailures;

    JsonObject configuration =
        target["configuration"].to<JsonObject>();

    configuration["failureThreshold"] =
        status.configuration.failureThreshold;
    configuration["bootDelay"] =
        status.configuration.bootDelay;
    configuration["powerOffTime"] =
        status.configuration.powerOffTime;
    configuration["maxRestartPerDay"] =
        status.configuration.maxRestartPerDay;

    JsonObject statistics =
        target["statistics"].to<JsonObject>();

    statistics["restartCount"] =
        status.statistics.restartCount;
    statistics["lastSuccess"] =
        static_cast<uint64_t>(status.statistics.lastSuccess);
    statistics["lastFailure"] =
        static_cast<uint64_t>(status.statistics.lastFailure);
    statistics["lastRestart"] =
        static_cast<uint64_t>(status.statistics.lastRestart);
    statistics["lockedOutAt"] =
        static_cast<uint64_t>(status.statistics.lockedOutAt);
}

void JsonStatusSerializer::writePower(
    JsonObject target,
    const PowerStatusData& status)
{
    JsonObject summary =
        target["summary"].to<JsonObject>();

    summary["state"] =
        static_cast<uint8_t>(status.summary.state);
    summary["stateText"] =
        powerStateText(status.summary.state);
    summary["available"] = status.summary.available;
    summary["restartInProgress"] =
        status.summary.restartInProgress;
    summary["restartCompleted"] =
        status.summary.restartCompleted;
    summary["lastOperationSucceeded"] =
        status.summary.lastOperationSucceeded;
    summary["activeRestartId"] =
        status.summary.activeRestartId;
    summary["powerOffTime"] =
        status.summary.powerOffTime;

    JsonObject statistics =
        target["statistics"].to<JsonObject>();

    statistics["restartCount"] =
        status.statistics.restartCount;
    statistics["errorCount"] =
        status.statistics.errorCount;
    statistics["lastPowerOn"] =
        static_cast<uint64_t>(status.statistics.lastPowerOn);
    statistics["lastPowerOff"] =
        static_cast<uint64_t>(status.statistics.lastPowerOff);
    statistics["lastRestart"] =
        static_cast<uint64_t>(status.statistics.lastRestart);
    statistics["lastError"] =
        static_cast<uint64_t>(status.statistics.lastError);

    writeRestartHistory(
        target["history"].to<JsonObject>(),
        status.history);
}

void JsonStatusSerializer::writeRestartHistory(
    JsonObject target,
    const RestartHistoryStatus& history)
{
    target["capacity"] = history.capacity;
    target["count"] = history.count;
    target["head"] = history.head;
    target["total"] = history.total;
    target["succeeded"] = history.succeeded;
    target["failed"] = history.failed;
    target["lastStartedAt"] =
        static_cast<uint64_t>(history.lastStartedAt);
    target["lastCompletedAt"] =
        static_cast<uint64_t>(history.lastCompletedAt);
    target["lastFailedAt"] =
        static_cast<uint64_t>(history.lastFailedAt);

    JsonArray entries =
        target["entries"].to<JsonArray>();

    const uint8_t capacity = RestartHistoryData::CAPACITY;
    const uint8_t count =
        history.count > capacity
            ? capacity
            : history.count;

    const uint8_t start =
        history.count < capacity
            ? 0
            : history.head;

    for (uint8_t i = 0; i < count; ++i)
    {
        const uint8_t index =
            static_cast<uint8_t>(
                (start + i) % capacity);

        const RestartHistoryEntry& entry =
            history.entries[index];

        if (entry.id == 0)
        {
            continue;
        }

        JsonObject item =
            entries.add<JsonObject>();

        item["id"] = entry.id;
        item["reason"] =
            static_cast<uint8_t>(entry.reason);
        item["reasonText"] =
            restartReasonText(entry.reason);
        item["result"] =
            static_cast<uint8_t>(entry.result);
        item["resultText"] =
            restartResultText(entry.result);
        item["startedAt"] =
            static_cast<uint64_t>(entry.startedAt);
        item["completedAt"] =
            static_cast<uint64_t>(entry.completedAt);
        item["powerOffAt"] =
            static_cast<uint64_t>(entry.powerOffAt);
        item["powerOnAt"] =
            static_cast<uint64_t>(entry.powerOnAt);
        item["requestedPowerOffTime"] =
            entry.requestedPowerOffTime;
        item["actualDuration"] =
            entry.actualDuration;
        item["controllerAvailableAtStart"] =
            entry.controllerAvailableAtStart;
    }
}

void JsonStatusSerializer::writeIPv4(
    JsonObject target,
    const char* key,
    const IPv4Address& address)
{
    if (key == nullptr)
    {
        return;
    }

    char buffer[16] {};

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%u.%u.%u.%u",
        address.bytes[0],
        address.bytes[1],
        address.bytes[2],
        address.bytes[3]);

    target[key] = buffer;
}
