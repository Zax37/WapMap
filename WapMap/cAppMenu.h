#ifndef H_C_APPMENU
#define H_C_APPMENU

#include "globals.h"
#include "../shared/gcnWidgets/wContext.h"
#include "cRulers.h"

#define APPMEN_FILE_NEW         1
#define APPMEN_FILE_OPEN        2
#define APPMEN_FILE_MRU         3
#define APPMEN_FILE_CLOSED      4
#define APPMEN_FILE_SAVE        5
#define APPMEN_FILE_SAVEAS      6
#define APPMEN_FILE_EXPORT      7
#define APPMEN_FILE_CLOSE       8
#define APPMEN_FILE_CLOSEALL    9
//file mru positions as ID
//file closed as mdi

#define APPMEN_EDIT_WORLD       1
#define APPMEN_EDIT_TILEPROP    2
#define APPMEN_EDIT_WORLDSCRIPT 3

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

#define APPMEN_PLANE_MGR    1
#define APPMEN_PLANE_ACTIVE 2

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

#define LAY_APPMENU_Y  27
#define LAY_APPMENU_H  23
#define LAY_MODEBAR_Y  54
#define LAY_MODEBAR_H  31
#define LAY_MDI_Y      85
#define LAY_VIEWPORT_Y 109

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
    AppMenu_Plane,
    AppMenu_View,
    AppMenu_Tools,
    AppMenu_Assets,
    AppMenu_WapMap,
    AppMenu_EntryCount
};

class cAppMenu : public gcn::ActionListener, gcn::SelectionListener {
private:
    cAppMenu_Entry *hEntries[AppMenu_EntryCount];
    int iHeight;
    SHR::Context *conOpen, *conOpenMRU, *conActivePlane, *conPlanesVisibilityList, *conPlaneVisibility;
    int iOpened;
    int iOverallWidth;
    cRulers *hRulers;
    bool bEnabled;

public:
    cAppMenu();

    ~cAppMenu();

    void Think(bool bConsumed);

    void Render();

    //bool IsFolded(){ return bFolded; };
    //void SetFolded(bool b);
    bool IsEnabled() { return bEnabled; };

    void SetEnabled(bool b) { bEnabled = b; };

    int GetHeight() { return iHeight; };

    void action(const gcn::ActionEvent &actionEvent);

    void valueChanged(const SelectionEvent &event);

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

    void FixInterfacePositions();

    void SetToolSpecificEnabled(bool b);

    cRulers *GetRulers() { return hRulers; };
};

#endif
