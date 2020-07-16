#ifndef WAPMAP_NEWMAP_H
#define WAPMAP_NEWMAP_H

#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wCheckbox.h"
#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wScrollArea.h"
#include "../../shared/gcnWidgets/wTextField.h"
#include "../../shared/gcnWidgets/wTree.h"
#include "window.h"

class winNewMap : public cWindow {
    SHR::Lab labName, labAuthor, labPlaneSize, labX, labTarget, labTargetValue;
    SHR::TextField tfName, tfAuthor, tfPlaneWidth, tfPlaneHeight;
    SHR::Tree targetGameTree;
    SHR::ScrollArea scrollArea;
    SHR::CBox cbAddBonusPlanes;
    SHR::But butOK;

public:
    winNewMap();

    void Open() override;

    void Draw(int piCode) override;

    void Think() override;

    void action(const ActionEvent &actionEvent) override;
};

#endif //WAPMAP_NEWMAP_H
