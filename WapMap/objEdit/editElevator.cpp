#include "editElevator.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../../shared/commonFunc.h"
#include "../cObjectUserData.h"
#include <cmath>

extern HGE *hge;

namespace ObjEdit {

    cEditObjElevator::cEditObjElevator(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enElevator;

        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Elevator", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 350, 480));
        win->setClose(1);
        win->addActionListener(hAL);
        win->setShadow(0);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        win->add(vpAdv);

        labType = new SHR::Lab(GETL2S("EditObj_Elevator", "Type"));
        labType->adjustSize();
        win->add(labType, 10, 20);

        type = new SHR::ComboBut(GV->hGfxInterface);
        type->addEntry(SHR::ComboButEntry(0, "Standard", GETL2S("EditObj_Elevator", "Standard_TT")));
        type->addEntry(SHR::ComboButEntry(0, "Start", GETL2S("EditObj_Elevator", "Start_TT")));
        type->addEntry(SHR::ComboButEntry(0, "Stop", GETL2S("EditObj_Elevator", "Stop_TT")));
        type->addEntry(SHR::ComboButEntry(0, "Trigger", GETL2S("EditObj_Elevator", "Trigger_TT")));
        type->addActionListener(hAL);
        type->adjustSize();
        win->add(type, 16 + labType->getWidth(), 19);

        cbOneWay = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Elevator", "OneWay"));
        cbOneWay->adjustSize();
        cbOneWay->addActionListener(hAL);
        cbOneWay->SetTooltip(GETL2S("EditObj_Elevator", "OneWayDesc"));
        win->add(cbOneWay, win->getWidth() - cbOneWay->getWidth() - 14, 21);

        if (!strcmp(hTempObj->GetLogic(), "Elevator"))
            type->setSelectedEntryID(0);
        else if (!strcmp(hTempObj->GetLogic(), "StartElevator") || !strcmp(hTempObj->GetLogic(), "OneWayStartElevator"))
            type->setSelectedEntryID(1);
        else if (!strcmp(hTempObj->GetLogic(), "StopElevator"))
            type->setSelectedEntryID(2);
        else if (!strcmp(hTempObj->GetLogic(), "TriggerElevator") || !strcmp(hTempObj->GetLogic(), "OneWayTriggerElevator"))
            type->setSelectedEntryID(3);

        int width = hTempObj->GetParam(WWD::Param_Width);
        int height = hTempObj->GetParam(WWD::Param_Height);

        if (type->getSelectedEntryID() <= 0) {
            if (width > 0) {
                int x = hTempObj->GetParam(WWD::Param_LocationX);
                rArea.x1 = x - width / 2;
                rArea.x2 = x + width / 2;
            } else {
                rArea.x1 = hTempObj->GetParam(WWD::Param_MinX);
                rArea.x2 = hTempObj->GetParam(WWD::Param_MaxX);
                width = rArea.x2 - rArea.x1;
            }

            if (height > 0) {
                int y = hTempObj->GetParam(WWD::Param_LocationY);
                rArea.y1 = y - height / 2;
                rArea.y2 = y + height / 2;
            } else {
                rArea.y1 = hTempObj->GetParam(WWD::Param_MinY);
                rArea.y2 = hTempObj->GetParam(WWD::Param_MaxY);
                height = rArea.y2 - rArea.y1;
            }
        } else {
            rArea.x1 = hTempObj->GetParam(WWD::Param_MinX);
            rArea.x2 = hTempObj->GetParam(WWD::Param_MaxX);
            rArea.y1 = hTempObj->GetParam(WWD::Param_MinY);
            rArea.y2 = hTempObj->GetParam(WWD::Param_MaxY);

            if (!rArea.x1) rArea.x1 = GetUserDataFromObj(obj)->GetX();
            if (!rArea.x2) rArea.x2 = GetUserDataFromObj(obj)->GetX();
            if (!rArea.y1) rArea.y1 = GetUserDataFromObj(obj)->GetY();
            if (!rArea.y2) rArea.y2 = GetUserDataFromObj(obj)->GetY();

            width = rArea.x2 - rArea.x1;
            height = rArea.y2 - rArea.y1;
        }

        int dir = hTempObj->GetParam(WWD::Param_Direction);
        if (!dir) {
            if (width && height) hTempObj->SetParam(WWD::Param_Direction, dir = 3);
            else if (width) hTempObj->SetParam(WWD::Param_Direction, dir = 6);
            else if (height) hTempObj->SetParam(WWD::Param_Direction, dir = 2);
        }

        cbOneWay->setSelected(rArea.x1 == rArea.x2 && rArea.y1 == rArea.y2 && rArea.x1 != 0 && rArea.y2 != 0 ||
                              std::string(hTempObj->GetLogic()).starts_with("OneWay"));

#define AREA_SECTION_Y 52

        labMoveArea = new SHR::Lab(GETL2S("EditObj_Elevator", "MoveArea"));
        labMoveArea->adjustSize();
        win->add(labMoveArea, 10, AREA_SECTION_Y + 10);

        #define InCorner(val, min, max) ((min == 0 && max == 0) || val == min || val == max)

        bool bAutomatic = dir && InCorner(GetUserDataFromObj(obj)->GetX(), rArea.x1, rArea.x2)
                && InCorner(GetUserDataFromObj(obj)->GetY(), rArea.y1, rArea.y2);
        std::string group = std::to_string((int)this);

        automatic = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Elevator", "Automatic"), group, bAutomatic);
        automatic->adjustSize();
        automatic->addActionListener(hAL);

        hTravelDistPick = new cProcPickXYLockable(GETL2S("EditObj_Elevator", "Distance"),
                GETL2S("EditObj_Elevator", "Horizontal"), width,
                GETL2S("EditObj_Elevator", "Vertical"), height,
                width != 0 && width == height, GETL2S("EditObj_Elevator", "LockAspect_TT"));
        hTravelDistPick->addActionListener(hAL);
        hTravelDistPick->setEnabled(bAutomatic);

        manual = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Elevator", "Manual"), group, !bAutomatic);
        manual->adjustSize();
        manual->addActionListener(hAL);

        hRectPick = new cProcPickRect(hTempObj);
        hRectPick->setValues(rArea.x1, rArea.y1, rArea.x2, rArea.y2);
        hRectPick->setActionListener(hAL);
        hRectPick->setType(PickRect_MinMax);
        hRectPick->setEnabled(!bAutomatic);

        int manualX = win->getWidth() - hRectPick->getWidth() - 15;

        win->add(automatic, 10, AREA_SECTION_Y + 40);
        win->add(manual, manualX, AREA_SECTION_Y + 40);
        win->add(hTravelDistPick, 10, AREA_SECTION_Y + 70);
        win->add(hRectPick, manualX, AREA_SECTION_Y + 70);

#define SPEED_SECTION_Y 260

        bool regularElevator = !strcmp(obj->GetLogic(), "Elevator");

        int speedX = hTempObj->GetParam(WWD::Param_SpeedX);
        int speedY = hTempObj->GetParam(WWD::Param_SpeedY);
        if (!speedX) {
            speedX = DEFAULT_ELEVATOR_SPEED;
            hTempObj->SetParam(WWD::Param_SpeedX, speedX);
        }
        else if (speedX < 0) speedX = regularElevator ? 0 : 9999;
        if (!speedY) {
            speedY = DEFAULT_ELEVATOR_SPEED;
            hTempObj->SetParam(WWD::Param_SpeedY, speedY);
        }
        else if (speedY < 0) speedY = regularElevator ? 0 : 9999;

        if (speedX > 9999) speedX = 9999;
        if (speedY > 9999) speedY = 9999;

        hSpeedPick = new cProcPickXYLockable(GETL2S("EditObj_Elevator", "Speed"),
                GETL2S("EditObj_Elevator", "Horizontal"), speedX,
                GETL2S("EditObj_Elevator", "Vertical"), speedY,
                speedX != 0 && speedX == speedY, GETL2S("EditObj_Elevator", "LockAspect_TT"));
        hSpeedPick->addActionListener(hAL);
        win->add(hSpeedPick, 10, SPEED_SECTION_Y + 10);

        labSpeedWarning = new SHR::Lab(GETL2S("EditObj_Elevator", "SpeedWarning"));
        labSpeedWarning->adjustSize();
        UpdateSpeedWarning();
        win->add(labSpeedWarning, 10, SPEED_SECTION_Y + 100);

        int xOffset = win->getWidth() - 3*32 - 16;
        int yOffset = SPEED_SECTION_Y + 10;

        labDirection = new SHR::Lab(GETL2S("EditObj_Elevator", "Direction"));
        labDirection->adjustSize();
        win->add(labDirection, xOffset, yOffset);

        yOffset += 30;

        butDirection[0] = hState->MakeButton(xOffset, yOffset, Icon_UpLeft, win);
        butDirection[1] = hState->MakeButton(xOffset + 32, yOffset, Icon_Up, win);
        butDirection[2] = hState->MakeButton(xOffset + 64, yOffset, Icon_UpRight, win);
        butDirection[3] = hState->MakeButton(xOffset, yOffset + 32, Icon_Left, win);
        butDirection[4] = hState->MakeButton(xOffset + 64, yOffset + 32, Icon_Right, win);
        butDirection[5] = hState->MakeButton(xOffset, yOffset + 64, Icon_DownLeft, win);
        butDirection[6] = hState->MakeButton(xOffset + 32, yOffset + 64, Icon_Down, win);
        butDirection[7] = hState->MakeButton(xOffset + 64, yOffset + 64, Icon_DownRight, win);
        for (auto & i : butDirection) {
            i->removeActionListener(GV->editState->al);
            i->addActionListener(hAL);
        }

        hTempObj->SetParam(WWD::Param_Width, 0);
        hTempObj->SetParam(WWD::Param_Height, 0);

        UpdateDirection(true);
        UpdateOneWayCheckBoxEnabled();
        UpdateAllowedDirections();

        win->add(_butAddNext, 120, 420);
        win->add(_butSave, 233, 420);
    }

