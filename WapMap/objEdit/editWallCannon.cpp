#include "editWallCannon.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include <cmath>
#include <hgeVector.h>
#include "cannonGeometry.h"
#include "../windows/coordinateNavigation.h"

extern HGE *hge;

namespace ObjEdit {
    cEditObjWallCannon::cEditObjWallCannon(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enWallCannon;
        bSkullCannon = (GV->editState->hParser->GetBaseLevel() == 9);
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_WallCannon", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 310, 135 + 15));
        win->setClose(1);
        win->addActionListener(hAL);
        win->setShadow(0);
        win->add(vpAdv);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        char tmp[50];
        sprintf(tmp, "%p", this);

        labDir = new SHR::Lab(GETL2S("EditObj_WallCannon", "Direction"));
        labDir->adjustSize();
        win->add(labDir, 5, 10);

        rbType[0] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_WallCannon", "Left"), tmp, 0);
        rbType[0]->adjustSize();
        rbType[0]->addActionListener(hAL);
        win->add(rbType[0], 125, 10);
        rbType[1] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_WallCannon", "Right"), tmp, 0);
        rbType[1]->adjustSize();
        rbType[1]->addActionListener(hAL);
        win->add(rbType[1], 175 + 32, 10);

        if (bSkullCannon) {
            rbType[!(hTempObj->GetDrawFlags() & WWD::Flag_dr_Mirror)]->setSelected(1);
        } else {
            rbType[0]->setSelected(!strcmp(hTempObj->GetImageSet(), "LEVEL_TOWERCANNONLEFT"));
            rbType[1]->setSelected(!strcmp(hTempObj->GetImageSet(), "LEVEL_TOWERCANNONRIGHT"));
        }

        if (!rbType[0]->isSelected() && !rbType[1]->isSelected()) {
            rbType[1]->setSelected(1);
            if (bSkullCannon) {
                if (!(hTempObj->GetDrawFlags() & WWD::Flag_dr_Mirror)) {
                    hTempObj->SetDrawFlags((WWD::OBJ_DRAW_FLAGS) (hTempObj->GetDrawFlags() + WWD::Flag_dr_Mirror));
                    GetUserDataFromObj(hTempObj)->SyncToObj();
                }
            } else
                hTempObj->SetImageSet("LEVEL_TOWERCANNONRIGHT");
            hState->vPort->MarkToRedraw();
        }

        bOrient = rbType[1]->isSelected();

        win->add(_butAddNext, 204, 35 + 6);
        win->add(_butSave, 204, 97 - 6);

        iSpeedX = hTempObj->GetParam(WWD::Param_SpeedX) == 0 ? 500 : hTempObj->GetParam(WWD::Param_SpeedX);
        iSpeedY = hTempObj->GetParam(WWD::Param_SpeedY);
        const auto initial = CannonGeometry::Components(iSpeedX, iSpeedY, bOrient);
        iLinearSpeed = initial ? initial->magnitude : 500;

        labSpeedX = new SHR::Lab(GETL2S("EditObj_WallCannon", "SpeedX"));
        labSpeedX->adjustSize();
        win->add(labSpeedX, 5, 35);

        sprintf(tmp, "%d", iSpeedX);
        tfSpeedX = new SHR::TextField(tmp);
        tfSpeedX->setDimension(gcn::Rectangle(0, 0, 50, 20));
        tfSpeedX->SetNumerical(1, 1);
        tfSpeedX->addActionListener(hAL);
        win->add(tfSpeedX, 150, 35);

        labSpeedY = new SHR::Lab(GETL2S("EditObj_WallCannon", "SpeedY"));
        labSpeedY->adjustSize();
        win->add(labSpeedY, 5, 60);

        sprintf(tmp, "%d", iSpeedY);
        tfSpeedY = new SHR::TextField(tmp);
        tfSpeedY->setDimension(gcn::Rectangle(0, 0, 50, 20));
        tfSpeedY->SetNumerical(1, 1);
        tfSpeedY->addActionListener(hAL);
        win->add(tfSpeedY, 150, 60);

        labLinSpeed = new SHR::Lab(GETL2S("EditObj_WallCannon", "LinSpeed"));
        labLinSpeed->adjustSize();
        win->add(labLinSpeed, 5, 85);

        sprintf(tmp, "%d", iLinearSpeed);
        tfLinSpeed = new SHR::TextField(tmp);
        tfLinSpeed->setDimension(gcn::Rectangle(0, 0, 50, 20));
        tfLinSpeed->SetNumerical(1, 0);
        tfLinSpeed->addActionListener(hAL);
        win->add(tfLinSpeed, 150, 85);

        labAngle = new SHR::Lab(GETL2S("EditObj_WallCannon", "Angle"));
        labAngle->adjustSize();
        win->add(labAngle, 5, 110);

        iRealSpeedX = initial ? initial->worldX : bOrient ? 500 : -500;
        iAngle = initial ? initial->angle : 0;
        sprintf(tmp, "%d", iAngle);
        tfAngle = new SHR::TextField(tmp);
        tfAngle->setDimension(gcn::Rectangle(0, 0, 50, 20));
        tfAngle->SetNumerical(1, 1);
        tfAngle->addActionListener(hAL);
        win->add(tfAngle, 150, 110);

        bDraggingArrow = 0;

        if (!hTempObj->GetParam(WWD::Param_LocationZ)) {
            hTempObj->SetParam(WWD::Param_LocationZ, 3990);
            GetUserDataFromObj(hTempObj)->SetZ(3990);
            hState->vPort->MarkToRedraw();
        }
        if (!ReadComponents()) { iSpeedX = 500; iSpeedY = 0; }
#ifdef WAPMAP_ENABLE_IMGUI
        nativeWindow = true;
        win->setVisible(false);
#endif
    }

