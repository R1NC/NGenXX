#include "Crypto.hxx"

#include <utility>
#include <algorithm>
#include <array>
#if defined(__cpp_lib_ranges)
#include <ranges>
#endif

#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#include <NGenXXLog.hxx>
#include <NGenXXCoding.hxx>

namespace
{
    constexpr auto OK = 1;
    constexpr auto AES_Key_BITS = 128;

    constexpr auto MD5_BYTES_LEN = 16uz;
    constexpr auto SHA256_BYTES_LEN = 32uz;

    std::optional<std::string> readErrMsg()
    {
        const auto err = ERR_get_error();
        if (err == 0) [[unlikely]]
        {
            return std::nullopt;
        }

        char errMsg[256];
        ERR_error_string_n(err, errMsg, sizeof(errMsg));
        const auto actualLen = std::strlen(errMsg);
        if (actualLen <= 0) [[unlikely]]
        {
            return std::nullopt;
        }

        return std::make_optional(std::string(errMsg, actualLen));
    }

    class Evp
    {
    public:
        Evp() = delete;
        Evp(const Evp &) = delete;
        Evp &operator=(const Evp &) = delete;
        Evp(Evp &&) = delete;
        Evp &operator=(Evp &&) = delete;

        explicit Evp(size_t keyLen) : keyLen(keyLen)
        {
            this->ctx = EVP_CIPHER_CTX_new();
        }

        EVP_CIPHER_CTX* context() const
        {
            return this->ctx;
        }

        const EVP_CIPHER* cipher() const
        {
            switch (this->keyLen)
            {
            case 16:
                return EVP_aes_128_gcm();
            case 24:
            case 32:
                return EVP_aes_256_gcm();
            default:
                return nullptr;
            }
        }

        ~Evp()
        {
            if (this->ctx != nullptr) [[likely]]
            {
                EVP_CIPHER_CTX_free(this->ctx);
                this->ctx = nullptr;
            }
        }

    private:
        const size_t keyLen;
        EVP_CIPHER_CTX *ctx;
    };

    struct Base64CharValidator {
        static constexpr size_t charIdx(char c) noexcept {
            //The range of byte(unsigned char) is just [0, 255], same as the size of validTable!
            return static_cast<byte>(c);
        }
        
        static constexpr std::array<bool, 256> createValidTable() noexcept {
            std::array<bool, 256> table{};
            constexpr const std::string_view validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
            for (const auto c : validChars) {
                table[charIdx(c)] = true;
            }
            return table;
        }
        
        bool operator()(char c) const noexcept {
            static const auto table = createValidTable();
            return table[charIdx(c)];
        }
    };
}

#pragma mark Rand

bool NGenXX::Core::Crypto::rand(size_t len, byte *bytes)
{
    if (len == 0 || bytes == nullptr) [[unlikely]]
    {
        return false;
    }
    const auto ret = RAND_bytes(bytes, static_cast<int>(len));
    return ret != -1;
}

#pragma mark AES

Bytes NGenXX::Core::Crypto::AES::encrypt(BytesView inBytes, BytesView keyBytes)
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

    Bytes fixedIn(outLen, paddingData);
    std::memcpy(fixedIn.data(), in, inLen);
    Bytes out(outLen, 0);

    AES_KEY aes_key;

    if (const auto ret = AES_set_encrypt_key(key, AES_Key_BITS, &aes_key); ret != 0) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "AES_set_encrypt_key failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }
    
    for (auto offset(0z); offset < outLen; offset += AES_BLOCK_SIZE)
    {
        AES_encrypt(fixedIn.data() + offset, out.data() + offset, &aes_key);
    }

    return out;
}

