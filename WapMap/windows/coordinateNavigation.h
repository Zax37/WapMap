#pragma once
#include <charconv>
#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>

namespace CoordinateNavigation {
struct Point { int x; int y; };

inline std::optional<int> Parse(std::string_view text) {
    if (text.empty()) return 0;
    int value{};
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) return std::nullopt;
    return value;
}

inline std::optional<Point> Resolve(std::string_view x, std::string_view y,
                                    bool world, int tileWidth, int tileHeight) {
    const auto parsedX = Parse(x), parsedY = Parse(y);
    if (!parsedX || !parsedY || tileWidth <= 0 || tileHeight <= 0) return std::nullopt;
    const std::int64_t worldX = world ? *parsedX : std::int64_t(*parsedX) * tileWidth + tileWidth / 2;
    const std::int64_t worldY = world ? *parsedY : std::int64_t(*parsedY) * tileHeight + tileHeight / 2;
    if (worldX < std::numeric_limits<int>::min() || worldX > std::numeric_limits<int>::max() ||
        worldY < std::numeric_limits<int>::min() || worldY > std::numeric_limits<int>::max()) return std::nullopt;
    return Point{static_cast<int>(worldX), static_cast<int>(worldY)};
}
}
