#include "Crypto.hxx"
#include "../../include/NGenXXLog.hxx"
#include <string>
#include <cstring>

#include "../util/TypeUtil.hxx"
#include "../../../external/openssl/include/openssl/bio.h"
#include "../../../external/openssl/include/openssl/rand.h"
#include "../../../external/openssl/include/openssl/evp.h"
#include "../../../external/openssl/include/openssl/aes.h"
#include "../../../external/openssl/include/openssl/sha.h"
#include "../../../external/openssl/include/openssl/md5.h"
#include "../../../external/openssl/include/openssl/buffer.h"

constexpr int OpenSSL_OK = 1;
constexpr int OpenSSL_AES_Key_BITS = 128;

bool NGenXX::Crypto::rand(const size_t len, byte *bytes)
{
    if (len <= 0 || bytes == NULL)
        return false;
    int ret = RAND_bytes(bytes, len);
    return ret != -1;
}

const Bytes NGenXX::Crypto::AES::encrypt(const Bytes &inBytes, const Bytes &keyBytes)
{
    auto [in, inLen] = inBytes;
    auto [key, keyLen] = keyBytes;
    if (in == NULL || inLen == 0 || key == NULL || keyLen != AES_BLOCK_SIZE)
        return BytesEmpty;
    // keyLen = AES_BLOCK_SIZE * 8;
    size_t outLen = inLen;
    if (inLen % AES_BLOCK_SIZE != 0)
    {
        outLen = (inLen / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    }
    byte fixedIn[outLen];
    std::memset(fixedIn, 0, outLen);
    std::memcpy(fixedIn, in, inLen);
    byte out[outLen];
    std::memset(out, 0, outLen);
    int offset = 0;

    AES_KEY aes_key;

    int ret = AES_set_encrypt_key(key, OpenSSL_AES_Key_BITS, &aes_key);
    if (ret != 0)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "AES_set_encrypt_key error:" + std::to_string(ret));
        return BytesEmpty;
    }

    while (offset < inLen)
    {
        AES_encrypt(fixedIn + offset, out + offset, &aes_key);
        offset += AES_BLOCK_SIZE;
    }

    return {out, outLen};
}

const Bytes NGenXX::Crypto::AES::decrypt(const Bytes &inBytes, const Bytes &keyBytes)
{
    auto [in, inLen] = inBytes;
    auto [key, keyLen] = keyBytes;
    if (in == NULL || inLen == 0 || key == NULL || keyLen != AES_BLOCK_SIZE)
        return BytesEmpty;
    // keyLen = AES_BLOCK_SIZE * 8;
    size_t outLen = inLen;
    if (inLen % AES_BLOCK_SIZE != 0)
    {
        outLen = (inLen / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
    }
    byte fixedIn[outLen];
    std::memset(fixedIn, 0, outLen);
    std::memcpy(fixedIn, in, inLen);
    byte out[outLen];
    std::memset(out, 0, outLen);
    int offset = 0;

    AES_KEY aes_key;

    int ret = AES_set_decrypt_key(key, OpenSSL_AES_Key_BITS, &aes_key);
    if (ret != 0)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "AES_set_decrypt_key error:" + std::to_string(ret));
        return BytesEmpty;
    }

    while (offset < inLen)
    {
        AES_decrypt(fixedIn + offset, out + offset, &aes_key);
        offset += AES_BLOCK_SIZE;
    }

    return trimBytes({out, outLen});
}

const EVP_CIPHER *aesGcmCipher(const Bytes &keyBytes)
{
    auto [key, keyLen] = keyBytes;
    if (key != NULL && keyLen > 0)
    {
        if (keyLen == 16)
            return EVP_aes_128_gcm();
        if (keyLen == 24)
            return EVP_aes_256_gcm();
        if (keyLen == 32)
            return EVP_aes_128_gcm();
    }
    return NULL;
}

const Bytes NGenXX::Crypto::AES::gcmEncrypt(const Bytes &inBytes, const Bytes &keyBytes, const Bytes &initVectorBytes, const Bytes &aadBytes, const size_t tagBits)
{
    if (!NGenXX::Crypto::AES::checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits))
        return BytesEmpty;
    auto [in, inLen] = inBytes;
    auto [key, keyLen] = keyBytes;
    auto [initVector, initVectorLen] = initVectorBytes;
    auto [aad, aadLen] = aadBytes;
    const size_t tagLen = tagBits / 8;

    byte tag[tagLen];
    std::memset(tag, 0, tagLen);
    size_t outLen = inLen + tagLen;
    byte out[outLen];
    std::memset(out, 0, outLen);

    auto cipher = aesGcmCipher(keyBytes);

    EVP_CIPHER_CTX *ctx;

    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_CIPHER_CTX_new failed");
        return BytesEmpty;
    }

    int ret = EVP_EncryptInit_ex(ctx, cipher, NULL, NULL, NULL);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptInit_ex cipher error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, initVectorLen, NULL);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_IVLEN error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = EVP_EncryptInit_ex(ctx, NULL, NULL, key, initVector);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptInit_ex initVector error:" + std::to_string(ret));
        return BytesEmpty;
    }

    int len;

    if (aad != NULL && aadLen > 0)
    {
        ret = EVP_EncryptUpdate(ctx, NULL, &len, aad, aadLen);
        if (ret != OpenSSL_OK)
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptUpdate aad error:" + std::to_string(ret));
            return BytesEmpty;
        }
    }

    ret = EVP_EncryptUpdate(ctx, out, &len, in, inLen);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptUpdate error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = EVP_EncryptFinal_ex(ctx, out, &len);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptFinal_ex error:" + std::to_string(ret));
        return BytesEmpty;
    }

    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, tagLen, tag);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_GET_TAG error:" + std::to_string(ret));
        return BytesEmpty;
    }
    std::memcpy(out + inLen, tag, tagLen);

    EVP_CIPHER_CTX_free(ctx);

    return {out, outLen};
}

