#include "Crypto.hxx"

#include <string>
#include <cstring>

#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#include <NGenXXLog.hxx>

namespace
{
    constexpr auto OK = 1;
    constexpr auto AES_Key_BITS = 128;

    constexpr auto MD5_BYTES_LEN = 16uz;
    constexpr auto SHA256_BYTES_LEN = 32uz;

    class EvpCipherCtx
    {
    public:
        EvpCipherCtx()
        {
            raw = EVP_CIPHER_CTX_new();
        }
        ~EvpCipherCtx()
        {
            if (raw != nullptr) [[likely]]
            {
                EVP_CIPHER_CTX_free(raw);
                raw = nullptr;
            }
        }
        EvpCipherCtx(const EvpCipherCtx &) = delete;
        EvpCipherCtx &operator=(const EvpCipherCtx &) = delete;
        EvpCipherCtx(EvpCipherCtx &&) = delete;
        EvpCipherCtx &operator=(EvpCipherCtx &&) = delete;
        EVP_CIPHER_CTX *raw;
    };
}

#pragma mark Rand

bool NGenXX::Crypto::rand(size_t len, byte *bytes)
{
    if (len == 0 || bytes == nullptr) [[unlikely]]
    {
        return false;
    }
    const auto ret = RAND_bytes(bytes, static_cast<int>(len));
    return ret != -1;
}

#pragma mark AES

Bytes NGenXX::Crypto::AES::encrypt(const Bytes &inBytes, const Bytes &keyBytes)
{
    const auto in = inBytes.data(), key = keyBytes.data();
    const auto inLen = inBytes.size(), keyLen = keyBytes.size();
    if (in == nullptr || inLen == 0 || key == nullptr || keyLen != AES_BLOCK_SIZE) [[unlikely]]
    {
        return {};
    }
    
    //ECB-PKCS5 Padding:
    auto outLen = inLen;
    const auto paddingSize = AES_BLOCK_SIZE - (inLen % AES_BLOCK_SIZE);
    const auto paddingData = static_cast<byte>(paddingSize);
    outLen += paddingSize;

    byte fixedIn[outLen];
    std::memset(fixedIn, paddingData, outLen);
    std::memcpy(fixedIn, in, inLen);
    byte out[outLen];
    std::memset(out, 0, outLen);

    AES_KEY aes_key;

    if (const auto ret = AES_set_encrypt_key(key, AES_Key_BITS, &aes_key); ret != 0) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "AES_set_encrypt_key error:{}", ret);
        return {};
    }
    
    for (auto offset(0z); offset < outLen; offset += AES_BLOCK_SIZE)
    {
        AES_encrypt(fixedIn + offset, out + offset, &aes_key);
    }

    return wrapBytes(out, outLen);
}

Bytes NGenXX::Crypto::AES::decrypt(const Bytes &inBytes, const Bytes &keyBytes)
{
    const auto in = inBytes.data(), key = keyBytes.data();
    const auto inLen = inBytes.size(), keyLen = keyBytes.size();
    if (in == nullptr || inLen == 0 || key == nullptr 
        || keyLen != AES_BLOCK_SIZE || inLen % AES_BLOCK_SIZE != 0) [[unlikely]]
    {
        return {};
    }

    byte fixedIn[inLen];
    std::memcpy(fixedIn, in, inLen);
    byte out[inLen];
    std::memset(out, 0, inLen);

    AES_KEY aes_key;
    if (const auto ret = AES_set_decrypt_key(key, AES_Key_BITS, &aes_key); ret != 0) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "AES_set_decrypt_key error:{}", ret);
        return {};
    }

    for (auto offset(0z); offset < inLen; offset += AES_BLOCK_SIZE)
    {
        AES_decrypt(fixedIn + offset, out + offset, &aes_key);
    }

    // Proper PKCS5/PKCS7 unpadding
    auto paddingSize = static_cast<size_t>(out[inLen - 1]);
    if (paddingSize == 0 || paddingSize > AES_BLOCK_SIZE) [[unlikely]]
    {
        return {};
    }

    // Verify padding bytes
    for (decltype(paddingSize) i = 0; i < paddingSize; i++) 
    {
        if (out[inLen - 1 - i] != paddingSize) [[unlikely]]
        {
            return {};
        }
    }
    
    auto outLen = inLen - paddingSize;
    return wrapBytes(out, outLen);
}

