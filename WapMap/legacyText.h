#pragma once
#include <Windows.h>
#include <optional>
#include <string>
#include <string_view>

namespace LegacyText {
inline UINT CodePage(const char* language) {
    if (language[0] == 'P') return 1250;
    if (language[0] == 'R') return 1251;
    return 1252;
}

inline std::string ToUtf8(std::string_view text, UINT codePage) {
    if (text.empty()) return {};
    const int length = static_cast<int>(text.size());
    const int wideLength = MultiByteToWideChar(codePage, 0, text.data(), length, nullptr, 0);
    std::wstring wide(wideLength, 0);
    MultiByteToWideChar(codePage, 0, text.data(), length, wide.data(), wideLength);
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, wide.data(), wideLength, nullptr, 0, nullptr, nullptr);
    std::string result(bytes, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), wideLength, result.data(), bytes, nullptr, nullptr);
    return result;
}

inline std::optional<std::string> FromUtf8(std::string_view text, UINT codePage, size_t maximumBytes) {
    if (text.empty()) return std::string{};
    const int length = static_cast<int>(text.size());
    const int wideLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), length, nullptr, 0);
    if (!wideLength) return std::nullopt;
    std::wstring wide(wideLength, 0);
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), length, wide.data(), wideLength);
    if (codePage == CP_ACP) codePage = GetACP();
    if (codePage == CP_UTF8) {
        if (text.size() > maximumBytes) return std::nullopt;
        return std::string(text);
    }
    BOOL substituted = FALSE;
    const int bytes = WideCharToMultiByte(codePage, WC_NO_BEST_FIT_CHARS, wide.data(), wideLength,
                                         nullptr, 0, nullptr, &substituted);
    if (!bytes || substituted || static_cast<size_t>(bytes) > maximumBytes) return std::nullopt;
    std::string result(bytes, 0);
    WideCharToMultiByte(codePage, WC_NO_BEST_FIT_CHARS, wide.data(), wideLength,
                        result.data(), bytes, nullptr, &substituted);
    if (substituted) return std::nullopt;
    return result;
}
}
