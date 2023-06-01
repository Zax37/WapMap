#include "editing_ww.h"
#include "loadmap.h"
#include "../globals.h"
#include "../version.h"
#include "../../shared/commonFunc.h"
#include "../langID.h"
#include "../returncodes.h"
#include "../cObjectUserData.h"
#include "dialog.h"
#include <cmath>
#include "../cAppMenu.h"
#include "../../shared/gcnWidgets/wComboButton.h"
#include "../../shared/gcnWidgets/wScrollArea.h"
#include "../cNativeController.h"
#include "../windows/tileBrowser.h"
#include "../windows/imgsetBrowser.h"
#include "../windows/options.h"
#include "../windows/about.h"
#include <direct.h>
#include "../databanks/logics.h"
#include "../databanks/tiles.h"
#include "../databanks/imageSets.h"
#include "../cBrush.h"
#include <filesystem>

extern HGE *hge;

State::EditingWW::EditingWW() {
    hParser = NULL;

    fade_fAlpha = 255;
    fade_fTimer = 0;
    fade_iAction = 0;
    GV->editState = this;

    //fHomeBackButTimer = 0.0;
}

State::EditingWW::EditingWW(WWD::Parser *phParser) {
    hParser = phParser;
    GV->editState = this;
}

bool State::EditingWW::Opaque() {
    return true;
}

void State::EditingWW::ExpandLogicBrowser() {
    bLogicBrowserExpanded = true;

    SHR::Lab *labs[] = {labbrlLogicName, labbrlFilePath, labbrlFileSize, labbrlFileChecksum, labbrlFileModDate};
    int width = 0;
    for (auto lab : labs) {
        if (lab->getWidth() > width) {
            width = lab->getWidth();
        }
    }
    width += 10;

    int yOffset = 16;

    winLogicBrowser->setDimension(gcn::Rectangle(0, 0, 800, 225));
    GV->editState->winLogicBrowser->setPosition((hge->System_GetState(HGE_SCREENWIDTH) - GV->editState->winLogicBrowser->getWidth()) / 2,
                                                (hge->System_GetState(HGE_SCREENHEIGHT) - GV->editState->winLogicBrowser->getHeight()) / 2);
    winLogicBrowser->add(labbrlLogicName, 305, yOffset);
    winLogicBrowser->add(labbrlFilePath, 305, yOffset + 25);
    winLogicBrowser->add(labbrlFileSize, 305, yOffset + 50);
    winLogicBrowser->add(labbrlFileChecksum, 305, yOffset + 75);
    winLogicBrowser->add(labbrlFileModDate, 305, yOffset + 100);
    winLogicBrowser->add(labbrlLogicNameV, 305 + width, yOffset);
    winLogicBrowser->add(labbrlFilePathV, 305 + width, yOffset + 25);
    winLogicBrowser->add(labbrlFileSizeV, 305 + width, yOffset + 50);
    winLogicBrowser->add(labbrlFileChecksumV, 305 + width, yOffset + 75);
    winLogicBrowser->add(labbrlFileModDateV, 305 + width, yOffset + 100);
    winLogicBrowser->add(butbrlBrowseDir, 305, yOffset + 160);
    winLogicBrowser->add(butbrlEdit, 305, yOffset + 130);
    winLogicBrowser->add(butbrlRename, 305 + 248, yOffset + 130);
    winLogicBrowser->add(butbrlDelete, 305 + 248, yOffset + 160);
}

void State::EditingWW::FoldLogicBrowser() {
    bLogicBrowserExpanded = false;

    winLogicBrowser->remove(labbrlLogicName);
    winLogicBrowser->remove(labbrlFilePath);
    winLogicBrowser->remove(labbrlFileSize);
    winLogicBrowser->remove(labbrlFileChecksum);
    winLogicBrowser->remove(labbrlFileModDate);
    winLogicBrowser->remove(labbrlLogicNameV);
    winLogicBrowser->remove(labbrlFilePathV);
    winLogicBrowser->remove(labbrlFileSizeV);
    winLogicBrowser->remove(labbrlFileChecksumV);
    winLogicBrowser->remove(labbrlFileModDateV);
    winLogicBrowser->remove(butbrlBrowseDir);
    winLogicBrowser->remove(butbrlEdit);
    winLogicBrowser->remove(butbrlRename);
    winLogicBrowser->remove(butbrlDelete);

    winLogicBrowser->setDimension(gcn::Rectangle(0, 0, 300, 225));
    GV->editState->winLogicBrowser->setPosition((hge->System_GetState(HGE_SCREENWIDTH) - GV->editState->winLogicBrowser->getWidth()) / 2,
                                                (hge->System_GetState(HGE_SCREENHEIGHT) - GV->editState->winLogicBrowser->getHeight()) / 2);
}

