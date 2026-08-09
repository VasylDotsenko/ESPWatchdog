#pragma once

#include <ESP8266WiFi.h>

#include "Core/IService.h"
#include "Core/Timer.h"
#include "TuyaPacket.h"
#include "TuyaProtocol.h"

struct TuyaStatus
{
    bool connected = false;
    bool relayState = false;
    uint32_t reconnectCount = 0;
    uint32_t commandCount = 0;
    uint32_t errorCount = 0;
};

enum class TuyaState : uint8_t
{
    Disconnected,
    Connecting,
    Connected
};

class TuyaService final : public IService
{
public:
    static constexpr uint32_t CONNECT_TIMEOUT_MS = 5000;
    static constexpr uint32_t SESSION_RESPONSE_TIMEOUT_MS = 1200;
    static constexpr uint32_t RECONNECT_INTERVAL_MS = 10000;
    static constexpr uint32_t RECEIVE_TIMEOUT_MS = 3000;
    static constexpr size_t RECEIVE_BUFFER_SIZE = Tuya::MAX_PACKET_6699_SIZE;

    bool begin() override;
    void loop() override;

    bool connected() const
    {
        return m_state == TuyaState::Connected;
    }

    TuyaState state() const
    {
        return m_state;
    }

    const TuyaStatus& data() const
    {
        return m_status;
    }

    bool relayOn();
    bool relayOff();
    bool relaySet(bool state);
    bool relayState() const;
    bool queryStatus();

    void disconnect();

private:
    bool connect();

    bool initializeProtocol();

    uint32_t nextSequence();

    bool sendHeartbeat();

    bool sendStatusQuery();

    bool sendCommand(bool state);

    bool sendCommand35(bool state);

    bool ensureSession35();

    bool writePacket(
        const Tuya::Packet6699& packet);

    bool readPacket6699(
        Tuya::Packet6699& packet,
        uint32_t timeoutMs);

    bool receivePacket();

    bool processReceiveBuffer();

    bool processPacket(
        const uint8_t* data,
        size_t size);

    bool processPacket6699(
        const uint8_t* data,
        size_t size);

    void processJsonPayload(
        const char* json);

    void updateConnection();

private:
    WiFiClient m_client;

    Timer m_reconnectTimer;
    Timer m_receiveTimer;

    TuyaState m_state = TuyaState::Disconnected;

    TuyaStatus m_status;

    Tuya::Protocol m_protocol;

    Tuya::Packet6699 m_packet6699;

    uint8_t m_receiveBuffer[RECEIVE_BUFFER_SIZE] {};

    size_t m_receiveLength = 0;

    uint32_t m_sequence = 1;
};

extern TuyaService TuyaLan;
