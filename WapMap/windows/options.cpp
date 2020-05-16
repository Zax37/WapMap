#include "options.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../../shared/commonFunc.h"
#include "../cInterfaceSheet.h"
#include "../../shared/gcnWidgets/wIconListbox.h"
#include "../cNativeController.h"

extern HGE *hge;

cListModelDisplay::cListModelDisplay(bool bCrazyHook) {
    if (bCrazyHook) {
        const char *cptr = GETL2S("Options", "Default");
        m_vOpt.push_back(std::string(cptr));
    }
    std::vector<SHR::DisplayMode> displays = SHR::GetDisplayModes();
    for (int i = 0; i < displays.size(); i++) {
        if (displays[i].iDepth == 32 && ((bCrazyHook && displays[i].iWidth >= 640 && displays[i].iHeight >= 480) ||
                                         (!bCrazyHook && displays[i].iWidth >= 1024 && displays[i].iHeight >= 768))) {
            char tmp[256];
            sprintf(tmp, "%dx%d", displays[i].iWidth, displays[i].iHeight);
            bool already = 0;
            for (int x = 0; x < m_vOpt.size(); x++)
                if (!strcmp(tmp, m_vOpt[x].c_str())) {
                    already = 1;
                    break;
                }
            if (already) continue;
            m_vOpt.push_back(std::string(tmp));
        }
    }
}

cListModelDisplay::~cListModelDisplay() {

}

std::string cListModelDisplay::getElementAt(int i) {
    if (i >= 0 && i < m_vOpt.size())
        return m_vOpt[i];
    return "";
}

int cListModelDisplay::getNumberOfElements() {
    return m_vOpt.size();
}


