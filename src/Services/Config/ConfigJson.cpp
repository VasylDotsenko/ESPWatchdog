#include "Config.h"

#include <ArduinoJson.h>

#include "Services/Logger/Logger.h"
#include "Services/Storage/Storage.h"

namespace
{
    constexpr const char* CONFIG_FILE = "/config.json";
}

bool ConfigService::loadJson()
{
    JsonDocument doc;

    if (!Storage.readJson(CONFIG_FILE, doc))
    {
        return false;
    }

    if ((doc["version"] | 0) != CONFIG_VERSION)
    {
        Log.warning("Unsupported configuration version");
        return false;
    }

    JsonObjectConst device = doc["device"];

    copyString(
        m_data.device.hostname,
        device["hostname"] | "ESP-Watchdog");

    JsonObjectConst wifi = doc["wifi"];

    copyString(
        m_data.wifi.ssid,
        wifi["ssid"] | "");

    copyString(
        m_data.wifi.password,
        wifi["password"] | "");

    m_data.wifi.reconnectInterval =
        wifi["reconnectInterval"] | 10000;

    m_data.wifi.connectTimeout =
        wifi["connectTimeout"] | 15000;

    JsonObjectConst watchdog = doc["watchdog"];

    copyString(
        m_data.watchdog.targetHost,
        watchdog["targetHost"] | "192.168.1.100");

    m_data.watchdog.targetPort =
        watchdog["targetPort"] | 80;

    m_data.watchdog.pingInterval =
        watchdog["pingInterval"] | 5000;

    m_data.watchdog.pingTimeout =
        watchdog["pingTimeout"] | 1000;

    m_data.watchdog.failCount =
        watchdog["failCount"] | 5;

    m_data.watchdog.bootDelay =
        watchdog["bootDelay"] | 120000;

    m_data.watchdog.powerOffTime =
        watchdog["powerOffTime"] | 10000;

    m_data.watchdog.maxRestartPerDay =
        watchdog["maxRestartPerDay"] | 20;

    JsonObjectConst relay = doc["relay"];

    m_data.relay.enabled =
        relay["enabled"] | false;

    m_data.relay.pin =
        relay["pin"] | 5;

    m_data.relay.activeHigh =
        relay["activeHigh"] | true;

    //
    // Tuya
    //
    JsonObjectConst tuya = doc["tuya"];

    copyString(
        m_data.tuya.ipAddress,
        tuya["ip"] | "");

    m_data.tuya.port =
        tuya["port"] | 6668;

    copyString(
        m_data.tuya.deviceId,
        tuya["deviceId"] | "");

    copyString(
        m_data.tuya.localKey,
        tuya["localKey"] | "");

    m_data.tuya.protocolVersion =
        tuya["version"] | 33;

    m_data.tuya.relayDps =
        tuya["relayDps"] | 1;

    Log.info("Configuration loaded");

    return true;
}

bool ConfigService::saveJson() const
{
    JsonDocument doc;

    doc["version"] = CONFIG_VERSION;

    JsonObject device = doc["device"].to<JsonObject>();

    device["hostname"] = m_data.device.hostname;

    JsonObject wifi = doc["wifi"].to<JsonObject>();

    wifi["ssid"] = m_data.wifi.ssid;
    wifi["password"] = m_data.wifi.password;
    wifi["reconnectInterval"] = m_data.wifi.reconnectInterval;
    wifi["connectTimeout"] = m_data.wifi.connectTimeout;

    JsonObject watchdog = doc["watchdog"].to<JsonObject>();

    watchdog["targetHost"] = m_data.watchdog.targetHost;
    watchdog["targetPort"] = m_data.watchdog.targetPort;
    watchdog["pingInterval"] = m_data.watchdog.pingInterval;
    watchdog["pingTimeout"] = m_data.watchdog.pingTimeout;
    watchdog["failCount"] = m_data.watchdog.failCount;
    watchdog["bootDelay"] = m_data.watchdog.bootDelay;
    watchdog["powerOffTime"] = m_data.watchdog.powerOffTime;
    watchdog["maxRestartPerDay"] = m_data.watchdog.maxRestartPerDay;

    JsonObject relay = doc["relay"].to<JsonObject>();

    relay["enabled"] = m_data.relay.enabled;
    relay["pin"] = m_data.relay.pin;
    relay["activeHigh"] = m_data.relay.activeHigh;

    //
    // Tuya
    //
    JsonObject tuya = doc["tuya"].to<JsonObject>();

    tuya["ip"] = m_data.tuya.ipAddress;
    tuya["port"] = m_data.tuya.port;
    tuya["deviceId"] = m_data.tuya.deviceId;
    tuya["localKey"] = m_data.tuya.localKey;
    tuya["version"] = m_data.tuya.protocolVersion;
    tuya["relayDps"] = m_data.tuya.relayDps;

    return Storage.writeJson(CONFIG_FILE, doc);
}