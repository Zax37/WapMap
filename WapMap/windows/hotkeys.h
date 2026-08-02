#ifndef H_WIN_HOTKEYS
#define H_WIN_HOTKEYS

#include "window.h"

class winHotkeys : public cWindow {
private:
    WIDG::Viewport *vp;
    int m_width = 520;
    int m_height = 40;
    const int m_separatorX = 168;
    const int m_descrWidth = m_width - 2*12 - m_separatorX;
    const int m_descrSpacing = 8;
public:
    winHotkeys();

    virtual void Think();

    virtual void Draw(int piCode);
};

#endif // H_WIN_HOTKEYS