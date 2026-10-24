#pragma once
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace ElevatorPathGeometry {
using Steps = std::vector<std::pair<int, int>>;
struct Point { std::int64_t x, y; };
inline bool Fits(std::int64_t n) {
    return n >= std::numeric_limits<int>::min() && n <= std::numeric_limits<int>::max();
}
inline Point Move(int direction, int amount) {
    const std::int64_t n = amount;
    return {(direction == 1 || direction == 4 || direction == 7) ? -n :
            (direction == 3 || direction == 6 || direction == 9) ? n : 0,
            direction >= 7 && direction <= 9 ? -n : direction >= 1 && direction <= 3 ? n : 0};
}
inline std::optional<Point> End(const Steps& steps, int x, int y) {
    if (steps.empty() || steps.size() > 8) return std::nullopt;
    Point p{x, y};
    for (const auto& [direction, amount] : steps) {
        if (direction < 1 || direction > 9 || amount < 0) return std::nullopt;
        const auto delta = Move(direction, amount);
        p.x += delta.x; p.y += delta.y;
        if (!Fits(p.x) || !Fits(p.y)) return std::nullopt;
    }
    return p;
}
inline int Direction(std::int64_t x, std::int64_t y) {
    return (y < 0 ? 7 : y > 0 ? 1 : 4) + (x < 0 ? 0 : x > 0 ? 2 : 1);
}
inline std::optional<std::pair<int, int>> Drag(std::int64_t dx, std::int64_t dy) {
    // Differences originate from two int coordinates.
    const auto ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
    if (!ax && !ay) return std::nullopt;
    int dir;
    std::int64_t distance;
    if (2 * ax > 5 * ay) { dir = dx > 0 ? 6 : 4; distance = ax; }
    else if (2 * ax < ay) { dir = dy > 0 ? 2 : 8; distance = ay; }
    else { dir = Direction(dx, dy); distance = ax > ay ? ax : ay; }
    if (!Fits(distance)) return std::nullopt;
    return std::pair{dir, static_cast<int>(distance)};
}
struct Closure { Steps steps; bool removed; };
inline std::optional<Closure> Close(Steps steps, int x, int y) {
    if (!End(steps, x, y)) return std::nullopt;
    bool removed = false;
    while (!steps.empty()) {
        const auto end = End(steps, x, y);
        if (!end) return std::nullopt;
        auto dx = static_cast<std::int64_t>(x) - end->x;
        auto dy = static_cast<std::int64_t>(y) - end->y;
        Steps result = steps;
        bool representable = true;
        while (dx || dy) {
            const auto ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
            const auto distance = dx && dy ? (ax < ay ? ax : ay) : (dx ? ax : ay);
            if (!Fits(distance)) { representable = false; break; }
            const int dir = Direction(dx, dy);
            if (!result.empty() && result.back().first == dir && Fits(static_cast<std::int64_t>(result.back().second) + distance))
                result.back().second += static_cast<int>(distance);
            else result.emplace_back(dir, static_cast<int>(distance));
            const auto delta = Move(dir, static_cast<int>(distance));
            dx -= delta.x; dy -= delta.y;
        }
        if (representable && result.size() <= 8 && End(result, x, y)) return Closure{std::move(result), removed};
        steps.pop_back(); removed = true;
    }
    return Closure{Steps{{5, 0}}, true};
}
}
