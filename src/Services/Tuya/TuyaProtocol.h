#pragma once

#include <Arduino.h>

#include "Services/Tuya/TuyaPacket.h"

class TuyaProtocol
{
public:
    TuyaProtocol();

    void begin(const char* deviceId,
               const char* localKey,
               uint8_t protocolVersion,
               uint8_t relayDps);

    //
    // Sequence
    //
    uint32_t nextSequence();

    //
    // Packet builders
    //
    bool buildHeartbeat(Tuya::Packet& packet);

    bool buildStatusQuery(Tuya::Packet& packet);

    bool buildControl(Tuya::Packet& packet,
                      bool relayState);

    //
    // Packet parser
    //
    bool parse(const Tuya::Packet& packet);

    //
    // State
    //
    bool relayState() const
    {
        return m_relayState;
    }

private:
    bool encryptPayload(
        Tuya::Command command,
        const uint8_t* plain,
        size_t plainLength,
        Tuya::Packet& packet);

    bool decryptPayload(
        const Tuya::Packet& packet,
        uint8_t* plain,
        size_t& plainLength);

    bool parseStatusJson(
        const char* json);

    bool parseHeartbeat(
        const Tuya::Packet& packet);

private:
    char m_deviceId[25]{};
    uint8_t m_localKey[16]{};

    uint8_t m_protocolVersion = 33;
    uint8_t m_relayDps = 1;

    uint32_t m_sequence = 1;

    bool m_relayState = false;
};