winOptions::winOptions() {
    myWin = new SHR::Win(&GV->gcnParts, GETL(Lang_Options));
    myWin->setClose(true);
    myWin->setVisible(false);
    myWin->addActionListener(this);

    vp = new WIDG::Viewport(this, 0);
    myWin->add(vp, 0, 0);

    settingsCategoriesList = new SHR::IconListBox(this);
    settingsCategoriesList->adjustSize();
    settingsCategoriesList->setWidth(140);
    settingsCategoriesList->addActionListener(this);
    scrollAreaCategories = new SHR::ScrollArea(settingsCategoriesList, SHR::ScrollArea::SHOW_NEVER, SHR::ScrollArea::SHOW_NEVER);
    scrollAreaCategories->setDimension(settingsCategoriesList->getDimension());
    myWin->add(scrollAreaCategories, 0, 8);
    myWin->setHeight(24 + settingsCategoriesList->getHeight());

    auto* conWapMap = new SHR::Container();
    conWapMap->setOpaque(false);
    {
        int xOffset = 8, yOffset = 10;
        labLang = new SHR::Lab(GETL(Lang_Language));
        labLang->adjustSize();
        conWapMap->add(labLang, xOffset, yOffset);

        lmLang = new cDynamicListModel();

        ddoptLang = new SHR::DropDown();
        ddoptLang->setListModel(lmLang);
        ddoptLang->setDimension(gcn::Rectangle(0, 0, 150, 20));
        ddoptLang->addActionListener(this);
        ddoptLang->SetGfx(&GV->gcnParts);
        ddoptLang->adjustHeight();
        conWapMap->add(ddoptLang, xOffset + 130, yOffset - 1);

        HANDLE hFind = INVALID_HANDLE_VALUE;
        WIN32_FIND_DATA fdata;
        hFind = FindFirstFile("lang/*", &fdata);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (fdata.cFileName[0] != '.') {
                    std::string filen = fdata.cFileName;
                    filen = filen.substr(0, filen.rfind('.'));
                    lmLang->Add(filen);
                }
            } while (FindNextFile(hFind, &fdata) != 0);
        }

        for (int i = 0; i < lmLang->getNumberOfElements(); i++) {
            if (!strcmp(lmLang->getElementAt(i).c_str(), GV->Lang->GetName())) {
                ddoptLang->setSelected(i);
                break;
            }
        }

        yOffset += 22;

        labChangesLang = new SHR::Lab(GETL(Lang_OptionChangesAfterRestart));
        labChangesLang->adjustSize();
        labChangesLang->setColor(0x5e5e5e);
        labChangesLang->setVisible(false);
        conWapMap->add(labChangesLang, xOffset, yOffset);

        yOffset += 25;

        labRes = new SHR::Lab(GETL(Lang_DisplayResolution));
        labRes->adjustSize();
        conWapMap->add(labRes, xOffset, yOffset);

        ddoptRes = new SHR::DropDown();
        ddoptRes->setListModel(new cListModelDisplay());
        ddoptRes->setDimension(gcn::Rectangle(0, 0, 150, 20));
        ddoptRes->addActionListener(this);
        ddoptRes->SetGfx(&GV->gcnParts);
        ddoptRes->adjustHeight();
        conWapMap->add(ddoptRes, xOffset + 130, yOffset - 1);

        {
            char actres[64];
            sprintf(actres, "%dx%d", GV->iScreenW, GV->iScreenH);
            for (int i = 0; i < ddoptRes->getListModel()->getNumberOfElements(); i++) {
                if (!strcmp(ddoptRes->getListModel()->getElementAt(i).c_str(), actres)) {
                    ddoptRes->setSelected(i);
                    break;
                }
            }
        }

        yOffset += 22;

        labChangesRes = new SHR::Lab(GETL(Lang_OptionChangesAfterRestart));
        labChangesRes->adjustSize();
        labChangesRes->setVisible(0);
        labChangesRes->setColor(0x5e5e5e);
        conWapMap->add(labChangesRes, xOffset, yOffset);

        yOffset += 30;

        cbOptionsAlfaHigherPlanes = new SHR::CBox(GV->hGfxInterface, GETL(Lang_AlphaOverlapping));
        cbOptionsAlfaHigherPlanes->adjustSize();
        cbOptionsAlfaHigherPlanes->addActionListener(this);
        cbOptionsAlfaHigherPlanes->setSelected(GV->bAlphaHigherPlanes);
        conWapMap->add(cbOptionsAlfaHigherPlanes, xOffset, yOffset);

        yOffset += 25;

        cboptSmoothZooming = new SHR::CBox(GV->hGfxInterface, GETL(Lang_UseSmoothZooming));
        cboptSmoothZooming->adjustSize();
        cboptSmoothZooming->setSelected(GV->bSmoothZoom);
        conWapMap->add(cboptSmoothZooming, xOffset, yOffset);

        yOffset += 25;

        cboptAutoUpdate = new SHR::CBox(GV->hGfxInterface, GETL(Lang_AutoUpdate));
        cboptAutoUpdate->adjustSize();
        cboptAutoUpdate->setSelected(GV->bAutoUpdate);
        conWapMap->add(cboptAutoUpdate, xOffset, yOffset);
    }
    optionsForCategory.push_back(conWapMap);
    myWin->add(conWapMap, scrollAreaCategories->getWidth(), 8);

    for (WWD::GAME i = WWD::Games_First; i <= WWD::Games_Last; ++i) {
        int xOffset = 8, yOffset = 10;
        auto* conGame = new SHR::Container();
        conGame->setOpaque(false);

        auto labDesc = new SHR::Lab(GETL2S("Options", "GameInstallDir"));
        labDesc->adjustSize();
        conGame->add(labDesc, xOffset, yOffset);
        widgetsToDelete.push_back(labDesc);

        xOffset += 5 + labDesc->getWidth();

        auto tfPath = new SHR::TextField(GV->gamePaths[i].c_str());
        tfPath->adjustSize();
        tfPath->addActionListener(this);
        tfPath->setWidth(250);
        conGame->add(tfPath, xOffset, yOffset);
        widgetsToDelete.push_back(tfPath);
        pathTextFields.push_back(tfPath);

        xOffset += 5 + tfPath->getWidth();

        widgetsToDelete.push_back(butPath[i - WWD::Games_First] = GV->editState->MakeButton(xOffset, yOffset - 6, Icon_Open, conGame, true, true, this));

        optionsForCategory.push_back(conGame);
        myWin->add(conGame, scrollAreaCategories->getWidth(), 8);
    }

    {
        auto* conClaw = optionsForCategory[WWD::Game_Claw];
        int xOffset = 8, yOffset = 15 + pathTextFields[0]->getHeight();

        labVersion = new SHR::Lab(std::string(GETL2S("Options", "DetectedVer")) + " " + GV->editState->hNativeController->GetVersionStr());
        labVersion->adjustSize();
        conClaw->add(labVersion, pathTextFields[0]->getX(), yOffset);

        yOffset += 30;

        //crazyhook options

        labCrazyHookSettings = new SHR::Lab(GETL2S("Options", (GV->editState->hNativeController->IsCrazyHookAvailable() ? "CrazyHook" : "CrazyHookNotFound")));
        labCrazyHookSettings->adjustSize();
        conClaw->add(labCrazyHookSettings, xOffset, yOffset);

        yOffset += 30;

        labGameRes = new SHR::Lab(GETL2S("Options", "GameRes"));
        labGameRes->adjustSize();
        conClaw->add(labGameRes, xOffset, yOffset);

        ddoptGameRes = new SHR::DropDown();
        ddoptGameRes->setListModel(new cListModelDisplay(true));
        ddoptGameRes->setDimension(gcn::Rectangle(0, 0, 150, 20));
        ddoptGameRes->addActionListener(this);
        ddoptGameRes->SetGfx(&GV->gcnParts);
        ddoptGameRes->adjustHeight();
        conClaw->add(ddoptGameRes, xOffset + 5 + labGameRes->getWidth(), yOffset - 1);

        yOffset += 30;

        {
            char actres[64];
            int w, h;
            GV->editState->hNativeController->GetDisplayResolution(&w, &h);
            if (w == -1 && h == -1) {
                ddoptGameRes->setSelected(0);
            } else {
                sprintf(actres, "%dx%d", w, h);
                for (int i = 0; i < ddoptGameRes->getListModel()->getNumberOfElements(); i++) {
                    if (!strcmp(ddoptGameRes->getListModel()->getElementAt(i).c_str(), actres)) {
                        ddoptGameRes->setSelected(i);
                        break;
                    }
                }
            }
        }

        cboptCrazyHookDebugInfo = new SHR::CBox(GV->hGfxInterface, GETL2S("Options", "DebugInf"));
        cboptCrazyHookDebugInfo->adjustSize();
        cboptCrazyHookDebugInfo->setSelected(GV->editState->hNativeController->IsDebugInfoOn());
        conClaw->add(cboptCrazyHookDebugInfo, xOffset, yOffset);

        yOffset += 25;

        cboptCrazyHookGodMode = new SHR::CBox(GV->hGfxInterface, GETL2S("Options", "GodMode"));
        cboptCrazyHookGodMode->adjustSize();
        cboptCrazyHookGodMode->setSelected(GV->editState->hNativeController->IsGodModeOn());
        conClaw->add(cboptCrazyHookGodMode, xOffset, yOffset);

        yOffset += 25;

        cboptCrazyHookArmor = new SHR::CBox(GV->hGfxInterface, GETL2S("Options", "ArmorMode"));
        cboptCrazyHookArmor->adjustSize();
        cboptCrazyHookArmor->setSelected(GV->editState->hNativeController->IsArmorModeOn());
        conClaw->add(cboptCrazyHookArmor, xOffset, yOffset);
    }

    butSave = new SHR::But(GV->hGfxInterface, GETL(Lang_Save));
    butSave->setDimension(gcn::Rectangle(0, 0, 100, 33));
    butSave->addActionListener(this);
    myWin->add(butSave, myWin->getWidth() - 90, myWin->getHeight() - 55);
    butSave->setX(butSave->getX() - 10);
}