void State::EditingWW::Init() {
    hAU = GV->bAutoUpdate ? new cAutoUpdater() : nullptr;
    bWindowFocused = true;
    iManipulatedGuide = -1;
    bShowGuideLines = true;
    bEditObjDelete = false;
    bMaximized = GV->bStartMaximized;
    bDragWindow = false;

    for (auto& timer : fAppBarTimers)
        timer = 0.0f;

    fCameraMoveTimer = 0;
    iTileSelectX1 = iTileSelectX2 = iTileSelectY1 = iTileSelectY2 = -1;
    bDragSelection = false;
    iDragSelectionOrigX = iDragSelectionOrigY = 0;
    szObjSearchBuffer = NULL;
    btpDragDropMask = false;
    hTempAttrib = nullptr;
    lastbrushx = lastbrushy = 0;

    bForceObjectClipbPreview = bForceTileClipbPreview = false;

    bDrawTileProperties = false;
    bConstRedraw = false;
    bExit = false;

    iMode = EWW_MODE_TILE;
    iActiveTool = EWW_TOOL_NONE;
    fZoom = 1.0f;
    fDestZoom = 1.0f;
    fLastZoom = fZoom;

    iTilePreviewX = iTilePreviewY = -1;

    fCamLastX = fCamLastY = 0;

    iTilePicked = EWW_TILE_NONE;

    FirstRun_data = 0;

    GV->Console->Print("~w~Creating viewport...");

    gui = new gcn::Gui();
    gui->setGraphics(GV->gcnGraphics);
    gui->setInput(GV->gcnInput);

    conMain = new SHR::Container();
    conMain->setOpaque(0);
    conMain->setDimension(
            gcn::Rectangle(0, 0, hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT)));

    vPort = new Viewport(this, 0, LAY_VIEWPORT_Y, hge->System_GetState(HGE_SCREENWIDTH) - 17,
                         hge->System_GetState(HGE_SCREENHEIGHT) - LAY_VIEWPORT_Y - LAY_STATUS_H - 17);
    vPort->GetWidget()->addMouseListener(this);
    conMain->add(vPort->GetWidget(), vPort->GetX(), vPort->GetY());

    mainListener = new EditingWWMainListener(this);
    vp = new EditingWWvpCallback(this);

    gui->addGlobalKeyListener(mainListener);
    conMain->addMouseListener(mainListener);

    MDI = new cMDI();
    hAppMenu = new cAppMenu();
    conMain->add(hAppMenu, LAY_APPMENU_X, LAY_APPMENU_Y);

    hRulers = new cRulers();

    vpMain = new WIDG::Viewport(vp, VP_VIEWPORT);
    conMain->add(vpMain, 0, 0);

    conWriteID = new SHR::Container();
    conWriteID->setDimension(gcn::Rectangle(0, 0, 48, 20));
    conWriteID->setVisible(0);
    conWriteID->setOpaque(0);
    conMain->add(conWriteID, 200, 200);

    tfWriteID = new SHR::TextField();
    tfWriteID->setDimension(gcn::Rectangle(0, 0, 48, 20));
    tfWriteID->addActionListener(mainListener);
    tfWriteID->addFocusListener(mainListener);
    conWriteID->add(tfWriteID, 0, 0);

    cbutActiveMode = new SHR::ComboBut(GV->hGfxInterface);
    cbutActiveMode->addEntry(SHR::ComboButEntry(GV->sprIcons16[Icon16_ModeTile], GETL(Lang_ModeTile)));
    cbutActiveMode->addEntry(SHR::ComboButEntry(GV->sprIcons16[Icon16_ModeObject], GETL(Lang_ModeObject)));
    cbutActiveMode->addActionListener(mainListener);
    cbutActiveMode->adjustSize();
    conMain->add(cbutActiveMode, 5, LAY_MODEBAR_Y + 5);

    int modeMenuStartX = 10 + cbutActiveMode->getDimension().width;

    hmbTile = new cmmbTile(modeMenuStartX);
    hmbTile->SetVisible(false);
    hmbObject = new cmmbObject(modeMenuStartX);
    hmbObject->SetVisible(false);
    hmbActive = NULL;

    SetIconBarVisible(false);

    sliVer = new SHR::Slider(100);
    sliVer->setOrientation(SHR::Slider::VERTICAL);
    sliVer->setScaleStart(0);
    sliVer->addActionListener(mainListener);
    sliVer->setMarkerLength(40);
    conMain->add(sliVer, 0, 0);

    sliHor = new SHR::Slider(100);
    sliHor->setOrientation(SHR::Slider::HORIZONTAL);
    sliHor->setScaleStart(0);
    sliHor->addActionListener(mainListener);
    sliHor->setMarkerLength(40);
    conMain->add(sliHor, 0, 0);

    butMicroTileCB = new SHR::But(GV->hGfxInterface, GV->sprIcons16[Icon16_ModeTile]);
    butMicroTileCB->setDimension(gcn::Rectangle(0, 0, 26, 24));
    butMicroTileCB->addActionListener(mainListener);
    butMicroTileCB->SetTooltip(GETL2S("ClipboardPreview", "TileClipboard"));
    conMain->add(butMicroTileCB, 0, 0);

    butMicroObjectCB = new SHR::But(GV->hGfxInterface, GV->sprIcons16[Icon16_ModeObject]);
    butMicroObjectCB->setDimension(gcn::Rectangle(0, 0, 26, 24));
    butMicroObjectCB->addActionListener(mainListener);
    butMicroObjectCB->SetTooltip(GETL2S("ClipboardPreview", "ObjectClipboard"));
    conMain->add(butMicroObjectCB, 0, 0);

    sliZoom = new SHR::Slider(-5, 5);
    sliZoom->setDimension(gcn::Rectangle(0, 0, 150, 20));
    sliZoom->setStyle(SHR::Slider::POINTER);
    sliZoom->addActionListener(mainListener);
    for (int i = -5; i <= 5; i++)
        sliZoom->addKeyValue(i);
    conMain->add(sliZoom, 0, 0);

    /*conResizeLeft = new SHR::Container();
    conResizeLeft->setDimension(gcn::Rectangle(vPort->GetX(), vPort->GetY() + 40, 40, vPort->GetHeight() - 80));
    conResizeLeft->setVisible(0);
    conResizeLeft->setOpaque(0);
    conMain->add(conResizeLeft);
    conResizeRight = new SHR::Container();
    conResizeRight->setDimension(
            gcn::Rectangle(vPort->GetX() + vPort->GetWidth() - 40, vPort->GetY() + 40, 40, vPort->GetHeight() - 80));
    conResizeRight->setVisible(0);
    conResizeRight->setOpaque(0);
    conMain->add(conResizeRight);
    conResizeUp = new SHR::Container();
    conResizeUp->setDimension(gcn::Rectangle(vPort->GetX(), vPort->GetY(), vPort->GetWidth(), 40));
    conResizeUp->setVisible(0);
    conResizeUp->setOpaque(0);
    conMain->add(conResizeUp);
    conResizeDown = new SHR::Container();
    conResizeDown->setDimension(
            gcn::Rectangle(vPort->GetX(), vPort->GetY() + vPort->GetHeight() - 40, vPort->GetWidth(), 40));
    conResizeDown->setVisible(0);
    conResizeDown->setOpaque(0);
    conMain->add(conResizeDown);

    butExtLayerUL = MakeButton(4, 4, Icon_AddUpLeft, conResizeUp);
    butExtLayerUL->setFocusable(false);
    butExtLayerUR = MakeButton(vPort->GetWidth() - 36, 4, Icon_AddUpRight, conResizeUp);
    butExtLayerUR->setFocusable(false);
    butExtLayerDL = MakeButton(4, 4, Icon_AddDownLeft, conResizeDown);
    butExtLayerDL->setFocusable(false);
    butExtLayerDR = MakeButton(vPort->GetWidth() - 36, 4, Icon_AddDownRight, conResizeDown);
    butExtLayerDR->setFocusable(false);
    butExtLayerUp = MakeButton(vPort->GetWidth() / 2, 4, Icon_AddUp, conResizeUp);
    butExtLayerUp->setFocusable(false);
    butExtLayerDown = MakeButton(vPort->GetWidth() / 2, 4, Icon_AddDown, conResizeDown);
    butExtLayerDown->setFocusable(false);
    butExtLayerLeft = MakeButton(4, vPort->GetHeight() / 2 - 40 - 16, Icon_AddLeft, conResizeLeft);
    butExtLayerLeft->setFocusable(false);
    butExtLayerRight = MakeButton(4, vPort->GetHeight() / 2 - 40 - 16, Icon_AddRight, conResizeRight);
    butExtLayerRight->setFocusable(false);*/

    hNativeController = new cNativeController();
    hNativeController->SetPath(GV->gamePaths[WWD::Game_Claw]);

    {
        int w = atoi(GV->ini->GetValue("CrazyHook", "DisplayWidth", "-1")),
            h = atoi(GV->ini->GetValue("CrazyHook", "DisplayHeight", "-1"));
        if (w != -1 && h != -1) {
            std::vector<SHR::DisplayMode> modes = SHR::GetDisplayModes();
            bool bfound = 0;
            for (size_t i = 0; i < modes.size(); i++)
                if (modes[i].iWidth == w && modes[i].iHeight == h) {
                    bfound = 1;
                    break;
                }
            if (!bfound) {
                w = 1024;
                h = 768;
            }
        }

        hNativeController->SetDisplayResolution(w, h);
        hNativeController->SetDebugInfo(atoi(GV->ini->GetValue("CrazyHook", "DebugInfo", "0")) != 0);
        hNativeController->SetGodMode(atoi(GV->ini->GetValue("CrazyHook", "GodMode", "0")) != 0);
        hNativeController->SetArmorMode(atoi(GV->ini->GetValue("CrazyHook", "ArmorMode", "0")) != 0);
    }

    hwinNewMap = new winNewMap();
    hwinOptions = new winOptions();
    hwinTileBrowser = new winTileBrowser();
    hwinImageSetBrowser = new winImageSetBrowser();
    hwinAbout = new winAbout();
    hWindows.push_back(hwinNewMap);
    hWindows.push_back(hwinOptions);
    hWindows.push_back(hwinTileBrowser);
    hWindows.push_back(hwinImageSetBrowser);
    hWindows.push_back(hwinAbout);

    for (auto & hWindow : hWindows) {
        SHR::Win *win = hWindow->GetWindow();
        if (win != 0) {
            conMain->add(win, hge->System_GetState(HGE_SCREENWIDTH) / 2 - win->getWidth() / 2,
                         hge->System_GetState(HGE_SCREENHEIGHT) / 2 - win->getHeight() / 2);
        }
    }

    winTilePicker = new SHR::Win(&GV->gcnParts, GETL2S("TilePicker", "WinCaption"));
    winTilePicker->setDimension(gcn::Rectangle(0, 0, 250, vPort->GetHeight()));
    winTilePicker->setClose(1);
    winTilePicker->addActionListener(mainListener);
    winTilePicker->setVisible(0);
    conMain->add(winTilePicker, vPort->GetX(), vPort->GetY());
    btpiFixedPos = 0;

    vptpi = new WIDG::Viewport(vp, VP_TILEPICKER);
    winTilePicker->add(vptpi, 0, 0);

    buttpiModePencil = MakeButton(5, 15, Icon_Pencil, winTilePicker);
    labtpiModePencil = new SHR::Lab(GETL2S("TilePicker", "ModePencil"));
    labtpiModePencil->adjustSize();
    winTilePicker->add(labtpiModePencil, 5 + 35, 15 + 6);
    buttpiModeBrush = MakeButton(115, 15, Icon_Brush, winTilePicker);
    labtpiModeBrush = new SHR::Lab(GETL2S("TilePicker", "ModeBrush"));
    labtpiModeBrush->adjustSize();
    winTilePicker->add(labtpiModeBrush, 115 + 35, 15 + 6);

    buttpiPoint = MakeButton(5, 15 + 40, Icon_Point, winTilePicker);
    buttpiLine = MakeButton(5 + 32, 15 + 40, Icon_Line, winTilePicker);
    buttpiRect = MakeButton(5, 15 + 40 + 32, Icon_Rect, winTilePicker);
    buttpiEllipse = MakeButton(5 + 32, 15 + 40 + 32, Icon_Ellipse, winTilePicker);
    buttpiFlood = MakeButton(5, 15 + 40 + 32 * 2, Icon_Fill, winTilePicker);
    buttpiSpray = MakeButton(5 + 32, 15 + 40 + 32 * 2, Icon_Spray, winTilePicker);

    labtpiToolOptions = new SHR::Lab(GETL2S("TilePicker", "ToolOptions"));
    labtpiToolOptions->adjustSize();
    winTilePicker->add(labtpiToolOptions, 10 + 64, 55);

    char labtmp[128];
    sprintf(labtmp, "%s: 1", GETL2S("TilePicker", "Pointsize"));
    labtpiPointSize = new SHR::Lab(labtmp);
    labtpiPointSize->adjustSize();
    winTilePicker->add(labtpiPointSize, 15 + 64, 45 + 30);

    slitpiPointSize = new SHR::Slider(1, 20);
    slitpiPointSize->setOrientation(SHR::Slider::HORIZONTAL);
    slitpiPointSize->setStyle(SHR::Slider::POINTER);
    slitpiPointSize->setDimension(gcn::Rectangle(0, 0, 150, 14));
    slitpiPointSize->addActionListener(mainListener);
    winTilePicker->add(slitpiPointSize, 15 + 64, 45 + 50);

    sprintf(labtmp, "%s: 1", GETL2S("TilePicker", "LineThickness"));
    labtpiLineThickness = new SHR::Lab(labtmp);
    labtpiLineThickness->adjustSize();
    winTilePicker->add(labtpiLineThickness, 15 + 64, 45 + 30);

    slitpiLineThickness = new SHR::Slider(1, 20);
    slitpiLineThickness->setOrientation(SHR::Slider::HORIZONTAL);
    slitpiLineThickness->setStyle(SHR::Slider::POINTER);
    slitpiLineThickness->setDimension(gcn::Rectangle(0, 0, 150, 14));
    slitpiLineThickness->addActionListener(mainListener);
    winTilePicker->add(slitpiLineThickness, 15 + 64, 45 + 50);

    cbtpiRectFilled = new SHR::CBox(GV->hGfxInterface, GETL2S("TilePicker", "Filled"));
    cbtpiRectFilled->setSelected(1);
    cbtpiRectFilled->adjustSize();
    winTilePicker->add(cbtpiRectFilled, 15 + 64, 45 + 30);

    cbtpiEllipseFilled = new SHR::CBox(GV->hGfxInterface, GETL2S("TilePicker", "Filled"));
    cbtpiEllipseFilled->setSelected(1);
    cbtpiEllipseFilled->adjustSize();
    winTilePicker->add(cbtpiEllipseFilled, 15 + 64, 45 + 30);

    sprintf(labtmp, "%s: 2", GETL2S("TilePicker", "SpraySize"));
    labtpiSpraySize = new SHR::Lab(labtmp);
    labtpiSpraySize->adjustSize();
    winTilePicker->add(labtpiSpraySize, 15 + 64, 45 + 30);

    slitpiSpraySize = new SHR::Slider(2, 20);
    slitpiSpraySize->setOrientation(SHR::Slider::HORIZONTAL);
    slitpiSpraySize->setStyle(SHR::Slider::POINTER);
    slitpiSpraySize->addActionListener(mainListener);
    slitpiSpraySize->setDimension(gcn::Rectangle(0, 0, 150, 14));
    winTilePicker->add(slitpiSpraySize, 15 + 64, 45 + 50);

    sprintf(labtmp, "%s: 50%%", GETL2S("TilePicker", "SprayDensity"));
    labtpiSprayDensity = new SHR::Lab(labtmp);
    labtpiSprayDensity->adjustSize();
    winTilePicker->add(labtpiSprayDensity, 15 + 64, 45 + 70);

    slitpiSprayDensity = new SHR::Slider(10, 100);
    slitpiSprayDensity->setOrientation(SHR::Slider::HORIZONTAL);
    slitpiSprayDensity->setStyle(SHR::Slider::POINTER);
    slitpiSprayDensity->addActionListener(mainListener);
    slitpiSprayDensity->setValue(50);
    slitpiSprayDensity->setDimension(gcn::Rectangle(0, 0, 150, 14));
    winTilePicker->add(slitpiSprayDensity, 15 + 64, 45 + 90);


    buttpiReloadBrush = MakeButton(5, 10, Icon_Reload, winTilePicker);
    labtpiReloadBrush = new SHR::Lab(GETL2S("TilePicker", "ReloadBrushes"));
    labtpiReloadBrush->adjustSize();
    winTilePicker->add(labtpiReloadBrush, 5 + 35, 10);

    //cbtpiShowTileID = new SHR::CBox()
    cbtpiShowTileID = new SHR::CBox(GV->hGfxInterface, GETL2S("TilePicker", "ShowTileID"));
    cbtpiShowTileID->adjustSize();
    cbtpiShowTileID->addActionListener(mainListener);
    winTilePicker->add(cbtpiShowTileID, 5, 10);

    cbtpiShowProperties = new SHR::CBox(GV->hGfxInterface, GETL2S("TilePicker", "ShowTileProp"));
    cbtpiShowProperties->adjustSize();
    cbtpiShowProperties->addActionListener(mainListener);
    winTilePicker->add(cbtpiShowProperties, 5, 10);

    slitpiPicker = new SHR::Slider(0);
    slitpiPicker->setOrientation(SHR::Slider::VERTICAL);
    slitpiPicker->setDimension(gcn::Rectangle(0, 0, 15, 100));
    slitpiPicker->setMarkerLength(40);
    winTilePicker->add(slitpiPicker, 230, 100);

    wintpFillColor = new SHR::Win(&GV->gcnParts, GETL(Lang_FillColor));
    wintpFillColor->setDimension(gcn::Rectangle(0, 0, 194, 61));
    wintpFillColor->setMovable(1);
    wintpFillColor->setVisible(0);
    wintpFillColor->setClose(0);
    conMain->add(wintpFillColor, 250, hge->System_GetState(HGE_SCREENHEIGHT) - 97);

    vptpfcPalette = new WIDG::Viewport(vp, VP_FILLCOLOR);
    vptpfcPalette->setVisible(0);
    wintpFillColor->add(vptpfcPalette, 0, 0);

    buttpfcShow = new SHR::But(GV->hGfxInterface, GETL(Lang_SelectFill));
    buttpfcShow->setDimension(gcn::Rectangle(0, 0, 180, 33));
    buttpfcShow->addActionListener(mainListener);
    wintpFillColor->add(buttpfcShow, 5, 9);

    winSearchObj = new SHR::Win(&GV->gcnParts, GETL(Lang_SearchObject));
    winSearchObj->setDimension(gcn::Rectangle(0, 0, 450, 135));
    winSearchObj->setMovable(1);
    winSearchObj->setVisible(0);
    winSearchObj->setClose(1);
    conMain->add(winSearchObj, hge->System_GetState(HGE_SCREENWIDTH) / 2 - 225,
                 hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 257);

    butObjSearchSelectAll = new SHR::But(GV->hGfxInterface, GETL2S("ObjectSearch", "SelectAll"));
    butObjSearchSelectAll->adjustSize();
    butObjSearchSelectAll->setHeight(33);
    butObjSearchSelectAll->addActionListener(mainListener);
    butObjSearchSelectAll->setVisible(0);
    winSearchObj->add(butObjSearchSelectAll, winSearchObj->getWidth() - butObjSearchSelectAll->getWidth() - 10, 65);

    cbObjSearchCaseSensitive = new SHR::CBox(GV->hGfxInterface, GETL2S("ObjectSearch", "CaseSensitive"));
    cbObjSearchCaseSensitive->adjustSize();
    cbObjSearchCaseSensitive->addActionListener(mainListener);
    winSearchObj->add(cbObjSearchCaseSensitive, 5, 65);

    labObjSearchResults = new SHR::Lab(GETL2S("ObjectSearch", "NoResults"));
    labObjSearchResults->adjustSize();
    labObjSearchResults->setVisible(0);
    winSearchObj->add(labObjSearchResults, 5, 85);

    labobjseaInfo = new SHR::Lab(GETL2S("ObjectSearch", "InputLabel"));
    labobjseaInfo->adjustSize();
    winSearchObj->add(labobjseaInfo, 5, 17);

    ddObjSearchTerm = new SHR::DropDown();
    ddObjSearchTerm->setListModel(new EditingWWlModel(this, LMODEL_SEARCHTERM));
    ddObjSearchTerm->setDimension(gcn::Rectangle(0, 0, winSearchObj->getWidth() - labobjseaInfo->getWidth() - 20, 20));
    ddObjSearchTerm->addActionListener(mainListener);
    ddObjSearchTerm->SetGfx(&GV->gcnParts);
    ddObjSearchTerm->adjustHeight();
    winSearchObj->add(ddObjSearchTerm, labobjseaInfo->getWidth() + 10, 14);

    butObjSearchSelect = new SHR::But(GV->hGfxInterface, GETL2S("ObjectSearch", "GoToObject"));
    butObjSearchSelect->adjustSize();

    vpobjseaRender = new WIDG::Viewport(vp, VP_OBJSEARCH);
    winSearchObj->add(vpobjseaRender, 0, 0);

    winobjseaName = new SHR::TextField("");
    winobjseaName->setDimension(gcn::Rectangle(0, 0, winSearchObj->getWidth() - 15, 20));
    winobjseaName->addActionListener(mainListener);
    winSearchObj->add(winobjseaName, 5, 40);

    sliSearchObj = new SHR::Slider(10);
    sliSearchObj->setOrientation(SHR::Slider::VERTICAL);
    sliSearchObj->setDimension(gcn::Rectangle(0, 0, 11, 423));
    sliSearchObj->setVisible(0);
    sliSearchObj->setMarkerLength(20);
    winSearchObj->add(sliSearchObj, 430, 105);

    winTileProp = new SHR::Win(&GV->gcnParts, GETL(Lang_TileProperties));
    winTileProp->setDimension(gcn::Rectangle(0, 0, 300, 350));
    winTileProp->setVisible(false);
    winTileProp->setMovable(true);
    winTileProp->setClose(true);
    conMain->add(winTileProp, 300, 300);
    buttpPrev = MakeButton(0, 9, Icon_Undo, winTileProp, true, true);
    buttpPrev->SetTooltip(GETL2("Tooltip", Lang_TT_TP_Previous));

    tftpTileID = new SHR::TextField("0");
    tftpTileID->setDimension(gcn::Rectangle(0, 0, 100, 20));
    tftpTileID->addActionListener(mainListener);
    winTileProp->add(tftpTileID, 100, 15);

    buttpNext = MakeButton(264, 9, Icon_Redo, winTileProp, 1, 1);
    buttpNext->SetTooltip(GETL2("Tooltip", Lang_TT_TP_Next));

    tftpW = new SHR::TextField("0");
    tftpW->setDimension(gcn::Rectangle(0, 0, 35, 20));
    tftpW->addActionListener(mainListener);
    winTileProp->add(tftpW, 220, 48);

    tftpH = new SHR::TextField("0");
    tftpH->setDimension(gcn::Rectangle(0, 0, 35, 20));
    tftpH->addActionListener(mainListener);
    winTileProp->add(tftpH, 220, 73);

    buttpZoom = MakeButton(264, 60, Icon_Zoom, winTileProp, 1, 1);
    buttpZoom->SetTooltip(GETL2("Tooltip", Lang_TT_TP_Zoom));
    buttpShow = MakeButton(264, 100, Icon_Eye, winTileProp, 1, 1);
    buttpShow->SetTooltip(GETL2("Tooltip", Lang_TT_TP_Preview));
    //buttpPipette = MakeButton(265, 115, Icon_Pipette, winTileProp, 0);
    //buttpPipette->SetTooltip(GETL2("Tooltip", Lang_TT_TP_Pipette));
    buttpApply = MakeButton(264, 140, Icon_Apply, winTileProp, 1, 1);
    buttpApply->SetTooltip(GETL2("Tooltip", Lang_TT_TP_Apply));

    rbtpSingle = new SHR::RadBut(GV->hGfxInterface, GETL(Lang_AttribSingle), "atribtype");
    rbtpSingle->adjustSize();
    rbtpSingle->addActionListener(mainListener);
    winTileProp->add(rbtpSingle, 145, 120);
    rbtpDouble = new SHR::RadBut(GV->hGfxInterface, GETL(Lang_AttribDouble), "atribtype");
    rbtpDouble->adjustSize();
    rbtpDouble->addActionListener(mainListener);
    winTileProp->add(rbtpDouble, 145, 140);
    rbtpMask = new SHR::RadBut(GV->hGfxInterface, GETL(Lang_Mask), "atribtype");
    rbtpMask->adjustSize();
    rbtpMask->addActionListener(mainListener);
    winTileProp->add(rbtpMask, 145, 160);

    for (int i = 0; i < 5; i++) {
        rbtpIn[i] = new SHR::RadBut(GV->hGfxInterface, "", "atribinside");
        rbtpIn[i]->addActionListener(mainListener);
        rbtpIn[i]->adjustSize();
        rbtpOut[i] = new SHR::RadBut(GV->hGfxInterface, "", "atriboutside");
        rbtpOut[i]->addActionListener(mainListener);
        rbtpOut[i]->adjustSize();
    }
    for (int i = 0; i < 2; i++) {
        winTileProp->add(rbtpIn[i], 107 + i * 100, 192);
    }
    for (int i = 2; i < 5; i++) {
        winTileProp->add(rbtpIn[i], 7 + (i - 2) * 100, 222);
    }

    for (int i = 0; i < 2; i++) {
        winTileProp->add(rbtpOut[i], 107 + i * 100, 252);
    }
    for (int i = 2; i < 5; i++) {
        winTileProp->add(rbtpOut[i], 7 + (i - 2) * 100, 282);
    }

    tftpX1 = new SHR::TextField("0");
    tftpX1->setDimension(gcn::Rectangle(0, 0, 35, 20));
    tftpX1->addActionListener(mainListener);
    winTileProp->add(tftpX1, 110, 310);
    tftpY1 = new SHR::TextField("0");
    tftpY1->setDimension(gcn::Rectangle(0, 0, 35, 20));
    tftpY1->addActionListener(mainListener);
    winTileProp->add(tftpY1, 160, 310);
    tftpX2 = new SHR::TextField("0");
    tftpX2->setDimension(gcn::Rectangle(0, 0, 35, 20));
    tftpX2->addActionListener(mainListener);
    winTileProp->add(tftpX2, 210, 310);
    tftpY2 = new SHR::TextField("0");
    tftpY2->setDimension(gcn::Rectangle(0, 0, 35, 20));
    tftpY2->addActionListener(mainListener);
    winTileProp->add(tftpY2, 260, 310);

    itpSelectedTile = -1;
    vpTileProp = new WIDG::Viewport(vp, VP_TILEPROP);
    winTileProp->add(vpTileProp, 0, 0);

    wintoolSelArea = new SHR::Win(&GV->gcnParts, GETL2S("ObjProp", "SelVals"));
    wintoolSelArea->setDimension(gcn::Rectangle(0, 0, 340, 165));
    wintoolSelArea->setShow(false);
    wintoolSelArea->setClose(true);
    wintoolSelArea->addActionListener(mainListener);
    conMain->add(wintoolSelArea, vPort->GetX(), vPort->GetY() + vPort->GetHeight() - 125);

    char label[200];
    sprintf(label, "~w~X1: ~y~0~w~, Y1: ~y~0~w~, X2: ~y~0~w~, Y2: ~y~0~w~");
    labtoolSelAreaValues = new SHR::Lab(label);
    labtoolSelAreaValues->adjustSize();
    wintoolSelArea->add(labtoolSelAreaValues, 7, 13);

    buttoolSelAreaPickMinX = new SHR::But(GV->hGfxInterface, GETL2S("ObjProp", "PickX1"));
    buttoolSelAreaPickMinX->setDimension(gcn::Rectangle(0, 0, 160, 33));
    buttoolSelAreaPickMinX->addActionListener(mainListener);
    wintoolSelArea->add(buttoolSelAreaPickMinX, 5, 35);
    buttoolSelAreaPickMinY = new SHR::But(GV->hGfxInterface, GETL2S("ObjProp", "PickY1"));
    buttoolSelAreaPickMinY->setDimension(gcn::Rectangle(0, 0, 160, 33));
    buttoolSelAreaPickMinY->addActionListener(mainListener);
    wintoolSelArea->add(buttoolSelAreaPickMinY, 5, 70);
    buttoolSelAreaPickMaxX = new SHR::But(GV->hGfxInterface, GETL2S("ObjProp", "PickX2"));
    buttoolSelAreaPickMaxX->setDimension(gcn::Rectangle(0, 0, 160, 33));
    buttoolSelAreaPickMaxX->addActionListener(mainListener);
    wintoolSelArea->add(buttoolSelAreaPickMaxX, 170, 35);
    buttoolSelAreaPickMaxY = new SHR::But(GV->hGfxInterface, GETL2S("ObjProp", "PickY2"));
    buttoolSelAreaPickMaxY->setDimension(gcn::Rectangle(0, 0, 160, 33));
    buttoolSelAreaPickMaxY->addActionListener(mainListener);
    wintoolSelArea->add(buttoolSelAreaPickMaxY, 170, 70);

    buttoolSelAreaAll = new SHR::But(GV->hGfxInterface, GETL2S("ObjProp", "PickByArea"));
    buttoolSelAreaAll->setDimension(gcn::Rectangle(0, 0, 160, 33));
    buttoolSelAreaAll->addActionListener(mainListener);
    wintoolSelArea->add(buttoolSelAreaAll, 5, 110);

    buttoolSelAreaOK = new SHR::But(GV->hGfxInterface, GETL2S("ObjProp", "Accept"));
    buttoolSelAreaOK->setDimension(gcn::Rectangle(0, 0, 160, 33));
    buttoolSelAreaOK->addActionListener(mainListener);
    wintoolSelArea->add(buttoolSelAreaOK, 170, 110);

    winMeasureOpt = new SHR::Win(&GV->gcnParts, GETL(Lang_MeasureOptions));
    winMeasureOpt->setDimension(gcn::Rectangle(0, 0, 300, 100));
    winMeasureOpt->setVisible(0);
    winMeasureOpt->addActionListener(mainListener);
    winMeasureOpt->setClose(1);
    conMain->add(winMeasureOpt, 0, 0);

    cbmeasAbsoluteDistance = new SHR::CBox(GV->hGfxInterface, GETL(Lang_ShowAbsoluteDistance));
    cbmeasAbsoluteDistance->adjustSize();
    winMeasureOpt->add(cbmeasAbsoluteDistance, 5, 15);

    butmeasClear = MakeButton(260, 15, Icon_X, winMeasureOpt);

    winObjectBrush = new SHR::Win(&GV->gcnParts, GETL(Lang_BrushProperties));
    winObjectBrush->setDimension(gcn::Rectangle(0, 0, 330, 165));
    winObjectBrush->setVisible(false);
    winObjectBrush->setClose(true);
    winObjectBrush->addActionListener(mainListener);
    conMain->add(winObjectBrush, 0, 0);

    char tmp[128];
    int offsetY = 18;

    cbobrUpdateRects = new SHR::CBox(GV->hGfxInterface, GETL2S("Tool_ObjectBrush", "UpdateRects"));
    cbobrUpdateRects->adjustSize();
    cbobrUpdateRects->setSelected(true);
    winObjectBrush->add(cbobrUpdateRects, 5, offsetY);

    offsetY += 25;

    labobrDispX = new SHR::Lab(GETL2S("Tool_ObjectBrush", "ScatterX"));
    labobrDispX->adjustSize();
    winObjectBrush->add(labobrDispX, 5, offsetY);

    tfobrDispX = new SHR::TextField("0");
    tfobrDispX->setDimension(gcn::Rectangle(0, 0, 50, 20));
    tfobrDispX->SetNumerical(1, 0);
    winObjectBrush->add(tfobrDispX, labobrDispX->getWidth() + 15, offsetY - 2);

    labobrDispY = new SHR::Lab(GETL2S("Tool_ObjectBrush", "ScatterY"));
    labobrDispY->adjustSize();
    winObjectBrush->add(labobrDispY, tfobrDispX->getX() + tfobrDispX->getWidth() + 13, offsetY);

    tfobrDispY = new SHR::TextField("0");
    tfobrDispY->setDimension(gcn::Rectangle(0, 0, 50, 20));
    tfobrDispY->SetNumerical(1, 0);
    winObjectBrush->add(tfobrDispY, labobrDispY->getX() + labobrDispY->getWidth() + 13, offsetY - 2);

    offsetY += 25;

    cbobrApplyScatterSeparately = new SHR::CBox(GV->hGfxInterface, GETL2S("Tool_ObjectBrush", "ScatterSeparately"));
    cbobrApplyScatterSeparately->adjustSize();
    winObjectBrush->add(cbobrApplyScatterSeparately, 5, offsetY);

    offsetY += 25;

    sprintf(tmp, "%s: 64px", GETL(Lang_BrushInterval));
    labobrDistance = new SHR::Lab(tmp);
    labobrDistance->adjustSize();
    winObjectBrush->add(labobrDistance, 5, offsetY);

    offsetY += 25;

    sliobrDistance = new SHR::Slider(SHR::Slider::HORIZONTAL);
    sliobrDistance->setScale(10.0f, 700.0f);
    sliobrDistance->setDimension(gcn::Rectangle(0, 0, 315, 16));
    sliobrDistance->setValue(64.0f);
    sliobrDistance->addActionListener(mainListener);
    sliobrDistance->setMarkerLength(45);
    winObjectBrush->add(sliobrDistance, 5, offsetY);

    winSpacing = new SHR::Win(&GV->gcnParts, GETL2S("WinSpacing", "WinCaption"));
    winSpacing->setDimension(gcn::Rectangle(0, 0, 250, 100));
    winSpacing->setVisible(0);
    winSpacing->setClose(1);
    winSpacing->addActionListener(mainListener);
    conMain->add(winSpacing, hge->System_GetState(HGE_SCREENWIDTH) / 2 - 100,
                 hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 50);

    labspacingDistance = new SHR::Lab(GETL2S("WinSpacing", "Distance"));
    labspacingDistance->adjustSize();
    winSpacing->add(labspacingDistance, 5, 17);

    tfspacingDistance = new SHR::TextField("0");
    tfspacingDistance->setDimension(gcn::Rectangle(0, 0, 100, 20));
    tfspacingDistance->SetNumerical(1, 1);
    winSpacing->add(tfspacingDistance, 115, 15);

    butspacingOK = new SHR::But(GV->hGfxInterface, GETL(Lang_OK));
    butspacingOK->setDimension(gcn::Rectangle(0, 0, 100, 33));
    butspacingOK->addActionListener(mainListener);
    winSpacing->add(butspacingOK, 75, 40);

    winCamera = new SHR::Win(&GV->gcnParts, GETL(Lang_Camera));
    winCamera->setDimension(gcn::Rectangle(0, 0, 200, 200));
    winCamera->setVisible(0);
    winCamera->setClose(1);
    conMain->add(winCamera, 300, 300);

    labcamCoordX = new SHR::Lab(GETL(Lang_LocationX));
    labcamCoordX->adjustSize();
    winCamera->add(labcamCoordX, 5, 75);

    tfcamSetToX = new SHR::TextField("0");
    tfcamSetToX->setDimension(gcn::Rectangle(0, 0, 75, 20));
    winCamera->add(tfcamSetToX, 115, 75);

    labcamCoordY = new SHR::Lab(GETL(Lang_LocationY));
    labcamCoordY->adjustSize();
    winCamera->add(labcamCoordY, 5, 110);

    tfcamSetToY = new SHR::TextField("0");
    tfcamSetToY->setDimension(gcn::Rectangle(0, 0, 75, 20));
    winCamera->add(tfcamSetToY, 115, 110);

    butcamSetTo = new SHR::But(GV->hGfxInterface, GETL(Lang_SetToCoords));
    butcamSetTo->setDimension(gcn::Rectangle(0, 0, 90, 33));
    butcamSetTo->addActionListener(mainListener);
    winCamera->add(butcamSetTo, 5, 145);

    butcamSetToSpawn = new SHR::But(GV->hGfxInterface, GETL(Lang_SetToSpawn));
    butcamSetToSpawn->setDimension(gcn::Rectangle(0, 0, 90, 33));
    butcamSetToSpawn->addActionListener(mainListener);
    winCamera->add(butcamSetToSpawn, 105, 145);

    winLogicBrowser = new SHR::Win(&GV->gcnParts, GETL2S("Win_LogicBrowser", "WinCaption"));
    winLogicBrowser->setDimension(gcn::Rectangle(0, 0, 300, 225));
    winLogicBrowser->setVisible(false);
    winLogicBrowser->setClose(true);
    winLogicBrowser->addActionListener(mainListener);
    conMain->add(winLogicBrowser, hge->System_GetState(HGE_SCREENWIDTH) / 2 - 150,
                 hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 110);

    labbrlLogics = new SHR::Lab(GETL2S("Win_LogicBrowser", "SelectCustomLogic"));
    labbrlLogics->adjustSize();
    labbrlLogicName = new SHR::Lab(GETL2S("Win_LogicBrowser", "LogicName"));
    labbrlLogicName->adjustSize();
    labbrlFilePath = new SHR::Lab(GETL2S("Win_LogicBrowser", "FilePath"));
    labbrlFilePath->adjustSize();
    labbrlFileSize = new SHR::Lab(GETL2S("Win_LogicBrowser", "FileSize"));
    labbrlFileSize->adjustSize();
    labbrlFileChecksum = new SHR::Lab(GETL2S("Win_LogicBrowser", "FileChecksum"));
    labbrlFileChecksum->adjustSize();
    labbrlFileModDate = new SHR::Lab(GETL2S("Win_LogicBrowser", "ModDate"));
    labbrlFileModDate->adjustSize();

    labbrlLogicNameV = new SHR::Lab("");
    labbrlFilePathV = new SHR::Lab("");
    labbrlFileSizeV = new SHR::Lab("");
    labbrlFileChecksumV = new SHR::Lab("");
    labbrlFileModDateV = new SHR::Lab("");

    lbbrlLogicList = new SHR::ListBox(0);
    lbbrlLogicList->addActionListener(mainListener);
    lbbrlLogicList->setDimension(gcn::Rectangle(0, 0, 280, 130));

    winLogicBrowser->add(labbrlLogics, 8, 15);

    sabrlLogicList = new SHR::ScrollArea(lbbrlLogicList, SHR::ScrollArea::SHOW_NEVER, SHR::ScrollArea::SHOW_AUTO);
    sabrlLogicList->setDimension(gcn::Rectangle(0, 0, 280, 130));
    winLogicBrowser->add(sabrlLogicList, 8, 40);

    butbrlNew = new SHR::But(GV->hGfxInterface, GETL2S("Win_LogicBrowser", "AddNew"));
	butbrlNew->setIcon(GV->sprIcons16[Icon16_Add]);
	butbrlNew->setDimension(gcn::Rectangle(0, 0, 280, 26));
	butbrlNew->addActionListener(mainListener);
	winLogicBrowser->add(butbrlNew, 8, 176);

    butbrlBrowseDir = new SHR::But(GV->hGfxInterface, GETL2S("Win_LogicBrowser", "BrowseDir"));
    butbrlBrowseDir->setIcon(GV->sprIcons16[Icon16_Open]);
    butbrlBrowseDir->setDimension(gcn::Rectangle(0, 0, 235, 26));
    butbrlBrowseDir->addActionListener(mainListener);

    butbrlEdit = new SHR::But(GV->hGfxInterface, GETL2S("Win_LogicBrowser", "Edit"));
    butbrlEdit->setIcon(GV->sprIcons16[Icon16_Pencil]);
    butbrlEdit->setDimension(gcn::Rectangle(0, 0, 235, 26));
    butbrlEdit->addActionListener(mainListener);

    butbrlRename = new SHR::But(GV->hGfxInterface, GETL2S("Win_LogicBrowser", "Rename"));
    butbrlRename->setIcon(GV->sprIcons16[Icon16_WriteID]);
    butbrlRename->setDimension(gcn::Rectangle(0, 0, 235, 26));
    butbrlRename->addActionListener(mainListener);

    butbrlDelete = new SHR::But(GV->hGfxInterface, GETL2S("Win_LogicBrowser", "Delete"));
    butbrlDelete->setIcon(GV->sprIcons16[Icon16_Trash]);
    butbrlDelete->setDimension(gcn::Rectangle(0, 0, 235, 26));
    butbrlDelete->addActionListener(mainListener);

    winDB = new SHR::Win(&GV->gcnParts, GETL(Lang_Database));
    winDB->setDimension(gcn::Rectangle(0, 0, 800, 600));
    winDB->setVisible(0);
    winDB->setClose(1);
    conMain->add(winDB, hge->System_GetState(HGE_SCREENWIDTH) / 2 - 400,
                 hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 300);


    condbTiles = new SHR::Container();
    condbTiles->setDimension(gcn::Rectangle(0, 0, 790, 570));
    condbImages = new SHR::Container();
    condbImages->setDimension(gcn::Rectangle(0, 0, 790, 570));
    condbAnims = new SHR::Container();
    condbAnims->setDimension(gcn::Rectangle(0, 0, 790, 570));
    condbSounds = new SHR::Container();
    condbSounds->setDimension(gcn::Rectangle(0, 0, 790, 570));

    tadbTabs = new SHR::TabbedArea();
    tadbTabs->addTab(GETL2S("WinDatabase", "Tab_Tilesets"), condbTiles);
    tadbTabs->addTab(GETL2S("WinDatabase", "Tab_Imagesets"), condbImages);
    tadbTabs->addTab(GETL2S("WinDatabase", "Tab_Anims"), condbAnims);
    tadbTabs->addTab(GETL2S("WinDatabase", "Tab_Sounds"), condbSounds);
    tadbTabs->setDimension(gcn::Rectangle(0, 0, 790, 570));

    winDB->add(tadbTabs, 5, 10);

    //database - imagesets
    dbAssetsImg = new SHR::ListBox(new EditingWWlModel(this, LMODEL_ASSETS));
    dbAssetsImg->setDimension(gcn::Rectangle(0, 0, 600, 200));
    dbAssetsImg->addActionListener(mainListener);

    labdbisSetChecksum = new SHR::Lab(GETL2S("WinDatabase", "SetChecksum"));
    labdbisSetChecksum->adjustSize();
    condbImages->add(labdbisSetChecksum, 5, 5);

    labdbisSetFileCount = new SHR::Lab("");
    labdbisSetFileCount->adjustSize();
    condbImages->add(labdbisSetFileCount, 5, 45);

    labdbisSetFileSize = new SHR::Lab("");
    labdbisSetFileSize->adjustSize();
    condbImages->add(labdbisSetFileSize, 5, 65);

    for (int i = 0; i < 3; i++) {
        char ident[64];
        sprintf(ident, "Show_%d", i + 1);
        rbdbisShow[i] = new SHR::RadBut(GV->hGfxInterface, GETL2S("WinDatabase", ident), "db_is_show");
        rbdbisShow[i]->adjustSize();
        condbImages->add(rbdbisShow[i], 10 + i * 100, 520);
    }
    rbdbisShow[0]->setSelected(1);

    labdbisFileChecksum = new SHR::Lab(GETL2S("WinDatabase", "FileChecksum"));
    labdbisFileChecksum->adjustSize();
    condbImages->add(labdbisFileChecksum, 490, 5);

    labdbisFileIndex = new SHR::Lab(GETL2S("WinDatabase", "FileIndex"));
    labdbisFileIndex->adjustSize();
    condbImages->add(labdbisFileIndex, 490, 45);

    labdbisFrameID = new SHR::Lab(GETL2S("WinDatabase", "FileFrame"));
    labdbisFrameID->adjustSize();
    condbImages->add(labdbisFrameID, 490, 65);

    labdbisFileID = new SHR::Lab(GETL2S("WinDatabase", "FileID"));
    labdbisFileID->adjustSize();
    condbImages->add(labdbisFileID, 490, 85);

    labdbisFileSize = new SHR::Lab(GETL2S("WinDatabase", "FileSize"));
    labdbisFileSize->adjustSize();
    condbImages->add(labdbisFileSize, 490, 105);

    labdbisFileDim = new SHR::Lab(GETL2S("WinDatabase", "FileDim"));
    labdbisFileDim->adjustSize();
    condbImages->add(labdbisFileDim, 490, 125);

    labdbisFileOffset = new SHR::Lab(GETL2S("WinDatabase", "FileOffset"));
    labdbisFileOffset->adjustSize();
    condbImages->add(labdbisFileOffset, 490, 145);

    labdbisFileUserData = new SHR::Lab(GETL2S("WinDatabase", "FileUserData"));
    labdbisFileUserData->adjustSize();
    condbImages->add(labdbisFileUserData, 490, 165);

    sadbAssetsImg = new SHR::ScrollArea(dbAssetsImg, SHR::ScrollArea::SHOW_AUTO, SHR::ScrollArea::SHOW_ALWAYS);
    sadbAssetsImg->setDimension(gcn::Rectangle(0, 0, 300, 445));
    sadbAssetsImg->setOpaque(0);
    condbImages->add(sadbAssetsImg, 0, 90);

    dbFramesImg = new SHR::ListBox(new EditingWWlModel(this, LMODEL_ASSETFRAMES));
    dbFramesImg->setDimension(gcn::Rectangle(0, 0, 280, 200));

    sadbFramesImg = new SHR::ScrollArea(dbFramesImg, SHR::ScrollArea::SHOW_AUTO, SHR::ScrollArea::SHOW_ALWAYS);
    sadbFramesImg->setDimension(gcn::Rectangle(0, 0, 182, 206));
    sadbFramesImg->setOpaque(0);
    sadbFramesImg->setVisible(0);
    condbImages->add(sadbFramesImg, 302, 0);

    cbdbisBorder = new SHR::CBox(GV->hGfxInterface, GETL2S("WinDatabase", "Border"));
    cbdbisBorder->adjustSize();
    condbImages->add(cbdbisBorder, 305, 210);

    cbdbisOffsetBorder = new SHR::CBox(GV->hGfxInterface, GETL2S("WinDatabase", "OffsetBorder"));
    cbdbisOffsetBorder->adjustSize();
    condbImages->add(cbdbisOffsetBorder, 305 + 121, 210);

    cbdbisFlipX = new SHR::CBox(GV->hGfxInterface, GETL2S("WinDatabase", "FlipX"));
    cbdbisFlipX->adjustSize();
    condbImages->add(cbdbisFlipX, 305 + 121 * 2, 210);

    cbdbisFlipY = new SHR::CBox(GV->hGfxInterface, GETL2S("WinDatabase", "FlipY"));
    cbdbisFlipY->adjustSize();
    condbImages->add(cbdbisFlipY, 305 + 121 * 3, 210);

    vpDB = new WIDG::Viewport(vp, VP_DB);
    winDB->add(vpDB, 0, 0);

    //database - animations
    dbAssetsAni = new SHR::ListBox(new EditingWWlModel(this, LMODEL_ANIMATIONS));
    dbAssetsAni->setDimension(gcn::Rectangle(0, 0, 450, 200));
    dbAssetsAni->addActionListener(mainListener);

    sadbAssetsAni = new SHR::ScrollArea(dbAssetsAni, SHR::ScrollArea::SHOW_NEVER, SHR::ScrollArea::SHOW_ALWAYS);
    sadbAssetsAni->setDimension(gcn::Rectangle(0, 0, 450, 135));
    sadbAssetsAni->setOpaque(0);
    condbAnims->add(sadbAssetsAni, 0, 0);

    dbFramesAni = new SHR::ListBox(new EditingWWlModel(this, LMODEL_ANIMFRAMES));
    dbFramesAni->setDimension(gcn::Rectangle(0, 0, 332, 200));

    sadbFramesAni = new SHR::ScrollArea(dbFramesAni, SHR::ScrollArea::SHOW_NEVER, SHR::ScrollArea::SHOW_ALWAYS);
    sadbFramesAni->setDimension(gcn::Rectangle(0, 0, 332, 135));
    sadbFramesAni->setOpaque(0);
    sadbFramesAni->setVisible(0);
    condbAnims->add(sadbFramesAni, 455, 0);

    fdbAniTimer = 0;

    cbdbBorderAni = new SHR::CBox(GV->hGfxInterface, GETL(Lang_Border));
    cbdbBorderAni->adjustSize();
    condbAnims->add(cbdbBorderAni, 5, 140);

    cbdbAniAnimate = new SHR::CBox(GV->hGfxInterface, GETL(Lang_Animate));
    cbdbAniAnimate->adjustSize();
    condbAnims->add(cbdbAniAnimate, cbdbBorderAni->getWidth() + 10, 140);

    cbdbAniLoop = new SHR::CBox(GV->hGfxInterface, GETL2("WinDatabase", Lang_Db_Loop));
    cbdbAniLoop->adjustSize();
    condbAnims->add(cbdbAniLoop, cbdbAniAnimate->getX() + cbdbAniAnimate->getWidth() + 5, 140);

    cbdbFlip = new SHR::CBox(GV->hGfxInterface, GETL(Lang_Mirror));
    cbdbFlip->adjustSize();
    condbAnims->add(cbdbFlip, cbdbAniLoop->getX() + cbdbAniLoop->getWidth() + 5, 140);

    labdbAnimSpeed = new SHR::Lab(GETL2("WinDatabase", Lang_Db_AnimSpeed));
    labdbAnimSpeed->adjustSize();
    condbAnims->add(labdbAnimSpeed, 5, 160);

    //database - sounds
    dbAssetsSnd = new SHR::ListBox(new EditingWWlModel(this, LMODEL_SOUNDS_CLEAR));
    dbAssetsSnd->setDimension(gcn::Rectangle(0, 0, 300, 540));
    dbAssetsSnd->addActionListener(mainListener);

    sadbAssetsSnd = new SHR::ScrollArea(dbAssetsSnd, SHR::ScrollArea::SHOW_NEVER, SHR::ScrollArea::SHOW_ALWAYS);
    sadbAssetsSnd->setDimension(gcn::Rectangle(0, 0, 300, 540));
    sadbAssetsSnd->setOpaque(0);
    condbSounds->add(sadbAssetsSnd, 0, 0);

    spdbSounds = new cSoundPlayer(condbSounds, 350, 230, NULL);

    vpDB = new WIDG::Viewport(vp, VP_DB);
    condbAnims->add(vpDB, 0, 0);

    {
        int x = labdbAnimSpeed->getX() + labdbAnimSpeed->getWidth() + 5;
        dddbAnimSpeed = new SHR::DropDown();
        dddbAnimSpeed->setListModel(new EditingWWlModel(this, LMODEL_ANIMSPEED));
        dddbAnimSpeed->setDimension(gcn::Rectangle(0, 0, cbdbAniLoop->getX() + cbdbAniLoop->getWidth() - x, 20));
        dddbAnimSpeed->SetGfx(&GV->gcnParts);
        dddbAnimSpeed->adjustHeight();
        dddbAnimSpeed->setSelected(3);
        condbAnims->add(dddbAnimSpeed, x, 160);
    }

    labdbAniImageSet = new SHR::Lab(GETL2("WinDatabase", Lang_Db_AniImageset));
    labdbAniImageSet->adjustSize();
    condbAnims->add(labdbAniImageSet, cbdbFlip->getX() + cbdbFlip->getWidth() + 50, 140);
    tfdbAniImageSet = new SHR::TextField();
    tfdbAniImageSet->adjustHeight();
    tfdbAniImageSet->setWidth(350);
    condbAnims->add(tfdbAniImageSet, labdbAniImageSet->getX() + labdbAniImageSet->getWidth() + 10, 140);

    winMapShot = new SHR::Win(&GV->gcnParts, GETL(Lang_MapShot));
    winMapShot->setDimension(gcn::Rectangle(0, 0, 500, 300));
    winMapShot->setVisible(0);
    winMapShot->setClose(1);
    conMain->add(winMapShot, 200, 200);

    tfmsSaveAs = new SHR::TextField();
    tfmsSaveAs->setDimension(gcn::Rectangle(0, 0, 348, 20));
    tfmsSaveAs->setText("");
    tfmsSaveAs->addActionListener(mainListener);
    winMapShot->add(tfmsSaveAs, 100, 15);

    butmsSaveAs = new SHR::But(GV->hGfxInterface, GV->sprIcons[Icon_Open]);
    butmsSaveAs->setDimension(gcn::Rectangle(0, 0, 32, 32));
    butmsSaveAs->addActionListener(mainListener);
    winMapShot->add(butmsSaveAs, 458, 9);

    butmsSave = new SHR::But(GV->hGfxInterface, GETL(Lang_Save));
    butmsSave->setDimension(gcn::Rectangle(0, 0, 100, 33));
    butmsSave->addActionListener(mainListener);
    winMapShot->add(butmsSave, 390, 245);

    labmsSaveAs = new SHR::Lab(GV->Lang->GetString("Strings", Lang_SaveAs));
    labmsSaveAs->adjustSize();
    winMapShot->add(labmsSaveAs, 5, 15);

    sprintf(tmp, "%s: 25.0%%", GV->Lang->GetString("Strings", Lang_Scale));
    labmsScale = new SHR::Lab(tmp);
    labmsScale->adjustSize();
    winMapShot->add(labmsScale, 5, 47);

    slimsScale = new SHR::Slider();
    slimsScale->setOrientation(SHR::Slider::HORIZONTAL);
    slimsScale->setScaleStart(0.0f);
    slimsScale->setScaleEnd(100.0f);
    slimsScale->setDimension(gcn::Rectangle(0, 0, 390, 11));
    slimsScale->setValue(25.0f);
    slimsScale->setStyle(SHR::Slider::BALL);
    slimsScale->addActionListener(mainListener);
    winMapShot->add(slimsScale, 100, 50);

    labmsPlane = new SHR::Lab(GETL(Lang_Plane));
    labmsPlane->adjustSize();
    winMapShot->add(labmsPlane, 5, 75);

    ddmsPlane = new SHR::DropDown();
    ddmsPlane->setListModel(new EditingWWlModel(this, LMODEL_PLANES));
    ddmsPlane->setDimension(gcn::Rectangle(0, 0, 250, 20));
    ddmsPlane->addActionListener(mainListener);
    ddmsPlane->SetGfx(&GV->gcnParts);
    ddmsPlane->adjustHeight();
    ddmsPlane->disableScroll(1);
    if (hParser != NULL)
        ddmsPlane->setSelected(GetActivePlaneID());
    winMapShot->add(ddmsPlane, 100, 72);

    cbmsDrawObjects = new SHR::CBox(GV->hGfxInterface, GETL2("Mapshot", Lang_Ms_DrawObjects));
    cbmsDrawObjects->adjustSize();
    winMapShot->add(cbmsDrawObjects, 360, 72);

    cbmsOpenFileDirectory = new SHR::CBox(GV->hGfxInterface, GETL2S("Mapshot", "OpenFileDir"));
    cbmsOpenFileDirectory->adjustSize();
    cbmsOpenFileDirectory->setSelected(1);
    winMapShot->add(cbmsOpenFileDirectory, 5, 255);

    cpmsPicker = new cColorPicker(winMapShot, 5, 120, 0xFFFFFFFF);
    cpmsPicker->SetName(GETL2("Mapshot", Lang_Ms_BackgroundColor));

    labmsCompression = new SHR::Lab(GETL2("Mapshot", Lang_Ms_CompressionLevel));
    labmsCompression->adjustSize();
    labmsCompression->setVisible(0);
    winMapShot->add(labmsCompression, 260, 120);

    labmsCompDisc = new SHR::Lab("");
    labmsCompDisc->setDimension(gcn::Rectangle(0, 0, 230, 80));
    labmsCompDisc->setVisible(0);
    winMapShot->add(labmsCompDisc, 260, 160);

    slimsCompression = new SHR::Slider(0, 100);
    slimsCompression->setDimension(gcn::Rectangle(0, 0, 230, 20));
    slimsCompression->setVisible(0);
    slimsCompression->setStyle(SHR::Slider::POINTER);
    slimsCompression->addActionListener(mainListener);
    winMapShot->add(slimsCompression, 260, 145);

    if (hParser != NULL) {
        char tmp[128];
        sprintf(tmp, GETL(Lang_MapShotDimensions), int(GetActivePlane()->GetPlaneWidthPx() / 4.0f),
                int(GetActivePlane()->GetPlaneHeightPx() / 4.0f));
        labmsDimensions = new SHR::Lab(tmp);
    } else {
        labmsDimensions = new SHR::Lab("");
    }
    labmsDimensions->adjustSize();
    winMapShot->add(labmsDimensions, 5, 97);

    winpmMain = new SHR::Win(&GV->gcnParts, GETL2S("PlaneProperties", "Caption"));
    winpmMain->setDimension(gcn::Rectangle(0, 0, 220, 300));
    winpmMain->setVisible(false);
    winpmMain->setClose(true);
    conMain->add(winpmMain, hge->System_GetState(HGE_SCREENWIDTH) / 2 - 112,
                 hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 162);

    lbpmPlanes = new SHR::ListBox(new EditingWWlModel(this, LMODEL_PLANESPROP));
    lbpmPlanes->setDimension(gcn::Rectangle(0, 0, 200, 235));
    lbpmPlanes->addActionListener(mainListener);
    lbpmPlanes->setOpaque(false);

    sapmPlanes = new SHR::ScrollArea(lbpmPlanes, SHR::ScrollArea::SHOW_NEVER,SHR::ScrollArea::SHOW_AUTO);
    sapmPlanes->setDimension(gcn::Rectangle(0, 0, 200, 235));
    winpmMain->add(sapmPlanes, 8, 40);

    labpmPlanes = new SHR::Lab(GETL2S("PlaneProperties", "SelectPlane"));
    labpmPlanes->adjustSize();
    winpmMain->add(labpmPlanes, 8, 16);

    labpmName = new SHR::Lab(GETL(Lang_Name));
    labpmName->adjustSize();
    winpmMain->add(labpmName, 230, 16);

    labpmTileSet = new SHR::Lab(GETL2S("PlaneProperties", "Tileset"));
    labpmTileSet->adjustSize();
    winpmMain->add(labpmTileSet, 230, 46);

    int labW = std::max(labpmName->getWidth(), labpmTileSet->getWidth());

    tfpmName = new SHR::TextField();
    tfpmName->setDimension(gcn::Rectangle(0, 0, (520 - (240 + labW)) - 45, 20));
    winpmMain->add(tfpmName, 240 + labW, 15);

    ddpmTileSet = new SHR::DropDown();
    ddpmTileSet->setDimension(gcn::Rectangle(0, 0, (520 - (240 + labW)) - 45, 20));
    ddpmTileSet->SetGfx(&GV->gcnParts);
    ddpmTileSet->adjustHeight();

    winpmMain->add(ddpmTileSet, 240 + labW, 44);

    labpmOffset = new SHR::Lab(GETL2S("PlaneProperties", "Offset"));
    labpmOffset->adjustSize();
    //winpmMain->add(labpmOffset, 220, 75);

    labpmOffX = new SHR::Lab("X:");
    labpmOffX->adjustSize();
    //winpmMain->add(labpmOffX, 325, 75);
    labpmOffY = new SHR::Lab("Y:");
    labpmOffY->adjustSize();
    //winpmMain->add(labpmOffY, 410, 75);

    tfpmOffX = new SHR::TextField("");
    tfpmOffX->SetNumerical(1);
    tfpmOffX->setDimension(gcn::Rectangle(0, 0, 60, 20));
    //winpmMain->add(tfpmOffX, 345, 73);

    tfpmOffY = new SHR::TextField("");
    tfpmOffY->SetNumerical(1);
    tfpmOffY->setDimension(gcn::Rectangle(0, 0, 60, 20));
    //winpmMain->add(tfpmOffY, 430, 73);

    /*tbpmImagesets = new SHR::TextBox();
	tbpmImagesets->setFont(GV->gcnParts.gcnfntMyriad13);
	tbpmImagesets->setMinimalSize((525-(230+labpmImagesets->getWidth())-15)+3-35, 100);
	tbpmImagesets->setDimension(gcn::Rectangle(0, 0, 20, 20));

	sapmImagesets = new SHR::ScrollArea();
	sapmImagesets->setDimension(gcn::Rectangle(0, 0, (525-(230+labpmImagesets->getWidth()))-35, 100));
	sapmImagesets->setContent(tbpmImagesets);
	sapmImagesets->setVerticalScrollPolicy(SHR::ScrollArea::SHOW_ALWAYS);
	sapmImagesets->setOpaque(0);
	winpmMain->add(sapmImagesets, 230+labpmImagesets->getWidth(), 45);*/

    labpmPlaneSize = new SHR::Lab(GETL2S("PlaneProperties", "Size"));
    labpmPlaneSize->adjustSize();
    winpmMain->add(labpmPlaneSize, 230, 110);
    labpmAnchor = new SHR::Lab(GETL2S("PlaneProperties", "Anchor"));
    labpmAnchor->adjustSize();
    winpmMain->add(labpmAnchor, 525 - 96 - 48, 110);

    labpmWidth = new SHR::Lab(GETL2S("PlaneProperties", "Width"));
    labpmWidth->adjustSize();
    winpmMain->add(labpmWidth, 230, 141);
    labpmHeight = new SHR::Lab(GETL2S("PlaneProperties", "Height"));
    labpmHeight->adjustSize();
    winpmMain->add(labpmHeight, 230, 176);

    labW = std::max(labpmWidth->getWidth(), labpmHeight->getWidth());

    tfpmPlaneSizeX = new SHR::TextField("");
    tfpmPlaneSizeX->SetNumerical(1);
    tfpmPlaneSizeX->setDimension(gcn::Rectangle(0, 0, 120 - labW, 20));
    tfpmPlaneSizeX->addActionListener(mainListener);
    winpmMain->add(tfpmPlaneSizeX, 240 + labW, 140);
    tfpmPlaneSizeY = new SHR::TextField("");
    tfpmPlaneSizeY->SetNumerical(1);
    tfpmPlaneSizeY->setDimension(gcn::Rectangle(0, 0, 120 - labW, 20));
    tfpmPlaneSizeY->addActionListener(mainListener);
    winpmMain->add(tfpmPlaneSizeY, 240 + labW, 175);

    ipmAnchor = 5;
    ipmSizeX = ipmSizeY = -1;

