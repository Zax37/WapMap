#include "goToLocation.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../states/dialog.h"

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
    auto* mainPlane = GV->editState->hParser->GetMainPlane();
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
    if (!v) return;
    v->erase(v->begin() + i);
}

stLocation* LocationsList::getLocation(int i) {
    if (!v || i >= (int)(v->size())) return NULL;
    return &(v->at(i));
}

int LocationsList::renameLocation(int index, std::string& newName) {
    if (!v)
        return LocName_ErrorGeneric;
    auto* location = getLocation(index);

    if (location->Object) {
        if (!validateObjectPtr(location->Object))
            return LocName_ErrorGeneric;
        location->Object->SetName(newName.c_str());
        return LocName_Success;
    }

    if (newName.empty())
        return LocName_ErrorEmpty;

    for (int i = 0; i < v->size(); ++i) {
        if (getElementAt(i) == newName)
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
    if (!v || i >= (int)(v->size()))
        return "";
    if (!isObjectList)
        return v->at(i).Name;

    auto *object = v->at(i).Object;
    if (!validateObjectPtr(object)) {
        deleteLocation(i);
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
    RefreshLists();
    myWin.setPosition(160, LAY_VIEWPORT_Y + 40);
    myWin.setVisible(true);
    myWin.getParent()->moveToTop(&myWin);
}

void winLocationsBrowser::OnDocumentChange() {
    RefreshLists();
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

    if (actionEvent.getSource() == butAddFav) {
        std::string init = lmLocs[m_selectedTab]->getElementAt(i);
        int ret = State::InputDialog(GETL(Lang_WM), getStr(L"DialogInputName"), ST_DIALOG_BUT_OKCANCEL, init.c_str());
        if (ret == RETURN_OK && init != GV->szLastInputDialogText) {
            GV->szLastInputDialogText.resize(63);
            lmLocs[Locs::Favourites]->addLocation(GV->szLastInputDialogText, location->X, location->Y);
            GV->editState->MarkUnsaved();
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
    int tab = tabarLocs->getSelectedTabIndex();
    if (m_selectedTab != tab) {
        lbLocs->setListModel(lmLocs[tab]);
        m_selectedTab = tab;
        saLocs->setVerticalScrollAmount(0);
    }
    int selected = lbLocs->getSelected();
    butAddFav->setEnabled(selected >= 0 && m_selectedTab != Locs::Favourites);
    butDelete->setEnabled(selected >= 0);
    butEdit->setEnabled(selected >= 0);
    butMoveDown->setEnabled(selected >= 0 && selected < lmLocs[m_selectedTab]->getNumberOfElements() - 1);
    butMoveUp->setEnabled(selected >= 1 && selected < lmLocs[m_selectedTab]->getNumberOfElements());
    butFollowWarp->setEnabled(selected >= 0 && m_selectedTab == Locs::Warps);
}