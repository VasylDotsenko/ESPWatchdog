#include "WebApiPower.h"

#include "Services/Config/Config.h"
#include "Services/Power/PowerService.h"

namespace
{
    void sendJson(
        ESP8266WebServer& server,
        int statusCode,
        const char* json)
    {
        server.sendHeader(
            "Cache-Control",
            "no-store");

        server.sendHeader(
            "Access-Control-Allow-Origin",
            "*");

        server.send(
            statusCode,
            "application/json",
            json != nullptr ? json : "{}");
    }

    uint32_t requestedPowerOffTime(
        ESP8266WebServer& server)
    {
        uint32_t powerOffTime =
            Config.data().watchdog.powerOffTime;

        if (server.hasArg("powerOffTime"))
        {
            const uint32_t requested =
                static_cast<uint32_t>(
                    server.arg("powerOffTime").toInt());

            if (requested >= 1000 &&
                requested <= 60000)
            {
                powerOffTime = requested;
            }
        }

        return powerOffTime;
    }
}

void WebApiPower::handleOn(
    ESP8266WebServer& server)
{
    if (Power.restartInProgress())
    {
        sendJson(
            server,
            409,
            "{\"ok\":false,\"error\":\"restart_in_progress\"}");
        return;
    }

    if (!Power.available())
    {
        sendJson(
            server,
            503,
            "{\"ok\":false,\"error\":\"power_controller_unavailable\"}");
        return;
    }

    if (!Power.powerOn())
    {
        sendJson(
            server,
            500,
            "{\"ok\":false,\"error\":\"power_on_failed\"}");
        return;
    }

    sendJson(
        server,
        200,
        "{\"ok\":true,\"command\":\"power_on\"}");
}

void WebApiPower::handleOff(
    ESP8266WebServer& server)
{
    if (Power.restartInProgress())
    {
        sendJson(
            server,
            409,
            "{\"ok\":false,\"error\":\"restart_in_progress\"}");
        return;
    }

    if (!Power.available())
    {
        sendJson(
            server,
            503,
            "{\"ok\":false,\"error\":\"power_controller_unavailable\"}");
        return;
    }

    if (!Power.powerOff())
    {
        sendJson(
            server,
            500,
            "{\"ok\":false,\"error\":\"power_off_failed\"}");
        return;
    }

    sendJson(
        server,
        200,
        "{\"ok\":true,\"command\":\"power_off\"}");
}

void WebApiPower::handleRestart(
    ESP8266WebServer& server,
    char* jsonBuffer,
    size_t jsonBufferSize)
{
    if (Power.restartInProgress())
    {
        sendJson(
            server,
            409,
            "{\"ok\":false,\"error\":\"restart_in_progress\"}");
        return;
    }

    if (jsonBuffer == nullptr ||
        jsonBufferSize == 0)
    {
        sendJson(
            server,
            500,
            "{\"ok\":false,\"error\":\"internal_buffer_unavailable\"}");
        return;
    }

    const uint32_t powerOffTime =
        requestedPowerOffTime(server);

    if (!Power.restart(
            powerOffTime,
            RestartReason::ManualCommand))
    {
        sendJson(
            server,
            503,
            "{\"ok\":false,\"error\":\"restart_not_started\"}");
        return;
    }

    snprintf(
        jsonBuffer,
        jsonBufferSize,
        "{\"ok\":true,\"command\":\"restart\",\"powerOffTime\":%lu}",
        static_cast<unsigned long>(powerOffTime));

    sendJson(
        server,
        202,
        jsonBuffer);
}
