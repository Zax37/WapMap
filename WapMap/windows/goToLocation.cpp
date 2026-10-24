#include "goToLocation.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../states/dialog.h"
#include <unordered_set>
#ifdef WAPMAP_ENABLE_IMGUI
#include "../imguiTheme.h"
#include "../legacyText.h"
#endif

#define LOCATIONS_WIN_WIDTH 250
#define LOCATIONS_WIN_HEIGHT 401
#define LOCATIONS_BUTTON_OFFSETX 40

extern HGE *hge;

enum LocNameRet {
    LocName_Success = 0,
    LocName_ErrorEmpty,
    LocName_ErrorDuplicate,
    LocName_ErrorTooLong,
    LocName_ErrorGeneric
};

bool validateObjectPtr(WWD::Object* object) {
    if (!object || !GV->editState->hParser) return false;
    auto* mainPlane = GV->editState->hParser->GetMainPlane();
    if (!mainPlane) return false;
    for (int i = 0; i < mainPlane->GetObjectsCount(); i++) {
        if (mainPlane->GetObjectByIterator(i) == object)
            return 1;
    }
    return 0;
}

int LocationsList::addLocation(std::string& name, int x, int y) {
    if (!v)
        return LocName_ErrorGeneric;
    if (name.empty())
        return LocName_ErrorEmpty;
    for (int i = 0; i < v->size(); ++i) {
        if (name == getLocation(i)->Name)
            return LocName_ErrorDuplicate;
    }
    int len = name.length();
    if (len > 63)
        return LocName_ErrorTooLong;
    stLocation loc;
    strcpy(loc.Name, name.c_str());
    loc.X = x;
    loc.Y = y;
    v->push_back(loc);
    return LocName_Success;
}

bool LocationsList::addLocationLinkedToObject(WWD::Object *object) {
    if (!v || !object)
        return 0;

    stLocation loc;
    loc.X = object->GetX();
    loc.Y = object->GetY();
    loc.Object = object;
    v->push_back(loc);
    return 1;
}

void LocationsList::deleteLocation(int i) {
    if (!v || i < 0 || i >= getNumberOfElements()) return;
    v->erase(v->begin() + i);
}

stLocation* LocationsList::getLocation(int i) {
    if (!v || i < 0 || i >= (int)(v->size())) return NULL;
    return &(v->at(i));
}

int LocationsList::renameLocation(int index, std::string& newName) {
    if (!v)
        return LocName_ErrorGeneric;
    auto* location = getLocation(index);
    if (!location) return LocName_ErrorGeneric;

    if (location->Object) {
        if (!validateObjectPtr(location->Object))
            return LocName_ErrorGeneric;
        location->Object->SetName(newName.c_str());
        return LocName_Success;
    }

    if (newName.empty())
        return LocName_ErrorEmpty;

    for (int i = 0; i < v->size(); ++i) {
        if (i != index && getElementAt(i) == newName)
            return LocName_ErrorDuplicate;
    }

    if (newName.length() > 63)
        return LocName_ErrorTooLong;

    sprintf(location->Name, "%s", newName.c_str());
    return LocName_Success;
}

void LocationsList::sort() {
    if (!isObjectList)
        return;

    std::sort(v->begin(), v->end(), [](stLocation& a, stLocation& b) {
        auto *meta = a.Object->GetMeta();
        if (!meta)
            return (bool)0;
        int first = meta->locationListIndex;

        meta = b.Object->GetMeta();
        if (!meta)
            return (bool)1;
        int second = meta->locationListIndex;

        return first < second;
    });

    for (int i = 0; i < v->size(); i++)
        v->at(i).Object->SetMeta(i + 1);
}

bool LocationsList::moveElement(int index, int move) {
    if (index < 0 || index >= getNumberOfElements())
        return 0;
    if (move == 0)
        return 0;
    int dest = move < 0 ? index - 1 : index + 1;
    if (dest < 0 || dest >= getNumberOfElements())
        return 0;

    if (this->isObjectList) {
        getLocation(index)->Object->SetMeta(dest + 1);
        getLocation(dest)->Object->SetMeta(index + 1);
    }

    auto copy = v->at(dest);
    v->at(dest) = v->at(index);
    v->at(index) = copy;

    return 1;
}

