#include "cAppMenu.h"
#include "states/editing_ww.h"
#include "../shared/commonFunc.h"
#include "langID.h"
#include "states/dialog.h"
#include "cNativeController.h"
#include "states/stats.h"
#include "states/loadmap.h"
#include "windows/tileBrowser.h"
#include "windows/imgsetBrowser.h"
#include "windows/options.h"
#include "windows/about.h"

#include "databanks/logics.h"
#include "version.h"

extern HGE *hge;

cAppMenu_Entry::cAppMenu_Entry(std::string lab, Gfx16Icons ico) {
    strLabel = lab;
    iIcon = ico;
    bEnabled = true;
    hContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    hContext->hide();
    GV->editState->conMain->add(hContext, 400, 400);
}

cAppMenu::cAppMenu() {
    bEnabled = true;
    iHovered = iOpened = iSelected = -1;

    setTabInEnabled(false);
    setTabOutEnabled(false);
    setFocusable(true);
    addMouseListener(this);
    addKeyListener(this);
    addFocusListener(this);

    hEntries[AppMenu_File] = new cAppMenu_Entry(GETL2S("AppMenu", "Main_File"), Icon16_New);
    hEntries[AppMenu_Edit] = new cAppMenu_Entry(GETL2S("AppMenu", "Main_Edit"), Icon16_Pencil);
    hEntries[AppMenu_Edit]->SetEnabled(false);
    hEntries[AppMenu_View] = new cAppMenu_Entry(GETL2S("AppMenu", "Main_View"), Icon16_View);
    hEntries[AppMenu_View]->SetEnabled(false);
    hEntries[AppMenu_Navigation] = new cAppMenu_Entry(GETL2S("AppMenu", "Main_Navigation"), Icon16_View);
    hEntries[AppMenu_Navigation]->SetEnabled(false);
    hEntries[AppMenu_Tools] = new cAppMenu_Entry(GETL2S("AppMenu", "Main_Tools"), Icon16_Tools);
    hEntries[AppMenu_Tools]->SetEnabled(false);
    hEntries[AppMenu_Assets] = new cAppMenu_Entry(GETL2S("AppMenu", "Main_Assets"), Icon16_Database);
    hEntries[AppMenu_Assets]->SetEnabled(false);
    hEntries[AppMenu_WapMap] = new cAppMenu_Entry(GETL2S("AppMenu", "Main_WapMap"), Icon16_Settings);

    setDimension(gcn::Rectangle(0, 0, GetMenuWidth(),
                                LAY_APPMENU_Y + LAY_APPMENU_H));

    for (auto & hEntry : hEntries) {
        hEntry->GetContext()->addActionListener(this);
    }

    SHR::Context *workcon = hEntries[AppMenu_File]->GetContext();
    workcon->AddElement(APPMEN_FILE_NEW, GETL2S("AppMenu", "File_New"), GV->sprIcons16[Icon16_New]);
    workcon->AddElement(APPMEN_FILE_OPEN, GETL2S("AppMenu", "File_Open"), GV->sprIcons16[Icon16_Open]);
    workcon->AddElement(APPMEN_FILE_RECENT, GETL2S("AppMenu", "File_Recent"));
    workcon->AddSeparator();
    workcon->AddElement(APPMEN_FILE_CLOSE, GETL2S("AppMenu", "File_Close"));
    workcon->AddElement(APPMEN_FILE_CLOSEALL, GETL2S("AppMenu", "File_CloseAll"));
    workcon->AddSeparator();
    workcon->AddElement(APPMEN_FILE_SAVE, GETL2S("AppMenu", "File_Save"), GV->sprIcons16[Icon16_Save]);
    workcon->AddElement(APPMEN_FILE_SAVEAS, GETL2S("AppMenu", "File_SaveAs"));
#ifdef _DEBUG
    workcon->AddElement(APPMEN_FILE_EXPORT, GETL2S("AppMenu", "File_Export"));
#endif
    workcon->AddSeparator();
    workcon->AddElement(APPMEN_FILE_EXIT, GETL2S("AppMenu", "File_Exit"));
    workcon->adjustSize();

    conOpenMRU = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    conOpenMRU->hide();
    conOpenMRU->addActionListener(this);
    GV->editState->conMain->add(conOpenMRU, 400, 400);

    SyncDocumentClosed();

    workcon = hEntries[AppMenu_Edit]->GetContext();
    workcon->AddElement(APPMEN_EDIT_WORLD, GETL2S("AppMenu", "Edit_World"), GV->sprIcons16[Icon16_World]);
    workcon->AddElement(APPMEN_EDIT_PLANES, GETL2S("AppMenu", "Edit_Planes"), GV->sprIcons16[Icon16_Planes]);
    workcon->AddElement(APPMEN_EDIT_TILEPROP, GETL2S("AppMenu", "Edit_TileProp"), GV->sprIcons16[Icon16_Properties]);
    workcon->AddElement(APPMEN_EDIT_WORLDSCRIPT, GETL2S("AppMenu", "Edit_GlobalScript"), GV->sprIcons16[Icon16_Code]);
    workcon->adjustSize();

    conPlaneVisibility = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    conPlaneVisibility->hide();
    conPlaneVisibility->addActionListener(this);
    GV->editState->conMain->add(conPlaneVisibility, 400, 400);

    conPlaneVisibility->AddElement(APPMEN_PLANEVIS_BORDER, GETL2S("AppMenu", "PlaneDraw_Border"),
                                   GV->sprIcons16[Icon16_Boundary]);
    conPlaneVisibility->AddElement(APPMEN_PLANEVIS_GRID, GETL2S("AppMenu", "PlaneDraw_Grid"),
                                   GV->sprIcons16[Icon16_Grid]);
    conPlaneVisibility->AddElement(APPMEN_PLANEVIS_OBJECTS, GETL2S("AppMenu", "PlaneDraw_Objects"),
                                   GV->sprIcons16[Icon16_ModeObject]);
    conPlaneVisibility->ReserveIconSpace(1, 1);
    conPlaneVisibility->adjustSize();

    conPlanesVisibilityList = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    conPlanesVisibilityList->hide();
    conPlanesVisibilityList->addActionListener(this);
    conPlanesVisibilityList->addSelectionListener(this);
    GV->editState->conMain->add(conPlanesVisibilityList, 400, 400);

    workcon = hEntries[AppMenu_View]->GetContext();
    workcon->AddElement(APPMEN_VIEW_PLANES, GETL2S("AppMenu", "View_PlaneVisibility"), GV->sprIcons16[Icon16_Planes]);
    workcon->AddElement(APPMEN_VIEW_RULERS, GETL2S("AppMenu", "View_Rulers"), GV->sprIcons16[Icon16_Measure]);
    workcon->AddElement(APPMEN_VIEW_GUIDELINES, GETL2S("AppMenu", "View_GuideLines"), GV->sprIcons16[Icon16_Grid]);
    hEntries[AppMenu_View]->GetContext()->GetElementByID(APPMEN_VIEW_GUIDELINES)->SetIcon(
            GV->sprIcons16[Icon16_Applied], 1);
    workcon->AddElement(APPMEN_VIEW_TILEPROP, GETL2S("AppMenu", "View_TileProp"), GV->sprIcons16[Icon16_Properties]);
    workcon->GetElementByID(APPMEN_VIEW_PLANES)->SetCascade(conPlanesVisibilityList);
    workcon->adjustSize();

    workcon = hEntries[AppMenu_Navigation]->GetContext();
    workcon->AddElement(APPMEN_GO_SPAWN, GETL2S("AppMenu", "Go_Spawn"), GV->sprIcons16[Icon16_ClawHead]);
    workcon->AddElement(APPMEN_GO_END, GETL2S("AppMenu", "Go_End"), GV->sprIcons16[Icon16_Map]);
    workcon->AddElement(APPMEN_GO_PREV_WARP, GETL2S("AppMenu", "Go_Prev_Warp"), GV->sprIcons16[Icon16_Warp]);
    workcon->AddElement(APPMEN_GO_LOCATION, GETL2S("AppMenu", "Go_Location"), GV->sprIcons16[Icon16_Location]);
    workcon->AddElement(APPMEN_GO_COORDS, GETL2S("AppMenu", "Go_Coords"), GV->sprIcons16[Icon16_Coords]);
    workcon->adjustSize();

    workcon = hEntries[AppMenu_Tools]->GetContext();
    workcon->AddElement(APPMEN_TOOLS_MAPSHOT, GETL(Lang_MapShot), GV->sprIcons16[Icon16_Mapshot]);
    workcon->AddElement(APPMEN_TOOLS_PLAY, GETL2S("AppMenu", "Tools_Play"), GV->sprIcons16[Icon16_Play]);
    workcon->AddElement(APPMEN_TOOLS_MEASURE, GETL2S("AppMenu", "Tools_Measure"), GV->sprIcons16[Icon16_Measure]);
    workcon->AddElement(APPMEN_TOOLS_STATS, GETL2S("AppMenu", "Tools_Stats"), GV->sprIcons16[Icon16_Stats]);
    workcon->adjustSize();
    workcon->GetElementByID(APPMEN_TOOLS_MAPSHOT)->SetEnabled(true);
    workcon->GetElementByID(APPMEN_TOOLS_STATS)->SetEnabled(0);

    workcon = hEntries[AppMenu_Assets]->GetContext();
    workcon->AddElement(APPMEN_ASSETS_TILES, GETL2S("AppMenu", "Assets_Tiles"), GV->sprIcons16[Icon16_ModeTile]);
    workcon->AddElement(APPMEN_ASSETS_IMAGESETS, GETL2S("AppMenu", "Assets_ImgSets"), GV->sprIcons16[Icon16_Flip]);
    workcon->AddElement(APPMEN_ASSETS_ANIMS, GETL2S("AppMenu", "Assets_Anims"), GV->sprIcons16[Icon16_Animation]);
    workcon->AddElement(APPMEN_ASSETS_SOUNDS, GETL2S("AppMenu", "Assets_Sounds"), GV->sprIcons16[Icon16_Sound]);
    workcon->AddElement(APPMEN_ASSETS_LOGICS, GETL2S("AppMenu", "Assets_Logics"), GV->sprIcons16[Icon16_Code]);
    workcon->AddElement(APPMEN_ASSETS_PALETTES, GETL2S("AppMenu", "Assets_Palettes"), GV->sprIcons16[Icon16_Brush]);
    workcon->adjustSize();

    workcon = hEntries[AppMenu_WapMap]->GetContext();
    workcon->AddElement(APPMEN_WM_SETTINGS, GETL2S("AppMenu", "About_Settings"), GV->sprIcons16[Icon16_Settings]);
    workcon->AddElement(APPMEN_WM_ABOUT, GETL2S("AppMenu", "About_About"), GV->sprIcons16[Icon16_InfoCloud]);
    /*workcon->AddElement(APPMEN_WM_UPDATE, GETL2S("AppMenu", "About_Update"), GV->sprIcons16[Icon16_AutoUpdate]);
    workcon->GetElementByID(APPMEN_WM_UPDATE)->SetEnabled(0);
    workcon->AddElement(APPMEN_WM_README, GETL2S("AppMenu", "About_Readme"), GV->sprIcons16[Icon16_Readme]);
    workcon->GetElementByID(APPMEN_WM_README)->SetEnabled(0);
    workcon->AddElement(APPMEN_WM_SITE, GETL2S("AppMenu", "About_Website"), GV->sprIcons16[Icon16_RSS]);
    workcon->GetElementByID(APPMEN_WM_SITE)->SetEnabled(0);*/
    workcon->adjustSize();

    mDimension.width = 0;
    for (auto & hEntry : hEntries)
        mDimension.width += hEntry->GetWidth();
}

