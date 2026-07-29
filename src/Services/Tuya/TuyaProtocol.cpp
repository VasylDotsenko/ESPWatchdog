#include "TuyaProtocol.h"

#include <ArduinoJson.h>
#include <cstring>

namespace
{
    constexpr const char* VERSION_33 = "3.3";

    void copyString(
        char* destination,
        size_t destinationSize,
        const char* source)
    {
        if (destination == nullptr ||
            destinationSize == 0)
        {
            return;
        }

        if (source == nullptr)
        {
            destination[0] = '\0';
            return;
        }

        strncpy(
            destination,
            source,
            destinationSize - 1);

        destination[destinationSize - 1] = '\0';
    }
}

namespace Tuya
{
    bool Protocol::begin(
        const char* deviceId,
        const char* localKey,
        uint8_t protocolVersion)
    {
        reset();

        if (deviceId == nullptr ||
            localKey == nullptr)
        {
            return false;
        }

        if (deviceId[0] == '\0' ||
            localKey[0] == '\0')
        {
            return false;
        }

        if (protocolVersion != SUPPORTED_VERSION)
        {
            return false;
        }

        if (!m_crypto.setKey(localKey))
        {
            return false;
        }

        copyString(
            m_deviceId,
            sizeof(m_deviceId),
            deviceId);

        m_protocolVersion = protocolVersion;
        m_ready = true;

        return true;
    }

    void Protocol::reset()
    {
        m_crypto.clear();

        memset(
            m_deviceId,
            0,
            sizeof(m_deviceId));

        m_protocolVersion = SUPPORTED_VERSION;
        m_ready = false;
    }

    bool Protocol::ready() const
    {
        return m_ready &&
               m_crypto.ready();
    }

    uint8_t Protocol::protocolVersion() const
    {
        return m_protocolVersion;
    }

    bool Protocol::buildHeartbeat(
        uint32_t sequence,
        Packet& packet)
    {
        return packet.build(
            Command::HeartBeat,
            sequence,
            nullptr,
            0);
    }

    bool Protocol::buildStatusQuery(
        uint32_t sequence,
        Packet& packet)
    {
        if (!ready())
        {
            return false;
        }

        JsonDocument doc;

        doc["gwId"] = m_deviceId;
        doc["devId"] = m_deviceId;

        char json[MAX_JSON_SIZE] {};

        const size_t jsonLength =
            serializeJson(
                doc,
                json,
                sizeof(json));

        if (jsonLength == 0 ||
            jsonLength >= sizeof(json))
        {
            return false;
        }

        return buildEncryptedJsonPacket(
            Command::DPQuery,
            sequence,
            json,
            packet);
    }

    bool Protocol::buildSetDps(
        uint32_t sequence,
        uint8_t dps,
        bool value,
        Packet& packet)
    {
        if (!ready())
        {
            return false;
        }

        if (dps == 0)
        {
            return false;
        }

        JsonDocument doc;

        doc["devId"] = m_deviceId;
        doc["uid"] = m_deviceId;

        char timestamp[16] {};

        snprintf(
            timestamp,
            sizeof(timestamp),
            "%lu",
            static_cast<unsigned long>(millis() / 1000));

        doc["t"] = timestamp;

        JsonObject dpsObject =
            doc["dps"].to<JsonObject>();

        char dpsKey[4] {};

        snprintf(
            dpsKey,
            sizeof(dpsKey),
            "%u",
            dps);

        dpsObject[dpsKey] = value;

        char json[MAX_JSON_SIZE] {};

        const size_t jsonLength =
            serializeJson(
                doc,
                json,
                sizeof(json));

        if (jsonLength == 0 ||
            jsonLength >= sizeof(json))
        {
            return false;
        }

        return buildEncryptedJsonPacket(
            Command::Control,
            sequence,
            json,
            packet);
    }

