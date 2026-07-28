#include "TuyaCrypto.h"

#include <cstring>

#include <bearssl/bearssl_block.h>
#include <bearssl/bearssl_hash.h>

namespace
{
    char hexDigit(
        uint8_t value,
        bool uppercase)
    {
        static constexpr char LOWER[] = "0123456789abcdef";
        static constexpr char UPPER[] = "0123456789ABCDEF";

        return uppercase
            ? UPPER[value & 0x0F]
            : LOWER[value & 0x0F];
    }
}

namespace Tuya
{
    bool TuyaCrypto::setKey(const char* localKey)
    {
        if (localKey == nullptr)
        {
            clear();
            return false;
        }

        return setKey(
            reinterpret_cast<const uint8_t*>(localKey),
            strlen(localKey));
    }

    bool TuyaCrypto::setKey(
        const uint8_t* key,
        size_t keyLength)
    {
        clear();

        if (key == nullptr)
        {
            return false;
        }

        if (keyLength != LOCAL_KEY_SIZE)
        {
            return false;
        }

        memcpy(
            m_key,
            key,
            LOCAL_KEY_SIZE);

        m_ready = true;

        return true;
    }

    void TuyaCrypto::clear()
    {
        memset(
            m_key,
            0,
            sizeof(m_key));

        m_ready = false;
    }

    bool TuyaCrypto::ready() const
    {
        return m_ready;
    }

    size_t TuyaCrypto::encryptedSize(size_t plainLength) const
    {
        const size_t remainder =
            plainLength % AES_BLOCK_SIZE;

        const size_t padding =
            AES_BLOCK_SIZE - remainder;

        return plainLength + padding;
    }

    bool TuyaCrypto::encrypt(
        const uint8_t* input,
        size_t inputLength,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength) const
    {
        outputLength = 0;

        if (!m_ready)
        {
            return false;
        }

        if ((input == nullptr && inputLength > 0) ||
            output == nullptr)
        {
            return false;
        }

        const size_t paddedLength =
            encryptedSize(inputLength);

        if (outputCapacity < paddedLength)
        {
            return false;
        }

        const uint8_t padding =
            static_cast<uint8_t>(
                paddedLength - inputLength);

        size_t offset = 0;

        while (offset < paddedLength)
        {
            uint8_t block[AES_BLOCK_SIZE] {};

            for (size_t i = 0; i < AES_BLOCK_SIZE; ++i)
            {
                const size_t index = offset + i;

                block[i] = index < inputLength
                    ? input[index]
                    : padding;
            }

            if (!encryptBlock(
                    block,
                    output + offset))
            {
                outputLength = 0;
                return false;
            }

            offset += AES_BLOCK_SIZE;
        }

        outputLength = paddedLength;

        return true;
    }

    bool TuyaCrypto::decrypt(
        const uint8_t* input,
        size_t inputLength,
        uint8_t* output,
        size_t outputCapacity,
        size_t& outputLength,
        bool verifyPadding) const
    {
        outputLength = 0;

        if (!m_ready)
        {
            return false;
        }

        if (input == nullptr ||
            output == nullptr)
        {
            return false;
        }

        if (inputLength == 0 ||
            inputLength % AES_BLOCK_SIZE != 0)
        {
            return false;
        }

        if (outputCapacity < inputLength)
        {
            return false;
        }

        for (size_t offset = 0;
             offset < inputLength;
             offset += AES_BLOCK_SIZE)
        {
            if (!decryptBlock(
                    input + offset,
                    output + offset))
            {
                outputLength = 0;
                return false;
            }
        }

        const uint8_t padding =
            output[inputLength - 1];

        if (padding == 0 ||
            padding > AES_BLOCK_SIZE ||
            padding > inputLength)
        {
            return false;
        }

        if (verifyPadding)
        {
            for (size_t i = 0; i < padding; ++i)
            {
                if (output[inputLength - 1 - i] != padding)
                {
                    return false;
                }
            }
        }

        outputLength = inputLength - padding;

        return true;
    }

