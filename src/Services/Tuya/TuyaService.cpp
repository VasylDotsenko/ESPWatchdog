#include "TuyaService.h"

#include "Services/Config/Config.h"
#include "Services/Logger/Logger.h"

TuyaService Tuya;

bool TuyaService::begin()
{
    m_state = TuyaState::Disconnected;

    m_status = TuyaStatus();

    m_client.setNoDelay(true);

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

    m_receiveTimer.restart();

    Log.info("Tuya connected");

    return true;
}

void TuyaService::disconnect()
{
    if (m_client.connected())
        m_client.stop();

    m_state = TuyaState::Disconnected;
    m_status.connected = false;

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
    //
    // Буде реалізовано через TuyaProtocol
    //

    (void)state;

    return false;
}

bool TuyaService::receivePacket()
{
    if (!m_client.available())
        return true;

    processPacket();

    m_receiveTimer.restart();

    return true;
}

void TuyaService::processPacket()
{
    //
    // Буде реалізовано після створення TuyaProtocol
    //
}