#pragma mark AES-GCM

const EVP_CIPHER *aesGcmCipher(const Bytes &keyBytes)
{
    const auto key = keyBytes.data();
    const auto keyLen = keyBytes.size();
    if (key != nullptr && keyLen > 0) [[likely]]
    {
        if (keyLen == 16)
        {
            return EVP_aes_128_gcm();
        }
        if (keyLen == 24)
        {
            return EVP_aes_256_gcm();
        }
        if (keyLen == 32)
        {
            return EVP_aes_128_gcm();
        }
    }
    return nullptr;
}

Bytes NGenXX::Crypto::AES::gcmEncrypt(const Bytes &inBytes, const Bytes &keyBytes, const Bytes &initVectorBytes, const Bytes &aadBytes, size_t tagBits)
{
    if (!checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits)) [[unlikely]]
    {
        return {};
    }
    const auto in = inBytes.data(), key = keyBytes.data(), initVector = initVectorBytes.data(), aad = aadBytes.data();
    const auto inLen = inBytes.size(), keyLen = keyBytes.size(), initVectorLen = initVectorBytes.size(), aadLen = aadBytes.size();
    const auto tagLen = tagBits / 8;

    byte tag[tagLen];
    std::memset(tag, 0, tagLen);
    const auto outLen = inLen + tagLen;
    byte out[outLen];
    std::memset(out, 0, outLen);

    const auto cipher = aesGcmCipher(keyBytes);

    const EvpCipherCtx evpCipherCtx;
    if (evpCipherCtx.raw == nullptr) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_CIPHER_CTX_new failed");
        return {};
    }

    auto ret = EVP_EncryptInit_ex(evpCipherCtx.raw, cipher, nullptr, nullptr, nullptr);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptInit_ex cipher error:{}", ret);
        return {};
    }

    ret = EVP_CIPHER_CTX_ctrl(evpCipherCtx.raw, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(initVectorLen), nullptr);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_IVLEN error:{}", ret);
        return {};
    }

    ret = EVP_EncryptInit_ex(evpCipherCtx.raw, nullptr, nullptr, key, initVector);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptInit_ex initVector error:{}", ret);
        return {};
    }

    int len;

    if (aad != nullptr && aadLen > 0) [[likely]]
    {
        ret = EVP_EncryptUpdate(evpCipherCtx.raw, nullptr, &len, aad, static_cast<int>(aadLen));
        if (ret != OK) [[unlikely]]
        {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptUpdate aad error:{}", ret);
            return {};
        }
    }

    ret = EVP_EncryptUpdate(evpCipherCtx.raw, out, &len, in, static_cast<int>(inLen));
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptUpdate error:{}", ret);
        return {};
    }

    ret = EVP_EncryptFinal_ex(evpCipherCtx.raw, out, &len);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptFinal_ex error:{}", ret);
        return {};
    }

    ret = EVP_CIPHER_CTX_ctrl(evpCipherCtx.raw, EVP_CTRL_GCM_GET_TAG, static_cast<int>(tagLen), tag);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_GET_TAG error:{}", ret);
        return {};
    }
    std::memcpy(out + inLen, tag, tagLen);

    return wrapBytes(out, outLen);
}

