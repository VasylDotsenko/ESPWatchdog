#pragma once

#include <Arduino.h>

constexpr uint16_t CONFIG_VERSION = 3;

constexpr size_t HOSTNAME_LENGTH      = 32;
constexpr size_t WIFI_SSID_LENGTH     = 32;
constexpr size_t WIFI_PASSWORD_LENGTH = 64;
constexpr size_t TARGET_HOST_LENGTH   = 64;

constexpr size_t TUYA_IP_LENGTH        = 16;
constexpr size_t TUYA_DEVICE_ID_LENGTH = 24;
constexpr size_t TUYA_LOCAL_KEY_LENGTH = 17;

struct DeviceConfig
{
    char hostname[HOSTNAME_LENGTH];
};

struct WiFiConfig
{
    char ssid[WIFI_SSID_LENGTH];
    char password[WIFI_PASSWORD_LENGTH];
    uint32_t reconnectInterval;
    uint32_t connectTimeout;
};

struct WatchdogConfig
{
    char targetHost[TARGET_HOST_LENGTH];
    uint16_t targetPort;
    uint32_t pingInterval;
    uint32_t pingTimeout;
    uint8_t failCount;
    uint32_t bootDelay;
    uint32_t powerOffTime;
    uint8_t maxRestartPerDay;
};

struct RelayConfig
{
    bool enabled;
    uint8_t pin;
    bool activeHigh;
};

struct TuyaConfig
{
    char ipAddress[TUYA_IP_LENGTH];

    uint16_t port;

    char deviceId[TUYA_DEVICE_ID_LENGTH];

    char localKey[TUYA_LOCAL_KEY_LENGTH];

    uint8_t protocolVersion;

    uint8_t relayDps;

    bool statusPollingEnabled;

    uint32_t statusPollingInterval;
};

struct AppConfig
{
    DeviceConfig device;
    WiFiConfig wifi;
    WatchdogConfig watchdog;
    RelayConfig relay;
    TuyaConfig tuya;
};

class ConfigService
{
public:
    bool begin();

    bool load();

    bool save() const;

    bool updateFromJson(
        const char* json,
        size_t length,
        char* error,
        size_t errorSize);

    void reset();

    AppConfig& data();

    const AppConfig& data() const;

private:
    void setDefaults();

    bool validate() const;

    bool loadJson();

    bool saveJson() const;

    template<size_t N>
    static void copyString(char (&dst)[N], const char* src)
    {
        if (src == nullptr)
        {
            dst[0] = '\0';
            return;
        }

        strncpy(dst, src, N - 1);
        dst[N - 1] = '\0';
    }

private:
    AppConfig m_data {};
};

extern ConfigService Config;
