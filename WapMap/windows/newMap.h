#ifndef WAPMAP_NEWMAP_H
#define WAPMAP_NEWMAP_H

#include "../../shared/gcnwidgets/wButton.h"
#include "../../shared/gcnwidgets/wCheckbox.h"
#include "../../shared/gcnwidgets/wLabel.h"
#include "../../shared/gcnwidgets/wScrollArea.h"
#include "../../shared/gcnwidgets/wTextField.h"
#include "../../shared/gcnwidgets/wTree.h"
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