cAppMenu::~cAppMenu() {

}

void cAppMenu::SyncDocumentSwitched() {
    hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_SAVE)->SetEnabled(GV->editState->hParser != NULL);
    hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_SAVEAS)->SetEnabled(GV->editState->hParser != NULL);
#ifdef _DEBUG
    hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_EXPORT)->SetEnabled(GV->editState->hParser != NULL);
#endif
    hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_CLOSE)->SetEnabled(GV->editState->hParser != NULL);
    hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_CLOSEALL)->SetEnabled(
            GV->editState->MDI->GetDocsCount() > 1);
    hEntries[AppMenu_Edit]->SetEnabled(GV->editState->hParser != NULL);
    hEntries[AppMenu_View]->SetEnabled(GV->editState->hParser != NULL);
    hEntries[AppMenu_Navigation]->SetEnabled(GV->editState->hParser != NULL);
    hEntries[AppMenu_Tools]->SetEnabled(GV->editState->hParser != NULL);
    hEntries[AppMenu_Assets]->SetEnabled(GV->editState->hParser != NULL);
    if (GV->editState->hParser == NULL) {
        GV->editState->winpmMain->setVisible(false);
        GV->editState->winWorld->setVisible(false);
    } else {
        SyncPlanes();
        hEntries[AppMenu_Tools]->GetContext()->GetElementByID(APPMEN_TOOLS_PLAY)->SetEnabled(
                strlen(GV->editState->hParser->GetFilePath()) > 0);
    }
}

