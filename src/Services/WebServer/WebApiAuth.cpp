#include "WebApiAuth.h"

#include <Arduino.h>
#include <cstring>

#include "Services/Config/Config.h"
#include "Services/Logger/Logger.h"
#include "Services/WiFi/WiFiService.h"
#include "WebApiResponse.h"

namespace
{
    bool tokenMatches(
        const char* received,
        const char* expected)
    {
        if (received == nullptr ||
            expected == nullptr ||
            received[0] == '\0' ||
            expected[0] == '\0')
        {
            return false;
        }

        const size_t receivedLength =
            strlen(received);

        const size_t expectedLength =
            strlen(expected);

        if (receivedLength != expectedLength ||
            receivedLength >= API_TOKEN_LENGTH)
        {
            return false;
        }

        uint8_t diff = 0;

        for (size_t i = 0; i < expectedLength; ++i)
        {
            const char a = received[i];
            const char b = expected[i];

            diff |= static_cast<uint8_t>(a ^ b);
        }

        return diff == 0;
    }
}

bool WebApiAuth::authorizeCommand(
    ESP8266WebServer& server)
{
    if (Network.setupMode())
    {
        return true;
    }

    const auto& security =
        Config.data().security;

    if (!security.apiAuthEnabled)
    {
        return true;
    }

    if (security.apiToken[0] == '\0')
    {
        Log.warning(
            "WebServer: command blocked, api auth enabled without token, uri=%s",
            server.uri().c_str());

        WebApiResponse::sendJson(
            server,
            403,
            "{\"ok\":false,\"error\":\"api_auth_misconfigured\"}");

        return false;
    }

    String headerToken =
        server.header("Authorization");

    if (headerToken.startsWith("Bearer "))
    {
        headerToken =
            headerToken.substring(7);
    }

    if (tokenMatches(
            headerToken.c_str(),
            security.apiToken))
    {
        return true;
    }

    Log.warning(
        "WebServer: unauthorized command, uri=%s",
        server.uri().c_str());

    WebApiResponse::sendJson(
        server,
        401,
        "{\"ok\":false,\"error\":\"unauthorized\"}");

    return false;
}