std::string LocationsList::getElementAt(int i) {
    if (!v || i < 0 || i >= (int)(v->size()))
        return "";
    if (!isObjectList)
        return v->at(i).Name;

    auto *object = v->at(i).Object;
    if (!validateObjectPtr(object)) {
        // List rendering must not invalidate indices while it iterates rows.
        return "";
    }
    std::string name = object->GetName();
    if (name.empty()) {
        name.reserve(64);
        name += object->GetLogic();
        name += " #";
        name += std::to_string(object->GetParam(WWD::Param_ID));
    }
    return name;
}

inline const char* getStr(const wchar_t* wstr) {
    return GETL2SV("Win_GoToLocations", wstr);
}

winLocationsBrowser::winLocationsBrowser() : cWindow(getStr(L"WinCaption"), LOCATIONS_WIN_WIDTH, LOCATIONS_WIN_HEIGHT) {
    int x = 8, y = 8;

    tabarLocs = new SHR::TabbedArea();
    tabarLocs->addActionListener(this);
    tabarLocs->setDimension(gcn::Rectangle(0, 0, LOCATIONS_WIN_WIDTH, 25));
    myWin.add(tabarLocs, 0, y);

    for (int i = 0; i < Locs::COUNT; i++) {
        lmLocs[i] = new LocationsList();
        wchar_t temp[32];
        wsprintfW(temp, L"Tab_%d", i);
        tabarLocs->addTab(getStr(temp));
    }
    
    lmLocs[Locs::Checkpoints]->v = &vCheckpoints;
    vCheckpoints.reserve(100);
    lmLocs[Locs::Checkpoints]->isObjectList = 1;

    lmLocs[Locs::Warps]->v = &vWarps;
    vWarps.reserve(100);
    lmLocs[Locs::Warps]->isObjectList = 1;

    y += 25;

    lbLocs = new SHR::ListBox(lmLocs[Locs::Favourites]);
    lbLocs->setDimension(gcn::Rectangle(0, 0, LOCATIONS_WIN_WIDTH - 4, 310));
    lbLocs->addActionListener(this);
    saLocs = new SHR::ScrollArea(lbLocs, SHR::ScrollArea::SHOW_NEVER, SHR::ScrollArea::SHOW_AUTO);
    saLocs->setDimension(gcn::Rectangle(0, 0, LOCATIONS_WIN_WIDTH - 4, 310));
    myWin.add(saLocs, 1, y);

    y += 316;

    butAddFav = new SHR::But(GV->hGfxInterface, GV->sprIcons16[Icon16_Add]);
    butAddFav->setDimension(gcn::Rectangle(0, 0, 32, 30));
    butAddFav->addActionListener(this);
    butAddFav->SetTooltip(GETL(Lang_AddFavLocation));
    myWin.add(butAddFav, x, y);
    x += LOCATIONS_BUTTON_OFFSETX;

    butDelete = new SHR::But(GV->hGfxInterface, GV->sprIcons16[Icon16_Trash]);
    butDelete->setDimension(gcn::Rectangle(0, 0, 32, 30));
    butDelete->addActionListener(this);
    butDelete->SetTooltip(GETL(Lang_Delete));
    butDelete->setEnabled(0);
    myWin.add(butDelete, x, y);
    x += LOCATIONS_BUTTON_OFFSETX;

    butEdit = new SHR::But(GV->hGfxInterface, GV->sprIcons16[Icon16_Pencil]);
    butEdit->setDimension(gcn::Rectangle(0, 0, 32, 30));
    butEdit->addActionListener(this);
    butEdit->SetTooltip(getStr(L"Edit"));
    butEdit->setEnabled(0);
    myWin.add(butEdit, x, y);
    x += LOCATIONS_BUTTON_OFFSETX;

    butMoveDown = new SHR::But(GV->hGfxInterface, GV->sprIcons16[Icon16_Down]);
    butMoveDown->setDimension(gcn::Rectangle(0, 0, 32, 30));
    butMoveDown->addActionListener(this);
    butMoveDown->SetTooltip(GETL(Lang_MoveDown));
    butMoveDown->setEnabled(0);
    myWin.add(butMoveDown, x, y);
    x += LOCATIONS_BUTTON_OFFSETX;

    butMoveUp = new SHR::But(GV->hGfxInterface, GV->sprIcons16[Icon16_Up]);
    butMoveUp->setDimension(gcn::Rectangle(0, 0, 32, 30));
    butMoveUp->addActionListener(this);
    butMoveUp->SetTooltip(GETL(Lang_MoveUp));
    butMoveUp->setEnabled(0);
    myWin.add(butMoveUp, x, y);
    x += LOCATIONS_BUTTON_OFFSETX;

    butFollowWarp = new SHR::But(GV->hGfxInterface, GV->sprIcons16[Icon16_Warp]);
    butFollowWarp->setDimension(gcn::Rectangle(0, 0, 32, 30));
    butFollowWarp->addActionListener(this);
    butFollowWarp->SetTooltip(getStr(L"FollowWarp"));
    butFollowWarp->setEnabled(0);
    myWin.add(butFollowWarp, x, y);
}

