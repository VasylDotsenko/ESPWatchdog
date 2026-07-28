#pragma once

#include <Arduino.h>

#include "Common.h"

//=============================================================================
// WiFi Configuration
//=============================================================================

struct WiFiConfiguration
{
    char ssid[Model::SSID_LENGTH] {};

    char password[Model::PASSWORD_LENGTH] {};

    char hostname[Model::HOSTNAME_LENGTH] {};
};

//=============================================================================
// Health Check Configuration
//=============================================================================

struct HealthConfiguration
{
    char host[Model::HOST_LENGTH] {};

    uint16_t port = 0;

    uint32_t interval = 10000;

    uint32_t timeout = 1000;

    uint8_t failuresBeforeReset = 3;
};

//=============================================================================
// Relay Configuration
//=============================================================================

struct RelayConfiguration
{
    uint8_t pin = 0;

    bool activeHigh = true;

    uint32_t pulseTime = 3000;
};

//=============================================================================
// Logging Configuration
//=============================================================================

struct LoggingConfiguration
{
    bool enabled = true;

    uint8_t level = 3;
};

//=============================================================================
// Device Configuration
//=============================================================================

struct DeviceConfiguration
{
    char name[Model::DEVICE_NAME_LENGTH] {};

    char location[Model::TEXT_LENGTH] {};
};

//=============================================================================
// Config Data
//=============================================================================

struct ConfigData
{
    DeviceConfiguration device;

    WiFiConfiguration wifi;

    HealthConfiguration health;

    RelayConfiguration relay;

    LoggingConfiguration logging;
};