#define OFFSET 19

    int offX = 525 - 96 - 48, offY = 140;

    butpmResUL = MakeButton(offX, offY, Icon16_UpLeft, winpmMain, 1, 1);
    butpmResU = MakeButton(offX + OFFSET, offY, Icon16_Up, winpmMain, 1, 1);
    butpmResUR = MakeButton(offX + 2 * OFFSET, offY, Icon16_UpRight, winpmMain, 1, 1);

    butpmResL = MakeButton(offX, offY + OFFSET, Icon16_Left, winpmMain, 1, 1);
    butpmResC = MakeButton(offX + OFFSET, offY + OFFSET, Icon16_Anchor, winpmMain, 1, 1);
    butpmResR = MakeButton(offX + 2 * OFFSET, offY + OFFSET, Icon16_Right, winpmMain, 1, 1);

    butpmResDL = MakeButton(offX, offY + 2 * OFFSET, Icon16_DownLeft, winpmMain, 1, 1);
    butpmResD = MakeButton(offX + OFFSET, offY + 2 * OFFSET, Icon16_Down, winpmMain, 1, 1);
    butpmResDR = MakeButton(offX + 2 * OFFSET, offY + 2 * OFFSET, Icon16_DownRight, winpmMain, 1, 1);

    labpmTileSize = new SHR::Lab(GETL2S("PlaneProperties", "TileDim"));
    labpmTileSize->adjustSize();
    winpmMain->add(labpmTileSize, 500, 17);

    labpmTileSize_x = new SHR::Lab("x");
    labpmTileSize_x->adjustSize();
    winpmMain->add(labpmTileSize_x, 676, 17);

    tfpmTileSizeX = new SHR::TextField("");
    tfpmTileSizeX->SetNumerical(1);
    tfpmTileSizeX->setDimension(gcn::Rectangle(0, 0, 35, 20));
    winpmMain->add(tfpmTileSizeX, 636, 15);

    tfpmTileSizeY = new SHR::TextField("");
    tfpmTileSizeY->SetNumerical(1);
    tfpmTileSizeY->setDimension(gcn::Rectangle(0, 0, 35, 20));
    winpmMain->add(tfpmTileSizeY, 689, 15);

    labpmMovement = new SHR::Lab(GETL2S("PlaneProperties", "Movement"));
    labpmMovement->adjustSize();
    winpmMain->add(labpmMovement, 500, 47);

    labpmMovX = new SHR::Lab("X:");
    labpmMovX->adjustSize();
    winpmMain->add(labpmMovX, 621, 47);
    labpmMovY = new SHR::Lab("Y:");
    labpmMovY->adjustSize();
    winpmMain->add(labpmMovY, 674, 47);

    tfpmMovX = new SHR::TextField("");
    tfpmMovX->SetNumerical(1);
    tfpmMovX->setDimension(gcn::Rectangle(0, 0, 35, 20));
    winpmMain->add(tfpmMovX, 636, 45);

    tfpmMovY = new SHR::TextField("");
    tfpmMovY->SetNumerical(1);
    tfpmMovY->setDimension(gcn::Rectangle(0, 0, 35, 20));
    winpmMain->add(tfpmMovY, 689, 45);

    labpmZCoord = new SHR::Lab(GETL2S("PlaneProperties", "ZCoord"));
    labpmZCoord->adjustSize();
    winpmMain->add(labpmZCoord, 500, 77);

    tfpmZCoord = new SHR::TextField();
    tfpmZCoord->SetNumerical(1);
    tfpmZCoord->setDimension(gcn::Rectangle(0, 0, 89, 20));
    winpmMain->add(tfpmZCoord, 636, 75);

    labpmFlags = new SHR::Lab(GETL2S("PlaneProperties", "Flags"));
    labpmFlags->adjustSize();
    winpmMain->add(labpmFlags, 500, 110);

    cbpmFlagMainPlane = new SHR::CBox(GV->hGfxInterface, GETL2S("PlaneProperties", "FlagMainPlane"));
    cbpmFlagMainPlane->adjustSize();
    winpmMain->add(cbpmFlagMainPlane, 500, 135);

    cbpmFlagNoDraw = new SHR::CBox(GV->hGfxInterface, GETL2S("PlaneProperties", "FlagNoDraw"));
    cbpmFlagNoDraw->adjustSize();
    winpmMain->add(cbpmFlagNoDraw, 500, 155);

    cbpmFlagWrapX = new SHR::CBox(GV->hGfxInterface, GETL2S("PlaneProperties", "FlagWrapX"));
    cbpmFlagWrapX->adjustSize();
    winpmMain->add(cbpmFlagWrapX, 500, 175);

    cbpmFlagWrapY = new SHR::CBox(GV->hGfxInterface, GETL2S("PlaneProperties", "FlagWrapY"));
    cbpmFlagWrapY->adjustSize();
    winpmMain->add(cbpmFlagWrapY, 500, 195);

    cbpmAutoTileSize = new SHR::CBox(GV->hGfxInterface, GETL2S("PlaneProperties", "FlagAutoTileSize"));
    cbpmAutoTileSize->adjustSize();
    winpmMain->add(cbpmAutoTileSize, 500, 215);

    butpmSave = new SHR::But(GV->hGfxInterface, GETL(Lang_Save));
    butpmSave->setDimension(gcn::Rectangle(0, 0, 100, 33));
    butpmSave->addActionListener(mainListener);
    winpmMain->add(butpmSave, 500, 245);

    butpmDelete = new SHR::But(GV->hGfxInterface, GETL(Lang_Delete));
    butpmDelete->setDimension(gcn::Rectangle(0, 0, 100, 33));
    butpmDelete->addActionListener(mainListener);
    winpmMain->add(butpmDelete, 610, 245);

    vppm = new WIDG::Viewport(vp, VP_PLANEPROPERTIES);
    winpmMain->add(vppm, 0, 0);

    winWorld = new SHR::Win(&GV->gcnParts, GETL(Lang_MapProperties));
    winWorld->setDimension(gcn::Rectangle(0, 0, 400, 290));
    winWorld->setVisible(false);
    winWorld->setClose(true);
    conMain->add(winWorld, hge->System_GetState(HGE_SCREENWIDTH) / 2 - winWorld->getWidth() / 2,
                 hge->System_GetState(HGE_SCREENHEIGHT) / 2 - winWorld->getHeight() / 2);

    labwpName = new SHR::Lab(GETL(Lang_Name));
    labwpName->adjustSize();
    winWorld->add(labwpName, 5, 15);

    labwpAuthor = new SHR::Lab(GETL(Lang_Author));
    labwpAuthor->adjustSize();
    winWorld->add(labwpAuthor, 5, 40);

    labwpDate = new SHR::Lab(GETL(Lang_Date));
    labwpDate->adjustSize();
    winWorld->add(labwpDate, 5, 65);

    labwpREZ = new SHR::Lab(GETL(Lang_PathREZ));
    labwpREZ->adjustSize();
    winWorld->add(labwpREZ, 5, 100);

    labwpTiles = new SHR::Lab(GETL(Lang_PathTiles));
    labwpTiles->adjustSize();
    winWorld->add(labwpTiles, 5, 125);

    labwpPalette = new SHR::Lab(GETL(Lang_PathPAL));
    labwpPalette->adjustSize();
    winWorld->add(labwpPalette, 5, 150);

    labwpExe = new SHR::Lab(GETL(Lang_Exe));
    labwpExe->adjustSize();
    winWorld->add(labwpExe, 5, 175);

    tfwpName = new SHR::TextField("");
    tfwpName->setDimension(gcn::Rectangle(0, 0, 240, 20));
    tfwpName->setMaxLength(58);
    winWorld->add(tfwpName, 150, 15);

    tfwpAuthor = new SHR::TextField("");
    tfwpAuthor->setDimension(gcn::Rectangle(0, 0, 240, 20));
    winWorld->add(tfwpAuthor, 150, 40);

    tfwpDate = new SHR::TextField("");
    tfwpDate->setDimension(gcn::Rectangle(0, 0, 240, 20));
    tfwpDate->setEnabled(0);
    winWorld->add(tfwpDate, 150, 65);

    tfwpREZ = new SHR::TextField("");
    tfwpREZ->setDimension(gcn::Rectangle(0, 0, 240, 20));
    winWorld->add(tfwpREZ, 150, 100);

    tfwpTiles = new SHR::TextField("");
    tfwpTiles->setDimension(gcn::Rectangle(0, 0, 240, 20));
    winWorld->add(tfwpTiles, 150, 125);

    tfwpPalette = new SHR::TextField("");
    tfwpPalette->setDimension(gcn::Rectangle(0, 0, 240, 20));
    winWorld->add(tfwpPalette, 150, 150);

    tfwpExe = new SHR::TextField("");
    tfwpExe->setDimension(gcn::Rectangle(0, 0, 240, 20));
    winWorld->add(tfwpExe, 150, 175);

    cbwpCompress = new SHR::CBox(GV->hGfxInterface, GETL(Lang_Compress));
    cbwpCompress->adjustSize();
    cbwpCompress->addActionListener(mainListener);
    winWorld->add(cbwpCompress, 5, 210);

    cbwpZCoord = new SHR::CBox(GV->hGfxInterface, GETL(Lang_ZCoord));
    cbwpZCoord->adjustSize();
    cbwpZCoord->addActionListener(mainListener);
    winWorld->add(cbwpZCoord, 200, 210);

    /*labwpMapBuild = new SHR::Lab(GETL2S("Win_MapProperties", "MapBuild"));
    labwpMapBuild->adjustSize();
    winWorld->add(labwpMapBuild, 400, 10);

    tfwpMapBuild = new SHR::TextField("");
    tfwpMapBuild->setDimension(gcn::Rectangle(0, 0, 240, 20));
    tfwpMapBuild->setEnabled(0);
    winWorld->add(tfwpMapBuild, 400 + 150, 10);

    labwpMapVersion = new SHR::Lab(GETL2S("Win_MapProperties", "MapVersion"));
    labwpMapVersion->adjustSize();
    winWorld->add(labwpMapVersion, 400, 35);

    tfwpMapVersion = new SHR::TextField("");
    tfwpMapVersion->setDimension(gcn::Rectangle(0, 0, 240, 20));
    winWorld->add(tfwpMapVersion, 400 + 150, 35);

    labwpWapVersion = new SHR::Lab(GETL2S("Win_MapProperties", "Editor"));
    labwpWapVersion->adjustSize();
    winWorld->add(labwpWapVersion, 400, 60);

    tfwpWapVersion = new SHR::TextField("");
    tfwpWapVersion->setDimension(gcn::Rectangle(0, 0, 240, 20));
    tfwpWapVersion->setEnabled(0);
    winWorld->add(tfwpWapVersion, 400 + 150, 60);

    labwpMapDescription = new SHR::Lab(GETL2S("Win_MapProperties", "Description"));
    labwpMapDescription->adjustSize();
    winWorld->add(labwpMapDescription, 400, 95);

    tbwpMapDescription = new SHR::TextBox("");
    tbwpMapDescription->setFont(GV->gcnParts.gcnfntMyriad13);
    tbwpMapDescription->setDimension(gcn::Rectangle(0, 0, 240, 5));
    tbwpMapDescription->setMinimalSize(240, 95);

    sawpMapDescription = new SHR::ScrollArea();
    sawpMapDescription->setContent(tbwpMapDescription);
    sawpMapDescription->setBackgroundColor(0x3c3c3c);
    sawpMapDescription->setDimension(gcn::Rectangle(0, 0, 240, 100));
    winWorld->add(sawpMapDescription, 400 + 150, 95);*/

    butwpSave = new SHR::But(GV->hGfxInterface, GETL(Lang_OK));
    butwpSave->setDimension(gcn::Rectangle(0, 0, 100, 33));
    butwpSave->addActionListener(mainListener);

    butwpCancel = new SHR::But(GV->hGfxInterface, GETL(Lang_Cancel));
    butwpCancel->setDimension(gcn::Rectangle(0, 0, 100, 33));
    butwpCancel->addActionListener(mainListener);

    winWorld->add(butwpSave, 180, 235);
    winWorld->add(butwpCancel, 290, 235);

    vpWorld = new WIDG::Viewport(vp, VP_WORLD);
    winWorld->add(vpWorld, 0, 0);

    winDuplicate = new SHR::Win(&GV->gcnParts, GETL(Lang_Duplicating));
    winDuplicate->setDimension(gcn::Rectangle(0, 0, 210, 170));
    winDuplicate->setVisible(0);
    winDuplicate->setClose(1);
    winDuplicate->addActionListener(mainListener);
    conMain->add(winDuplicate, hge->System_GetState(HGE_SCREENWIDTH) / 2 - 105,
                 hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 100);

    offsetY = 16;

    labdTimes = new SHR::Lab(std::string(GETL(Lang_CopyXTimes)) + ':');
    labdTimes->adjustSize();
    winDuplicate->add(labdTimes, 5, offsetY + 1);

    tfdTimes = new SHR::TextField("1");
    tfdTimes->setDimension(gcn::Rectangle(0, 0, 75, 20));
    tfdTimes->SetNumerical(1, 0);
    tfdTimes->addActionListener(mainListener);
    winDuplicate->add(tfdTimes, 125, offsetY);

    offsetY += 25;

    labdOffsetX = new SHR::Lab(std::string(GETL(Lang_OffsetX)) + ':');
    labdOffsetX->adjustSize();
    labdOffsetX->addActionListener(mainListener);
    winDuplicate->add(labdOffsetX, 5, offsetY + 1);

    tfdOffsetX = new SHR::TextField("0");
    tfdOffsetX->setDimension(gcn::Rectangle(0, 0, 75, 20));
    tfdOffsetX->SetNumerical(1, 1);
    tfdOffsetX->addActionListener(mainListener);
    winDuplicate->add(tfdOffsetX, 125, offsetY);

    offsetY += 25;

    labdOffsetY = new SHR::Lab(std::string(GETL(Lang_OffsetY)) + ':');
    labdOffsetY->adjustSize();
    winDuplicate->add(labdOffsetY, 5, offsetY + 1);

    tfdOffsetY = new SHR::TextField("0");
    tfdOffsetY->setDimension(gcn::Rectangle(0, 0, 75, 20));
    tfdOffsetY->SetNumerical(1, 1);
    winDuplicate->add(tfdOffsetY, 125, offsetY);

    offsetY += 25;

    labdChangeZ = new SHR::Lab(std::string(GETL(Lang_OffsetZ)) + ':');
    labdChangeZ->adjustSize();
    winDuplicate->add(labdChangeZ, 5, offsetY + 1);

    tfdChangeZ = new SHR::TextField("0");
    tfdChangeZ->setDimension(gcn::Rectangle(0, 0, 75, 20));
    tfdChangeZ->SetNumerical(1, 1);
    winDuplicate->add(tfdChangeZ, 125, offsetY);

    offsetY += 30;

    butdOK = new SHR::But(GV->hGfxInterface, GETL(Lang_OK));
    butdOK->setDimension(gcn::Rectangle(0, 0, 75, 25));
    butdOK->addActionListener(mainListener);
    winDuplicate->add(butdOK, 68, offsetY);

    gui->setTop(conMain);

    dwBG = GV->colBaseDark;
    fCamX = fCamY = 0;
    cScrollOrientation = 2;
    bDragDropScroll = false;

    GV->Console->AddModifiableFloat("camx", &fCamX);
    GV->Console->AddModifiableFloat("camy", &fCamY);
    GV->Console->AddModifiableFloat("zoom", &fZoom);
    GV->Console->AddModifiableFloat("dzoom", &fDestZoom);
    GV->Console->AddModifiableBool("smoothz", &GV->bSmoothZoom);
    GV->Console->AddModifiableBool("redraw", &bConstRedraw);
    GV->Console->AddModifiableBool("drawprop", &bDrawTileProperties);

    conRecentFiles = new SHR::Container();
    conRecentFiles->setOpaque(false);
    labLoadLastOpened = new SHR::Lab(GETL2S("HomeScreen", "RecentDocs"));
    labLoadLastOpened->adjustSize();
    conRecentFiles->setDimension(gcn::Rectangle(0, 0, 200, 20));
    conRecentFiles->add(labLoadLastOpened, 0, 0);

    butCrashRetrieve = NULL;
    conCrashRetrieve = NULL;

    if (GV->szCmdLine[0] != '"') {
        FILE *f = fopen("runtime.tmp", "r");
        if (f) {
            GV->Console->Printf("~w~Crash registered while used last time. Recovering tabs.");
            fclose(f);
            std::ifstream ifs("runtime.tmp");
            std::string temp;
            int linesCount = 0;
            for (auto & line : szCrashRetrieve) {
                line = NULL;
            }
            while (getline(ifs, temp)) {
                if (temp.length()) {
                    if (linesCount < 10) {
                        int base = 0;
                        try {
                            iCrashRetrieveIcon[linesCount] = WWD::GetGameTypeFromFile(temp.c_str(), &base);
                        }
                        catch (...) {
                            iCrashRetrieveIcon[linesCount] = WWD::Game_Unknown;
                        }
                        if (base > 0 && base < 15 && iCrashRetrieveIcon[linesCount] == WWD::Game_Claw) {
                            iCrashRetrieveIcon[linesCount] = 50 + base;
                        } else if (base > 0 && base < 9 && iCrashRetrieveIcon[linesCount] == WWD::Game_Gruntz) {
                            iCrashRetrieveIcon[linesCount] = 150 + base;
                        }
                        if (GV->fntMyriad16->GetStringWidth(temp.c_str()) > 460) {
                            do {
                                temp.erase(0, 5);
                            } while (GV->fntMyriad16->GetStringWidth(temp.c_str()) > 450);
                            temp.insert(0, "(...)");
                        }
                        szCrashRetrieve[linesCount] = new char[strlen(temp.c_str()) + 1];
                        strcpy(szCrashRetrieve[linesCount], temp.c_str());
                    }
                    linesCount++;
                }
            }
            if (linesCount > 0) {
                conCrashRetrieve = new SHR::Container();
                conCrashRetrieve->setOpaque(false);
                conCrashRetrieve->setDimension(gcn::Rectangle(0, 0, 460, 25 + (linesCount > 10 ? 11 : linesCount) * 25 + 40));
                vpCrashRetrieve = new WIDG::Viewport(vp, VP_CRASHRETRIEVE);
                conCrashRetrieve->add(vpCrashRetrieve, 0, 0);
                butCrashRetrieve = new SHR::But(GV->hGfxInterface, GETL2S("WinCrashRetrieve", "Retrieve"));
                butCrashRetrieve->setDimension(gcn::Rectangle(0, 0, 100, 33));
                butCrashRetrieve->addActionListener(mainListener);
                conCrashRetrieve->add(butCrashRetrieve, 460 / 2 - 50, conCrashRetrieve->getHeight() - 20);
                if (linesCount > 10) {
                    char tmp[512];
                    sprintf(tmp, GETL2S("WinCrashRetrieve", "MoreTabs"), linesCount - 10);
                    szCrashRetrieveMore = new char[strlen(tmp) + 1];
                    strcpy(szCrashRetrieveMore, tmp);
                } else {
                    szCrashRetrieveMore = NULL;
                }
            }
        }
    }

    winWelcome = new SHR::Container();
    winWelcome->setOpaque(false);
    conMain->add(winWelcome, hge->System_GetState(HGE_SCREENWIDTH) / 2 - 300, 0);

    vpws = new WIDG::Viewport(vp, VP_WELCOMESCREEN);
    winWelcome->add(vpws, 0, 0);

    static hgeSprite* icons[] = {
            GV->sprIcons16[Icon16_New],
            GV->sprIcons16[Icon16_Open],
            GV->sprIcons16[Icon16_Readme],
    };

    static const char* options[] = {
            GETL2S("HomeScreen", "New"),
            GETL2S("HomeScreen", "Open"),
            GETL2S("HomeScreen", "Changelog"),
    };

    for (int i = 0; i < WelcomeScreenOptions_Count; ++i) {
        welcomeScreenOptions[i] = new SHR::Link(options[i], icons[i]);
        welcomeScreenOptions[i]->adjustSize();
        welcomeScreenOptions[i]->addActionListener(mainListener);
        winWelcome->add(welcomeScreenOptions[i], 0, 95 + i * 25);
    }

    winWelcome->setVisible(GV->szCmdLine.empty());

    for (auto & link : lnkLastOpened) {
        link = NULL;
    }
    hMruList = new cMruList();
    MruListUpdated();
    hAppMenu->SyncMRU();

    if ((hMruList->IsValid() && hMruList->GetFilesCount() > 0) || conCrashRetrieve) {
        SHR::Container *conToAdd = (conCrashRetrieve ? conCrashRetrieve : conRecentFiles);

        winWelcome->add(conToAdd, 0, winWelcome->getHeight() + 50);
        winWelcome->setY(vPort->GetY() + vPort->GetHeight() / 2 - winWelcome->getHeight() / 2);
    }

    if (hMruList->IsValid() && hMruList->GetFilesCount() > 0) {
        char *n = SHR::GetFile(hMruList->GetRecentlyUsedFile(0));
        bool bDots = false;
        while (GV->fntMyriad16->GetStringWidth(n) > 140) {
            if (strlen(n) < 5) break;
            if (!bDots) {
                n[strlen(n) - 4] = '.';
                n[strlen(n) - 3] = '.';
                n[strlen(n) - 2] = '.';
                n[strlen(n) - 1] = '\0';
            } else {
                n[strlen(n) - 4] = '.';
                n[strlen(n) - 1] = '\0';
            }
            bDots = true;
        }
    }

    winWelcome->setY(vPort->GetY() + vPort->GetHeight() / 2 - winWelcome->getHeight() / 2);

    btpZoomTile = false;

    GV->Console->Printf("~w~Initing interface...");

    if (hParser != NULL) {
        SynchronizeWithParser();
    } else {
        iMode = EWW_MODE_NONE;
        InitEmpty();
    }

    GV->Console->Printf("~w~Creating context menu models...");

    conmodEditableObject = new SHR::ContextModel();
    conmodEditableObject->AddElement(OBJMENU_MOVE, GETL(Lang_Move), GV->sprIcons16[Icon16_Move]);
    conmodEditableObject->AddElement(OBJMENU_EDIT, GETL2S("Various", "ContextEdit"), GV->sprIcons16[Icon16_Edit]);
    conmodEditableObject->AddElement(OBJMENU_CUT, GETL(Lang_Cut), GV->sprIcons16[Icon16_Cut]);
    conmodEditableObject->AddElement(OBJMENU_COPY, GETL(Lang_Copy), GV->sprIcons16[Icon16_Copy]);
    conmodEditableObject->AddElement(OBJMENU_DELETE, GETL(Lang_Delete), GV->sprIcons16[Icon16_X]);
    //conmodObject->AddElement(OBJMENU_COPYTOCB, GETL(Lang_CopyToClipboard), GV->sprIcons16[Icon16_Export]);
    conmodEditableObject->AddElement(OBJMENU_DUPLICATE, GETL(Lang_Duplicate), GV->sprIcons16[Icon16_Star]);
    conmodEditableObject->AddElement(OBJMENU_USEASBRUSH, GETL(Lang_UseAsBrush), GV->sprIcons16[Icon16_Brush]);
    conmodEditableObject->AddElement(OBJMENU_PROPERTIES, GETL(Lang_Properties), GV->sprIcons16[Icon16_Properties]);
    conmodEditableObject->AddElement(OBJMENU_FLAGS, GETL2S("Various", "ContextFlags"), GV->sprIcons16[Icon16_Flags]);
    conmodEditableObject->AddElement(OBJMENU_ZCOORD, GETL2S("Various", "ContextZCoord"), GV->sprIcons16[Icon16_ZCoord]);

    conmodObject = new SHR::ContextModel();
    conmodObject->AddElement(OBJMENU_MOVE, GETL(Lang_Move), GV->sprIcons16[Icon16_Move]);
    conmodObject->AddElement(OBJMENU_CUT, GETL(Lang_Cut), GV->sprIcons16[Icon16_Cut]);
    conmodObject->AddElement(OBJMENU_COPY, GETL(Lang_Copy), GV->sprIcons16[Icon16_Copy]);
    conmodObject->AddElement(OBJMENU_DELETE, GETL(Lang_Delete), GV->sprIcons16[Icon16_X]);
    //conmodObject->AddElement(OBJMENU_COPYTOCB, GETL(Lang_CopyToClipboard), GV->sprIcons16[Icon16_Export]);
    conmodObject->AddElement(OBJMENU_DUPLICATE, GETL(Lang_Duplicate), GV->sprIcons16[Icon16_Star]);
    conmodObject->AddElement(OBJMENU_USEASBRUSH, GETL(Lang_UseAsBrush), GV->sprIcons16[Icon16_Brush]);
    conmodObject->AddElement(OBJMENU_PROPERTIES, GETL(Lang_Properties), GV->sprIcons16[Icon16_Properties]);
    conmodObject->AddElement(conmodEditableObject->GetElementByID(OBJMENU_FLAGS));
    conmodObject->AddElement(conmodEditableObject->GetElementByID(OBJMENU_ZCOORD));

    objmAlignContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    objmAlignContext->AddElement(OBJMENU_ALIGN_HOR, GETL2S("Various", "ContextAlign_Hor"),
                                 GV->sprIcons16[Icon16_FlipX]);
    objmAlignContext->AddElement(OBJMENU_ALIGN_VERT, GETL2S("Various", "ContextAlign_Vert"),
                                 GV->sprIcons16[Icon16_FlipY]);
    objmAlignContext->adjustSize();
    objmAlignContext->hide();
    objmAlignContext->addActionListener(mainListener);
    conMain->add(objmAlignContext);

    objmFlipContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    objmFlipContext->AddElement(OBJMENU_FLIP_X, GETL2S("Various", "ContextFlipX"),
                                 GV->sprIcons16[Icon16_Mirror]);
    objmFlipContext->AddElement(OBJMENU_FLIP_Y, GETL2S("Various", "ContextFlipY"),
                                 GV->sprIcons16[Icon16_Invert]);
    objmFlipContext->adjustSize();
    objmFlipContext->hide();
    objmFlipContext->addActionListener(mainListener);
    conMain->add(objmFlipContext);

    objmSpaceContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    objmSpaceContext->AddElement(OBJMENU_SPACE_HOR, GETL2S("Various", "ContextSpaceHor"), GV->sprIcons16[Icon16_FlipX]);
    objmSpaceContext->AddElement(OBJMENU_SPACE_VERT, GETL2S("Various", "ContextSpaceVer"),
                                 GV->sprIcons16[Icon16_FlipY]);
    objmSpaceContext->adjustSize();
    objmSpaceContext->hide();
    objmSpaceContext->addActionListener(mainListener);
    conMain->add(objmSpaceContext);

    conmodObjectMultiple = new SHR::ContextModel();
    conmodObjectMultiple->AddElement(OBJMENU_MOVE, GETL(Lang_Move), GV->sprIcons16[Icon16_Move]);
    conmodObjectMultiple->AddElement(OBJMENU_CUT, GETL(Lang_Cut), GV->sprIcons16[Icon16_Cut]);
    conmodObjectMultiple->AddElement(OBJMENU_COPY, GETL(Lang_Copy), GV->sprIcons16[Icon16_Copy]);
    conmodObjectMultiple->AddElement(OBJMENU_DELETE, GETL(Lang_Delete), GV->sprIcons16[Icon16_X]);
    //conmodObjectMultiple->AddElement(OBJMENU_COPYTOCB, GETL(Lang_CopyToClipboard), GV->sprIcons16[Icon16_Export]);
    conmodObjectMultiple->AddElement(OBJMENU_DUPLICATE, GETL(Lang_Duplicate), GV->sprIcons16[Icon16_Star]);
    conmodObjectMultiple->AddElement(OBJMENU_USEASBRUSH, GETL(Lang_UseAsBrush), GV->sprIcons16[Icon16_Brush]);
    conmodObjectMultiple->AddElement(conmodEditableObject->GetElementByID(OBJMENU_FLAGS));
    conmodObjectMultiple->AddElement(conmodEditableObject->GetElementByID(OBJMENU_ZCOORD));
    conmodObjectMultiple->AddElement(OBJMENU_ALIGN, GETL2S("Various", "ContextAlign"), GV->sprIcons16[Icon16_Align]);
    conmodObjectMultiple->GetElementByID(OBJMENU_ALIGN)->SetCascade(objmAlignContext);
    conmodObjectMultiple->AddElement(OBJMENU_FLIP, GETL2S("Various", "ContextFlip"), GV->sprIcons16[Icon16_Flip]);
    conmodObjectMultiple->GetElementByID(OBJMENU_FLIP)->SetCascade(objmFlipContext);
    conmodObjectMultiple->AddElement(OBJMENU_SPACE, GETL2S("Various", "ContextSpace"), GV->sprIcons16[Icon16_FlipX]);
    conmodObjectMultiple->GetElementByID(OBJMENU_SPACE)->SetCascade(objmSpaceContext);

    conmodSpawnPoint = new SHR::ContextModel();
    conmodSpawnPoint->AddElement(OBJMENU_MOVE, GETL(Lang_Move), GV->sprIcons16[Icon16_Move]);
    conmodSpawnPoint->AddElement(OBJMENU_TESTFROMHERE, GETL2S("Various", "ContextTestFromHere"),
                            GV->sprIcons16[Icon16_Play]);

    //conmodUseAsBrush = new SHR::ContextModel();
    //conmodUseAsBrush->AddElement(OBJMENU_USEASBRUSH, GETL(Lang_UseAsBrush), GV->sprIcons16[Icon16_Brush]);

    conmodAtEmptyPaste = new SHR::ContextModel();
    conmodAtEmptyPaste->AddElement(OBJMENU_PASTE, GETL(Lang_Paste), GV->sprIcons16[Icon16_Paste]);
    conmodAtEmptyPaste->AddElement(OBJMENU_NEWOBJ, GETL(Lang_NewObjectCtx), GV->sprIcons16[Icon16_Star]);
    conmodAtEmptyPaste->AddElement(OBJMENU_SETSPAWNP, GETL(Lang_SetSpawnPoint), GV->sprIcons16[Icon16_Move]);
    conmodAtEmptyPaste->AddElement(OBJMENU_TESTFROMHERE, GETL2S("Various", "ContextTestFromHere"),
                                   GV->sprIcons16[Icon16_Play]);

    conmodAtEmpty = new SHR::ContextModel();
    conmodAtEmpty->AddElement(OBJMENU_NEWOBJ, GETL(Lang_NewObjectCtx), GV->sprIcons16[Icon16_Star]);
    conmodAtEmpty->AddElement(OBJMENU_SETSPAWNP, GETL(Lang_SetSpawnPoint), GV->sprIcons16[Icon16_Move]);
    conmodAtEmpty->AddElement(OBJMENU_TESTFROMHERE, GETL2S("Various", "ContextTestFromHere"),
                              GV->sprIcons16[Icon16_Play]);

    conmodAtEmptyPaste->GetElementByID(OBJMENU_NEWOBJ)->SetCascade(hmbObject->GetContext());
    conmodAtEmpty->GetElementByID(OBJMENU_NEWOBJ)->SetCascade(hmbObject->GetContext());

    conMain->add(hmbObject->GetContext(), 400, 400);

    objZCoordContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    objZCoordContext->AddElement(OBJMENU_ZC_INC3, GETL2S("Various", "ContextZ_Increase3"),
                                 GV->sprIcons16[Icon16_AddTriple]);
    objZCoordContext->AddElement(OBJMENU_ZC_INC2, GETL2S("Various", "ContextZ_Increase2"),
                                 GV->sprIcons16[Icon16_AddDouble]);
    objZCoordContext->AddElement(OBJMENU_ZC_INC, GETL2S("Various", "ContextZ_Increase"), GV->sprIcons16[Icon16_Add]);
    objZCoordContext->AddElement(OBJMENU_ZC_DEC, GETL2S("Various", "ContextZ_Decrease"), GV->sprIcons16[Icon16_Sub]);
    objZCoordContext->AddElement(OBJMENU_ZC_DEC2, GETL2S("Various", "ContextZ_Decrease2"),
                                 GV->sprIcons16[Icon16_SubDouble]);
    objZCoordContext->AddElement(OBJMENU_ZC_DEC3, GETL2S("Various", "ContextZ_Decrease3"),
                                 GV->sprIcons16[Icon16_SubTriple]);
    objZCoordContext->AddElement(OBJMENU_ZC_BACK, GETL2S("Various", "ContextZ_Back"), GV->sprIcons16[Icon16_PlaneBack]);
    objZCoordContext->AddElement(OBJMENU_ZC_ACTION, GETL2S("Various", "ContextZ_Action"),
                                 GV->sprIcons16[Icon16_PlaneAction]);
    objZCoordContext->AddElement(OBJMENU_ZC_FRONT, GETL2S("Various", "ContextZ_Front"),
                                 GV->sprIcons16[Icon16_PlaneFront]);
    conmodObject->GetElementByID(OBJMENU_ZCOORD)->SetCascade(objZCoordContext);
    objZCoordContext->adjustSize();
    objZCoordContext->hide();
    objZCoordContext->addActionListener(mainListener);
    conMain->add(objZCoordContext, 400, 400);

    objFlagContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    objFlagContext->AddElement(OBJMENU_FLAGS_DRAW, GETL(Lang_DrawingFlags), 0);
    //objFlagContext->AddElement(OBJMENU_FLAGS_ADDITIONAL, GETL(Lang_AddFlags), 0);
    objFlagContext->AddElement(OBJMENU_FLAGS_DYNAMIC, GETL(Lang_DynamicFlags), 0);
    objFlagContext->adjustSize();
    objFlagContext->hide();
    conMain->add(objFlagContext, 400, 400);
    conmodObject->GetElementByID(OBJMENU_FLAGS)->SetCascade(objFlagContext);

    objFlagDrawContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    objFlagDrawContext->AddElement(OBJMENU_FLAGS_DRAW_NODRAW, GETL(Lang_NoDraw), 0);
    objFlagDrawContext->AddElement(OBJMENU_FLAGS_DRAW_FLIPX, GETL(Lang_Mirror), 0);
    objFlagDrawContext->AddElement(OBJMENU_FLAGS_DRAW_FLIPY, GETL(Lang_Invert), 0);
    objFlagDrawContext->AddElement(OBJMENU_FLAGS_DRAW_FLASH, GETL(Lang_Flash), 0);
    objFlagDrawContext->ReserveIconSpace(0, 1);
    objFlagDrawContext->adjustSize();
    objFlagDrawContext->addActionListener(mainListener);
    objFlagDrawContext->hide();
    conMain->add(objFlagDrawContext, 400, 400);
    objFlagContext->GetElementByID(OBJMENU_FLAGS_DRAW)->SetCascade(objFlagDrawContext);

    /*objFlagAddContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    objFlagAddContext->AddElement(OBJMENU_FLAGS_ADDITIONAL_DIFFICULT, GETL(Lang_ObjFlag_Difficult), 0);
    objFlagAddContext->AddElement(OBJMENU_FLAGS_ADDITIONAL_EXTRAMEMORY, GETL(Lang_ObjFlag_ExtraMemory), 0);
    objFlagAddContext->AddElement(OBJMENU_FLAGS_ADDITIONAL_EYECANDY, GETL(Lang_ObjFlag_EyeCandy), 0);
    objFlagAddContext->AddElement(OBJMENU_FLAGS_ADDITIONAL_FASTCPU, GETL(Lang_ObjFlag_FastCPU), 0);
    objFlagAddContext->AddElement(OBJMENU_FLAGS_ADDITIONAL_HIGHDETAIL, GETL(Lang_ObjFlag_HighDetail), 0);
    objFlagAddContext->AddElement(OBJMENU_FLAGS_ADDITIONAL_MULTIPLAYER, GETL(Lang_ObjFlag_Multiplayer), 0);
    objFlagAddContext->ReserveIconSpace(0, 1);
    objFlagAddContext->adjustSize();
    objFlagAddContext->addActionListener(mainListener);
    objFlagAddContext->hide();
    conMain->add(objFlagAddContext, 400, 400);
    objFlagContext->GetElementByID(OBJMENU_FLAGS_ADDITIONAL)->SetCascade(objFlagAddContext);*/

    objFlagDynamicContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    objFlagDynamicContext->AddElement(OBJMENU_FLAGS_DYNAMIC_ALWAYSACTIVE, GETL(Lang_ObjFlag_AlwaysActive), 0);
    objFlagDynamicContext->AddElement(OBJMENU_FLAGS_DYNAMIC_AUTOHITDAMAGE, GETL(Lang_ObjFlag_AutoHitDmg), 0);
    objFlagDynamicContext->AddElement(OBJMENU_FLAGS_DYNAMIC_NOHIT, GETL(Lang_ObjFlag_NoHit), 0);
    objFlagDynamicContext->AddElement(OBJMENU_FLAGS_DYNAMIC_SAFE, GETL(Lang_ObjFlag_Safe), 0);
    objFlagDynamicContext->ReserveIconSpace(0, 1);
    objFlagDynamicContext->adjustSize();
    objFlagDynamicContext->addActionListener(mainListener);
    objFlagDynamicContext->hide();
    conMain->add(objFlagDynamicContext, 400, 400);
    objFlagContext->GetElementByID(OBJMENU_FLAGS_DYNAMIC)->SetCascade(objFlagDynamicContext);

    objContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    objContext->SetModel(conmodObject);
    objContext->adjustSize();
    objContext->addActionListener(mainListener);
    objContext->hide();
    conMain->add(objContext, 400, 400);

    advcon_Warp = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    advcon_Warp->AddElement(OBJMENU_ADV_WARP_GOTO, GETL2S("EditObj_Warp", "Context_GoTo"), GV->sprIcons16[Icon16_Warp]);
    advcon_Warp->adjustSize();
    advcon_Warp->hide();
    advcon_Warp->addActionListener(mainListener);
    conMain->add(advcon_Warp, 400, 400);

    advcon_Container = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    advcon_Container->AddElement(OBJMENU_ADV_CONTAINER_RAND, GETL2S("EditObj_Statue", "Context_Randomize"),
                                 GV->sprIcons16[Icon16_Treasure]);
    advcon_Container->adjustSize();
    advcon_Container->hide();
    advcon_Container->addActionListener(mainListener);
    conMain->add(advcon_Container, 400, 400);

    conmodTilesSelected = new SHR::ContextModel();
    conmodTilesSelected->AddElement(TILMENU_CUT, GETL(Lang_Cut), GV->sprIcons16[Icon16_Cut]);
    conmodTilesSelected->AddElement(TILMENU_COPY, GETL(Lang_Copy), GV->sprIcons16[Icon16_Copy]);
    conmodTilesSelected->AddElement(TILMENU_DELETE, GETL(Lang_Delete), GV->sprIcons16[Icon16_X]);

    conmodTilesPaste = new SHR::ContextModel();
    conmodTilesPaste->AddElement(TILMENU_PASTE, GETL(Lang_Paste), GV->sprIcons16[Icon16_Paste]);

    tilContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    tilContext->SetModel(conmodTilesSelected);
    tilContext->adjustSize();
    tilContext->addActionListener(mainListener);
    tilContext->hide();
    conMain->add(tilContext, 400, 400);

    if (GV->bFirstRun)
        FirstRun_Open();

    if (!GV->szCmdLine.compare(0, 14, "-updateCleanup")) {
        GV->Console->Printf("Cleaning up after update.");
        unlink("updatetmp/Updater.exe");
        rmdir("updatetmp");
    }

    GV->Console->Print("~w~Creating inventory controller...");
    hInvCtrl = new cInventoryController();

    vWidgetsToMinimalise.push_back(winCamera);
    vWidgetsToMinimalise.push_back(winWorld);
    vWidgetsToMinimalise.push_back(winMapShot);
    vWidgetsToMinimalise.push_back(winSearchObj);
    vWidgetsToMinimalise.push_back(winpmMain);
    vWidgetsToMinimalise.push_back(winDB);
    vWidgetsToMinimalise.push_back(winTileProp);

    GV->Console->Print("~w~Creating IPC server...");
    hServerIPC = new cServerIPC();

    bMaximized = !bMaximized;
    ToggleFullscreen();
    //FixInterfacePositions();

    GV->Console->Print("~g~Everything OK.");
}

