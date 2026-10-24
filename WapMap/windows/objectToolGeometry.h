#pragma once
#include <cstdint>
#include <limits>
namespace ObjectToolGeometry {
inline bool Fits(std::int64_t value) {
    return value >= std::numeric_limits<int>::min() && value <= std::numeric_limits<int>::max();
}
inline bool CanOffset(int base, int step, int count) {
    const auto offset = std::int64_t(step) * count;
    return count >= 0 && Fits(offset) && Fits(std::int64_t(base) + offset);
}
}
