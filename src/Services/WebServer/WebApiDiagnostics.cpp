#include "WebApiDiagnostics.h"

#include <ArduinoJson.h>

#include "Core/Application.h"
#include "Services/RuntimeGuard/RuntimeGuardService.h"
#include "WebApiResponse.h"

namespace
{
    constexpr uint32_t MIN_HEAP_WARNING_BYTES = 12000;
    constexpr uint8_t MAX_HEAP_FRAGMENTATION_WARNING = 45;
    constexpr int32_t MIN_RSSI_WARNING_DBM = -80;

    const char* levelText(
        bool error,
        bool warning)
    {
        if (error)
        {
            return "bad";
        }

        if (warning)
        {
            return "warn";
        }

        return "ok";
    }
}

void WebApiDiagnostics::handleGet(
    ESP8266WebServer& server,
    char* jsonBuffer,
    size_t jsonBufferSize)
{
    if (jsonBuffer == nullptr ||
        jsonBufferSize == 0)
    {
        WebApiResponse::sendJson(
            server,
            500,
            "{\"ok\":false,\"error\":\"diagnostics_buffer_unavailable\"}");

        return;
    }

    const ApiStatusData status =
        App.status();

    const bool heapWarning =
        status.system.memory.freeHeap < MIN_HEAP_WARNING_BYTES;

    const bool fragmentationWarning =
        status.system.memory.heapFragmentation >
        MAX_HEAP_FRAGMENTATION_WARNING;

    const bool networkWarning =
        !status.network.summary.connected;

    const bool signalWarning =
        status.network.summary.connected &&
        status.network.signal.rssi < MIN_RSSI_WARNING_DBM;

    const bool healthWarning =
        !status.health.summary.available;

    const bool watchdogWarning =
        status.watchdog.summary.restartPending ||
        status.watchdog.summary.restartRequired ||
        status.watchdog.summary.cooldown;

    const bool watchdogError =
        status.watchdog.summary.lockedOut;

    const bool powerWarning =
        !status.power.summary.available ||
        status.power.summary.restartInProgress;

    const bool powerError =
        status.power.statistics.errorCount > 0 &&
        !status.power.summary.lastOperationSucceeded;

    const bool hasError =
        watchdogError ||
        powerError;

    const bool hasWarning =
        heapWarning ||
        fragmentationWarning ||
        networkWarning ||
        signalWarning ||
        healthWarning ||
        watchdogWarning ||
        powerWarning;

    JsonDocument doc;

    doc["ok"] = !hasError;
    doc["level"] = levelText(hasError, hasWarning);

    JsonObject system =
        doc["system"].to<JsonObject>();

    system["freeHeap"] = status.system.memory.freeHeap;
    system["heapFragmentation"] =
        status.system.memory.heapFragmentation;
    system["uptimeSeconds"] =
        static_cast<unsigned long>(status.system.uptime.seconds);
    system["resetReason"] =
        status.system.firmware.resetReason;
    system["heapWarning"] = heapWarning;
    system["fragmentationWarning"] =
        fragmentationWarning;

    JsonObject network =
        doc["network"].to<JsonObject>();

    network["connected"] = status.network.summary.connected;
    network["rssi"] = status.network.signal.rssi;
    network["quality"] = status.network.signal.quality;
    network["reconnectCount"] =
        status.network.statistics.reconnectCount;
    network["warning"] =
        networkWarning || signalWarning;

    JsonObject health =
        doc["health"].to<JsonObject>();

    health["available"] = status.health.summary.available;
    health["running"] = status.health.summary.running;
    health["responseTime"] =
        status.health.summary.responseTime;
    health["sent"] = status.health.statistics.sent;
    health["lost"] = status.health.statistics.lost;
    health["consecutiveFails"] =
        status.health.statistics.consecutiveFails;
    health["warning"] = healthWarning;

    JsonObject watchdog =
        doc["watchdog"].to<JsonObject>();

    watchdog["enabled"] = status.watchdog.summary.enabled;
    watchdog["restartPending"] =
        status.watchdog.summary.restartPending;
    watchdog["lockedOut"] =
        status.watchdog.summary.lockedOut;
    watchdog["cooldown"] =
        status.watchdog.summary.cooldown;
    watchdog["restartCount"] =
        status.watchdog.statistics.restartCount;
    watchdog["warning"] =
        watchdogWarning || watchdogError;

    JsonObject power =
        doc["power"].to<JsonObject>();

    power["available"] = status.power.summary.available;
    power["restartInProgress"] =
        status.power.summary.restartInProgress;
    power["restartCount"] =
        status.power.statistics.restartCount;
    power["errorCount"] =
        status.power.statistics.errorCount;
    power["warning"] =
        powerWarning || powerError;

    const RuntimeGuardStatus guardStatus =
        RuntimeGuard.status();

    JsonObject runtimeGuard =
        doc["runtimeGuard"].to<JsonObject>();

    runtimeGuard["enabled"] =
        guardStatus.enabled;
    runtimeGuard["degraded"] =
        guardStatus.degraded;
    runtimeGuard["restartScheduled"] =
        guardStatus.restartScheduled;
    runtimeGuard["freeHeap"] =
        guardStatus.freeHeap;
    runtimeGuard["heapFragmentation"] =
        guardStatus.heapFragmentation;
    runtimeGuard["minFreeHeap"] =
        guardStatus.minFreeHeap;
    runtimeGuard["maxHeapFragmentation"] =
        guardStatus.maxHeapFragmentation;
    runtimeGuard["degradedSince"] =
        guardStatus.degradedSince;
    runtimeGuard["restartAt"] =
        guardStatus.restartAt;

    const size_t written =
        serializeJson(
            doc,
            jsonBuffer,
            jsonBufferSize);

    if (written == 0 ||
        written >= jsonBufferSize)
    {
        WebApiResponse::sendJson(
            server,
            500,
            "{\"ok\":false,\"error\":\"diagnostics_serialization_failed\"}");

        return;
    }

    WebApiResponse::sendJson(
        server,
        200,
        jsonBuffer);
}