void State::EditingWW::InitEmpty() {
    tfwpName->setText("");
    tfwpAuthor->setText("");
    tfwpDate->setText("");
    tfwpREZ->setText("");
    tfwpTiles->setText("");
    tfwpPalette->setText("");
    tfwpExe->setText("");

    sliVer->setVisible(0);
    sliHor->setVisible(0);
    sliZoom->setVisible(0);
    butMicroTileCB->setVisible(0);
    butMicroObjectCB->setVisible(0);

    cbutActiveMode->setVisible(0);
#ifdef WM_ADD_LUA_EXECUTER
    butIconLua->setEnabled(0);
#endif

    hmbActive = NULL;
    ExitMode(iMode);
    iMode = EWW_MODE_NONE;

    hge->System_SetState(HGE_TITLE, WA_TITLEBAR);
}

void State::EditingWW::SynchronizeWithParser() {
    for (int i = 0; i < hParser->GetPlanesCount(); i++) {
        if ((hParser->GetPlane(i)->GetFlags() & WWD::Flag_p_MainPlane)) {
            hmbTile->ddActivePlane->setSelected(i);
        }
    }

    cbutActiveMode->setVisible(1);
#ifdef WM_ADD_LUA_EXECUTER
    butIconLua->setEnabled(1);
#endif

    if (hmbActive == NULL)
        SetMode(cbutActiveMode->getSelectedEntryID() ? EWW_MODE_OBJECT : EWW_MODE_TILE);
    hmbActive->SetEnabled(1);

    fCamX = hParser->GetStartX() - (vPort->GetWidth() / 2 / fZoom);
    fCamY = hParser->GetStartY() - (vPort->GetHeight() / 2 / fZoom);
}

