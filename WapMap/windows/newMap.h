#ifndef WAPMAP_NEWMAP_H
#define WAPMAP_NEWMAP_H

#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wCheckbox.h"
#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wScrollArea.h"
#include "../../shared/gcnWidgets/wTextField.h"
#include "../../shared/gcnWidgets/wTree.h"
#include "window.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include <array>
#endif

class winNewMap : public cWindow {
    SHR::Lab labName, labAuthor, labPlaneSize, labX, labTarget, labTargetValue;
    SHR::TextField tfName, tfAuthor, tfPlaneWidth, tfPlaneHeight;
    SHR::Tree targetGameTree;
    SHR::ScrollArea scrollArea;
    SHR::CBox cbAddBonusPlanes;
    SHR::But butOK;
#ifdef WAPMAP_ENABLE_IMGUI
    bool nativeOpen{}, positionOnOpen{}, resetTreeScroll{}, pendingCreate{};
    std::array<std::array<char, 257>, 4> fields{};
#endif

public:
    winNewMap();

    void Open() override;
    void Close() override;
#ifdef WAPMAP_ENABLE_IMGUI
    void DrawImGui() override;
    bool isOpened() override { return nativeOpen; }
#endif

    void Draw(int piCode) override;

    void Think() override;

    void action(const ActionEvent &actionEvent) override;
};

#endif //WAPMAP_NEWMAP_H
