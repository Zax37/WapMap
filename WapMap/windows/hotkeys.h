#ifndef H_WIN_HOTKEYS
#define H_WIN_HOTKEYS

#include "../../shared/gcnWidgets/wSlider.h"
#include "../../shared/gcnWidgets/wScrollArea.h"

#include "window.h"

class winHotkeys : public cWindow {
private:
    WIDG::Viewport *vp;
    int m_width, m_height;
    int m_hotkeysPerFrame, m_separatorX, m_descrWidth, m_descrSpacing;
    SHR::ScrollArea *saHotkeys;
    SHR::Container *conHotkeys;
    std::vector<std::pair<const char*, const char*>> m_vHotkeysList;

public:
    winHotkeys();

    virtual void Think() override;
    virtual void Draw(int piCode) override;

    void createHotkeysList();
};

#endif // H_WIN_HOTKEYS