void State::EditingWW::SwitchActiveModeMenuBar(cModeMenuBar *n) {
    if (hmbActive != 0)
        hmbActive->SetVisible(0);
    hmbActive = n;
    if (hmbActive != 0)
        hmbActive->SetVisible(1);
}

void State::EditingWW::Destroy() {
    delete MDI;
    delete hServerIPC;
    if (hParser != NULL) FreeResources();
    GV->Console->DeleteModifiableVar("camx");
    GV->Console->DeleteModifiableVar("camy");
    GV->Console->DeleteModifiableVar("zoom");
    delete mainListener;
    delete vp;
    delete vpMain;
    delete sliVer;
    delete sliHor;
    delete conMain;
    delete gui;
    delete vPort;
}

bool State::EditingWW::Think() {
    GV->Console->Think();
    switch (fade_iAction) {
        case 0:
            fade_fAlpha -= hge->Timer_GetDelta() * 512;
            if (fade_fAlpha < 0) {
                fade_iAction++;
                fade_fAlpha = 0;
            }
            return false;
        case 1:
            fade_fTimer += hge->Timer_GetDelta();
            if (fade_fTimer >= 0.5) {
                fade_iAction++;
                fade_fAlpha = 0;
            }
            return false;
        case 2:
            fade_fAlpha += hge->Timer_GetDelta() * 256;
            if (fade_fAlpha > 255) {
                fade_iAction++;
                GV->sprLogoBig->SetColor(0xFFFFFFFF);
                GV->fntMyriad10->SetColor(0xFFFFFFFF);
                GV->bWinter = false;
                GV->sprSnowflake->SetColor(0xFFFFFFFF);

                ApplicationStartup();
            } else {
                return false;
            }
    }

    if (hServerIPC->Think())
        return false;

    if (bExit) return true;

    if (hNativeController->ExecutableChanged()) {
        hwinOptions->SyncWithExe();
    }

    if (!vstrMapsToLoad.empty()) {
        if (!MDI->IsDocAlreadyOpen(vstrMapsToLoad[0])) {
            GV->StateMgr->Push(new State::LoadMap(vstrMapsToLoad[0].c_str()));
        }
        vstrMapsToLoad.erase(vstrMapsToLoad.begin());
        return false;
    }

    /*if (NewMap_data != 0)
        if (NewMap_data->bKill)
            NewMap_Close();*/

    if (FirstRun_data != 0)
        if (FirstRun_data->bKill)
            FirstRun_Close();

    /*if (hParser == NULL || iMode != EWW_MODE_TILE) {
        conResizeDown->setVisible(false);
        conResizeUp->setVisible(false);
        conResizeLeft->setVisible(false);
        conResizeRight->setVisible(false);
    } else {
        if (fCamX < 0 && !GetActivePlane()->GetFlag(WWD::Flag_p_XWrapping)) {
            conResizeLeft->setDimension(
                    gcn::Rectangle(vPort->GetX(), vPort->GetY() + 40, -fCamX, vPort->GetHeight() - 80));
            conResizeLeft->setVisible(true);
            butExtLayerDL->setVisible(true);
            butExtLayerUL->setVisible(true);
            int offset = 40 - abs(fCamX);
            butExtLayerLeft->setX(4 - offset);
            butExtLayerDL->setX(4 - offset);
            butExtLayerUL->setX(4 - offset);
        } else {
            conResizeLeft->setVisible(false);
            butExtLayerDL->setVisible(false);
            butExtLayerUL->setVisible(false);
        }

        if (fCamY < 0 && !(GetActivePlane()->GetFlags() & WWD::Flag_p_YWrapping)) {
            conResizeUp->setDimension(gcn::Rectangle(vPort->GetX(), vPort->GetY(), vPort->GetWidth(), -fCamY));
            conResizeUp->setVisible(true);
            int offset = 40 - abs(fCamY);
            butExtLayerUp->setY(4 - offset);
            butExtLayerUL->setY(4 - offset);
            butExtLayerUR->setY(4 - offset);
        } else
            conResizeUp->setVisible(false);

        int maxx = std::max(GetActivePlane()->GetPlaneWidthPx() - vPort->GetWidth() / fZoom, -40.0f);
        if (fCamX > maxx && !(GetActivePlane()->GetFlags() & WWD::Flag_p_XWrapping)) {
            conResizeRight->setDimension(
                    gcn::Rectangle(vPort->GetX() + vPort->GetWidth() - (fCamX - maxx), vPort->GetY() + 40, fCamX - maxx,
                                   vPort->GetHeight() - 80));
            conResizeRight->setVisible(true);
            butExtLayerDR->setVisible(true);
            butExtLayerUR->setVisible(true);
            butExtLayerDR->setX(vPort->GetWidth() - 36 + (40 - (fCamX - maxx)));
            butExtLayerUR->setX(vPort->GetWidth() - 36 + (40 - (fCamX - maxx)));
        } else {
            conResizeRight->setVisible(false);
            butExtLayerDR->setVisible(false);
            butExtLayerUR->setVisible(false);
        }

        int maxy = std::max(GetActivePlane()->GetPlaneHeightPx() - vPort->GetHeight() / fZoom, -40.0f);
        if (fCamY > maxy && !(GetActivePlane()->GetFlags() & WWD::Flag_p_YWrapping)) {
            conResizeDown->setDimension(
                    gcn::Rectangle(vPort->GetX(), vPort->GetY() + vPort->GetHeight() - (fCamY - maxy),
                                   vPort->GetWidth(), fCamY - maxy));
            conResizeDown->setVisible(true);
        } else {
            conResizeDown->setVisible(false);
        }
    }*/

    try {
        gui->logic();
    }
    catch (gcn::Exception &exc) {
        GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(),
                            exc.getLine());
    }
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);

    if (bDragWindow) {
        POINT mouse;
        GetCursorPos(&mouse);

        if (!hge->Input_GetKeyState(HGEK_LBUTTON)) {
            bDragWindow = false;
            if (!bMaximized && mouse.y <= 1) {
                ToggleFullscreen();
            }
        } else {
            int dmx, dmy;
            dmx = mouse.x - iWindowDragX;
            dmy = mouse.y - iWindowDragY;
            GV->SetCursor(DRAG);

            if (bMaximized) {
                if (dmx || dmy) {
                    float oldW = hge->System_GetState(HGE_SCREENWIDTH);
                    ToggleFullscreen();
                    iWindowDragX *= GV->iScreenW / oldW;
                    windowDragStartRect.left *= GV->iScreenW / oldW;
                }
            } else {
                SetWindowPos(hge->System_GetState(HGE_HWND), NULL, windowDragStartRect.left + dmx,
                             windowDragStartRect.top + dmy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            }
        }
    }

    hRulers->Think();
    MDI->Think(conMain->getWidgetAt(mx, my) != NULL);

    /*if( iMode != EWW_MODE_SIMULATION && hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_I) )
	 hAppMenu->SetFolded(!hAppMenu->IsFolded());*/

    if (hParser == NULL) return false;

    bool bMouseConsumed = false;
    if (conMain->getWidgetAt(mx, my) != NULL && conMain->getWidgetAt(mx, my) != vPort->GetWidget()) {
        bMouseConsumed = true;
    }

    if (!GV->editState->MDI->GetActiveDoc()->vGuides.empty() && bShowGuideLines) {
        WWD::Plane *mainPl;
        for (int i = 0; i < hParser->GetPlanesCount(); i++)
            if (hParser->GetPlane(i)->GetFlag(WWD::Flag_p_MainPlane)) {
                mainPl = hParser->GetPlane(i);
                break;
            }

        if (conMain->getWidgetAt(mx, my) == vPort->GetWidget()) {
            for (size_t i = 0; i < GV->editState->MDI->GetActiveDoc()->vGuides.size(); i++) {
                stGuideLine gl = GV->editState->MDI->GetActiveDoc()->vGuides[i];
                if (gl.iPos < 0) continue;
                int scrPos = (gl.bOrient == GUIDE_HORIZONTAL ? Wrd2ScrY(mainPl, gl.iPos)
                                                             : Wrd2ScrX(mainPl, gl.iPos));
                if ((gl.bOrient == GUIDE_HORIZONTAL &&
                    (scrPos < vPort->GetY() || scrPos > vPort->GetY() + vPort->GetHeight())) ||
                    (gl.bOrient == GUIDE_VERTICAL &&
                    (scrPos < vPort->GetX() || scrPos > vPort->GetX() + vPort->GetWidth())))
                    continue;
                if (((gl.bOrient == GUIDE_HORIZONTAL && (my > scrPos - 5 && my < scrPos + 5)) ||
                     (gl.bOrient == GUIDE_VERTICAL && (mx > scrPos - 5 && mx < scrPos + 5))) &&
                    iActiveTool != EWW_TOOL_MOVEOBJECT && iActiveTool != EWW_TOOL_EDITOBJ) {
                    bMouseConsumed = true;
                    if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                        iManipulatedGuide = i;
                        break;
                    }
                }
            }
        }

        if (iManipulatedGuide != -1) {
            stGuideLine gl = GV->editState->MDI->GetActiveDoc()->vGuides[GV->editState->iManipulatedGuide];
            if (conMain->getWidgetAt(mx, my) == vPort->GetWidget()) {
                if (hge->Input_GetKeyState(HGEK_LBUTTON)) {
                    gl.iPos = (gl.bOrient == GUIDE_HORIZONTAL ? Scr2WrdY(mainPl, my)
                                                              : Scr2WrdX(mainPl, mx));
                    GV->editState->MDI->GetActiveDoc()->vGuides[GV->editState->iManipulatedGuide] = gl;
                } else {
                    iManipulatedGuide = -1;
                }
            } else if (gl.iPos == -100 || gl.iPos >= 0) {
                if (hge->Input_GetKeyState(HGEK_LBUTTON)) {
                    gl.iPos = -100;
                    GV->editState->MDI->GetActiveDoc()->vGuides[GV->editState->iManipulatedGuide] = gl;
                } else {
                    GV->editState->MDI->GetActiveDoc()->vGuides.erase(
                            GV->editState->MDI->GetActiveDoc()->vGuides.begin() + iManipulatedGuide);
                    iManipulatedGuide = -1;
                }
            }
            bMouseConsumed = true;
        }
    }

    for (auto & hWindow : hWindows) {
        hWindow->Think();
    }

    if (iMode == EWW_MODE_OBJECT) {
        bMouseConsumed = ObjectThink(bMouseConsumed);
    } else if (iMode == EWW_MODE_TILE) {
        bMouseConsumed = TileThink(bMouseConsumed);
    }

    if (iActiveTool == EWW_TOOL_MEASURE && !bMouseConsumed) {
        if (hge->Input_KeyDown(HGEK_LBUTTON)) {
            int scrX = mx, scrY = my;
            if (hge->Input_GetKeyState(HGEK_SHIFT) && !m_vMeasurePoints.empty()) {
                int x1 = Wrd2ScrX(GetActivePlane(), m_vMeasurePoints[m_vMeasurePoints.size() - 1].first),
                    y1 = Wrd2ScrY(GetActivePlane(), m_vMeasurePoints[m_vMeasurePoints.size() - 1].second);
                int lenx = mx - x1, leny = my - y1;
                float ratio = float(lenx) / float(leny);
                if (ratio >= -0.50f && ratio <= 0.50) { //straight up
                    scrX = x1;
                } else if (ratio > 1.5f || ratio < -1.5f) {
                    scrY = y1;
                } else if (ratio >= 0.50f && ratio <= 1.50f) {
                    if (lenx < 0 && leny < 0) {
                        int dist = std::min(abs(lenx), abs(leny));
                        scrX = x1 - dist;
                        scrY = y1 - dist;
                    } else {
                        int dist = std::min(abs(lenx), abs(leny));
                        scrX = x1 + dist;
                        scrY = y1 + dist;
                    }
                } else if (ratio <= -0.5 && ratio >= -1.5) {
                    if (lenx < 0) {
                        int dist = std::min(lenx, leny);
                        scrX = x1 + dist;
                        scrY = y1 - dist;
                    } else {
                        int dist = std::min(lenx, leny);
                        scrX = x1 - dist;
                        scrY = y1 + dist;
                    }
                }
            }
            m_vMeasurePoints.emplace_back(Scr2WrdX(GetActivePlane(), scrX), Scr2WrdY(GetActivePlane(), scrY));
        }
    }

    bool scrollCaught = false;
    bool hotKeyUsed = hge->Input_GetKeyState(HGEK_SHIFT)
            || hge->Input_GetKeyState(HGEK_CTRL) || hge->Input_GetKeyState(HGEK_ALT);
    if (mx > vPort->GetX() && mx < vPort->GetX() + vPort->GetWidth() && my > vPort->GetY() &&
        my < vPort->GetY() + vPort->GetHeight() && !bMouseConsumed) {
        if ((cScrollOrientation == 0 && !hotKeyUsed) || hge->Input_GetKeyState(HGEK_SHIFT)) {
            fCamY -= hge->Input_GetMouseWheel() * 32;
            scrollCaught = true;
        } else if ((cScrollOrientation == 1 && !hotKeyUsed) || hge->Input_GetKeyState(HGEK_CTRL)) {
            fCamX -= hge->Input_GetMouseWheel() * 32;
            scrollCaught = true;
        }
    }

    if (((cScrollOrientation == 2 && !hotKeyUsed) || hge->Input_GetKeyState(HGEK_ALT)) && !scrollCaught) {
        float fOldZoom = fZoom;
        if (conMain->getWidgetAt(mx, my) == vPort->GetWidget() && !bMouseConsumed)
            (GV->bSmoothZoom ? fDestZoom : fZoom) += hge->Input_GetMouseWheel() * 0.10;

        if (fDestZoom < 0.1) fDestZoom = 0.1;
        if (fDestZoom > 2) fDestZoom = 2;

        if (GV->bSmoothZoom && fZoom != fDestZoom) {
            float fChange = fDestZoom - fZoom;
            if (fChange > 0) fZoom += std::max(fChange * (hge->Timer_GetDelta() * 2), float(0.005));
            else fZoom += std::min(fChange * (hge->Timer_GetDelta() * 2), float(-0.005));
            if ((fChange > 0 && fZoom > fDestZoom) || (fChange < 0 && fZoom < fDestZoom))
                fZoom = fDestZoom;
        }

        if (fZoom < 0.1) fZoom = 0.1;
        if (fZoom > 2) fZoom = 2;
        //if( hge->Input_GetMouseWheel() > 0 ){
        //fCamX += ((hge->System_GetState(HGE_SCREENWIDTH)/2)*0.2);
        //fCamY += ((hge->System_GetState(HGE_SCREENHEIGHT)/2)*0.2);
        fCamX += (vPort->GetWidth() / 2 / fOldZoom) / (GetActivePlane()->GetMoveModX() / 100.0f);
        fCamY += (vPort->GetHeight() / 2 / fOldZoom) / (GetActivePlane()->GetMoveModY() / 100.0f);
        fCamX -= (vPort->GetWidth() / 2 / fZoom) / (GetActivePlane()->GetMoveModX() / 100.0f);
        fCamY -= (vPort->GetHeight() / 2 / fZoom) / (GetActivePlane()->GetMoveModY() / 100.0f);
        //}
        if (fOldZoom != fZoom) {
            if (objContext->isVisible()) objContext->setVisible(false);
            if (tilContext->isVisible()) tilContext->setVisible(false);
            UpdateScrollBars();
            float fZ = 0;
            if (fZoom > 1) {
                fZ += (fZoom - 1.0) * 5.0f;
            } else if (fZoom < 1.0f) {
                fZ -= ((1.0f - fZoom) / 0.9f) * 5.0f;
            }
            sliZoom->setValue(fZ);
        }
    }

    //SHORTCUTS
    HandleHotkeys();

    if (!bMouseConsumed && (vObjectsPicked.empty() ||
                            (!hge->Input_GetKeyState(HGEK_CTRL) && !hge->Input_GetKeyState(HGEK_SHIFT) &&
                            !hge->Input_GetKeyState(HGEK_ALT)))) {
        if (fCameraMoveTimer == 0 || fCameraMoveTimer > 0.1) {
            if (fCameraMoveTimer > 0.1) fCameraMoveTimer -= 0.1;
            if (vPort->GetWidget()->isFocused() && bWindowFocused) {
                if (hge->Input_GetKeyState(HGEK_UP)) {
                    fCamY -= GetActivePlane()->GetTileHeight();
                } else if (hge->Input_GetKeyState(HGEK_DOWN)) {
                    fCamY += GetActivePlane()->GetTileHeight();
                }
                if (hge->Input_GetKeyState(HGEK_LEFT)) {
                    fCamX -= GetActivePlane()->GetTileWidth();
                } else if (hge->Input_GetKeyState(HGEK_RIGHT)) {
                    fCamX += GetActivePlane()->GetTileWidth();
                }
            }
        }
        if (hge->Input_GetKeyState(HGEK_UP) || hge->Input_GetKeyState(HGEK_DOWN) || hge->Input_GetKeyState(HGEK_LEFT) ||
            hge->Input_GetKeyState(HGEK_RIGHT))
            fCameraMoveTimer += hge->Timer_GetDelta();
        else
            fCameraMoveTimer = 0;
    }

    if (fCamX < -WORKSPACE_MARGINS_SIZE) fCamX = -WORKSPACE_MARGINS_SIZE;
    if (fCamY < -WORKSPACE_MARGINS_SIZE) fCamY = -WORKSPACE_MARGINS_SIZE;

    if (fCamX > hParser->GetMainPlane()->GetPlaneWidthPx() - vPort->GetWidth() / fZoom + WORKSPACE_MARGINS_SIZE)
        fCamX = hParser->GetMainPlane()->GetPlaneWidthPx() - vPort->GetWidth() / fZoom + WORKSPACE_MARGINS_SIZE;

    if (fCamY > hParser->GetMainPlane()->GetPlaneHeightPx() - vPort->GetHeight() / fZoom + WORKSPACE_MARGINS_SIZE)
        fCamY = hParser->GetMainPlane()->GetPlaneHeightPx() - vPort->GetHeight() / fZoom + WORKSPACE_MARGINS_SIZE;

    sliHor->setValue(fCamX);
    sliVer->setValue(fCamY);

    if (fCamLastX != fCamX || fCamLastY != fCamY) {
        vPort->MarkToRedraw();
        fCamLastX = fCamX;
        fCamLastY = fCamY;
    }
    if (fLastZoom != fZoom) {
        vPort->MarkToRedraw();
        fLastZoom = fZoom;
    }

    if (bConstRedraw)
        vPort->MarkToRedraw();

    return false;
}

