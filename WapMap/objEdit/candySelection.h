#pragma once
#include <optional>
namespace CandySelection {
struct Choice { bool animated; int frame; };
inline std::optional<Choice> Image(int count, std::optional<int> frame) {
    if (count <= 0 || (frame && *frame < 0)) return std::nullopt;
    return Choice{!frame && count > 1, frame.value_or(-1)};
}
}
