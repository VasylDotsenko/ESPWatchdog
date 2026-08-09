#pragma once

#include <Arduino.h>
#include <ESP8266WebServer.h>

namespace WebApiStatus
{
    void handleStatus(
        ESP8266WebServer& server,
        char* jsonBuffer,
        size_t jsonBufferSize);

    void handleSystem(
        ESP8266WebServer& server,
        char* jsonBuffer,
        size_t jsonBufferSize);

    void handleNetwork(
        ESP8266WebServer& server,
        char* jsonBuffer,
        size_t jsonBufferSize);

    void handleHealth(
        ESP8266WebServer& server,
        char* jsonBuffer,
        size_t jsonBufferSize);

    void handleWatchdog(
        ESP8266WebServer& server,
        char* jsonBuffer,
        size_t jsonBufferSize);

    void handlePower(
        ESP8266WebServer& server,
        char* jsonBuffer,
        size_t jsonBufferSize);
}
