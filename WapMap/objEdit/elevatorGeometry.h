#pragma once
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <cmath>

namespace ElevatorGeometry {
inline bool Fits(std::int64_t n) {
    return n >= std::numeric_limits<int>::min() && n <= std::numeric_limits<int>::max();
}
inline std::optional<std::pair<int, int>> Range(int origin, int distance, bool negative) {
    if (distance < 0) return std::nullopt;
    const auto end = static_cast<std::int64_t>(origin) + (negative ? -static_cast<std::int64_t>(distance) : distance);
    if (!Fits(end)) return std::nullopt;
    return negative ? std::pair{static_cast<int>(end), origin} : std::pair{origin, static_cast<int>(end)};
}
inline std::optional<int> Scale(int value, double ratio, bool sourceX) {
    if (!std::isfinite(ratio) || ratio == 0) return std::nullopt;
    const double result = std::round(sourceX ? value / ratio : value * ratio);
    if (!std::isfinite(result) || result < std::numeric_limits<int>::min() || result > std::numeric_limits<int>::max())
        return std::nullopt;
    return static_cast<int>(result);
}
}
