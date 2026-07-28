#pragma once

#include <Arduino.h>

//=============================================================================
// NetworkResult
//=============================================================================

struct NetworkResult
{
    enum class Status : uint8_t
    {
        Success = 0,

        Timeout,

        DnsFailed,

        HostUnreachable,

        NetworkUnavailable,

        Cancelled,

        Error
    };

    //---------------------------------------------------------------------

    bool success = false;

    Status status = Status::Error;

    //---------------------------------------------------------------------

    uint32_t responseTime = 0;

    //---------------------------------------------------------------------

    uint32_t packetsSent = 0;

    uint32_t packetsReceived = 0;

    uint32_t packetsLost = 0;

    //---------------------------------------------------------------------

    void reset()
    {
        success = false;

        status = Status::Error;

        responseTime = 0;

        packetsSent = 0;

        packetsReceived = 0;

        packetsLost = 0;
    }
};