Bytes NGenXX::Core::Crypto::AES::decrypt(BytesView inBytes, BytesView keyBytes)
{
    const auto in = inBytes.data(), key = keyBytes.data();
    const auto inLen = inBytes.size(), keyLen = keyBytes.size();
    if (in == nullptr || inLen == 0 || key == nullptr 
        || keyLen != AES_BLOCK_SIZE || inLen % AES_BLOCK_SIZE != 0) [[unlikely]]
    {
        return {};
    }

    Bytes fixedIn(inLen, 0);
    std::memcpy(fixedIn.data(), in, inLen);
    Bytes out(inLen, 0);

    AES_KEY aes_key;
    if (const auto ret = AES_set_decrypt_key(key, AES_Key_BITS, &aes_key); ret != 0) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "AES_set_decrypt_key failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    for (auto offset(0z); offset < inLen; offset += AES_BLOCK_SIZE)
    {
        AES_decrypt(fixedIn.data() + offset, out.data() + offset, &aes_key);
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
    
    out.resize(inLen - paddingSize);
    return out;
}

#pragma mark AES-GCM

Bytes NGenXX::Core::Crypto::AES::gcmEncrypt(BytesView inBytes, BytesView keyBytes, BytesView initVectorBytes, BytesView aadBytes, size_t tagBits)
{
    if (!checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits)) [[unlikely]]
    {
        return {};
    }
    const auto in = inBytes.data(), key = keyBytes.data(), initVector = initVectorBytes.data(), aad = aadBytes.data();
    const auto inLen = inBytes.size(), keyLen = keyBytes.size(), initVectorLen = initVectorBytes.size(), aadLen = aadBytes.size();
    const auto tagLen = tagBits / 8;

    Bytes tag(tagLen, 0);
    const auto outLen = inLen;
    Bytes out(outLen, 0);

    const auto evp = Evp(keyLen);
    if (evp.context() == nullptr) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_CIPHER_CTX_new failed");
        return {};
    }

    auto ret = EVP_EncryptInit_ex(evp.context(), evp.cipher(), nullptr, nullptr, nullptr);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptInit_ex cipher failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    ret = EVP_CIPHER_CTX_ctrl(evp.context(), EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(initVectorLen), nullptr);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_IVLEN failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    ret = EVP_EncryptInit_ex(evp.context(), nullptr, nullptr, key, initVector);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptInit_ex initVector failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    int len;

    if (aad != nullptr && aadLen > 0) [[likely]]
    {
        ret = EVP_EncryptUpdate(evp.context(), nullptr, &len, aad, static_cast<int>(aadLen));
        if (ret != OK) [[unlikely]]
        {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptUpdate aad failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
            return {};
        }
    }

    ret = EVP_EncryptUpdate(evp.context(), out.data(), &len, in, static_cast<int>(inLen));
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptUpdate encrypt failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    ret = EVP_EncryptFinal_ex(evp.context(), out.data(), &len);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_EncryptFinal_ex encrypt failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    ret = EVP_CIPHER_CTX_ctrl(evp.context(), EVP_CTRL_GCM_GET_TAG, static_cast<int>(tagLen), tag.data());
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmEncrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_GET_TAG failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }
    out.insert(out.end(), tag.begin(), tag.end());

    return out;
}

Bytes NGenXX::Core::Crypto::AES::gcmDecrypt(BytesView inBytes, BytesView keyBytes, BytesView initVectorBytes, BytesView aadBytes, size_t tagBits)
{
    if (!checkGcmParams(inBytes, keyBytes, initVectorBytes, aadBytes, tagBits)) [[unlikely]]
    {
        return {};
    }
    const auto in = inBytes.data(), key = keyBytes.data(), initVector = initVectorBytes.data(), aad = aadBytes.data();
    const auto inLen_ = inBytes.size(), keyLen = keyBytes.size(), initVectorLen = initVectorBytes.size(), aadLen = aadBytes.size();
    const auto tagLen = tagBits / 8;

    const auto inLen = inLen_ - tagLen;
    Bytes tag(tagLen, 0);
    std::memcpy(tag.data(), in + inLen, tagLen);
    const auto outLen = inLen;
    Bytes out(outLen, 0);

    const auto evp = Evp(keyLen);
    if (evp.context() == nullptr) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_CIPHER_CTX_new failed");
        return {};
    }

    auto ret = EVP_DecryptInit_ex(evp.context(), evp.cipher(), nullptr, nullptr, nullptr);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptInit_ex cipher failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    ret = EVP_CIPHER_CTX_ctrl(evp.context(), EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(initVectorLen), nullptr);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_IVLEN failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    ret = EVP_DecryptInit_ex(evp.context(), nullptr, nullptr, key, initVector);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptInit_ex initVector failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    int len;

    if (aad != nullptr && aadLen > 0) [[likely]]
    {
        ret = EVP_DecryptUpdate(evp.context(), nullptr, &len, aad, static_cast<int>(aadLen));
        if (ret != OK) [[unlikely]]
        {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptUpdate aad failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
            return {};
        }
    }

    ret = EVP_DecryptUpdate(evp.context(), out.data(), &len, in, static_cast<int>(inLen));
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptUpdate decrypt failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    ret = EVP_CIPHER_CTX_ctrl(evp.context(), EVP_CTRL_GCM_SET_TAG, static_cast<int>(tagLen), tag.data());
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_CIPHER_CTX_ctrl EVP_CTRL_GCM_SET_TAG failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    ret = EVP_DecryptFinal_ex(evp.context(), out.data(), &len);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "aesGcmDecrypt EVP_DecryptFinal_ex decrypt failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    out.resize(outLen);
    return out;
}

