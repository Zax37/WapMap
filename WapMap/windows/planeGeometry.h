#pragma once
#include <cstdint>
#include <limits>

inline bool PlaneArithmeticFits(int width, int height, int tileWidth, int tileHeight) {
    // Non-positive values are handled by the existing required-size dialogs.
    if (width <= 0 || height <= 0 || tileWidth <= 0 || tileHeight <= 0) return true;
    const auto limit = std::numeric_limits<int>::max();
    return std::int64_t(width) * height <= limit && std::int64_t(width) * tileWidth <= limit &&
           std::int64_t(height) * tileHeight <= limit;
}
