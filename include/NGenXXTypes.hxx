#ifndef NGENXX_INCLUDE_TYPES_HXX_
#define NGENXX_INCLUDE_TYPES_HXX_

#include "NGenXXTypes.h"

#ifdef __cplusplus

#include <cstring>

#include <string>
#include <vector>
#include <variant>
#include <unordered_map>
#include <functional>
#include <limits>
#include <type_traits>

#pragma mark Limits constants

constexpr auto MinInt32 = std::numeric_limits<int32_t>::min();
constexpr auto MaxInt32 = std::numeric_limits<int32_t>::max();
constexpr auto MinInt64 = std::numeric_limits<int64_t>::min();
constexpr auto MaxInt64 = std::numeric_limits<int64_t>::max();
constexpr auto MinFloat32 = std::numeric_limits<float>::min();
constexpr auto MaxFloat32 = std::numeric_limits<float>::max();
constexpr auto MinFloat64 = std::numeric_limits<double>::min();
constexpr auto MaxFloat64 = std::numeric_limits<double>::max();

#pragma mark Concepts

template <typename T>
concept ConstT = std::is_const_v<T>;
    
template <typename T>
concept VoidT = std::is_void_v<T>;

template<typename T>
concept NumberT =
    std::is_arithmetic_v<T> ||
        (std::is_enum_v<T> && std::is_convertible_v<std::underlying_type_t<T>, int>);

template<typename T>
concept IntegerT = NumberT<T> && std::integral<T>;

template<typename T>
concept FloatT = NumberT<T> && std::floating_point<T>;

template<typename T>
concept EnumT = NumberT<T> && std::is_enum_v<T>;

template <typename T>
concept CharacterT =
    std::is_same_v<T, char> || std::is_same_v<T, wchar_t>
#ifdef __cpp_char8_t
    || std::is_same_v<T, char8_t>
#endif
    || std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t>;

template<class T, class U>
concept DerivedT = std::is_base_of_v<U, T>;

template<typename T>
concept PolymorphicT = std::is_polymorphic_v<T>;

template<typename T>
concept MemcpyableT = std::is_trivially_copyable_v<T>;

