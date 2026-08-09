#pragma once

#include <Arduino.h>
#include <ESP8266WebServer.h>

namespace WebApiConfig
{
    void handleGet(
        ESP8266WebServer& server);

    void handleUpdate(
        ESP8266WebServer& server,
        char* jsonBuffer,
        size_t jsonBufferSize);
}
