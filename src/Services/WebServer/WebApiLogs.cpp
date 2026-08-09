#include "WebApiLogs.h"

#include <cstdio>

#include "Services/Logger/Logger.h"
#include "WebJsonUtils.h"
#include "WebApiResponse.h"

void WebApiLogs::handleGet(
    ESP8266WebServer& server)
{
    const uint8_t count =
        Log.count();

    char chunk[96] {};

    WebApiResponse::applyHeaders(server);

    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(
        200,
        "application/json",
        "");

    snprintf(
        chunk,
        sizeof(chunk),
        "{\"capacity\":%u,\"count\":%u,\"entries\":[",
        Logger::LOG_CAPACITY,
        count);

    server.sendContent(chunk);

    for (uint8_t i = 0; i < count; ++i)
    {
        LogEntry entry;

        if (!Log.entry(
                i,
                entry))
        {
            continue;
        }

        snprintf(
            chunk,
            sizeof(chunk),
            "%s{\"timestamp\":%lu,\"level\":%u,\"levelText\":\"",
            i == 0 ? "" : ",",
            static_cast<unsigned long>(entry.timestamp),
            static_cast<unsigned int>(entry.level));

        server.sendContent(chunk);

        WebJsonUtils::sendEscaped(
            server,
            entry.levelText);

        server.sendContent("\",\"message\":\"");

        WebJsonUtils::sendEscaped(
            server,
            entry.message);

        server.sendContent("\"}");
    }

    server.sendContent("]}");
    server.sendContent("");
}
