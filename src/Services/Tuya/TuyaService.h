#pragma once

#include <ESP8266WiFi.h>

#include "Core/IService.h"
#include "Core/Timer.h"

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
    static constexpr uint32_t RECONNECT_INTERVAL_MS = 10000;
    static constexpr uint32_t RECEIVE_TIMEOUT_MS = 3000;

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

    void disconnect();

private:
    bool connect();

    bool sendCommand(bool state);

    bool receivePacket();

    void processPacket();

    void updateConnection();

private:
    WiFiClient m_client;

    Timer m_reconnectTimer;
    Timer m_receiveTimer;

    TuyaState m_state = TuyaState::Disconnected;

    TuyaStatus m_status;
};

extern TuyaService Tuya;

