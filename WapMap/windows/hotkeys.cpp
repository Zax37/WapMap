#include "hotkeys.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"

static const std::vector<std::pair<const char*, const char*>> hotkeysList = {
    {"Z", "zoom tool"},
    {"Escape", "reset tool / close window / cancel current action"},
    {"Space (hold to toggle)", "hand navigation tool"},
    {"Alt", "ghost preview clipboard content"},
    {"Ctrl+F", "search objects"},
    {"Ctrl+N", "new document"},
    {"Ctrl+O", "open document"},
    {"Ctrl+S", "save document"},
    {"Ctrl+Shift+S", "save document as"},
    {"Ctrl+W", "close document"},
    {"Ctrl+Shift+T", "open most recent document"},
    {"Ctrl+R", "show rulers"},
    {"Ctrl+G", "show grid"},
    {"Alt+G", "open go to window"},
    {"Ctrl+M", "switch edit modes"},
    {"F11", "toggle fullscreen"},
    {"E", "edit selected object"},
    {"P", "open properties of selected object"},
    {"M", "mirror selected object(-s)"},
    {"I", "invert selected object(-s)"},
    {"G", "grab selected object(-s)"},
    {"Shift (hold to toggle)", "align to XY axis when moving objects / add to selection"},
    {"Ctrl (hold to toggle)", "align to grid or tile center when moving objects"},
    {"Tab", "move focus to next UI element"},
    {"PageUp/PageDown", "increase selected object(-s) Z by 10"},
    {"Ctrl+PageUp/PageDown", "increase selected object(-s) Z by 100"},
    {"Shift+PageUp/PageDown", "increase selected object(-s) Z by 1000"},
    {"Ctrl+D", "duplicate selected object(-s)"},
    {"Delete", "delete selected object(-s)"},
    {"Ctrl+X", "cut selected object(-s)"},
    {"Ctrl+C", "copy selected object(-s)"},
    {"Ctrl+V", "paste selected object(-s)"},
    {"Ctrl+A", "select all tiles/objects"}
};

winHotkeys::winHotkeys() : cWindow("Hotkeys") {
    for (auto hotkey : hotkeysList) {
        m_height += GV->fntMyriad16->GetStringBlockHeight(m_descrWidth, hotkey.second) + m_descrSpacing;
    }
    m_height -= 14;
    myWin.setDimension(gcn::Rectangle(0, 0, m_width, m_height));
    vp = new WIDG::Viewport(this, 0);
    myWin.add(vp, 0, 0);
}

void winHotkeys::Think() {

}

void winHotkeys::Draw(int piCode) {
    int dx, dy;
    myWin.getAbsolutePosition(dx, dy);
    dy += 28;

    unsigned char alpha = myWin.getAlpha();
    int xline = dx + m_separatorX;
    hge->Gfx_RenderLine(xline, dy - 4, xline, dy + m_height - 28 + 1, SETA(GV->colLineBright, alpha));

    dx += 12;

    int n = 0, gridEndX = dx + m_width - 12;

    for (auto hotkey : hotkeysList) {
        GV->fntMyriad16->printfb(dx, dy, m_separatorX - 12, 200, HGETEXT_LEFT, 0, hotkey.first);
        GV->fntMyriad16->printfb(dx + m_separatorX, dy, m_descrWidth, 200, HGETEXT_LEFT, 0, hotkey.second);
        dy += GV->fntMyriad16->GetStringBlockHeight(m_descrWidth, hotkey.second) + m_descrSpacing;
        if (++n < hotkeysList.size()) {
            hge->Gfx_RenderLine(dx - 12, dy - 4, gridEndX, dy - 4, SETA(GV->colLineBright, alpha));
        }
    }
}