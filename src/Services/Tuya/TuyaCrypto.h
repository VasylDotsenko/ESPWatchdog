#pragma once

#include <Arduino.h>

namespace Tuya
{
    class TuyaCrypto
    {
    public:
        static constexpr size_t AES_BLOCK_SIZE = 16;
        static constexpr size_t LOCAL_KEY_SIZE = 16;
        static constexpr size_t MD5_SIZE = 16;
        static constexpr size_t MD5_HEX_SIZE = 33;
        static constexpr size_t GCM_IV_SIZE = 12;
        static constexpr size_t GCM_TAG_SIZE = 16;

        TuyaCrypto() = default;

        bool setKey(const char* localKey);

        bool setKey(
            const uint8_t* key,
            size_t keyLength);

        void clear();

        [[nodiscard]]
        bool ready() const;

        [[nodiscard]]
        size_t encryptedSize(size_t plainLength) const;

        bool encrypt(
            const uint8_t* input,
            size_t inputLength,
            uint8_t* output,
            size_t outputCapacity,
            size_t& outputLength) const;

        bool decrypt(
            const uint8_t* input,
            size_t inputLength,
            uint8_t* output,
            size_t outputCapacity,
            size_t& outputLength,
            bool verifyPadding = true) const;

        bool encryptGcm(
            const uint8_t* input,
            size_t inputLength,
            const uint8_t iv[GCM_IV_SIZE],
            const uint8_t* aad,
            size_t aadLength,
            uint8_t* output,
            size_t outputCapacity,
            size_t& outputLength,
            uint8_t tag[GCM_TAG_SIZE]) const;

        bool decryptGcm(
            const uint8_t* input,
            size_t inputLength,
            const uint8_t iv[GCM_IV_SIZE],
            const uint8_t* aad,
            size_t aadLength,
            const uint8_t tag[GCM_TAG_SIZE],
            uint8_t* output,
            size_t outputCapacity,
            size_t& outputLength) const;

        bool deriveTuya35SessionKey(
            const uint8_t localNonce[LOCAL_KEY_SIZE],
            const uint8_t remoteNonce[LOCAL_KEY_SIZE],
            uint8_t sessionKey[LOCAL_KEY_SIZE]) const;

        static bool randomBytes(
            uint8_t* output,
            size_t outputLength);

        static bool md5(
            const uint8_t* input,
            size_t inputLength,
            uint8_t output[MD5_SIZE]);

        static bool md5Hex(
            const uint8_t* input,
            size_t inputLength,
            char output[MD5_HEX_SIZE],
            bool uppercase = false);

        static bool toHex(
            const uint8_t* input,
            size_t inputLength,
            char* output,
            size_t outputCapacity,
            bool uppercase = false);

        static bool constantTimeEquals(
            const uint8_t* lhs,
            const uint8_t* rhs,
            size_t length);

    private:
        bool encryptBlock(
            const uint8_t input[AES_BLOCK_SIZE],
            uint8_t output[AES_BLOCK_SIZE]) const;

        bool decryptBlock(
            const uint8_t input[AES_BLOCK_SIZE],
            uint8_t output[AES_BLOCK_SIZE]) const;

    private:
        uint8_t m_key[LOCAL_KEY_SIZE] {};
        bool m_ready = false;
    };
}
