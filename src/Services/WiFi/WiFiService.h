#pragma once

#include <Arduino.h>

#include "Core/IService.h"
#include "Core/Timer.h"
#include "Models/NetworkData.h"
#include "Models/NetworkStatusData.h"

class WiFiService final : public IService
{
public:
    bool begin() override;
    void loop() override;

    bool connect();
    void disconnect();

    bool isConnected() const;
    bool setupMode() const;
    NetworkState connectionState() const;
    const NetworkData& data() const;
    NetworkStatusData status() const;

private:
    bool startSetupPortal(const char* reason);
    void updateData();
    void clearAddressData();

    NetworkData m_data;
    Timer m_reconnectTimer;
    Timer m_connectTimeout;
    NetworkState m_state = NetworkState::Disconnected;
};

extern WiFiService Network;