void State::EditingWW::GainFocus(ReturnCode<void> code, bool bFlipped) {
    switch (code.type) {
        case ReturnCodeType::LoadMap:
            if (code.value != 0) {
                DocumentData *dd = (DocumentData *) code.value;
                hParser = dd->hParser;
                SprBank = dd->hSprBank;
                hTileset = dd->hTilesBank;
                hSndBank = dd->hSndBank;
                hAniBank = dd->hAniBank;
                hDataCtrl = dd->hDataCtrl;
                hCustomLogics = dd->hCustomLogicBank;
                //delete md;
                if (MDI->GetActiveDoc() != NULL) {
                    PrepareForDocumentSwitch();
                }
                hAppMenu->SyncDocumentOpened();
                MDI->SetActiveDoc(MDI->AddDocument(dd));
                SynchronizeWithParser();
                vPort->MarkToRedraw();

                if (strlen(hParser->GetFilePath()) > 0) {
                    char *n = SHR::GetFile(hParser->GetFilePath());
                    bool bdots = 0;
                    while (GV->fntMyriad16->GetStringWidth(n) > 140) {
                        if (strlen(n) < 5) break;
                        if (!bdots) {
                            n[strlen(n) - 4] = '.';
                            n[strlen(n) - 3] = '.';
                            n[strlen(n) - 2] = '.';
                            n[strlen(n) - 1] = '\0';
                        } else {
                            n[strlen(n) - 4] = '.';
                            n[strlen(n) - 1] = '\0';
                        }
                        bdots = 1;
                    }
                    //winWelcome->setY(vPort->GetY() + vPort->GetHeight() / 2 - winWelcome->getHeight() / 2);
                }
            } else {
                MDI->UnlockMapReload();
            }
        break;
        case ReturnCodeType::DialogState:

        break;
        case ReturnCodeType::MapShot:
            if (code.value == 1) {
                winMapShot->setVisible(false);
                if (cbmsOpenFileDirectory->isSelected()) {
                    char *command = new char[strlen(tfmsSaveAs->getText().c_str()) + 32];
                    sprintf(command, "Explorer.exe /select,%s", tfmsSaveAs->getText().c_str());
                    system(command);
                    delete[] command;
                }
            }
        break;
        case ReturnCodeType::ObjectProp:
            if (bFlipped) {
                int *ptr = (int *) code.value;
                toolsaMinX = ptr[0];
                toolsaMinY = ptr[1];
                toolsaMaxX = ptr[2];
                toolsaMaxY = ptr[3];
                delete[] ptr;
                SetTool(EWW_TOOL_OBJSELAREA);
            } else if (code.value == 1) {
                objContext->EmulateClickID(OBJMENU_MOVE);
            } else if (code.value == 2 && !vObjectsPicked.empty()) {
                GetActivePlane()->DeleteObject(vObjectsPicked[0]);
            }
        break;
    }
}

WWD::Plane *State::EditingWW::GetActivePlane() {
    return hParser->GetPlane(hmbTile->ddActivePlane->getSelected());
}

int State::EditingWW::GetActivePlaneID() {
    return hmbTile->ddActivePlane->getSelected();
}

void State::EditingWW::SetIconBarVisible(bool b) {
    //ddActivePlane->setVisible(b);
    cbutActiveMode->setVisible(b);
    //if( hmbActive != 0 )
    // hmbActive->setVisible(b);
}

void State::EditingWW::LockToolSpecificFunctions(bool bLock) {
    cbutActiveMode->setEnabled(!bLock);
#ifdef WM_ADD_LUA_EXECUTER
    butIconLua->setEnabled(bLock);
#endif

    if (hmbActive != 0)
        hmbActive->SetEnabled(!bLock);

    MDI->BlockDocumentSelection(bLock);
    hAppMenu->SetToolSpecificEnabled(!bLock);
}

void State::EditingWW::SetTool(int iNewTool) {
    CloseTool(iNewTool);
    OpenTool(iNewTool);
    iActiveTool = iNewTool;
}

void State::EditingWW::OpenTool(int iNewTool) {
    if (iNewTool == EWW_TOOL_PENCIL || iNewTool == EWW_TOOL_FILL) {
        if (iNewTool == EWW_TOOL_FILL) {
            hmbTile->butIconFill->setHighlight(true);
        } else {
            hmbTile->butIconPencil->setHighlight(true);
        }
        if (iActiveTool != EWW_TOOL_FILL) {
            iTileDrawMode = EWW_DRAW_POINT;
        }
        iActiveTool = iNewTool;
        RebuildTilePicker(true);
        winTilePicker->setVisible(true);
    } else if (iNewTool == EWW_TOOL_WRITEID) {
        hmbTile->butIconWriteID->setHighlight(true);
        iTileWriteIDx = iTileWriteIDy = -1;
    } else if (iNewTool == EWW_TOOL_SPACEOBJ) {
        winSpacing->setVisible(true);
        winSpacing->requestMoveToTop();
    } else if (iNewTool == EWW_TOOL_BRUSH) {
        hmbTile->butIconBrush->setHighlight(true);
        iActiveTool = iNewTool;
        iTileDrawMode = EWW_DRAW_POINT;
        RebuildTilePicker(true);
        if (!winTilePicker->isVisible())
            winTilePicker->setVisible(true);
        iLastBrushPlacedX = iLastBrushPlacedY = -1;
    } else if (iNewTool == EWW_TOOL_BRUSHOBJECT) {
        winObjectBrush->setPosition(vPort->GetX(), vPort->GetY() + vPort->GetHeight() - winObjectBrush->getHeight());
        winObjectBrush->setVisible(true);
        cbobrApplyScatterSeparately->setEnabled(vObjectsBrushCB.size() > 1);
    } else if (iNewTool == EWW_TOOL_MEASURE) {
        PutToBottomRight(winMeasureOpt);
        winMeasureOpt->setVisible(true);
    } else if (iNewTool == EWW_TOOL_OBJSELAREA) {
        bDragSelection = false;
        PutToBottomRight(wintoolSelArea);
        wintoolSelArea->setShow(true);
        wintoolSelArea->unDrag();
        char label[200];
        sprintf(label, "~w~X1: ~y~%d~w~, Y1: ~y~%d~w~, X2: ~y~%d~w~, Y2: ~y~%d~w~",
                toolsaMinX, toolsaMinY, toolsaMaxX, toolsaMaxY);
        labtoolSelAreaValues->setCaption(label);
        labtoolSelAreaValues->adjustSize();
        toolsaAction = TOOL_OBJSA_NONE;
        LockToolSpecificFunctions(true);
    } else if (iNewTool == EWW_TOOL_EDITOBJ) {
        MinimizeWindows();
        LockToolSpecificFunctions(true);
        bEditObjDelete = false;
    } else if (iNewTool == EWW_TOOL_ALIGNOBJ) {

    } else if (iNewTool == EWW_TOOL_MOVEOBJECT) {
        bEditObjDelete = false;
    } else if (iNewTool == EWW_TOOL_DUPLICATE) {
        ShowAndUpdateDuplicateMenu();
    } else if (iNewTool == EWW_TOOL_NONE) {
        hmbTile->butIconSelect->setHighlight(true);
        iTilePicked = EWW_TILE_NONE;
        GV->SetCursor(DEFAULT);
    } else if (iNewTool == EWW_TOOL_ZOOM) {
        hmbTile->butZoom->setHighlight(true);
        GV->SetCursor(ZOOM_IN);
    }
}

void State::EditingWW::CloseTool(int iNewTool) {
    if (iActiveTool == EWW_TOOL_PENCIL || iActiveTool == EWW_TOOL_FILL) {
        hmbTile->butIconPencil->setHighlight(false);
        hmbTile->butIconFill->setHighlight(false);
        if (iNewTool != EWW_TOOL_PENCIL && iNewTool != EWW_TOOL_FILL) {
            iTilePicked = EWW_TILE_NONE;
            if (iNewTool != EWW_TOOL_BRUSH) {
                winTilePicker->setVisible(false);
            }
        }
        wintpFillColor->setVisible(false);
        vTileGhosting.clear();
        vPort->MarkToRedraw();
        return;
    } else if (iActiveTool == EWW_TOOL_WRITEID) {
        hmbTile->butIconWriteID->setHighlight(false);
        vTileGhosting.clear();
        vPort->MarkToRedraw();
    } else if (iActiveTool == EWW_TOOL_SPACEOBJ) {
        winSpacing->setVisible(false);
    } else if (iActiveTool == EWW_TOOL_BRUSH) {
        hmbTile->butIconBrush->setHighlight(false);
        if (iNewTool != EWW_TOOL_PENCIL && iNewTool != EWW_TOOL_FILL) {
            winTilePicker->setVisible(false);
        }

        if (iTilePicked >= 0 && MDI->GetActiveDoc() != NULL)
            hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetBrushByIterator(
                    iTilePicked)->RemoveSettingsFromContainer(winTilePicker);
        vTileGhosting.clear();
        iTilePicked = EWW_TILE_NONE;
        return;
    } else if (iActiveTool == EWW_TOOL_DUPLICATE) {
        winDuplicate->setVisible(false);
    } else if (iActiveTool == EWW_TOOL_BRUSHOBJECT) {
        winObjectBrush->setVisible(false);
    } else if (iActiveTool == EWW_TOOL_MEASURE) {
        winMeasureOpt->setVisible(false);
    } else if (iActiveTool == EWW_TOOL_OBJSELAREA) {
        wintoolSelArea->setShow(false);
        LockToolSpecificFunctions(false);
    } else if (iActiveTool == EWW_TOOL_EDITOBJ) {
        LockToolSpecificFunctions(false);
        delete hEditObj;
        hEditObj = NULL;
        MaximizeWindows();
    } else if (iActiveTool == EWW_TOOL_ALIGNOBJ) {

    } else if (iActiveTool == EWW_TOOL_NONE) {
        hmbTile->butIconSelect->setHighlight(false);
        iTileSelectX1 = iTileSelectY1 = iTileSelectX2 = iTileSelectY2 = -1;
        iDragSelectionOrigX = iDragSelectionOrigY = -1;
        bDragSelection = false;
        vPort->MarkToRedraw();
    } else if (iActiveTool == EWW_TOOL_ZOOM) {
        hmbTile->butZoom->setHighlight(false);
        GV->SetCursor(DEFAULT);
    }
}

void State::EditingWW::MinimizeWindows() {
    for (auto & i : vWidgetsToMinimalise) {
        if (i->isVisible()) {
            i->setVisible(false);
            vMinimalisedWidgets.push_back(i);
        }
    }
    /*if (NewMap_data != 0)
        NewMap_data->bKill = true;*/
}

void State::EditingWW::MaximizeWindows() {
    for (auto & vMinimalisedWidget : vMinimalisedWidgets)
        vMinimalisedWidget->setVisible(true);
    vMinimalisedWidgets.clear();
}

void State::EditingWW::FreeResources() {
    for (auto object : vObjectClipboard) {
        delete object;
    }
    vObjectClipboard.clear();
    vObjectsBrushCB.clear();
    vObjectsHL.clear();
    vObjectsPicked.clear();
    //delete hStartingPosObj;
}

void State::EditingWW::MarkUnsaved() {
    if (MDI->GetActiveDoc()->bSaved) {
        char tmp[128];
        char *filename = SHR::GetFile(hParser->GetFilePath());
        sprintf(tmp, "%s* - %s", filename, WA_TITLEBAR);
        delete[] filename;
        hge->System_SetState(HGE_TITLE, tmp);
        MDI->GetActiveDoc()->bSaved = 0;
    }
}

void State::EditingWW::UpdateScrollBars() {
    if (hParser == NULL) {
        sliHor->setScale(0, 0);
        sliVer->setScale(0, 0);
        return;
    }

    double seh = hParser->GetMainPlane()->GetPlaneWidthPx() - vPort->GetWidth() / fZoom + WORKSPACE_MARGINS_SIZE,
           sev = hParser->GetMainPlane()->GetPlaneHeightPx() - vPort->GetHeight() / fZoom + WORKSPACE_MARGINS_SIZE;

    if (seh + WORKSPACE_MARGINS_SIZE > 0) {
        sliHor->setVisible(true);
        sliHor->setScaleStart(-WORKSPACE_MARGINS_SIZE);
        sliHor->setScaleEnd(seh);
        sliHor->adjustMarkerLength();
    } else {
        sliHor->setVisible(false);
    }

    if (sev + WORKSPACE_MARGINS_SIZE > 0) {
        sliVer->setVisible(true);
        sliVer->setScaleStart(-WORKSPACE_MARGINS_SIZE);
        sliVer->setScaleEnd(sev);
        sliVer->adjustMarkerLength();
    } else {
        sliVer->setVisible(false);
    }
}

void State::EditingWW::SwitchPlane() {
    UpdateScrollBars();
    if (iMode == EWW_MODE_OBJECT && !(GetActivePlane()->GetFlags() & WWD::Flag_p_MainPlane)) {
        SetTool(EWW_TOOL_NONE);
        SetMode(EWW_MODE_TILE);
    }
    if (iActiveTool == EWW_TOOL_BRUSH || iActiveTool == EWW_TOOL_FILL || iActiveTool == EWW_TOOL_PENCIL) {
        iTilePicked = EWW_TILE_NONE;
        RefreshTilePickerSlider();
    }
    if (iActiveTool == EWW_TOOL_BRUSH)
        vTileGhosting.clear();
    hAppMenu->SyncPlaneSwitched();
}

