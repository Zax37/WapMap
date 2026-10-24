#pragma once
#ifdef WAPMAP_ENABLE_IMGUI
#include "window.h"
class winLogicBrowser : public cWindow {
    bool open{}, positionOnOpen{}, busy{};
    WWD::Parser* document{};
    std::string selectedName, actionName;
    gcn::Widget* pendingAction{};
public:
    winLogicBrowser();
    void Open() override;
    void Close() override;
    void Think() override;
    void DrawImGui() override;
    void OnDocumentChange() override { Close(); }
    bool isOpened() override { return open; }
    void SelectLogic(const std::string& name) { selectedName = name; }
};
#endif
