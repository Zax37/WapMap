#pragma once
#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>
#include <vector>

namespace MapTextLayout {
inline int Frame(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A' + 1;
    if (c >= 'a' && c <= 'z') return c - 'a' + 1;
    if (c >= '0' && c <= '9') return c - '0' + 27;
    if (c == '\'' || c == '`') return 37;
    constexpr std::string_view symbols = ",.!?=<>()[]@#$%^&*";
    const auto index = symbols.find(c);
    return index == symbols.npos ? -1 : 37 + static_cast<int>(index);
}
struct Glyph { int frame, x, y; };
struct Layout { std::vector<Glyph> glyphs; int width, height; };
inline bool Fits(std::int64_t value) {
    return value >= std::numeric_limits<int>::min() && value <= std::numeric_limits<int>::max();
}
template<class Width>
std::optional<Layout> Build(std::string_view text, int originX, int originY, int alignment, Width width) {
    struct Pending { int frame; size_t line; std::int64_t offset; int lower; };
    std::vector<Pending> pending;
    std::vector<std::int64_t> lines{0};
    for (const char c : text) {
        if (c == '\n') { lines.push_back(0); continue; }
        if (c == ' ') { lines.back() += 5; continue; }
        const int frame = Frame(c);
        if (frame < 0) continue;
        pending.push_back({frame, lines.size() - 1, lines.back(), c == '.' ? 3 : c == ',' ? 6 : 0});
        lines.back() += std::max(0, width(frame));
        if (!Fits(lines.back() + 20)) return std::nullopt;
    }
    if (pending.empty()) return std::nullopt;
    const auto longest = *std::max_element(lines.begin(), lines.end());
    const auto height = std::int64_t(lines.size()) * 10 + 10;
    if (!Fits(longest + 20) || !Fits(height) || !Fits(std::int64_t(originX) + longest + 20) || !Fits(std::int64_t(originY) + height)) return std::nullopt;
    Layout result{{}, static_cast<int>(longest + 20), static_cast<int>(height)};
    for (const auto& glyph : pending) {
        const auto gap = longest - lines[glyph.line];
        const auto shift = alignment == 1 ? gap / 2 : alignment == 2 ? gap : 0;
        const auto x = std::int64_t(originX) + 10 + glyph.offset + shift - (glyph.frame == 9 ? 2 : 0);
        const auto y = std::int64_t(originY) + 10 + std::int64_t(glyph.line) * 10 + glyph.lower;
        if (!Fits(x) || !Fits(y)) return std::nullopt;
        result.glyphs.push_back({glyph.frame, static_cast<int>(x), static_cast<int>(y)});
    }
    return result;
}
}