winOptions::~winOptions() {
    delete labChangesRes;
    delete labChangesLang;
    delete labRes;
    delete labLang;
    delete labGameRes;
    delete labCrazyHookSettings;
    delete labVersion;
    delete butSave;
    delete ddoptLang;
    delete ddoptRes;
    delete ddoptGameRes;
    delete lmLang;
    delete cbOptionsAlfaHigherPlanes;
    delete cboptCrazyHookGodMode;
    delete cboptCrazyHookArmor;
    delete cboptCrazyHookDebugInfo;
    delete cboptAutoUpdate;
    delete cboptSmoothZooming;

    for (auto widget : widgetsToDelete) {
        delete widget;
    }

    for (auto option : optionsForCategory) {
        delete option;
    }

    delete vp;
    delete myWin;
}

void winOptions::Think() {

}

void winOptions::Draw(int piCode) {
    /*int dx, dy;
    myWin->getAbsolutePosition(dx, dy);

    unsigned char alpha = myWin->getAlpha();
    DWORD col = SETA(0xFFFFFFFF, alpha),
            colt = SETA(0xFFFFFFFF, (unsigned char) (alpha * 0.5f));

    GV->sprIcons[Icon_WapMap]->SetColor(col);
    GV->sprIcons[Icon_WapMap]->Render(dx + 5, dy + 90);

    GV->sprIcons[Icon_CrazyHook]->SetColor(GV->editState->hNativeController->IsCrazyHookAvailable() ? col : colt);
    GV->sprIcons[Icon_CrazyHook]->Render(dx + 300, dy + 90);

    //upper
    hge->Gfx_RenderLine(dx, dy + 80, dx + myWin->getWidth(), dy + 80, GV->colLineDark);
    hge->Gfx_RenderLine(dx, dy + 81, dx + myWin->getWidth(), dy + 81, GV->colLineBright);

    //bottom
    hge->Gfx_RenderLine(dx, dy + 270, dx + myWin->getWidth(), dy + 270, GV->colLineDark);
    hge->Gfx_RenderLine(dx, dy + 271, dx + myWin->getWidth(), dy + 271, GV->colLineBright);

    //middle
    hge->Gfx_RenderLine(dx + 296, dy + 81, dx + 296, dy + 270, GV->colLineDark);
    hge->Gfx_RenderLine(dx + 297, dy + 81, dx + 297, dy + 270, GV->colLineBright);*/
}

