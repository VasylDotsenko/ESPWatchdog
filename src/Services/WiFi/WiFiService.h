#pragma once

#include <Arduino.h>

#include "Core/IService.h"
#include "Core/Timer.h"
#include "Models/NetworkData.h"

class WiFiService final : public IService
{
public:
    bool begin() override;
    void loop() override;

    bool connect();
    void disconnect();

    bool isConnected() const;
    NetworkState connectionState() const;
    const NetworkData& data() const;

private:
    void updateData();
    void clearAddressData();

    NetworkData m_data;
    Timer m_reconnectTimer;
    Timer m_connectTimeout;
    NetworkState m_state = NetworkState::Disconnected;
};

extern WiFiService Network;