#pragma mark MD5

Bytes NGenXX::Core::Crypto::Hash::md5(BytesView inBytes)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }
    constexpr auto outLen = MD5_BYTES_LEN;
    Bytes out(outLen, 0);

    MD5_CTX md5;

    auto ret = MD5_Init(&md5);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "MD5_Init failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    ret = MD5_Update(&md5, in, inLen);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "MD5_Update failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    ret = MD5_Final(out.data(), &md5);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "MD5_Final failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    return out;
}

#pragma mark SHA1

Bytes NGenXX::Core::Crypto::Hash::sha1(BytesView inBytes)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0)  [[unlikely]]
    {
        return {};
    }
    unsigned int outLen = EVP_MAX_MD_SIZE;
    Bytes out;
    out.reserve(outLen);

    const auto ctx = EVP_MD_CTX_create();
    const auto md = EVP_sha1();

    auto ret = EVP_DigestInit_ex(ctx, md, nullptr);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "EVP_DigestInit_ex failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    ret = EVP_DigestUpdate(ctx, in, inLen);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "EVP_DigestUpdate failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    ret = EVP_DigestFinal_ex(ctx, out.data(), &outLen);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "EVP_DigestFinal_ex failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    EVP_MD_CTX_destroy(ctx);

    return out;
}

#pragma mark SHA256

Bytes NGenXX::Core::Crypto::Hash::sha256(BytesView inBytes)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }
    constexpr auto outLen = SHA256_BYTES_LEN;
    Bytes out;
    out.reserve(outLen);

    SHA256_CTX sha256;

    auto ret = SHA256_Init(&sha256);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "SHA256_Init failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    ret = SHA256_Update(&sha256, in, inLen);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "SHA256_Update failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    ret = SHA256_Final(out.data(), &sha256);
    if (ret != OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "SHA256_Final failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return {};
    }

    return out;
}

#pragma mark RSA

std::string NGenXX::Core::Crypto::RSA::genKey(std::string_view base64, bool isPublic) 
{
    if (base64.empty()) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "RSA genKey: empty base64 input");
        return {};
    }
    
    const auto cleanedBase64 = ngenxxCodingStrTrim(base64);
    if (!Base64::validate(cleanedBase64)) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "RSA genKey: invalid cleanedBase64");
        return {};
    }
    
    using namespace std::string_view_literals;
    
    constexpr auto ChunkSize = 64uz; 
    constexpr auto DividerS = "-----"sv;
    constexpr auto BeginS = "BEGIN "sv;
    constexpr auto EndS = "END "sv;
    constexpr auto KeyS = " KEY"sv;
    constexpr auto NewLineS = "\n"sv;
    constexpr auto PublicS = "PUBLIC"sv;
    constexpr auto PrivateS = "PRIVATE"sv;

    const auto mid = isPublic ? PublicS : PrivateS;
    const auto numChunks = (cleanedBase64.size() + ChunkSize - 1) / ChunkSize;
    const auto headerSize = DividerS.size() + BeginS.size() + mid.size() + KeyS.size() + DividerS.size() + NewLineS.size(); 
    const auto footerSize = DividerS.size() + EndS.size() + mid.size() + KeyS.size() + DividerS.size() + NewLineS.size(); 
    const auto contentSize = cleanedBase64.size() + numChunks * NewLineS.size();
    
    std::string result; 
    result.reserve(headerSize + contentSize + footerSize); 
    
    result.append(DividerS).append(BeginS).append(mid).append(KeyS).append(DividerS).append(NewLineS); 
    
    for (size_t i = 0; i < cleanedBase64.size(); i += ChunkSize) 
    { 
        const auto remainingSize = cleanedBase64.size() - i;
        const auto currentChunkSize = std::min(ChunkSize, remainingSize);
        result.append(cleanedBase64.substr(i, currentChunkSize)).append(NewLineS); 
    } 
    
    result.append(DividerS).append(EndS).append(mid).append(KeyS).append(DividerS).append(NewLineS); 
    
    return result; 
}

