#pragma once
namespace AmbientTiming {
inline bool ValidInterval(int minimum, int maximum) {
    return minimum >= 0 && maximum > minimum;
}
}
