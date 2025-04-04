#include "Coding.hxx"

#include <cctype>

#include <iomanip>
#include <algorithm>
#include <charconv>
#if defined(USE_STD_RANGES)
#include <ranges>
#include <numeric>
#endif

std::string NGenXX::Coding::Case::upper(const std::string_view &str)
{
    std::string s(str);
    s.reserve(str.size());
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

std::string NGenXX::Coding::Case::lower(const std::string_view &str)
{
    std::string s(str);
    s.reserve(str.size());
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

std::string NGenXX::Coding::Hex::bytes2str(const Bytes &bytes)
{
    if (bytes.empty()) [[unlikely]]
    {
        return {};
    }
    std::string str;
    str.resize(bytes.size() * 2);
    auto transF = [](byte b, char *buf) {
        auto [ptr, errCode] = std::to_chars(buf, buf + 2, static_cast<int>(b), 16);
        if (errCode != std::errc()) [[unlikely]] 
        {
            buf[0] = '0';
            buf[1] = '0';
        }
        else if (ptr - buf == 1) // Zero-pad for single-digit results
        {
            buf[1] = buf[0];
            buf[0] = '0';
        }
    };
#if defined(USE_STD_RANGES)
    auto hexView = bytes 
        | std::ranges::views::transform([&str](byte b) {
            transF(b, &str[b * 2]);
            return std::string_view(&str[b * 2], 2);
        });
    return std::accumulate(hexView.begin(), hexView.end(), std::string{});
#else
    for (size_t i = 0; i < bytes.size(); ++i) 
    {
        transF(str[i], &str[i * 2]);
    }
    return str;
#endif
}

Bytes NGenXX::Coding::Hex::str2bytes(const std::string &str)
{
    auto sLen = str.length();
    if (sLen < 2) [[unlikely]]
    {
        return {};
    }
    auto transF = [](const std::string &s) { 
        auto hex = 0;
        auto [_, errCode] = std::from_chars(s.data(), s.data() + s.size(), hex, 16);
        if (errCode != std::errc()) [[unlikely]]
        {
            return static_cast<byte>(0);
        }
        return static_cast<byte>(hex);
    };
#if defined(USE_STD_RANGES_CHUNK)
    auto byteView = str 
        | std::ranges::views::chunk(2) 
        | std::ranges::views::transform(transF);
    return Bytes(byteView.begin(), byteView.end());
#else
    Bytes bytes;
    bytes.reserve(sLen / 2);
    for (decltype(sLen) i(0); i < sLen; i += 2)
    {
        auto s = str.substr(i, 2);
        bytes.emplace_back(transF(s));
    }
    return bytes;
#endif
}

std::string NGenXX::Coding::bytes2str(const Bytes &bytes)
{
    return std::string(bytes.begin(), bytes.end());
}

Bytes NGenXX::Coding::str2bytes(const std::string_view &str)
{
    return Bytes(str.begin(), str.end());
}

std::string NGenXX::Coding::strTrim(const std::string_view &str) 
{
#if defined(USE_STD_RANGES)
    auto findSpaceF = [](char c) { return std::isspace(static_cast<int>(c)); };
    auto trimmed = str 
        | std::ranges::views::drop_while(findSpaceF)
        | std::ranges::views::reverse
        | std::ranges::views::drop_while(findSpaceF)
        | std::ranges::views::reverse;
    return std::string(trimmed.begin(), trimmed.end());
#else
    constexpr auto invalidChars = " \t\n\r\f\v";
    const auto begin = str.find_first_not_of(invalidChars);
    if (begin == std::string_view::npos) [[unlikely]]
    {
        return {};
    }
    const auto end = str.find_last_not_of(invalidChars);
    return std::string(str.substr(begin, end - begin + 1));
#endif
}