NGenXX::Core::Crypto::RSA::Codec::Codec(BytesView key, int padding) : padding(padding)
{
    this->bmem = BIO_new_mem_buf(key.data(), static_cast<int>(key.size()));
    if (!this->bmem) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Failed to create BIO mem buffer for RSA, err: {}", readErrMsg().value_or(""));
    }
}

void NGenXX::Core::Crypto::RSA::Codec::moveImp(Codec &&other) noexcept
{
    const_cast<int&>(this->padding) = other.padding;
    this->bmem = std::exchange(other.bmem, nullptr);
    this->rsa = std::exchange(other.rsa, nullptr);
}

void NGenXX::Core::Crypto::RSA::Codec::cleanup() noexcept
{
    if (this->bmem != nullptr) [[likely]]
    {
        BIO_free(this->bmem);
        this->bmem = nullptr;
    }
    if (this->rsa != nullptr) [[likely]]
    {
        RSA_free(this->rsa);
        this->rsa = nullptr;
    }
}

NGenXX::Core::Crypto::RSA::Codec::Codec(Codec &&other) noexcept
    : padding(other.padding)
{
    this->moveImp(std::move(other));
}

NGenXX::Core::Crypto::RSA::Codec& NGenXX::Core::Crypto::RSA::Codec::operator=(Codec &&other) noexcept
{
    if (this != &other) [[likely]]
    {
        this->cleanup();
        this->moveImp(std::move(other));
    }
    return *this;
}

NGenXX::Core::Crypto::RSA::Codec::~Codec()
{
    this->cleanup();
}

std::size_t NGenXX::Core::Crypto::RSA::Codec::outLen() const
{
    if (this->rsa == nullptr) [[unlikely]]
    {
        return 0;
    }
    return RSA_size(this->rsa);
}

NGenXX::Core::Crypto::RSA::Encrypt::Encrypt(BytesView key, int padding) : Codec(key, padding)
{
    if (this->bmem == nullptr) [[unlikely]]
    {
        return;
    }
    this->rsa = PEM_read_bio_RSA_PUBKEY(this->bmem, nullptr, nullptr, nullptr);
    if (!this->rsa) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "RSA Failed to create public key, err: {}", readErrMsg().value_or(""));
    }
}

std::optional<Bytes> NGenXX::Core::Crypto::RSA::Encrypt::process(BytesView in) const
{
    if (this->rsa == nullptr) [[unlikely]]
    {
        return std::nullopt;
    }
    Bytes outBytes(this->outLen(), 0);
    if (const auto ret = RSA_public_encrypt(static_cast<int>(in.size()), in.data(), outBytes.data(), this->rsa, this->padding); ret == -1) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "RSA encrypt failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return std::nullopt;
    }
    return std::make_optional(outBytes);
}

NGenXX::Core::Crypto::RSA::Decrypt::Decrypt(BytesView key, int padding) : Codec(key, padding)
{
    if (this->bmem == nullptr) [[unlikely]]
    {
        return;
    }
    this->rsa = PEM_read_bio_RSAPrivateKey(this->bmem, nullptr, nullptr, nullptr);
    if (!this->rsa) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "RSA Failed to create private key, err: {}", readErrMsg().value_or(""));
    }
}

std::optional<Bytes> NGenXX::Core::Crypto::RSA::Decrypt::process(BytesView in) const
{
    if (this->rsa == nullptr) [[unlikely]]
    {
        return std::nullopt;
    }
    Bytes outBytes(this->outLen(), 0);
    if (const auto ret = RSA_private_decrypt(static_cast<int>(in.size()), in.data(), outBytes.data(), this->rsa, this->padding); ret == -1) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "RSA decrypt failed, ret: {}, err: {}", ret, readErrMsg().value_or(""));
        return std::nullopt;
    }
    return std::make_optional(outBytes);
}

