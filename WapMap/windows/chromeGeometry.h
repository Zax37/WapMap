#pragma once
#include <algorithm>
#include <cmath>
namespace ChromeGeometry {
inline float Zoom(float value) {
    if (!std::isfinite(value)) return 1;
    value = std::clamp(value, -5.f, 5.f);
    return value >= 0 ? 1 + value / 5 : 1 + value / 5 * .9f;
}
}
