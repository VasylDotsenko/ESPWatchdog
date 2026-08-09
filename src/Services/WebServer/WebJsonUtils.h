#pragma once

#include <ESP8266WebServer.h>

namespace WebJsonUtils
{
    void sendEscaped(
        ESP8266WebServer& server,
        const char* value);
}
