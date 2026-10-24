#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace MapStatistics {
using Counts = std::array<std::int64_t, 6>;
inline void AddTile(Counts& counts, bool invisible, bool filled, std::span<const unsigned char> attributes, bool known = true) {
    if (invisible) { ++counts[0]; return; }
    if (filled) { ++counts[1]; return; }
    if (!known || attributes.empty()) { ++counts[5]; return; }
    std::array<bool, 6> seen{};
    for (auto attribute : attributes) seen[attribute < 5 ? attribute : 5] = true;
    for (size_t i = 0; i < seen.size(); ++i) if (seen[i]) ++counts[i];
}
inline constexpr std::array<std::int64_t, 10> Values{100,500,1500,2500,2500,5000,7500,10000,15000,25000};
inline std::optional<size_t> Treasure(std::string_view imageSet) {
    std::string name(imageSet);
    for (auto& c : name) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    constexpr std::array<std::string_view, 10> names{"coins","goldbars","rings","chalices","necklace","crosses","scepters","geckos","crowns","jeweledskull"};
    for (size_t i = 0; i < names.size(); ++i) if (name.find(names[i]) != name.npos) return i;
    return std::nullopt;
}
inline std::int64_t Score(const std::array<std::int64_t, 10>& treasures) {
    std::int64_t result = 0;
    for (size_t i = 0; i < treasures.size(); ++i) result += treasures[i] * Values[i];
    return result;
}
}
