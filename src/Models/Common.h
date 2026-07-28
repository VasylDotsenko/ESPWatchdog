#pragma once

#include <Arduino.h>

//=============================================================================
// Model Constants
//=============================================================================

namespace Model
{
    //-------------------------------------------------------------------------
    // Generic strings
    //-------------------------------------------------------------------------

    inline constexpr size_t NAME_LENGTH = 32;

    inline constexpr size_t SHORT_TEXT_LENGTH = 32;

    inline constexpr size_t TEXT_LENGTH = 64;

    inline constexpr size_t LONG_TEXT_LENGTH = 128;

    //-------------------------------------------------------------------------
    // Network
    //-------------------------------------------------------------------------

    inline constexpr size_t HOST_LENGTH = 64;

    inline constexpr size_t IP_LENGTH = 16;

    inline constexpr size_t MAC_LENGTH = 18;

    inline constexpr size_t SSID_LENGTH = 32;

    inline constexpr size_t PASSWORD_LENGTH = 64;

    //-------------------------------------------------------------------------
    // Firmware
    //-------------------------------------------------------------------------

    inline constexpr size_t VERSION_LENGTH = 16;

    inline constexpr size_t SDK_VERSION_LENGTH = 32;

    inline constexpr size_t CORE_VERSION_LENGTH = 32;

    inline constexpr size_t BUILD_DATE_LENGTH = 16;

    inline constexpr size_t BUILD_TIME_LENGTH = 16;

    inline constexpr size_t RESET_REASON_LENGTH = 64;

    //-------------------------------------------------------------------------
    // MQTT
    //-------------------------------------------------------------------------

    inline constexpr size_t MQTT_TOPIC_LENGTH = 128;

    inline constexpr size_t MQTT_CLIENT_ID_LENGTH = 32;

    //-------------------------------------------------------------------------
    // Config
    //-------------------------------------------------------------------------

    inline constexpr size_t HOSTNAME_LENGTH = 32;

    inline constexpr size_t DEVICE_NAME_LENGTH = 32;
}