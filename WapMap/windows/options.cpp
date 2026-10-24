#include "options.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../../shared/commonFunc.h"
#include "../cInterfaceSheet.h"
#include "../../shared/gcnWidgets/wIconListbox.h"
#include "../cNativeController.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../imguiTheme.h"
#include "../legacyText.h"
#endif
#include <cstdio>

extern HGE *hge;

cListModelDisplay::cListModelDisplay(bool bCrazyHook) {
    if (bCrazyHook) {
        const char *cptr = GETL2S("Options", "Default");
        m_vOpt.push_back(std::string(cptr));
    }

    int horizontal, vertical;
    GV->GetDesktopResolution(horizontal, vertical);
    std::vector<SHR::DisplayMode> displays = SHR::GetDisplayModes();

    for (int i = 0; i < displays.size(); i++) {
        if (displays[i].iDepth == 32 && ((bCrazyHook && displays[i].iWidth >= 640 && displays[i].iHeight >= 480) ||
                                        (!bCrazyHook && displays[i].iWidth >= MIN_SUPPORTED_SCREEN_WIDTH
                                                     && displays[i].iHeight >= MIN_SUPPORTED_SCREEN_HEIGHT))
                                        && (bCrazyHook || (displays[i].iWidth <= horizontal && displays[i].iHeight <= vertical))) {
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

    if (!bCrazyHook) {
        std::string screenRes = std::to_string(horizontal) + 'x' + std::to_string(vertical);

        bool already = 0;
        for (int x = 0; x < m_vOpt.size(); x++)
            if (screenRes == m_vOpt[x]) {
                already = 1;
                break;
            }
        if (!already) m_vOpt.push_back(screenRes);
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


winOptions::winOptions() : cWindow(GETL(Lang_Options)) {
    vp = new WIDG::Viewport(this, 0);
    myWin.add(vp, 0, 0);

    settingsCategoriesList = new SHR::IconListBox(this);
    settingsCategoriesList->adjustSize();
    settingsCategoriesList->setWidth(140);
    settingsCategoriesList->addActionListener(this);
    scrollAreaCategories = new SHR::ScrollArea(settingsCategoriesList, SHR::ScrollArea::SHOW_NEVER, SHR::ScrollArea::SHOW_NEVER);
    scrollAreaCategories->setDimension(settingsCategoriesList->getDimension());
    scrollAreaCategories->setOpaque(false);
    myWin.add(scrollAreaCategories, 0, 8);
    myWin.setHeight(26 + settingsCategoriesList->getHeight());

    auto* conWapMap = new SHR::Container();
    conWapMap->setOpaque(false);
    {
        int xOffset = 8, yOffset = 10;
        labLang = new SHR::Lab(std::string(GETL(Lang_Language)) + ':');
        labLang->adjustSize();
        conWapMap->add(labLang, xOffset, yOffset);

        lmLang = new cDynamicListModel();

        ddoptLang = new SHR::DropDown();
        ddoptLang->setListModel(lmLang);
        ddoptLang->setDimension(gcn::Rectangle(0, 0, 150, 20));
        ddoptLang->addActionListener(this);
        ddoptLang->SetGfx(&GV->gcnParts);
        ddoptLang->adjustHeight();
        conWapMap->add(ddoptLang, xOffset + labLang->getWidth() + 8, yOffset - 2);

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
            FindClose(hFind);
        }

        yOffset += 22;

        labChangesLang = new SHR::Lab(GETL(Lang_OptionChangesAfterRestart));
        labChangesLang->adjustSize();
        labChangesLang->setVisible(false);
        conWapMap->add(labChangesLang, xOffset, yOffset);

        yOffset += 25;

        cbOptionsAlfaHigherPlanes = new SHR::CBox(GV->hGfxInterface, GETL(Lang_AlphaOverlapping));
        cbOptionsAlfaHigherPlanes->adjustSize();
        cbOptionsAlfaHigherPlanes->addActionListener(this);
        conWapMap->add(cbOptionsAlfaHigherPlanes, xOffset, yOffset);

        yOffset += 25;

        cboptSmoothZooming = new SHR::CBox(GV->hGfxInterface, GETL(Lang_UseSmoothZooming));
        cboptSmoothZooming->adjustSize();
        conWapMap->add(cboptSmoothZooming, xOffset, yOffset);

        yOffset += 25;

        cboptAutoUpdate = new SHR::CBox(GV->hGfxInterface, GETL(Lang_AutoUpdate));
        cboptAutoUpdate->adjustSize();
        conWapMap->add(cboptAutoUpdate, xOffset, yOffset);

        yOffset += 25;

        cboptStartMaximized = new SHR::CBox(GV->hGfxInterface, GETL(Lang_StartMaximized));
        cboptStartMaximized->adjustSize();
        conWapMap->add(cboptStartMaximized, xOffset, yOffset);
    }
    optionsForCategory.push_back(conWapMap);
    myWin.add(conWapMap, scrollAreaCategories->getWidth(), 8);

    for (WWD::GAME i = WWD::Games_First; i <= WWD::Games_Last; ++i) {
        int xOffset = 8, yOffset = 10;
        auto* conGame = new SHR::Container();
        conGame->setOpaque(false);

        auto labDesc = new SHR::Lab(GETL2S("Options", "GameInstallDir"));
        labDesc->adjustSize();
        conGame->add(labDesc, xOffset, yOffset);
        widgetsToDelete.push_back(labDesc);

        xOffset += 8 + labDesc->getWidth();

        auto tfPath = new SHR::TextField(GV->gamePaths[i].c_str());
        tfPath->adjustSize();
        tfPath->addActionListener(this);
        tfPath->setWidth(250);
        conGame->add(tfPath, xOffset, yOffset);
        widgetsToDelete.push_back(tfPath);
        pathTextFields.push_back(tfPath);

        xOffset += 8 + tfPath->getWidth();

        widgetsToDelete.push_back(butPath[i - WWD::Games_First] = GV->editState->MakeButton(xOffset, yOffset - 6, Icon_Open, conGame, true, true, this));

        optionsForCategory.push_back(conGame);
        myWin.add(conGame, scrollAreaCategories->getWidth(), 8);
    }

    {
        auto* conClaw = optionsForCategory[WWD::Game_Claw];
        int xOffset = 8, yOffset = 15 + pathTextFields[0]->getHeight();

        labVersion = new SHR::Lab(std::string(GETL2S("Options", "DetectedVer")) + " " + GV->editState->hNativeController->GetVersionStr());
        labVersion->adjustSize();
        conClaw->add(labVersion, pathTextFields[0]->getX(), yOffset);

        yOffset += 30;

        //crazyhook options

        labCrazyHookSettings = new SHR::Lab(GETL2SV("Options", (GV->editState->hNativeController->IsCrazyHookAvailable() ? L"CrazyHook" : L"CrazyHookNotFound")));
        labCrazyHookSettings->adjustSize();
        conClaw->add(labCrazyHookSettings, xOffset, yOffset);

        yOffset += 30;

        labGameRes = new SHR::Lab(std::string(GETL2S("Options", "GameRes")) + ':');
        labGameRes->adjustSize();
        conClaw->add(labGameRes, xOffset, yOffset);

        ddoptGameRes = new SHR::DropDown();
        ddoptGameRes->setListModel(new cListModelDisplay(true));
        ddoptGameRes->setDimension(gcn::Rectangle(0, 0, 150, 20));
        ddoptGameRes->addActionListener(this);
        ddoptGameRes->SetGfx(&GV->gcnParts);
        ddoptGameRes->adjustHeight();
        conClaw->add(ddoptGameRes, xOffset + 8 + labGameRes->getWidth(), yOffset - 2);

        yOffset += 30;

        cboptCrazyHookDebugInfo = new SHR::CBox(GV->hGfxInterface, GETL2S("Options", "DebugInf"));
        cboptCrazyHookDebugInfo->adjustSize();
        conClaw->add(cboptCrazyHookDebugInfo, xOffset, yOffset);

        yOffset += 25;

        cboptCrazyHookGodMode = new SHR::CBox(GV->hGfxInterface, GETL2S("Options", "GodMode"));
        cboptCrazyHookGodMode->adjustSize();
        conClaw->add(cboptCrazyHookGodMode, xOffset, yOffset);

        yOffset += 25;

        cboptCrazyHookArmor = new SHR::CBox(GV->hGfxInterface, GETL2S("Options", "ArmorMode"));
        cboptCrazyHookArmor->adjustSize();
        conClaw->add(cboptCrazyHookArmor, xOffset, yOffset);
    }

    butSave = new SHR::But(GV->hGfxInterface, GETL(Lang_Save));
    butSave->setDimension(gcn::Rectangle(0, 0, 100, 33));
    butSave->addActionListener(this);
    myWin.add(butSave, myWin.getWidth() - 90, myWin.getHeight() - 55);
    butSave->setX(butSave->getX() - 10);
}

winOptions::~winOptions() {
    delete labChangesLang;
    delete labLang;
    delete labGameRes;
    delete labCrazyHookSettings;
    delete labVersion;
    delete butSave;
    delete ddoptLang;
    delete ddoptGameRes;
    delete lmLang;
    delete cbOptionsAlfaHigherPlanes;
    delete cboptCrazyHookGodMode;
    delete cboptCrazyHookArmor;
    delete cboptCrazyHookDebugInfo;
    delete cboptAutoUpdate;
    delete cboptSmoothZooming;
    delete cboptStartMaximized;

    for (auto widget : widgetsToDelete) {
        delete widget;
    }

    for (auto option : optionsForCategory) {
        delete option;
    }

    delete vp;
}

void winOptions::Think() {
#ifdef WAPMAP_ENABLE_IMGUI
    if (!nativeOpen || busy || !GV->editState->_isOnTop()) return;
    auto* widget = pendingAction; pendingAction = nullptr;
    if (pendingProbe || widget == butSave) {
        pendingProbe = false;
        UpdatedClawExePath();
    }
    if (widget) {
        busy = true;
        action(gcn::ActionEvent(widget, ""));
        busy = false;
    }
#endif
}

void winOptions::Draw(int piCode) {
    /*int dx, dy;
    myWin.getAbsolutePosition(dx, dy);

    unsigned char alpha = myWin.getAlpha();
    DWORD col = SETA(0xFFFFFFFF, alpha),
            colt = SETA(0xFFFFFFFF, (unsigned char) (alpha * 0.5f));

    GV->sprIcons[Icon_WapMap]->SetColor(col);
    GV->sprIcons[Icon_WapMap]->Render(dx + 5, dy + 90);

    GV->sprIcons[Icon_CrazyHook]->SetColor(GV->editState->hNativeController->IsCrazyHookAvailable() ? col : colt);
    GV->sprIcons[Icon_CrazyHook]->Render(dx + 300, dy + 90);

    //upper
    hge->Gfx_RenderLine(dx, dy + 80, dx + myWin.getWidth(), dy + 80, GV->colLineDark);
    hge->Gfx_RenderLine(dx, dy + 81, dx + myWin.getWidth(), dy + 81, GV->colLineBright);

    //bottom
    hge->Gfx_RenderLine(dx, dy + 270, dx + myWin.getWidth(), dy + 270, GV->colLineDark);
    hge->Gfx_RenderLine(dx, dy + 271, dx + myWin.getWidth(), dy + 271, GV->colLineBright);

    //middle
    hge->Gfx_RenderLine(dx + 296, dy + 81, dx + 296, dy + 270, GV->colLineDark);
    hge->Gfx_RenderLine(dx + 297, dy + 81, dx + 297, dy + 270, GV->colLineBright);*/
}

void winOptions::Open(WWD::GAME game) {
    if (game < WWD::Game_Unknown || game > WWD::Games_Last) game = WWD::Game_Unknown;
#ifdef WAPMAP_ENABLE_IMGUI
    myWin.setVisible(false);
    nativeOpen = positionOnOpen = true;
    pendingProbe = false; pendingAction = nullptr;
#else
    cWindow::Open();
#endif

    settingsCategoriesList->setSelected(game);
    optionsForCategory[0]->setVisible(game == WWD::Game_Unknown);
    for (WWD::GAME i = WWD::Games_First; i <= WWD::Games_Last; ++i) {
        optionsForCategory[i]->setVisible(game == i);
    }

    std::string lang(GV->Lang->GetName()), res = std::to_string(GV->iScreenW) + 'x' + std::to_string(GV->iScreenH);
    for (int i = 0; i < lmLang->getNumberOfElements(); ++i) {
        if (lmLang->getElementAt(i) == lang) {
            ddoptLang->setSelected(i);
            break;
        }
    }
    labChangesLang->setVisible(false);
    cbOptionsAlfaHigherPlanes->setSelected(GV->bAlphaHigherPlanes);
    cboptSmoothZooming->setSelected(GV->bSmoothZoom);
    cboptAutoUpdate->setSelected(GV->bAutoUpdate);
    cboptStartMaximized->setSelected(GV->bStartMaximized);

    for (WWD::GAME i = WWD::Games_First; i <= WWD::Games_Last; ++i) {
        pathTextFields[i - WWD::Games_First]->setText(GV->gamePaths[i]);
    }
    UpdatedClawExePath(true);

    int w, h;
    GV->editState->hNativeController->GetDisplayResolution(&w, &h);
    if (w == -1 && h == -1) {
        ddoptGameRes->setSelected(0);
    } else {
        res = std::to_string(w) + 'x' + std::to_string(h);
        for (int i = 0; i < ddoptGameRes->getListModel()->getNumberOfElements(); i++) {
            if (ddoptGameRes->getListModel()->getElementAt(i) == res) {
                ddoptGameRes->setSelected(i);
                break;
            }
        }
    }

    cboptCrazyHookDebugInfo->setSelected(GV->editState->hNativeController->IsDebugInfoOn());
    cboptCrazyHookGodMode->setSelected(GV->editState->hNativeController->IsGodModeOn());
    cboptCrazyHookArmor->setSelected(GV->editState->hNativeController->IsArmorModeOn());
#ifdef WAPMAP_ENABLE_IMGUI
    paths.resize(pathTextFields.size());
    for (int i = 0; i < static_cast<int>(paths.size()); ++i) UpdateNativePath(i);
#endif
}

void winOptions::Close() {
#ifdef WAPMAP_ENABLE_IMGUI
    nativeOpen = pendingProbe = false; pendingAction = nullptr;
#endif
    cWindow::Close();
    // Executable-change notifications also read these retained fields. Restore
    // saved paths so a later notification cannot revive a cancelled draft.
    for (int i = 0; i < static_cast<int>(pathTextFields.size()); ++i)
        pathTextFields[i]->setText(GV->gamePaths[static_cast<WWD::GAME>(i + WWD::Games_First)]);
    GV->editState->hNativeController->SetPath(GV->gamePaths[WWD::Game_Claw]);
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
        pathTextFields[game - WWD::Games_First]->setText(path, true);
#ifdef WAPMAP_ENABLE_IMGUI
        if (nativeOpen) UpdateNativePath(game - WWD::Games_First);
#endif
        GV->SetLastOpenPath(path);
        delete[] path;
    }
}

void winOptions::action(const ActionEvent &actionEvent) {
    if (actionEvent.getSource() == &myWin) {
        Close();
    } else if (actionEvent.getSource() == settingsCategoriesList) {
        for (int i = 0; i < optionsForCategory.size(); ++i) {
            optionsForCategory[i]->setVisible(settingsCategoriesList->getSelected() == i);
        }
    } else if (actionEvent.getSource() == ddoptLang) {
        labChangesLang->setVisible(
                !(!strcmp(ddoptLang->getListModel()->getElementAt(ddoptLang->getSelected()).c_str(),
                          GV->Lang->GetName())));
    } else if (actionEvent.getSource() == butSave) {
        if (ddoptLang->getSelected() < 0 || ddoptLang->getSelected() >= lmLang->getNumberOfElements() ||
            ddoptGameRes->getSelected() < 0 || ddoptGameRes->getSelected() >= ddoptGameRes->getListModel()->getNumberOfElements()) return;
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

        sprintf(tmp, "%d", cboptStartMaximized->isSelected());
        GV->ini->SetValue("WapMap", "StartMaximized", tmp);
        GV->bStartMaximized = cboptStartMaximized->isSelected();

        int w, h;
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
        UpdatedClawExePath();
    } else for (int i = 0; i < static_cast<int>(pathTextFields.size()); ++i) {
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

void winOptions::UpdatedClawExePath(bool forceDetect) {
    if (forceDetect || !GV->editState->hNativeController->IsValid() ||
        GV->editState->hNativeController->GetPath() != pathTextFields[WWD::Game_Claw - WWD::Games_First]->getText())
        GV->editState->hNativeController->SetPath(pathTextFields[WWD::Game_Claw - WWD::Games_First]->getText());
    bool b = GV->editState->hNativeController->IsValid() && GV->editState->hNativeController->IsCrazyHookAvailable();
    cboptCrazyHookDebugInfo->setEnabled(b);
    cboptCrazyHookGodMode->setEnabled(b);
    cboptCrazyHookArmor->setEnabled(b);
    labCrazyHookSettings->setCaption(GETL2SV("Options", (b ? L"CrazyHook" : L"CrazyHookNotFound")));
    labCrazyHookSettings->adjustSize();
    ddoptGameRes->setEnabled(b);

    labGameRes->setColor(SETA(0xe1e1e1, b ? 0xFF : 0x77));

    if (GV->editState->hNativeController->IsValid()) {
        labVersion->setCaption(std::string(GETL2S("Options", "DetectedVer")) + " " +
                               GV->editState->hNativeController->GetVersionStr());
        labVersion->adjustSize();
        labVersion->setVisible(true);
    } else {
        labVersion->setVisible(false);
    }
}

#ifdef WAPMAP_ENABLE_IMGUI
void winOptions::UpdateNativePath(int index) {
    if (index < 0 || index >= static_cast<int>(paths.size())) return;
    const auto text = LegacyText::ToUtf8(pathTextFields[index]->getText(), CP_ACP);
    std::snprintf(paths[index].data(), paths[index].size(), "%s", text.c_str());
}

void winOptions::DrawImGui() {
    if (!nativeOpen || busy || !GV->editState->_isOnTop()) return;
    const auto area = myWin.getChildrenArea();
    int width = myWin.getWidth() + 8, height = myWin.getHeight();
    for (auto* category : optionsForCategory)
        height = std::max(height, area.y + category->getY() + category->getHeight() + 8);
    if (positionOnOpen) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, {0.5f, 0.5f});
        ImGui::SetNextWindowFocus(); positionOnOpen = false;
    }
    ImGui::SetNextWindowSize({static_cast<float>(width), static_cast<float>(height)}, ImGuiCond_Always);
    const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
    const auto title = LegacyText::ToUtf8(GETL(Lang_Options), cp) + "###WapMapSettings";
    bool visible = true;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, GETL(Lang_Options));
        const auto origin = ImGui::GetWindowPos(); auto* draw = ImGui::GetWindowDrawList();
        const auto position = [&](gcn::Widget* widget) {
            auto* parent = widget->getParent();
            const bool inCategory = parent != &myWin;
            ImGui::SetCursorPos({static_cast<float>(area.x + widget->getX() + (inCategory ? parent->getX() : 0)),
                static_cast<float>(area.y + widget->getY() + (inCategory ? parent->getY() : 0))});
        };
        int selected = std::clamp(settingsCategoriesList->getSelected(), 0, static_cast<int>(optionsForCategory.size()) - 1);
        ImGui::SetCursorPos({static_cast<float>(area.x), area.y + 8.f});
        ImGui::PushStyleColor(ImGuiCol_ChildBg, WapMapImGui::Grey(0x13));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});
        if (ImGui::BeginChild("categories", {140.f, static_cast<float>(settingsCategoriesList->getHeight())}, ImGuiChildFlags_NavFlattened)) {
            for (int i = 0; i < getNumberOfElements(); ++i) {
                ImGui::PushID(i); const auto p = ImGui::GetCursorScreenPos();
                const float row = static_cast<float>(settingsCategoriesList->getRowHeight());
                // The category list is one tab stop; arrows may still visit
                // every row. Share navigation with the form outside the child.
                ImGui::PushItemFlag(ImGuiItemFlags_NoTabStop, i != selected);
                const bool activated = ImGui::InvisibleButton("##category", {140.f, row}, ImGuiButtonFlags_EnableNav);
                ImGui::PopItemFlag();
                if (activated || (WapMapImGui::IsKeyboardFocused() && selected != i)) {
                    selected = i; settingsCategoriesList->setSelected(i);
                    action(gcn::ActionEvent(settingsCategoriesList, ""));
                }
                auto* listDraw = ImGui::GetWindowDrawList();
                if (selected == i || ImGui::IsItemHovered() || ImGui::IsItemFocused())
                    listDraw->AddRectFilled(p, {p.x + 140, p.y + row}, selected == i ? IM_COL32(23, 150, 255, 255) : IM_COL32(255, 255, 255, 34));
                WapMapImGui::Sprite(listDraw, *hge, getIcon(i), {p.x + 4, p.y + 8});
                const auto caption = getElementAt(i);
                WapMapImGui::BitmapText(listDraw, *hge, *GV->fntMyriad16, {p.x + 40, p.y + (row - GV->fntMyriad16->GetHeight()) / 2}, caption.c_str());
                ImGui::PopID();
            }
        }
        ImGui::EndChild(); ImGui::PopStyleVar(2); ImGui::PopStyleColor();
        auto* category = optionsForCategory[selected];
        const auto label = [&](SHR::Lab* lab) {
            if (lab->getParent() != category || !lab->isLocallyVisible()) return;
            const ImVec2 p{origin.x + area.x + category->getX() + lab->getX(), origin.y + area.y + category->getY() + lab->getY()};
            const bool disabled = lab == labGameRes && !ddoptGameRes->isLocallyEnabled();
            WapMapImGui::BitmapText(draw, *hge, *GV->fntMyriad16, p, lab->getCaption().c_str(), IM_COL32(225, 225, 225, disabled ? 119 : 255));
        };
        for (auto* lab : {labLang, labChangesLang, labVersion, labCrazyHookSettings, labGameRes}) label(lab);
        for (auto* widget : widgetsToDelete) if (auto* lab = dynamic_cast<SHR::Lab*>(widget)) label(lab);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {5.f, std::max(0.f, (20.f - GV->fntMyriad16->GetHeight()) / 2)});
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, WapMapImGui::Grey(0x23));
        for (auto* dropdown : {ddoptLang, ddoptGameRes}) if (dropdown->getParent() == category) {
            ImGui::PushID(dropdown); position(dropdown); ImGui::SetNextItemWidth(static_cast<float>(dropdown->getWidth()));
            auto* model = dropdown->getListModel(); const int index = dropdown->getSelected();
            const auto preview = index >= 0 && index < model->getNumberOfElements() ? LegacyText::ToUtf8(model->getElementAt(index), cp) : std::string{};
            ImGui::BeginDisabled(!dropdown->isLocallyEnabled());
            if (ImGui::BeginCombo("##value", preview.c_str())) {
                for (int i = 0; i < model->getNumberOfElements(); ++i) {
                    ImGui::PushID(i); const auto text = LegacyText::ToUtf8(model->getElementAt(i), cp);
                    if (ImGui::Selectable(text.c_str(), index == i)) {
                        dropdown->setSelected(i);
                        if (dropdown == ddoptLang) action(gcn::ActionEvent(ddoptLang, ""));
                    }
                    if (index == i) ImGui::SetItemDefaultFocus(); ImGui::PopID();
                }
                ImGui::EndCombo();
            }
            ImGui::EndDisabled(); ImGui::PopID();
        }
        if (selected >= WWD::Games_First && selected <= WWD::Games_Last) {
            const int index = selected - WWD::Games_First; auto* field = pathTextFields[index];
            position(field); ImGui::SetNextItemWidth(static_cast<float>(field->getWidth()));
            const bool entered = ImGui::InputText("##path", paths[index].data(), paths[index].size(), ImGuiInputTextFlags_EnterReturnsTrue);
            const auto encoded = LegacyText::FromUtf8(paths[index].data(), CP_ACP, 4096);
            if (!encoded) {
                draw->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(220, 75, 75, 255));
                if (ImGui::IsItemHovered()) {
                    const auto message = LegacyText::ToUtf8(GETL2S("MapProperties", "InvalidText"), cp);
                    ImGui::SetTooltip("%s", message.c_str());
                }
            }
            if (encoded && (entered || ImGui::IsItemDeactivatedAfterEdit())) {
                field->setText(*encoded);
                if (selected == WWD::Game_Claw) pendingProbe = true;
            }
        }
        ImGui::PopStyleColor(); ImGui::PopStyleVar(3);
        for (auto* checkbox : {cbOptionsAlfaHigherPlanes, cboptSmoothZooming, cboptAutoUpdate, cboptStartMaximized,
            cboptCrazyHookDebugInfo, cboptCrazyHookGodMode, cboptCrazyHookArmor}) if (checkbox->getParent() == category) {
            ImGui::PushID(checkbox); position(checkbox); bool value = checkbox->isSelected();
            ImGui::BeginDisabled(!checkbox->isLocallyEnabled());
            if (WapMapImGui::BitmapCheckbox(*hge, *GV->fntMyriad16, *GV->hGfxInterface, checkbox->getCaption().c_str(), value)) checkbox->setSelected(value);
            ImGui::EndDisabled(); ImGui::PopID();
        }
        if (selected >= WWD::Games_First && selected <= WWD::Games_Last) {
            auto* button = butPath[selected - WWD::Games_First]; position(button);
            ImGui::PushID(button);
            ImGui::BeginDisabled(pendingAction != nullptr);
            if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, "", {static_cast<float>(button->getWidth()), static_cast<float>(button->getHeight())})) pendingAction = button;
            const auto p = ImGui::GetItemRectMin(); const auto* icon = button->getIcon();
            if (icon) WapMapImGui::Sprite(draw, *hge, icon, {p.x + (button->getWidth() - icon->GetWidth()) / 2, p.y + (button->getHeight() - icon->GetHeight()) / 2});
            ImGui::EndDisabled();
            ImGui::PopID();
        }
        std::vector<std::optional<std::string>> encodedPaths;
        bool valid = ddoptLang->getSelected() >= 0 && ddoptLang->getSelected() < lmLang->getNumberOfElements() &&
            ddoptGameRes->getSelected() >= 0 && ddoptGameRes->getSelected() < ddoptGameRes->getListModel()->getNumberOfElements();
        for (const auto& path : paths) {
            encodedPaths.push_back(LegacyText::FromUtf8(path.data(), CP_ACP, 4096));
            valid = valid && encodedPaths.back().has_value();
        }
        ImGui::SetCursorPos({static_cast<float>(width - 108), static_cast<float>(height - 41)});
        ImGui::BeginDisabled(!valid || pendingAction != nullptr);
        if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, butSave->getCaption().c_str(), {100, 33})) {
            for (int i = 0; i < static_cast<int>(pathTextFields.size()); ++i) pathTextFields[i]->setText(*encodedPaths[i]);
            pendingAction = butSave;
        }
        ImGui::EndDisabled();
        draw->AddLine({origin.x + area.x + 140, origin.y + 25}, {origin.x + area.x + 140, origin.y + height}, IM_COL32(86, 86, 86, 255));
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) visible = false;
    }
    ImGui::End();
    if (!visible) {
        pendingProbe = false;
        pendingAction = &myWin;
    }
}
#endif
