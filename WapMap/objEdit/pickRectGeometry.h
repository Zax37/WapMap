#pragma once
#include "../windows/coordinateNavigation.h"
#include <algorithm>
#include <array>
#include <string_view>

namespace PickRectGeometry {
struct Rectangle { int x1, y1, x2, y2; };
inline Rectangle FromDrag(int startX, int startY, int x, int y) {
    return {std::min(startX, x), std::min(startY, y), std::max(startX, x), std::max(startY, y)};
}
inline std::array<bool, 4> Invalid(const std::array<std::string_view, 4>& text, bool allowEmpty, bool allowEmptyAxis) {
    std::array<int, 4> values{};
    std::array<bool, 4> invalid{};
    bool empty = allowEmpty;
    for (int i = 0; i < 4; ++i) {
        const auto value = CoordinateNavigation::Parse(text[i]);
        invalid[i] = !value || *value < 0;
        values[i] = value.value_or(0);
        empty = empty && !invalid[i] && values[i] == 0;
    }
    for (int axis = 0; axis < 2; ++axis) {
        const int end = axis + 2;
        const bool unordered = values[axis] > values[end] || invalid[axis] || invalid[end];
        const bool nullAxis = allowEmptyAxis && values[axis] == 0 && values[end] == 0;
        invalid[axis] = unordered || (values[axis] == 0 && !empty && !nullAxis);
        invalid[end] = unordered || (values[end] == 0 && !empty && !nullAxis);
    }
    return invalid;
}
}
