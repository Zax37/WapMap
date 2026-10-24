#pragma once
#ifdef WAPMAP_ENABLE_IMGUI
#include "window.h"
class winMeasureOptions : public cWindow {
    bool open{}, positionOnOpen{}, clearRequested{}, stopRequested{};
    WWD::Parser* document{};
public:
    winMeasureOptions();
    void Open() override;
    void Close() override;
    void DrawImGui() override;
    void Think() override;
    void OnDocumentChange() override { Close(); }
    bool isOpened() override { return open; }
};
#endif
