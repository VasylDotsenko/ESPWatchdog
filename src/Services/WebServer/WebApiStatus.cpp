#include "WebApiStatus.h"

#include "Core/Application.h"
#include "Serializers/JsonStatusSerializer.h"

namespace
{
    void sendJson(
        ESP8266WebServer& server,
        int statusCode,
        const char* json)
    {
        server.sendHeader(
            "Access-Control-Allow-Origin",
            "*");

        server.sendHeader(
            "Access-Control-Allow-Methods",
            "GET, POST, OPTIONS");

        server.sendHeader(
            "Access-Control-Allow-Headers",
            "Content-Type, Authorization");

        server.send(
            statusCode,
            "application/json",
            json);
    }
}

void WebApiStatus::handleStatus(
    ESP8266WebServer& server,
    char* jsonBuffer,
    size_t jsonBufferSize)
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serialize(
            App.status(),
            jsonBuffer,
            jsonBufferSize,
            jsonLength))
    {
        sendJson(
            server,
            500,
            "{\"error\":\"status_serialization_failed\"}");

        return;
    }

    sendJson(
        server,
        200,
        jsonBuffer);
}

void WebApiStatus::handleSystem(
    ESP8266WebServer& server,
    char* jsonBuffer,
    size_t jsonBufferSize)
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serializeSystem(
            App.status().system,
            jsonBuffer,
            jsonBufferSize,
            jsonLength))
    {
        sendJson(
            server,
            500,
            "{\"error\":\"system_serialization_failed\"}");

        return;
    }

    sendJson(
        server,
        200,
        jsonBuffer);
}

void WebApiStatus::handleNetwork(
    ESP8266WebServer& server,
    char* jsonBuffer,
    size_t jsonBufferSize)
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serializeNetwork(
            App.status().network,
            jsonBuffer,
            jsonBufferSize,
            jsonLength))
    {
        sendJson(
            server,
            500,
            "{\"error\":\"network_serialization_failed\"}");

        return;
    }

    sendJson(
        server,
        200,
        jsonBuffer);
}

void WebApiStatus::handleHealth(
    ESP8266WebServer& server,
    char* jsonBuffer,
    size_t jsonBufferSize)
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serializeHealth(
            App.status().health,
            jsonBuffer,
            jsonBufferSize,
            jsonLength))
    {
        sendJson(
            server,
            500,
            "{\"error\":\"health_serialization_failed\"}");

        return;
    }

    sendJson(
        server,
        200,
        jsonBuffer);
}

void WebApiStatus::handleWatchdog(
    ESP8266WebServer& server,
    char* jsonBuffer,
    size_t jsonBufferSize)
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serializeWatchdog(
            App.status().watchdog,
            jsonBuffer,
            jsonBufferSize,
            jsonLength))
    {
        sendJson(
            server,
            500,
            "{\"error\":\"watchdog_serialization_failed\"}");

        return;
    }

    sendJson(
        server,
        200,
        jsonBuffer);
}

void WebApiStatus::handlePower(
    ESP8266WebServer& server,
    char* jsonBuffer,
    size_t jsonBufferSize)
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serializePower(
            App.status().power,
            jsonBuffer,
            jsonBufferSize,
            jsonLength))
    {
        sendJson(
            server,
            500,
            "{\"error\":\"power_serialization_failed\"}");

        return;
    }

    sendJson(
        server,
        200,
        jsonBuffer);
}
