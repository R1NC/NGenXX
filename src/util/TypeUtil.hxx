#ifndef NGENXX_SRC_UTIL_TYPE_HXX_
#define NGENXX_SRC_UTIL_TYPE_HXX_

#include <cstdlib>
#include <cstring>

#include <NGenXXTypes.hxx>

static inline const char *copyStr(const std::string &s)
{
    const char *c = s.c_str();
    auto len = std::strlen(c);
    auto nc = mallocX<char>(len);
    std::strncpy(nc, c, len);
    return nc;
}

static inline const char **copyStrVector(const std::vector<std::string> &sv, size_t strMaxLen)
{
    auto sArrLen = sizeof(char *) * sv.size();
    auto sArr = mallocX<char *>(sArrLen);
    for (size_t i = 0; i < sv.size(); i++)
    {
        auto len = sizeof(char) * strMaxLen;
        sArr[i] = mallocX<char>(len);
        std::strncpy(sArr[i], sv[i].c_str(), len);
    }
    return const_cast<const char **>(sArr);
}

static inline const byte *copyBytes(const Bytes &t)
{
    auto cs = t.data();
    auto len = t.size();
    if (cs == nullptr || len == 0) [[unlikely]]
    {
        return nullptr;
    }
    auto ncs = mallocX<byte>(len);
    std::memcpy(static_cast<void *>(ncs), cs, len);
    return ncs;
}

static inline Bytes trimBytes(const Bytes &bytes)
{
    auto data = bytes.data();
    auto len = bytes.size();
    auto fixedLen = len;
    for (auto i = len - 1; i > 0; i--)
    {
        if (data[i] <= 0) [[unlikely]]
        {
            fixedLen--;
        }
    }
    return Bytes(bytes.begin(), bytes.begin() + fixedLen);
}

#endif // NGENXX_SRC_UTIL_TYPE_HXX_