bool State::EditingWW::OpenDocuments() {
    char szFileopen[1024] = "\0";
    OPENFILENAME ofn;
    ZeroMemory((&ofn), sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hge->System_GetState(HGE_HWND);
    ofn.lpstrFilter = "WapWorld Document (*.WWD)\0*.wwd\0Wszystkie pliki (*.*)\0*.*\0\0";
    ofn.lpstrFile = szFileopen;
    ofn.nMaxFile = 1024;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
    ofn.lpstrDefExt = "wwd";
    ofn.lpstrInitialDir = GV->szLastOpenPath;
    if (GetOpenFileName(&ofn) && ofn.lpstrFile) {
        char* str = ofn.lpstrFile;
        std::string dir(str);

        if (!std::filesystem::is_directory(dir)) {
            const char* actualDir = SHR::GetDir(dir.c_str());
            vstrMapsToLoad = { dir };
            GV->SetLastOpenPath(actualDir);
            delete[] actualDir;
        }
        else {
            GV->SetLastOpenPath(str);
            dir.push_back('\\');
            str += dir.length();
            while (*str) {
                std::string filename(str);
                str += (filename.length() + 1);
                vstrMapsToLoad.push_back(dir + filename);
            }
        }

        return true;
    }
    return false;
}

void State::EditingWW::SaveAs() {
    OPENFILENAME ofn;
    char szFileopen[512] = "\0";
    ZeroMemory((&ofn), sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hge->System_GetState(HGE_HWND);
    ofn.lpstrFilter = "WapWorld Document (*.wwd)\0*.wwd\0Wszystkie pliki (*.*)\0*.*\0\0";
    ofn.lpstrFile = szFileopen;
    ofn.nMaxFile = sizeof(szFileopen);
    ofn.lpstrDefExt = "wwd";
    ofn.lpstrInitialDir = GV->szLastSavePath;
    if (GetSaveFileName(&ofn)) {
        hParser->SetFilePath((const char *) szFileopen);
        bool ok = false;
        try {
            MDI->PrepareDocToSave(MDI->GetActiveDocIt());
            hParser->CompileToFile(szFileopen, true);
            hDataCtrl->RelocateDocument(szFileopen);
            ok = true;
        }
        catch (WWD::Exception &exc) {
#ifdef BUILD_DEBUG
            GV->Console->Printf("~r~WWD exception: ~y~%d ~w~(~y~%s~w~:~y~%d~w~)", exc.iErrorCode, exc.szFile, exc.iLine);
#else
            GV->Console->Printf("~r~WWD exception ~y~%d", exc.iErrorCode);
#endif
        }
        if (ok) {
            char *fl = SHR::GetFile(hParser->GetFilePath());
            delete[] MDI->GetActiveDoc()->szFileName;
            int size = strlen(fl) - 3;
            MDI->GetActiveDoc()->szFileName = new char[size];
            strncpy(MDI->GetActiveDoc()->szFileName, fl, size - 1);
            MDI->GetActiveDoc()->szFileName[size - 1] = 0;
            MDI->GetActiveDoc()->bSaved = true;
            delete[] fl;
            MDI->UpdateCrashList();
        }
        char *lastSave = SHR::GetDir(szFileopen);
        GV->SetLastSavePath(lastSave);
        delete[] lastSave;
    }
}

void State::EditingWW::Export() {
    OPENFILENAME ofn;
    char szFileopen[512] = "\0";
    ZeroMemory((&ofn), sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hge->System_GetState(HGE_HWND);
    ofn.lpstrFilter = "Wszystkie pliki (*.*)\0*.*\0\0";
    ofn.lpstrFile = szFileopen;
    ofn.nMaxFile = sizeof(szFileopen);
    ofn.lpstrInitialDir = GV->szLastSavePath;
    if (GetSaveFileName(&ofn)) {
        std::ofstream ofs(szFileopen, std::ofstream::out);
        WMD::ExportTileProperties(hParser, ofs);
        ofs.close();

        char *lastSave = SHR::GetDir(szFileopen);
        GV->SetLastSavePath(lastSave);
        delete[] lastSave;
    }
}

void State::EditingWW::SetMode(int piMode) {
    if (iMode == piMode) return;
    ExitMode(iMode);
    EnterMode(piMode);
    iMode = piMode;
    hAppMenu->SyncModeSwitched();
}

void State::EditingWW::EnterMode(int piMode) {
    if (piMode == EWW_MODE_TILE) {
        SwitchActiveModeMenuBar(hmbTile);
        cbutActiveMode->setSelectedEntryID(0);
        UpdateScrollBars();
    } else if (piMode == EWW_MODE_OBJECT) {
        bDragSelection = false;
        SwitchActiveModeMenuBar(hmbObject);
        cbutActiveMode->setSelectedEntryID(1);
        UpdateScrollBars();
    }
}

void State::EditingWW::ExitMode(int piMode) {
    if (piMode == EWW_MODE_TILE) {
        SwitchActiveModeMenuBar(NULL);
        if (iTileSelectX1 != -1)
            vPort->MarkToRedraw();
        iTileSelectX1 = iTileSelectX2 = iTileSelectY1 = iTileSelectY2 = -1;
        tilContext->setVisible(false);
    } else if (piMode == EWW_MODE_OBJECT) {
        objContext->setVisible(false);
        winSearchObj->setVisible(false);
        SwitchActiveModeMenuBar(NULL);
        vObjectsPicked.clear();
        vObjectsHL.clear();
    }
}

SHR::But *State::EditingWW::MakeButton(int x, int y, Gfx16Icons icon, SHR::Container *dest, bool enable, bool visible,
                                       gcn::ActionListener *actionListener) {
    SHR::But *but = new SHR::But(GV->hGfxInterface, GV->sprIcons16[icon]);
    but->setDimension(gcn::Rectangle(0, 0, 18, 18));
    but->addActionListener(actionListener ? actionListener : mainListener);
    but->setEnabled(enable);
    but->setVisible(visible);
    dest->add(but, x, y);
    return but;
}

SHR::But *State::EditingWW::MakeButton(int x, int y, Gfx32Icons icon, SHR::Container *dest, bool enable, bool visible,
                                       gcn::ActionListener *actionListener) {
    SHR::But *but = new SHR::But(GV->hGfxInterface, GV->sprIcons[icon]);
    but->setDimension(gcn::Rectangle(0, 0, 32, 32));
    but->addActionListener(actionListener ? actionListener : mainListener);
    but->setEnabled(enable);
    but->setVisible(visible);
    dest->add(but, x, y);
    return but;
}

void State::EditingWW::ToggleFullscreen() {
    bMaximized = !bMaximized;

    tagMONITORINFO monitorInfo;
    monitorInfo.cbSize = sizeof(tagMONITORINFO);
    HMONITOR monitor = MonitorFromWindow(hge->System_GetState(HGE_HWND), MONITOR_DEFAULTTOPRIMARY);
    GetMonitorInfoA(monitor, &monitorInfo);

    if (bMaximized) {
        hge->System_SetState(HGE_BORDER_WIDTH, 0);
        ::SetWindowPos(hge->System_GetState(HGE_HWND), NULL, monitorInfo.rcWork.left, monitorInfo.rcWork.top,
                monitorInfo.rcWork.right - monitorInfo.rcWork.left, monitorInfo.rcWork.bottom - monitorInfo.rcWork.top, SWP_SHOWWINDOW);
    } else {
        hge->System_SetState(HGE_BORDER_WIDTH, 5);
        const auto scr_width = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
        const auto scr_height = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;
        ::SetWindowPos(hge->System_GetState(HGE_HWND), NULL, monitorInfo.rcWork.left + (scr_width - GV->iScreenW) / 2,
                     monitorInfo.rcWork.top + (scr_height - GV->iScreenH) / 2, GV->iScreenW, GV->iScreenH, SWP_SHOWWINDOW);
    }
    FixInterfacePositions();
}

void State::EditingWW::FixInterfacePositions() {
    conMain->setDimension(gcn::Rectangle(0, 0, hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT)));
    MDI->SetY(LAY_MDI_Y);
    MDI->UpdateMaxTabSize();
    if (hParser) {
        int rulersOffset = hRulers->IsVisible() * 18;
        vPort->SetPos(1 + rulersOffset, LAY_VIEWPORT_Y + rulersOffset);
        vPort->Resize(hge->System_GetState(HGE_SCREENWIDTH) - 17 - rulersOffset + !bMaximized,
                      hge->System_GetState(HGE_SCREENHEIGHT) - vPort->GetY() - LAY_STATUS_H - 17);
        sliHor->setDimension(gcn::Rectangle(vPort->GetX() - rulersOffset - 1, vPort->GetY() + vPort->GetHeight(),
                vPort->GetWidth() + rulersOffset + 3, 18
        ));
        sliVer->setDimension(gcn::Rectangle(vPort->GetX() + vPort->GetWidth() - !bMaximized, vPort->GetY() - rulersOffset,
                18, vPort->GetHeight() + rulersOffset - 2
        ));
        sliZoom->setPosition(hge->System_GetState(HGE_SCREENWIDTH) - 160, hge->System_GetState(HGE_SCREENHEIGHT) - 25);
    } else {
        vPort->SetPos(1, LAY_MODEBAR_Y);
        vPort->Resize(hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT) - LAY_STATUS_H - LAY_MODEBAR_Y - 9);
        winWelcome->setPosition(vPort->GetX() + (vPort->GetWidth() - winWelcome->getWidth()) / 2,
                                vPort->GetY() + (vPort->GetHeight() - winWelcome->getHeight()) / 2);
    }
    UpdateScrollBars();
    butMicroTileCB->setPosition(4, hge->System_GetState(HGE_SCREENHEIGHT) - LAY_STATUS_H + 3);
    butMicroObjectCB->setPosition(32, hge->System_GetState(HGE_SCREENHEIGHT) - LAY_STATUS_H + 3);
    GV->Console->FixPos();
}

bool State::EditingWW::PromptExit() {
    if (MDI->IsAnyDocUnsaved()) {
        for (int i = 0; i < MDI->GetDocsCount(); i++) {
            if (!MDI->CloseDocByIt(i)) return 0;
            i--;
        }
    }
    return 1;
}

int State::EditingWW::Scr2WrdX(WWD::Plane *pl, int px) {
    float x = int(fCamX * (pl->GetMoveModX() / 100.0f) * fZoom) + px - vPort->GetX();
    if (!pl->GetFlag(WWD::Flag_p_MainPlane)) {
        x -= vPort->GetWidth() / 2.f * (1.0f - (pl->GetMoveModX() / 100.0f));
    }
    return x / fZoom;
}

int State::EditingWW::Scr2WrdY(WWD::Plane *pl, int py) {
    float y = int(fCamY * (pl->GetMoveModY() / 100.0f) * fZoom) + py - vPort->GetY();
    if (!pl->GetFlag(WWD::Flag_p_MainPlane)) {
        y -= vPort->GetHeight() / 2.f * (1.0f - (pl->GetMoveModY() / 100.0f));
    }
    return y / fZoom;
}

int State::EditingWW::Wrd2ScrX(WWD::Plane *pl, int x) {
    x = x * fZoom;
    x += vPort->GetX();
    x -= int(fCamX * (pl->GetMoveModX() / 100.0f) * fZoom);
    if (!pl->GetFlag(WWD::Flag_p_MainPlane)) {
        x += vPort->GetWidth() / 2.f * (1.0f - (pl->GetMoveModX() / 100.0f));
    }
    return x;
}

int State::EditingWW::Wrd2ScrY(WWD::Plane *pl, int y) {
    y = y * fZoom;
    y += vPort->GetY();
    y -= int(fCamY * (pl->GetMoveModY() / 100.0f) * fZoom);
    if (!pl->GetFlag(WWD::Flag_p_MainPlane)) {
        y += vPort->GetHeight() / 2.f * (1.0f - (pl->GetMoveModY() / 100.0f));
    }
    return y;
}

int State::EditingWW::Wrd2ScrXrb(WWD::Plane *pl, int x) {
    x = x * fZoom;
    x -= int(fCamX * (pl->GetMoveModX() / 100.0f) * fZoom);
    if (!pl->GetFlag(WWD::Flag_p_MainPlane)) {
        x += vPort->GetWidth() / 2.f * (1.0f - (pl->GetMoveModX() / 100.0f));
    }
    return x;
}

int State::EditingWW::Wrd2ScrYrb(WWD::Plane *pl, int y) {
    y = y * fZoom;
    y -= int(fCamY * (pl->GetMoveModY() / 100.0f) * fZoom);
    if (!pl->GetFlag(WWD::Flag_p_MainPlane)) {
        y += vPort->GetHeight() / 2.f * (1.0f - (pl->GetMoveModY() / 100.0f));
    }
    return y;
}


void State::EditingWW::SyncAttribMenuWithTile() {
    if (itpSelectedTile == -1) return;
    int dx, dy;
    winTileProp->getAbsolutePosition(dx, dy);
    delete hTempAttrib;
    WWD::TileAttrib *attrib = hParser->GetTileAttribs(itpSelectedTile);
    if (!attrib) {
        hTempAttrib = nullptr;
        return;
    }

    char tmp[64];
    sprintf(tmp, "%d", attrib->getWidth());
    tftpW->setText(tmp);
    sprintf(tmp, "%d", attrib->getHeight());
    tftpH->setText(tmp);

    int inside = 0;

    switch (attrib->getType()) {
        case WWD::AttribType_Single: {
            inside = ((WWD::SingleTileAttrib *) attrib)->getAttrib();
            hTempAttrib = new WWD::SingleTileAttrib((WWD::SingleTileAttrib *) attrib);
            rbtpSingle->setSelected(true);
            tftpX1->setVisible(false);
            tftpY1->setVisible(false);
            tftpX2->setVisible(false);
            tftpY2->setVisible(false);
            for (auto &pRadBut : rbtpOut)
                pRadBut->setVisible(false);
            winTileProp->setDimension(gcn::Rectangle(dx, dy, 300, 260));
            break;
        }
        case WWD::AttribType_Double: {
            inside = ((WWD::DoubleTileAttrib *) attrib)->getInsideAttrib();
            int outside = ((WWD::DoubleTileAttrib *) attrib)->getOutsideAttrib();
            hTempAttrib = new WWD::DoubleTileAttrib((WWD::DoubleTileAttrib *) attrib);
            rbtpDouble->setSelected(true);
            tftpX1->setVisible(true);
            tftpY1->setVisible(true);
            tftpX2->setVisible(true);
            tftpY2->setVisible(true);
            for (int i = 0; i < 5; i++) {
                rbtpOut[i]->setVisible(true);
            }
            rbtpOut[outside >= WWD::Attrib_Unknown ? 0 : outside]->setSelected(true);
            auto mask = ((WWD::DoubleTileAttrib *) attrib)->getMask();
            sprintf(tmp, "%d", mask.x1);
            tftpX1->setText(tmp);
            sprintf(tmp, "%d", mask.y1);
            tftpY1->setText(tmp);
            sprintf(tmp, "%d", mask.x2);
            tftpX2->setText(tmp);
            sprintf(tmp, "%d", mask.y2);
            tftpY2->setText(tmp);
            winTileProp->setDimension(gcn::Rectangle(dx, dy, 300, 350));
            break;
        }
        case WWD::AttribType_Mask: {
            hTempAttrib = new WWD::MaskTileAttrib((WWD::MaskTileAttrib *) attrib);
            rbtpMask->setSelected(true);
            tftpX1->setVisible(false);
            tftpY1->setVisible(false);
            tftpX2->setVisible(false);
            tftpY2->setVisible(false);
            for (auto &pRadBut : rbtpOut)
                pRadBut->setVisible(false);
            winTileProp->setDimension(gcn::Rectangle(dx, dy, 300, 260));
            break;
        }
    }

    rbtpIn[inside >= WWD::Attrib_Unknown ? 0 : inside]->setSelected(true);
}

std::vector<WWD::Object *> State::EditingWW::GetObjectsInArea(WWD::Plane *plane, int x, int y, int w, int h) {
    std::vector<WWD::Object *> vec;
    for (int obit = 0; obit < plane->GetObjectsCount(); obit++) {
        WWD::Object *obj = plane->GetObjectByIterator(obit);
        float posx = obj->GetParam(WWD::Param_LocationX), posy = obj->GetParam(WWD::Param_LocationY);
        hgeSprite *spr = SprBank->GetObjectSprite(obj);
        float x1, y1, x2, y2;

        float hsx, hsy;
        spr->GetHotSpot(&hsx, &hsy);
        float sprw = spr->GetWidth() / 2, sprh = spr->GetHeight() / 2;
        hsx -= sprw;
        hsy -= sprh;
        x1 = posx - sprw - hsx;
        y1 = posy - sprw - hsx;
        x2 = posx + sprw - hsy;
        y2 = posy + sprw - hsy;

        if ((x1 > x && x1 < x + w) || (y1 > y && y1 < y + h) ||
            (x2 > x && x2 < x + w) || (y2 > y && y2 < y + h)) {
            vec.push_back(obj);
        }
    }
    return vec;
}

void State::EditingWW::GfxRestore() {
    vPort->GfxLost();
    if (hParser != NULL)
        for (int i = 0; i < hParser->GetPlanesCount(); i++) {
            if (hPlaneData[i]->hRB != NULL)
                hPlaneData[i]->hRB->GfxLost();
        }
    vPort->MarkToRedraw();
}

void State::EditingWW::PrepareForDocumentSwitch() {
    if (hParser == NULL) return;
    MDI->GetActiveDoc()->fCamX = fCamX;
    MDI->GetActiveDoc()->fCamY = fCamY;
    MDI->GetActiveDoc()->iSelectedPlane = hmbTile->ddActivePlane->getSelected();
    MDI->GetActiveDoc()->vObjectsPicked = vObjectsPicked;
    MDI->GetActiveDoc()->fZoom = fZoom;
    MDI->GetActiveDoc()->hPlaneData = hPlaneData;
    MDI->GetActiveDoc()->hStartingPosObj = hStartingPosObj;
    MDI->GetActiveDoc()->iTileSelectX1 = iTileSelectX1;
    MDI->GetActiveDoc()->iTileSelectY1 = iTileSelectY1;
    MDI->GetActiveDoc()->iTileSelectX2 = iTileSelectX2;
    MDI->GetActiveDoc()->iTileSelectY2 = iTileSelectY2;
}

void State::EditingWW::SyncLogicBrowser() {
    bool validSelect = (lbbrlLogicList->getSelected() >= 0 &&
                        lbbrlLogicList->getSelected() < hCustomLogics->getNumberOfElements());
    if (validSelect) {
        if (!bLogicBrowserExpanded) ExpandLogicBrowser();

        cCustomLogic *logic = hCustomLogics->GetLogicByIterator(lbbrlLogicList->getSelected());
        cFile f = logic->GetFile();
        labbrlLogicNameV->setCaption(logic->GetName());
        std::string path = logic->GetPath();
        if (path.length() > 50) {
            path[47] = '.';
            path[48] = '.';
            path[49] = '.';
            path[50] = '\0';
            path.resize(50);
        }
        for (auto &it : path) {
            if (it == '/') it = '\\';
        }
        labbrlFilePathV->setCaption(path);
        char *szFileSize = SHR::FormatSize(f.hFeed->GetFileSize(f.strPath.c_str()));
        labbrlFileSizeV->setCaption(szFileSize);
        delete[] szFileSize;
        labbrlFileChecksumV->setCaption(logic->GetHash());
        szFileSize = SHR::FormatTimeFromUnix(f.hFeed->GetFileModTime(f.strPath.c_str()));
        labbrlFileModDateV->setCaption(szFileSize);
        delete[] szFileSize;
    } else if (bLogicBrowserExpanded) {
        FoldLogicBrowser();
    }

    labbrlLogicNameV->adjustSize();
    labbrlFileSizeV->adjustSize();
    labbrlFileChecksumV->adjustSize();
    labbrlFileModDateV->adjustSize();
    labbrlFilePathV->adjustSize();
}

void State::EditingWW::DocumentSwitched() {
    vObjectsHL.clear();
    sliVer->setVisible(MDI->GetActiveDoc() != NULL);
    sliHor->setVisible(MDI->GetActiveDoc() != NULL);
    winWelcome->setVisible(MDI->GetActiveDoc() == NULL);
    sliZoom->setVisible(MDI->GetActiveDoc() != NULL);
    butMicroTileCB->setVisible(MDI->GetActiveDoc() != NULL);
    butMicroObjectCB->setVisible(MDI->GetActiveDoc() != NULL);
    if (MDI->GetActiveDoc() == NULL) {
        winLogicBrowser->setVisible(false);
        if (conCrashRetrieve != NULL) {
            winWelcome->remove(conCrashRetrieve);
            delete conCrashRetrieve;
            conCrashRetrieve = NULL;
            winWelcome->add(conRecentFiles, 0, winWelcome->getHeight() + 50);
            winWelcome->setY(vPort->GetY() + vPort->GetHeight() / 2 - winWelcome->getHeight() / 2);
        }
        SetTool(EWW_TOOL_NONE);
        SwitchActiveModeMenuBar(NULL);
        hParser = NULL;
        ddpmTileSet->setListModel(0);
        lbbrlLogicList->setListModel(0);
        InitEmpty();
        vPort->MarkToRedraw();
        hAppMenu->SyncDocumentSwitched();
        hNativeController->SyncDocumentSwitched();
        hDataCtrl = NULL;

        for (cWindow* win : hWindows) {
            win->Close();
        }

        winTileProp->setVisible(false);
        FixInterfacePositions();
        iTileSelectX1 = iTileSelectY1 = iTileSelectX2 = iTileSelectY2 = -1;
        return;
    }

    iTileSelectX1 = MDI->GetActiveDoc()->iTileSelectX1;
    iTileSelectY1 = MDI->GetActiveDoc()->iTileSelectY1;
    iTileSelectX2 = MDI->GetActiveDoc()->iTileSelectX2;
    iTileSelectY2 = MDI->GetActiveDoc()->iTileSelectY2;

    cbutActiveMode->setVisible(true);

    if (iActiveTool == EWW_TOOL_BRUSH || iActiveTool == EWW_TOOL_FILL || iActiveTool == EWW_TOOL_PENCIL) {
        if (iActiveTool == EWW_TOOL_BRUSH && MDI->GetActiveDoc() != NULL && iTilePicked != EWW_TILE_NONE)
            HandleBrushSwitch(iTilePicked, EWW_TILE_NONE);
        iTilePicked = EWW_TILE_NONE;
    }

    hParser = MDI->GetActiveDoc()->hParser;
    SprBank = MDI->GetActiveDoc()->hSprBank;
    hTileset = MDI->GetActiveDoc()->hTilesBank;
    hSndBank = MDI->GetActiveDoc()->hSndBank;
    hAniBank = MDI->GetActiveDoc()->hAniBank;
    hCustomLogics = MDI->GetActiveDoc()->hCustomLogicBank;
    hDataCtrl = MDI->GetActiveDoc()->hDataCtrl;
    hPlaneData = MDI->GetActiveDoc()->hPlaneData;
    fZoom = MDI->GetActiveDoc()->fZoom;
    fDestZoom = MDI->GetActiveDoc()->fZoom;

    lbbrlLogicList->setListModel(hCustomLogics);
    if (winLogicBrowser->isVisible()) {
        SyncLogicBrowser();
    }

    float fZ = 0;
    if (fZoom > 1) {
        fZ += (fZoom - 1.0) * 5.0f;
    } else if (fZoom < 1.0f) {
        fZ -= ((1.0f - fZoom) / 0.9f) * 5.0f;
    }
    sliZoom->setValue(fZ);

    for (int i = 0; i < hParser->GetPlanesCount(); i++)
        if (hParser->GetPlane(i)->GetFlags() & WWD::Flag_p_MainPlane) {
            plMain = hParser->GetPlane(i);
            break;
        }

    hStartingPosObj = MDI->GetActiveDoc()->hStartingPosObj;

    hmbTile->ddActivePlane->setSelected(MDI->GetActiveDoc()->iSelectedPlane);

    vObjectsPicked = MDI->GetActiveDoc()->vObjectsPicked;
    if (MDI->GetActiveDoc()->fCamX != -1 && MDI->GetActiveDoc()->fCamY != -1) {
        fCamX = MDI->GetActiveDoc()->fCamX;
        fCamY = MDI->GetActiveDoc()->fCamY;
    } else {
        fCamX = hParser->GetStartX() - vPort->GetWidth() / 2 * fZoom;
        fCamY = hParser->GetStartY() - vPort->GetHeight() / 2 * fZoom;
    }
    vPort->MarkToRedraw();

    hmbTile->DocumentSwitched();
    hmbObject->DocumentSwitched();

    SyncWorldOptionsWithParser();
    lbpmPlanes->setSelected(-1);
    SyncPlaneProperties();

    char tmp[128];
    sprintf(tmp, GETL(Lang_MapShotDimensions),
            int(hParser->GetPlane(ddmsPlane->getSelected())->GetPlaneWidthPx() * (slimsScale->getValue() / 100)),
            int(hParser->GetPlane(ddmsPlane->getSelected())->GetPlaneHeightPx() * (slimsScale->getValue() / 100)));
    labmsDimensions->setCaption(tmp);
    labmsDimensions->adjustSize();

    char *filename = 0;
    if (strlen(hParser->GetFilePath()) > 0)
        filename = SHR::GetFile(hParser->GetFilePath());
    else {
        const char *str = GETL2S("NewMap", "NewDoc");
        filename = new char[strlen(str) + 1];
        strcpy(filename, str);
    }
    if (MDI->GetActiveDoc()->bSaved)
        sprintf(tmp, "%s - %s", filename, WA_TITLEBAR);
    else
        sprintf(tmp, "%s* - %s*", filename, WA_TITLEBAR);
    hge->System_SetState(HGE_TITLE, tmp);

    itpSelectedTile = -1;
    auto tileSet = hTileset->GetSet(hParser->GetMainPlane()->GetImageSet(0));
    if (tileSet && tileSet->GetTilesCount() > 0)
        itpSelectedTile = tileSet->GetTileByIterator(0)->GetID();
    sprintf(tmp, "%d", itpSelectedTile);
    tftpTileID->setText(tmp);
    SyncAttribMenuWithTile();

    UpdateSearchResults();

    char *path, *filewithoutex;
    if (strlen(hParser->GetFilePath()) > 0) {
        path = SHR::GetDir(hParser->GetFilePath());
    } else {
        char exe_name[MAX_PATH + 1];
        exe_name[MAX_PATH] = '\0';
        ::GetModuleFileName(NULL, exe_name, MAX_PATH);
        path = SHR::GetDir(exe_name);
    }
    filewithoutex = SHR::GetFileWithoutExt(filename);
    char pathtemp[512];
    sprintf(pathtemp, "%s\\%s.jpg", path, filewithoutex);
    delete[] path;
    delete[] filename;
    delete[] filewithoutex;
    tfmsSaveAs->setText(pathtemp, true);

    hInvCtrl->MapSwitch();
    SwitchPlane();
    hAppMenu->SyncDocumentSwitched();
    hNativeController->SyncDocumentSwitched(hParser);

    for (cWindow* win : hWindows) {
        win->OnDocumentChange();
    }

    if (hmbActive == 0) {
        cbutActiveMode->simulateAction();
        //SwitchActiveModeMenuBar(cbutActiveMode->getSelectedEntryID() ? (cModeMenuBar*)hmbObject : (cModeMenuBar*)hmbTile);
    }
    FixInterfacePositions();

    slitpiPicker->setValue(0);
}

void State::EditingWW::SetZoom(float fZ) {
    if (GV->bSmoothZoom) {
        fDestZoom = fZ;
    } else {
        fDestZoom = fZoom = fZ;
    }
}

void State::EditingWW::FileDraggedIn() {
    draggedFilesIn = true;

    int i = 0;
    auto& files = hge->System_GetDraggedFiles();
    for (auto it = files.begin(); it != files.end();) {
        const char* dot = strrchr(it->c_str(), '.');
        if (!dot) {
            it = files.erase(it);
            continue;
        }
        char* ext = SHR::ToLower(dot + 1);
        if (strcmp(ext, "wwd") != 0) {
            it = files.erase(it);
        } else {
            if (i < 10) {
                int base = 0;
                try {
                    iDraggedFileIcon[i] = WWD::GetGameTypeFromFile(it->c_str(), &base);
                }
                catch (...) {
                    iDraggedFileIcon[i] = WWD::Game_Unknown;
                }
                if (base > 0 && base < 15 && iDraggedFileIcon[i] == WWD::Game_Claw) {
                    iDraggedFileIcon[i] = 50 + base;
                } else if (base > 0 && base < 9 && iDraggedFileIcon[i] == WWD::Game_Gruntz) {
                    iDraggedFileIcon[i] = 150 + base;
                }
            }

            ++it;
            ++i;
        }
        delete[] ext;
    }
}

void State::EditingWW::FileDraggedOut() {
    draggedFilesIn = false;
}

void State::EditingWW::FileDropped() {
    for (auto filepath : hge->System_GetDraggedFiles()) {
        vstrMapsToLoad.push_back(filepath.c_str());
    }
    draggedFilesIn = false;
}

void State::EditingWW::ApplicationStartup() {
    if (GV->szCmdLine[0] != '\0') {
        if (GV->szCmdLine[0] == '"') {
            int str_length = GV->szCmdLine.length() - 1;
            char *tmp = new char[str_length];
            for (int i = 1; i < str_length; i++)
                tmp[i - 1] = GV->szCmdLine[i];
            tmp[str_length - 1] = '\0';
            GV->editState->vstrMapsToLoad.emplace_back(tmp);
        } else if (GV->szCmdLine[0] == '-') {
            if (!GV->szCmdLine.compare("-clearBrush")) {
                GV->Console->Print("Cleaning brushes dir...");
                HANDLE hFind = INVALID_HANDLE_VALUE;
                WIN32_FIND_DATA fdata;
                hFind = FindFirstFile("brush/*", &fdata);
                if (hFind != INVALID_HANDLE_VALUE) {
                    do {
                        if (fdata.cFileName[0] != '.') {
                            char *ext = SHR::GetExtension(fdata.cFileName);
                            char *extl = SHR::ToLower(ext);
                            delete[] ext;
                            if (!strcmp(extl, "lua")) {
                                char *path = new char[7 + strlen(fdata.cFileName)];
                                sprintf(path, "brush/%s", fdata.cFileName);
                                unlink(path);
                                delete[] path;
                                GV->Console->Printf("- Deleted ~y~%s~w~", fdata.cFileName);
                            }
                            delete[] extl;
                        }
                    } while (FindNextFile(hFind, &fdata) != 0);
                }
            }
        } else {
            GV->editState->vstrMapsToLoad.emplace_back(GV->szCmdLine);
        }
    }
}

void State::EditingWW::SyncWorldOptionsWithParser() {
    tfwpName->setText(hParser->GetName());
    tfwpAuthor->setText(hParser->GetAuthor());
    tfwpDate->setText(hParser->GetDate());
    tfwpREZ->setText(hParser->GetRezPath());
    tfwpTiles->setText(hParser->GetTilesPath());
    tfwpPalette->setText(hParser->GetPalettePath());
    tfwpExe->setText(hParser->GetExePath());

    /*char tmp[256];
    sprintf(tmp, "%d", MDI->GetActiveDoc()->iMapBuild);
    tfwpMapBuild->setText(tmp);

    if (MDI->GetActiveDoc()->iWapMapBuild <= 0) {
        sprintf(tmp, "%s", "WapWorld");
    } else {
        sprintf(tmp, "WapMap v%s (b%d)", MDI->GetActiveDoc()->strWapMapVersion.c_str(),
                MDI->GetActiveDoc()->iWapMapBuild);
    }
    tfwpWapVersion->setText(tmp);

    tfwpMapVersion->setText(MDI->GetActiveDoc()->strMapVersion);
    tbwpMapDescription->setText(MDI->GetActiveDoc()->strMapDescription);*/

    cbwpCompress->setSelected(hParser->GetFlag(WWD::Flag_w_Compress));
    cbwpZCoord->setSelected(hParser->GetFlag(WWD::Flag_w_UseZCoords));
}

void State::EditingWW::SaveWorldOptions() {
    if (!strlen(tfwpName->getText().c_str())) {
        GV->StateMgr->Push(
                new State::Dialog(PRODUCT_NAME, GETL2S("MapProperties", "NoName"), ST_DIALOG_ICON_ERROR, ST_DIALOG_BUT_OK));
        return;
    } else if (strlen(tfwpName->getText().c_str()) > 64) {
        GV->StateMgr->Push(new State::Dialog(PRODUCT_NAME, GETL2S("MapProperties", "TooLongName"), ST_DIALOG_ICON_ERROR,
                                             ST_DIALOG_BUT_OK));
        return;
    }
    if (!strlen(tfwpAuthor->getText().c_str())) {
        GV->StateMgr->Push(
                new State::Dialog(PRODUCT_NAME, GETL2S("MapProperties", "NoAuthor"), ST_DIALOG_ICON_ERROR, ST_DIALOG_BUT_OK));
        return;
    } else if (strlen(tfwpAuthor->getText().c_str()) > 64) {
        GV->StateMgr->Push(
                new State::Dialog(PRODUCT_NAME, GETL2S("MapProperties", "TooLongAuthor"), ST_DIALOG_ICON_ERROR,
                                  ST_DIALOG_BUT_OK));
        return;
    }
    if (!strlen(tfwpREZ->getText().c_str())) {
        GV->StateMgr->Push(
                new State::Dialog(PRODUCT_NAME, GETL2S("MapProperties", "NoRez"), ST_DIALOG_ICON_ERROR, ST_DIALOG_BUT_OK));
        return;
    } else if (strlen(tfwpREZ->getText().c_str()) > 256) {
        GV->StateMgr->Push(new State::Dialog(PRODUCT_NAME, GETL2S("MapProperties", "TooLongRez"), ST_DIALOG_ICON_ERROR,
                                             ST_DIALOG_BUT_OK));
        return;
    }
    if (!strlen(tfwpTiles->getText().c_str())) {
        GV->StateMgr->Push(
                new State::Dialog(PRODUCT_NAME, GETL2S("MapProperties", "NoTiles"), ST_DIALOG_ICON_ERROR, ST_DIALOG_BUT_OK));
        return;
    } else if (strlen(tfwpTiles->getText().c_str()) > 128) {
        GV->StateMgr->Push(new State::Dialog(PRODUCT_NAME, GETL2S("MapProperties", "TooLongTiles"), ST_DIALOG_ICON_ERROR,
                                             ST_DIALOG_BUT_OK));
        return;
    }
    if (!strlen(tfwpPalette->getText().c_str())) {
        GV->StateMgr->Push(
                new State::Dialog(PRODUCT_NAME, GETL2S("MapProperties", "NoPalette"), ST_DIALOG_ICON_ERROR, ST_DIALOG_BUT_OK));
        return;
    } else if (strlen(tfwpPalette->getText().c_str()) > 128) {
        GV->StateMgr->Push(
                new State::Dialog(PRODUCT_NAME, GETL2S("MapProperties", "TooLongPalette"), ST_DIALOG_ICON_ERROR,
                                  ST_DIALOG_BUT_OK));
        return;
    }
    if (!strlen(tfwpExe->getText().c_str())) {
        GV->StateMgr->Push(
                new State::Dialog(PRODUCT_NAME, GETL2S("MapProperties", "NoExe"), ST_DIALOG_ICON_ERROR, ST_DIALOG_BUT_OK));
        return;
    } else if (strlen(tfwpExe->getText().c_str()) > 128) {
        GV->StateMgr->Push(new State::Dialog(PRODUCT_NAME, GETL2S("MapProperties", "TooLongExe"), ST_DIALOG_ICON_ERROR,
                                             ST_DIALOG_BUT_OK));
        return;
    }
    winWorld->setVisible(false);
    bool bchange = false;
    if (strcmp(hParser->GetName(), tfwpName->getText().c_str()) != 0) {
        bchange = true;
        char *fixname = FixLevelName(hParser->GetBaseLevel(), tfwpName->getText().c_str());
        hParser->SetName(fixname);
        tfwpName->setText(fixname);
        delete[] fixname;
    }
    if (strcmp(hParser->GetAuthor(), tfwpAuthor->getText().c_str()) != 0) {
        bchange = 1;
        hParser->SetAuthor(tfwpAuthor->getText().c_str());
    }
    if (strcmp(hParser->GetRezPath(), tfwpREZ->getText().c_str()) != 0) {
        bchange = 1;
        hParser->SetRezPath(tfwpREZ->getText().c_str());
    }
    if (strcmp(hParser->GetTilesPath(), tfwpTiles->getText().c_str()) != 0) {
        bchange = 1;
        hParser->SetTilesPath(tfwpTiles->getText().c_str());
    }
    if (strcmp(hParser->GetPalettePath(), tfwpPalette->getText().c_str()) != 0) {
        bchange = 1;
        hParser->SetPalettePath(tfwpPalette->getText().c_str());
    }
    if (strcmp(hParser->GetExePath(), tfwpExe->getText().c_str()) != 0) {
        bchange = 1;
        hParser->SetExePath(tfwpExe->getText().c_str());
    }
    if (hParser->GetFlag(WWD::Flag_w_Compress) != cbwpCompress->isSelected()) {
        bchange = 1;
        hParser->SetFlag(WWD::Flag_w_Compress, cbwpCompress->isSelected());
    }
    if (hParser->GetFlag(WWD::Flag_w_UseZCoords) != cbwpZCoord->isSelected()) {
        bchange = 1;
        hParser->SetFlag(WWD::Flag_w_UseZCoords, cbwpZCoord->isSelected());
    }

    /*if (MDI->GetActiveDoc()->strMapDescription.compare(tbwpMapDescription->getText()) != 0) {
        bchange = 1;
        MDI->GetActiveDoc()->strMapDescription = tbwpMapDescription->getText();
    }
    if (MDI->GetActiveDoc()->strMapVersion.compare(tfwpMapVersion->getText()) != 0) {
        bchange = 1;
        MDI->GetActiveDoc()->strMapVersion = tfwpMapVersion->getText();
    }
    if (MDI->GetActiveDoc()->iMapBuild != atoi(tfwpMapBuild->getText().c_str())) {
        bchange = 1;
        MDI->GetActiveDoc()->iMapBuild = atoi(tfwpMapBuild->getText().c_str());
    }*/
    if (bchange)
        MarkUnsaved();
}

void State::EditingWW::UpdateSettings() {
    //cboptShowWelcomeScreen->setSelected(GV->bShowWelcomeScreen);
}

void State::EditingWW::MruListUpdated() {
    GV->Console->Printf("Reloading recent files...");
    for (int i = 0; i < 10; i++) {
        if (lnkLastOpened[i] != 0) {
            delete lnkLastOpened[i];
            lnkLastOpened[i] = 0;
        }
    }

    labLoadLastOpened->setVisible(hMruList->IsValid());

    if (hMruList->IsValid()) {
        for (int i = 0; i < hMruList->GetFilesCount(); i++) {
            std::string tmp(hMruList->GetRecentlyUsedFile(i));
            if (GV->fntMyriad16->GetStringWidth(tmp.c_str()) > 400) {
                do {
                    tmp.erase(0, 5);
                } while (GV->fntMyriad16->GetStringWidth(tmp.c_str()) > 390);
                tmp.insert(0, "(...)");
            }

            WWD::GAME gt;
            int base = 0;
            try {
                gt = WWD::GetGameTypeFromFile(hMruList->GetRecentlyUsedFile(i), &base);
            }
            catch (...) {
                gt = WWD::Game_Unknown;
            }

            hgeSprite *ico = GV->sprGamesSmall[gt];
            gamesLastOpened[i] = gt;
            if (gt == WWD::Game_Claw && base > 0 && base < 15) {
                gamesLastOpened[i] = 50 + base;
                ico = GV->sprLevelsMicro16[gt - WWD::Games_First][base - 1];
            } else if (gt == WWD::Game_Gruntz && base > 0 && base < 9) {
                gamesLastOpened[i] = 150 + base;
                ico = GV->sprLevelsMicro16[gt - WWD::Games_First][base - 1];
            }

            lnkLastOpened[i] = new SHR::Link(tmp, ico);
            lnkLastOpened[i]->adjustSize();
            lnkLastOpened[i]->setEnabled(gt != WWD::Game_Unknown);
            lnkLastOpened[i]->addActionListener(mainListener);
            conRecentFiles->add(lnkLastOpened[i], 0, 25 + i * 25);
        }
    }
    GV->Console->Printf("Reloading main context...");
    hAppMenu->SyncMRU();
    GV->Console->Printf("Done.");
}

bool State::EditingWW::ValidateLevelName(const char *name, bool bAllowNoNum) {
    bool bFoundNum = false;
    for (int i = 0; i < strlen(name); i++) {
        if (name[i] >= 48 && name[i] <= 57) {
            if (bFoundNum) return 0;
            bFoundNum = 1;
            int v = name[i] - 48;
            if (i + 1 < strlen(name) && name[i + 1] >= 48 && name[i + 1] <= 57) {
                v *= 10;
                v += name[i + 1] - 48;
                i++;
            }
            if (v < 0 || v > 14) return 0;
        }
    }
    return bAllowNoNum ? 1 : bFoundNum;
}

char *State::EditingWW::FixLevelName(int iBaseLvl, const char *name) {
    if (ValidateLevelName(name, false)) {
        char *r = new char[strlen(name) + 1];
        strcpy(r, name);
        return r;
    }
    char *t = new char[strlen(name) + 1 + 6];
    int offset = 0;
    for (int i = 0; i <= strlen(name); i++) {
        if (name[i] >= 48 && name[i] <= 57) {
            offset++;
            continue;
        }
        t[i - offset] = name[i];
    }
    char *r = new char[strlen(t) + 6];
    sprintf(r, "%s (%d)", t, iBaseLvl);
    delete[] t;
    return r;
}

void State::EditingWW::AppFocus(bool bGain) {
    bWindowFocused = bGain;
}

void State::EditingWW::TextEditMoveToNextTile(bool saving) {
    WWD::Tile* tile = GetActivePlane()->GetTile(iTileWriteIDx, iTileWriteIDy);
    if (!tile) return;
    if (saving) {
        const char* text = tfWriteID->getText().c_str();
        if (text[0] == 'f' || text[0] == 'F') {
            if (!tile->IsFilled()) {
                tile->SetFilled(true);
                MarkUnsaved();
            }
        } else {
            int tid = atoi(text);
            if (tid == 0) {
                if (!tile->IsInvisible()) {
                    tile->SetInvisible(true);
                    MarkUnsaved();
                }
            } else if (tile->GetID() != tid) {
                tile->SetID(tid);
                MarkUnsaved();
            }
        }
    }
    ++tile;
    if (++iTileWriteIDx == GetActivePlane()->GetPlaneWidth()) {
        fCamX = 0;
        iTileWriteIDx = 0;
        if (iTileWriteIDy == GetActivePlane()->GetPlaneHeight()) {
            iTileWriteIDy = 0;
            tile = GetActivePlane()->GetTile(0);
            fCamY = 0;
        } else {
            ++iTileWriteIDy;
            fCamY += GetActivePlane()->GetTileHeight() / fZoom;
        }
    } else {
        fCamX += GetActivePlane()->GetTileWidth() / fZoom;
    }

    if (tile->IsFilled()) {
        tfWriteID->setText("FILL");
    } else if (tile->IsInvisible()) {
        tfWriteID->setText("0");
    } else {
        tfWriteID->setText(std::to_string(tile->GetID()));
    }

    tfWriteID->setSelectionPosition(0);
    tfWriteID->setCaretPosition(tfWriteID->getText().length());
    tfWriteID->requestFocus();
    vPort->MarkToRedraw();
}

void State::EditingWW::showObjectModeContextMenu(MouseEvent& mouseEvent) {
    contextX = mouseEvent.getX() + vPort->GetX();
    contextY = mouseEvent.getY() + vPort->GetY();

    if (vObjectsPicked.empty() && vObjectsHL.empty()) {
        vObjectsHL.clear();
        vPort->MarkToRedraw();

        bool canTestFromPos = hNativeController->IsValid() &&
                              hNativeController->IsCrazyHookAvailable() &&
                              strlen(hParser->GetFilePath()) > 0;
        if (vObjectClipboard.empty()) {
            conmodAtEmpty->GetElementByID(OBJMENU_TESTFROMHERE)->SetEnabled(canTestFromPos);
            objContext->SetModel(conmodAtEmpty);
        } else {
            char ncap[256];
            if (vObjectClipboard.size() == 1)
                sprintf(ncap, "%s: ~y~%s~l~", GETL(Lang_Paste), vObjectClipboard[0]->GetLogic());
            else
                sprintf(ncap, "%s: ~y~%s~l~", GETL(Lang_Paste), GETL(Lang_ManyObjects));
            conmodAtEmptyPaste->GetElementByID(OBJMENU_PASTE)->SetCaption(ncap);
            conmodAtEmptyPaste->GetElementByID(OBJMENU_TESTFROMHERE)->SetEnabled(canTestFromPos);

            objContext->SetModel(conmodAtEmptyPaste);
        }
    } else {
        int x = Scr2WrdX(GetActivePlane(), contextX),
                y = Scr2WrdY(GetActivePlane(), contextY);

        bool any = false;

        for (auto obj : vObjectsPicked) {
            WWD::Rect box = SprBank->GetObjectRenderRect(obj);
            if (x > box.x1 &&
                x < box.x1 + box.x2 &&
                y > box.y1 &&
                y < box.y1 + box.y2) {
                any = true;
                break;
            }
        }

        if (!any) {
            vObjectsPicked = vObjectsHL;
            if (vObjectsPicked.empty()) {
                mousePressed(mouseEvent);
                return;
            }
        }

        vObjectsHL.clear();
        vPort->MarkToRedraw();

        if (vObjectsPicked.size() == 1) {
            if (vObjectsPicked[0] == hStartingPosObj) {
                bool canTestFromPos = hNativeController->IsValid() &&
                                      hNativeController->IsCrazyHookAvailable() &&
                                      strlen(hParser->GetFilePath()) > 0;
                conmodSpawnPoint->GetElementByID(OBJMENU_TESTFROMHERE)->SetEnabled(canTestFromPos);
                objContext->SetModel(conmodSpawnPoint);
            } else {
                auto *con = (SHR::Context *) 1;
                bool spec = AreObjectSpecificOptionsAvailable(vObjectsPicked[0], &con);
                if (IsEditableObject(vObjectsPicked[0]))
                    objContext->SetModel(conmodEditableObject);
                else
                    objContext->SetModel(conmodObject);
                if (!strcmp(vObjectsPicked[0]->GetLogic(), "CustomLogic") &&
                    hCustomLogics->GetLogicByName(vObjectsPicked[0]->GetName()) != 0) {
                    objContext->AddElement(OBJMENU_EDITLOGIC, GETL2S("Various", "ContextEditLogic"),
                                           GV->sprIcons16[Icon16_Code],
                                           objContext->GetElementByID(OBJMENU_PROPERTIES));
                }
                if (spec) {
                    objContext->AddElement(OBJMENU_SPECIFICPROP, GETL2S("Various", "ContextAdv"),
                                           GV->sprIcons16[Icon16_PropertyTree],
                                           objContext->GetElementByID(OBJMENU_PROPERTIES));
                    objContext->GetElementByID(OBJMENU_SPECIFICPROP)->SetCascade(con);
                }
            }
        } else {
            objContext->SetModel(conmodObjectMultiple);
        }

        for (int flag = 0; flag < 8; flag++) {
            SHR::Context *hmyContext = objFlagDrawContext;
            int flagtype = 0, flagpos = flag, menupos = OBJMENU_FLAGS_DRAW + flag + 1;
            if (flag > 3 && flag < 8) {
                flagtype = 1;
                flagpos -= 4;
                menupos = OBJMENU_FLAGS_DYNAMIC + flagpos + 1;
                hmyContext = objFlagDynamicContext;
            } //else if (flag > 7) {
            //flagtype = 2;
            //flagpos -= 8;
            //menupos = OBJMENU_FLAGS_ADDITIONAL + flagpos + 1;
            //hmyContext = objFlagAddContext;
            //}
            int binaryValue = pow(2, flagpos);
            bool valueSet = false;
            unsigned char fValue = 0;

            for (auto &picked : vObjectsPicked) {
                if (picked == hStartingPosObj) continue;
                int flags;
                if (flagtype == 0) flags = int(picked->GetDrawFlags());
                else if (flagtype == 1) flags = int(picked->GetDynamicFlags());
                else if (flagtype == 2) flags = int(picked->GetAddFlags());
                bool flagsNewValue = (flags & binaryValue);
                if (!valueSet) {
                    fValue = flagsNewValue;
                    valueSet = true;
                } else {
                    if (flagsNewValue != fValue)
                        fValue = 2;
                }
            }

            hgeSprite *ico = 0;
            if (fValue == 1) ico = GV->sprIcons16[Icon16_Applied];
            else if (fValue == 2) ico = GV->sprIcons16[Icon16_AppliedPartially];

            hmyContext->GetElementByID(menupos)->SetIcon(ico, 0);
        }
    }

    objContext->adjustSize();
    objContext->setPosition(contextX, contextY);
    if (contextX + objContext->getWidth() > hge->System_GetState(HGE_SCREENWIDTH))
        objContext->setX(contextX - objContext->getWidth());
    if (contextY + objContext->getHeight() > hge->System_GetState(HGE_SCREENHEIGHT))
        objContext->setY(contextY - objContext->getHeight());
    objContext->setVisible(true);
    objContext->requestFocus();
}

void State::EditingWW::UpdateSelectAreaWindowButtons(SHR::But *source) {
    buttoolSelAreaAll->setEnabled(source != buttoolSelAreaAll);
    buttoolSelAreaPickMinX->setEnabled(source != buttoolSelAreaPickMinX);
    buttoolSelAreaPickMinY->setEnabled(source != buttoolSelAreaPickMinY);
    buttoolSelAreaPickMaxX->setEnabled(source != buttoolSelAreaPickMaxX);
    buttoolSelAreaPickMaxY->setEnabled(source != buttoolSelAreaPickMaxY);
}

void State::EditingWW::PutToBottomRight(SHR::Win *window) {
    window->setPosition(
        vPort->GetX() + vPort->GetWidth() - window->getWidth() - 10,
        vPort->GetY() + vPort->GetHeight() - window->getHeight() - 10
    );
}
