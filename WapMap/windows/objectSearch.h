#pragma once
#include "coordinateNavigation.h"
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
namespace ObjectSearch {
inline bool Match(std::string_view text, std::string_view query, bool caseSensitive) {
    if (query.empty()) return false;
    if (caseSensitive) return text.find(query) != std::string_view::npos;
    return std::search(text.begin(), text.end(), query.begin(), query.end(), [](unsigned char a, unsigned char b) {
        return std::tolower(a) == std::tolower(b);
    }) != text.end();
}
inline bool MatchId(int id, std::string_view query) {
    if (query.empty()) return false;
    const auto parsed = CoordinateNavigation::Parse(query);
    return parsed && *parsed == id;
}
}