Bytes NGenXX::Crypto::AES::gcmDecrypt(const Bytes &inBytes, const Bytes &keyBytes, const Bytes &initVectorBytes, const Bytes &aadBytes, size_t tagBits)
{
    if (!checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits)) [[unlikely]]
    {
        return {};
    }
    const auto in = inBytes.data(), key = keyBytes.data(), initVector = initVectorBytes.data(), aad = aadBytes.data();
    const auto inLen_ = inBytes.size(), keyLen = keyBytes.size(), initVectorLen = initVectorBytes.size(), aadLen = aadBytes.size();
    const auto tagLen = tagBits / 8;

    const auto inLen = inLen_ - tagLen;
    byte tag[tagLen];
    std::memcpy(tag, in + inLen, tagLen);
    const auto outLen = inLen;
    byte out[outLen];
    std::memset(out, 0, outLen);

    const auto cipher = aesGcmCipher(keyBytes);

    const EvpCipherCtx evpCipherCtx;
    if (evpCipherCtx.raw == nullptr) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_CIPHER_CTX_new failed");
        return {};
    }

    auto ret = EVP_DecryptInit_ex(evpCipherCtx.raw, cipher, nullptr, nullptr, nullptr);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptInit_ex cipher error:{}", ret);
        return {};
    }

    ret = EVP_CIPHER_CTX_ctrl(evpCipherCtx.raw, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(initVectorLen), nullptr);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmdDecrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_IVLEN error:{}", ret);
        return {};
    }

    ret = EVP_DecryptInit_ex(evpCipherCtx.raw, nullptr, nullptr, key, initVector);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptInit_ex initVector error:{}", ret);
        return {};
    }

    int len;

    if (aad != nullptr && aadLen > 0) [[likely]]
    {
        ret = EVP_DecryptUpdate(evpCipherCtx.raw, nullptr, &len, aad, static_cast<int>(aadLen));
        if (ret != OK) [[unlikely]]
        {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptUpdate aad error:{}", ret);
            return {};
        }
    }

    ret = EVP_DecryptUpdate(evpCipherCtx.raw, out, &len, in, static_cast<int>(inLen));
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptUpdate error:{}", ret);
        return {};
    }

    ret = EVP_CIPHER_CTX_ctrl(evpCipherCtx.raw, EVP_CTRL_GCM_SET_TAG, static_cast<int>(tagLen), tag);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_TAG error:{}", ret);
        return {};
    }

    ret = EVP_DecryptFinal_ex(evpCipherCtx.raw, out, &len);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptFinal_ex error:{}", ret);
        return {};
    }

    return wrapBytes(out, outLen);
}

#pragma mark MD5

Bytes NGenXX::Crypto::Hash::md5(const Bytes &inBytes)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }
    constexpr auto outLen = MD5_BYTES_LEN;
    byte out[outLen] = {};

    MD5_CTX md5;

    auto ret = MD5_Init(&md5);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "MD5_Init error:{}", ret);
        return {};
    }

    ret = MD5_Update(&md5, in, inLen);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "MD5_Update error:{}", ret);
        return {};
    }

    ret = MD5_Final(out, &md5);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "MD5_Final error:{}", ret);
        return {};
    }

    return wrapBytes(out, outLen);
}

#pragma mark SHA1

Bytes NGenXX::Crypto::Hash::sha1(const Bytes &inBytes)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0)  [[unlikely]]
    {
        return {};
    }
    unsigned int outLen = EVP_MAX_MD_SIZE;
    byte out[outLen];
    memset(out, 0, outLen);

    const auto ctx = EVP_MD_CTX_create();
    const auto md = EVP_sha1();

    auto ret = EVP_DigestInit_ex(ctx, md, nullptr);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "EVP_DigestInit_ex error:{}", ret);
        return {};
    }

    ret = EVP_DigestUpdate(ctx, in, inLen);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "EVP_DigestUpdate error:{}", ret);
        return {};
    }

    ret = EVP_DigestFinal_ex(ctx, out, &outLen);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "EVP_DigestFinal_ex error:{}", ret);
        return {};
    }

    EVP_MD_CTX_destroy(ctx);

    return wrapBytes(out, outLen);
}

