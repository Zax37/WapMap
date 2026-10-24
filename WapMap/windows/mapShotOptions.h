#pragma once
#ifdef WAPMAP_ENABLE_IMGUI
#include "window.h"
#include <array>
class winMapShotOptions : public cWindow {
    bool open{}, positionOnOpen{}, busy{}, browseRequested{}, saveRequested{};
    WWD::Parser* document{};
    std::array<char, 4097> path{};
public:
    winMapShotOptions();
    void Open() override;
    void Close() override;
    void Think() override;
    void DrawImGui() override;
    void OnDocumentChange() override { Close(); }
    bool isOpened() override { return open; }
};
#endif