void cAppMenu::SyncDocumentClosed() {
    hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_CLOSE)->SetEnabled(
            GV->editState->MDI->GetDocsCount() >= 1);
    hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_SAVE)->SetEnabled(
            GV->editState->MDI->GetDocsCount() >= 1);
    hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_SAVEAS)->SetEnabled(
            GV->editState->MDI->GetDocsCount() >= 1);
#ifdef _DEBUG
    hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_EXPORT)->SetEnabled(
            GV->editState->MDI->GetDocsCount() >= 1);
#endif
    hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_CLOSEALL)->SetEnabled(
            GV->editState->MDI->GetDocsCount() > 1);

    if (GV->editState->MDI->GetDocsCount() == 0) {
        hEntries[AppMenu_Edit]->SetEnabled(false);
        hEntries[AppMenu_View]->SetEnabled(false);
        hEntries[AppMenu_Navigation]->SetEnabled(false);
        hEntries[AppMenu_Tools]->SetEnabled(false);
        hEntries[AppMenu_Assets]->SetEnabled(false);
    }
}

void cAppMenu::SyncDocumentOpened() {
    hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_CLOSE)->SetEnabled(true);
    hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_CLOSEALL)->SetEnabled(
            GV->editState->MDI->GetDocsCount() > 1);
    hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_SAVE)->SetEnabled(true);
    hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_SAVEAS)->SetEnabled(true);
#ifdef _DEBUG
    hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_EXPORT)->SetEnabled(true);
#endif
    SyncMRU();
    hEntries[AppMenu_Edit]->SetEnabled(true);
    hEntries[AppMenu_View]->SetEnabled(true);
    hEntries[AppMenu_Navigation]->SetEnabled(true);
    hEntries[AppMenu_Tools]->SetEnabled(true);
    hEntries[AppMenu_Assets]->SetEnabled(true);
}

void cAppMenu::SyncRecentlyClosedRebuild() {
    /*if (GV->editState->MDI->GetCachedClosedDocsCount() == 0) {
        hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_CLOSED)->SetEnabled(false);
        hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_CLOSED)->SetCascade(NULL);
    }*/
}

void cAppMenu::SyncMRU() {
    if (GV->editState->hMruList->IsValid() && GV->editState->hMruList->GetFilesCount() > 0) {
        hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_RECENT)->SetEnabled(true);
        hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_RECENT)->SetCascade(conOpenMRU);
        conOpenMRU->ClearElements();
        for (int i = 0; i < GV->editState->hMruList->GetFilesCount(); i++) {
            char *lab = SHR::GetFile(GV->editState->hMruList->GetRecentlyUsedFile(i));
            hgeSprite *ico;
            if (GV->editState->gamesLastOpened[i] < 50) {
                ico = GV->sprGamesSmall[GV->editState->gamesLastOpened[i]];
            } else {
                int game = 0, ic = GV->editState->gamesLastOpened[i] - 50;
                while (ic >= 50) {
                    ++game;
                    ic -= 50;
                }
                ico = GV->sprLevelsMicro16[game][ic - 1];
            }
            conOpenMRU->AddElement(i, lab, ico);
            delete[] lab;
        }
        conOpenMRU->adjustSize();
    } else {
        hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_RECENT)->SetEnabled(false);
        hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_RECENT)->SetCascade(NULL);
    }
    hEntries[AppMenu_File]->GetContext()->adjustSize();
}

