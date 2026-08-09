#include "WebApiConfig.h"

#include <cstring>

#include "Services/Config/Config.h"
#include "WebJsonUtils.h"
#include "WebApiResponse.h"

namespace
{
    struct ConfigApplyPolicy
    {
        const char* section = "unknown";

        const char* applyMode = "restart";

        bool requiresRestart = true;

        bool restartRecommended = true;
    };

    ConfigApplyPolicy detectApplyPolicy(
        const String& body)
    {
        if (body.indexOf("\"security\"") >= 0)
        {
            return {
                "security",
                "live",
                false,
                false
            };
        }

        if (body.indexOf("\"device\"") >= 0)
        {
            return {
                "device",
                "restart",
                true,
                true
            };
        }

        if (body.indexOf("\"wifi\"") >= 0)
        {
            return {
                "wifi",
                "restart",
                true,
                true
            };
        }

        if (body.indexOf("\"watchdog\"") >= 0)
        {
            return {
                "watchdog",
                "restart",
                true,
                true
            };
        }

        if (body.indexOf("\"relay\"") >= 0)
        {
            return {
                "relay",
                "restart",
                true,
                true
            };
        }

        if (body.indexOf("\"tuya\"") >= 0)
        {
            return {
                "tuya",
                "restart",
                true,
                true
            };
        }

        return {};
    }

    void maskSecret(
        const char* source,
        char* output,
        size_t outputSize)
    {
        if (output == nullptr ||
            outputSize == 0)
        {
            return;
        }

        output[0] = '\0';

        if (source == nullptr)
        {
            return;
        }

        const size_t length =
            strlen(source);

        const size_t capacity =
            outputSize - 1;

        const size_t copyLength =
            length < capacity
                ? length
                : capacity;

        for (size_t i = 0; i < copyLength; ++i)
        {
            if (copyLength > 4 &&
                i < (copyLength - 4))
            {
                output[i] = '*';
            }
            else
            {
                output[i] = source[i];
            }
        }

        output[copyLength] =
            '\0';
    }
}