    void cEditObjElevator::UpdateOneWayCheckBoxEnabled() {
        switch (type->getSelectedEntryID()) {
            case 0:
            case 2:
                cbOneWay->setSelected(false);
                cbOneWay->setEnabled(false);
                break;
            case 1:
            case 3:
                cbOneWay->setEnabled(true);
                break;
        }
    }

    cEditObjElevator::~cEditObjElevator() {
        delete labType;
        delete type;
        delete cbOneWay;
        delete labMoveArea;
        delete automatic;
        delete manual;
        delete hTravelDistPick;
        delete hRectPick;
        delete hSpeedPick;
        delete labSpeedWarning;
        delete labDirection;

        for (auto & i : butDirection)
            delete i;

        delete win;
        hState->vPort->MarkToRedraw(true);
    }

    void cEditObjElevator::Save() {

    }

    void cEditObjElevator::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = true;
            return;
        } else if (actionEvent.getSource() == cbOneWay) {
            UpdateLogic();
            hTempObj->SetParam(WWD::Param_Direction, iDirection);
            UpdateDirection();
            UpdateAllowedDirections();
        } else if (actionEvent.getSource() == type) {
            bool needsDirUpdate = cbOneWay->isSelected();
            UpdateLogic();
            UpdateOneWayCheckBoxEnabled();
            if (needsDirUpdate) {
                UpdateDirection();
            }
            UpdateAllowedDirections();
            UpdateSpeedWarning();
        } else if (actionEvent.getSource() == automatic || actionEvent.getSource() == manual) {
            hTravelDistPick->setEnabled(automatic->isSelected());
            hRectPick->setEnabled(manual->isSelected());

            if (automatic->isSelected()) {
                for (auto & i : butDirection)
                    i->setEnabled(true);
                UpdateLogic();
                UpdateDirection();
                UpdateAllowedDirections();
            } else {
                for (auto & pBut : butDirection)
                    pBut->setEnabled(true);
                UpdateDirection();
            }
        } else if (actionEvent.getSource() == hTravelDistPick) {
            if (actionEvent.getId() == "CHANGE_X" && hTravelDistPick->isEnabledX()) {
                if (iDirection == 1 || iDirection == 4 || iDirection == 7) { // goes left
                    if (!rArea.x2) rArea.x2 = hTempObj->GetX();
                    rArea.x1 = rArea.x2 - hTravelDistPick->getValueX();
                } else {
                    if (!rArea.x1) rArea.x2 = hTempObj->GetX();
                    rArea.x2 = rArea.x1 + hTravelDistPick->getValueX();
                }
                hRectPick->setValues(rArea.x1, -1, rArea.x2, -1);
            } else if (actionEvent.getId() == "CHANGE_Y" && hTravelDistPick->isEnabledY()) {
                if (iDirection < 4) { // goes up
                    if (!rArea.y2) rArea.y2 = hTempObj->GetY();
                    rArea.y1 = rArea.y2 - hTravelDistPick->getValueY();
                } else {
                    if (!rArea.y1) rArea.y1 = hTempObj->GetY();
                    rArea.y2 = rArea.y1 + hTravelDistPick->getValueY();
                }
                hRectPick->setValues(-1, rArea.y1, -1, rArea.y2);
            }
        } else if (actionEvent.getSource() == hSpeedPick) {
            if (actionEvent.getId() == "CHANGE_X") {
                hTempObj->SetParam(WWD::Param_SpeedX, hSpeedPick->getValueX());
            } else if (actionEvent.getId() == "CHANGE_Y") {
                hTempObj->SetParam(WWD::Param_SpeedY, hSpeedPick->getValueY());
            }
            UpdateSpeedWarning();
        } else {
            for (int i = 0; i < 8; i++) {
                if (actionEvent.getSource() == butDirection[i]) {
                    for (auto & pBut : butDirection)
                        pBut->setEnabled(true);
                    hTempObj->SetParam(WWD::Param_Direction, i < 4 ? i + 1 : i + 2);
                    UpdateDirection();
                    UpdateAllowedDirections();
                    break;
                }
            }
        }
    }

    void cEditObjElevator::_Think(bool bMouseConsumed) {
        if (!bMouseConsumed)
            hRectPick->Think();
    }

    void cEditObjElevator::Draw() {
        int dx, dy;
        win->getAbsolutePosition(dx, dy);
        dy += 15;

        hge->Gfx_RenderLine(dx, dy + SPEED_SECTION_Y, dx + win->getWidth(), dy + SPEED_SECTION_Y, GV->colLineDark);
        hge->Gfx_RenderLine(dx, dy + SPEED_SECTION_Y + 1, dx + win->getWidth(), dy + SPEED_SECTION_Y + 1, GV->colLineBright);

        hge->Gfx_RenderLine(dx, dy + AREA_SECTION_Y, dx + win->getWidth(), dy + AREA_SECTION_Y, GV->colLineDark);
        hge->Gfx_RenderLine(dx, dy + AREA_SECTION_Y + 1, dx + win->getWidth(), dy + AREA_SECTION_Y + 1, GV->colLineBright);
    }

    void cEditObjElevator::UpdateLogic() {
        bool oneWay = cbOneWay->isSelected();
        switch (type->getSelectedEntryID()) {
            case 0:
                hTempObj->SetLogic("Elevator");
                break;
            case 1:
                hTempObj->SetLogic(oneWay ? "OneWayStartElevator" : "StartElevator");
                break;
            case 2:
                hTempObj->SetLogic("StopElevator");
                break;
            case 3:
                hTempObj->SetLogic(oneWay ? "OneWayTriggerElevator" : "TriggerElevator");
                break;
        }
    }