    bool cEditObjWallCannon::ReadComponents() {
        const auto x = CoordinateNavigation::Parse(tfSpeedX->getText()), y = CoordinateNavigation::Parse(tfSpeedY->getText());
        const auto velocity = x && y ? CannonGeometry::Components(*x, *y, bOrient) : std::nullopt;
        tfSpeedX->setMarkedInvalid(!velocity); tfSpeedY->setMarkedInvalid(!velocity);
        _butSave->setEnabled(velocity.has_value() && !bDraggingArrow);
        if (!velocity) return false;
        iSpeedX = velocity->x; iSpeedY = velocity->y; iRealSpeedX = velocity->worldX;
        iLinearSpeed = velocity->magnitude; iAngle = velocity->angle;
        tfLinSpeed->setMarkedInvalid(false); tfAngle->setMarkedInvalid(false);
        UpdateLinearSpeed(); UpdateAngleInput();
        hState->vPort->MarkToRedraw();
        return true;
    }

    void cEditObjWallCannon::SyncInputs() {
        tfSpeedX->setText(std::to_string(iSpeedX)); tfSpeedY->setText(std::to_string(iSpeedY));
        tfSpeedX->setMarkedInvalid(false); tfSpeedY->setMarkedInvalid(false);
        tfLinSpeed->setMarkedInvalid(false); tfAngle->setMarkedInvalid(false);
        UpdateLinearSpeed(); UpdateAngleInput();
#ifdef WAPMAP_ENABLE_IMGUI
        for (auto* field : {tfSpeedX, tfSpeedY, tfLinSpeed, tfAngle}) nativeIntegers.erase(field);
#endif
    }

    void cEditObjWallCannon::SetArrowDragging(bool dragging) {
        bDraggingArrow = dragging; bAllowDragging = !dragging;
        for (auto* field : {tfSpeedX, tfSpeedY, tfLinSpeed, tfAngle}) field->setEnabled(!dragging);
        for (auto* radio : rbType) radio->setEnabled(!dragging);
        _butSave->setEnabled(!dragging);
    }

    void cEditObjWallCannon::HandleEscape() {
        if (bDraggingArrow) {
            SetArrowDragging(false); ReadComponents(); hState->vPort->MarkToRedraw(); return;
        }
        cObjEdit::HandleEscape();
    }

