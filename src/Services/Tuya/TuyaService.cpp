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
    m_connectedAt = 0;
    m_lastCommandAt = 0;
    m_statusPollScheduled = false;

    m_client.setNoDelay(true);

    initializeProtocol();

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

    updateStatusPolling();
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
    m_connectedAt = millis();
    m_lastCommandAt = 0;
    m_statusPollScheduled = false;

    m_receiveTimer.start(
        RECEIVE_TIMEOUT_MS,
        TimerMode::OneShot);

    scheduleStatusPoll();

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
    m_connectedAt = 0;
    m_statusPollScheduled = false;
    m_statusPollTimer.stop();
    m_protocol.reset();

    Log.warning("Tuya disconnected");
}

void TuyaService::updateConnection()
{
    if (m_state != TuyaState::Connected)
    {
        return;
    }

    if (!WiFi.isConnected())
    {
        disconnect();
    }
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
    {
        if (!connect())
        {
            return false;
        }
    }

    return sendCommand(state);
}

bool TuyaService::relayState() const
{
    return m_status.relayState;
}

bool TuyaService::queryStatus()
{
    if (!connected())
    {
        if (!connect())
        {
            return false;
        }
    }

    return sendStatusQuery();
}

bool TuyaService::sendHeartbeat()
{
    if (!connected())
    {
        return false;
    }

    Tuya::Packet packet;

    const uint32_t sequence = nextSequence();

    if (!m_protocol.buildHeartbeat(
            sequence,
            packet))
    {
        m_status.errorCount++;
        return false;
    }

    const size_t written =
        m_client.write(
            packet.data(),
            packet.size());

    if (written != packet.size())
    {
        m_status.errorCount++;
        return false;
    }

    Log.info(
        "Tuya: heartbeat sent, seq=%lu bytes=%lu",
        static_cast<unsigned long>(sequence),
        static_cast<unsigned long>(packet.size()));

    return true;
}