    bool Protocol::decryptPayload(
        const Packet& packet,
        char* output,
        size_t outputCapacity,
        size_t& outputLength) const
    {
        outputLength = 0;

        if (!ready())
        {
            return false;
        }

        if (output == nullptr ||
            outputCapacity == 0)
        {
            return false;
        }

        const uint8_t* payload =
            packet.payload();

        const size_t payloadLength =
            packet.payloadSize();

        if (payload == nullptr ||
            payloadLength == 0)
        {
            output[0] = '\0';
            return false;
        }

        size_t encryptedLength = 0;

        const uint8_t* encrypted =
            skipVersionHeader(
                payload,
                payloadLength,
                encryptedLength);

        if (encrypted == nullptr ||
            encryptedLength == 0)
        {
            return false;
        }

        uint8_t decrypted[MAX_JSON_SIZE] {};
        size_t decryptedLength = 0;

        if (!m_crypto.decrypt(
                encrypted,
                encryptedLength,
                decrypted,
                sizeof(decrypted),
                decryptedLength))
        {
            return false;
        }

        if (decryptedLength + 1 > outputCapacity)
        {
            return false;
        }

        memcpy(
            output,
            decrypted,
            decryptedLength);

        output[decryptedLength] = '\0';
        outputLength = decryptedLength;

        return true;
    }

    bool Protocol::buildEncryptedJsonPacket(
        Command command,
        uint32_t sequence,
        const char* json,
        Packet& packet)
    {
        if (!ready())
        {
            return false;
        }

        uint8_t payload[VERSION_HEADER_SIZE + MAX_ENCRYPTED_SIZE] {};
        size_t payloadLength = 0;

        if (!encryptJsonPayload(
                json,
                payload,
                sizeof(payload),
                payloadLength))
        {
            return false;
        }

        return packet.build(
            command,
            sequence,
            payload,
            payloadLength);
    }

    bool Protocol::encryptJsonPayload(
        const char* json,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength) const
    {
        outputLength = 0;

        if (!ready())
        {
            return false;
        }

        if (json == nullptr ||
            output == nullptr)
        {
            return false;
        }

        const size_t jsonLength =
            strlen(json);

        if (jsonLength == 0 ||
            jsonLength > MAX_JSON_SIZE)
        {
            return false;
        }

        if (!appendVersionHeader(
                output,
                outputCapacity,
                outputLength))
        {
            return false;
        }

        size_t encryptedLength = 0;

        if (!m_crypto.encrypt(
                reinterpret_cast<const uint8_t*>(json),
                jsonLength,
                output + outputLength,
                outputCapacity - outputLength,
                encryptedLength))
        {
            outputLength = 0;
            return false;
        }

        outputLength += encryptedLength;

        return true;
    }

    bool Protocol::appendVersionHeader(
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength) const
    {
        if (output == nullptr)
        {
            return false;
        }

        if (outputCapacity < VERSION_HEADER_SIZE)
        {
            return false;
        }

        memset(
            output,
            0,
            VERSION_HEADER_SIZE);

        memcpy(
            output,
            VERSION_33,
            strlen(VERSION_33));

        outputLength = VERSION_HEADER_SIZE;

        return true;
    }

    bool Protocol::hasVersionHeader(
        const uint8_t* payload,
        size_t payloadLength) const
    {
        if (payload == nullptr ||
            payloadLength < VERSION_HEADER_SIZE)
        {
            return false;
        }

        return payload[0] == '3' &&
               payload[1] == '.' &&
               payload[2] == '3';
    }

    const uint8_t* Protocol::skipVersionHeader(
        const uint8_t* payload,
        size_t payloadLength,
        size_t& encryptedLength) const
    {
        encryptedLength = 0;

        if (payload == nullptr ||
            payloadLength == 0)
        {
            return nullptr;
        }

        if (hasVersionHeader(
                payload,
                payloadLength))
        {
            encryptedLength =
                payloadLength - VERSION_HEADER_SIZE;

            return payload + VERSION_HEADER_SIZE;
        }

        encryptedLength = payloadLength;

        return payload;
    }
}