const Bytes NGenXX::Crypto::AES::gcmDecrypt(const Bytes &inBytes, const Bytes &keyBytes, const Bytes &initVectorBytes, const Bytes &aadBytes, const size_t tagBits)
{
    if (!NGenXX::Crypto::AES::checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits))
        return BytesEmpty;
    auto [in, inLen_] = inBytes;
    auto [key, keyLen] = keyBytes;
    auto [initVector, initVectorLen] = initVectorBytes;
    auto [aad, aadLen] = aadBytes;
    const size_t tagLen = tagBits / 8;

    size_t inLen = inLen_ - tagLen;
    byte tag[tagLen];
    std::memcpy(tag, in + inLen, tagLen);
    size_t outLen = inLen;
    byte out[outLen];
    std::memset(out, 0, outLen);

    auto cipher = aesGcmCipher(keyBytes);

    EVP_CIPHER_CTX *ctx;

    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_CIPHER_CTX_new failed");
        return BytesEmpty;
    }

    int ret = EVP_DecryptInit_ex(ctx, cipher, NULL, NULL, NULL);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptInit_ex cipher error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, initVectorLen, NULL);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmdDecrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_IVLEN error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = EVP_DecryptInit_ex(ctx, NULL, NULL, key, initVector);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptInit_ex initVector error:" + std::to_string(ret));
        return BytesEmpty;
    }

    int len;

    if (aad != NULL && aadLen > 0)
    {
        ret = EVP_DecryptUpdate(ctx, NULL, &len, aad, aadLen);
        if (ret != OpenSSL_OK)
        {
            ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptUpdate aad error:" + std::to_string(ret));
            return BytesEmpty;
        }
    }

    ret = EVP_DecryptUpdate(ctx, out, &len, in, inLen);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptUpdate error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tagLen, tag);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_TAG error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = EVP_DecryptFinal_ex(ctx, out, &len);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptFinal_ex error:" + std::to_string(ret));
        return BytesEmpty;
    }

    EVP_CIPHER_CTX_free(ctx);

    return {out, outLen};
}

const Bytes NGenXX::Crypto::Hash::md5(const Bytes &inBytes)
{
    auto [in, inLen] = inBytes;
    if (in == NULL || inLen == 0)
        return BytesEmpty;
    size_t outLen = MD5_BYTES_LEN;
    byte out[outLen];
    std::memset(out, 0, outLen);

    MD5_CTX md5;

    int ret = MD5_Init(&md5);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "MD5_Init error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = MD5_Update(&md5, in, inLen);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "MD5_Update error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = MD5_Final(out, &md5);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "MD5_Final error:" + std::to_string(ret));
        return BytesEmpty;
    }

    return {out, outLen};
}

const Bytes NGenXX::Crypto::Hash::sha256(const Bytes &inBytes)
{
    auto [in, inLen] = inBytes;
    if (in == NULL || inLen == 0)
        return BytesEmpty;
    size_t outLen = SHA256_BYTES_LEN;
    byte out[outLen];
    std::memset(out, 0, outLen);

    SHA256_CTX sha256;

    int ret = SHA256_Init(&sha256);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "SHA256_Init error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = SHA256_Update(&sha256, in, inLen);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "SHA256_Update error:" + std::to_string(ret));
        return BytesEmpty;
    }

    ret = SHA256_Final(out, &sha256);
    if (ret != OpenSSL_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "SHA256_Final error:" + std::to_string(ret));
        return BytesEmpty;
    }

    return {out, outLen};
}

const Bytes NGenXX::Crypto::Base64::encode(const Bytes &inBytes)
{
    auto [in, inLen] = inBytes;
    if (in == NULL || inLen == 0)
        return BytesEmpty;

    BIO *bio, *b64;
    BUF_MEM *bptr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, in, inLen);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bptr);
    BIO_set_close(bio, BIO_NOCLOSE);

    Bytes outBytes = {reinterpret_cast<byte *>(bptr->data), bptr->length};
    BIO_free_all(bio);

    return outBytes;
}

const Bytes NGenXX::Crypto::Base64::decode(const Bytes &inBytes)
{
    auto [in, inLen] = inBytes;
    if (in == NULL || inLen == 0)
        return BytesEmpty;

    BIO *bio, *b64;

    size_t outLen = inLen * 2;
    byte outBuffer[outLen];
    std::memset(outBuffer, 0, outLen);

    bio = BIO_new_mem_buf(in, -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_read(bio, outBuffer, inLen);

    Bytes outBytes = {outBuffer, outLen};
    BIO_free_all(bio);

    return trimBytes(outBytes);
}