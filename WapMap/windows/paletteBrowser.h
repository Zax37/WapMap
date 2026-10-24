#ifndef PALETTEBROWSER_H
#define PALETTEBROWSER_H

#include "window.h"
#include "../../shared/gcnWidgets/wListbox.h"

class winPaletteBrowser : public cWindow {
    SHR::ListBox *lbPalettes;
    WIDG::Viewport *vp;

    void UpdatePalettes();
#ifdef WAPMAP_ENABLE_IMGUI
    bool nativeOpen = false, positionOnOpen = false;
    std::string pendingPalette;
public:
    void DrawImGui() override;
    void Think() override;
    void Close() override;
    bool isOpened() override { return nativeOpen; }
#endif
public:
    winPaletteBrowser();
    ~winPaletteBrowser() override;

    void OnDocumentChange() override;
    void Open() override;
    void Draw(int piCode) override;
    void action(const ActionEvent &actionEvent) override;
};

#ifdef WAPMAP_ENABLE_IMGUI
class winFillColor : public cWindow {
    bool open{}, expanded{}, positionOnOpen{}, stopRequested{};
    bool resizePosition{};
    float nextX{}, nextY{};
    WWD::Parser* document{};
    WWD::Plane* pendingPlane{};
    int pendingColor = -1;
    bool Eligible() const;
public:
    winFillColor();
    void Open() override;
    void Close() override;
    void Think() override;
    void DrawImGui() override;
    void OnDocumentChange() override { Close(); }
    bool isOpened() override { return open; }
};
#endif
#endif //PALETTEBROWSER_H
