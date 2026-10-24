#pragma once
#include <array>
namespace EnemyGeometry {
inline std::array<bool, 2> InvalidDestination(int x, int y, int width, int height) {
    return {x <= 0 || x >= width, y <= 0 || y >= height};
}
}