void winLocationsBrowser::AddFavLocation(int x, int y) {
    int ret = State::InputDialog(GETL(Lang_WM), getStr(L"DialogInputName"), ST_DIALOG_BUT_OKCANCEL, NULL);
    if (ret != RETURN_OK)
        return;
    int rr = lmLocs[Locs::Favourites]->addLocation(GV->szLastInputDialogText, x, y);
    if (rr == LocName_Success) {
        GV->editState->MarkUnsaved();
    } else {
        wchar_t temp[32];
        wsprintfW(temp, L"DialogInputNameError_%d", rr);
        State::MessageBox(GETL(Lang_Error), getStr(temp), ST_DIALOG_ICON_ERROR);
    }
}

void winLocationsBrowser::RefreshLists() {
    auto* editor = GV->editState;
    if (!editor->hParser || !editor->MDI->GetActiveDoc() || !editor->hParser->GetMainPlane()) {
        lmLocs[Locs::Favourites]->v = nullptr;
        vCheckpoints.clear(); vWarps.clear();
        return;
    }
    auto* mainPlane = GV->editState->hParser->GetMainPlane();

    lmLocs[Locs::Favourites]->v = &(GV->editState->MDI->GetActiveDoc()->vFavLocations);
    lmLocs[Locs::Checkpoints]->v->clear();
    lmLocs[Locs::Warps]->v->clear();

    for (int i = 0; i < mainPlane->GetObjectsCount(); i++) {
        auto *obj =  mainPlane->GetObjectByIterator(i);
        const char* logic = obj->GetLogic();

        if (strstr(logic, "Checkpoint")) {
            lmLocs[Locs::Checkpoints]->addLocationLinkedToObject(obj);
            continue;
        }
        // mapping warps
        if (obj->GetParam(WWD::Param_SpeedX) <= 0 && obj->GetParam(WWD::Param_SpeedY) <= 0)
            continue;

        if (!strcmp(logic, "BossWarp") || (!strcmp(logic, "SpecialPowerup") &&
                (!strcmp(obj->GetImageSet(), "GAME_WARP") || !strcmp(obj->GetImageSet(), "GAME_VERTWARP")))) {
            lmLocs[Locs::Warps]->addLocationLinkedToObject(obj);
        }
    }
    lmLocs[Locs::Checkpoints]->sort();
    lmLocs[Locs::Warps]->sort();
}

void winLocationsBrowser::Open() {
    if (!GV->editState->hParser) return;
    RefreshLists();
#ifdef WAPMAP_ENABLE_IMGUI
    document = GV->editState->hParser;
    nativeOpen = positionOnOpen = resetScroll = true;
    pendingAction = nullptr;
    myWin.setVisible(false);
#else
    myWin.setPosition(160, LAY_VIEWPORT_Y + 40);
    myWin.setVisible(true);
    myWin.getParent()->moveToTop(&myWin);
#endif
}

void winLocationsBrowser::Close() {
#ifdef WAPMAP_ENABLE_IMGUI
    nativeOpen = false; document = nullptr; pendingAction = nullptr;
#endif
    cWindow::Close();
}

