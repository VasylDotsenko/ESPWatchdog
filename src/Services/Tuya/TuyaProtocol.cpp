#include "TuyaProtocol.h"

#include <ArduinoJson.h>
#include <cstring>

namespace
{
    constexpr const char* VERSION_33 = "3.3";
    constexpr const char* VERSION_35 = "3.5";

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

        if (!isSupportedVersion(protocolVersion))
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
        m_sessionCrypto.clear();

        memset(
            m_deviceId,
            0,
            sizeof(m_deviceId));

        m_protocolVersion = SUPPORTED_VERSION;
        memset(
            m_localNonce,
            0,
            sizeof(m_localNonce));

        memset(
            m_remoteNonce,
            0,
            sizeof(m_remoteNonce));

        m_ready = false;
        m_sessionReady = false;
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

    bool Protocol::sessionReady() const
    {
        if (m_protocolVersion == SUPPORTED_VERSION_33)
        {
            return ready();
        }

        return ready() &&
               m_sessionReady &&
               m_sessionCrypto.ready();
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

    bool Protocol::buildSetDps(
        uint32_t sequence,
        uint8_t dps,
        bool value,
        Packet6699& packet)
    {
        if (!sessionReady() ||
            m_protocolVersion != SUPPORTED_VERSION_35)
        {
            return false;
        }

        if (dps == 0)
        {
            return false;
        }

        JsonDocument doc;

        doc["protocol"] = 5;

        char timestamp[16] {};

        snprintf(
            timestamp,
            sizeof(timestamp),
            "%lu",
            static_cast<unsigned long>(millis() / 1000));

        doc["t"] = timestamp;

        JsonObject dataObject =
            doc["data"].to<JsonObject>();

        JsonObject dpsObject =
            dataObject["dps"].to<JsonObject>();

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

        uint8_t plaintext[VERSION_35_HEADER_SIZE + MAX_JSON_SIZE] {};
        size_t plaintextLength = 0;

        if (!appendVersion35Header(
                plaintext,
                sizeof(plaintext),
                plaintextLength))
        {
            return false;
        }

        if (plaintextLength + jsonLength > sizeof(plaintext))
        {
            return false;
        }

        memcpy(
            plaintext + plaintextLength,
            json,
            jsonLength);

        plaintextLength += jsonLength;

        return packet.buildEncrypted(
            Command::ControlNew,
            sequence,
            plaintext,
            plaintextLength,
            m_sessionCrypto);
    }

    bool Protocol::buildSessionStart(
        uint32_t sequence,
        Packet6699& packet)
    {
        if (!ready() ||
            m_protocolVersion != SUPPORTED_VERSION_35)
        {
            return false;
        }

        if (!TuyaCrypto::randomBytes(
                m_localNonce,
                sizeof(m_localNonce)))
        {
            return false;
        }

        m_sessionReady = false;
        m_sessionCrypto.clear();

        return packet.buildEncrypted(
            Command::SessionKeyStart,
            sequence,
            m_localNonce,
            sizeof(m_localNonce),
            m_crypto);
    }

    bool Protocol::processSessionResponse(
        const Packet6699& packet)
    {
        if (!ready() ||
            m_protocolVersion != SUPPORTED_VERSION_35 ||
            packet.command() != Command::SessionKeyResp)
        {
            return false;
        }

        uint8_t payload[128] {};
        size_t payloadLength = 0;

        if (!packet.decryptPayload(
                m_crypto,
                payload,
                sizeof(payload),
                payloadLength))
        {
            return false;
        }

        size_t payloadOffset = 0;

        if (payloadLength >= 4 + TuyaCrypto::LOCAL_KEY_SIZE + TuyaCrypto::SHA256_SIZE &&
            payload[0] == 0x00 &&
            payload[1] == 0x00 &&
            payload[2] == 0x00 &&
            payload[3] == 0x00)
        {
            payloadOffset = 4;
        }

        if (payloadLength < payloadOffset + TuyaCrypto::LOCAL_KEY_SIZE + TuyaCrypto::SHA256_SIZE)
        {
            return false;
        }

        memcpy(
            m_remoteNonce,
            payload + payloadOffset,
            TuyaCrypto::LOCAL_KEY_SIZE);

        uint8_t expectedHmac[TuyaCrypto::SHA256_SIZE] {};

        if (!m_crypto.hmacSha256(
                m_localNonce,
                sizeof(m_localNonce),
                expectedHmac))
        {
            return false;
        }

        if (!TuyaCrypto::constantTimeEquals(
                expectedHmac,
                payload + payloadOffset + TuyaCrypto::LOCAL_KEY_SIZE,
                TuyaCrypto::SHA256_SIZE))
        {
            return false;
        }

        uint8_t sessionKey[TuyaCrypto::LOCAL_KEY_SIZE] {};

        if (!m_crypto.deriveTuya35SessionKey(
                m_localNonce,
                m_remoteNonce,
                sessionKey))
        {
            return false;
        }

        if (!m_sessionCrypto.setKey(
                sessionKey,
                sizeof(sessionKey)))
        {
            return false;
        }

        m_sessionReady = true;

        return true;
    }

    bool Protocol::buildSessionFinish(
        uint32_t sequence,
        Packet6699& packet)
    {
        if (!ready() ||
            m_protocolVersion != SUPPORTED_VERSION_35 ||
            !m_sessionReady)
        {
            return false;
        }

        uint8_t hmac[TuyaCrypto::SHA256_SIZE] {};

        if (!m_crypto.hmacSha256(
                m_remoteNonce,
                sizeof(m_remoteNonce),
                hmac))
        {
            return false;
        }

        return packet.buildEncrypted(
            Command::SessionKeyFinish,
            sequence,
            hmac,
            sizeof(hmac),
            m_crypto);
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

    bool Protocol::decryptPayload(
        const Packet6699& packet,
        char* output,
        size_t outputCapacity,
        size_t& outputLength)
    {
        outputLength = 0;

        if (!sessionReady() ||
            output == nullptr ||
            outputCapacity == 0)
        {
            return false;
        }

        uint8_t decrypted[MAX_JSON_SIZE + VERSION_35_HEADER_SIZE + 4] {};
        size_t decryptedLength = 0;

        if (!packet.decryptPayload(
                m_sessionCrypto,
                decrypted,
                sizeof(decrypted),
                decryptedLength))
        {
            return false;
        }

        size_t jsonLength = 0;

        const uint8_t* json =
            skipTuya35PlainHeader(
                decrypted,
                decryptedLength,
                jsonLength);

        if (json == nullptr ||
            jsonLength == 0)
        {
            return false;
        }

        if (jsonLength + 1 > outputCapacity)
        {
            return false;
        }

        memcpy(
            output,
            json,
            jsonLength);

        output[jsonLength] = '\0';
        outputLength = jsonLength;

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

    bool Protocol::appendVersion35Header(
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength) const
    {
        if (output == nullptr ||
            outputCapacity < VERSION_35_HEADER_SIZE)
        {
            return false;
        }

        memset(
            output,
            0,
            VERSION_35_HEADER_SIZE);

        memcpy(
            output,
            VERSION_35,
            strlen(VERSION_35));

        outputLength = VERSION_35_HEADER_SIZE;

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

    const uint8_t* Protocol::skipTuya35PlainHeader(
        const uint8_t* payload,
        size_t payloadLength,
        size_t& jsonLength) const
    {
        jsonLength = 0;

        if (payload == nullptr ||
            payloadLength == 0)
        {
            return nullptr;
        }

        size_t offset = 0;

        if (payloadLength >= 4 &&
            payload[0] == 0 &&
            payload[1] == 0 &&
            payload[2] == 0)
        {
            offset += 4;
        }

        if (payloadLength >= offset + VERSION_35_HEADER_SIZE &&
            payload[offset] == '3' &&
            payload[offset + 1] == '.' &&
            payload[offset + 2] == '5')
        {
            offset += VERSION_35_HEADER_SIZE;
        }

        if (offset >= payloadLength)
        {
            return nullptr;
        }

        jsonLength = payloadLength - offset;

        return payload + offset;
    }
}