void winOptions::Open(WWD::GAME game) {
    myWin->setVisible(true);
    myWin->getParent()->moveToTop(myWin);

    settingsCategoriesList->setSelected(game);
    optionsForCategory[0]->setVisible(game == WWD::Game_Unknown);
    for (WWD::GAME i = WWD::Games_First; i <= WWD::Games_Last; ++i) {
        optionsForCategory[i]->setVisible(game == i);
    }
}

void winOptions::Close() {
    myWin->setVisible(false);
}

void winOptions::SyncWithExe() {
    action(gcn::ActionEvent(pathTextFields[WWD::Game_Claw - WWD::Games_First], ""));
}

void winOptions::PickAndSetGameLocation(WWD::GAME game) {
    OPENFILENAME ofn;
    char szFileopen[512] = "\0";
    ZeroMemory((&ofn), sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hge->System_GetState(HGE_HWND);
    const char* filter;
    switch (game) {
    case WWD::Game_Claw:
        filter = "CLAW.EXE\0claw.exe\0\0";
        break;
    case WWD::Game_GetMedieval:
        filter = "MEDIEVAL.EXE\0medieval.exe\0\0";
        break;
    case WWD::Game_Gruntz:
        filter = "GRUNTZ.EXE\0gruntz.exe\0\0";
        break;
    case WWD::Game_Claw2:
        filter = "CLAW2.EXE\0claw2.exe\0\0";
        break;
    default:
        return;
    }
    ofn.lpstrFilter = filter;

    ofn.lpstrFile = szFileopen;
    ofn.nMaxFile = sizeof(szFileopen);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = "exe";
    ofn.lpstrInitialDir = GV->szLastOpenPath;
    if (GetOpenFileName(&ofn) != 0) {
        char *path = SHR::GetDir(szFileopen);
        GV->gamePaths[game] = path;
        pathTextFields[game - WWD::Games_First]->setText(path, true);
        GV->SetLastOpenPath(path);
        delete[] path;
    }
}

void winOptions::action(const ActionEvent &actionEvent) {
    if (actionEvent.getSource() == settingsCategoriesList) {
        for (int i = 0; i < optionsForCategory.size(); ++i) {
            optionsForCategory[i]->setVisible(settingsCategoriesList->getSelected() == i);
        }
    } else if (actionEvent.getSource() == ddoptLang) {
        labChangesLang->setVisible(
                !(!strcmp(ddoptLang->getListModel()->getElementAt(ddoptLang->getSelected()).c_str(),
                          GV->Lang->GetName())));
    } else if (actionEvent.getSource() == ddoptRes) {
        int x, y;
        sscanf(ddoptRes->getListModel()->getElementAt(ddoptRes->getSelected()).c_str(), "%dx%d", &x, &y);
        labChangesRes->setVisible(
                (x != hge->System_GetState(HGE_SCREENWIDTH) || y != hge->System_GetState(HGE_SCREENHEIGHT)));
    } else if (actionEvent.getSource() == butSave) {
        for (WWD::GAME i = WWD::Games_First; i <= WWD::Games_Last; ++i) {
            GV->gamePaths[(WWD::GAME)i] = pathTextFields[i - WWD::Games_First]->getText();
            GV->ini->SetValue("Paths", WWD::GAME_NAMES[i], GV->gamePaths[i].c_str());
        }

        GV->ini->SetValue("WapMap", "Language",
                          ddoptLang->getListModel()->getElementAt(ddoptLang->getSelected()).c_str());
        char tmp[8];
        sprintf(tmp, "%d", cbOptionsAlfaHigherPlanes->isSelected());
        GV->ini->SetValue("WapMap", "AlphaOverlapping", tmp);
        GV->bAlphaHigherPlanes = cbOptionsAlfaHigherPlanes->isSelected();

        sprintf(tmp, "%d", cboptAutoUpdate->isSelected());
        GV->ini->SetValue("WapMap", "AutoUpdate", tmp);
        GV->bAutoUpdate = cboptAutoUpdate->isSelected();

        sprintf(tmp, "%d", cboptSmoothZooming->isSelected());
        GV->ini->SetValue("WapMap", "SmoothZoom", tmp);
        if (GV->bSmoothZoom != cboptSmoothZooming->isSelected())
            GV->editState->fZoom = GV->editState->fDestZoom;
        GV->bSmoothZoom = cboptSmoothZooming->isSelected();

        int w, h;
        sscanf(ddoptRes->getListModel()->getElementAt(ddoptRes->getSelected()).c_str(), "%dx%d", &w, &h);
        sprintf(tmp, "%d", w);
        GV->ini->SetValue("WapMap", "DisplayWidth", tmp);
        sprintf(tmp, "%d", h);
        GV->ini->SetValue("WapMap", "DisplayHeight", tmp);
        GV->iScreenW = w;
        GV->iScreenH = h;

        if (ddoptGameRes->getSelected() == 0) {
            w = h = -1;
            sprintf(tmp, "%d", w);
            GV->ini->SetValue("CrazyHook", "DisplayWidth", tmp);
            GV->ini->SetValue("CrazyHook", "DisplayHeight", tmp);
        } else {
            sscanf(ddoptGameRes->getListModel()->getElementAt(ddoptGameRes->getSelected()).c_str(), "%dx%d", &w, &h);
            sprintf(tmp, "%d", w);
            GV->ini->SetValue("CrazyHook", "DisplayWidth", tmp);
            sprintf(tmp, "%d", h);
            GV->ini->SetValue("CrazyHook", "DisplayHeight", tmp);
        }
        GV->editState->hNativeController->SetDisplayResolution(w, h);

        sprintf(tmp, "%d", cboptCrazyHookDebugInfo->isSelected());
        GV->ini->SetValue("CrazyHook", "DebugInfo", tmp);
        GV->editState->hNativeController->SetDebugInfo(cboptCrazyHookDebugInfo->isSelected());

        sprintf(tmp, "%d", cboptCrazyHookGodMode->isSelected());
        GV->ini->SetValue("CrazyHook", "GodMode", tmp);
        GV->editState->hNativeController->SetGodMode(cboptCrazyHookGodMode->isSelected());

        sprintf(tmp, "%d", cboptCrazyHookArmor->isSelected());
        GV->ini->SetValue("CrazyHook", "ArmorMode", tmp);
        GV->editState->hNativeController->SetArmorMode(cboptCrazyHookArmor->isSelected());

        GV->ini->SaveFile("settings.cfg");
        Close();
    } else if (actionEvent.getSource() == pathTextFields[WWD::Game_Claw - WWD::Games_First]) {
        if (!GV->editState->hNativeController->IsValid() ||
            GV->editState->hNativeController->GetPath() != pathTextFields[WWD::Game_Claw - WWD::Games_First]->getText())
            GV->editState->hNativeController->SetPath(pathTextFields[WWD::Game_Claw - WWD::Games_First]->getText());
        bool b = GV->editState->hNativeController->IsValid() && GV->editState->hNativeController->IsCrazyHookAvailable();
        cboptCrazyHookDebugInfo->setEnabled(b);
        cboptCrazyHookGodMode->setEnabled(b);
        cboptCrazyHookArmor->setEnabled(b);
        labCrazyHookSettings->setCaption(GETL2S("Options", (b ? "CrazyHook" : "CrazyHookNotFound")));
        labCrazyHookSettings->adjustSize();
        ddoptGameRes->setEnabled(b);

        labGameRes->setColor(b ? 0xa1a1a1 : 0x5e5e5e);
        labCrazyHookSettings->setColor(b ? 0xa1a1a1 : 0x5e5e5e);

        if (GV->editState->hNativeController->IsValid()) {
            labVersion->setCaption(std::string(GETL2S("Options", "DetectedVer")) + " " +
                                   GV->editState->hNativeController->GetVersionStr());
            labVersion->adjustSize();
        }
        labVersion->setVisible(GV->editState->hNativeController->IsValid());
    } else for (int i = 0; i < WWD::Games_Count; ++i) {
        if (actionEvent.getSource() == butPath[i]) {
            PickAndSetGameLocation((WWD::GAME)(i + WWD::Games_First));
            return;
        }
    }
}

std::string winOptions::getElementAt(int i) {
    if (i >= WWD::Games_First && i <= WWD::Games_Last) {
        return WWD::GAME_NAMES[(WWD::GAME)i];
    }

    return "WapMap";
}

hgeSprite *winOptions::getIcon(int i) {
    if (i == 0) {
        return GV->sprIcons[Icon_WapMap];
    } else {
        return GV->sprGamesBig[i];
    }
}
