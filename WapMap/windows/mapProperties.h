#pragma once
#ifdef WAPMAP_ENABLE_IMGUI
#include "window.h"
#include <array>

class winMapProperties : public cWindow {
    bool open{}, positionOnOpen{}, compression{}, useZ{};
    WWD::Parser* document{};
    std::array<std::array<char, 1025>, 7> fields{};
public:
    winMapProperties();
    void Open() override;
    void Close() override;
    void DrawImGui() override;
    void OnDocumentChange() override { Close(); }
    bool isOpened() override { return open; }
};
#endif
