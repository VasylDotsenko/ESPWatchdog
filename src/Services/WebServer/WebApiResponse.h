#pragma once

#include <ESP8266WebServer.h>

namespace WebApiResponse
{
    void applyHeaders(
        ESP8266WebServer& server);

    void sendJson(
        ESP8266WebServer& server,
        int statusCode,
        const char* json);

    void sendNoContent(
        ESP8266WebServer& server);
}
