#include "WebServerService.h"

#include "WebPages.h"
#include "WebApiConfig.h"
#include "WebApiLogs.h"
#include "WebApiPower.h"

#include <ArduinoJson.h>
#include <cstring>

#include "Core/Application.h"
#include "Services/Config/Config.h"
#include "Serializers/JsonStatusSerializer.h"
#include "Services/Logger/Logger.h"
#include "Services/Power/PowerService.h"

WebServerService WebServer;

namespace
{
    constexpr uint32_t ESP_RESTART_DELAY_MS = 2000;
    constexpr uint32_t ESP_RESTART_SETTLE_MS = 50;
}

bool WebServerService::begin()
{
    configureRoutes();

    m_server.begin();

    Log.info(
        "WebServer: started, port=%u",
        DEFAULT_PORT);

    return true;
}

void WebServerService::loop()
{
    m_server.handleClient();

    if (m_restartRequested &&
        static_cast<int32_t>(millis() - m_restartAt) >= 0)
    {
        m_restartRequested = false;

        delay(ESP_RESTART_SETTLE_MS);
        yield();
        ESP.restart();
    }
}

void WebServerService::configureRoutes()
{
    m_server.on(
        "/",
        HTTP_GET,
        [this]()
        {
            handleRoot();
        });

    m_server.on(
        "/api/status",
        HTTP_GET,
        [this]()
        {
            handleApiStatus();
        });

    m_server.on(
        "/api",
        HTTP_GET,
        [this]()
        {
            handleApiIndex();
        });

    m_server.on(
        "/api/system",
        HTTP_GET,
        [this]()
        {
            handleApiSystem();
        });

    m_server.on(
        "/api/system/restart",
        HTTP_POST,
        [this]()
        {
            handleApiSystemRestart();
        });

    m_server.on(
        "/api/system/restart",
        HTTP_OPTIONS,
        [this]()
        {
            handleApiOptions();
        });

    m_server.on(
        "/api/network",
        HTTP_GET,
        [this]()
        {
            handleApiNetwork();
        });

    m_server.on(
        "/api/health",
        HTTP_GET,
        [this]()
        {
            handleApiHealth();
        });

    m_server.on(
        "/api/watchdog",
        HTTP_GET,
        [this]()
        {
            handleApiWatchdog();
        });

    m_server.on(
        "/api/power",
        HTTP_GET,
        [this]()
        {
            handleApiPower();
        });

    m_server.on(
        "/api/config",
        HTTP_GET,
        [this]()
        {
            handleApiConfig();
        });

    m_server.on(
        "/api/logs",
        HTTP_GET,
        [this]()
        {
            handleApiLogs();
        });

    m_server.on(
        "/api/config",
        HTTP_POST,
        [this]()
        {
            handleApiConfigUpdate();
        });

    m_server.on(
        "/api/config",
        HTTP_OPTIONS,
        [this]()
        {
            handleApiOptions();
        });

    m_server.on(
        "/config/device",
        HTTP_GET,
        [this]()
        {
            handleConfigPage();
        });

    m_server.on(
        "/config/wifi",
        HTTP_GET,
        [this]()
        {
            handleConfigPage();
        });

    m_server.on(
        "/config/watchdog",
        HTTP_GET,
        [this]()
        {
            handleConfigPage();
        });

    m_server.on(
        "/config/relay",
        HTTP_GET,
        [this]()
        {
            handleConfigPage();
        });

    m_server.on(
        "/config/tuya",
        HTTP_GET,
        [this]()
        {
            handleConfigPage();
        });

    m_server.on(
        "/config/security",
        HTTP_GET,
        [this]()
        {
            handleConfigPage();
        });

    m_server.on(
        "/logs",
        HTTP_GET,
        [this]()
        {
            handleLogsPage();
        });

    m_server.on(
        "/api/power/on",
        HTTP_POST,
        [this]()
        {
            handleApiPowerOn();
        });

    m_server.on(
        "/api/power/off",
        HTTP_POST,
        [this]()
        {
            handleApiPowerOff();
        });

    m_server.on(
        "/api/power/restart",
        HTTP_POST,
        [this]()
        {
            handleApiPowerRestart();
        });

    m_server.on(
        "/api/power/on",
        HTTP_OPTIONS,
        [this]()
        {
            handleApiOptions();
        });

    m_server.on(
        "/api/power/off",
        HTTP_OPTIONS,
        [this]()
        {
            handleApiOptions();
        });

    m_server.on(
        "/api/power/restart",
        HTTP_OPTIONS,
        [this]()
        {
            handleApiOptions();
        });

    m_server.on(
        "/health",
        HTTP_GET,
        [this]()
        {
            handleHealth();
        });

    m_server.onNotFound(
        [this]()
        {
            handleNotFound();
        });
}