    void cEditObjWallCannon::Save() {
        if (!ReadComponents()) return;
        hTempObj->SetParam(WWD::Param_SpeedX, atoi(tfSpeedX->getText().c_str()));
        hTempObj->SetParam(WWD::Param_SpeedY, atoi(tfSpeedY->getText().c_str()));
    }

    cEditObjWallCannon::~cEditObjWallCannon() {
        for (int i = 0; i < 2; i++)
            delete rbType[i];
        delete labLinSpeed;
        delete labAngle;
        delete tfLinSpeed;
        delete tfAngle;
        delete labDir;
        delete tfSpeedX;
        delete tfSpeedY;
        delete labSpeedX;
        delete labSpeedY;
        delete win;
        hState->vPort->MarkToRedraw();
    }

    void cEditObjWallCannon::_Think(bool consumed) {
        if (bDraggingArrow && !hge->Input_GetKeyState(HGEK_LBUTTON)) {
            SetArrowDragging(false); SyncInputs(); ReadComponents(); return;
        }
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        if (consumed || hState->conMain->getWidgetAt(mx, my) != hState->vPort->GetWidget()) return;
        const int x = GetUserDataFromObj(hTempObj)->GetX(), y = GetUserDataFromObj(hTempObj)->GetY();
        if (bDraggingArrow) {
            const auto velocity = CannonGeometry::Drag(hState->Scr2WrdX(hState->GetActivePlane(), mx),
                hState->Scr2WrdY(hState->GetActivePlane(), my), x, y, bOrient, hge->Input_GetKeyState(HGEK_SHIFT));
            if (velocity) {
                iSpeedX = velocity->x; iSpeedY = velocity->y; iRealSpeedX = velocity->worldX;
                iLinearSpeed = velocity->magnitude; iAngle = velocity->angle;
                hState->vPort->MarkToRedraw();
            }
            return;
        }
        const float screenX = static_cast<float>(hState->Wrd2ScrX(hState->GetActivePlane(), x));
        const float screenY = static_cast<float>(hState->Wrd2ScrY(hState->GetActivePlane(), y));
        const float zoom = hState->fZoom;
        if (mx > screenX + (static_cast<float>(iRealSpeedX) - 30) * zoom && mx < screenX + (static_cast<float>(iRealSpeedX) + 30) * zoom &&
            my > screenY + (static_cast<float>(iSpeedY) - 30) * zoom && my < screenY + (static_cast<float>(iSpeedY) + 30) * zoom &&
            hge->Input_KeyDown(HGEK_LBUTTON)) {
            SyncInputs(); SetArrowDragging(true); _bDragging = false;
        }
    }