void WebApiConfig::handleGet(
    ESP8266WebServer& server)
{
    const AppConfig& config =
        Config.data();

    char maskedPassword[WIFI_PASSWORD_LENGTH] {};
    char maskedLocalKey[TUYA_LOCAL_KEY_LENGTH] {};
    char chunk[160] {};

    maskSecret(
        config.wifi.password,
        maskedPassword,
        sizeof(maskedPassword));

    maskSecret(
        config.tuya.localKey,
        maskedLocalKey,
        sizeof(maskedLocalKey));

    WebApiResponse::applyHeaders(server);

    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(
        200,
        "application/json",
        "");

    snprintf(
        chunk,
        sizeof(chunk),
        "{\"version\":%u,\"device\":{\"hostname\":\"",
        CONFIG_VERSION);

    server.sendContent(chunk);
    WebJsonUtils::sendEscaped(server, config.device.hostname);

    server.sendContent("\"},\"wifi\":{\"ssid\":\"");
    WebJsonUtils::sendEscaped(server, config.wifi.ssid);

    snprintf(
        chunk,
        sizeof(chunk),
        "\",\"passwordSet\":%s,\"passwordMasked\":\"",
        strlen(config.wifi.password) > 0 ? "true" : "false");

    server.sendContent(chunk);
    WebJsonUtils::sendEscaped(server, maskedPassword);

    snprintf(
        chunk,
        sizeof(chunk),
        "\",\"reconnectInterval\":%lu,\"connectTimeout\":%lu}",
        static_cast<unsigned long>(config.wifi.reconnectInterval),
        static_cast<unsigned long>(config.wifi.connectTimeout));

    server.sendContent(chunk);

    server.sendContent(",\"watchdog\":{\"targetHost\":\"");
    WebJsonUtils::sendEscaped(server, config.watchdog.targetHost);

    snprintf(
        chunk,
        sizeof(chunk),
        "\",\"targetPort\":%u,\"pingInterval\":%lu,\"pingTimeout\":%lu,"
        "\"failCount\":%u",
        config.watchdog.targetPort,
        static_cast<unsigned long>(config.watchdog.pingInterval),
        static_cast<unsigned long>(config.watchdog.pingTimeout),
        config.watchdog.failCount);

    server.sendContent(chunk);

    snprintf(
        chunk,
        sizeof(chunk),
        ",\"bootDelay\":%lu,\"powerOffTime\":%lu,\"maxRestartPerDay\":%u}",
        static_cast<unsigned long>(config.watchdog.bootDelay),
        static_cast<unsigned long>(config.watchdog.powerOffTime),
        config.watchdog.maxRestartPerDay);

    server.sendContent(chunk);

    snprintf(
        chunk,
        sizeof(chunk),
        ",\"relay\":{\"enabled\":%s,\"pin\":%u,\"activeHigh\":%s}",
        config.relay.enabled ? "true" : "false",
        config.relay.pin,
        config.relay.activeHigh ? "true" : "false");

    server.sendContent(chunk);

    server.sendContent(",\"tuya\":{\"ip\":\"");
    WebJsonUtils::sendEscaped(server, config.tuya.ipAddress);

    snprintf(
        chunk,
        sizeof(chunk),
        "\",\"port\":%u,\"deviceId\":\"",
        config.tuya.port);

    server.sendContent(chunk);
    WebJsonUtils::sendEscaped(server, config.tuya.deviceId);

    snprintf(
        chunk,
        sizeof(chunk),
        "\",\"localKeySet\":%s,\"localKeyMasked\":\"",
        strlen(config.tuya.localKey) > 0 ? "true" : "false");

    server.sendContent(chunk);
    WebJsonUtils::sendEscaped(server, maskedLocalKey);

    snprintf(
        chunk,
        sizeof(chunk),
        "\",\"version\":%u,\"relayDps\":%u,\"statusPollingEnabled\":%s,"
        "\"statusPollingInterval\":%lu}",
        config.tuya.protocolVersion,
        config.tuya.relayDps,
        config.tuya.statusPollingEnabled ? "true" : "false",
        static_cast<unsigned long>(config.tuya.statusPollingInterval));

    server.sendContent(chunk);

    snprintf(
        chunk,
        sizeof(chunk),
        ",\"security\":{\"apiAuthEnabled\":%s,\"apiTokenSet\":%s}}",
        config.security.apiAuthEnabled ? "true" : "false",
        strlen(config.security.apiToken) > 0 ? "true" : "false");

    server.sendContent(chunk);
    server.sendContent("");
}

void WebApiConfig::handleUpdate(
    ESP8266WebServer& server,
    char* jsonBuffer,
    size_t jsonBufferSize)
{
    if (!server.hasArg("plain"))
    {
        WebApiResponse::sendJson(
            server,
            400,
            "{\"ok\":false,\"error\":\"empty_body\"}");
        return;
    }

    if (jsonBuffer == nullptr ||
        jsonBufferSize == 0)
    {
        WebApiResponse::sendJson(
            server,
            500,
            "{\"ok\":false,\"error\":\"internal_buffer_unavailable\"}");
        return;
    }

    const String body =
        server.arg("plain");

    const ConfigApplyPolicy policy =
        detectApplyPolicy(body);

    char error[48] {};

    if (!Config.updateFromJson(
            body.c_str(),
            body.length(),
            error,
            sizeof(error)))
    {
        snprintf(
            jsonBuffer,
            jsonBufferSize,
            "{\"ok\":false,\"error\":\"%s\"}",
            error[0] != '\0' ? error : "update_failed");

        WebApiResponse::sendJson(
            server,
            400,
            jsonBuffer);
        return;
    }

    snprintf(
        jsonBuffer,
        jsonBufferSize,
        "{\"ok\":true,\"message\":\"configuration_updated\","
        "\"section\":\"%s\",\"applyMode\":\"%s\","
        "\"requiresRestart\":%s,\"restartRecommended\":%s}",
        policy.section,
        policy.applyMode,
        policy.requiresRestart ? "true" : "false",
        policy.restartRecommended ? "true" : "false");

    WebApiResponse::sendJson(
        server,
        200,
        jsonBuffer);
}
