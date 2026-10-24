#pragma once
#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ImageImportData {
using Bytes = std::vector<unsigned char>;
using Palette = std::array<std::uint32_t, 256>;
struct Image {
    int width = 0, height = 0;
    std::array<std::uint32_t, 4> user{};
    std::uint32_t flags = 1;
    Bytes pixels;
    std::optional<Palette> palette;
};
inline std::uint32_t U32(std::span<const unsigned char> b, size_t p) {
    return b[p] | (std::uint32_t(b[p+1]) << 8) | (std::uint32_t(b[p+2]) << 16) | (std::uint32_t(b[p+3]) << 24);
}
inline int U16(std::span<const unsigned char> b, size_t p) { return b[p] | (b[p+1] << 8); }
inline bool Dimensions(int w, int h) { return w > 0 && h > 0 && w <= 16384 && h <= 16384 && std::int64_t(w) * h <= 64 * 1024 * 1024; }
inline std::optional<int> ID(std::string_view text, int maximum = 2147483647) {
    int n = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), n);
    if (parsed.ec != std::errc() || parsed.ptr != text.data() + text.size() || n < 0 || n > maximum) return std::nullopt;
    return n;
}
inline std::optional<int> FilenameID(std::string_view path, int maximum = 2147483647) {
    const auto slash = path.find_last_of("/\\");
    path.remove_prefix(slash == path.npos ? 0 : slash + 1);
    const auto first = path.find_first_of("0123456789");
    if (first == path.npos) return std::nullopt;
    const auto last = path.find_first_not_of("0123456789", first);
    return ID(path.substr(first, last == path.npos ? path.npos : last - first), maximum);
}
inline bool Name(std::string_view name, bool imageSet) {
    if (name.empty() || name.size() > 128) return false;
    size_t start = 0;
    for (size_t i = 0; i <= name.size(); ++i) {
        if (i == name.size() || name[i] == '/' || name[i] == '\\') {
            if ((!imageSet && i != name.size()) || i == start) return false;
            const auto part = name.substr(start, i-start);
            if (part == "." || part == ".." || part.back() == '.' || part.back() == ' ') return false;
            std::string upper(part);
            for (auto& c : upper) if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
            upper = upper.substr(0, upper.find('.'));
            if (upper == "CON" || upper == "PRN" || upper == "AUX" || upper == "NUL" ||
                (upper.size() == 4 && (upper.starts_with("COM") || upper.starts_with("LPT")) && upper[3] >= '1' && upper[3] <= '9')) return false;
            start = i + 1;
        } else if (static_cast<unsigned char>(name[i]) < 32 || std::string_view(":*?\"<>|").find(name[i]) != name.npos) return false;
    }
    return true;
}
inline std::optional<Image> PID(std::span<const unsigned char> bytes) {
    if (bytes.size() < 32) return std::nullopt;
    Image out;
    const auto w = U32(bytes, 8), h = U32(bytes, 12);
    if (w > 16384 || h > 16384 || !Dimensions(static_cast<int>(w), static_cast<int>(h))) return std::nullopt;
    out.width = w; out.height = h; out.flags = U32(bytes, 4);
    for (int i=0;i<4;++i) out.user[i] = U32(bytes, 16+4*i);
    size_t end = bytes.size();
    if (out.flags & 128) {
        if (end < 32 + 768) return std::nullopt;
        end -= 768; out.palette.emplace();
        for (int i=0;i<256;++i) (*out.palette)[i] = 0xFF000000u | (bytes[end+3*i] << 16) | (bytes[end+3*i+1] << 8) | bytes[end+3*i+2];
    }
    const size_t count = size_t(w)*h;
    out.pixels.reserve(count);
    size_t p=32;
    while (out.pixels.size() < count) {
        if (p >= end) return std::nullopt;
        unsigned n = bytes[p++];
        if (out.flags & 32) {
            if (n == 0) return std::nullopt;
            if (n > 128) {
                n -= 128;
                if (n > count-out.pixels.size()) return std::nullopt;
                out.pixels.insert(out.pixels.end(), n, 0);
            } else {
                if (n > end-p || n > count-out.pixels.size()) return std::nullopt;
                out.pixels.insert(out.pixels.end(), bytes.begin()+p, bytes.begin()+p+n); p += n;
            }
        } else if (n > 192) {
            n -= 192;
            if (p >= end || n > count-out.pixels.size()) return std::nullopt;
            out.pixels.insert(out.pixels.end(), n, bytes[p++]);
        } else out.pixels.push_back(static_cast<unsigned char>(n));
    }
    return out;
}
inline std::optional<Image> PCX(std::span<const unsigned char> bytes) {
    if (bytes.size() < 128+769 || bytes[0] != 10 || bytes[2] != 1 || bytes[3] != 8 || bytes[65] != 1 || bytes[bytes.size()-769] != 12) return std::nullopt;
    Image out;
    out.width = U16(bytes, 8)-U16(bytes, 4)+1;
    out.height = U16(bytes, 10)-U16(bytes, 6)+1;
    const int stride = U16(bytes, 66);
    if (!Dimensions(out.width,out.height) || stride < out.width) return std::nullopt;
    out.palette.emplace();
    const size_t end = bytes.size()-769;
    for (int i=0;i<256;++i) (*out.palette)[i] = 0xFF000000u | (bytes[end+1+3*i] << 16) | (bytes[end+2+3*i] << 8) | bytes[end+3+3*i];
    out.pixels.reserve(size_t(out.width)*out.height);
    size_t p=128, decoded=0, total=size_t(stride)*out.height;
    while (decoded < total) {
        if (p >= end) return std::nullopt;
        unsigned value=bytes[p++], n=1;
        if (value >= 192) { n=value & 63; if (!n || p>=end) return std::nullopt; value=bytes[p++]; }
        if (n > total-decoded) return std::nullopt;
        for (unsigned i=0;i<n;++i,++decoded) if (decoded % stride < static_cast<size_t>(out.width)) out.pixels.push_back(value);
    }
    return out;
}
inline Bytes EncodePID(const Image& image) {
    if (!Dimensions(image.width,image.height) || image.pixels.size() != size_t(image.width)*image.height) return {};
    Bytes out;
    const auto word = [&](std::uint32_t n) { for (int i=0;i<4;++i) out.push_back(static_cast<unsigned char>(n >> (8*i))); };
    word(0); word((image.flags & (1|2|4|8|16|64)) | 32); word(image.width); word(image.height);
    for (auto n:image.user) word(n);
    size_t p=0;
    while (p<image.pixels.size()) {
        const bool zero=image.pixels[p]==0;
        const size_t begin=p, maximum=zero?127:128;
        while (p<image.pixels.size() && p-begin<maximum && (image.pixels[p]==0)==zero) ++p;
        out.push_back(static_cast<unsigned char>(p-begin+(zero?128:0)));
        if (!zero) out.insert(out.end(),image.pixels.begin()+begin,image.pixels.begin()+p);
    }
    return out;
}
inline unsigned char Closest(std::uint32_t rgb, const Palette& palette, int transparent, bool alpha = true) {
    if (alpha && (rgb >> 24) < 128) return static_cast<unsigned char>(transparent);
    int best=0, distance=2147483647;
    for (int i=0;i<256;++i) {
        if (i==transparent) continue;
        const int dr=int((rgb>>16)&255)-int((palette[i]>>16)&255);
        const int dg=int((rgb>>8)&255)-int((palette[i]>>8)&255);
        const int db=int(rgb&255)-int(palette[i]&255);
        const int n=dr*dr+dg*dg+db*db;
        if (n<distance) { distance=n; best=i; if (!n) break; }
    }
    return static_cast<unsigned char>(best);
}
}
