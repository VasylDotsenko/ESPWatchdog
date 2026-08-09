#pragma once

#include <Arduino.h>

#include "Common.h"

//=============================================================================
// IPv4 Address
//=============================================================================

struct IPv4Address
{
    uint8_t bytes[4] {};

    void clear()
    {
        memset(bytes, 0, sizeof(bytes));
    }

    bool isValid() const
    {
        return bytes[0] != 0 ||
               bytes[1] != 0 ||
               bytes[2] != 0 ||
               bytes[3] != 0;
    }
};

//=============================================================================
// Network State
//=============================================================================

enum class NetworkState : uint8_t
{
    Disconnected = 0,

    Connecting,

    Connected,

    SetupPortal
};

//=============================================================================
// Network Configuration
//=============================================================================

struct NetworkConfiguration
{
    char hostname[Model::HOSTNAME_LENGTH] {};

    char ssid[Model::SSID_LENGTH] {};
};

//=============================================================================
// Network Address
//=============================================================================

struct NetworkAddress
{
    IPv4Address ip;

    IPv4Address gateway;

    IPv4Address subnet;

    IPv4Address dns1;

    IPv4Address dns2;
};

//=============================================================================
// Network Signal
//=============================================================================

struct NetworkSignal
{
    int32_t rssi = 0;

    uint8_t quality = 0;
};

//=============================================================================
// Network Statistics
//=============================================================================

struct NetworkStatistics
{
    uint32_t reconnectCount = 0;

    uint32_t connectTime = 0;

    uint32_t disconnectTime = 0;
};

//=============================================================================
// Network Data
//=============================================================================

struct NetworkData
{
    bool connected = false;

    NetworkState state = NetworkState::Disconnected;

    NetworkConfiguration configuration;

    NetworkAddress address;

    NetworkSignal signal;

    NetworkStatistics statistics;
};
