#include "TuyaPacket.h"

#include <cstring>

namespace
{
    uint32_t readU32BE(const uint8_t* data)
    {
        return (static_cast<uint32_t>(data[0]) << 24) |
               (static_cast<uint32_t>(data[1]) << 16) |
               (static_cast<uint32_t>(data[2]) << 8) |
               (static_cast<uint32_t>(data[3]));
    }

    void writeU32BE(
        uint8_t* data,
        uint32_t value)
    {
        data[0] = static_cast<uint8_t>((value >> 24) & 0xFF);
        data[1] = static_cast<uint8_t>((value >> 16) & 0xFF);
        data[2] = static_cast<uint8_t>((value >> 8) & 0xFF);
        data[3] = static_cast<uint8_t>(value & 0xFF);
    }

    uint32_t crc32(
        const uint8_t* data,
        size_t length)
    {
        uint32_t crc = 0xFFFFFFFF;

        for (size_t i = 0; i < length; ++i)
        {
            crc ^= data[i];

            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                const bool lsb = (crc & 1) != 0;

                crc >>= 1;

                if (lsb)
                {
                    crc ^= 0xEDB88320;
                }
            }
        }

        return ~crc;
    }
}

namespace Tuya
{
    Packet::Packet()
    {
        clear();
    }

    void Packet::clear()
    {
        memset(
            &m_header,
            0,
            sizeof(m_header));

        memset(
            &m_footer,
            0,
            sizeof(m_footer));

        memset(
            m_buffer,
            0,
            sizeof(m_buffer));

        m_payload = nullptr;
        m_payloadLength = 0;
        m_size = 0;
    }

    bool Packet::parse(
        const uint8_t* data,
        size_t size)
    {
        clear();

        if (data == nullptr)
        {
            return false;
        }

        if (size < HEADER_SIZE + FOOTER_SIZE)
        {
            return false;
        }

        if (size > MAX_PACKET_SIZE)
        {
            return false;
        }

        const uint32_t prefix =
            readU32BE(data);

        if (prefix != PREFIX)
        {
            return false;
        }

        const uint32_t sequence =
            readU32BE(data + 4);

        const uint32_t command =
            readU32BE(data + 8);

        const uint32_t length =
            readU32BE(data + 12);

        if (length < FOOTER_SIZE)
        {
            return false;
        }

        const size_t packetSize =
            HEADER_SIZE + static_cast<size_t>(length);

        if (packetSize != size)
        {
            return false;
        }

        const size_t payloadLength =
            static_cast<size_t>(length) - FOOTER_SIZE;

        if (payloadLength > MAX_PAYLOAD_SIZE)
        {
            return false;
        }

        const size_t crcOffset =
            HEADER_SIZE + payloadLength;

        const uint32_t receivedCrc =
            readU32BE(data + crcOffset);

        const uint32_t suffix =
            readU32BE(data + crcOffset + 4);

        if (suffix != SUFFIX)
        {
            return false;
        }

        const uint32_t calculatedCrc =
            crc32(
                data,
                crcOffset);

        if (receivedCrc != calculatedCrc)
        {
            return false;
        }

        memcpy(
            m_buffer,
            data,
            size);

        m_header.prefix = prefix;
        m_header.sequence = sequence;
        m_header.command = command;
        m_header.length = length;

        m_footer.crc32 = receivedCrc;
        m_footer.suffix = suffix;

        m_payloadLength = payloadLength;
        m_payload = payloadLength > 0
            ? m_buffer + HEADER_SIZE
            : nullptr;

        m_size = size;

        return true;
    }

    bool Packet::build(
        Command command,
        uint32_t sequence,
        const uint8_t* payload,
        size_t payloadLength)
    {
        clear();

        if (payloadLength > MAX_PAYLOAD_SIZE)
        {
            return false;
        }

        if (payload == nullptr &&
            payloadLength > 0)
        {
            return false;
        }

        const size_t packetSize =
            HEADER_SIZE +
            payloadLength +
            FOOTER_SIZE;

        if (packetSize > MAX_PACKET_SIZE)
        {
            return false;
        }

        const uint32_t length =
            static_cast<uint32_t>(
                payloadLength + FOOTER_SIZE);

        writeU32BE(
            m_buffer,
            PREFIX);

        writeU32BE(
            m_buffer + 4,
            sequence);

        writeU32BE(
            m_buffer + 8,
            static_cast<uint32_t>(command));

        writeU32BE(
            m_buffer + 12,
            length);

        if (payloadLength > 0)
        {
            memcpy(
                m_buffer + HEADER_SIZE,
                payload,
                payloadLength);
        }

        const size_t crcOffset =
            HEADER_SIZE + payloadLength;

        const uint32_t calculatedCrc =
            crc32(
                m_buffer,
                crcOffset);

        writeU32BE(
            m_buffer + crcOffset,
            calculatedCrc);

        writeU32BE(
            m_buffer + crcOffset + 4,
            SUFFIX);

        m_header.prefix = PREFIX;
        m_header.sequence = sequence;
        m_header.command = static_cast<uint32_t>(command);
        m_header.length = length;

        m_footer.crc32 = calculatedCrc;
        m_footer.suffix = SUFFIX;

        m_payloadLength = payloadLength;
        m_payload = payloadLength > 0
            ? m_buffer + HEADER_SIZE
            : nullptr;

        m_size = packetSize;

        return true;
    }
}