#define GET_X_MINMAX() int tX = hTravelDistPick->getValueX(); \
    if (!tX) { tX = hTravelDistPick->getValueY(); if (!tX) tX = 150; int sY = hSpeedPick->getValueY(); \
    hTempObj->SetParam(WWD::Param_SpeedX, sY); \
    hTravelDistPick->setValueX(tX); hTravelDistPick->setLocked(true); hSpeedPick->setValueX(sY); } \
    int x = GetUserDataFromObj(hTempObj)->GetX(); \
    rArea.x1 = left ? x - tX : x; \
    rArea.x2 = left ? x : x + tX;

#define GET_Y_MINMAX() int tY = hTravelDistPick->getValueY(); \
    if (!tY) { tY = hTravelDistPick->getValueX(); if (!tY) tY = 150; int sX = hSpeedPick->getValueX(); \
    hTempObj->SetParam(WWD::Param_SpeedY, sX); \
    hTravelDistPick->setValueY(tY); hTravelDistPick->setLocked(true); hSpeedPick->setValueY(sX); } \
    int y = GetUserDataFromObj(hTempObj)->GetY(); \
    rArea.y1 = top ? y - tY : y; \
    rArea.y2 = top ? y : y + tY;

    void cEditObjElevator::UpdateDirection(bool init) {
        iDirection = hTempObj->GetParam(WWD::Param_Direction);
        if (init && automatic->isSelected()) {
            if (GetUserDataFromObj(hTempObj)->GetX() == rArea.x1 && (iDirection == 1 || iDirection == 4 || iDirection == 7)) {
                iDirection += 2;
            } else if (GetUserDataFromObj(hTempObj)->GetX() == rArea.x2 && (iDirection == 3 || iDirection == 6 || iDirection == 9)) {
                iDirection -= 2;
            }

            if (GetUserDataFromObj(hTempObj)->GetY() == rArea.y1 && iDirection < 4) {
                iDirection += 6;
            } else if (GetUserDataFromObj(hTempObj)->GetY() == rArea.y2 && iDirection > 6) {
                iDirection -= 6;
            }
        }

        if (iDirection > 5 && iDirection <= 9) {
            butDirection[iDirection - 2]->setEnabled(false);
        } else if (iDirection > 0 && iDirection < 5) {
            butDirection[iDirection - 1]->setEnabled(false);
        }

        bool horizontal = iDirection == 4 || iDirection == 6;
        bool vertical = iDirection == 2 || iDirection == 8;
        hSpeedPick->setEnabledX(!vertical);
        hSpeedPick->setEnabledY(!horizontal);

        hTravelDistPick->setEnabledX(!vertical);
        hTravelDistPick->setEnabledY(!horizontal);

        if (!init && automatic->isSelected()) {
            bool top = iDirection < 4;
            bool left = iDirection % 3 == 1;
            if (horizontal) {
                GET_X_MINMAX();
                hTempObj->SetParam(WWD::Param_MinX, rArea.x1);
                hTempObj->SetParam(WWD::Param_MaxX, rArea.x2);
                hTempObj->SetParam(WWD::Param_MinY, 0);
                hTempObj->SetParam(WWD::Param_MaxY, 0);
                hRectPick->setValues(rArea.x1, 0, rArea.x2, 0);
            } else if (vertical) {
                GET_Y_MINMAX();
                hTempObj->SetParam(WWD::Param_MinY, rArea.y1);
                hTempObj->SetParam(WWD::Param_MaxY, rArea.y2);
                hTempObj->SetParam(WWD::Param_MinX, 0);
                hTempObj->SetParam(WWD::Param_MaxX, 0);
                hRectPick->setValues(0, rArea.y1, 0, rArea.y2);

                if (cbOneWay->isSelected()) {
                    hTempObj->SetParam(WWD::Param_Direction, iDirection == 8 ? 2 : 8);
                }
            } else {
                GET_X_MINMAX();
                GET_Y_MINMAX();

                if (cbOneWay->isSelected()) {
                    int tgX = left ? rArea.x1 : rArea.x2;
                    int tgY = top ? rArea.y1 : rArea.y2;

                    hTempObj->SetParam(WWD::Param_MinX, tgX);
                    hTempObj->SetParam(WWD::Param_MaxX, tgX);
                    hTempObj->SetParam(WWD::Param_MinY, tgY);
                    hTempObj->SetParam(WWD::Param_MaxY, tgY);
                    hRectPick->setValues(rArea.x1 = tgX, rArea.y1 = tgY, rArea.x2 = tgX, rArea.y2 = tgY);

                    if (iDirection % 3 == 0) {
                        hTempObj->SetParam(WWD::Param_Direction, iDirection == 3 ? 9 : 3);
                    }
                }
                else {
                    hTempObj->SetParam(WWD::Param_MinX, rArea.x1);
                    hTempObj->SetParam(WWD::Param_MaxX, rArea.x2);
                    hTempObj->SetParam(WWD::Param_MinY, rArea.y1);
                    hTempObj->SetParam(WWD::Param_MaxY, rArea.y2);
                    hRectPick->setValues(rArea.x1, rArea.y1, rArea.x2, rArea.y2);

                    if (iDirection == 1 || iDirection == 7 || iDirection == 9) {
                        hTempObj->SetParam(WWD::Param_Direction, 3);
                    }
                }
            }
        }
    }

    void cEditObjElevator::UpdateAllowedDirections() {
        bool enabled = type->getSelectedEntryID() == 0 || !automatic->isSelected();
        butDirection[2]->setEnabled(enabled && iDirection != 3);
        butDirection[5]->setEnabled(enabled && iDirection != 7);
    }

    void cEditObjElevator::UpdateSpeedWarning() {
        bool regularElevator = !strcmp(hTempObj->GetLogic(), "Elevator");
        int speedX = hSpeedPick->isEnabledX() ? hSpeedPick->getValueX() : 0,
            speedY = hSpeedPick->isEnabledY() ? hSpeedPick->getValueY() : 0;
        labSpeedWarning->setVisible(!regularElevator && ((speedX > 0 && speedX < 60) || (speedY > 0 && speedY < 60)));
    }

    void cEditObjElevator::ObjectMovedInsideEasyEdit(int fromX, int fromY) {
        if (automatic->isSelected()) {
            int diffX = GetUserDataFromObj(hTempObj)->GetX() - fromX,
                diffY = GetUserDataFromObj(hTempObj)->GetY() - fromY;

            if (hTempObj->GetParam(WWD::Param_MinX) != 0) {
                rArea.x1 += diffX;
                hTempObj->SetParam(WWD::Param_MinX,
                                   hTempObj->GetParam(WWD::Param_MinX) + diffX);
            }
            if (hTempObj->GetParam(WWD::Param_MinY) != 0) {
                rArea.y1 += diffY;
                hTempObj->SetParam(WWD::Param_MinY,
                                   hTempObj->GetParam(WWD::Param_MinY) + diffY);
            }
            if (hTempObj->GetParam(WWD::Param_MaxX) != 0) {
                rArea.x2 += diffX;
                hTempObj->SetParam(WWD::Param_MaxX,
                                   hTempObj->GetParam(WWD::Param_MaxX) + diffX);
            }
            if (hTempObj->GetParam(WWD::Param_MaxY) != 0) {
                rArea.y2 += diffY;
                hTempObj->SetParam(WWD::Param_MaxY,
                                   hTempObj->GetParam(WWD::Param_MaxY) + diffY);
            }
        }
    }
}