#pragma mark Base64

bool NGenXX::Core::Crypto::Base64::validate(std::string_view in)
{
    if (in.empty() || in.size() % 4 != 0) [[unlikely]]
    {
        return false;
    }
    
    static constexpr Base64CharValidator validator{};
    
#if defined(__cpp_lib_ranges)
    return std::ranges::all_of(in, validator);
#else
    for (const auto c : in)
    {
        if (!validator(c)) [[unlikely]]
        {
            return false;
        }
    }
    return true;
#endif
}

Bytes NGenXX::Core::Crypto::Base64::encode(BytesView inBytes, bool noNewLines)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }

    const auto b64 = BIO_new(BIO_f_base64());
    if (!b64) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Failed to create BIO for Base64, err: {}", readErrMsg().value_or(""));
        return {};
    }

    const auto bsmem = BIO_new(BIO_s_mem());
    if (!bsmem) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Failed to create BIO mem for Base64, err: {}", readErrMsg().value_or(""));
        BIO_free(b64);
        return {};
    }

    if (noNewLines)
    {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    }
    
    const auto bpush = BIO_push(b64, bsmem);
    if (!bpush) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Failed to push BIO mem for Base64, err: {}", readErrMsg().value_or(""));
        BIO_free_all(b64);
        return {};
    }

    const int writeResult = BIO_write(b64, in, static_cast<int>(inLen));
    if (writeResult <= 0) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Failed to write to Base64 BIO, err: {}", readErrMsg().value_or(""));
        BIO_free_all(b64);
        return {};
    }

    const int flushResult = BIO_flush(b64);
    if (flushResult <= 0) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Failed to flush Base64 BIO, err: {}", readErrMsg().value_or(""));
        BIO_free_all(b64);
        return {};
    }

    char *outBytes = nullptr;
    const auto outLen = BIO_get_mem_data(bpush, &outBytes);
    if (outLen <= 0 || outBytes == nullptr) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Failed to get Base64 encoded data, err: {}", readErrMsg().value_or(""));
        BIO_free_all(b64);
        return {};
    }

    // Copy data before freeing BIO to avoid potential memory issues
    Bytes out(outLen);
    std::memcpy(out.data(), outBytes, outLen);

    BIO_free_all(b64);

    return out;
}

Bytes NGenXX::Core::Crypto::Base64::decode(BytesView inBytes, bool noNewLines)
{
    const auto in = inBytes.data();
    const auto inLen = inBytes.size();
    if (in == nullptr || inLen == 0) [[unlikely]]
    {
        return {};
    }

    // Validate Base64 characters
    for (size_t i = 0; i < inLen; i++)
    {
        if (std::isalnum(in[i]) || in[i] == '+' || in[i] == '/' || in[i] == '=')
        {
            continue;
        }
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Invalid Base64 char: {}", in[i]);
        return {};
    }

    // Create memory BIO with explicit length
    const auto bmem = BIO_new_mem_buf(in, static_cast<int>(inLen));
    if (!bmem) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Failed to create BIO mem buffer for Base64, err: {}", readErrMsg().value_or(""));
        return {};
    }

    const auto b64 = BIO_new(BIO_f_base64());
    if (!b64) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Failed to create BIO for Base64, err: {}", readErrMsg().value_or(""));
        BIO_free_all(bmem);
        return {};
    }

    const auto bpush = BIO_push(b64, bmem);
    if (!bpush) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Failed to push BIO mem for Base64, err: {}", readErrMsg().value_or(""));
        BIO_free_all(b64);
        return {};
    }

    if (noNewLines)
    {
        BIO_set_flags(bpush, BIO_FLAGS_BASE64_NO_NL);
    }

    auto outLen = (inLen * 3) / 4 + 3; // Add padding for safety
    Bytes outBytes(outLen, 0);
    
    const auto bytesRead = BIO_read(bpush, outBytes.data(), static_cast<int>(outLen));
    if (bytesRead <= 0) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "Failed to decode Base64, err: {}", readErrMsg().value_or(""));
        BIO_free_all(bpush);
        return {};
    }

    BIO_free_all(bpush);
    outBytes.resize(bytesRead);
    return outBytes;
}