void cAppMenu::SyncPlaneSelectedVisibility() {
    int sel = conPlanesVisibilityList->GetSelectedID();
    if (sel == -1) return;
    bool objs = GV->editState->hParser->GetPlane(sel)->GetFlags() & WWD::Flag_p_MainPlane;
    conPlaneVisibility->GetElementByID(APPMEN_PLANEVIS_OBJECTS)->SetEnabled(objs);
    if (objs)
        conPlaneVisibility->GetElementByID(APPMEN_PLANEVIS_OBJECTS)->SetIcon(
                (GV->editState->hPlaneData[sel]->bDrawObjects ? GV->sprIcons16[Icon16_Applied] : NULL), 1);
    else
        conPlaneVisibility->GetElementByID(APPMEN_PLANEVIS_OBJECTS)->SetIcon(NULL, 1);
    conPlaneVisibility->GetElementByID(APPMEN_PLANEVIS_BORDER)->SetIcon(
            (GV->editState->hPlaneData[sel]->bDrawBoundary ? GV->sprIcons16[Icon16_Applied] : NULL), 1);
    conPlaneVisibility->GetElementByID(APPMEN_PLANEVIS_GRID)->SetIcon(
            (GV->editState->hPlaneData[sel]->bDrawGrid ? GV->sprIcons16[Icon16_Applied] : NULL), 1);
}

void cAppMenu::SyncPlaneVisibility() {
    for (int i = 0; i < GV->editState->hParser->GetPlanesCount(); i++) {
        bool draw = GV->editState->hPlaneData[i]->bDraw;
        conPlanesVisibilityList->GetElementByID(i)->SetCascade(draw ? conPlaneVisibility : NULL);
        conPlanesVisibilityList->GetElementByID(i)->SetIcon(GV->sprIcons16[(draw ? Icon16_View : Icon16_ViewNo)]);
    }
}

void cAppMenu::SyncPlaneSwitched() {

}

void cAppMenu::SyncModeSwitched() {
    //hEntries[AppMenu_Mode]->SetIcon(GV->editState->GetActiveMode() == EWW_MODE_OBJECT ? Icon16_ModeObject : Icon16_ModeTile);
}

void cAppMenu::SyncPlanes() {
    conPlanesVisibilityList->ClearElements();
    for (int i = 0; i < GV->editState->hParser->GetPlanesCount(); i++) {
        conPlanesVisibilityList->AddElement(i, GV->editState->hParser->GetPlane(i)->GetName(),
                                            GV->sprIcons16[(GV->editState->hPlaneData[i]->bDraw ? Icon16_View
                                                                                                : Icon16_ViewNo)]);
        conPlanesVisibilityList->GetElementByID(i)->SetCascade(conPlaneVisibility);
    }
    conPlanesVisibilityList->adjustSize();
}

void cAppMenu::switchTo(int i) {
#ifdef WAPMAP_ENABLE_IMGUI
    nativeFocus = true;
    return;
#endif
    if (i < 0 || i >= AppMenu_EntryCount || !hEntries[i]->IsEnabled()) {
        return;
    }

    if (iOpened != i) {
        closeCurrent();
        hEntries[i]->GetContext()->setVisible(true);
    }
    int xOffset = LAY_APPMENU_X;
    for (int j = 0; j < i; ++j) xOffset += hEntries[j]->GetWidth();
    hEntries[i]->GetContext()->setTopLineOffset(hEntries[i]->GetWidth());
    hEntries[i]->GetContext()->setPosition(xOffset, LAY_APPMENU_Y + LAY_APPMENU_H);
    GV->editState->conMain->moveToTop(hEntries[i]->GetContext());
    iOpened = i;
}

void cAppMenu::closeCurrent() {
    if (iOpened == -1) return;
    hEntries[iOpened]->GetContext()->SetSelectedIt(-1);
    hEntries[iOpened]->GetContext()->setVisible(false);
    iOpened = -1;
}

void cAppMenu::draw(Graphics* graphics) {
#ifdef WAPMAP_ENABLE_IMGUI
    return;
#endif
    hge->Gfx_SetClipping();
    if (!hge->Input_IsMouseOver()) {
        iHovered = -1;
        iSelected = -1;
        closeCurrent();
    } else {
        float mouseX, mouseY;
        int menuX, menuY;
        getAbsolutePosition(menuX, menuY);
        hge->Input_GetMousePos(&mouseX, &mouseY);
        // Keyboard selection persists only while the pointer stays still.
        // In maximized mode movement can bypass this widget's mouse listener.
        if (lastMouseX != static_cast<int>(mouseX) || lastMouseY != static_cast<int>(mouseY)) {
            iSelected = -1;
        }
        lastMouseX = static_cast<int>(mouseX);
        lastMouseY = static_cast<int>(mouseY);
        iHovered = GetHoveredEntry(static_cast<int>(mouseX) - menuX,
                                   static_cast<int>(mouseY) - menuY);

        if (iHovered >= 0 && iOpened != -1 && iHovered != iOpened) {
            switchTo(iHovered);
        }
    }
    int xOffset = LAY_APPMENU_X;
    for (int i = 0; i < AppMenu_EntryCount; ++i) {
        xOffset += hEntries[i]->Render(
            xOffset, LAY_APPMENU_Y,
            iOpened == i || (iSelected == -1 && iHovered == i) || iSelected == i);
    }
}

