#include "hotkeys.h"

#include "../shared/commonFunc.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"

#define WIN_HOTKEYS_CATEGORIES 4
#define WIN_HOTKEYS_COUNT {9, 15, 21, 5}

inline const char* getStr(const wchar_t* str) {
    return GETL2SV("Win_Hotkeys", str);
}

void winHotkeys::createHotkeysList() {
    int hotkeysCount[] = WIN_HOTKEYS_COUNT;

    m_vHotkeysList.reserve(100);
    for (int i = 0; i < WIN_HOTKEYS_CATEGORIES; i++) {
        wchar_t buf[64], buf2[64];
        wsprintfW(buf, L"Category_%d", i);
        std::pair<const char*, const char*> category = {"", getStr(buf)};
        m_vHotkeysList.push_back(category);
        for (int k = 0; k < hotkeysCount[i]; k++) {
            wsprintfW(buf, L"Key_%d_%d", i, k);
            wsprintfW(buf2, L"Key_Descr_%d_%d", i, k);
            std::pair<const char*, const char*> hotkey = {getStr(buf), getStr(buf2)};
            m_vHotkeysList.push_back(hotkey);
        }
    }
}

winHotkeys::winHotkeys() : cWindow(getStr(L"WinCaption")) {
    m_hotkeysPerFrame = 15;
    m_descrSpacing = 23;

    m_height = m_descrSpacing * m_hotkeysPerFrame + 26;
    m_width = m_height * 1.61;
    myWin.setDimension(gcn::Rectangle(0, 0, m_width, m_height));

    m_separatorX = m_width * 0.3;
    m_descrWidth = m_width - 2*10 - m_separatorX;

    conHotkeys = new SHR::Container();
    conHotkeys->setDimension(gcn::Rectangle(0, 0, 100, 100));
    conHotkeys->setOpaque(0);
    saHotkeys = new SHR::ScrollArea(conHotkeys, SHR::ScrollArea::SHOW_NEVER, SHR::ScrollArea::SHOW_AUTO);
    saHotkeys->setDimension(gcn::Rectangle(0, 0, m_width - 2, m_descrSpacing * m_hotkeysPerFrame));
    saHotkeys->setOpaque(0);
    myWin.add(saHotkeys, 0, 8);
    conHotkeys->setWidth(saHotkeys->getChildrenArea().width);

    createHotkeysList();

    conHotkeys->setHeight(m_vHotkeysList.size() * (m_descrSpacing + 4));

    vp = new WIDG::Viewport(this, 0);
    myWin.add(vp, 0, 0);
}

void winHotkeys::Think() {

}

void winHotkeys::Draw(int piCode) {
    int dx, dy;
    myWin.getAbsolutePosition(dx, dy);

    int ymax = dy + m_height - 12;
    dy += 28;

    int xline = dx + m_separatorX;
    int endx = dx + m_width - 16;

    unsigned char alpha = myWin.getAlpha();
    DWORD colDark = SETA(GV->colLineDark, alpha);
    DWORD colBright = SETA(GV->colLineBright, alpha);

    int startCount = saHotkeys->getVerticalScrollAmount() / 28;
    int count = -1;

    for (auto hotkey : m_vHotkeysList) {
        if (dy >= ymax)
            break;
        if (++count < startCount)
            continue;
        bool isMaxScrolled = saHotkeys->getVerticalScrollAmount() == saHotkeys->getVerticalMaxScroll();
        if (isMaxScrolled && count + m_hotkeysPerFrame < m_vHotkeysList.size()) {
            count++;
            continue;
        }
        if (strlen(hotkey.first) == 0) {
            SHR::SetQuad(&q, SETA(0x151515, alpha), dx, dy - 4, endx, dy + m_descrSpacing - 4);
            hge->Gfx_RenderQuad(&q);
            GV->fntMyriad16->printfb(dx + 10, dy, m_width - 20, 20, HGETEXT_CENTER, 0, hotkey.second);
        }
        else {
            GV->fntMyriad16->printfb(dx + 10, dy, m_separatorX - 12, 20, HGETEXT_LEFT, 0, hotkey.first);
            GV->fntMyriad16->printfb(dx + 10 + m_separatorX, dy, m_descrWidth, 20, HGETEXT_LEFT, 0, hotkey.second);
            hge->Gfx_RenderLine(xline - 1, dy - 4, xline - 1, dy + 18, colDark);
            hge->Gfx_RenderLine(xline, dy - 4, xline, dy + 18, colBright);
        }
        dy += m_descrSpacing;
        hge->Gfx_RenderLine(dx, dy - 4, endx, dy - 4, colDark);
        hge->Gfx_RenderLine(dx, dy - 3, endx, dy - 3, colBright);
    }
}