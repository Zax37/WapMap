#pragma once
#include <algorithm>
#include <cstdint>

namespace TilePropertyGeometry {
inline bool ValidSize(int width, int height, bool mask) {
    return width > 0 && height > 0 && width <= 9999 && height <= 9999 &&
        (!mask || static_cast<int64_t>(width) * height <= 1024 * 1024);
}

// Copy the overlapping rectangle using the old stride; new cells stay Clear.
template <typename Cell, typename Write>
void PreserveMask(const Cell* source, int oldWidth, int oldHeight, int width, int height, Write write) {
    if (!source) return;
    for (int y = 0; y < std::min(height, oldHeight); ++y)
        for (int x = 0; x < std::min(width, oldWidth); ++x)
            write(x, y, source[y * oldWidth + x]);
}
}
