#pragma once

#include <ESP8266WebServer.h>

namespace WebApiAuth
{
    bool authorizeCommand(
        ESP8266WebServer& server);
}
