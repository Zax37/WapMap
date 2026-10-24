#pragma once
#ifdef WAPMAP_ENABLE_IMGUI
#include "window.h"
#include <array>
class winObjectSearch : public cWindow {
    bool open{}, positionOnOpen{}, caseSensitive{};
    int term{};
    WWD::Parser* document{};
    WWD::Plane* pendingPlane{};
    std::array<char, 1025> query{};
    std::vector<WWD::Object*> pendingSelection;
    bool navigate{};
public:
    winObjectSearch();
    void Open() override;
    void Close() override;
    void Think() override;
    void DrawImGui() override;
    void OnDocumentChange() override { Close(); }
    bool isOpened() override { return open; }
};
#endif