#pragma mark SHA256
Bytes NGenXX::Crypto::Hash::sha256(const Bytes &inBytes)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }
    constexpr auto outLen = SHA256_BYTES_LEN;
    byte out[outLen] = {};

    SHA256_CTX sha256;

    auto ret = SHA256_Init(&sha256);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "SHA256_Init error:{}", ret);
        return {};
    }

    ret = SHA256_Update(&sha256, in, inLen);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "SHA256_Update error:{}", ret);
        return {};
    }

    ret = SHA256_Final(out, &sha256);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "SHA256_Final error:{}", ret);
        return {};
    }

    return wrapBytes(out, outLen);
}

#pragma mark RSA

RSA *ngenxxCryptoCreateRSA(const Bytes &key, bool isPublic)
{
    RSA *rsa = nullptr;
    const auto bio = BIO_new_mem_buf(key.data(), -1);
    if (!bio) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "RSA Failed to create BIO");
        return rsa;
    }
    if (isPublic) 
    {
        rsa = PEM_read_bio_RSA_PUBKEY(bio, &rsa, nullptr, nullptr);
    } 
    else 
    {
        rsa = PEM_read_bio_RSAPrivateKey(bio, &rsa, nullptr, nullptr);
    }
    if (!rsa) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "RSA Failed to create key");
    }
    BIO_free(bio);
    return rsa;
}

Bytes NGenXX::Crypto::RSA::encrypt(const Bytes &in, const Bytes &key, int padding)
{
    const auto rsa = ngenxxCryptoCreateRSA(key, true);
    const auto outLen = RSA_size(rsa);
    byte outBytes[outLen];
    if (const auto ret = RSA_public_encrypt(static_cast<int>(in.size()), in.data(), outBytes, rsa, padding); ret == -1) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "RSA encrypt error:{}", ret);
        RSA_free(rsa);
        return {};
    }
    auto out = wrapBytes(outBytes, outLen);
    RSA_free(rsa);
    return out;
}

Bytes NGenXX::Crypto::RSA::decrypt(const Bytes &in, const Bytes &key, int padding)
{
    const auto rsa = ngenxxCryptoCreateRSA(key, false);
    const auto outLen = RSA_size(rsa);
    byte outBytes[outLen];
    if (const auto ret = RSA_private_decrypt(static_cast<int>(in.size()), in.data(), outBytes, rsa, padding); ret == -1) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "RSA decrypt error:{}", ret);
        RSA_free(rsa);
        return {};
    }
    auto out = wrapBytes(outBytes, outLen);
    RSA_free(rsa);
    return out;
}

#pragma mark Base64

Bytes NGenXX::Crypto::Base64::encode(const Bytes &inBytes)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }

    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bmem = BIO_new(BIO_s_mem());

    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    b64 = BIO_push(b64, bmem);

    BIO_write(b64, in, static_cast<int>(inLen));
    BIO_flush(b64);

    char *outBytes = nullptr;
    const auto outLen = BIO_get_mem_data(bmem, &outBytes);
    if (outLen == 0) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "Failed to encode Base64");
        BIO_free_all(b64);
        return {};
    }

    auto out = wrapBytes(reinterpret_cast<byte *>(outBytes), outLen);

    BIO_free_all(b64);

    return out;
}

Bytes NGenXX::Crypto::Base64::decode(const Bytes &inBytes)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }

    BIO *bio = BIO_new_mem_buf(in, -1);
    BIO *b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    auto outLen = inLen * 3 / 4;
    byte outBytes[outLen];
    outLen = BIO_read(bio, outBytes, static_cast<int>(inLen));
    if (outLen == 0) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "Failed to decode Base64");
        BIO_free_all(bio);
        return {};
    }

    auto out = wrapBytes(outBytes, outLen);
    BIO_free_all(bio);

    return out;
}