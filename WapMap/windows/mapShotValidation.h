#pragma once
#include <Windows.h>
#include <filesystem>
#include <cmath>
#include <limits>
#include <optional>
#include <utility>
#include <array>
#include <algorithm>
#include <cstdint>
namespace MapShotValidation {
struct ChunkPlan {
    int width, height, chunkWidth, chunkHeight, columns, rows;
    std::array<int, 4> Bounds(int index) const {
        const int x = (index % columns) * chunkWidth, y = (index / columns) * chunkHeight;
        return {x, y, std::min(chunkWidth, width - x), std::min(chunkHeight, height - y)};
    }
};
inline std::optional<ChunkPlan> Plan(int width, int height, int limit = 2048) {
    if (width <= 0 || height <= 0 || limit <= 0) return {};
    const int cw = std::min(width, limit), ch = std::min(height, limit);
    const int columns = 1 + (width - 1) / cw, rows = 1 + (height - 1) / ch;
    if (std::int64_t(columns) * rows > std::numeric_limits<int>::max()) return {};
    return ChunkPlan{width, height, cw, ch, columns, rows};
}
inline bool CanSave(const std::string& path) {
    if (path.empty()) return false;
    std::error_code error;
    const std::filesystem::path destination(path);
    auto parent = destination.parent_path();
    if (parent.empty()) parent = std::filesystem::current_path(error);
    if (error || destination.filename().empty()) return false;
    const DWORD attributes = GetFileAttributesW(destination.c_str());
    if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_READONLY))) return false;
    return std::filesystem::is_directory(parent, error) && !error;
}
inline std::optional<std::pair<int, int>> Dimensions(int width, int height, double scale) {
    if (width <= 0 || height <= 0 || !std::isfinite(scale) || scale <= 0 || scale > 100) return {};
    const double w = width * (scale / 100), h = height * (scale / 100);
    if (w < 1 || h < 1 || w > std::numeric_limits<int>::max() || h > std::numeric_limits<int>::max()) return {};
    return std::pair{static_cast<int>(w), static_cast<int>(h)};
}
}