int cAppMenu::GetMenuWidth() const {
    int width = LAY_APPMENU_X;
    for (auto* entry : hEntries) {
        width += entry->GetWidth();
    }
    return width;
}

bool cAppMenu::IsMenuBarPoint(const int x, const int y) const {
#ifdef WAPMAP_ENABLE_IMGUI
    const int menuWidth = GetNativeMenuWidth();
#else
    const int menuWidth = GetMenuWidth() - LAY_APPMENU_X;
#endif
    return y >= LAY_APPMENU_Y && y < LAY_APPMENU_Y + LAY_APPMENU_H
           && x >= LAY_APPMENU_X && x < LAY_APPMENU_X + menuWidth;
}

int cAppMenu_Entry::GetWidth() const {
    return GV->fntMyriad16->GetStringWidth(GetLabel().c_str()) + 20;
}

int cAppMenu_Entry::Render(int x, int y, bool bFocused) {
    int w = GetWidth(), h = LAY_APPMENU_H;
    const bool unfolded = hContext != nullptr && hContext->isVisible();
    bFocused = bFocused && IsEnabled();
    if (bFocused) {
        SHR::SetQuad(&q, 0xFFFFFFFF, x, y + 2, x + w, y + h - 2 * (!unfolded));
        q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col =
            unfolded ? SETA(0x0F0F0F, hContext->getAlpha()) : 0xFF454545;
        hge->Gfx_RenderQuad(&q);

        if (unfolded) {
            int borderColor = SETA(0x1d1d1d, hContext->getAlpha());
            hge->Gfx_RenderLine(x, y + 2, x, y + LAY_APPMENU_H, borderColor);
            hge->Gfx_RenderLine(x, y + 2, x + w - 1, y + 2, borderColor);
            hge->Gfx_RenderLine(x + w, y + 2, x + w, y + LAY_APPMENU_H, borderColor);
        }
    }

    //hge->Gfx_RenderLine(x + w - 2, y + 1, x + w - 2, y + LAY_APPMENU_H, 0xFF333233);
    /*GV->sprIcons16[GetIcon()]->Render(x + 4, y + 4);
    if (fTimer > 0.0f && bEnabled) {
        GV->sprIcons16[GetIcon()]->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
        GV->sprIcons16[GetIcon()]->SetColor(
                SETA(0xFFFFFF, (unsigned char) ((fTimer > 0.2f ? 0.2f : fTimer) * 5.0f * 255.0f)));
        GV->sprIcons16[GetIcon()]->Render(x + 4, y + 4);
        GV->sprIcons16[GetIcon()]->SetBlendMode(BLEND_DEFAULT);
    }
    GV->sprIcons16[GetIcon()]->SetColor(0xFFFFFFFF);*/
    const DWORD originalColor = GV->fntMyriad16->GetColor();
    GV->fntMyriad16->SetColor(IsEnabled() ? 0xFFE1E1E1 : 0xFF777777);
    GV->fntMyriad16->Render(x + w / 2, y + h / 2 - 1, HGETEXT_CENTER | HGETEXT_MIDDLE,
                            GetLabel().c_str(), 1);
    GV->fntMyriad16->SetColor(originalColor);

    //hge->Gfx_RenderLine(x + w - 1, y, x + w - 1, y + LAY_APPMENU_H, 0xFF111111);
    return w;
}

void cAppMenu::valueChanged(const SelectionEvent &event) {
    if (event.getSource() == conPlanesVisibilityList) {
        SyncPlaneSelectedVisibility();
    }
}

void cAppMenu::focusLost(const FocusEvent &event) {
    iSelected = -1;
    if (iOpened != -1) {
        Widget *widget = event.getOtherSource();
        if (!widget) closeCurrent();
        else {
            SHR::Context* context = hEntries[iOpened]->GetContext();
            while (context) {
                if (widget == context) {
                    event.stopEvent();
                    return;
                }
                if (!context->GetSelectedElement()) context = NULL;
                else context = context->GetSelectedElement()->GetCascade();
            }
            closeCurrent();
        }
    }
}

void cAppMenu::keyPressed(KeyEvent &keyEvent) {
#ifdef WAPMAP_ENABLE_IMGUI
    if (keyEvent.getKey().getValue() == Key::LEFT_ALT && bEnabled && mEnabled) {
        nativeFocus = true;
        GV->editState->vPort->GetWidget()->requestFocus();
    }
    return;
#endif
    if (!mEnabled) return;

    int key = keyEvent.getKey().getValue();
    if (iOpened != -1 && (key == Key::ENTER || (key >= Key::LEFT && key <= Key::DOWN))) { // any of arrow keys is pressed with open menu
        iSelected = iOpened;

        SHR::Context *context = hEntries[iOpened]->GetContext(), *prev = 0, *next = 0;
        SHR::ContextEl *option = 0;
        while (context) {
            option = context->GetSelectedElement();
            if (option) next = option->GetCascade();
            if (next && next->isVisible()) {
                prev = context;
                context = next;
            } else {
                switch (key) {
                    case Key::LEFT:
                        if (prev) {
                            context->setVisible(false);
                            return;
                        }
                        context = 0;
                        break;
                    case Key::RIGHT:
                        if (next) {
                            context->distributeValueChangedEvent();
                            context->OpenSubContext(context->GetSelectedIt());
                            next->SelectNext();
                            return;
                        }
                        context = 0;
                        break;
                    case Key::UP:
                        context->SelectPrev();
                        return;
                    case Key::DOWN:
                        context->SelectNext();
                        return;
                    case Key::ENTER:
                        if (option) {
                            context->EmulateClickID(option->GetID());
                        }
                        return;
                }
            }
        }
    }

    switch (key) {
        case Key::LEFT:
            if (iSelected <= 0) {
                iSelected = AppMenu_EntryCount;
            }

            while (!hEntries[--iSelected]->IsEnabled()) {
                if (iSelected == 0) {
                    iSelected = AppMenu_EntryCount;
                }
            }

            if (iOpened != -1) {
                switchTo(iSelected);
                hEntries[iOpened]->GetContext()->SelectNext();
            }
            break;
        case Key::RIGHT:
            if (iSelected >= AppMenu_EntryCount - 1) {
                iSelected = -1;
            }

            while (!hEntries[++iSelected]->IsEnabled()) {
                if (iSelected == AppMenu_EntryCount - 1) {
                    iSelected = -1;
                }
            }

            if (iOpened != -1) {
                switchTo(iSelected);
                hEntries[iOpened]->GetContext()->SelectNext();
            }
            break;
        case Key::DOWN:
            if (iOpened == -1 && iSelected != -1) {
                switchTo(iSelected);
                hEntries[iOpened]->GetContext()->SelectNext();
            }
            break;
        case Key::ESCAPE:
            if (iOpened != -1) {
                closeCurrent();
            } else {
                iSelected = -1;
                GV->editState->vPort->GetWidget()->requestFocus();
            }
            break;
        case Key::LEFT_ALT:
            if (iSelected == -1 && iOpened == -1) {
                iSelected = 0;
            } else {
                //GV->editState->mainListener->stopAltMenu();
                closeCurrent();
                iSelected = -1;
            }
            break;
    }
}

