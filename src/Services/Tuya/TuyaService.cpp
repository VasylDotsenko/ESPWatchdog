#include "TuyaService.h"

#include <ArduinoJson.h>
#include <cstring>

#include "Services/Config/Config.h"
#include "Services/Logger/Logger.h"

TuyaService TuyaLan;

namespace
{
    uint32_t readU32BE(const uint8_t* data)
    {
        return (static_cast<uint32_t>(data[0]) << 24) |
               (static_cast<uint32_t>(data[1]) << 16) |
               (static_cast<uint32_t>(data[2]) << 8) |
               (static_cast<uint32_t>(data[3]));
    }
}

bool TuyaService::begin()
{
    m_state = TuyaState::Disconnected;

    m_status = TuyaStatus();

    m_receiveLength = 0;
    m_sequence = 1;

    m_client.setNoDelay(true);

    initializeProtocol();

    m_reconnectTimer.start(
        1,
        TimerMode::OneShot);

    Log.info("TuyaService started");

    return true;
}

void TuyaService::loop()
{
    updateConnection();

    if (m_state != TuyaState::Connected)
        return;

    if (!m_client.connected())
    {
        disconnect();
        return;
    }

    receivePacket();
}

bool TuyaService::connect()
{
    if (m_state == TuyaState::Connected)
        return true;

    const auto& cfg = Config.data().tuya;

    if (strlen(cfg.ipAddress) == 0)
        return false;

    if (!initializeProtocol())
    {
        m_status.errorCount++;
        Log.warning("Tuya: protocol not configured");
        return false;
    }

    Log.info("Connecting to Tuya %s:%u",
             cfg.ipAddress,
             cfg.port);

    m_state = TuyaState::Connecting;

    if (!m_client.connect(cfg.ipAddress, cfg.port))
    {
        m_state = TuyaState::Disconnected;
        m_status.connected = false;
        m_status.reconnectCount++;

        Log.warning("Unable to connect to Tuya");

        return false;
    }

    m_state = TuyaState::Connected;
    m_status.connected = true;
    m_receiveLength = 0;

    m_receiveTimer.start(
        RECEIVE_TIMEOUT_MS,
        TimerMode::OneShot);

    Log.info("Tuya connected");

    return true;
}

void TuyaService::disconnect()
{
    if (m_client.connected())
        m_client.stop();

    m_state = TuyaState::Disconnected;
    m_status.connected = false;
    m_receiveLength = 0;

    m_reconnectTimer.start(
        RECONNECT_INTERVAL_MS,
        TimerMode::OneShot);

    Log.warning("Tuya disconnected");
}

void TuyaService::updateConnection()
{
    if (m_state == TuyaState::Connected)
        return;

    if (!WiFi.isConnected())
        return;

    if (!m_reconnectTimer.expired())
        return;

    m_reconnectTimer.restart();

    connect();
}

bool TuyaService::relayOn()
{
    return relaySet(true);
}

bool TuyaService::relayOff()
{
    return relaySet(false);
}

bool TuyaService::relaySet(bool state)
{
    if (!connected())
        return false;

    return sendCommand(state);
}

bool TuyaService::relayState() const
{
    return m_status.relayState;
}

bool TuyaService::sendCommand(bool state)
{
    if (!connected())
    {
        return false;
    }

    if (!m_protocol.ready() &&
        !initializeProtocol())
    {
        m_status.errorCount++;
        return false;
    }

    const auto& cfg = Config.data().tuya;

    Tuya::Packet packet;

    if (!m_protocol.buildSetDps(
            nextSequence(),
            cfg.relayDps,
            state,
            packet))
    {
        m_status.errorCount++;
        Log.warning("Tuya: unable to build relay command");
        return false;
    }

    const size_t written =
        m_client.write(
            packet.data(),
            packet.size());

    if (written != packet.size())
    {
        m_status.errorCount++;
        Log.warning("Tuya: command write failed");
        return false;
    }

    m_status.commandCount++;
    m_status.relayState = state;

    Log.info(
        "Tuya: relay command sent, state=%u",
        state ? 1 : 0);

    return true;
}