void WebServerService::handleRoot()
{
    m_server.sendHeader(
        "Cache-Control",
        "no-store");

    m_server.send_P(
        200,
        "text/html",
        WEB_INDEX_HTML);
}

void WebServerService::handleApiStatus()
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serialize(
            App.status(),
            m_jsonBuffer,
            sizeof(m_jsonBuffer),
            jsonLength))
    {
        sendJson(
            500,
            "{\"error\":\"status_serialization_failed\"}");

        return;
    }

    sendJson(
        200,
        m_jsonBuffer);
}

void WebServerService::handleApiIndex()
{
    sendJson(
        200,
        "{"
        "\"name\":\"ESP Watchdog API\","
        "\"version\":1,"
        "\"endpoints\":["
        "{\"method\":\"GET\",\"path\":\"/\",\"description\":\"dashboard\"},"
        "{\"method\":\"GET\",\"path\":\"/api\",\"description\":\"api index\"},"
        "{\"method\":\"GET\",\"path\":\"/api/status\",\"description\":\"aggregate status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/system\",\"description\":\"system status\"},"
        "{\"method\":\"POST\",\"path\":\"/api/system/restart\",\"description\":\"restart ESP\"},"
        "{\"method\":\"GET\",\"path\":\"/api/network\",\"description\":\"network status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/health\",\"description\":\"health status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/watchdog\",\"description\":\"watchdog status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/power\",\"description\":\"power status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/config\",\"description\":\"runtime configuration\"},"
        "{\"method\":\"GET\",\"path\":\"/api/logs\",\"description\":\"runtime logs\"},"
        "{\"method\":\"POST\",\"path\":\"/api/config\",\"description\":\"update configuration\"},"
        "{\"method\":\"GET\",\"path\":\"/config/device\",\"description\":\"device configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/config/wifi\",\"description\":\"wifi configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/config/watchdog\",\"description\":\"watchdog configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/config/relay\",\"description\":\"relay configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/config/tuya\",\"description\":\"tuya configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/config/security\",\"description\":\"security configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/logs\",\"description\":\"runtime logs page\"},"
        "{\"method\":\"POST\",\"path\":\"/api/power/on\",\"description\":\"turn power on\"},"
        "{\"method\":\"POST\",\"path\":\"/api/power/off\",\"description\":\"turn power off\"},"
        "{\"method\":\"POST\",\"path\":\"/api/power/restart\",\"description\":\"restart power output\"},"
        "{\"method\":\"GET\",\"path\":\"/health\",\"description\":\"liveness\"}"
        "]"
        "}");
}

void WebServerService::handleApiSystem()
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serializeSystem(
            App.status().system,
            m_jsonBuffer,
            sizeof(m_jsonBuffer),
            jsonLength))
    {
        sendJson(500, "{\"error\":\"system_serialization_failed\"}");
        return;
    }

    sendJson(200, m_jsonBuffer);
}

void WebServerService::handleApiSystemRestart()
{
    if (!authorizeCommand())
    {
        return;
    }

    if (!m_restartRequested)
    {
        m_restartRequested = true;
        m_restartAt = millis() + ESP_RESTART_DELAY_MS;

        Log.warning(
            "WebServer: ESP restart scheduled, delay=%lu ms",
            static_cast<unsigned long>(ESP_RESTART_DELAY_MS));
    }

    snprintf(
        m_jsonBuffer,
        sizeof(m_jsonBuffer),
        "{\"ok\":true,\"command\":\"esp_restart\",\"delayMs\":%lu}",
        static_cast<unsigned long>(ESP_RESTART_DELAY_MS));

    sendJson(202, m_jsonBuffer);
}

