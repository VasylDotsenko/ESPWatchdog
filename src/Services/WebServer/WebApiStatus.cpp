#include "WebApiStatus.h"

#include "Core/Application.h"
#include "Serializers/JsonStatusSerializer.h"
#include "WebApiResponse.h"

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
        WebApiResponse::sendJson(
            server,
            500,
            "{\"error\":\"status_serialization_failed\"}");

        return;
    }

    WebApiResponse::sendJson(
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
        WebApiResponse::sendJson(
            server,
            500,
            "{\"error\":\"system_serialization_failed\"}");

        return;
    }

    WebApiResponse::sendJson(
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
        WebApiResponse::sendJson(
            server,
            500,
            "{\"error\":\"network_serialization_failed\"}");

        return;
    }

    WebApiResponse::sendJson(
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
        WebApiResponse::sendJson(
            server,
            500,
            "{\"error\":\"health_serialization_failed\"}");

        return;
    }

    WebApiResponse::sendJson(
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
        WebApiResponse::sendJson(
            server,
            500,
            "{\"error\":\"watchdog_serialization_failed\"}");

        return;
    }

    WebApiResponse::sendJson(
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
        WebApiResponse::sendJson(
            server,
            500,
            "{\"error\":\"power_serialization_failed\"}");

        return;
    }

    WebApiResponse::sendJson(
        server,
        200,
        jsonBuffer);
}
