#pragma once
#ifdef WAPMAP_ENABLE_IMGUI
#include "window.h"
#include <array>
#include <unordered_map>
class cBrush;
class cTileImageSet;
class winTilePicker : public cWindow {
    bool open{}, positionOnOpen{}, stopRequested{}, pickRequested{}, brushPick{};
    WWD::Parser* document{};
    WWD::Plane* pendingPlane{};
    cTileImageSet* pendingSet{};
    cBrush* pendingBrush{};
    int pendingTile{}, previousHeight{};
    gcn::Widget* pendingAction{};
    struct Draft { std::array<char, 1025> text{}; std::string legacy; };
    std::unordered_map<gcn::Widget*, Draft> drafts;
    bool Eligible() const;
public:
    winTilePicker();
    void Open() override;
    void Close() override;
    void Think() override;
    void DrawImGui() override;
    void OnDocumentChange() override { Close(); }
    bool isOpened() override { return open; }
};
#endif
