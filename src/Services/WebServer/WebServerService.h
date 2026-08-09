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

    void handleApiIndex();

    void handleApiSystem();

    void handleApiNetwork();

    void handleApiHealth();

    void handleApiWatchdog();

    void handleApiPower();

    void handleApiConfig();

    void handleApiConfigUpdate();

    void handleApiSystemRestart();

    void handleApiPowerOn();

    void handleApiPowerOff();

    void handleApiPowerRestart();

    void handleApiOptions();

    void handleHealth();

    void handleConfigPage();

    void handleNotFound();

    void sendJson(
        int statusCode,
        const char* json);

    uint32_t requestedPowerOffTime();

    void maskSecret(
        const char* source,
        char* output,
        size_t outputSize) const;

private:
    ESP8266WebServer m_server { DEFAULT_PORT };

    char m_jsonBuffer[JSON_BUFFER_SIZE] {};

    bool m_restartRequested = false;

    uint32_t m_restartAt = 0;
};

extern WebServerService WebServer;
