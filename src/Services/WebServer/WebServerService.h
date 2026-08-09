#pragma once

#include <Arduino.h>
#include <ESP8266WebServer.h>

#include "Core/IService.h"

class WebServerService final : public IService
{
public:
    static constexpr uint16_t DEFAULT_PORT = 80;
    static constexpr size_t JSON_BUFFER_SIZE = 8192;

    bool begin() override;

    void loop() override;

private:
    void configureRoutes();

    void handleRoot();

    void handleApiStatus();

    void handleApiSystem();

    void handleApiNetwork();

    void handleApiHealth();

    void handleApiWatchdog();

    void handleApiPower();

    void handleHealth();

    void handleNotFound();

    void sendJson(
        int statusCode,
        const char* json);

private:
    ESP8266WebServer m_server { DEFAULT_PORT };

    char m_jsonBuffer[JSON_BUFFER_SIZE] {};
};

extern WebServerService WebServer;
