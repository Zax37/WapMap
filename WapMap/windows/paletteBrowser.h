#ifndef PALETTEBROWSER_H
#define PALETTEBROWSER_H

#include "window.h"
#include "../../shared/gcnwidgets/wListbox.h"

class winPaletteBrowser : public cWindow {
    SHR::ListBox *lbPalettes;
    WIDG::Viewport *vp;

    void UpdatePalettes();
public:
    winPaletteBrowser();
    ~winPaletteBrowser() override;

    void OnDocumentChange() override;
    void Open() override;
    void Draw(int piCode) override;
    void action(const ActionEvent &actionEvent) override;
};

#endif //PALETTEBROWSER_H