void winLocationsBrowser::OnDocumentChange() {
#ifdef WAPMAP_ENABLE_IMGUI
    Close();
#endif
    RefreshLists();
    lbLocs->setSelected(-1);
}

void winLocationsBrowser::action(const ActionEvent &actionEvent) {
    if (actionEvent.getSource() == tabarLocs) {
        RefreshLists();
        return; // changing tabs in Think()
    }

    int i = lbLocs->getSelected();
    if (i < 0 || i >= lmLocs[m_selectedTab]->getNumberOfElements())
        return;
    auto* location = lmLocs[m_selectedTab]->getLocation(i);
    if (!location || (location->Object && !validateObjectPtr(location->Object))) return;
    if (location->Object) { location->X = location->Object->GetX(); location->Y = location->Object->GetY(); }

    if (actionEvent.getSource() == butAddFav) {
        std::string init = lmLocs[m_selectedTab]->getElementAt(i);
        int ret = State::InputDialog(GETL(Lang_WM), getStr(L"DialogInputName"), ST_DIALOG_BUT_OKCANCEL, init.c_str());
        if (ret == RETURN_OK) {
            const int result = lmLocs[Locs::Favourites]->addLocation(GV->szLastInputDialogText, location->X, location->Y);
            if (result == LocName_Success) GV->editState->MarkUnsaved();
            else {
                wchar_t key[32]; wsprintfW(key, L"DialogInputNameError_%d", result);
                State::MessageBox(GETL(Lang_Error), getStr(key), ST_DIALOG_ICON_ERROR);
            }
        }
        return;
    }

    if (actionEvent.getSource() == lbLocs) {
        GV->editState->NavigateToPoint(location->X, location->Y);
    }
    else if (actionEvent.getSource() == butDelete) {
        if (m_selectedTab == Locs::Checkpoints || m_selectedTab == Locs::Warps) {
            if (validateObjectPtr(location->Object)) {
                GV->editState->hParser->GetMainPlane()->DeleteObject(location->Object);
                GV->editState->vPort->MarkToRedraw();
            }
        }
        lmLocs[m_selectedTab]->deleteLocation(i);
        GV->editState->MarkUnsaved();
    }
    else if (actionEvent.getSource() == butEdit) {
        std::string initStr;
        if (lmLocs[m_selectedTab]->isObjectList)
            initStr = lmLocs[m_selectedTab]->getLocation(i)->Object->GetName();
        else
            initStr = lmLocs[m_selectedTab]->getLocation(i)->Name;
         
        int ret = State::InputDialog(GETL(Lang_WM), getStr(L"DialogInputName"), ST_DIALOG_BUT_OKCANCEL, initStr.c_str());
        if (ret == RETURN_OK && initStr != GV->szLastInputDialogText) {
            int rr = lmLocs[m_selectedTab]->renameLocation(i, GV->szLastInputDialogText);
            if (rr == LocName_Success) {
                GV->editState->MarkUnsaved();
            } else {
                wchar_t temp[32];
                wsprintfW(temp, L"DialogInputNameError_%d", rr);
                State::MessageBox(GETL(Lang_Error), getStr(temp), ST_DIALOG_ICON_ERROR);
            }
            RefreshLists();
        }
    }
    else if (actionEvent.getSource() == butMoveDown) {
        int selected = lbLocs->getSelected();
        if (lmLocs[m_selectedTab]->moveElement(selected, 1)) {
            lbLocs->setSelected(selected + 1);
            GV->editState->MarkUnsaved();
        }
    }
    else if (actionEvent.getSource() == butMoveUp) {
        int selected = lbLocs->getSelected();
        if (lmLocs[m_selectedTab]->moveElement(selected, -1)) {
            lbLocs->setSelected(selected - 1);
            GV->editState->MarkUnsaved();
        }
    }
    else if (actionEvent.getSource() == butFollowWarp) {
        GV->editState->NavigateToWarpDestination(location->Object);
    }
};

