#include "Config.h"

#include "Services/Logger/Logger.h"

namespace DefaultConfig
{
    constexpr char HOSTNAME[] = "ESP-Watchdog";
    constexpr char TARGET_HOST[] = "192.168.1.100";

    constexpr uint16_t TARGET_PORT = 80;
    constexpr uint32_t WIFI_RECONNECT_INTERVAL = 10000;
    constexpr uint32_t WIFI_CONNECT_TIMEOUT = 15000;
    constexpr uint32_t PING_INTERVAL = 5000;
    constexpr uint32_t PING_TIMEOUT = 1000;
    constexpr uint8_t FAIL_COUNT = 5;
    constexpr uint32_t BOOT_DELAY = 120000;
    constexpr uint32_t POWER_OFF_TIME = 10000;
    constexpr uint8_t MAX_RESTART_PER_DAY = 20;

    constexpr bool RELAY_ENABLED = false;
    constexpr uint8_t RELAY_PIN = 5;
    constexpr bool RELAY_ACTIVE_HIGH = true;

    // Tuya defaults
    constexpr char TUYA_IP[] = "";
    constexpr uint16_t TUYA_PORT = 6668;
    constexpr char TUYA_DEVICE_ID[] = "";
    constexpr char TUYA_LOCAL_KEY[] = "";
    constexpr uint8_t TUYA_PROTOCOL_VERSION = 33;
    constexpr uint8_t TUYA_RELAY_DPS = 1;
}

void ConfigService::setDefaults()
{
    memset(&m_data, 0, sizeof(m_data));

    copyString(m_data.device.hostname, DefaultConfig::HOSTNAME);

    copyString(m_data.wifi.ssid, "");
    copyString(m_data.wifi.password, "");

    m_data.wifi.reconnectInterval = DefaultConfig::WIFI_RECONNECT_INTERVAL;
    m_data.wifi.connectTimeout = DefaultConfig::WIFI_CONNECT_TIMEOUT;

    copyString(m_data.watchdog.targetHost, DefaultConfig::TARGET_HOST);

    m_data.watchdog.targetPort = DefaultConfig::TARGET_PORT;
    m_data.watchdog.pingInterval = DefaultConfig::PING_INTERVAL;
    m_data.watchdog.pingTimeout = DefaultConfig::PING_TIMEOUT;
    m_data.watchdog.failCount = DefaultConfig::FAIL_COUNT;
    m_data.watchdog.bootDelay = DefaultConfig::BOOT_DELAY;
    m_data.watchdog.powerOffTime = DefaultConfig::POWER_OFF_TIME;
    m_data.watchdog.maxRestartPerDay = DefaultConfig::MAX_RESTART_PER_DAY;

    m_data.relay.enabled = DefaultConfig::RELAY_ENABLED;
    m_data.relay.pin = DefaultConfig::RELAY_PIN;
    m_data.relay.activeHigh = DefaultConfig::RELAY_ACTIVE_HIGH;

    // Tuya
    copyString(m_data.tuya.ipAddress, DefaultConfig::TUYA_IP);
    m_data.tuya.port = DefaultConfig::TUYA_PORT;
    copyString(m_data.tuya.deviceId, DefaultConfig::TUYA_DEVICE_ID);
    copyString(m_data.tuya.localKey, DefaultConfig::TUYA_LOCAL_KEY);
    m_data.tuya.protocolVersion = DefaultConfig::TUYA_PROTOCOL_VERSION;
    m_data.tuya.relayDps = DefaultConfig::TUYA_RELAY_DPS;
}

bool ConfigService::validate() const
{
    if (strlen(m_data.device.hostname) == 0)
    {
        Log.error("Config: hostname is empty");
        return false;
    }

    if (m_data.wifi.reconnectInterval < 1000)
    {
        Log.error("Config: invalid reconnectInterval");
        return false;
    }

    if (m_data.wifi.connectTimeout < 1000)
    {
        Log.error("Config: invalid connectTimeout");
        return false;
    }

    if (strlen(m_data.watchdog.targetHost) == 0)
    {
        Log.error("Config: targetHost is empty");
        return false;
    }

    if (m_data.watchdog.targetPort == 0)
    {
        Log.error("Config: invalid targetPort");
        return false;
    }

    if (m_data.watchdog.pingInterval < 100)
    {
        Log.error("Config: invalid pingInterval");
        return false;
    }

    if (m_data.watchdog.pingTimeout < 100)
    {
        Log.error("Config: invalid pingTimeout");
        return false;
    }

    if (m_data.watchdog.failCount == 0)
    {
        Log.error("Config: invalid failCount");
        return false;
    }

    if (m_data.watchdog.bootDelay < 1000)
    {
        Log.error("Config: invalid bootDelay");
        return false;
    }

    if (m_data.watchdog.powerOffTime < 1000)
    {
        Log.error("Config: invalid powerOffTime");
        return false;
    }

    if (m_data.watchdog.maxRestartPerDay == 0)
    {
        Log.error("Config: invalid maxRestartPerDay");
        return false;
    }

    if (m_data.relay.pin > 16)
    {
        Log.error("Config: invalid relay pin");
        return false;
    }

    // Tuya validation
    if (m_data.tuya.port == 0)
    {
        Log.error("Config: invalid tuya port");
        return false;
    }

    if (m_data.tuya.protocolVersion != 33 &&
        m_data.tuya.protocolVersion != 34)
    {
        Log.error("Config: invalid tuya protocolVersion");
        return false;
    }

    if (m_data.tuya.relayDps == 0)
    {
        Log.error("Config: invalid tuya relayDps");
        return false;
    }

    return true;
}