    void cEditObjWallCannon::Action(const gcn::ActionEvent& event) {
        auto* source = event.getSource();
        if (source == win) { bKill = true; return; }
        if (source == rbType[0] || source == rbType[1]) {
            const bool right = source == rbType[1];
            const auto x = CoordinateNavigation::Parse(tfSpeedX->getText()), y = CoordinateNavigation::Parse(tfSpeedY->getText());
            const auto velocity = x && y ? CannonGeometry::Components(*x, *y, right) : std::nullopt;
            const auto position = std::int64_t(hTempObj->GetParam(WWD::Param_LocationX)) + (right == bOrient || bSkullCannon ? 0 : right ? 49 : -49);
            if (!velocity || !CannonGeometry::Fits(position)) { rbType[bOrient ? 1 : 0]->setSelected(true); ReadComponents(); return; }
            bOrient = right;
            if (bSkullCannon) {
                const auto flags = static_cast<int>(hTempObj->GetDrawFlags());
                hTempObj->SetDrawFlags(static_cast<WWD::OBJ_DRAW_FLAGS>(right ? flags & ~WWD::Flag_dr_Mirror : flags | WWD::Flag_dr_Mirror));
            } else {
                hTempObj->SetLogic(right ? "TowerCannonRight" : "TowerCannonLeft");
                hTempObj->SetImageSet(right ? "LEVEL_TOWERCANNONRIGHT" : "LEVEL_TOWERCANNONLEFT");
                hTempObj->SetParam(WWD::Param_LocationX, static_cast<int>(position));
            }
            GetUserDataFromObj(hTempObj)->SyncToObj();
            hState->hPlaneData[hState->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(hTempObj);
            ReadComponents(); SyncInputs(); return;
        }
        if (source == tfSpeedX || source == tfSpeedY) { ReadComponents(); return; }
        if (source == tfAngle || source == tfLinSpeed) {
            const auto magnitude = CoordinateNavigation::Parse(tfLinSpeed->getText()), angle = CoordinateNavigation::Parse(tfAngle->getText());
            const auto velocity = magnitude && angle ? CannonGeometry::Polar(*magnitude, *angle, bOrient) : std::nullopt;
            tfLinSpeed->setMarkedInvalid(!velocity); tfAngle->setMarkedInvalid(!velocity);
            _butSave->setEnabled(velocity.has_value());
            if (!velocity) return;
            iSpeedX = velocity->x; iSpeedY = velocity->y; iRealSpeedX = velocity->worldX;
            iLinearSpeed = velocity->magnitude; iAngle = velocity->angle;
            tfSpeedX->setText(std::to_string(iSpeedX)); tfSpeedY->setText(std::to_string(iSpeedY));
            tfSpeedX->setMarkedInvalid(false); tfSpeedY->setMarkedInvalid(false);
            if (source != tfAngle) UpdateAngleInput();
            if (source != tfLinSpeed) UpdateLinearSpeed();
            hState->vPort->MarkToRedraw();
        }
    }

    void cEditObjWallCannon::UpdateLinearSpeed() {
        char tmp[50];
        sprintf(tmp, "%d", iLinearSpeed);
        tfLinSpeed->setText(tmp);
    }

    void cEditObjWallCannon::UpdateAngleInput() {
        char tmp[50];
        sprintf(tmp, "%d", iAngle);
        tfAngle->setText(tmp);
        tfAngle->setMarkedInvalid(iAngle > 360);
    }

    void cEditObjWallCannon::Draw() {
        int dx, dy;
        win->getAbsolutePosition(dx, dy);
        hge->Gfx_RenderLine(dx, dy + 47, dx + win->getWidth(), dy + 47, GV->colLineDark);
        hge->Gfx_RenderLine(dx, dy + 48, dx + win->getWidth(), dy + 48, GV->colLineBright);

        hge->Gfx_RenderLine(dx, dy + 47 + 52, dx + 205, dy + 47 + 52, GV->colLineDark);
        hge->Gfx_RenderLine(dx, dy + 48 + 52, dx + 205, dy + 48 + 52, GV->colLineBright);

        hge->Gfx_RenderLine(dx + 205, dy + 47, dx + 205, dy + win->getHeight(), GV->colLineDark);
        hge->Gfx_RenderLine(dx + 206, dy + 47, dx + 206, dy + win->getHeight(), GV->colLineBright);
    }

    void cEditObjWallCannon::RenderObjectOverlay() {
        State::EditingWW *mn = GV->editState;
        mn->vPort->ClipScreen();
        int objx = mn->Wrd2ScrX(mn->GetActivePlane(), GetUserDataFromObj(hTempObj)->GetX()),
            objy = mn->Wrd2ScrY(mn->GetActivePlane(), GetUserDataFromObj(hTempObj)->GetY());
        hgeVector vecDest(iRealSpeedX, iSpeedY);
        //vecDest.Clamp(200);
        if (bDraggingArrow) {
            GV->sprArrowVerticalM->SetColor(COL_ORANGE_ARROW);
            GV->sprArrowVerticalU->SetColor(COL_ORANGE_ARROW);
        }
        float z = GV->editState->fZoom;
        mn->RenderArrow(objx, objy, objx + vecDest.x * z, objy + vecDest.y * z, 1, !bDraggingArrow);
    }
}
