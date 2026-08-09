#include "Services/WiFi/WiFiService.h"

#include <ESP8266WiFi.h>
#include <cstring>

#include "Services/Config/Config.h"
#include "Services/Logger/Logger.h"

namespace
{
void copyText(char* destination, size_t destinationSize, const char* source)
{
    if (destinationSize == 0)
    {
        return;
    }

    if (source == nullptr)
    {
        destination[0] = '\0';
        return;
    }

    std::strncpy(destination, source, destinationSize - 1);
    destination[destinationSize - 1] = '\0';
}

void copyAddress(const IPAddress& source, IPv4Address& destination)
{
    for (uint8_t index = 0; index < 4; ++index)
    {
        destination.bytes[index] = source[index];
    }
}

uint8_t signalQuality(int32_t rssi)
{
    if (rssi <= -100)
    {
        return 0;
    }

    if (rssi >= -50)
    {
        return 100;
    }

    return static_cast<uint8_t>(2 * (rssi + 100));
}
}

WiFiService Network;

bool WiFiService::begin()
{
    const AppConfig& config = Config.data();

    m_data = NetworkData {};
    copyText(
        m_data.configuration.hostname,
        sizeof(m_data.configuration.hostname),
        config.device.hostname);
    copyText(
        m_data.configuration.ssid,
        sizeof(m_data.configuration.ssid),
        config.wifi.ssid);

    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);
    WiFi.hostname(config.device.hostname);

    m_state = NetworkState::Disconnected;
    m_reconnectTimer.start(config.wifi.reconnectInterval, TimerMode::Periodic);

    return connect();
}

bool WiFiService::connect()
{
    const WiFiConfig& config = Config.data().wifi;

    if (m_state == NetworkState::Connecting ||
        m_state == NetworkState::Connected)
    {
        return true;
    }

    if (config.ssid[0] == '\0')
    {
        Log.error("WiFi: SSID is empty");
        return false;
    }

    WiFi.begin(config.ssid, config.password);
    m_connectTimeout.start(config.connectTimeout, TimerMode::OneShot);

    m_data.connected = false;
    m_data.state = NetworkState::Connecting;
    m_data.statistics.connectTime = millis();
    m_state = NetworkState::Connecting;

    Log.info("WiFi: connecting to %s", config.ssid);
    return true;
}

void WiFiService::disconnect()
{
    WiFi.disconnect(false);

    m_connectTimeout.stop();
    m_data.connected = false;
    m_data.state = NetworkState::Disconnected;
    m_data.statistics.disconnectTime = millis();
    clearAddressData();
    m_state = NetworkState::Disconnected;
}

void WiFiService::loop()
{
    switch (m_state)
    {
        case NetworkState::Disconnected:
            if (m_reconnectTimer.expired())
            {
                connect();
            }
            break;

        case NetworkState::Connecting:
            if (WiFi.status() == WL_CONNECTED)
            {
                m_connectTimeout.stop();
                m_state = NetworkState::Connected;
                m_data.state = NetworkState::Connected;
                updateData();

                const IPv4Address& ip = m_data.address.ip;
                Log.info(
                    "WiFi: connected, IP=%u.%u.%u.%u",
                    ip.bytes[0], ip.bytes[1], ip.bytes[2], ip.bytes[3]);
                break;
            }

            if (m_connectTimeout.expired())
            {
                Log.warning("WiFi: connection timeout");
                ++m_data.statistics.reconnectCount;
                disconnect();
            }
            break;

        case NetworkState::Connected:
            if (WiFi.status() != WL_CONNECTED)
            {
                Log.warning("WiFi: disconnected");
                ++m_data.statistics.reconnectCount;
                disconnect();
                break;
            }

            updateData();
            break;
    }
}

bool WiFiService::isConnected() const
{
    return m_state == NetworkState::Connected;
}

NetworkState WiFiService::connectionState() const
{
    return m_state;
}

const NetworkData& WiFiService::data() const
{
    return m_data;
}

NetworkStatusData WiFiService::status() const
{
    NetworkStatusData status;

    status.summary.connected = m_data.connected;
    status.summary.state = m_data.state;

    copyText(
        status.configuration.hostname,
        sizeof(status.configuration.hostname),
        m_data.configuration.hostname);

    copyText(
        status.configuration.ssid,
        sizeof(status.configuration.ssid),
        m_data.configuration.ssid);

    status.address.ip = m_data.address.ip;
    status.address.gateway = m_data.address.gateway;
    status.address.subnet = m_data.address.subnet;
    status.address.dns1 = m_data.address.dns1;
    status.address.dns2 = m_data.address.dns2;

    status.signal.rssi = m_data.signal.rssi;
    status.signal.quality = m_data.signal.quality;

    status.statistics.reconnectCount =
        m_data.statistics.reconnectCount;
    status.statistics.connectTime =
        m_data.statistics.connectTime;
    status.statistics.disconnectTime =
        m_data.statistics.disconnectTime;

    return status;
}

void WiFiService::updateData()
{
    m_data.connected = true;
    m_data.state = NetworkState::Connected;

    copyAddress(WiFi.localIP(), m_data.address.ip);
    copyAddress(WiFi.gatewayIP(), m_data.address.gateway);
    copyAddress(WiFi.subnetMask(), m_data.address.subnet);
    copyAddress(WiFi.dnsIP(0), m_data.address.dns1);
    copyAddress(WiFi.dnsIP(1), m_data.address.dns2);

    m_data.signal.rssi = WiFi.RSSI();
    m_data.signal.quality = signalQuality(m_data.signal.rssi);
}

void WiFiService::clearAddressData()
{
    m_data.address = NetworkAddress {};
    m_data.signal.rssi = 0;
    m_data.signal.quality = 0;
}
