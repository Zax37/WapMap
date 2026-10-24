#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
namespace CannonGeometry {
struct Velocity { int x, y, worldX, magnitude, angle; };
inline bool Fits(std::int64_t value) {
    return value >= std::numeric_limits<int>::min() && value <= std::numeric_limits<int>::max();
}
inline int Normalize(int angle) { angle %= 360; return angle < 0 ? angle + 360 : angle; }
inline std::optional<Velocity> Components(int x, int y, bool right) {
    const auto worldX = right ? std::int64_t(x) : -std::int64_t(x);
    const double length = std::hypot(static_cast<double>(x), static_cast<double>(y));
    if (!Fits(worldX) || length > std::numeric_limits<int>::max()) return std::nullopt;
    const double degrees = std::atan2(static_cast<double>(y), static_cast<double>(x)) * 180.0 / 3.14159265358979323846;
    return Velocity{x, y, static_cast<int>(worldX), static_cast<int>(length), Normalize(static_cast<int>(std::lround(degrees)))};
}
inline std::optional<Velocity> Polar(int magnitude, int angle, bool right) {
    if (magnitude < 0) return std::nullopt;
    angle = Normalize(angle);
    const double radians = angle * 3.14159265358979323846 / 180.0;
    const int x = static_cast<int>(magnitude * std::cos(radians)), y = static_cast<int>(magnitude * std::sin(radians));
    auto result = Components(x, y, right);
    if (result) { result->magnitude = magnitude; result->angle = angle; }
    return result;
}
inline std::optional<Velocity> Drag(int x, int y, int originX, int originY, bool right, bool snap) {
    auto dx = std::int64_t(x) - originX, dy = std::int64_t(y) - originY;
    if (snap) {
        const double ratio = dy ? static_cast<double>(dx) / dy : 2.0;
        if (ratio >= -0.5 && ratio <= 0.5) dx = 0;
        else if (ratio > 1.5 || ratio < -1.5) dy = 0;
        else {
            const auto distance = std::min(std::abs(dx), std::abs(dy));
            dx = dx < 0 ? -distance : distance; dy = dy < 0 ? -distance : distance;
        }
    }
    const auto localX = right ? dx : -dx;
    if (!Fits(localX) || !Fits(dy)) return std::nullopt;
    return Components(static_cast<int>(localX), static_cast<int>(dy), right);
}
}
