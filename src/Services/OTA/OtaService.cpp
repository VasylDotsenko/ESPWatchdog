#include "OtaService.h"

#include <ArduinoOTA.h>

#include "Core/Version.h"
#include "Services/Config/Config.h"
#include "Services/Logger/Logger.h"
#include "Services/WiFi/WiFiService.h"

OtaService OTA;

namespace
{
    uint8_t g_lastProgress = 255;
}

bool OtaService::begin()
{
    m_started = false;
    m_updating = false;

    Log.info("OTAService: ready");

    return true;
}

void OtaService::loop()
{
    if (Network.setupMode())
    {
        return;
    }

    if (!Network.isConnected())
    {
        return;
    }

    if (!m_started &&
        !start())
    {
        return;
    }

    ArduinoOTA.handle();
}

bool OtaService::active() const
{
    return m_started;
}

bool OtaService::updating() const
{
    return m_updating;
}

void OtaService::setUpdating(
    bool updating)
{
    m_updating = updating;
}

bool OtaService::start()
{
    const AppConfig& config =
        Config.data();

    ArduinoOTA.setHostname(
        config.device.hostname);

    if (config.security.apiAuthEnabled &&
        config.security.apiToken[0] != '\0')
    {
        ArduinoOTA.setPassword(
            config.security.apiToken);
    }

    ArduinoOTA.onStart(
        [this]()
        {
            g_lastProgress = 255;

            setUpdating(true);

            Log.warning("OTA: update started");
        });

    ArduinoOTA.onEnd(
        [this]()
        {
            setUpdating(false);

            Log.info("OTA: update completed");
        });

    ArduinoOTA.onProgress(
        [](unsigned int progress, unsigned int total)
        {
            if (total == 0)
            {
                return;
            }

            const uint8_t percent =
                static_cast<uint8_t>(
                    (progress * 100U) / total);

            if (percent != g_lastProgress &&
                percent % 10 == 0)
            {
                g_lastProgress = percent;

                Log.info(
                    "OTA: progress=%u%%",
                    percent);
            }
        });

    ArduinoOTA.onError(
        [this](ota_error_t error)
        {
            setUpdating(false);

            Log.error(
                "OTA: error=%u",
                static_cast<unsigned int>(error));
        });

    ArduinoOTA.begin();

    m_started = true;

    Log.info(
        "OTAService: started, hostname=%s version=%s auth=%s",
        config.device.hostname,
        FW_VERSION_FULL,
        (config.security.apiAuthEnabled &&
         config.security.apiToken[0] != '\0')
            ? "enabled"
            : "disabled");

    return true;
}
