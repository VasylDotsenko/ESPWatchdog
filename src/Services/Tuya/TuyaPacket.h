#pragma once

#include <Arduino.h>

namespace Tuya
{
    constexpr uint32_t PREFIX = 0x000055AA;
    constexpr uint32_t SUFFIX = 0x0000AA55;

    constexpr size_t MAX_PAYLOAD_SIZE = 1024;
    constexpr size_t HEADER_SIZE = 16;
    constexpr size_t FOOTER_SIZE = 8;
    constexpr size_t MAX_PACKET_SIZE =
        HEADER_SIZE +
        MAX_PAYLOAD_SIZE +
        FOOTER_SIZE;

    enum class Command : uint32_t
    {
        HeartBeat      = 9,
        Query          = 10,
        Control        = 7,
        Status         = 8,
        DPQuery        = 10,
        Updatedps      = 18
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
}