void cAppMenu::action(const gcn::ActionEvent &actionEvent) {
    iSelected = iOpened = -1;
    if (actionEvent.getSource() == hEntries[AppMenu_File]->GetContext()) {
        int id = hEntries[AppMenu_File]->GetContext()->GetSelectedID();
        if (id == APPMEN_FILE_NEW) {
            GV->editState->hwinNewMap->Open();
        } else if (id == APPMEN_FILE_OPEN) {
            GV->editState->OpenDocuments();
        } else if (id == APPMEN_FILE_RECENT) {
            iOpened = AppMenu_File;
            return;
        } else if (id == APPMEN_FILE_EXPORT) {
            GV->editState->Export();
        } else if (id == APPMEN_FILE_EXIT) {
            if (GV->editState->PromptExit()) {
                GV->editState->bExit = true;
            }
        } else if (id == APPMEN_FILE_SAVEAS ||
                   (id == APPMEN_FILE_SAVE && strlen(GV->editState->hParser->GetFilePath()) == 0)) {
            GV->editState->SaveAs();
        } else if (id == APPMEN_FILE_SAVE) {
            GV->editState->MDI->SaveCurrent();
        } else if (id == APPMEN_FILE_CLOSE) {
            GV->editState->MDI->CloseDocByIt(GV->editState->MDI->GetActiveDocIt());
        } else if (id == APPMEN_FILE_CLOSEALL) {
            GV->editState->MDI->CloseAllDocs();
        }
        hEntries[AppMenu_File]->GetContext()->setVisible(false);
    } else if (actionEvent.getSource() == hEntries[AppMenu_Edit]->GetContext()) {
        int id = hEntries[AppMenu_Edit]->GetContext()->GetSelectedID();
        if (id == APPMEN_EDIT_WORLD) {
#ifdef WAPMAP_ENABLE_IMGUI
            GV->editState->hwinMapProperties->Open();
#else
            if (GV->editState->winWorld->isVisible()) GV->editState->SyncWorldOptionsWithParser();
            GV->editState->winWorld->setPosition(hge->System_GetState(HGE_SCREENWIDTH) / 2 - GV->editState->winWorld->getWidth() / 2,
                                                    hge->System_GetState(HGE_SCREENHEIGHT) / 2 - GV->editState->winWorld->getHeight() / 2);
            GV->editState->winWorld->setVisible(true);
            GV->editState->conMain->moveToTop(GV->editState->winWorld);
#endif
        } else if (id == APPMEN_EDIT_PLANES) {
#ifdef WAPMAP_ENABLE_IMGUI
            GV->editState->hwinPlaneProperties->Open();
#else
            GV->editState->winpmMain->setPosition(hge->System_GetState(HGE_SCREENWIDTH) / 2 - GV->editState->winpmMain->getWidth() / 2,
                                                  hge->System_GetState(HGE_SCREENHEIGHT) / 2 - GV->editState->winpmMain->getHeight() / 2);
            GV->editState->conMain->moveToTop(GV->editState->winpmMain);
            GV->editState->winpmMain->setVisible(true);
            GV->editState->SyncPlaneProperties();
#endif
        } else if (id == APPMEN_EDIT_TILEPROP) {
#ifdef WAPMAP_ENABLE_IMGUI
            GV->editState->hwinTileProperties->Open();
#else
            GV->editState->winTileProp->setPosition(hge->System_GetState(HGE_SCREENWIDTH) / 2 - GV->editState->winTileProp->getWidth() / 2,
                                                  hge->System_GetState(HGE_SCREENHEIGHT) / 2 - GV->editState->winTileProp->getHeight() / 2);
            GV->editState->winTileProp->setVisible(true);
            GV->editState->conMain->moveToTop(GV->editState->winTileProp);
#endif
        } else if (id == APPMEN_EDIT_WORLDSCRIPT) {
            if (strlen(GV->editState->hParser->GetFilePath()) == 0) {
                State::MessageBox(PRODUCT_NAME, GETL2S("Win_LogicBrowser", "GlobalScriptDocumentSave"),
                                  ST_DIALOG_ICON_ERROR, ST_DIALOG_BUT_OK);
            } else {
                if (GV->editState->hCustomLogics->GetGlobalScript() == 0) {
                    if (State::MessageBox(PRODUCT_NAME, GETL2S("Win_LogicBrowser", "NoGlobalScript"),
                                          ST_DIALOG_ICON_WARNING, ST_DIALOG_BUT_YESNO) == RETURN_YES) {
                        GV->editState->hDataCtrl->FixCustomDir();
                        GV->editState->hDataCtrl->OpenCodeEditor("main", true);
                    }
                } else {
                    GV->editState->hDataCtrl->OpenCodeEditor("main");
                }
            }
        }
        hEntries[AppMenu_Edit]->GetContext()->setVisible(false);
    } else if (actionEvent.getSource() == conOpenMRU) {
        hEntries[AppMenu_File]->GetContext()->setVisible(false);
        conOpenMRU->setVisible(false);
        GV->editState->vstrMapsToLoad.emplace_back(GV->editState->hMruList->GetRecentlyUsedFile(conOpenMRU->GetSelectedID()));
    } else if (actionEvent.getSource() == hEntries[AppMenu_WapMap]->GetContext()) {
        int id = hEntries[AppMenu_WapMap]->GetContext()->GetSelectedID();
        if (id == APPMEN_WM_SETTINGS) {
            GV->editState->hwinOptions->Open();
        } else if (id == APPMEN_WM_README) {

        } else if (id == APPMEN_WM_ABOUT) {
            GV->editState->hwinAbout->Open();
        } else if (id == APPMEN_WM_UPDATE) {

        } else if (id == APPMEN_WM_SITE) {

        }
        hEntries[AppMenu_WapMap]->GetContext()->setVisible(false);
    } else if (actionEvent.getSource() == hEntries[AppMenu_View]->GetContext()) {
        int id = hEntries[AppMenu_View]->GetContext()->GetSelectedID();
        if (id == APPMEN_VIEW_RULERS) {
            GV->editState->hRulers->SetVisible(!GV->editState->hRulers->IsVisible());
        } else if (id == APPMEN_VIEW_GUIDELINES) {
            GV->editState->bShowGuideLines = !GV->editState->bShowGuideLines;
            hEntries[AppMenu_View]->GetContext()->GetElementByID(APPMEN_VIEW_GUIDELINES)->SetIcon(
                    GV->editState->bShowGuideLines ? GV->sprIcons16[Icon16_Applied] : NULL, true);
        } else if (id == APPMEN_VIEW_TILEPROP) {
            GV->editState->bDrawTileProperties = !GV->editState->bDrawTileProperties;
            GV->editState->vPort->MarkToRedraw();
            hEntries[AppMenu_View]->GetContext()->GetElementByID(APPMEN_VIEW_TILEPROP)->SetIcon(
                    GV->editState->bDrawTileProperties ? GV->sprIcons16[Icon16_Applied] : NULL, true);
        }
        iOpened = AppMenu_View;
    } else if (actionEvent.getSource() == hEntries[AppMenu_Navigation]->GetContext()) {
        int id = hEntries[AppMenu_Navigation]->GetContext()->GetSelectedID();
        if (id == APPMEN_GO_SPAWN) {
            GV->editState->NavigateToStartLocation();
            iOpened = AppMenu_Navigation;
        } else if (id == APPMEN_GO_END) {
            GV->editState->NavigateToEndLocation();
            iOpened = AppMenu_Navigation;
        } else if (id == APPMEN_GO_LOCATION) {
            GV->editState->hwinLocationsBrowser->Open();
            hEntries[AppMenu_Navigation]->GetContext()->setVisible(false);
        } else if (id == APPMEN_GO_COORDS) {
            GV->editState->hwinGoToCoords->Open();
            hEntries[AppMenu_Navigation]->GetContext()->setVisible(false);
        } else if (id == APPMEN_GO_PREV_WARP) {
            int warpX = GV->editState->MDI->GetActiveDoc()->fPrevWarpX;
            int warpY = GV->editState->MDI->GetActiveDoc()->fPrevWarpY;
            if (warpX > 0 && warpY > 0)
                GV->editState->NavigateToPoint(warpX, warpY);
            iOpened = AppMenu_Navigation;
        }
    } else if (actionEvent.getSource() == conPlanesVisibilityList) {
        int id = conPlanesVisibilityList->GetSelectedID();
        GV->editState->hPlaneData[id]->bDraw = !GV->editState->hPlaneData[id]->bDraw;
        GV->editState->vPort->MarkToRedraw();
        SyncPlaneVisibility();
        if (GV->editState->hPlaneData[id]->bDraw) {
            conPlaneVisibility->setVisible(true);
        }
        iOpened = AppMenu_View;
    } else if (actionEvent.getSource() == conPlaneVisibility) {
        int id = conPlaneVisibility->GetSelectedID(),
            pl = conPlanesVisibilityList->GetSelectedID();
        if (id == APPMEN_PLANEVIS_BORDER) {
            GV->editState->hPlaneData[pl]->bDrawBoundary = !GV->editState->hPlaneData[pl]->bDrawBoundary;
        } else if (id == APPMEN_PLANEVIS_GRID) {
            GV->editState->hPlaneData[pl]->bDrawGrid = !GV->editState->hPlaneData[pl]->bDrawGrid;
        } else if (id == APPMEN_PLANEVIS_OBJECTS) {
            GV->editState->hPlaneData[pl]->bDrawObjects = !GV->editState->hPlaneData[pl]->bDrawObjects;
        }
        if (GV->editState->hPlaneData[pl]->hRB != 0)
            GV->editState->hPlaneData[pl]->hRB->Redraw();
        GV->editState->vPort->MarkToRedraw();
        SyncPlaneSelectedVisibility();
        iOpened = AppMenu_View;
    } else if (actionEvent.getSource() == hEntries[AppMenu_Tools]->GetContext()) {
        int id = hEntries[AppMenu_Tools]->GetContext()->GetSelectedID();
        if (id == APPMEN_TOOLS_PLAY) {
            GV->editState->hNativeController->RunGame(GV->editState->MDI->GetActiveDoc()->hParser->GetFilePath());
        } else if (id == APPMEN_TOOLS_MEASURE)
            GV->editState->SetTool(EWW_TOOL_MEASURE);
        else if (id == APPMEN_TOOLS_MAPSHOT) {
#ifdef WAPMAP_ENABLE_IMGUI
            GV->editState->hwinMapShotOptions->Open();
#else
            GV->editState->winMapShot->setVisible(true);
            GV->editState->conMain->moveToTop(GV->editState->winMapShot);
#endif
        } else if (id == APPMEN_TOOLS_STATS) {
            GV->StateMgr->Push(new State::MapStats(GV->editState->hParser));
        }
        hEntries[AppMenu_Tools]->GetContext()->setVisible(false);
    } else if (actionEvent.getSource() == hEntries[AppMenu_Assets]->GetContext()) {
        int id = hEntries[AppMenu_Assets]->GetContext()->GetSelectedID();
        if (id == APPMEN_ASSETS_TILES) {
            GV->editState->hwinTileBrowser->Open();
        } else if (id == APPMEN_ASSETS_IMAGESETS) {
            GV->editState->hwinImageSetBrowser->Open();
        } else if (id == APPMEN_ASSETS_LOGICS) {
#ifdef WAPMAP_ENABLE_IMGUI
            GV->editState->hwinLogicBrowser->Open();
#else
            GV->editState->winLogicBrowser->setPosition((hge->System_GetState(HGE_SCREENWIDTH) - GV->editState->winLogicBrowser->getWidth()) / 2,
                                                        (hge->System_GetState(HGE_SCREENHEIGHT) - GV->editState->winLogicBrowser->getHeight()) / 2);
            GV->editState->winLogicBrowser->setVisible(true);
            GV->editState->conMain->moveToTop(GV->editState->winLogicBrowser);
#endif
        } else if (id == APPMEN_ASSETS_PALETTES) {
            GV->editState->hwinPaletteBrowser->Open();
        } else {
#ifdef WAPMAP_ENABLE_IMGUI
            if (id == APPMEN_ASSETS_SOUNDS) GV->editState->hwinSoundBrowser->Open();
            else GV->editState->hwinAnimationBrowser->Open();
#else
            GV->editState->winDB->setVisible(true);
            GV->editState->conMain->moveToTop(GV->editState->winDB);
            GV->editState->tadbTabs->setSelectedTab(id == APPMEN_ASSETS_SOUNDS);
#endif
        }
        hEntries[AppMenu_Assets]->GetContext()->setVisible(false);
    }
}

