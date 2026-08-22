#include "WebApiDiagnostics.h"

#include <cstdio>

#include "Core/Application.h"
#include "Services/RuntimeGuard/RuntimeGuardService.h"
#include "WebApiResponse.h"
#include "WebJsonUtils.h"

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

    const RuntimeGuardStatus guardStatus =
        RuntimeGuard.status();

    WebApiResponse::applyHeaders(server);

    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(
        200,
        "application/json",
        "");

    snprintf(
        jsonBuffer,
        jsonBufferSize,
        "{\"ok\":%s,\"level\":\"%s\",\"system\":{\"freeHeap\":%lu,"
        "\"heapFragmentation\":%u,\"uptimeSeconds\":%lu,"
        "\"resetReason\":\"",
        hasError ? "false" : "true",
        levelText(hasError, hasWarning),
        static_cast<unsigned long>(status.system.memory.freeHeap),
        status.system.memory.heapFragmentation,
        static_cast<unsigned long>(status.system.uptime.seconds));

    server.sendContent(jsonBuffer);
    WebJsonUtils::sendEscaped(server, status.system.firmware.resetReason);

    snprintf(
        jsonBuffer,
        jsonBufferSize,
        "\",\"heapWarning\":%s,\"fragmentationWarning\":%s},"
        "\"network\":{\"connected\":%s,\"rssi\":%d,\"quality\":%u,"
        "\"reconnectCount\":%lu,\"warning\":%s},"
        "\"health\":{\"available\":%s,\"running\":%s,"
        "\"responseTime\":%lu,\"sent\":%lu,\"lost\":%lu,"
        "\"consecutiveFails\":%lu,\"warning\":%s},",
        heapWarning ? "true" : "false",
        fragmentationWarning ? "true" : "false",
        status.network.summary.connected ? "true" : "false",
        status.network.signal.rssi,
        status.network.signal.quality,
        static_cast<unsigned long>(status.network.statistics.reconnectCount),
        (networkWarning || signalWarning) ? "true" : "false",
        status.health.summary.available ? "true" : "false",
        status.health.summary.running ? "true" : "false",
        static_cast<unsigned long>(status.health.summary.responseTime),
        static_cast<unsigned long>(status.health.statistics.sent),
        static_cast<unsigned long>(status.health.statistics.lost),
        static_cast<unsigned long>(status.health.statistics.consecutiveFails),
        healthWarning ? "true" : "false");

    server.sendContent(jsonBuffer);

    snprintf(
        jsonBuffer,
        jsonBufferSize,
        "\"watchdog\":{\"enabled\":%s,\"restartPending\":%s,"
        "\"lockedOut\":%s,\"cooldown\":%s,\"restartCount\":%lu,"
        "\"warning\":%s},"
        "\"power\":{\"available\":%s,\"restartInProgress\":%s,"
        "\"restartCount\":%lu,\"errorCount\":%lu,\"warning\":%s},",
        status.watchdog.summary.enabled ? "true" : "false",
        status.watchdog.summary.restartPending ? "true" : "false",
        status.watchdog.summary.lockedOut ? "true" : "false",
        status.watchdog.summary.cooldown ? "true" : "false",
        static_cast<unsigned long>(status.watchdog.statistics.restartCount),
        (watchdogWarning || watchdogError) ? "true" : "false",
        status.power.summary.available ? "true" : "false",
        status.power.summary.restartInProgress ? "true" : "false",
        static_cast<unsigned long>(status.power.statistics.restartCount),
        static_cast<unsigned long>(status.power.statistics.errorCount),
        (powerWarning || powerError) ? "true" : "false");

    server.sendContent(jsonBuffer);

    snprintf(
        jsonBuffer,
        jsonBufferSize,
        "\"runtimeGuard\":{\"enabled\":%s,\"degraded\":%s,"
        "\"restartScheduled\":%s,\"freeHeap\":%lu,"
        "\"heapFragmentation\":%u,\"minFreeHeap\":%lu,"
        "\"maxHeapFragmentation\":%u,\"degradedSince\":%lu,"
        "\"restartAt\":%lu}}",
        guardStatus.enabled ? "true" : "false",
        guardStatus.degraded ? "true" : "false",
        guardStatus.restartScheduled ? "true" : "false",
        static_cast<unsigned long>(guardStatus.freeHeap),
        guardStatus.heapFragmentation,
        static_cast<unsigned long>(guardStatus.minFreeHeap),
        guardStatus.maxHeapFragmentation,
        static_cast<unsigned long>(guardStatus.degradedSince),
        static_cast<unsigned long>(guardStatus.restartAt));

    server.sendContent(jsonBuffer);
    server.sendContent("");
}
