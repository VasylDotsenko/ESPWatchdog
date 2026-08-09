#pragma once

#include <Arduino.h>
#include <ESP8266WebServer.h>

namespace WebApiPower
{
    void handleOn(
        ESP8266WebServer& server);

    void handleOff(
        ESP8266WebServer& server);

    void handleRestart(
        ESP8266WebServer& server,
        char* jsonBuffer,
        size_t jsonBufferSize);
}
