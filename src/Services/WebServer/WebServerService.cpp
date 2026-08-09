#include "WebServerService.h"

#include "WebPages.h"
#include "WebApiAuth.h"
#include "WebApiIndex.h"
#include "WebApiStatus.h"
#include "WebApiConfig.h"
#include "WebApiLogs.h"
#include "WebApiPower.h"
#include "WebApiResponse.h"

#include "Services/Logger/Logger.h"

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
    configurePageRoutes();
    configureStatusApiRoutes();
    configureConfigApiRoutes();
    configureCommandApiRoutes();
    configureUtilityRoutes();

    m_server.onNotFound(
        [this]()
        {
            handleNotFound();
        });
}

void WebServerService::configurePageRoutes()
{
    m_server.on(
        "/",
        HTTP_GET,
        [this]()
        {
            handleRoot();
        });

    m_server.on(
        "/dashboard",
        HTTP_GET,
        [this]()
        {
            handleRoot();
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
}

void WebServerService::configureStatusApiRoutes()
{
    m_server.on(
        "/api/status",
        HTTP_GET,
        [this]()
        {
            handleApiStatus();
        });

    m_server.on(
        "/api/system",
        HTTP_GET,
        [this]()
        {
            handleApiSystem();
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
}

void WebServerService::configureConfigApiRoutes()
{
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
}

void WebServerService::configureCommandApiRoutes()
{
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
}

void WebServerService::configureUtilityRoutes()
{
    m_server.on(
        "/api",
        HTTP_GET,
        [this]()
        {
            handleApiIndex();
        });

    m_server.on(
        "/health",
        HTTP_GET,
        [this]()
        {
            handleHealth();
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
    WebApiStatus::handleStatus(
        m_server,
        m_jsonBuffer,
        sizeof(m_jsonBuffer));
}

void WebServerService::handleApiIndex()
{
    WebApiIndex::handleGet(m_server);
}

void WebServerService::handleApiSystem()
{
    WebApiStatus::handleSystem(
        m_server,
        m_jsonBuffer,
        sizeof(m_jsonBuffer));
}

void WebServerService::handleApiSystemRestart()
{
    if (!WebApiAuth::authorizeCommand(m_server))
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

    WebApiResponse::sendJson(
        m_server,
        202,
        m_jsonBuffer);
}

void WebServerService::handleApiNetwork()
{
    WebApiStatus::handleNetwork(
        m_server,
        m_jsonBuffer,
        sizeof(m_jsonBuffer));
}

void WebServerService::handleApiHealth()
{
    WebApiStatus::handleHealth(
        m_server,
        m_jsonBuffer,
        sizeof(m_jsonBuffer));
}

void WebServerService::handleApiWatchdog()
{
    WebApiStatus::handleWatchdog(
        m_server,
        m_jsonBuffer,
        sizeof(m_jsonBuffer));
}

void WebServerService::handleApiPower()
{
    WebApiStatus::handlePower(
        m_server,
        m_jsonBuffer,
        sizeof(m_jsonBuffer));
}

void WebServerService::handleApiConfig()
{
    WebApiConfig::handleGet(m_server);
}

void WebServerService::handleApiConfigUpdate()
{
    if (!WebApiAuth::authorizeCommand(m_server))
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
    if (!WebApiAuth::authorizeCommand(m_server))
    {
        return;
    }

    WebApiPower::handleOn(m_server);
}

void WebServerService::handleApiPowerOff()
{
    if (!WebApiAuth::authorizeCommand(m_server))
    {
        return;
    }

    WebApiPower::handleOff(m_server);
}

void WebServerService::handleApiPowerRestart()
{
    if (!WebApiAuth::authorizeCommand(m_server))
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
    WebApiResponse::sendNoContent(m_server);
}

void WebServerService::handleHealth()
{
    WebApiResponse::sendJson(
        m_server,
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
    WebApiResponse::sendJson(
        m_server,
        404,
        "{\"error\":\"not_found\"}");
}
