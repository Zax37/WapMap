#ifndef H_WIN_ABOUT
#define H_WIN_ABOUT

#include "window.h"

class winAbout : public cWindow {
private:
    WIDG::Viewport *vp;
#ifdef WAPMAP_ENABLE_IMGUI
    bool imguiOpen{};
    bool imguiPositionOnOpen{};
#endif
public:
    winAbout();

#ifdef WAPMAP_ENABLE_IMGUI
    void Open() override;
    void Close() override;
    void DrawImGui() override;
    bool isOpened() override { return imguiOpen; }
#endif

    virtual void Draw(int piCode);

    virtual void Think();
};

#endif // H_WIN_ABOUT
