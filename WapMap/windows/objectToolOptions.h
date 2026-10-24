#pragma once
#ifdef WAPMAP_ENABLE_IMGUI
#include "window.h"
#include <array>
class winObjectToolOptions : public cWindow {
    bool duplicate, open{}, positionOnOpen{}, applyRequested{}, stopRequested{};
    WWD::Parser* document{};
    std::array<std::array<char, 32>, 4> values{};
    bool Valid() const;
public:
    explicit winObjectToolOptions(bool duplicate);
    void Open() override;
    void Close() override;
    void DrawImGui() override;
    void Think() override;
    void OnDocumentChange() override { Close(); }
    bool isOpened() override { return open; }
};
class winObjectBrushOptions : public cWindow {
    bool open{}, positionOnOpen{}, stopRequested{};
    WWD::Parser* document{};
    std::array<std::array<char, 32>, 2> scatter{};
public:
    winObjectBrushOptions();
    void Open() override;
    void Close() override;
    void Think() override;
    void DrawImGui() override;
    void OnDocumentChange() override { Close(); }
    bool isOpened() override { return open; }
};
class winObjectAreaOptions : public cWindow {
    bool open{}, positionOnOpen{}, closeRequested{};
    WWD::Parser* document{};
    gcn::Widget* pendingAction{};
public:
    winObjectAreaOptions();
    void Open() override;
    void Close() override;
    void Think() override;
    void DrawImGui() override;
    void OnDocumentChange() override { Close(); }
    bool isOpened() override { return open; }
};
#endif
