#pragma once
#include <algorithm>
#include <string_view>

namespace TimedSeries {
inline int Delay(std::string_view logic, int storedDelay, std::string_view family) {
    const auto preset = [logic](std::string_view prefix) {
        return logic.size() == prefix.size() + 1 && logic.starts_with(prefix) &&
            logic.back() >= '2' && logic.back() <= '4';
    };
    if (preset(family) || (family == "SawBlade" && preset("FloorSaw")))
        return (logic.back() - '1') * 750;
    return std::max(0, storedDelay);
}
}
