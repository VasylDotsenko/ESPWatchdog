#pragma once

#include <Arduino.h>

#include "Models/NetworkData.h"

struct NetworkStatusSummary
{
    bool connected = false;
    NetworkState state = NetworkState::Disconnected;
};

struct NetworkStatusConfiguration
{
    char hostname[Model::HOSTNAME_LENGTH] {};
    char ssid[Model::SSID_LENGTH] {};
};

struct NetworkStatusAddress
{
    IPv4Address ip;
    IPv4Address gateway;
    IPv4Address subnet;
    IPv4Address dns1;
    IPv4Address dns2;
};

struct NetworkStatusSignal
{
    int32_t rssi = 0;
    uint8_t quality = 0;
};

struct NetworkStatusStatistics
{
    uint32_t reconnectCount = 0;
    uint32_t connectTime = 0;
    uint32_t disconnectTime = 0;
};

struct NetworkStatusData
{
    NetworkStatusSummary summary;
    NetworkStatusConfiguration configuration;
    NetworkStatusAddress address;
    NetworkStatusSignal signal;
    NetworkStatusStatistics statistics;
};