void winLocationsBrowser::Think() {
#ifdef WAPMAP_ENABLE_IMGUI
    if (busy) return;
    if (nativeOpen && document != GV->editState->hParser) { Close(); return; }
    if (pendingAction && nativeOpen && GV->editState->_isOnTop()) {
        auto* widget = pendingAction; pendingAction = nullptr;
        auto* model = lmLocs[pendingTab];
        int index = -1;
        for (int i = 0; i < model->getNumberOfElements(); ++i) {
            auto* loc = model->getLocation(i);
            if (pendingLocation.Object ? loc->Object == pendingLocation.Object :
                (!loc->Object && !strcmp(loc->Name, pendingLocation.Name) && loc->X == pendingLocation.X && loc->Y == pendingLocation.Y)) {
                index = i; break;
            }
        }
        if (index >= 0 && m_selectedTab == pendingTab) {
            lbLocs->setSelected(index);
            busy = true;
            action(gcn::ActionEvent(widget, ""));
            busy = false;
        }
    }
#endif
    // Remove deleted objects before drawing, and keep locations in step with
    // object movement without rebuilding or reordering the user's selection.
    if (isOpened()) {
        auto* selectedLocation = lmLocs[m_selectedTab]->getLocation(lbLocs->getSelected());
        auto* selectedObject = selectedLocation ? selectedLocation->Object : nullptr;
        std::unordered_set<WWD::Object*> objects;
        auto* main = GV->editState->hParser ? GV->editState->hParser->GetMainPlane() : nullptr;
        if (main)
            for (int i = 0; i < main->GetObjectsCount(); ++i) objects.insert(main->GetObjectByIterator(i));
        for (auto* list : {&vCheckpoints, &vWarps}) {
            list->erase(std::remove_if(list->begin(), list->end(), [&](const stLocation& loc) { return !objects.count(loc.Object); }), list->end());
            for (auto& loc : *list) { loc.X = loc.Object->GetX(); loc.Y = loc.Object->GetY(); }
        }
        if (selectedObject) {
            int index = -1;
            for (int i = 0; i < lmLocs[m_selectedTab]->getNumberOfElements(); ++i)
                if (lmLocs[m_selectedTab]->getLocation(i)->Object == selectedObject) { index = i; break; }
            lbLocs->setSelected(index);
        }
    }
    int tab = tabarLocs->getSelectedTabIndex();
    if (m_selectedTab != tab) {
        lbLocs->setListModel(lmLocs[tab]);
        m_selectedTab = tab;
        saLocs->setVerticalScrollAmount(0);
    }
    int selected = lbLocs->getSelected();
    butAddFav->setEnabled(selected >= 0 && m_selectedTab != Locs::Favourites);
    const bool valid = selected >= 0 && selected < lmLocs[m_selectedTab]->getNumberOfElements();
    butDelete->setEnabled(valid);
    butEdit->setEnabled(valid);
    butMoveDown->setEnabled(selected >= 0 && selected < lmLocs[m_selectedTab]->getNumberOfElements() - 1);
    butMoveUp->setEnabled(selected >= 1 && selected < lmLocs[m_selectedTab]->getNumberOfElements());
    butFollowWarp->setEnabled(valid && m_selectedTab == Locs::Warps);
}