template<typename T>
concept HashableT = requires(T a) 
{
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept Comparable = requires(T a, T b) 
{
    { a < b } -> std::convertible_to<bool>;
};

template<typename T>
concept Iterable = requires(T a) 
{
    { std::begin(a) } -> std::input_iterator;
    { std::end(a) } -> std::sentinel_for<decltype(std::begin(a))>;
};

#pragma mark Utils using concept

template<PolymorphicT T>
T* dynamicCastX(void* ptr) 
{
    return dynamic_cast<T *>(ptr);
}

template<HashableT T>
std::size_t getHash(const T& value) 
{
    return std::hash<T>{}(value);
}

template<Iterable T>
void iterate(const T& container, std::function<void(const typename T::value_type&)> &&func) 
{
    for (const auto& e : container)
    {
        std::move(func)(e);
    }
}

#pragma mark String to Number

int32_t str2int32(const std::string &str, const int32_t defaultI = MinInt32);
int64_t str2int64(const std::string &str, const int64_t defaultI = MinInt64);
float str2float32(const std::string &str, const float defaultI = MinFloat32);
double str2float64(const std::string &str, const double defaultI = MinFloat64);

#pragma mark Pointer cast

template <typename T = void>
T *addr2ptr(const address addr)
{
    return reinterpret_cast<T *>(addr);
}

template <typename T = void>
address ptr2addr(const T *ptr)
{
    return reinterpret_cast<address>(ptr);
}

#pragma mark Any Type

using Any = std::variant<int64_t, double, std::string>;

auto Any2String(const Any &v, const auto &defaultS = {})
{
    if (std::holds_alternative<std::string>(v)) [[likely]]
    {
        return std::get<std::string>(v);
    }
    if (std::holds_alternative<int64_t>(v)) [[unlikely]]
    {
        return std::to_string(std::get<int64_t>(v));
    }
    if (std::holds_alternative<double>(v)) [[unlikely]]
    {
        return std::to_string(std::get<double>(v));
    }
    return defaultS;
}

auto Any2Integer(const Any &dict, const auto defaultI = MinInt64)
{
    if (!std::holds_alternative<std::string>(dict)) [[likely]]
    {
        return std::get<int64_t>(dict);
    }
    else [[unlikely]]
    {
        return str2int64(std::get<std::string>(dict), defaultI);
    }
}

auto Any2Float(const Any &dict, const auto defaultF = MinFloat64)
{
    if (!std::holds_alternative<std::string>(dict)) [[likely]]
    {
        return std::get<double>(dict);
    }
    else [[unlikely]]
    {
        return str2float64(std::get<std::string>(dict), defaultF);
    }
}

#pragma mark Dict Type

using Dict = std::unordered_map<std::string, std::string>;
using DictAny = std::unordered_map<std::string, Any>;

inline std::string dictAnyReadString(const DictAny &dict, const std::string &key, const std::string &defaultS = {})
{
    return !dict.contains(key) ? defaultS : Any2String(dict.at(key), defaultS);
}

inline int64_t dictAnyReadInteger(const DictAny &dict, const std::string &key, const int64_t defaultI = MinInt64)
{
    return !dict.contains(key) ? defaultI : Any2Integer(dict.at(key), defaultI);
}

inline double dictAnyReadFloat(const DictAny &dict, const std::string &key, const double defaultF = MinFloat64)
{
    return !dict.contains(key) ? defaultF : Any2Float(dict.at(key), defaultF);
}

#pragma mark Bytes Type

using Bytes = std::vector<byte>;

inline Bytes wrapBytes(const byte *data, const size_t len) {
    return {data, data + len};
}

template <typename T>
std::string wrapStr(const T* ptr)
{
    if (ptr == nullptr) 
    {
        return {};
    }
    return {reinterpret_cast<const char*>(ptr)};
}

#pragma mark Memory Utils

template<MemcpyableT T>
void memcpyX(const T* src, T* dst, const std::size_t count)
{
    std::memcpy(dst, src, count * sizeof(T));
}

// malloc for character types
template <CharacterT T>
T* mallocX(const size_t count = 1)
{
    const auto len = count * sizeof(T) + 1;
    auto ptr = std::malloc(len);
    if (!ptr) [[unlikely]]
    {
        return nullptr;
    }
    std::memset(ptr, 0, len);
    return static_cast<T *>(ptr);
}

// malloc for non-character types
template <typename T>
requires (!CharacterT<T>)
T* mallocX(const size_t count = 1)
{
    auto ptr = std::calloc(count, sizeof(T));
    if (!ptr) [[unlikely]]
    {
        return nullptr;
    }
    return static_cast<T *>(ptr);
}

// free for non-const & non-void types
template <typename T>
requires (!ConstT<T> && !VoidT<T>)
void freeX(T* &ptr)
{
    if (!ptr) [[unlikely]]
    {
        return;
    }
    std::free(static_cast<void *>(ptr));
    ptr = nullptr;
}

// free for non-const & void types
template <typename T>
requires (!ConstT<T> && VoidT<T>)
void freeX(T* &ptr)
{
    if (!ptr) [[unlikely]]
    {
        return;
    }
    std::free(ptr);
    ptr = nullptr;
}

// free for const & non-void types
template <typename T>
requires (ConstT<T> && !VoidT<T>)
void freeX(T* &ptr)
{
    if (!ptr) [[unlikely]]
    {
        return;
    }
    std::free(const_cast<void *>(static_cast<const void *>(ptr)));
    ptr = nullptr;
}

// free for const & void types
template <typename T>
requires (ConstT<T> && VoidT<T>)
void freeX(T* &ptr)
{
    if (!ptr) [[unlikely]]
    {
        return;
    }
    std::free(const_cast<void *>(ptr));
    ptr = nullptr;
}

#endif

#endif // NGENXX_INCLUDE_TYPES_HXX_
