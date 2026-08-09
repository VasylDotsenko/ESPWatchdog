#pragma once

#include <Arduino.h>

#include "TuyaCrypto.h"

namespace Tuya
{
    constexpr uint32_t PREFIX = 0x000055AA;
    constexpr uint32_t SUFFIX = 0x0000AA55;
    constexpr uint32_t PREFIX_6699 = 0x00006699;
    constexpr uint32_t SUFFIX_6699 = 0x00009966;

    constexpr size_t MAX_PAYLOAD_SIZE = 1024;
    constexpr size_t HEADER_SIZE = 16;
    constexpr size_t FOOTER_SIZE = 8;
    constexpr size_t HEADER_6699_SIZE = 18;
    constexpr size_t FOOTER_6699_SIZE = 4;
    constexpr size_t IV_6699_SIZE = 12;
    constexpr size_t TAG_6699_SIZE = 16;
    constexpr size_t MAX_PACKET_SIZE =
        HEADER_SIZE +
        MAX_PAYLOAD_SIZE +
        FOOTER_SIZE;
    constexpr size_t MAX_PACKET_6699_SIZE =
        HEADER_6699_SIZE +
        IV_6699_SIZE +
        MAX_PAYLOAD_SIZE +
        TAG_6699_SIZE +
        FOOTER_6699_SIZE;

    enum class Command : uint32_t
    {
        HeartBeat      = 9,
        Query          = 10,
        Control        = 7,
        Status         = 8,
        ControlNew     = 13,
        DPQueryNew     = 16,
        DPQuery        = 10,
        Updatedps      = 18,
        SessionKeyStart  = 3,
        SessionKeyResp   = 4,
        SessionKeyFinish = 5
    };

    struct Header6699
    {
        uint32_t prefix;
        uint16_t reserved;
        uint32_t sequence;
        uint32_t command;
        uint32_t length;
    };

    struct Header
    {
        uint32_t prefix;
        uint32_t sequence;
        uint32_t command;
        uint32_t length;
    };

    struct Footer
    {
        uint32_t crc32;
        uint32_t suffix;
    };

    class Packet
    {
    public:
        Packet();

        void clear();

        bool parse(const uint8_t* data, size_t size);

        bool build(Command command,
                   uint32_t sequence,
                   const uint8_t* payload,
                   size_t payloadLength);

        const uint8_t* data() const
        {
            return m_buffer;
        }

        size_t size() const
        {
            return m_size;
        }

        Command command() const
        {
            return static_cast<Command>(m_header.command);
        }

        uint32_t sequence() const
        {
            return m_header.sequence;
        }

        const uint8_t* payload() const
        {
            return m_payload;
        }

        size_t payloadSize() const
        {
            return m_payloadLength;
        }

    private:
        Header m_header{};
        Footer m_footer{};

        uint8_t m_buffer[MAX_PACKET_SIZE];

        uint8_t* m_payload = nullptr;

        size_t m_payloadLength = 0;
        size_t m_size = 0;
    };

    class Packet6699
    {
    public:
        Packet6699();

        void clear();

        bool parse(
            const uint8_t* data,
            size_t size);

        bool buildPlain(
            Command command,
            uint32_t sequence,
            const uint8_t* payload,
            size_t payloadLength);

        bool buildEncrypted(
            Command command,
            uint32_t sequence,
            const uint8_t* plaintext,
            size_t plaintextLength,
            TuyaCrypto& crypto);

        bool decryptPayload(
            TuyaCrypto& crypto,
            uint8_t* output,
            size_t outputCapacity,
            size_t& outputLength) const;

        const uint8_t* data() const
        {
            return m_buffer;
        }

        size_t size() const
        {
            return m_size;
        }

        Command command() const
        {
            return static_cast<Command>(m_header.command);
        }

        uint32_t sequence() const
        {
            return m_header.sequence;
        }

        const uint8_t* encryptedPayload() const
        {
            return m_ciphertext;
        }

        size_t encryptedPayloadSize() const
        {
            return m_ciphertextLength;
        }

    private:
        bool buildHeader(
            Command command,
            uint32_t sequence,
            uint32_t length);

        const uint8_t* aad() const
        {
            return m_buffer + 4;
        }

        size_t aadSize() const
        {
            return HEADER_6699_SIZE - 4;
        }

    private:
        Header6699 m_header{};

        uint8_t m_buffer[MAX_PACKET_6699_SIZE] {};
        uint8_t* m_iv = nullptr;
        uint8_t* m_ciphertext = nullptr;
        uint8_t* m_tag = nullptr;

        size_t m_ciphertextLength = 0;
        size_t m_size = 0;
    };
}