#ifdef WAPMAP_ENABLE_IMGUI
void winLocationsBrowser::DrawImGui() {
    if (!nativeOpen || busy) return;
    auto& e = *GV->editState;
    if (!document || document != e.hParser) { Close(); return; }
    if (!e._isOnTop()) return;
    if (positionOnOpen) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos({viewport->Pos.x + 160.f, viewport->Pos.y + LAY_VIEWPORT_Y + 40.f}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowFocus(); positionOnOpen = false;
    }
    ImGui::SetNextWindowSize({250, 401}, ImGuiCond_Always);
    const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
    const auto title = LegacyText::ToUtf8(getStr(L"WinCaption"), cp) + "###WapMapLocations";
    bool visible = true;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, getStr(L"WinCaption"));
        const auto area = myWin.getChildrenArea(); const auto origin = ImGui::GetWindowPos();
        const auto position = [&](float x, float y) { ImGui::SetCursorPos({area.x + x, area.y + y}); };
        auto* draw = ImGui::GetWindowDrawList();
        float tabX = 0;
        for (int i = 0; i < Locs::COUNT; ++i) {
            wchar_t key[32]; wsprintfW(key, L"Tab_%d", i); const char* caption = getStr(key);
            const float width = GV->fntMyriad16->GetStringWidth(caption) + 8.f;
            ImGui::PushID(i); position(tabX, 8);
            if (ImGui::InvisibleButton("##tab", {width, 25}, ImGuiButtonFlags_EnableNav) && m_selectedTab != i) {
                tabarLocs->setSelectedTab(i); m_selectedTab = i;
                lbLocs->setListModel(lmLocs[i]); lbLocs->setSelected(-1);
                RefreshLists(); resetScroll = true;
            }
            const auto p = ImGui::GetItemRectMin();
            draw->AddRectFilled(p, {p.x + width, p.y + 25}, m_selectedTab == i ? IM_COL32(62, 62, 62, 255) : IM_COL32(35, 35, 35, 255));
            draw->AddLine({p.x + width - 1, p.y}, {p.x + width - 1, p.y + 25}, IM_COL32(86, 86, 86, 255));
            WapMapImGui::BitmapText(draw, *hge, *GV->fntMyriad16, {p.x + 4, p.y + 4}, caption);
            ImGui::PopID(); tabX += width;
        }
        const auto queue = [&](gcn::Widget* widget) {
            auto* loc = lmLocs[m_selectedTab]->getLocation(lbLocs->getSelected());
            if (!loc) return;
            pendingAction = widget; pendingLocation = *loc; pendingTab = m_selectedTab;
        };
        position(1, 33);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, WapMapImGui::Grey(0x13));
        ImGui::PushStyleColor(ImGuiCol_Header, {0.08f, 0.52f, 0.89f, 1.f});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});
        if (ImGui::BeginChild("locations", {246, 310})) {
            if (resetScroll) { ImGui::SetScrollY(0); resetScroll = false; }
            auto* model = lmLocs[m_selectedTab];
            for (int i = 0; i < model->getNumberOfElements(); ++i) {
                ImGui::PushID(i);
                const auto p = ImGui::GetCursorScreenPos();
                const auto text = model->getElementAt(i);
                if (ImGui::Selectable("##location", lbLocs->getSelected() == i, ImGuiSelectableFlags_None, {0, 20})) {
                    lbLocs->setSelected(i); queue(lbLocs);
                }
                WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, {p.x + 2, p.y + 2}, text.c_str());
                ImGui::PopID();
            }
        }
        ImGui::EndChild(); ImGui::PopStyleVar(2); ImGui::PopStyleColor(2);
        SHR::But* buttons[] = {butAddFav, butDelete, butEdit, butMoveDown, butMoveUp, butFollowWarp};
        const char* tips[] = {GETL(Lang_AddFavLocation), GETL(Lang_Delete), getStr(L"Edit"), GETL(Lang_MoveDown), GETL(Lang_MoveUp), getStr(L"FollowWarp")};
        const int selected = lbLocs->getSelected(), count = lmLocs[m_selectedTab]->getNumberOfElements();
        const bool valid = selected >= 0 && selected < count;
        const bool enabled[] = {valid && m_selectedTab != Locs::Favourites, valid, valid,
            valid && selected + 1 < count, valid && selected > 0, valid && m_selectedTab == Locs::Warps};
        for (int i = 0; i < 6; ++i) {
            ImGui::PushID(i + 10); ImGui::BeginDisabled(!enabled[i]);
            position(static_cast<float>(buttons[i]->getX()), static_cast<float>(buttons[i]->getY()));
            if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, "", {32, 30})) queue(buttons[i]);
            const auto p = ImGui::GetItemRectMin(); const auto* icon = buttons[i]->getIcon();
            if (icon) WapMapImGui::Sprite(draw, *hge, icon, {p.x + (32 - icon->GetWidth()) / 2, p.y + (30 - icon->GetHeight()) / 2},
                IM_COL32(255, 255, 255, static_cast<int>(255 * ImGui::GetStyle().Alpha)));
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                const auto tip = LegacyText::ToUtf8(tips[i], cp); ImGui::SetTooltip("%s", tip.c_str());
            }
            ImGui::EndDisabled(); ImGui::PopID();
        }
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) visible = false;
    }
    ImGui::End(); if (!visible) Close();
}
#endif