bool TuyaService::sendStatusQuery()
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

    if (cfg.protocolVersion == Tuya::Protocol::SUPPORTED_VERSION_35)
    {
        Log.warning(
            "Tuya: 3.5 status query is disabled until 6699 DPQuery is implemented");

        return false;
    }

    Tuya::Packet packet;

    const uint32_t sequence = nextSequence();

    if (!m_protocol.buildStatusQuery(
            sequence,
            packet))
    {
        m_status.errorCount++;
        return false;
    }

    const size_t written =
        m_client.write(
            packet.data(),
            packet.size());

    if (written != packet.size())
    {
        m_status.errorCount++;
        return false;
    }

    Log.info(
        "Tuya: status query sent, seq=%lu bytes=%lu",
        static_cast<unsigned long>(sequence),
        static_cast<unsigned long>(packet.size()));

    return true;
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

    if (cfg.protocolVersion == Tuya::Protocol::SUPPORTED_VERSION_35)
    {
        return sendCommand35(state);
    }

    Tuya::Packet packet;

    const uint32_t sequence = nextSequence();

    if (!m_protocol.buildSetDps(
            sequence,
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
    m_lastCommandAt = millis();
    scheduleStatusPoll();

    Log.info(
        "Tuya: relay command sent, seq=%lu dps=%u state=%u bytes=%lu",
        static_cast<unsigned long>(sequence),
        cfg.relayDps,
        state ? 1 : 0,
        static_cast<unsigned long>(packet.size()));

    return true;
}

bool TuyaService::sendCommand35(bool state)
{
    if (!ensureSession35())
    {
        m_status.errorCount++;
        Log.warning("Tuya: 3.5 session negotiation failed");
        return false;
    }

    const auto& cfg = Config.data().tuya;

    m_packet6699.clear();

    const uint32_t sequence = nextSequence();

    if (!m_protocol.buildSetDps(
            sequence,
            cfg.relayDps,
            state,
            m_packet6699))
    {
        m_status.errorCount++;
        Log.warning("Tuya: unable to build 3.5 relay command");
        return false;
    }

    if (!writePacket(m_packet6699))
    {
        m_status.errorCount++;
        Log.warning("Tuya: 3.5 command write failed");
        return false;
    }

    m_status.commandCount++;
    m_status.relayState = state;
    m_lastCommandAt = millis();
    scheduleStatusPoll();

    Log.info(
        "Tuya: 3.5 relay command sent, seq=%lu dps=%u state=%u bytes=%lu",
        static_cast<unsigned long>(sequence),
        cfg.relayDps,
        state ? 1 : 0,
        static_cast<unsigned long>(m_packet6699.size()));

    return true;
}

bool TuyaService::ensureSession35()
{
    if (m_protocol.sessionReady())
    {
        return true;
    }

    m_packet6699.clear();

    const uint32_t startSequence = nextSequence();

    if (!m_protocol.buildSessionStart(
            startSequence,
            m_packet6699))
    {
        return false;
    }

    if (!writePacket(m_packet6699))
    {
        return false;
    }

    yield();

    Log.info(
        "Tuya: 3.5 session start sent, seq=%lu",
        static_cast<unsigned long>(startSequence));

    m_packet6699.clear();

    if (!readPacket6699(
            m_packet6699,
            SESSION_RESPONSE_TIMEOUT_MS))
    {
        Log.warning(
            "Tuya: 3.5 session response timeout, rx=%lu connected=%u",
            static_cast<unsigned long>(m_receiveLength),
            m_client.connected() ? 1 : 0);
        return false;
    }

    if (!m_protocol.processSessionResponse(m_packet6699))
    {
        Log.warning("Tuya: 3.5 session response invalid");
        return false;
    }

    m_packet6699.clear();

    const uint32_t finishSequence = nextSequence();

    if (!m_protocol.buildSessionFinish(
            finishSequence,
            m_packet6699))
    {
        return false;
    }

    if (!writePacket(m_packet6699))
    {
        return false;
    }

    yield();

    Log.info(
        "Tuya: 3.5 session established");

    return true;
}

bool TuyaService::writePacket(
    const Tuya::Packet6699& packet)
{
    const size_t written =
        m_client.write(
            packet.data(),
            packet.size());

    return written == packet.size();
}

bool TuyaService::readPacket6699(
    Tuya::Packet6699& packet,
    uint32_t timeoutMs)
{
    const uint32_t startedAt = millis();

    while ((millis() - startedAt) < timeoutMs)
    {
        if (!m_client.connected())
        {
            disconnect();
            return false;
        }

        while (m_client.available())
        {
            if (m_receiveLength >= RECEIVE_BUFFER_SIZE)
            {
                m_receiveLength = 0;
                m_status.errorCount++;
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

        while (m_receiveLength >= Tuya::HEADER_6699_SIZE)
        {
            if (readU32BE(m_receiveBuffer) != Tuya::PREFIX_6699)
            {
                memmove(
                    m_receiveBuffer,
                    m_receiveBuffer + 1,
                    m_receiveLength - 1);

                --m_receiveLength;

                continue;
            }

            const uint32_t length =
                readU32BE(m_receiveBuffer + 14);

            const size_t packetSize =
                Tuya::HEADER_6699_SIZE +
                static_cast<size_t>(length) +
                Tuya::FOOTER_6699_SIZE;

            if (packetSize > Tuya::MAX_PACKET_6699_SIZE)
            {
                m_receiveLength = 0;
                return false;
            }

            if (m_receiveLength < packetSize)
            {
                break;
            }

            if (!packet.parse(
                    m_receiveBuffer,
                    packetSize))
            {
                m_receiveLength = 0;
                return false;
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

            Log.info(
                "Tuya: 3.5 packet received, cmd=%lu seq=%lu payload=%lu",
                static_cast<unsigned long>(
                    static_cast<uint32_t>(packet.command())),
                static_cast<unsigned long>(packet.sequence()),
                static_cast<unsigned long>(packet.encryptedPayloadSize()));

            return true;
        }

        yield();
    }

    return false;
}

void TuyaService::updateStatusPolling()
{
    const auto& cfg = Config.data().tuya;

    if (!connected() ||
        !cfg.statusPollingEnabled)
    {
        m_statusPollScheduled = false;
        return;
    }

    if (cfg.protocolVersion == Tuya::Protocol::SUPPORTED_VERSION_35)
    {
        return;
    }

    if (!m_statusPollScheduled)
    {
        scheduleStatusPoll();
        return;
    }

    if (!m_statusPollTimer.expired())
    {
        return;
    }

    const uint32_t now = millis();

    if (m_connectedAt != 0 &&
        (now - m_connectedAt) < STATUS_POLL_CONNECT_GRACE_MS)
    {
        scheduleStatusPoll();
        return;
    }

    if (m_lastCommandAt != 0 &&
        (now - m_lastCommandAt) < STATUS_POLL_COMMAND_GRACE_MS)
    {
        scheduleStatusPoll();
        return;
    }

    sendStatusQuery();

    scheduleStatusPoll();
}

void TuyaService::scheduleStatusPoll()
{
    const auto& cfg = Config.data().tuya;

    if (!cfg.statusPollingEnabled)
    {
        m_statusPollScheduled = false;
        return;
    }

    m_statusPollTimer.start(
        statusPollingInterval(),
        TimerMode::OneShot);

    m_statusPollScheduled = true;
}

uint32_t TuyaService::statusPollingInterval() const
{
    const uint32_t configured =
        Config.data().tuya.statusPollingInterval;

    if (configured < STATUS_POLL_MIN_INTERVAL_MS)
    {
        return STATUS_POLL_MIN_INTERVAL_MS;
    }

    return configured;
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

    const bool initialized =
        m_protocol.begin(
        cfg.deviceId,
        cfg.localKey,
        cfg.protocolVersion);

    if (!initialized)
    {
        if (Tuya::Protocol::isKnownUnsupportedVersion(
                cfg.protocolVersion))
        {
            Log.warning(
                "Tuya: protocol 3.%u is detected but not supported yet",
                cfg.protocolVersion % 10);
        }
        else
        {
            Log.warning(
                "Tuya: protocol initialization failed, version=%u",
                cfg.protocolVersion);
        }
    }

    return initialized;
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
        const uint32_t prefix =
            readU32BE(m_receiveBuffer);

        if (prefix == Tuya::PREFIX_6699 &&
            m_receiveLength < Tuya::HEADER_6699_SIZE)
        {
            return processed;
        }

        if (prefix != Tuya::PREFIX &&
            prefix != Tuya::PREFIX_6699)
        {
            memmove(
                m_receiveBuffer,
                m_receiveBuffer + 1,
                m_receiveLength - 1);

            --m_receiveLength;

            continue;
        }

        const bool isPacket6699 =
            prefix == Tuya::PREFIX_6699;

        const uint32_t length =
            isPacket6699
                ? readU32BE(m_receiveBuffer + 14)
                : readU32BE(m_receiveBuffer + 12);

        if (!isPacket6699 &&
            length < Tuya::FOOTER_SIZE)
        {
            m_receiveLength = 0;
            m_status.errorCount++;
            return false;
        }

        const size_t packetSize =
            isPacket6699
                ? Tuya::HEADER_6699_SIZE +
                  static_cast<size_t>(length) +
                  Tuya::FOOTER_6699_SIZE
                : Tuya::HEADER_SIZE +
                  static_cast<size_t>(length);

        if (packetSize > RECEIVE_BUFFER_SIZE)
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

        const bool ok =
            isPacket6699
                ? processPacket6699(
                    m_receiveBuffer,
                    packetSize)
                : processPacket(
                    m_receiveBuffer,
                    packetSize);

        if (!ok)
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

    Log.info(
        "Tuya: packet received, cmd=%lu seq=%lu payload=%lu",
        static_cast<unsigned long>(
            static_cast<uint32_t>(packet.command())),
        static_cast<unsigned long>(packet.sequence()),
        static_cast<unsigned long>(packet.payloadSize()));

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
        Log.warning(
            "Tuya: unable to decrypt payload, cmd=%lu seq=%lu",
            static_cast<unsigned long>(
                static_cast<uint32_t>(packet.command())),
            static_cast<unsigned long>(packet.sequence()));

        return true;
    }

    Log.info(
        "Tuya: JSON payload, length=%lu",
        static_cast<unsigned long>(jsonLength));

    processJsonPayload(json);

    return true;
}

bool TuyaService::processPacket6699(
    const uint8_t* data,
    size_t size)
{
    m_packet6699.clear();

    if (!m_packet6699.parse(
            data,
            size))
    {
        Log.warning("Tuya: invalid 3.5 packet");
        return false;
    }

    Log.info(
        "Tuya: 3.5 packet received, cmd=%lu seq=%lu payload=%lu",
        static_cast<unsigned long>(
            static_cast<uint32_t>(m_packet6699.command())),
        static_cast<unsigned long>(m_packet6699.sequence()),
        static_cast<unsigned long>(m_packet6699.encryptedPayloadSize()));

    if (!m_protocol.sessionReady())
    {
        return true;
    }

    char json[Tuya::Protocol::MAX_JSON_SIZE] {};
    size_t jsonLength = 0;

    if (!m_protocol.decryptPayload(
            m_packet6699,
            json,
            sizeof(json),
            jsonLength))
    {
        if (m_packet6699.command() == Tuya::Command::ControlNew &&
            m_packet6699.encryptedPayloadSize() == 4)
        {
            Log.info(
                "Tuya: 3.5 command ACK received, seq=%lu",
                static_cast<unsigned long>(m_packet6699.sequence()));

            return true;
        }

        Log.warning("Tuya: unable to decrypt 3.5 payload");
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

    JsonVariant dpsValue = doc["dps"][dpsKey];

    if (dpsValue.isNull())
    {
        dpsValue = doc["data"]["dps"][dpsKey];
    }

    if (dpsValue.is<bool>())
    {
        m_status.relayState =
            dpsValue.as<bool>();

        Log.info(
            "Tuya: relay state=%u",
            m_status.relayState ? 1 : 0);
    }
}
