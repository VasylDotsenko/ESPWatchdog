#pragma once

#include <Arduino.h>

#include "TuyaCrypto.h"
#include "TuyaPacket.h"

namespace Tuya
{
    class Protocol
    {
    public:
        static constexpr uint8_t SUPPORTED_VERSION_33 = 33;
        static constexpr uint8_t UNSUPPORTED_VERSION_34 = 34;
        static constexpr uint8_t UNSUPPORTED_VERSION_35 = 35;
        static constexpr uint8_t SUPPORTED_VERSION = SUPPORTED_VERSION_33;
        static constexpr size_t VERSION_HEADER_SIZE = 15;
        static constexpr size_t MAX_JSON_SIZE = 512;
        static constexpr size_t MAX_ENCRYPTED_SIZE =
            MAX_JSON_SIZE + TuyaCrypto::AES_BLOCK_SIZE;

        Protocol() = default;

        bool begin(
            const char* deviceId,
            const char* localKey,
            uint8_t protocolVersion);

        void reset();

        [[nodiscard]]
        bool ready() const;

        [[nodiscard]]
        uint8_t protocolVersion() const;

        [[nodiscard]]
        static bool isSupportedVersion(uint8_t protocolVersion)
        {
            return protocolVersion == SUPPORTED_VERSION_33;
        }

        [[nodiscard]]
        static bool isKnownUnsupportedVersion(uint8_t protocolVersion)
        {
            return protocolVersion == UNSUPPORTED_VERSION_34 ||
                   protocolVersion == UNSUPPORTED_VERSION_35;
        }

        bool buildHeartbeat(
            uint32_t sequence,
            Packet& packet);

        bool buildStatusQuery(
            uint32_t sequence,
            Packet& packet);

        bool buildSetDps(
            uint32_t sequence,
            uint8_t dps,
            bool value,
            Packet& packet);

        bool decryptPayload(
            const Packet& packet,
            char* output,
            size_t outputCapacity,
            size_t& outputLength) const;

    private:
        bool buildEncryptedJsonPacket(
            Command command,
            uint32_t sequence,
            const char* json,
            Packet& packet);

        bool encryptJsonPayload(
            const char* json,
            uint8_t* output,
            size_t outputCapacity,
            size_t& outputLength) const;

        bool appendVersionHeader(
            uint8_t* output,
            size_t outputCapacity,
            size_t& outputLength) const;

        bool hasVersionHeader(
            const uint8_t* payload,
            size_t payloadLength) const;

        const uint8_t* skipVersionHeader(
            const uint8_t* payload,
            size_t payloadLength,
            size_t& encryptedLength) const;

    private:
        TuyaCrypto m_crypto;

        char m_deviceId[24] {};

        uint8_t m_protocolVersion = SUPPORTED_VERSION;

        bool m_ready = false;
    };
}
