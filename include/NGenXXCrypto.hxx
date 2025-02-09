#ifndef NGENXX_INCLUDE_CRYPTO_HXX_
#define NGENXX_INCLUDE_CRYPTO_HXX_

#include "NGenXXTypes.hxx"

bool ngenxxCryptoRand(const size_t len, byte *bytes);

const Bytes ngenxxCryptoAesEncrypt(const Bytes &in, const Bytes &key);

const Bytes ngenxxCryptoAesDecrypt(const Bytes &in, const Bytes &key);

const Bytes ngenxxCryptoAesGcmEncrypt(const Bytes &in, const Bytes &key, const Bytes &initVector, const size_t tagBits,
                                      const Bytes &aad = BytesEmpty);

const Bytes ngenxxCryptoAesGcmDecrypt(const Bytes &in, const Bytes &key, const Bytes &initVector, const size_t tagBits,
                                      const Bytes &aad = BytesEmpty);

enum class NGenXXCryptoRSAPaddingX : int
{
    PKCS1 = 1,
    SSLV23 = 2,
    NO = 3,
    PKCS1_OAEP = 4,
    X931 = 5,
    PKCS1_PSS = 6
};

const Bytes ngenxxCryptoRsaEncrypt(const Bytes &in, const Bytes &key, const NGenXXCryptoRSAPaddingX padding);

const Bytes ngenxxCryptoRsaDecrypt(const Bytes &in, const Bytes &key, const NGenXXCryptoRSAPaddingX padding);

const Bytes ngenxxCryptoHashMd5(const Bytes &in);

const Bytes ngenxxCryptoHashSha256(const Bytes &in);

const Bytes ngenxxCryptoBase64Encode(const Bytes &in);

const Bytes ngenxxCryptoBase64Decode(const Bytes &in);

#endif // NGENXX_INCLUDE_CRYPTO_HXX_