    bool TuyaCrypto::md5(
        const uint8_t* input,
        size_t inputLength,
        uint8_t output[MD5_SIZE])
    {
        if ((input == nullptr && inputLength > 0) ||
            output == nullptr)
        {
            return false;
        }

        br_md5_context context;

        br_md5_init(&context);

        if (inputLength > 0)
        {
            br_md5_update(
                &context,
                input,
                inputLength);
        }

        br_md5_out(
            &context,
            output);

        return true;
    }

    bool TuyaCrypto::md5Hex(
        const uint8_t* input,
        size_t inputLength,
        char output[MD5_HEX_SIZE],
        bool uppercase)
    {
        if (output == nullptr)
        {
            return false;
        }

        uint8_t digest[MD5_SIZE] {};

        if (!md5(
                input,
                inputLength,
                digest))
        {
            output[0] = '\0';
            return false;
        }

        return toHex(
            digest,
            sizeof(digest),
            output,
            MD5_HEX_SIZE,
            uppercase);
    }

    bool TuyaCrypto::toHex(
        const uint8_t* input,
        size_t inputLength,
        char* output,
        size_t outputCapacity,
        bool uppercase)
    {
        if ((input == nullptr && inputLength > 0) ||
            output == nullptr)
        {
            return false;
        }

        const size_t required =
            (inputLength * 2) + 1;

        if (outputCapacity < required)
        {
            return false;
        }

        for (size_t i = 0; i < inputLength; ++i)
        {
            output[i * 2] =
                hexDigit(input[i] >> 4, uppercase);

            output[i * 2 + 1] =
                hexDigit(input[i], uppercase);
        }

        output[inputLength * 2] = '\0';

        return true;
    }

    bool TuyaCrypto::constantTimeEquals(
        const uint8_t* lhs,
        const uint8_t* rhs,
        size_t length)
    {
        if (lhs == nullptr ||
            rhs == nullptr)
        {
            return false;
        }

        uint8_t diff = 0;

        for (size_t i = 0; i < length; ++i)
        {
            diff |= lhs[i] ^ rhs[i];
        }

        return diff == 0;
    }

    bool TuyaCrypto::encryptBlock(
        const uint8_t input[AES_BLOCK_SIZE],
        uint8_t output[AES_BLOCK_SIZE]) const
    {
        if (!m_ready ||
            input == nullptr ||
            output == nullptr)
        {
            return false;
        }

        uint8_t block[AES_BLOCK_SIZE];
        uint8_t iv[AES_BLOCK_SIZE] {};

        memcpy(
            block,
            input,
            AES_BLOCK_SIZE);

        br_aes_big_cbcenc_keys context;

        br_aes_big_cbcenc_init(
            &context,
            m_key,
            LOCAL_KEY_SIZE);

        br_aes_big_cbcenc_run(
            &context,
            iv,
            block,
            AES_BLOCK_SIZE);

        memcpy(
            output,
            block,
            AES_BLOCK_SIZE);

        return true;
    }

    bool TuyaCrypto::decryptBlock(
        const uint8_t input[AES_BLOCK_SIZE],
        uint8_t output[AES_BLOCK_SIZE]) const
    {
        if (!m_ready ||
            input == nullptr ||
            output == nullptr)
        {
            return false;
        }

        uint8_t block[AES_BLOCK_SIZE];
        uint8_t iv[AES_BLOCK_SIZE] {};

        memcpy(
            block,
            input,
            AES_BLOCK_SIZE);

        br_aes_big_cbcdec_keys context;

        br_aes_big_cbcdec_init(
            &context,
            m_key,
            LOCAL_KEY_SIZE);

        br_aes_big_cbcdec_run(
            &context,
            iv,
            block,
            AES_BLOCK_SIZE);

        memcpy(
            output,
            block,
            AES_BLOCK_SIZE);

        return true;
    }
}