void cAppMenu::SetToolSpecificEnabled(bool b) {
    hEntries[AppMenu_File]->SetEnabled(b);
    hEntries[AppMenu_Edit]->SetEnabled(b);
    hEntries[AppMenu_Tools]->SetEnabled(b);
    hEntries[AppMenu_Assets]->SetEnabled(b);
}

void cAppMenu::NotifyRulersSwitch() {
    //SetFolded(IsFolded());
    GV->editState->FixInterfacePositions();
    hEntries[AppMenu_View]->GetContext()->GetElementByID(APPMEN_VIEW_RULERS)->SetIcon(
        GV->editState->hRulers->IsVisible() ? GV->sprIcons16[Icon16_Applied] : NULL, true);
}

void cAppMenu::mouseExited(MouseEvent &mouseEvent) {
    iHovered = -1;
    iSelected = -1;
}

void cAppMenu::mouseMoved(MouseEvent& mouseEvent) {
#ifdef WAPMAP_ENABLE_IMGUI
    return;
#endif
    const int hovered = GetHoveredEntry(mouseEvent.getX(), mouseEvent.getY());
    iSelected = -1;
    if (hovered != iHovered) {
        iHovered = hovered;
        if (iOpened != -1 && iHovered != -1) {
            switchTo(iHovered);
        }
    }
}

void cAppMenu::mousePressed(MouseEvent &mouseEvent) {
#ifdef WAPMAP_ENABLE_IMGUI
    return;
#endif
    int opened = iOpened;
    closeCurrent();

    iSelected = -1;
    const int hovered = GetHoveredEntry(mouseEvent.getX(), mouseEvent.getY());
    if (hovered >= 0 && hovered != opened) {
        switchTo(hovered);
    }
}

int cAppMenu::GetHoveredEntry(const int x, const int y) const {
    if (y < LAY_APPMENU_Y || y >= LAY_APPMENU_Y + LAY_APPMENU_H ||
        x < LAY_APPMENU_X) {
        return -1;
    }

    int entryX = x - LAY_APPMENU_X;
    for (int i = 0; i < AppMenu_EntryCount; ++i) {
        entryX -= hEntries[i]->GetWidth();
        if (entryX < 0) {
            return hEntries[i]->IsEnabled() ? i : -1;
        }
    }
    return -1;
}