void WebServerService::handleApiNetwork()
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serializeNetwork(
            App.status().network,
            m_jsonBuffer,
            sizeof(m_jsonBuffer),
            jsonLength))
    {
        sendJson(500, "{\"error\":\"network_serialization_failed\"}");
        return;
    }

    sendJson(200, m_jsonBuffer);
}

void WebServerService::handleApiHealth()
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serializeHealth(
            App.status().health,
            m_jsonBuffer,
            sizeof(m_jsonBuffer),
            jsonLength))
    {
        sendJson(500, "{\"error\":\"health_serialization_failed\"}");
        return;
    }

    sendJson(200, m_jsonBuffer);
}

void WebServerService::handleApiWatchdog()
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serializeWatchdog(
            App.status().watchdog,
            m_jsonBuffer,
            sizeof(m_jsonBuffer),
            jsonLength))
    {
        sendJson(500, "{\"error\":\"watchdog_serialization_failed\"}");
        return;
    }

    sendJson(200, m_jsonBuffer);
}

void WebServerService::handleApiPower()
{
    size_t jsonLength = 0;

    if (!JsonStatusSerializer::serializePower(
            App.status().power,
            m_jsonBuffer,
            sizeof(m_jsonBuffer),
            jsonLength))
    {
        sendJson(500, "{\"error\":\"power_serialization_failed\"}");
        return;
    }

    sendJson(200, m_jsonBuffer);
}

void WebServerService::handleApiConfig()
{
    WebApiConfig::handleGet(m_server);
}

void WebServerService::handleApiConfigUpdate()
{
    if (!authorizeCommand())
    {
        return;
    }

    WebApiConfig::handleUpdate(
        m_server,
        m_jsonBuffer,
        sizeof(m_jsonBuffer));
}

void WebServerService::handleApiLogs()
{
    WebApiLogs::handleGet(m_server);
}

void WebServerService::handleApiPowerOn()
{
    if (!authorizeCommand())
    {
        return;
    }

    WebApiPower::handleOn(m_server);
}

void WebServerService::handleApiPowerOff()
{
    if (!authorizeCommand())
    {
        return;
    }

    WebApiPower::handleOff(m_server);
}

void WebServerService::handleApiPowerRestart()
{
    if (!authorizeCommand())
    {
        return;
    }

    WebApiPower::handleRestart(
        m_server,
        m_jsonBuffer,
        sizeof(m_jsonBuffer));
}

void WebServerService::handleApiOptions()
{
    sendJson(
        204,
        "");
}

bool WebServerService::authorizeCommand()
{
    const auto& security =
        Config.data().security;

    if (!security.apiAuthEnabled ||
        security.apiToken[0] == '\0')
    {
        return true;
    }

    String headerToken =
        m_server.header("Authorization");

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

    if (m_server.hasArg("token") &&
        tokenMatches(
            m_server.arg("token").c_str(),
            security.apiToken))
    {
        return true;
    }

    Log.warning(
        "WebServer: unauthorized command, uri=%s",
        m_server.uri().c_str());

    sendUnauthorized();

    return false;
}

void WebServerService::sendUnauthorized()
{
    sendJson(
        401,
        "{\"ok\":false,\"error\":\"unauthorized\"}");
}

void WebServerService::handleHealth()
{
    sendJson(
        200,
        "{\"status\":\"ok\"}");
}

void WebServerService::handleConfigPage()
{
    handleRoot();
}

void WebServerService::handleLogsPage()
{
    handleRoot();
}

void WebServerService::handleNotFound()
{
    sendJson(
        404,
        "{\"error\":\"not_found\"}");
}

void WebServerService::sendJson(
    int statusCode,
    const char* json)
{
    m_server.sendHeader(
        "Access-Control-Allow-Origin",
        "*");

    m_server.sendHeader(
        "Access-Control-Allow-Methods",
        "GET, POST, OPTIONS");

    m_server.sendHeader(
        "Access-Control-Allow-Headers",
        "Content-Type, Authorization");

    m_server.sendHeader(
        "Cache-Control",
        "no-store");

    m_server.send(
        statusCode,
        "application/json",
        json != nullptr ? json : "{}");
}

bool WebServerService::tokenMatches(
    const char* received,
    const char* expected) const
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
