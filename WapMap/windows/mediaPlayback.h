#pragma once
#include <algorithm>
#include <cmath>
#include <string>

namespace MediaPlayback {
template <typename Duration>
void Advance(int count, int& frame, double& elapsed, bool& playing, bool loop, double deltaMs, Duration duration) {
    if (count <= 0) { frame = -1; elapsed = 0; playing = false; return; }
    frame = std::clamp(frame, 0, count - 1);
    if (!playing || !std::isfinite(deltaMs) || deltaMs < 0) return;
    elapsed += deltaMs;
    if (loop) {
        double cycle = 0;
        for (int i = 0; i < count; ++i) cycle += std::max(1, duration(i));
        if (elapsed >= cycle) elapsed = std::fmod(elapsed, cycle);
    }
    for (int steps = 0; steps < count && elapsed >= std::max(1, duration(frame)); ++steps) {
        elapsed -= std::max(1, duration(frame));
        if (frame + 1 < count) ++frame;
        else if (loop) frame = 0;
        else { playing = false; elapsed = 0; break; }
    }
}

template <typename Exists>
std::string ImageSet(std::string name, std::string declared, bool gruntz, Exists exists) {
    std::string value = declared.empty() ? name : declared;
    const std::string prefix = gruntz ? "IMAGEZ_" : "IMAGES_";
    for (size_t p = value.find(prefix); p != std::string::npos; p = value.find(prefix)) value.erase(p, prefix.size());
    if (!declared.empty() && value.compare(0, 5, "LEVEL") == 0) {
        size_t end = 5;
        while (end < value.size() && value[end] >= '0' && value[end] <= '9') ++end;
        value.erase(5, end - 5);
    }
    if (exists(value)) return value;
    auto split = value.rfind('_');
    if (split != std::string::npos && exists(value.substr(0, split))) return value.substr(0, split);
    if (gruntz && split != std::string::npos) {
        value.insert(split, "_SOUTH");
        if (!exists(value) && !value.empty() && exists(value.substr(0, value.size() - 1))) value.pop_back();
    }
    return value;
}
}