bool TuyaService::receivePacket()
{
    if (!m_client.available())
        return true;

    while (m_client.available())
    {
        if (m_receiveLength >= RECEIVE_BUFFER_SIZE)
        {
            m_receiveLength = 0;
            m_status.errorCount++;
            Log.warning("Tuya: receive buffer overflow");
            return false;
        }

        const int value = m_client.read();

        if (value < 0)
        {
            break;
        }

        m_receiveBuffer[m_receiveLength++] =
            static_cast<uint8_t>(value);
    }

    processReceiveBuffer();

    m_receiveTimer.restart();

    return true;
}

bool TuyaService::initializeProtocol()
{
    const auto& cfg = Config.data().tuya;

    return m_protocol.begin(
        cfg.deviceId,
        cfg.localKey,
        cfg.protocolVersion);
}

uint32_t TuyaService::nextSequence()
{
    return m_sequence++;
}

bool TuyaService::processReceiveBuffer()
{
    bool processed = false;

    while (m_receiveLength >= Tuya::HEADER_SIZE)
    {
        if (readU32BE(m_receiveBuffer) != Tuya::PREFIX)
        {
            memmove(
                m_receiveBuffer,
                m_receiveBuffer + 1,
                m_receiveLength - 1);

            --m_receiveLength;

            continue;
        }

        const uint32_t length =
            readU32BE(m_receiveBuffer + 12);

        if (length < Tuya::FOOTER_SIZE)
        {
            m_receiveLength = 0;
            m_status.errorCount++;
            return false;
        }

        const size_t packetSize =
            Tuya::HEADER_SIZE + static_cast<size_t>(length);

        if (packetSize > Tuya::MAX_PACKET_SIZE)
        {
            m_receiveLength = 0;
            m_status.errorCount++;
            Log.warning("Tuya: invalid packet size");
            return false;
        }

        if (m_receiveLength < packetSize)
        {
            return processed;
        }

        if (!processPacket(
                m_receiveBuffer,
                packetSize))
        {
            m_status.errorCount++;
        }

        const size_t remaining =
            m_receiveLength - packetSize;

        if (remaining > 0)
        {
            memmove(
                m_receiveBuffer,
                m_receiveBuffer + packetSize,
                remaining);
        }

        m_receiveLength = remaining;
        processed = true;
    }

    return processed;
}

bool TuyaService::processPacket(
    const uint8_t* data,
    size_t size)
{
    Tuya::Packet packet;

    if (!packet.parse(
            data,
            size))
    {
        Log.warning("Tuya: invalid packet");
        return false;
    }

    if (packet.payloadSize() == 0)
    {
        return true;
    }

    if (!m_protocol.ready())
    {
        return false;
    }

    char json[Tuya::Protocol::MAX_JSON_SIZE] {};
    size_t jsonLength = 0;

    if (!m_protocol.decryptPayload(
            packet,
            json,
            sizeof(json),
            jsonLength))
    {
        return true;
    }

    processJsonPayload(json);

    return true;
}

void TuyaService::processJsonPayload(const char* json)
{
    if (json == nullptr ||
        json[0] == '\0')
    {
        return;
    }

    JsonDocument doc;

    const DeserializationError error =
        deserializeJson(
            doc,
            json);

    if (error)
    {
        m_status.errorCount++;
        Log.warning("Tuya: invalid JSON payload");
        return;
    }

    const auto& cfg = Config.data().tuya;

    char dpsKey[4] {};

    snprintf(
        dpsKey,
        sizeof(dpsKey),
        "%u",
        cfg.relayDps);

    if (doc["dps"][dpsKey].is<bool>())
    {
        m_status.relayState =
            doc["dps"][dpsKey].as<bool>();

        Log.info(
            "Tuya: relay state=%u",
            m_status.relayState ? 1 : 0);
    }
}
