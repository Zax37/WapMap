#ifndef H_C_APPMENU
#define H_C_APPMENU

#include "globals.h"
#include "../shared/gcnWidgets/wContext.h"
#include "cRulers.h"

#define APPMEN_FILE_NEW         1
#define APPMEN_FILE_OPEN        2
#define APPMEN_FILE_RECENT      3
#define APPMEN_FILE_CLOSED      4
#define APPMEN_FILE_SAVE        5
#define APPMEN_FILE_SAVEAS      6
#define APPMEN_FILE_EXPORT      7
#define APPMEN_FILE_CLOSE       8
#define APPMEN_FILE_CLOSEALL    9
#define APPMEN_FILE_EXIT       10
//file mru positions as ID
//file closed as mdi

#define APPMEN_EDIT_WORLD       1
#define APPMEN_EDIT_PLANES      2
#define APPMEN_EDIT_TILEPROP    3
#define APPMEN_EDIT_WORLDSCRIPT 4

#define APPMEN_EDITT_PENCIL   100
#define APPMEN_EDITT_BRUSH    101
#define APPMEN_EDITT_FILL     102
#define APPMEN_EDITT_PROP     103
#define APPMEN_EDITM_SEARCH   200
#define APPMEN_EDITM_NEWOBJ   201
#define APPMEN_EDITM_BLANKOBJ 202

#define APPMEN_VIEW_RULERS     1
#define APPMEN_VIEW_PLANES     2
#define APPMEN_VIEW_TILEPROP   3
#define APPMEN_VIEW_GUIDELINES 4

#define APPMEN_PLANEVIS_BORDER  1
#define APPMEN_PLANEVIS_GRID    2
#define APPMEN_PLANEVIS_OBJECTS 3

#define APPMEN_TOOLS_MAPSHOT  2
#define APPMEN_TOOLS_PLAY     3
#define APPMEN_TOOLS_MEASURE  4
#define APPMEN_TOOLS_STATS    5

#define APPMEN_ASSETS_TILES     1
#define APPMEN_ASSETS_IMAGESETS 2
#define APPMEN_ASSETS_ANIMS     3
#define APPMEN_ASSETS_SOUNDS    4
#define APPMEN_ASSETS_LOGICS    5

#define APPMEN_WM_SETTINGS 1
#define APPMEN_WM_ABOUT    2
#define APPMEN_WM_UPDATE   3
#define APPMEN_WM_README   4
#define APPMEN_WM_SITE     5

#define LAY_APP_CUT_Y  168
#define LAY_APPMENU_X  36
#define LAY_APPMENU_Y  1
#define LAY_APPMENU_H  30
#define LAY_MODEBAR_Y  LAY_APPMENU_Y + LAY_APPMENU_H
#define LAY_MODEBAR_H  33
#define LAY_MDI_Y      LAY_MODEBAR_Y + LAY_MODEBAR_H
#define LAY_MDI_H      25
#define LAY_VIEWPORT_Y LAY_MDI_Y + LAY_MDI_H
#define LAY_STATUS_H   29
#define LAY_APP_BUTTONS_COUNT 3
#define LAY_APP_BUTTON_W (LAY_APPMENU_H + 10)

class cAppMenu_Entry {
private:
    std::string strLabel;
    Gfx16Icons iIcon;
    bool bEnabled;
    SHR::Context *hContext;
    float fTimer;
public:
    cAppMenu_Entry(std::string lab, Gfx16Icons ico);

    ~cAppMenu_Entry();

    SHR::Context *GetContext() { return hContext; };

    void SetIcon(Gfx16Icons n) { iIcon = n; };

    Gfx16Icons GetIcon() { return iIcon; };

    void SetEnabled(bool b) { bEnabled = b; };

    bool IsEnabled() { return bEnabled; };

    void SetLabel(std::string n) { strLabel = n; };

    std::string GetLabel() { return strLabel; };

    int Render(int x, int y, bool bFocused);

    int GetWidth();
};

enum enAppMenu_Entries {
    AppMenu_File = 0,
    AppMenu_Edit,
    AppMenu_View,
    AppMenu_Tools,
    AppMenu_Assets,
    AppMenu_WapMap,
    AppMenu_EntryCount
};

class cAppMenu : public gcn::Widget,
                        gcn::MouseListener,
                        gcn::KeyListener,
                        gcn::FocusListener,
                        gcn::ActionListener,
                        gcn::SelectionListener {
private:
    cAppMenu_Entry *hEntries[AppMenu_EntryCount];
    SHR::Context *conOpenMRU, *conPlanesVisibilityList, *conPlaneVisibility;
    int iSelected, iHovered, iOpened;
    bool bEnabled;

public:
    cAppMenu();

    ~cAppMenu();

    void setSelected(int i) { iSelected = i; }

    void switchTo(int i);

    void closeCurrent();

    void draw(Graphics* graphics) override;

    void keyPressed(KeyEvent& keyEvent) override;

    void mouseMoved(MouseEvent& mouseEvent) override;

    void mousePressed(MouseEvent& mouseEvent) override;

    void mouseExited(MouseEvent& mouseEvent) override;

    bool IsEnabled() { return bEnabled; };

    void SetEnabled(bool b) { bEnabled = b; };

    int GetHeight() { return mDimension.height; };

    void action(const gcn::ActionEvent &actionEvent) override;

    void focusLost(const FocusEvent& event) override;

    void valueChanged(const SelectionEvent &event) override;

    SHR::Context *GetContext(enAppMenu_Entries x) { return hEntries[x]->GetContext(); };

    void NotifyRulersSwitch();

    void SyncDocumentSwitched();

    void SyncDocumentClosed();

    void SyncDocumentOpened();

    void SyncRecentlyClosedRebuild();

    void SyncPlaneSwitched();

    void SyncModeSwitched();

    void SyncPlanes();

    void SyncMRU();

    //implemented
    void SyncPlaneVisibility();

    void SyncPlaneSelectedVisibility();

    void SetToolSpecificEnabled(bool b);
};

#endif
