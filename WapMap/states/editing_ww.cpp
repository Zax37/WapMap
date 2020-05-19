#include "editing_ww.h"
#include "loadmap.h"
#include "../globals.h"
#include "../version.h"
#include "../../shared/commonFunc.h"
#include "../langID.h"
#include "../returncodes.h"
#include "../cObjectUserData.h"
#include "error.h"
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
}

State::EditingWW::EditingWW(WWD::Parser *phParser) {
    hParser = phParser;
    GV->editState = this;
}

bool State::EditingWW::Opaque() {
    return 1;
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

    winLogicBrowser->setDimension(gcn::Rectangle(0, 0, 800, 221));
    winLogicBrowser->setPosition(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 400,
                                 hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 120);
    winLogicBrowser->add(labbrlLogicName, 305, yOffset);
    winLogicBrowser->add(labbrlFilePath, 305, yOffset + 25);
    winLogicBrowser->add(labbrlFileSize, 305, yOffset + 50);
    winLogicBrowser->add(labbrlFileChecksum, 305, yOffset + 75);
    winLogicBrowser->add(labbrlFileModDate, 305, yOffset + 100);
    //winLogicBrowser->add(tfbrlRename, 305 + width, 19);
    //winLogicBrowser->add(butbrlRenameOK, 305 + width + 5 + 200, 15);
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

void State::EditingWW::Init() {
    if (GV->bAutoUpdate) {
        hAU = new cAutoUpdater();
    }

    bWindowFocused = 1;
    bShowHand = 0;
    iManipulatedGuide = -1;
    bShowGuideLines = 1;
    dwCursorColor = 0xFFFFFFFF;
    bEditObjDelete = 0;
    bDragWindow = 0;
    fAppBarTimers[0] = fAppBarTimers[1] = 0.0f;
    fCameraMoveTimer = 0;
#ifdef BUILD_DEBUG
    bShowConsole = 1;
#endif
    iTileSelectX1 = iTileSelectX2 = iTileSelectY1 = iTileSelectY2 = -1;
    bObjDragSelection = 0;
    iObjDragOrigX = iObjDragOrigY = 0;
    fDoubleClickTimer = -1;
    szObjSearchBuffer = NULL;
    btpDragDropMask = 0;
    hTempAttrib = nullptr;
    fObjContextX = fObjContextY = 0;
    lastbrushx = lastbrushy = 0;
    szHint[0] = '\0';
    fHintTime = -1;

    bForceObjectClipbPreview = bForceTileClipbPreview = 0;

    bDrawTileProperties = 0;
    bLockScroll = 0;

    fObjPickLastMx = fObjPickLastMy = 0;
    bConstRedraw = 0;
    bExit = 0;

    iDoubleClickX = iDoubleClickY = -1;

    bOpenObjContext = 0;

    iMode = EWW_MODE_TILE;
    iActiveTool = EWW_TOOL_NONE;
    //iObjectPicked = iObjectHL = -1;
    fZoom = 1.0f;
    fDestZoom = 1.0f;
    fLastZoom = fZoom;

    iTilePreviewX = iTilePreviewY = -1;

    fCamLastX = fCamLastY = 0;

    iTilePicked = EWW_TILE_NONE;

    NewMap_data = 0;
    FirstRun_data = 0;

    GV->Console->Print("~w~Creating viewport...");

    gui = new gcn::Gui();
    gui->setGraphics(GV->gcnGraphics);
    gui->setInput(GV->gcnInput);

    conMain = new SHR::Container();
    conMain->setDimension(
            gcn::Rectangle(0, 0, hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT)));

    vPort = new Viewport(this, 0, 50 + 24, hge->System_GetState(HGE_SCREENWIDTH) - 17,
                         hge->System_GetState(HGE_SCREENHEIGHT) - 31 - 25 - 24 - 25 - 11);
    conMain->add(vPort->GetWidget(), vPort->GetX(), vPort->GetY());

    al = new EditingWWActionListener(this);
    fl = new EditingWWFocusListener(this);
    kl = new EditingWWKeyListener(this);
    vp = new EditingWWvpCallback(this);

    gui->addGlobalKeyListener(kl);

    MDI = new cMDI();

    hAppMenu = new cAppMenu();

    vpMain = new WIDG::Viewport(vp, VP_VIEWPORT);
    conMain->add(vpMain, 0, 0);

    conWriteID = new SHR::Container();
    conWriteID->setDimension(gcn::Rectangle(0, 0, 48, 20));
    conWriteID->setVisible(0);
    conWriteID->setOpaque(0);
    conMain->add(conWriteID, 200, 200);

    tfWriteID = new SHR::TextField();
    tfWriteID->setDimension(gcn::Rectangle(0, 0, 48, 20));
    tfWriteID->addActionListener(al);
    tfWriteID->addFocusListener(fl);
    conWriteID->add(tfWriteID, 0, 0);


    sliVer = new SHR::Slider(100);
    sliVer->setOrientation(SHR::Slider::VERTICAL);
    sliVer->setDimension(gcn::Rectangle(0, 0, 16, hge->System_GetState(HGE_SCREENHEIGHT) - 111 - 24 - 5));
    sliVer->setScaleStart(-40);
    sliVer->addActionListener(al);
    sliVer->setMarkerLength(40);
    conMain->add(sliVer, hge->System_GetState(HGE_SCREENWIDTH) - 19, 80 + 24);

    hAppMenu->FixInterfacePositions();

    sliHor = new SHR::Slider(100);
    sliHor->setOrientation(SHR::Slider::HORIZONTAL);
    sliHor->setDimension(gcn::Rectangle(0, 0, hge->System_GetState(HGE_SCREENWIDTH) - 19, 16));
    sliHor->setScaleStart(-40);
    sliHor->addActionListener(al);
    sliHor->setMarkerLength(40);
    conMain->add(sliHor, 0, hge->System_GetState(HGE_SCREENHEIGHT) - 30 - 17);

    butMicroTileCB = new SHR::But(GV->hGfxInterface, GV->sprMicroIcons[0]);
    butMicroTileCB->setDimension(gcn::Rectangle(0, 0, 18, 18));
    butMicroTileCB->addActionListener(al);
    butMicroTileCB->SetTooltip(GETL2S("ClipboardPreview", "TileClipboard"));
    conMain->add(butMicroTileCB, 2, hge->System_GetState(HGE_SCREENHEIGHT) - 25);

    butMicroObjectCB = new SHR::But(GV->hGfxInterface, GV->sprMicroIcons[1]);
    butMicroObjectCB->setDimension(gcn::Rectangle(0, 0, 18, 18));
    butMicroObjectCB->addActionListener(al);
    butMicroObjectCB->SetTooltip(GETL2S("ClipboardPreview", "ObjectClipboard"));
    conMain->add(butMicroObjectCB, 20, hge->System_GetState(HGE_SCREENHEIGHT) - 25);

    sliZoom = new SHR::Slider(-5, 5);
    sliZoom->setDimension(gcn::Rectangle(0, 0, 150, 20));
    sliZoom->setStyle(SHR::Slider::POINTER);
    sliZoom->addActionListener(al);
    for (int i = -5; i <= 5; i++)
        sliZoom->addKeyValue(i);
    conMain->add(sliZoom, hge->System_GetState(HGE_SCREENWIDTH) - 160, hge->System_GetState(HGE_SCREENHEIGHT) - 25);

    int yoff = 24 + 23;

    int xoff = 0;
#ifdef WM_ADD_LUA_EXECUTER
    butIconLua = MakeButton(555, 5 + yoff, Icon_Lua, conMain);
	butIconLua->SetTooltip(GETL2("Tooltip", Lang_TT_Lua));
#else
    xoff -= 32;
#endif

    //butIconShot->SetTooltip(GETL2("Tooltip", Lang_TT_Mapshot));

    cbutActiveMode = new SHR::ComboBut(GV->hGfxInterface);
    cbutActiveMode->addEntry(SHR::ComboButEntry(GV->sprIcons16[Icon16_ModeTile], GETL(Lang_ModeTile)));
    cbutActiveMode->addEntry(SHR::ComboButEntry(GV->sprIcons16[Icon16_ModeObject], GETL(Lang_ModeObject)));
    cbutActiveMode->addActionListener(al);
    cbutActiveMode->adjustSize();
    conMain->add(cbutActiveMode, 5, LAY_MODEBAR_Y + 4);

    int modeMenuStartX = 10 + cbutActiveMode->getDimension().width;

    hmbTile = new cmmbTile(modeMenuStartX);
    hmbTile->SetVisible(0);
    hmbObject = new cmmbObject(modeMenuStartX);
    hmbObject->SetVisible(0);
    hmbActive = NULL;

    SetIconBarVisible(0);

    conResizeLeft = new SHR::Container();
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
    butExtLayerUR = MakeButton(vPort->GetWidth() - 36, 4, Icon_AddUpRight, conResizeUp);
    butExtLayerDL = MakeButton(4, 4, Icon_AddDownLeft, conResizeDown);
    butExtLayerDR = MakeButton(vPort->GetWidth() - 36, 4, Icon_AddDownRight, conResizeDown);
    butExtLayerUp = MakeButton(vPort->GetWidth() / 2, 4, Icon_AddUp, conResizeUp);
    butExtLayerDown = MakeButton(vPort->GetWidth() / 2, 4, Icon_AddDown, conResizeDown);
    butExtLayerLeft = MakeButton(4, vPort->GetHeight() / 2 - 40 - 16, Icon_AddLeft, conResizeLeft);
    butExtLayerRight = MakeButton(4, vPort->GetHeight() / 2 - 40 - 16, Icon_AddRight, conResizeRight);

    hNativeController = new cNativeController();
    hNativeController->SetPath(GV->gamePaths[WWD::Game_Claw]);

    {
        int w = atoi(GV->ini->GetValue("CrazyHook", "DisplayWidth", "1024")),
            h = atoi(GV->ini->GetValue("CrazyHook", "DisplayHeight", "768"));
        if (w == -1 && h == -1) {

        } else {
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

    hwinOptions = new winOptions();
    hwinTileBrowser = new winTileBrowser();
    hwinImageSetBrowser = new winImageSetBrowser();
    hwinAbout = new winAbout();
    hWindows.push_back(hwinOptions);
    hWindows.push_back(hwinTileBrowser);
    hWindows.push_back(hwinImageSetBrowser);
    hWindows.push_back(hwinAbout);

    for (size_t i = 0; i < hWindows.size(); i++) {
        SHR::Win *win = hWindows[i]->GetWindow();
        if (win != 0) {
            conMain->add(win, hge->System_GetState(HGE_SCREENWIDTH) / 2 - win->getWidth() / 2,
                         hge->System_GetState(HGE_SCREENHEIGHT) / 2 - win->getHeight() / 2);
            hWindows[i]->OnRegistered();
        }
    }

    sliTilePicker = new SHR::Slider(SHR::Slider::HORIZONTAL);
    sliTilePicker->setScale(0.0f, 10.0f);
    sliTilePicker->setDimension(gcn::Rectangle(0, 0, hge->System_GetState(HGE_SCREENWIDTH) - 91, 11));
    sliTilePicker->setVisible(0);
    conMain->add(sliTilePicker, 40, hge->System_GetState(HGE_SCREENHEIGHT) - 144);

    winTilePicker = new SHR::Win(&GV->gcnParts, GETL2S("TilePicker", "WinCaption"));
    winTilePicker->setDimension(gcn::Rectangle(0, 0, 250, vPort->GetHeight()));
    winTilePicker->setClose(1);
    winTilePicker->addActionListener(al);
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
    slitpiPointSize->addActionListener(al);
    winTilePicker->add(slitpiPointSize, 15 + 64, 45 + 50);

    sprintf(labtmp, "%s: 1", GETL2S("TilePicker", "LineThickness"));
    labtpiLineThickness = new SHR::Lab(labtmp);
    labtpiLineThickness->setColor(0x5e5e5e);
    labtpiLineThickness->adjustSize();
    winTilePicker->add(labtpiLineThickness, 15 + 64, 45 + 30);

    slitpiLineThickness = new SHR::Slider(25);
    slitpiLineThickness->setOrientation(SHR::Slider::HORIZONTAL);
    slitpiLineThickness->setStyle(SHR::Slider::POINTER);
    slitpiLineThickness->setEnabled(0);
    slitpiLineThickness->setDimension(gcn::Rectangle(0, 0, 150, 14));
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
    slitpiSpraySize->addActionListener(al);
    slitpiSpraySize->setDimension(gcn::Rectangle(0, 0, 150, 14));
    winTilePicker->add(slitpiSpraySize, 15 + 64, 45 + 50);

    sprintf(labtmp, "%s: 50%%", GETL2S("TilePicker", "SprayDensity"));
    labtpiSprayDensity = new SHR::Lab(labtmp);
    labtpiSprayDensity->adjustSize();
    winTilePicker->add(labtpiSprayDensity, 15 + 64, 45 + 70);

    slitpiSprayDensity = new SHR::Slider(10, 100);
    slitpiSprayDensity->setOrientation(SHR::Slider::HORIZONTAL);
    slitpiSprayDensity->setStyle(SHR::Slider::POINTER);
    slitpiSprayDensity->addActionListener(al);
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
    cbtpiShowTileID->addActionListener(al);
    winTilePicker->add(cbtpiShowTileID, 5, 10);

    cbtpiShowProperties = new SHR::CBox(GV->hGfxInterface, GETL2S("TilePicker", "ShowTileProp"));
    cbtpiShowProperties->adjustSize();
    cbtpiShowProperties->addActionListener(al);
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
    buttpfcShow->addActionListener(al);
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
    butObjSearchSelectAll->addActionListener(al);
    butObjSearchSelectAll->setVisible(0);
    winSearchObj->add(butObjSearchSelectAll, 445 - butObjSearchSelectAll->getWidth(), 65);

    cbObjSearchCaseSensitive = new SHR::CBox(GV->hGfxInterface, GETL2S("ObjectSearch", "CaseSensitive"));
    cbObjSearchCaseSensitive->adjustSize();
    cbObjSearchCaseSensitive->addActionListener(al);
    winSearchObj->add(cbObjSearchCaseSensitive, 5, 65);

    labObjSearchResults = new SHR::Lab(GETL2S("ObjectSearch", "NoResults"));
    labObjSearchResults->adjustSize();
    labObjSearchResults->setVisible(0);
    winSearchObj->add(labObjSearchResults, 5, 85);

    labobjseaInfo = new SHR::Lab(GETL2S("ObjectSearch", "InputLabel"));
    labobjseaInfo->adjustSize();
    winSearchObj->add(labobjseaInfo, 5, 15);

    ddObjSearchTerm = new SHR::DropDown();
    ddObjSearchTerm->setListModel(new EditingWWlModel(this, LMODEL_SEARCHTERM));
    ddObjSearchTerm->setDimension(gcn::Rectangle(0, 0, 330 - (labobjseaInfo->getWidth() + 10), 20));
    ddObjSearchTerm->addActionListener(al);
    ddObjSearchTerm->SetGfx(&GV->gcnParts);
    ddObjSearchTerm->adjustHeight();
    winSearchObj->add(ddObjSearchTerm, labobjseaInfo->getWidth() + 10, 15);

    vpobjseaRender = new WIDG::Viewport(vp, VP_OBJSEARCH);
    winSearchObj->add(vpobjseaRender, 0, 0);

    winobjseaName = new SHR::TextField("");
    winobjseaName->setDimension(gcn::Rectangle(0, 0, 325, 20));
    winobjseaName->addActionListener(al);
    winSearchObj->add(winobjseaName, 5, 40);

    sliSearchObj = new SHR::Slider(10);
    sliSearchObj->setOrientation(SHR::Slider::VERTICAL);
    sliSearchObj->setDimension(gcn::Rectangle(0, 0, 11, 387));
    sliSearchObj->setVisible(0);
    winSearchObj->add(sliSearchObj, 430, 105);

    winTileProp = new SHR::Win(&GV->gcnParts, GETL(Lang_TileProperties));
    winTileProp->setDimension(gcn::Rectangle(0, 0, 300, 350));
    winTileProp->setVisible(0);
    winTileProp->setMovable(1);
    winTileProp->setClose(1);
    conMain->add(winTileProp, 300, 300);
    buttpPrev = MakeButton(0, 8, Icon_Undo, winTileProp, 1, 1);
    buttpPrev->SetTooltip(GETL2("Tooltip", Lang_TT_TP_Previous));
    buttpNext = MakeButton(265, 8, Icon_Redo, winTileProp, 1, 1);
    buttpNext->SetTooltip(GETL2("Tooltip", Lang_TT_TP_Next));
    buttpZoom = MakeButton(265, 45, Icon_Zoom, winTileProp, 1, 1);
    buttpZoom->SetTooltip(GETL2("Tooltip", Lang_TT_TP_Zoom));
    buttpShow = MakeButton(265, 80, Icon_Eye, winTileProp, 1, 1);
    buttpShow->SetTooltip(GETL2("Tooltip", Lang_TT_TP_Preview));
    buttpPipette = MakeButton(265, 115, Icon_Pipette, winTileProp, 0);
    buttpPipette->SetTooltip(GETL2("Tooltip", Lang_TT_TP_Pipette));
    buttpApply = MakeButton(265, 150, Icon_Apply, winTileProp, 1, 1);
    buttpApply->SetTooltip(GETL2("Tooltip", Lang_TT_TP_Apply));
    tftpTileID = new SHR::TextField("0");
    tftpTileID->setDimension(gcn::Rectangle(0, 0, 100, 20));
    tftpTileID->addActionListener(al);
    winTileProp->add(tftpTileID, 100, 10);

    rbtpSingle = new SHR::RadBut(GV->hGfxInterface, "", "atribtype");
    rbtpSingle->adjustSize();
    rbtpSingle->addActionListener(al);
    winTileProp->add(rbtpSingle, 145, 123);
    rbtpDouble = new SHR::RadBut(GV->hGfxInterface, "", "atribtype");
    rbtpDouble->adjustSize();
    rbtpDouble->addActionListener(al);
    winTileProp->add(rbtpDouble, 145, 150);

    for (int i = 0; i < 5; i++) {
        rbtpIn[i] = new SHR::RadBut(GV->hGfxInterface, "", "atribinside");
        rbtpIn[i]->addActionListener(al);
        rbtpIn[i]->adjustSize();
        rbtpOut[i] = new SHR::RadBut(GV->hGfxInterface, "", "atriboutside");
        rbtpOut[i]->addActionListener(al);
        rbtpOut[i]->adjustSize();
    }
    for (int i = 0; i < 2; i++) {
        winTileProp->add(rbtpIn[i], 105 + i * 100, 190);
        winTileProp->add(rbtpOut[i], 105 + i * 100, 250);
    }
    for (int i = 2; i < 5; i++) {
        winTileProp->add(rbtpIn[i], 5 + (i - 2) * 100, 220);
        winTileProp->add(rbtpOut[i], 5 + (i - 2) * 100, 280);
    }

    tftpX1 = new SHR::TextField("0");
    tftpX1->setDimension(gcn::Rectangle(0, 0, 35, 20));
    tftpX1->addActionListener(al);
    winTileProp->add(tftpX1, 110, 310);
    tftpY1 = new SHR::TextField("0");
    tftpY1->setDimension(gcn::Rectangle(0, 0, 35, 20));
    tftpY1->addActionListener(al);
    winTileProp->add(tftpY1, 160, 310);
    tftpX2 = new SHR::TextField("0");
    tftpX2->setDimension(gcn::Rectangle(0, 0, 35, 20));
    tftpX2->addActionListener(al);
    winTileProp->add(tftpX2, 210, 310);
    tftpY2 = new SHR::TextField("0");
    tftpY2->setDimension(gcn::Rectangle(0, 0, 35, 20));
    tftpY2->addActionListener(al);
    winTileProp->add(tftpY2, 260, 310);

    tftpW = new SHR::TextField("0");
    tftpW->setDimension(gcn::Rectangle(0, 0, 35, 20));
    tftpW->addActionListener(al);
    winTileProp->add(tftpW, 220, 54);

    tftpH = new SHR::TextField("0");
    tftpH->setDimension(gcn::Rectangle(0, 0, 35, 20));
    tftpH->addActionListener(al);
    winTileProp->add(tftpH, 220, 82);

    itpSelectedTile = -1;
    vpTileProp = new WIDG::Viewport(vp, VP_TILEPROP);
    winTileProp->add(vpTileProp, 0, 0);

    wintoolSelArea = new SHR::Win(&GV->gcnParts, GETL2S("ObjProp", "SelVals"));
    wintoolSelArea->setDimension(gcn::Rectangle(0, 0, 340, 165));
    wintoolSelArea->setShow(false);
    wintoolSelArea->setClose(true);
    wintoolSelArea->addActionListener(al);
    conMain->add(wintoolSelArea, vPort->GetX(), vPort->GetY() + vPort->GetHeight() - 125);

    char label[200];
    sprintf(label, "~w~X1: ~y~0~w~, Y1: ~y~0~w~, X2: ~y~0~w~, Y2: ~y~0~w~");
    labtoolSelAreaValues = new SHR::Lab(label);
    labtoolSelAreaValues->adjustSize();
    wintoolSelArea->add(labtoolSelAreaValues, 7, 13);

    buttoolSelAreaPickMinX = new SHR::But(GV->hGfxInterface, GETL2S("ObjProp", "PickX1"));
    buttoolSelAreaPickMinX->setDimension(gcn::Rectangle(0, 0, 160, 33));
    buttoolSelAreaPickMinX->addActionListener(al);
    wintoolSelArea->add(buttoolSelAreaPickMinX, 5, 35);
    buttoolSelAreaPickMinY = new SHR::But(GV->hGfxInterface, GETL2S("ObjProp", "PickY1"));
    buttoolSelAreaPickMinY->setDimension(gcn::Rectangle(0, 0, 160, 33));
    buttoolSelAreaPickMinY->addActionListener(al);
    wintoolSelArea->add(buttoolSelAreaPickMinY, 5, 70);
    buttoolSelAreaPickMaxX = new SHR::But(GV->hGfxInterface, GETL2S("ObjProp", "PickX2"));
    buttoolSelAreaPickMaxX->setDimension(gcn::Rectangle(0, 0, 160, 33));
    buttoolSelAreaPickMaxX->addActionListener(al);
    wintoolSelArea->add(buttoolSelAreaPickMaxX, 170, 35);
    buttoolSelAreaPickMaxY = new SHR::But(GV->hGfxInterface, GETL2S("ObjProp", "PickY2"));
    buttoolSelAreaPickMaxY->setDimension(gcn::Rectangle(0, 0, 160, 33));
    buttoolSelAreaPickMaxY->addActionListener(al);
    wintoolSelArea->add(buttoolSelAreaPickMaxY, 170, 70);

    buttoolSelAreaOK = new SHR::But(GV->hGfxInterface, GETL2S("ObjProp", "Accept"));
    buttoolSelAreaOK->setDimension(gcn::Rectangle(0, 0, 160, 33));
    buttoolSelAreaOK->addActionListener(al);
    wintoolSelArea->add(buttoolSelAreaOK, 5, 110);

    buttoolSelAreaAll = new SHR::But(GV->hGfxInterface, GETL2S("ObjProp", "PickByArea"));
    buttoolSelAreaAll->setDimension(gcn::Rectangle(0, 0, 160, 33));
    buttoolSelAreaAll->addActionListener(al);
    wintoolSelArea->add(buttoolSelAreaAll, 170, 110);

    winMeasureOpt = new SHR::Win(&GV->gcnParts, GETL(Lang_MeasureOptions));
    winMeasureOpt->setDimension(gcn::Rectangle(0, 0, 300, 100));
    winMeasureOpt->setVisible(0);
    winMeasureOpt->addActionListener(al);
    winMeasureOpt->setClose(1);
    conMain->add(winMeasureOpt, vPort->GetX(), vPort->GetY() + vPort->GetHeight() - 100);

    cbmeasAbsoluteDistance = new SHR::CBox(GV->hGfxInterface, GETL(Lang_ShowAbsoluteDistance));
    cbmeasAbsoluteDistance->adjustSize();
    winMeasureOpt->add(cbmeasAbsoluteDistance, 5, 15);

    butmeasClear = MakeButton(260, 15, Icon_X, winMeasureOpt);

    winObjectBrush = new SHR::Win(&GV->gcnParts, GETL(Lang_BrushProperties));
    winObjectBrush->setDimension(gcn::Rectangle(0, 0, 300, 150));
    winObjectBrush->setVisible(0);
    winObjectBrush->setClose(1);
    winObjectBrush->addActionListener(al);
    conMain->add(winObjectBrush, vPort->GetX(), vPort->GetY() + vPort->GetHeight() - 125);

    char tmp[128];
    sprintf(tmp, "%s: -", GETL(Lang_SourceObjectID));
    labobrSource = new SHR::Lab(tmp);
    labobrSource->adjustSize();
    winObjectBrush->add(labobrSource, 5, 12);

    labobrDispX = new SHR::Lab(GETL2S("Tool_ObjectBrush", "ScatterX"));
    labobrDispX->adjustSize();
    winObjectBrush->add(labobrDispX, 5, 35);

    tfobrDispX = new SHR::TextField("0");
    tfobrDispX->setDimension(gcn::Rectangle(0, 0, 50, 20));
    tfobrDispX->SetNumerical(1, 0);
    winObjectBrush->add(tfobrDispX, labobrDispX->getWidth() + 15, 35);

    labobrDispY = new SHR::Lab(GETL2S("Tool_ObjectBrush", "ScatterY"));
    labobrDispY->adjustSize();
    winObjectBrush->add(labobrDispY, tfobrDispX->getX() + tfobrDispX->getWidth() + 15, 35);

    tfobrDispY = new SHR::TextField("0");
    tfobrDispY->setDimension(gcn::Rectangle(0, 0, 50, 20));
    tfobrDispY->SetNumerical(1, 0);
    winObjectBrush->add(tfobrDispY, labobrDispY->getX() + labobrDispY->getWidth() + 15, 35);

    cbobrApplyScatterSeparately = new SHR::CBox(GV->hGfxInterface, GETL2S("Tool_ObjectBrush", "ScatterSeparately"));
    cbobrApplyScatterSeparately->adjustSize();
    winObjectBrush->add(cbobrApplyScatterSeparately, 5, 60);

    sliobrDistance = new SHR::Slider(SHR::Slider::HORIZONTAL);
    sliobrDistance->setScale(10.0f, 750.0f);
    sliobrDistance->setDimension(gcn::Rectangle(0, 0, 280, 16));
    sliobrDistance->setValue(64.0f);
    sliobrDistance->addActionListener(al);
    sliobrDistance->setMarkerLength(45);
    winObjectBrush->add(sliobrDistance, 5, 110);

    sprintf(tmp, "%s: 64px", GETL(Lang_BrushInterval));
    labobrDistance = new SHR::Lab(tmp);
    labobrDistance->adjustSize();
    winObjectBrush->add(labobrDistance, 5, 85);

    winSpacing = new SHR::Win(&GV->gcnParts, GETL2S("WinSpacing", "WinCaption"));
    winSpacing->setDimension(gcn::Rectangle(0, 0, 250, 100));
    winSpacing->setVisible(0);
    winSpacing->setClose(1);
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
    butspacingOK->addActionListener(al);
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
    butcamSetTo->addActionListener(al);
    winCamera->add(butcamSetTo, 5, 145);

    butcamSetToSpawn = new SHR::But(GV->hGfxInterface, GETL(Lang_SetToSpawn));
    butcamSetToSpawn->setDimension(gcn::Rectangle(0, 0, 90, 33));
    butcamSetToSpawn->addActionListener(al);
    winCamera->add(butcamSetToSpawn, 105, 145);

    winLogicBrowser = new SHR::Win(&GV->gcnParts, GETL2S("Win_LogicBrowser", "WinCaption"));
    winLogicBrowser->setDimension(gcn::Rectangle(0, 0, 300, 220));//800, 240));
    winLogicBrowser->setVisible(0);
    winLogicBrowser->setClose(1);
    winLogicBrowser->addActionListener(al);
    conMain->add(winLogicBrowser, hge->System_GetState(HGE_SCREENWIDTH) / 2 - 150,
                 hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 110);

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

    tfbrlRename = new SHR::TextField("");
    tfbrlRename->setDimension(gcn::Rectangle(0, 0, 200, 20));
    tfbrlRename->addActionListener(al);
    tfbrlRename->setVisible(0);

    butbrlRenameOK = new SHR::But(GV->hGfxInterface, "OK");
    butbrlRenameOK->setDimension(gcn::Rectangle(0, 0, 30, 30));
    butbrlRenameOK->addActionListener(al);
    butbrlRenameOK->setVisible(0);

    labbrlLogicNameV = new SHR::Lab("");
    labbrlLogicNameV->setColor(0xFFFFFF);
    labbrlFilePathV = new SHR::Lab("");
    labbrlFilePathV->setColor(0xFFFFFF);
    labbrlFileSizeV = new SHR::Lab("");
    labbrlFileSizeV->setColor(0xFFFFFF);
    labbrlFileChecksumV = new SHR::Lab("");
    labbrlFileChecksumV->setColor(0xFFFFFF);
    labbrlFileModDateV = new SHR::Lab("");
    labbrlFileModDateV->setColor(0xFFFFFF);

    lbbrlLogicList = new SHR::ListBox(0);
    lbbrlLogicList->addActionListener(al);
    lbbrlLogicList->setDimension(gcn::Rectangle(0, 0, 300, 215));

    sabrlLogicList = new SHR::ScrollArea(lbbrlLogicList, SHR::ScrollArea::SHOW_NEVER, SHR::ScrollArea::SHOW_AUTO);
    sabrlLogicList->setBackgroundColor(0x131313);
    sabrlLogicList->setDimension(gcn::Rectangle(0, 0, 296, 194));
    winLogicBrowser->add(sabrlLogicList, 0, 8);

    /*butbrlNew = new SHR::But(GV->hGfxInterface, GETL2S("Win_LogicBrowser", "AddNew"));
	butbrlNew->setIcon(GV->sprIcons16[Icon16_Add]);
	butbrlNew->setDimension(gcn::Rectangle(0, 0, 150, 25));
	butbrlNew->addActionListener(al);
	winLogicBrowser->add(butbrlNew, 0, 195);*/

    butbrlBrowseDir = new SHR::But(GV->hGfxInterface, GETL2S("Win_LogicBrowser", "BrowseDir"));
    butbrlBrowseDir->setIcon(GV->sprIcons16[Icon16_Open]);
    butbrlBrowseDir->setDimension(gcn::Rectangle(0, 0, 235, 25));
    butbrlBrowseDir->addActionListener(al);

    butbrlEdit = new SHR::But(GV->hGfxInterface, GETL2S("Win_LogicBrowser", "Edit"));
    butbrlEdit->setIcon(GV->sprIcons16[Icon16_Pencil]);
    butbrlEdit->setDimension(gcn::Rectangle(0, 0, 235, 25));
    butbrlEdit->addActionListener(al);

    /*butbrlEditExternal = new SHR::But(GV->hGfxInterface, GETL2S("Win_LogicBrowser", "OpenFile"));
	butbrlEditExternal->setIcon(GV->sprIcons16[Icon16_Open]);
	butbrlEditExternal->setDimension(gcn::Rectangle(0, 0, 235, 25));
	butbrlEditExternal->addActionListener(al);
	winLogicBrowser->add(butbrlEditExternal, 310, 190);*/

    butbrlRename = new SHR::But(GV->hGfxInterface, GETL2S("Win_LogicBrowser", "Rename"));
    butbrlRename->setIcon(GV->sprIcons16[Icon16_X]);
    butbrlRename->setDimension(gcn::Rectangle(0, 0, 235, 25));
    butbrlRename->addActionListener(al);

    butbrlDelete = new SHR::But(GV->hGfxInterface, GETL2S("Win_LogicBrowser", "Delete"));
    butbrlDelete->setIcon(GV->sprIcons16[Icon16_Trash]);
    butbrlDelete->setDimension(gcn::Rectangle(0, 0, 235, 25));
    butbrlDelete->addActionListener(al);

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
    dbAssetsImg->addActionListener(al);

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
    dbAssetsAni->addActionListener(al);

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
    dbAssetsSnd->addActionListener(al);

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
    tfmsSaveAs->addActionListener(al);
    winMapShot->add(tfmsSaveAs, 100, 15);

    butmsSaveAs = new SHR::But(GV->hGfxInterface, GV->sprIcons[Icon_Open]);
    butmsSaveAs->setDimension(gcn::Rectangle(0, 0, 32, 32));
    butmsSaveAs->addActionListener(al);
    winMapShot->add(butmsSaveAs, 458, 9);

    butmsSave = new SHR::But(GV->hGfxInterface, GETL(Lang_Save));
    butmsSave->setDimension(gcn::Rectangle(0, 0, 100, 33));
    butmsSave->addActionListener(al);
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
    slimsScale->addActionListener(al);
    winMapShot->add(slimsScale, 100, 50);

    labmsPlane = new SHR::Lab(GETL(Lang_Plane));
    labmsPlane->adjustSize();
    winMapShot->add(labmsPlane, 5, 75);

    ddmsPlane = new SHR::DropDown();
    ddmsPlane->setListModel(new EditingWWlModel(this, LMODEL_PLANES));
    ddmsPlane->setDimension(gcn::Rectangle(0, 0, 250, 20));
    ddmsPlane->addActionListener(al);
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
    slimsCompression->addActionListener(al);
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
    winpmMain->setDimension(gcn::Rectangle(0, 0, 225, 300));
    winpmMain->setVisible(false);
    winpmMain->setClose(true);
    conMain->add(winpmMain, hge->System_GetState(HGE_SCREENWIDTH) / 2 - 112,
                 hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 162);

    lbpmPlanes = new SHR::ListBox(new EditingWWlModel(this, LMODEL_PLANESPROP));
    lbpmPlanes->setDimension(gcn::Rectangle(0, 0, 210, 300));
    lbpmPlanes->addActionListener(al);

    sapmPlanes = new SHR::ScrollArea();
    sapmPlanes->setVerticalScrollPolicy(SHR::ScrollArea::SHOW_AUTO);
    sapmPlanes->setHorizontalScrollPolicy(SHR::ScrollArea::SHOW_NEVER);
    sapmPlanes->setDimension(gcn::Rectangle(0, 0, 210, 245));
    sapmPlanes->setContent(lbpmPlanes);
    sapmPlanes->setOpaque(0);
    winpmMain->add(sapmPlanes, 5, 35);

    labpmPlanes = new SHR::Lab(GETL2S("PlaneProperties", "SelectPlane"));
    labpmPlanes->adjustSize();
    winpmMain->add(labpmPlanes, 5, 15);

    labpmName = new SHR::Lab(GETL(Lang_Name));
    labpmName->adjustSize();
    winpmMain->add(labpmName, 220, 15);

    tfpmName = new SHR::TextField();
    tfpmName->setDimension(gcn::Rectangle(0, 0, (525 - (230 + labpmName->getWidth())) - 35, 20));
    winpmMain->add(tfpmName, 230 + labpmName->getWidth(), 15);

    labpmTileSet = new SHR::Lab(GETL2S("PlaneProperties", "Tileset"));
    labpmTileSet->adjustSize();
    winpmMain->add(labpmTileSet, 220, 45);

    ddpmTileSet = new SHR::DropDown();
    ddpmTileSet->setDimension(gcn::Rectangle(0, 0, (525 - (230 + labpmTileSet->getWidth())) - 35, 20));
    ddpmTileSet->SetGfx(&GV->gcnParts);
    ddpmTileSet->adjustHeight();

    winpmMain->add(ddpmTileSet, 230 + labpmTileSet->getWidth(), 45);

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
    winpmMain->add(labpmPlaneSize, 220, 115);
    labpmAnchor = new SHR::Lab(GETL2S("PlaneProperties", "Anchor"));
    labpmAnchor->adjustSize();
    winpmMain->add(labpmAnchor, 525 - 96 - 35, 115);

    labpmWidth = new SHR::Lab(GETL2S("PlaneProperties", "Width"));
    labpmWidth->adjustSize();
    winpmMain->add(labpmWidth, 220, 140);
    labpmHeight = new SHR::Lab(GETL2S("PlaneProperties", "Height"));
    labpmHeight->adjustSize();
    winpmMain->add(labpmHeight, 220, 170);

    tfpmPlaneSizeX = new SHR::TextField("");
    tfpmPlaneSizeX->SetNumerical(1);
    tfpmPlaneSizeX->setDimension(gcn::Rectangle(0, 0, 75, 20));
    tfpmPlaneSizeX->addActionListener(al);
    winpmMain->add(tfpmPlaneSizeX, 301, 140);
    tfpmPlaneSizeY = new SHR::TextField("");
    tfpmPlaneSizeY->SetNumerical(1);
    tfpmPlaneSizeY->setDimension(gcn::Rectangle(0, 0, 75, 20));
    tfpmPlaneSizeY->addActionListener(al);
    winpmMain->add(tfpmPlaneSizeY, 301, 170);

    ipmAnchor = 5;
    ipmSizeX = ipmSizeY = -1;

    butpmResUL = MakeButton(525 - 96 - 35, 140, Icon_UpLeft, winpmMain, 1, 1);
    butpmResU = MakeButton(525 - 64 - 35, 140, Icon_Up, winpmMain, 1, 1);
    butpmResUR = MakeButton(525 - 32 - 35, 140, Icon_UpRight, winpmMain, 1, 1);

    butpmResL = MakeButton(525 - 96 - 35, 140 + 32, Icon_Left, winpmMain, 1, 1);
    butpmResC = MakeButton(525 - 64 - 35, 140 + 32, Icon_Anchor, winpmMain, 1, 1);
    butpmResR = MakeButton(525 - 32 - 35, 140 + 32, Icon_Right, winpmMain, 1, 1);

    butpmResDL = MakeButton(525 - 96 - 35, 140 + 64, Icon_DownLeft, winpmMain, 1, 1);
    butpmResD = MakeButton(525 - 64 - 35, 140 + 64, Icon_Down, winpmMain, 1, 1);
    butpmResDR = MakeButton(525 - 32 - 35, 140 + 64, Icon_DownRight, winpmMain, 1, 1);

    labpmTileSize = new SHR::Lab(GETL2S("PlaneProperties", "TileDim"));
    labpmTileSize->adjustSize();
    winpmMain->add(labpmTileSize, 497, 15);

    labpmTileSize_x = new SHR::Lab("x");
    labpmTileSize_x->adjustSize();
    winpmMain->add(labpmTileSize_x, 676, 15);

    tfpmTileSizeX = new SHR::TextField("");
    tfpmTileSizeX->SetNumerical(1);
    tfpmTileSizeX->setDimension(gcn::Rectangle(0, 0, 35, 20));
    winpmMain->add(tfpmTileSizeX, 636, 13);

    tfpmTileSizeY = new SHR::TextField("");
    tfpmTileSizeY->SetNumerical(1);
    tfpmTileSizeY->setDimension(gcn::Rectangle(0, 0, 35, 20));
    winpmMain->add(tfpmTileSizeY, 689, 13);

    labpmMovement = new SHR::Lab(GETL2S("PlaneProperties", "Movement"));
    labpmMovement->adjustSize();
    winpmMain->add(labpmMovement, 497, 40);

    labpmMovX = new SHR::Lab("X:");
    labpmMovX->adjustSize();
    winpmMain->add(labpmMovX, 621, 40);
    labpmMovY = new SHR::Lab("Y:");
    labpmMovY->adjustSize();
    winpmMain->add(labpmMovY, 674, 40);

    tfpmMovX = new SHR::TextField("");
    tfpmMovX->SetNumerical(1);
    tfpmMovX->setDimension(gcn::Rectangle(0, 0, 35, 20));
    winpmMain->add(tfpmMovX, 636, 38);

    tfpmMovY = new SHR::TextField("");
    tfpmMovY->SetNumerical(1);
    tfpmMovY->setDimension(gcn::Rectangle(0, 0, 35, 20));
    winpmMain->add(tfpmMovY, 689, 38);

    labpmZCoord = new SHR::Lab(GETL2S("PlaneProperties", "ZCoord"));
    labpmZCoord->adjustSize();
    winpmMain->add(labpmZCoord, 532 - 35, 65);

    tfpmZCoord = new SHR::TextField();
    tfpmZCoord->SetNumerical(1);
    tfpmZCoord->setDimension(gcn::Rectangle(0, 0, 89, 20));
    winpmMain->add(tfpmZCoord, 636, 63);

    labpmFlags = new SHR::Lab(GETL2S("PlaneProperties", "Flags"));
    labpmFlags->adjustSize();
    winpmMain->add(labpmFlags, 532 - 35, 115);

    cbpmFlagMainPlane = new SHR::CBox(GV->hGfxInterface, GETL2S("PlaneProperties", "FlagMainPlane"));
    cbpmFlagMainPlane->adjustSize();
    winpmMain->add(cbpmFlagMainPlane, 532 - 35, 135);

    cbpmFlagNoDraw = new SHR::CBox(GV->hGfxInterface, GETL2S("PlaneProperties", "FlagNoDraw"));
    cbpmFlagNoDraw->adjustSize();
    winpmMain->add(cbpmFlagNoDraw, 532 - 35, 155);

    cbpmFlagWrapX = new SHR::CBox(GV->hGfxInterface, GETL2S("PlaneProperties", "FlagWrapX"));
    cbpmFlagWrapX->adjustSize();
    winpmMain->add(cbpmFlagWrapX, 532 - 35, 175);

    cbpmFlagWrapY = new SHR::CBox(GV->hGfxInterface, GETL2S("PlaneProperties", "FlagWrapY"));
    cbpmFlagWrapY->adjustSize();
    winpmMain->add(cbpmFlagWrapY, 532 - 35, 195);

    cbpmAutoTileSize = new SHR::CBox(GV->hGfxInterface, GETL2S("PlaneProperties", "FlagAutoTileSize"));
    cbpmAutoTileSize->adjustSize();
    winpmMain->add(cbpmAutoTileSize, 532 - 35, 215);

    butpmSave = new SHR::But(GV->hGfxInterface, GETL(Lang_Save));
    butpmSave->setDimension(gcn::Rectangle(0, 0, 100, 33));
    butpmSave->addActionListener(al);
    winpmMain->add(butpmSave, 258 - 35, 245);

    butpmDelete = new SHR::But(GV->hGfxInterface, GETL(Lang_Delete));
    butpmDelete->setDimension(gcn::Rectangle(0, 0, 100, 33));
    butpmDelete->addActionListener(al);
    winpmMain->add(butpmDelete, 358 - 25, 245);

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
    cbwpCompress->addActionListener(al);
    winWorld->add(cbwpCompress, 5, 210);

    cbwpZCoord = new SHR::CBox(GV->hGfxInterface, GETL(Lang_ZCoord));
    cbwpZCoord->adjustSize();
    cbwpZCoord->addActionListener(al);
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
    butwpSave->addActionListener(al);

    butwpCancel = new SHR::But(GV->hGfxInterface, GETL(Lang_Cancel));
    butwpCancel->setDimension(gcn::Rectangle(0, 0, 100, 33));
    butwpCancel->addActionListener(al);

    winWorld->add(butwpSave, 180, 235);
    winWorld->add(butwpCancel, 290, 235);

    vpWorld = new WIDG::Viewport(vp, VP_WORLD);
    winWorld->add(vpWorld, 0, 0);

    winDuplicate = new SHR::Win(&GV->gcnParts, GETL(Lang_Duplicating));
    winDuplicate->setDimension(gcn::Rectangle(0, 0, 210, 200));
    winDuplicate->setVisible(0);
    winDuplicate->setClose(1);
    conMain->add(winDuplicate, hge->System_GetState(HGE_SCREENWIDTH) / 2 - 105,
                 hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 100);

    labdCopiedObjectID = new SHR::Lab("");
    labdCopiedObjectID->adjustSize();
    winDuplicate->add(labdCopiedObjectID, 5, 15);

    labdTimes = new SHR::Lab(GETL(Lang_CopyXTimes));
    labdTimes->adjustSize();
    winDuplicate->add(labdTimes, 5, 40);

    tfdTimes = new SHR::TextField("1");
    tfdTimes->setDimension(gcn::Rectangle(0, 0, 75, 20));
    tfdTimes->SetNumerical(1, 0);
    tfdTimes->addActionListener(al);
    winDuplicate->add(tfdTimes, 125, 40);

    labdOffsetX = new SHR::Lab(GETL(Lang_OffsetX));
    labdOffsetX->adjustSize();
    labdOffsetX->addActionListener(al);
    winDuplicate->add(labdOffsetX, 5, 65);

    tfdOffsetX = new SHR::TextField("0");
    tfdOffsetX->setDimension(gcn::Rectangle(0, 0, 75, 20));
    tfdOffsetX->SetNumerical(1, 1);
    tfdOffsetX->addActionListener(al);
    winDuplicate->add(tfdOffsetX, 125, 65);

    labdOffsetY = new SHR::Lab(GETL(Lang_OffsetY));
    labdOffsetY->adjustSize();
    winDuplicate->add(labdOffsetY, 5, 90);

    tfdOffsetY = new SHR::TextField("0");
    tfdOffsetY->setDimension(gcn::Rectangle(0, 0, 75, 20));
    tfdOffsetY->SetNumerical(1, 1);
    winDuplicate->add(tfdOffsetY, 125, 90);

    labdChangeZ = new SHR::Lab(GETL2S("Win_Duplicate", "ChangeZ"));
    labdChangeZ->adjustSize();
    winDuplicate->add(labdChangeZ, 5, 115);

    tfdChangeZ = new SHR::TextField("0");
    tfdChangeZ->setDimension(gcn::Rectangle(0, 0, 75, 20));
    tfdChangeZ->SetNumerical(1, 1);
    winDuplicate->add(tfdChangeZ, 125, 115);

    butdOK = new SHR::But(GV->hGfxInterface, GETL(Lang_OK));
    butdOK->setDimension(gcn::Rectangle(0, 0, 75, 25));
    butdOK->addActionListener(al);
    winDuplicate->add(butdOK, 68, 145);

    gui->setTop(conMain);

    dwBG = GV->colBaseDark;
    fCamX = fCamY = 0;
    cScrollOrientation = 2;
    bDragDropScroll = 0;

    GV->Console->AddModifiableFloat("camx", &fCamX);
    GV->Console->AddModifiableFloat("camy", &fCamY);
    GV->Console->AddModifiableFloat("zoom", &fZoom);
    GV->Console->AddModifiableFloat("dzoom", &fDestZoom);
    GV->Console->AddModifiableBool("smoothz", &GV->bSmoothZoom);
    GV->Console->AddModifiableBool("redraw", &bConstRedraw);
    GV->Console->AddModifiableBool("drawprop", &bDrawTileProperties);

    conRecentFiles = new SHR::Container();
    conRecentFiles->setOpaque(0);
    labLoadLastOpened = new SHR::Lab(GETL2S("HomeScreen", "RecentDocs"));
    labLoadLastOpened->adjustSize();
    conRecentFiles->setDimension(gcn::Rectangle(0, 0, 200, 20));
    conRecentFiles->add(labLoadLastOpened, 5, 5);

    butCrashRetrieve = NULL;
    conCrashRetrieve = NULL;

    if (GV->szCmdLine[0] != '"') {
        FILE *f = fopen("runtime.tmp", "r");
        if (f) {
            GV->Console->Printf("~w~Crash registered while used last time. Recovering tabs.");
            fclose(f);
            std::ifstream ifs("runtime.tmp");
            std::string temp;
            int maxnamelen = 0;
            int linec = 0;
            for (int i = 0; i < 10; i++)
                szCrashRetrieve[i] = NULL;
            while (getline(ifs, temp)) {
                if (temp.length()) {
                    if (linec < 10) {
                        bool bAddSign = 1;
                        int iSignStart = 0;
                        char tmp[MAX_PATH + 1];
                        strcpy(tmp, temp.c_str());
                        int ibase = 0;
                        try {
                            iCrashRetrieveIcon[linec] = WWD::GetGameTypeFromFile(tmp, &ibase);
                        }
                        catch (...) {
                            iCrashRetrieveIcon[linec] = WWD::Game_Unknown;
                        }
                        if (ibase > 0 && ibase < 15 && iCrashRetrieveIcon[linec] == WWD::Game_Claw) {
                            iCrashRetrieveIcon[linec] = 50 + ibase;
                        }
                        while (GV->fntMyriad13->GetStringWidth(tmp) > 460) {
                            int len = strlen(tmp);
                            if (bAddSign) {
                                tmp[int(len / 4) - 2] = '(';
                                tmp[int(len / 4) - 1] = '.';
                                tmp[int(len / 4)] = '.';
                                tmp[int(len / 4) + 1] = '.';
                                tmp[int(len / 4) + 2] = ')';
                                bAddSign = 0;
                                iSignStart = int(len / 4) - 2;
                            }
                            for (int it = iSignStart - 3; it <= len - 3; it++)
                                tmp[it] = tmp[it + 3];
                            for (int it = iSignStart + 2; it < len - 3; it++)
                                tmp[it] = tmp[it + 3];
                            iSignStart -= 3;
                        }
                        szCrashRetrieve[linec] = new char[strlen(tmp) + 1];
                        strcpy(szCrashRetrieve[linec], tmp);
                        int len = GV->fntMyriad13->GetStringWidth(tmp) + 20;
                        if (len > maxnamelen)
                            maxnamelen = len;
                    }
                    linec++;
                }
            }
            if (linec > 0) {
                conCrashRetrieve = new SHR::Container();
                conCrashRetrieve->setOpaque(0);
                conCrashRetrieve->setDimension(gcn::Rectangle(0, 0, 590, 25 + (linec > 10 ? 11 : linec) * 25 + 40));
                vpCrashRetrieve = new WIDG::Viewport(vp, VP_CRASHRETRIEVE);
                conCrashRetrieve->add(vpCrashRetrieve, 0, 0);
                butCrashRetrieve = new SHR::But(GV->hGfxInterface, GETL2S("WinCrashRetrieve", "Retrieve"));
                butCrashRetrieve->setDimension(gcn::Rectangle(0, 0, 100, 33));
                butCrashRetrieve->addActionListener(al);
                conCrashRetrieve->add(butCrashRetrieve, 590 / 2 - 50, conCrashRetrieve->getHeight() - 40);
                if (linec > 10) {
                    char tmp[512];
                    sprintf(tmp, GETL2S("WinCrashRetrieve", "MoreTabs"), linec - 10);
                    szCrashRetrieveMore = new char[strlen(tmp) + 1];
                    strcpy(szCrashRetrieveMore, tmp);
                } else {
                    szCrashRetrieveMore = NULL;
                }
            }
        }
    }

    winWelcome = new SHR::Container();
    winWelcome->setDimension(gcn::Rectangle(0, 0, 600, 180));
    winWelcome->setOpaque(0);
    conMain->add(winWelcome, hge->System_GetState(HGE_SCREENWIDTH) / 2 - 300,
                 hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 150);

    vpws = new WIDG::Viewport(vp, VP_WELCOMESCREEN);
    winWelcome->add(vpws, 0, 0);

    butwsNew = new SHR::But(GV->hGfxInterface, GETL2S("HomeScreen", "New"));
    butwsNew->setDimension(gcn::Rectangle(0, 0, 128, 33));
    butwsNew->addActionListener(al);
    winWelcome->add(butwsNew, 11, 133);

    butwsOpen = new SHR::But(GV->hGfxInterface, GETL2S("HomeScreen", "Open"));
    butwsOpen->setDimension(gcn::Rectangle(0, 0, 128, 33));
    butwsOpen->addActionListener(al);
    winWelcome->add(butwsOpen, 11 + 150, 133);

    butwsRecently = new SHR::But(GV->hGfxInterface, GETL2S("HomeScreen", "LastUsed"));
    butwsRecently->setDimension(gcn::Rectangle(0, 0, 128, 33));
    butwsRecently->addActionListener(al);
    winWelcome->add(butwsRecently, 11 + 300, 133);

    butwsWhatsnew = new SHR::But(GV->hGfxInterface, GETL2S("HomeScreen", "Whatsnew"));
    butwsWhatsnew->setDimension(gcn::Rectangle(0, 0, 128, 33));
    butwsWhatsnew->addActionListener(al);
    winWelcome->add(butwsWhatsnew, 11 + 450, 133);

    winWelcome->setVisible(GV->szCmdLine.empty());

    for (auto & link : lnkLastOpened) {
        link = NULL;
    }
    hMruList = new cMruList();
    MruListUpdated();

    hAppMenu->SyncMRU();

    if ((hMruList->IsValid() && hMruList->GetFilesCount() > 0) || conCrashRetrieve) {
        SHR::Container *conToAdd = (conCrashRetrieve ? conCrashRetrieve : conRecentFiles);

        winWelcome->add(conToAdd, winWelcome->getWidth() / 2 - conToAdd->getWidth() / 2,
                        winWelcome->getHeight() + 10);
    }

    if (hMruList->IsValid() && hMruList->GetFilesCount() > 0) {
        char *n = SHR::GetFile(hMruList->GetRecentlyUsedFile(0));
        bool bDots = false;
        while (GV->fntMyriad13->GetStringWidth(n) > 140) {
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
        labwsRecently = new SHR::Lab(n);
        delete[] n;
        labwsRecently->adjustSize();
        winWelcome->add(labwsRecently, 375 - labwsRecently->getWidth() / 2, 108);
    } else {
        labwsRecently = 0;
    }

    winWelcome->setY(vPort->GetY() + vPort->GetHeight() / 2 - winWelcome->getHeight() / 2);

    butwsRecently->setEnabled(hMruList->GetFilesCount() > 0);

    btpZoomTile = 0;

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

    objmAlignContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
    objmAlignContext->AddElement(OBJMENU_ALIGN_HOR, GETL2S("Various", "ContextAlign_Hor"),
                                 GV->sprIcons16[Icon16_FlipX]);
    objmAlignContext->AddElement(OBJMENU_ALIGN_VERT, GETL2S("Various", "ContextAlign_Vert"),
                                 GV->sprIcons16[Icon16_FlipY]);
    objmAlignContext->adjustSize();
    objmAlignContext->hide();
    objmAlignContext->addActionListener(al);
    conMain->add(objmAlignContext, 400, 400);

    objmSpaceContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
    objmSpaceContext->AddElement(OBJMENU_SPACE_HOR, GETL2S("Various", "ContextSpaceHor"), GV->sprIcons16[Icon16_FlipX]);
    objmSpaceContext->AddElement(OBJMENU_SPACE_VERT, GETL2S("Various", "ContextSpaceVer"),
                                 GV->sprIcons16[Icon16_FlipY]);
    objmSpaceContext->adjustSize();
    objmSpaceContext->hide();
    objmSpaceContext->addActionListener(al);
    conMain->add(objmSpaceContext, 400, 400);

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
    conmodObjectMultiple->AddElement(OBJMENU_SPACE, GETL2S("Various", "ContextSpace"), GV->sprIcons16[Icon16_FlipX]);
    conmodObjectMultiple->GetElementByID(OBJMENU_SPACE)->SetCascade(objmSpaceContext);

    conmodSpawnPoint = new SHR::ContextModel();
    conmodSpawnPoint->AddElement(OBJMENU_MOVE, GETL(Lang_Move), GV->sprIcons16[Icon16_Move]);

    //conmodUseAsBrush = new SHR::ContextModel();
    //conmodUseAsBrush->AddElement(OBJMENU_USEASBRUSH, GETL(Lang_UseAsBrush), GV->sprIcons16[Icon16_Brush]);

    conmodPaste = new SHR::ContextModel();
    conmodPaste->AddElement(OBJMENU_PASTE, GETL(Lang_Paste), GV->sprIcons16[Icon16_Paste]);
    conmodPaste->AddElement(OBJMENU_NEWOBJ, GETL(Lang_NewObjectCtx), GV->sprIcons16[Icon16_Star]);
    conmodPaste->AddElement(OBJMENU_SETSPAWNP, GETL(Lang_SetSpawnPoint), GV->sprIcons16[Icon16_Move]);
    conmodPaste->AddElement(OBJMENU_TESTFROMHERE, GETL2S("Various", "ContextTestFromHere"),
                            GV->sprIcons16[Icon16_Play]);

    conmodAtEmpty = new SHR::ContextModel();
    conmodAtEmpty->AddElement(OBJMENU_NEWOBJ, GETL(Lang_NewObjectCtx), GV->sprIcons16[Icon16_Star]);
    conmodAtEmpty->AddElement(OBJMENU_SETSPAWNP, GETL(Lang_SetSpawnPoint), GV->sprIcons16[Icon16_Move]);
    conmodAtEmpty->AddElement(OBJMENU_TESTFROMHERE, GETL2S("Various", "ContextTestFromHere"),
                              GV->sprIcons16[Icon16_Play]);

    conmodPaste->GetElementByID(OBJMENU_NEWOBJ)->SetCascade(hmbObject->GetContext());
    conmodAtEmpty->GetElementByID(OBJMENU_NEWOBJ)->SetCascade(hmbObject->GetContext());

    conMain->add(hmbObject->GetContext(), 400, 400);

    objZCoordContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
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
    objZCoordContext->addActionListener(al);
    conMain->add(objZCoordContext, 400, 400);

    objFlagContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
    objFlagContext->AddElement(OBJMENU_FLAGS_DRAW, GETL(Lang_DrawingFlags), 0);
    objFlagContext->AddElement(OBJMENU_FLAGS_ADDITIONAL, GETL(Lang_AddFlags), 0);
    objFlagContext->AddElement(OBJMENU_FLAGS_DYNAMIC, GETL(Lang_DynamicFlags), 0);
    objFlagContext->adjustSize();
    objFlagContext->hide();
    conMain->add(objFlagContext, 400, 400);
    conmodObject->GetElementByID(OBJMENU_FLAGS)->SetCascade(objFlagContext);

    objFlagDrawContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
    objFlagDrawContext->AddElement(OBJMENU_FLAGS_DRAW_NODRAW, GETL(Lang_NoDraw), 0);
    objFlagDrawContext->AddElement(OBJMENU_FLAGS_DRAW_FLIPX, GETL(Lang_Mirror), 0);
    objFlagDrawContext->AddElement(OBJMENU_FLAGS_DRAW_FLIPY, GETL(Lang_Invert), 0);
    objFlagDrawContext->AddElement(OBJMENU_FLAGS_DRAW_FLASH, GETL(Lang_Flash), 0);
    objFlagDrawContext->ReserveIconSpace(0, 1);
    objFlagDrawContext->adjustSize();
    objFlagDrawContext->addActionListener(al);
    objFlagDrawContext->hide();
    conMain->add(objFlagDrawContext, 400, 400);
    objFlagContext->GetElementByID(OBJMENU_FLAGS_DRAW)->SetCascade(objFlagDrawContext);

    objFlagAddContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
    objFlagAddContext->AddElement(OBJMENU_FLAGS_ADDITIONAL_DIFFICULT, GETL(Lang_ObjFlag_Difficult), 0);
    objFlagAddContext->AddElement(OBJMENU_FLAGS_ADDITIONAL_EXTRAMEMORY, GETL(Lang_ObjFlag_ExtraMemory), 0);
    objFlagAddContext->AddElement(OBJMENU_FLAGS_ADDITIONAL_EYECANDY, GETL(Lang_ObjFlag_EyeCandy), 0);
    objFlagAddContext->AddElement(OBJMENU_FLAGS_ADDITIONAL_FASTCPU, GETL(Lang_ObjFlag_FastCPU), 0);
    objFlagAddContext->AddElement(OBJMENU_FLAGS_ADDITIONAL_HIGHDETAIL, GETL(Lang_ObjFlag_HighDetail), 0);
    objFlagAddContext->AddElement(OBJMENU_FLAGS_ADDITIONAL_MULTIPLAYER, GETL(Lang_ObjFlag_Multiplayer), 0);
    objFlagAddContext->ReserveIconSpace(0, 1);
    objFlagAddContext->adjustSize();
    objFlagAddContext->addActionListener(al);
    objFlagAddContext->hide();
    conMain->add(objFlagAddContext, 400, 400);
    objFlagContext->GetElementByID(OBJMENU_FLAGS_ADDITIONAL)->SetCascade(objFlagAddContext);

    objFlagDynamicContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
    objFlagDynamicContext->AddElement(OBJMENU_FLAGS_DYNAMIC_ALWAYSACTIVE, GETL(Lang_ObjFlag_AlwaysActive), 0);
    objFlagDynamicContext->AddElement(OBJMENU_FLAGS_DYNAMIC_AUTOHITDAMAGE, GETL(Lang_ObjFlag_AutoHitDmg), 0);
    objFlagDynamicContext->AddElement(OBJMENU_FLAGS_DYNAMIC_NOHIT, GETL(Lang_ObjFlag_NoHit), 0);
    objFlagDynamicContext->AddElement(OBJMENU_FLAGS_DYNAMIC_SAFE, GETL(Lang_ObjFlag_Safe), 0);
    objFlagDynamicContext->ReserveIconSpace(0, 1);
    objFlagDynamicContext->adjustSize();
    objFlagDynamicContext->addActionListener(al);
    objFlagDynamicContext->hide();
    conMain->add(objFlagDynamicContext, 400, 400);
    objFlagContext->GetElementByID(OBJMENU_FLAGS_DYNAMIC)->SetCascade(objFlagDynamicContext);

    objContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
    objContext->SetModel(conmodObject);
    objContext->adjustSize();
    objContext->addActionListener(al);
    objContext->hide();
    conMain->add(objContext, 400, 400);

    advcon_Warp = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
    advcon_Warp->AddElement(OBJMENU_ADV_WARP_GOTO, GETL2S("EditObj_Warp", "Context_GoTo"), GV->sprIcons16[Icon16_Warp]);
    advcon_Warp->adjustSize();
    advcon_Warp->hide();
    advcon_Warp->addActionListener(al);
    conMain->add(advcon_Warp, 400, 400);

    advcon_Container = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
    advcon_Container->AddElement(OBJMENU_ADV_CONTAINER_RAND, GETL2S("EditObj_Statue", "Context_Randomize"),
                                 GV->sprIcons16[Icon16_Treasure]);
    advcon_Container->adjustSize();
    advcon_Container->hide();
    advcon_Container->addActionListener(al);
    conMain->add(advcon_Container, 400, 400);

    conmodTilesSelected = new SHR::ContextModel();
    conmodTilesSelected->AddElement(TILMENU_CUT, GETL(Lang_Cut), GV->sprIcons16[Icon16_Cut]);
    conmodTilesSelected->AddElement(TILMENU_COPY, GETL(Lang_Copy), GV->sprIcons16[Icon16_Copy]);
    conmodTilesSelected->AddElement(TILMENU_DELETE, GETL(Lang_Delete), GV->sprIcons16[Icon16_X]);

    conmodTilesPaste = new SHR::ContextModel();
    conmodTilesPaste->AddElement(TILMENU_PASTE, GETL(Lang_Paste), GV->sprIcons16[Icon16_Paste]);

    tilContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
    tilContext->SetModel(conmodTilesSelected);
    tilContext->adjustSize();
    tilContext->addActionListener(al);
    tilContext->hide();
    conMain->add(tilContext, 400, 400);

    GV->IF->EnableCursor(1);

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

    hAppMenu->FixInterfacePositions();

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

    butMicroTileCB->setEnabled(0);
    butMicroObjectCB->setEnabled(0);

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

    butMicroTileCB->setEnabled(1);
    butMicroObjectCB->setEnabled(1);

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
    delete al;
    delete fl;
    delete kl;
    delete vp;
    //if( szHint != 0 )
    // delete [] szHint;
    delete vpMain;
    delete sliVer, sliHor;
    delete conMain;
    delete gui;
    delete vPort;
}

bool State::EditingWW::Think() {
    if (fade_iAction == 0) {
        fade_fAlpha -= hge->Timer_GetDelta() * 512;
        if (fade_fAlpha < 0) {
            fade_iAction++;
            fade_fAlpha = 0;
        }
        return 0;
    } else if (fade_iAction == 1) {
        fade_fTimer += hge->Timer_GetDelta();
        if (fade_fTimer >= 0.5) {
            fade_iAction++;
            fade_fAlpha = 0;
        }
        return 0;
    } else if (fade_iAction == 2) {
        fade_fAlpha += hge->Timer_GetDelta() * 512;
        if (fade_fAlpha > 255) {
            fade_iAction++;
            GV->sprLogoBig->SetColor(0xFFFFFFFF);
            GV->fntMyriad10->SetColor(0xFFFFFFFF);

            ApplicationStartup();
        } else
            return 0;
    }

    if (hServerIPC->Think())
        return 0;

    if (bExit) return 1;

#ifdef BUILD_DEBUG
                                                                                                                            if (hge->Input_KeyDown(HGEK_F1))
		bShowConsole = !bShowConsole;
#endif

    if (hNativeController->ExecutableChanged()) {
        hwinOptions->SyncWithExe();
    }

    if (vstrMapsToLoad.size() != 0) {
        GV->StateMgr->Push(new State::LoadMap(vstrMapsToLoad[0].c_str()));
        vstrMapsToLoad.erase(vstrMapsToLoad.begin());
        return 0;
    }

    if (fDoubleClickTimer != -1) {
        fDoubleClickTimer += hge->Timer_GetDelta();
        if (fDoubleClickTimer > 0.5)
            fDoubleClickTimer = -1;
    }

    if (NewMap_data != 0)
        if (NewMap_data->bKill)
            NewMap_Close();

    if (FirstRun_data != 0)
        if (FirstRun_data->bKill)
            FirstRun_Close();

    if (hParser == NULL || iMode != EWW_MODE_TILE) {
        conResizeDown->setVisible(0);
        conResizeUp->setVisible(0);
        conResizeLeft->setVisible(0);
        conResizeRight->setVisible(0);
    } else {
        if (fCamX < 0 && !(GetActivePlane()->GetFlags() & WWD::Flag_p_XWrapping)) {
            conResizeLeft->setDimension(
                    gcn::Rectangle(vPort->GetX(), vPort->GetY() + 40, -fCamX, vPort->GetHeight() - 80));
            conResizeLeft->setVisible(1);
            butExtLayerDL->setVisible(1);
            butExtLayerUL->setVisible(1);
            int offset = 40 - abs(fCamX);
            butExtLayerLeft->setX(4 - offset);
            butExtLayerDL->setX(4 - offset);
            butExtLayerUL->setX(4 - offset);
        } else {
            conResizeLeft->setVisible(0);
            butExtLayerDL->setVisible(0);
            butExtLayerUL->setVisible(0);
        }

        if (fCamY < 0 && !(GetActivePlane()->GetFlags() & WWD::Flag_p_YWrapping)) {
            conResizeUp->setDimension(gcn::Rectangle(vPort->GetX(), vPort->GetY(), vPort->GetWidth(), -fCamY));
            conResizeUp->setVisible(1);
            int offset = 40 - abs(fCamY);
            butExtLayerUp->setY(4 - offset);
            butExtLayerUL->setY(4 - offset);
            butExtLayerUR->setY(4 - offset);
        } else
            conResizeUp->setVisible(0);

        int maxx = std::max(GetActivePlane()->GetPlaneWidthPx() - vPort->GetWidth() / fZoom, -40.0f);
        if (fCamX > maxx && !(GetActivePlane()->GetFlags() & WWD::Flag_p_XWrapping)) {
            conResizeRight->setDimension(
                    gcn::Rectangle(vPort->GetX() + vPort->GetWidth() - (fCamX - maxx), vPort->GetY() + 40, fCamX - maxx,
                                   vPort->GetHeight() - 80));
            conResizeRight->setVisible(1);
            butExtLayerDR->setVisible(1);
            butExtLayerUR->setVisible(1);
            butExtLayerDR->setX(vPort->GetWidth() - 36 + (40 - (fCamX - maxx)));
            butExtLayerUR->setX(vPort->GetWidth() - 36 + (40 - (fCamX - maxx)));
        } else {
            conResizeRight->setVisible(0);
            butExtLayerDR->setVisible(0);
            butExtLayerUR->setVisible(0);
        }

        int maxy = std::max(GetActivePlane()->GetPlaneHeightPx() - vPort->GetHeight() / fZoom, -40.0f);
        if (fCamY > maxy && !(GetActivePlane()->GetFlags() & WWD::Flag_p_YWrapping)) {
            conResizeDown->setDimension(
                    gcn::Rectangle(vPort->GetX(), vPort->GetY() + vPort->GetHeight() - (fCamY - maxy),
                                   vPort->GetWidth(), fCamY - maxy));
            conResizeDown->setVisible(1);
        } else
            conResizeDown->setVisible(0);
    }

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
        if (!hge->Input_GetKeyState(HGEK_LBUTTON)) {
            bDragWindow = 0;
        } else {
            POINT mouse;
            GetCursorPos(&mouse);
            float dmx, dmy;
            dmx = mouse.x - iWindowDragX;
            dmy = mouse.y - iWindowDragY;

            SetWindowPos(hge->System_GetState(HGE_HWND), NULL, windowDragStartRect.left + dmx,
                         windowDragStartRect.top + dmy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
    }

    if (hge->Input_KeyDown(HGEK_LBUTTON) && conMain->getWidgetAt(mx, my) == NULL && my >= 2 && my < 24) {
        if (mx < hge->System_GetState(HGE_SCREENWIDTH) - 43 && my < 24) {
            bDragWindow = 1;
            POINT mouse;
            GetCursorPos(&mouse);
            iWindowDragX = mouse.x;
            iWindowDragY = mouse.y;
            GetWindowRect(hge->System_GetState(HGE_HWND), &windowDragStartRect);
        }
        if (mx > hge->System_GetState(HGE_SCREENWIDTH) - 5 - 20 - 22 &&
            mx < hge->System_GetState(HGE_SCREENWIDTH) - 5 - 22) //minimize
            ShowWindow(hge->System_GetState(HGE_HWND), SW_MINIMIZE);
        else if (mx > hge->System_GetState(HGE_SCREENWIDTH) - 5 - 20 &&
                 mx < hge->System_GetState(HGE_SCREENWIDTH) - 5) //close
            if (PromptExit()) {
                bExit = 1;
                return 1;
            }
    }

    hAppMenu->Think(conMain->getWidgetAt(mx, my) != NULL);
    MDI->Think(conMain->getWidgetAt(mx, my) != NULL);

    GV->Console->Think();
    if (hge->Input_KeyDown(HGEK_MBUTTON)) {
        cScrollOrientation++;
        if (cScrollOrientation > 2)
            cScrollOrientation = 0;
    }

    /*if( iMode != EWW_MODE_SIMULATION && hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_I) )
	 hAppMenu->SetFolded(!hAppMenu->IsFolded());*/

    if (hParser == NULL) return 0;

    bool bMouseConsumed = 0;
    if (conMain->getWidgetAt(mx, my) != NULL && conMain->getWidgetAt(mx, my) != vPort->GetWidget()) {
        bMouseConsumed = 1;
    }

    if (GV->editState->MDI->GetActiveDoc()->vGuides.size() > 0 && bShowGuideLines) {
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
                int scrpos = (gl.bOrient == GUIDE_HORIZONTAL ? Wrd2ScrY(mainPl, gl.iPos)
                                                             : Wrd2ScrX(mainPl, gl.iPos));
                if (gl.bOrient == GUIDE_HORIZONTAL &&
                    (scrpos < vPort->GetY() || scrpos > vPort->GetY() + vPort->GetHeight()) ||
                    gl.bOrient == GUIDE_VERTICAL &&
                    (scrpos < vPort->GetX() || scrpos > vPort->GetX() + vPort->GetWidth()))
                    continue;
                if ((gl.bOrient == GUIDE_HORIZONTAL && (my > scrpos - 5 && my < scrpos + 5) ||
                     gl.bOrient == GUIDE_VERTICAL && (mx > scrpos - 5 && mx < scrpos + 5)) &&
                    iActiveTool != EWW_TOOL_MOVEOBJECT && iActiveTool != EWW_TOOL_EDITOBJ) {
                    bMouseConsumed = 1;
                    bShowHand = 1;
                    if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                        iManipulatedGuide = i;
                        break;
                    }
                }
            }
        }

        if (iManipulatedGuide != -1) {
            stGuideLine gl = GV->editState->MDI->GetActiveDoc()->vGuides[GV->editState->iManipulatedGuide];
            if (gl.iPos < 0 && gl.iPos != -100)
                bShowHand = 1;
            if (conMain->getWidgetAt(mx, my) == vPort->GetWidget()) {
                if (hge->Input_GetKeyState(HGEK_LBUTTON)) {
                    gl.iPos = (gl.bOrient == GUIDE_HORIZONTAL ? Scr2WrdY(mainPl, my)
                                                              : Scr2WrdX(mainPl, mx));
                    GV->editState->MDI->GetActiveDoc()->vGuides[GV->editState->iManipulatedGuide] = gl;
                    bShowHand = 1;
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
            bMouseConsumed = 1;
        }
    }

    for (auto & hWindow : hWindows)
        hWindow->Think();

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

    bool scrollcatched = 0;
    bool hotkeyused =
            hge->Input_GetKeyState(HGEK_SHIFT) || hge->Input_GetKeyState(HGEK_CTRL) || hge->Input_GetKeyState(HGEK_ALT);
    if (mx > vPort->GetX() && mx < vPort->GetX() + vPort->GetWidth() && my > vPort->GetY() &&
        my < vPort->GetY() + vPort->GetHeight() && !bMouseConsumed) {
        if (cScrollOrientation == 0 && !hotkeyused || hge->Input_GetKeyState(HGEK_SHIFT)) {
            fCamY -= hge->Input_GetMouseWheel() * 32;
            scrollcatched = 1;
        } else if (cScrollOrientation == 1 && !hotkeyused || hge->Input_GetKeyState(HGEK_CTRL)) {
            fCamX -= hge->Input_GetMouseWheel() * 32;
            scrollcatched = 1;
        }
    }

    if ((cScrollOrientation == 2 && !hotkeyused || hge->Input_GetKeyState(HGEK_ALT)) && !scrollcatched) {
        float fOldZoom = fZoom;
        if (conMain->getWidgetAt(mx, my) == vPort->GetWidget() && !bMouseConsumed)
            (GV->bSmoothZoom ? fDestZoom : fZoom) += hge->Input_GetMouseWheel() * 0.10;

        if (fDestZoom < 0.1) fDestZoom = 0.1;
        if (fDestZoom > 2) fDestZoom = 2;

        if (GV->bSmoothZoom && fZoom != fDestZoom) {
            float fChange = fDestZoom - fZoom;
            if (fChange > 0) fZoom += std::max(fChange * (hge->Timer_GetDelta() * 2), float(0.005));
            else fZoom += std::min(fChange * (hge->Timer_GetDelta() * 2), float(-0.005));
            if (fChange > 0 && fZoom > fDestZoom || fChange < 0 && fZoom < fDestZoom)
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
            if (objContext->isVisible()) objContext->setVisible(0);
            if (tilContext->isVisible()) tilContext->setVisible(0);
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

    if (mx > vPort->GetX() && mx < vPort->GetX() + vPort->GetWidth() && my > vPort->GetY() &&
        my < vPort->GetY() + vPort->GetHeight() && !bMouseConsumed && hge->Input_KeyDown(HGEK_RBUTTON)) {
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        bDragDropScroll = 1;
        fDragLastMx = mx;
        fDragLastMy = my;
    }


    if (hge->Input_KeyUp(HGEK_RBUTTON)) {
        bDragDropScroll = 0;
    }


    if (bDragDropScroll) {
        fCamX -= ((mx - fDragLastMx) / fZoom) / (GetActivePlane()->GetMoveModX() / 100.0f);
        fCamY -= ((my - fDragLastMy) / fZoom) / (GetActivePlane()->GetMoveModY() / 100.0f);
        fDragLastMx = mx;
        fDragLastMy = my;
    }

    if (mx > hge->System_GetState(HGE_SCREENWIDTH) - 11 &&
        my > hge->System_GetState(HGE_SCREENHEIGHT) - 31 &&
        my < hge->System_GetState(HGE_SCREENHEIGHT) - 21
        /*cScrollOrientation != iHintID+1*/) {
        switch (cScrollOrientation) {
            case 0:
                SetHint(GETL(Lang_ScrollScrollsVerticaly));
                break;
            case 1:
                SetHint(GETL(Lang_ScrollScrollsHorizontaly));
                break;
            case 2:
                SetHint(GETL(Lang_ScrollZooms));
                break;
        }
    }
    //SHORTCUTS
    HandleHotkeys();

    if (!bMouseConsumed && (vObjectsPicked.size() == 0 ||
                            !hge->Input_GetKeyState(HGEK_CTRL) && !hge->Input_GetKeyState(HGEK_SHIFT) &&
                            !hge->Input_GetKeyState(HGEK_ALT))) {
        if (fCameraMoveTimer == 0 || fCameraMoveTimer > 0.1) {
            if (fCameraMoveTimer > 0.1) fCameraMoveTimer -= 0.1;
            if (vPort->GetWidget()->isFocused() && bWindowFocused) {
                if (hge->Input_GetKeyState(HGEK_UP)) {
                    fCamY -= (int(fCamY) % GetActivePlane()->GetTileHeight() != 0 ? int(fCamY) %
                                                                                    GetActivePlane()->GetTileHeight()
                                                                                  : GetActivePlane()->GetTileHeight());
                } else if (hge->Input_GetKeyState(HGEK_DOWN)) {
                    fCamY += (int(fCamY) % GetActivePlane()->GetTileHeight() != 0 ? int(fCamY) %
                                                                                    GetActivePlane()->GetTileHeight()
                                                                                  : GetActivePlane()->GetTileHeight());
                }
                if (hge->Input_GetKeyState(HGEK_LEFT)) {
                    fCamX -= (int(fCamX) % GetActivePlane()->GetTileWidth() != 0 ? int(fCamX) %
                                                                                   GetActivePlane()->GetTileWidth()
                                                                                 : GetActivePlane()->GetTileWidth());
                } else if (hge->Input_GetKeyState(HGEK_RIGHT)) {
                    fCamX += (int(fCamX) % GetActivePlane()->GetTileWidth() != 0 ? int(fCamX) %
                                                                                   GetActivePlane()->GetTileWidth()
                                                                                 : GetActivePlane()->GetTileWidth());
                }
            }
        }
        if (hge->Input_GetKeyState(HGEK_UP) || hge->Input_GetKeyState(HGEK_DOWN) || hge->Input_GetKeyState(HGEK_LEFT) ||
            hge->Input_GetKeyState(HGEK_RIGHT))
            fCameraMoveTimer += hge->Timer_GetDelta();
        else
            fCameraMoveTimer = 0;
    }

    int xresize = 0, yresize = 0;
    if (!(GetActivePlane()->GetFlags() & WWD::Flag_p_XWrapping) && iMode == EWW_MODE_TILE)
        xresize = 40;
    if (!(GetActivePlane()->GetFlags() & WWD::Flag_p_YWrapping) && iMode == EWW_MODE_TILE)
        yresize = 40;

    if (fCamX < -xresize) fCamX = -xresize;
    if (fCamY < -yresize) fCamY = -yresize;

    if (!(GetActivePlane()->GetFlags() & WWD::Flag_p_XWrapping))
        if (fCamX > GetActivePlane()->GetPlaneWidthPx() - vPort->GetWidth() / fZoom + xresize)
            fCamX = std::max(GetActivePlane()->GetPlaneWidthPx() - vPort->GetWidth() / fZoom + xresize,
                             float(-xresize));

    if (!(GetActivePlane()->GetFlags() & WWD::Flag_p_YWrapping))
        if (fCamY > GetActivePlane()->GetPlaneHeightPx() - vPort->GetHeight() / fZoom + yresize)
            fCamY = std::max(GetActivePlane()->GetPlaneHeightPx() - vPort->GetHeight() / fZoom + yresize,
                             float(-yresize));

    sliHor->setValue(fCamX);
    sliVer->setValue(sliVer->getScaleEnd() - fCamY);

    if (fCamLastX != fCamX || fCamLastY != fCamY) {
        vPort->MarkToRedraw(1);
        fCamLastX = fCamX;
        fCamLastY = fCamY;
    }
    if (fLastZoom != fZoom) {
        vPort->MarkToRedraw(1);
        fLastZoom = fZoom;
    }

    if (bConstRedraw)
        vPort->MarkToRedraw(1);
    return 0;
}

void State::EditingWW::GainFocus(int iReturnCode, bool bFlipped) {
    if (iReturnCode == 0) return;
    if (((returnCode *) iReturnCode)->Type == RC_LoadMap) {
        if (((returnCode *) iReturnCode)->Ptr != 0) {
            DocumentData *dd = (DocumentData *) ((returnCode *) iReturnCode)->Ptr;
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
            char *filename = SHR::GetFile(hParser->GetFilePath());
            SetHint("%s: %s", GETL2S("Hints", "FileOpened"), filename);
            delete[] filename;
            vPort->MarkToRedraw(1);

            if (strlen(hParser->GetFilePath()) > 0) {
                char *n = SHR::GetFile(hParser->GetFilePath());
                bool bdots = 0;
                while (GV->fntMyriad13->GetStringWidth(n) > 140) {
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
                if (!labwsRecently) {
                    labwsRecently = new SHR::Lab(n);
                    delete[] n;
                    labwsRecently->adjustSize();
                    winWelcome->add(labwsRecently, 375 - labwsRecently->getWidth() / 2, 108);
                } else {
                    labwsRecently->setCaption(n);
                    labwsRecently->adjustSize();
                    labwsRecently->setPosition(375 - labwsRecently->getWidth() / 2, 108);
                }
                butwsRecently->setEnabled(1);
                winWelcome->setY(vPort->GetY() + vPort->GetHeight() / 2 - winWelcome->getHeight() / 2);
            }
        } else {
            MDI->UnlockMapReload();
        }
    } else if (((returnCode *) iReturnCode)->Type == RC_ErrorState) {
        if (((returnCode *) iReturnCode)->Ptr == RETURN_YES) {
            exit(0);
        }
    } else if (((returnCode *) iReturnCode)->Type == RC_MapShot) {
        if (((returnCode *) iReturnCode)->Ptr == 1) {
            winMapShot->setVisible(0);
            if (cbmsOpenFileDirectory->isSelected()) {
                char *command = new char[strlen(tfmsSaveAs->getText().c_str()) + 32];
                sprintf(command, "Explorer.exe /select,%s", tfmsSaveAs->getText().c_str());
                system(command);
                delete[] command;
            }
        }
    } else if (((returnCode *) iReturnCode)->Type == RC_ObjectProp) {
        if (bFlipped) {
            int *ptr = (int *) (((returnCode *) iReturnCode)->Ptr);
            toolsaMinX = ptr[0];
            toolsaMinY = ptr[1];
            toolsaMaxX = ptr[2];
            toolsaMaxY = ptr[3];
            delete[] ptr;
            SetTool(EWW_TOOL_OBJSELAREA);
        } else if (((returnCode *) iReturnCode)->Ptr == 1) {
            objContext->EmulateClickID(OBJMENU_MOVE);
        } else if (((returnCode *) iReturnCode)->Ptr == 2) {
            GetActivePlane()->DeleteObject(vObjectsPicked[0]);
        }
    }
    delete ((returnCode *) iReturnCode);
}

WWD::Plane *State::EditingWW::GetActivePlane() {
    return hParser->GetPlane(hmbTile->ddActivePlane->getSelected());
}

int State::EditingWW::GetActivePlaneID() {
    return hmbTile->ddActivePlane->getSelected();
}

void State::EditingWW::SetHint(const char *pszFormat, ...) {
    va_list args;
            va_start(args, pszFormat);
    vsprintf(szHint, pszFormat, args);
            va_end(args);
    fHintTime = 0;
    GV->Console->Printf("~w~Action: ~y~%s", szHint);
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

void State::EditingWW::SetTool(int iNTool) {
    CloseTool(iNTool);
    OpenTool(iNTool);
    iActiveTool = iNTool;
}

void State::EditingWW::OpenTool(int iTool) {
    if (iTool == EWW_TOOL_PENCIL || iTool == EWW_TOOL_FILL) {
        iActiveTool = iTool;
        iTileDrawMode = EWW_DRAW_POINT;
        RebuildTilePicker(true);
        winTilePicker->setVisible(true);
        return;
    } else if (iTool == EWW_TOOL_WRITEID) {
        hmbTile->butIconWriteID->setHighlight(true);
        iTileWriteIDx = iTileWriteIDy = -1;
    } else if (iTool == EWW_TOOL_SPACEOBJ) {
        winSpacing->setVisible(true);
        winSpacing->requestMoveToTop();
    } else if (iTool == EWW_TOOL_BRUSH) {
        iActiveTool = iTool;
        iTileDrawMode = EWW_DRAW_POINT;
        RebuildTilePicker(true);
        if (!winTilePicker->isVisible())
            winTilePicker->setVisible(true);
        iLastBrushPlacedX = iLastBrushPlacedY = -1;
        return;
    } else if (iTool == EWW_TOOL_BRUSHOBJECT) {
        winObjectBrush->setVisible(true);
        cbobrApplyScatterSeparately->setEnabled(vObjectsBrushCB.size() > 1);
    } else if (iTool == EWW_TOOL_MEASURE) {
        winMeasureOpt->setVisible(true);
    } else if (iTool == EWW_TOOL_OBJSELAREA) {
        bObjDragSelection = false;
        wintoolSelArea->setPosition(vPort->GetX(), vPort->GetY() + vPort->GetHeight() - 125);
        wintoolSelArea->setShow(true);
        wintoolSelArea->unDrag();
        char label[200];
        sprintf(label, "~w~X1: ~y~%d~w~, Y1: ~y~%d~w~, X2: ~y~%d~w~, Y2: ~y~%d~w~",
                toolsaMinX, toolsaMinY, toolsaMaxX, toolsaMaxY);
        labtoolSelAreaValues->setCaption(label);
        labtoolSelAreaValues->adjustSize();
        toolsaAction = TOOL_OBJSA_NONE;
        LockToolSpecificFunctions(true);
    } else if (iTool == EWW_TOOL_EDITOBJ) {
        MinimizeWindows();
        LockToolSpecificFunctions(true);
        bEditObjDelete = false;
    } else if (iTool == EWW_TOOL_ALIGNOBJ) {

    } else if (iTool == EWW_TOOL_MOVEOBJECT)
        bEditObjDelete = false;
}

void State::EditingWW::CloseTool(int iTool) {
    if (iActiveTool == EWW_TOOL_PENCIL || iActiveTool == EWW_TOOL_FILL) {
        if (iTool != EWW_TOOL_PENCIL && iTool != EWW_TOOL_FILL) {
            iTilePicked = EWW_TILE_NONE;
            if (iTool != EWW_TOOL_BRUSH) {
                winTilePicker->setVisible(false);
            }
        }
        wintpFillColor->setVisible(false);
        vTileGhosting.clear();
        return;
    } else if (iActiveTool == EWW_TOOL_WRITEID) {
        hmbTile->butIconWriteID->setHighlight(0);
        vTileGhosting.clear();
    } else if (iActiveTool == EWW_TOOL_SPACEOBJ) {
        winSpacing->setVisible(0);
    } else if (iActiveTool == EWW_TOOL_BRUSH) {
        if (iTool != EWW_TOOL_PENCIL && iTool != EWW_TOOL_FILL) {
            winTilePicker->setVisible(false);
        }

        if (iTilePicked >= 0 && MDI->GetActiveDoc() != NULL)
            hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetBrushByIterator(
                    iTilePicked)->RemoveSettingsFromContainer(winTilePicker);
        vTileGhosting.clear();
        return;
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

    }
}

void State::EditingWW::MinimizeWindows() {
    for (int i = 0; i < vWidgetsToMinimalise.size(); i++) {
        if (vWidgetsToMinimalise[i]->isVisible()) {
            vWidgetsToMinimalise[i]->setVisible(0);
            vMinimalisedWidgets.push_back(vWidgetsToMinimalise[i]);
        }
    }
    if (NewMap_data != 0)
        NewMap_data->bKill = 1;
}

void State::EditingWW::MaximizeWindows() {
    for (int i = 0; i < vMinimalisedWidgets.size(); i++)
        vMinimalisedWidgets[i]->setVisible(1);
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
    bool condx = !(GetActivePlane()->GetFlags() & WWD::Flag_p_XWrapping) && iMode == EWW_MODE_TILE,
         condy = !(GetActivePlane()->GetFlags() & WWD::Flag_p_YWrapping) && iMode == EWW_MODE_TILE;
    if (condx)
        sliHor->setScaleStart(-40);
    else
        sliHor->setScaleStart(0);
    if (condy)
        sliVer->setScaleStart(-40);
    else
        sliVer->setScaleStart(0);
    sliHor->setScaleEnd(
            std::max(GetActivePlane()->GetPlaneWidthPx() - vPort->GetWidth() / fZoom + 40 * (condx), -40.0f));
    sliVer->setScaleEnd(
            std::max(GetActivePlane()->GetPlaneHeightPx() - vPort->GetHeight() / fZoom + 40 * (condy), -40.0f));
    sliHor->adjustMarkerLength();
    sliVer->adjustMarkerLength();
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
        bool ok = 0;
        try {
            MDI->PrepareDocToSave(MDI->GetActiveDocIt());
            hParser->CompileToFile(szFileopen, 1);
            hDataCtrl->RelocateDocument(szFileopen);
            ok = 1;
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
            MDI->GetActiveDoc()->bSaved = 1;
            SetHint("%s: %s", GETL2S("Hints", "FileSavedAs"), fl);
            delete[] fl;
            MDI->UpdateCrashList();
        }
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
        bObjDragSelection = 0;
        SwitchActiveModeMenuBar(hmbObject);
        cbutActiveMode->setSelectedEntryID(1);
        UpdateScrollBars();
    }
}

void State::EditingWW::ExitMode(int piMode) {
    if (piMode == EWW_MODE_TILE) {
        SwitchActiveModeMenuBar(NULL);
        if (iTileSelectX1 != -1)
            vPort->MarkToRedraw(1);
        iTileSelectX1 = iTileSelectX2 = iTileSelectY1 = iTileSelectY2 = -1;
        tilContext->setVisible(0);
    } else if (piMode == EWW_MODE_OBJECT) {
        objContext->setVisible(0);
        winSearchObj->setVisible(0);
        SwitchActiveModeMenuBar(NULL);
        vObjectsPicked.clear();
    }
}

SHR::But *State::EditingWW::MakeButton(int x, int y, EnumGfxIcons icon, SHR::Container *dest, bool enable, bool visible,
                                       gcn::ActionListener *actionListener) {
    SHR::But *but = new SHR::But(GV->hGfxInterface, GV->sprIcons[icon]);
    but->setDimension(gcn::Rectangle(0, 0, 32, 32));
    but->addActionListener(actionListener ? actionListener : al);
    but->setEnabled(enable);
    but->setVisible(visible);
    dest->add(but, x, y);
    return but;
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
    x = x / fZoom;
    return x;
}

int State::EditingWW::Scr2WrdY(WWD::Plane *pl, int py) {
    float y = int(fCamY * (pl->GetMoveModY() / 100.0f) * fZoom) + py - vPort->GetY();
    y = y / fZoom;
    return y;
}

int State::EditingWW::Wrd2ScrX(WWD::Plane *pl, int x) {
    x = x * fZoom;
    x += vPort->GetX();
    x -= int(fCamX * (pl->GetMoveModX() / 100.0f) * fZoom);
    return x;
}

int State::EditingWW::Wrd2ScrY(WWD::Plane *pl, int y) {
    y = y * fZoom;
    y += vPort->GetY();
    y -= int(fCamY * (pl->GetMoveModY() / 100.0f) * fZoom);
    return y;
}

int State::EditingWW::Wrd2ScrXrb(WWD::Plane *pl, int x) {
    x = x * fZoom;
    x -= int(fCamX * (pl->GetMoveModX() / 100.0f) * fZoom);
    return x;
}

int State::EditingWW::Wrd2ScrYrb(WWD::Plane *pl, int y) {
    y = y * fZoom;
    y -= int(fCamY * (pl->GetMoveModY() / 100.0f) * fZoom);
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
    hTempAttrib = new WWD::TileAttrib(attrib);
    char tmp[64];
    sprintf(tmp, "%d", attrib->GetW());
    tftpW->setText(tmp);
    sprintf(tmp, "%d", attrib->GetH());
    tftpH->setText(tmp);

    int inside = attrib->GetAtribInside();
    if (inside < 5) {
        rbtpIn[inside]->setSelected(1);
    }

    if (attrib->GetType() == WWD::AttribType_Single) {
        rbtpSingle->setSelected(1);
        tftpX1->setVisible(0);
        tftpY1->setVisible(0);
        tftpX2->setVisible(0);
        tftpY2->setVisible(0);
        for (int i = 0; i < 5; i++)
            rbtpOut[i]->setVisible(0);
        winTileProp->setDimension(gcn::Rectangle(dx, dy, 300, 260));
    } else if (attrib->GetType() == WWD::AttribType_Double) {
        rbtpDouble->setSelected(1);
        tftpX1->setVisible(1);
        tftpY1->setVisible(1);
        tftpX2->setVisible(1);
        tftpY2->setVisible(1);
        for (int i = 0; i < 5; i++)
            rbtpOut[i]->setVisible(1);
        sprintf(tmp, "%d", attrib->GetMask().x1);
        tftpX1->setText(tmp);
        sprintf(tmp, "%d", attrib->GetMask().y1);
        tftpY1->setText(tmp);
        sprintf(tmp, "%d", attrib->GetMask().x2);
        tftpX2->setText(tmp);
        sprintf(tmp, "%d", attrib->GetMask().y2);
        tftpY2->setText(tmp);
        winTileProp->setDimension(gcn::Rectangle(dx, dy, 300, 350));
        rbtpOut[attrib->GetAtribOutside()]->setSelected(1);
    }
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
    vPort->MarkToRedraw(1);
}

void State::EditingWW::PrepareForDocumentSwitch() {
    if (hParser == NULL) return;
    MDI->GetActiveDoc()->fCamX = fCamX;
    MDI->GetActiveDoc()->fCamY = fCamY;
    MDI->GetActiveDoc()->iSelectedPlane = hmbTile->ddActivePlane->getSelected();
    MDI->GetActiveDoc()->vObjectsPicked = vObjectsPicked;
    MDI->GetActiveDoc()->fZoom = fZoom;
    MDI->GetActiveDoc()->fDestZoom = fZoom;
    MDI->GetActiveDoc()->hPlaneData = hPlaneData;
    MDI->GetActiveDoc()->hStartingPosObj = hStartingPosObj;
}

void State::EditingWW::SyncLogicBrowser() {
    if (tfbrlRename->isVisible()) {
        tfbrlRename->setVisible(0);
        butbrlRenameOK->setVisible(0);
        labbrlLogicNameV->setVisible(1);
    }
    bool validselect = (lbbrlLogicList->getSelected() >= 0 &&
                        lbbrlLogicList->getSelected() < hCustomLogics->GetLogicsCount());
    if (validselect) {
        cCustomLogic *logic = hCustomLogics->GetLogicByIterator(lbbrlLogicList->getSelected());
        cFile f = logic->GetFile();
        labbrlLogicNameV->setCaption(logic->GetName());
        std::string path = f.hFeed->GetAbsoluteLocation() + '\\' + logic->GetFile().strPath;
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
    } else {
        labbrlLogicNameV->setCaption("");
        labbrlFilePathV->setCaption("");
        labbrlFileSizeV->setCaption("");
        labbrlFileChecksumV->setCaption("");
        labbrlFileModDateV->setCaption("");
    }

    butbrlRename->setEnabled(validselect);
    butbrlDelete->setEnabled(validselect);
    //butbrlEditExternal->setEnabled(validselect);
    butbrlEdit->setEnabled(validselect);

    labbrlLogicNameV->adjustSize();
    labbrlFileSizeV->adjustSize();
    labbrlFileChecksumV->adjustSize();
    labbrlFileModDateV->adjustSize();
    labbrlFilePathV->adjustSize();
}

void State::EditingWW::DocumentSwitched() {
    sliVer->setVisible(MDI->GetActiveDoc() != NULL);
    sliHor->setVisible(MDI->GetActiveDoc() != NULL);
    winWelcome->setVisible(MDI->GetActiveDoc() == NULL);
    sliZoom->setVisible(MDI->GetActiveDoc() != NULL);
    if (MDI->GetActiveDoc() == NULL) {
        winLogicBrowser->setVisible(0);
        if (conCrashRetrieve != 0 && conCrashRetrieve->isVisible()) {
            conCrashRetrieve->setVisible(0);
            winWelcome->remove(conCrashRetrieve);
            conRecentFiles->setVisible(1);
            winWelcome->add(conRecentFiles, winWelcome->getWidth() / 2 - conRecentFiles->getWidth() / 2,
                            winWelcome->getHeight() + 20);
            winWelcome->setY(vPort->GetY() + vPort->GetHeight() / 2 - winWelcome->getHeight() / 2);
        }
        SetTool(EWW_TOOL_NONE);
        SwitchActiveModeMenuBar(NULL);
        hParser = NULL;
        ddpmTileSet->setListModel(0);
        lbbrlLogicList->setListModel(0);
        InitEmpty();
        vPort->MarkToRedraw(1);
        hAppMenu->SyncDocumentSwitched();
        hNativeController->SyncDocumentSwitched();
        hDataCtrl = NULL;
        for (cWindow* win : hWindows) {
            win->Close();
        }
        return;
    }

    cbutActiveMode->setVisible(1);

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
    if (winLogicBrowser->isVisible())
        SyncLogicBrowser();

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
    vPort->MarkToRedraw(1);

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
    if (hTileset->GetSet("ACTION") != 0 && hTileset->GetSet("ACTION")->GetTilesCount() > 0)
        itpSelectedTile = hTileset->GetSet("ACTION")->GetTileByIterator(0)->GetID();
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
    tfmsSaveAs->setText(pathtemp, 1);

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
}

void State::EditingWW::SetZoom(float fZ) {
    if (GV->bSmoothZoom) {
        fDestZoom = fZ;
    } else {
        fDestZoom = fZoom = fZ;
    }
}

void State::EditingWW::FileDropped() {
    for (auto filepath : hge->System_GetDroppedFiles()) {
        char *ext = SHR::ToLower(strrchr(filepath, '.') + 1);
        if (!strcmp(ext, "wwd")) {
            GV->StateMgr->Push(new State::LoadMap(filepath));
        }
        delete[] ext;
    }
    //if( butIconOpen->isEnabled() )
    // GV->StateMgr->Push(new State::LoadMap(szPath));
}

void State::EditingWW::ApplicationStartup() {
    if (GV->szCmdLine[0] != '\0') {
        if (GV->szCmdLine[0] == '"') {
            int str_length = GV->szCmdLine.length() - 1;
            char *tmp = new char[str_length];
            for (int i = 1; i < str_length; i++)
                tmp[i - 1] = GV->szCmdLine[i];
            tmp[str_length - 1] = '\0';
            GV->StateMgr->Push(new State::LoadMap(tmp));
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
            GV->StateMgr->Push(new State::LoadMap(GV->szCmdLine.c_str()));
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
                new State::Error(GETL(Lang_Error), GETL2S("MapProperties", "NoName"), ST_ER_ICON_FATAL, ST_ER_BUT_OK,
                                 0));
        return;
    } else if (strlen(tfwpName->getText().c_str()) > 64) {
        GV->StateMgr->Push(new State::Error(GETL(Lang_Error), GETL2S("MapProperties", "TooLongName"), ST_ER_ICON_FATAL,
                                            ST_ER_BUT_OK, 0));
        return;
    }
    if (!strlen(tfwpAuthor->getText().c_str())) {
        GV->StateMgr->Push(
                new State::Error(GETL(Lang_Error), GETL2S("MapProperties", "NoAuthor"), ST_ER_ICON_FATAL, ST_ER_BUT_OK,
                                 0));
        return;
    } else if (strlen(tfwpAuthor->getText().c_str()) > 64) {
        GV->StateMgr->Push(
                new State::Error(GETL(Lang_Error), GETL2S("MapProperties", "TooLongAuthor"), ST_ER_ICON_FATAL,
                                 ST_ER_BUT_OK, 0));
        return;
    }
    if (!strlen(tfwpREZ->getText().c_str())) {
        GV->StateMgr->Push(
                new State::Error(GETL(Lang_Error), GETL2S("MapProperties", "NoRez"), ST_ER_ICON_FATAL, ST_ER_BUT_OK,
                                 0));
        return;
    } else if (strlen(tfwpREZ->getText().c_str()) > 256) {
        GV->StateMgr->Push(new State::Error(GETL(Lang_Error), GETL2S("MapProperties", "TooLongRez"), ST_ER_ICON_FATAL,
                                            ST_ER_BUT_OK, 0));
        return;
    }
    if (!strlen(tfwpTiles->getText().c_str())) {
        GV->StateMgr->Push(
                new State::Error(GETL(Lang_Error), GETL2S("MapProperties", "NoTiles"), ST_ER_ICON_FATAL, ST_ER_BUT_OK,
                                 0));
        return;
    } else if (strlen(tfwpTiles->getText().c_str()) > 128) {
        GV->StateMgr->Push(new State::Error(GETL(Lang_Error), GETL2S("MapProperties", "TooLongTiles"), ST_ER_ICON_FATAL,
                                            ST_ER_BUT_OK, 0));
        return;
    }
    if (!strlen(tfwpPalette->getText().c_str())) {
        GV->StateMgr->Push(
                new State::Error(GETL(Lang_Error), GETL2S("MapProperties", "NoPalette"), ST_ER_ICON_FATAL, ST_ER_BUT_OK,
                                 0));
        return;
    } else if (strlen(tfwpPalette->getText().c_str()) > 128) {
        GV->StateMgr->Push(
                new State::Error(GETL(Lang_Error), GETL2S("MapProperties", "TooLongPalette"), ST_ER_ICON_FATAL,
                                 ST_ER_BUT_OK, 0));
        return;
    }
    if (!strlen(tfwpExe->getText().c_str())) {
        GV->StateMgr->Push(
                new State::Error(GETL(Lang_Error), GETL2S("MapProperties", "NoExe"), ST_ER_ICON_FATAL, ST_ER_BUT_OK,
                                 0));
        return;
    } else if (strlen(tfwpExe->getText().c_str()) > 128) {
        GV->StateMgr->Push(new State::Error(GETL(Lang_Error), GETL2S("MapProperties", "TooLongExe"), ST_ER_ICON_FATAL,
                                            ST_ER_BUT_OK, 0));
        return;
    }
    winWorld->setVisible(false);
    bool bchange = 0;
    if (strcmp(hParser->GetName(), tfwpName->getText().c_str()) != 0) {
        bchange = 1;
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
        int maxW = 0;
        for (int i = 0; i < hMruList->GetFilesCount(); i++) {
            bool bAddSign = 1;
            int iSignStart = 0;
            char tmp[MAX_PATH];
            strcpy(tmp, hMruList->GetRecentlyUsedFile(i));
            while (GV->fntMyriad13->GetStringWidth(tmp) > 370) {
                int len = strlen(tmp);
                if (bAddSign) {
                    tmp[int(len / 4) - 2] = '(';
                    tmp[int(len / 4) - 1] = '.';
                    tmp[int(len / 4)] = '.';
                    tmp[int(len / 4) + 1] = '.';
                    tmp[int(len / 4) + 2] = ')';
                    bAddSign = 0;
                    iSignStart = int(len / 4) - 2;
                }
                for (int it = iSignStart - 3; it <= len - 3; it++)
                    tmp[it] = tmp[it + 3];
                for (int it = iSignStart + 2; it < len - 3; it++)
                    tmp[it] = tmp[it + 3];
                iSignStart -= 3;
            }

            WWD::GAME gt;
            int ibase = 0;
            try {
                gt = WWD::GetGameTypeFromFile(tmp, &ibase);
            }
            catch (...) {
                gt = WWD::Game_Unknown;
            }

            gamesLastOpened[i] = gt;
            if (gt == WWD::Game_Claw && ibase > 0 && ibase < 15) {
                gamesLastOpened[i] = 50 + ibase;
            }

            hgeSprite *ico = (gt == WWD::Game_Claw && ibase > 0 && ibase < 15 ? GV->sprLevelsMicro16[ibase - 1]
                                                                              : GV->sprGamesSmall[gt]);

            lnkLastOpened[i] = new SHR::Link(tmp, ico);
            lnkLastOpened[i]->adjustSize();
            lnkLastOpened[i]->setEnabled(gt != WWD::Game_Unknown);
            lnkLastOpened[i]->addActionListener(al);
            conRecentFiles->add(lnkLastOpened[i], 5, 25 + i * 20);
            int nw = lnkLastOpened[i]->getWidth() + 25;
            if (nw > maxW) maxW = nw;
        }
        conRecentFiles->setWidth(maxW);
        conRecentFiles->setHeight(25 + hMruList->GetFilesCount() * 20);
    }
    GV->Console->Printf("Reloading main context...");
    hAppMenu->SyncMRU();
    GV->Console->Printf("Done.");
}

bool State::EditingWW::ValidateLevelName(const char *name, bool bAllowNoNum) {
    bool bFoundNum = 0;
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
    if (ValidateLevelName(name, 0)) {
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

    vPort->MarkToRedraw(true);
}
