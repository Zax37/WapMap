#include "editWallCannon.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include <cmath>
#include <hgeVector.h>

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
            hState->vPort->MarkToRedraw(1);
        }

        bOrient = rbType[1]->isSelected();

        win->add(_butAddNext, 204, 35 + 6);
        win->add(_butSave, 204, 97 - 6);

        iSpeedX = hTempObj->GetParam(WWD::Param_SpeedX) == 0 ? 500 : hTempObj->GetParam(WWD::Param_SpeedX);
        iSpeedY = hTempObj->GetParam(WWD::Param_SpeedY);
        iLinearSpeed = sqrt(pow(iSpeedX, 2) + pow(iSpeedY, 2));

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

        iRealSpeedX = iSpeedX;
        if (!bOrient)
            iRealSpeedX *= -1;

        iAngle = CalcAngle(iRealSpeedX, iSpeedY, bOrient);
        sprintf(tmp, "%d", iAngle);
        tfAngle = new SHR::TextField(tmp);
        tfAngle->setDimension(gcn::Rectangle(0, 0, 50, 20));
        tfAngle->SetNumerical(1, 0);
        tfAngle->addActionListener(hAL);
        win->add(tfAngle, 150, 110);

        bDraggingArrow = 0;
    }

    int cEditObjWallCannon::CalcAngle(int iX, int iY, bool bOr) {
        if (!iY) return (iX < 0 ? 180 : 0);
        hgeVector vec(iX, iY);
        float ret = vec.Angle() * 57.2957795;
        if (bOr) {
            if (ret < 0) ret = 360 + ret;
            return ret;
        } else {
            return ret + 180;
        }
    }

    void cEditObjWallCannon::Save() {
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
        hState->vPort->MarkToRedraw(1);
    }

    void cEditObjWallCannon::_Think(bool bConsumed) {
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        State::EditingWW *mn = GV->editState;
        if (mn->conMain->getWidgetAt(mx, my) != mn->vPort->GetWidget() || bConsumed) return;
        int objwx = GetUserDataFromObj(hTempObj)->GetX(),
                objwy = GetUserDataFromObj(hTempObj)->GetY();
        int objx = mn->Wrd2ScrX(mn->GetActivePlane(), objwx),
                objy = mn->Wrd2ScrY(mn->GetActivePlane(), objwy);
        hgeVector vecDest(iRealSpeedX, iSpeedY);
        //vecDest.Clamp(200);
        if (bDraggingArrow) {
            if (!hge->Input_GetKeyState(HGEK_LBUTTON)) {
                bDraggingArrow = 0;
                bAllowDragging = 1;
                return;
            }
            int wmx = mn->Scr2WrdX(mn->GetActivePlane(), mx),
                    wmy = mn->Scr2WrdY(mn->GetActivePlane(), my);
            iSpeedY = wmy - objwy;
            if (bOrient) iSpeedX = wmx - objwx;
            else iSpeedX = objwx - wmx;
            iRealSpeedX = iSpeedX;
            if (!bOrient) iRealSpeedX *= -1;
            char tmp[50];
            sprintf(tmp, "%d", iSpeedX);
            tfSpeedX->setText(tmp);
            sprintf(tmp, "%d", iSpeedY);
            tfSpeedY->setText(tmp);
            iAngle = CalcAngle(iRealSpeedX, iSpeedY, bOrient);
            iLinearSpeed = sqrt(pow(iSpeedX, 2) + pow(iSpeedY, 2));
            UpdateAngleInput();
            UpdateLinearSpeed();
        }
        if (mx > objx + vecDest.x - 15 && mx < objx + vecDest.x + 15 &&
            my > objy + vecDest.y - 15 && my < objy + vecDest.y + 15) {
            if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                _bDragging = 0;
                bAllowDragging = 0;
                bDraggingArrow = 1;
            }
        }
    }

    void cEditObjWallCannon::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == rbType[0]) {
            if (bSkullCannon) {
                if (!(hTempObj->GetDrawFlags() & WWD::Flag_dr_Mirror)) {
                    hTempObj->SetDrawFlags((WWD::OBJ_DRAW_FLAGS) (hTempObj->GetDrawFlags() + WWD::Flag_dr_Mirror));
                    GetUserDataFromObj(hTempObj)->SyncToObj();
                }
            } else {
                hTempObj->SetLogic("TowerCannonLeft");
                hTempObj->SetImageSet("LEVEL_TOWERCANNONLEFT");
            }
            hState->vPort->MarkToRedraw(1);
            if (bOrient) {
                if (!bSkullCannon) {
                    hTempObj->SetParam(WWD::Param_LocationX, hTempObj->GetParam(WWD::Param_LocationX) - 49);
                    GetUserDataFromObj(hTempObj)->SyncToObj();
                }
                bOrient = 0;
                iRealSpeedX *= -1;
            }
        } else if (actionEvent.getSource() == rbType[1]) {
            if (bSkullCannon) {
                if (hTempObj->GetDrawFlags() & WWD::Flag_dr_Mirror) {
                    hTempObj->SetDrawFlags((WWD::OBJ_DRAW_FLAGS) (hTempObj->GetDrawFlags() - WWD::Flag_dr_Mirror));
                    GetUserDataFromObj(hTempObj)->SyncToObj();
                }
            } else {
                hTempObj->SetLogic("TowerCannonRight");
                hTempObj->SetImageSet("LEVEL_TOWERCANNONRIGHT");
            }
            hState->vPort->MarkToRedraw(1);
            if (!bOrient) {
                if (!bSkullCannon) {
                    hTempObj->SetParam(WWD::Param_LocationX, hTempObj->GetParam(WWD::Param_LocationX) + 49);
                    GetUserDataFromObj(hTempObj)->SyncToObj();
                }
                bOrient = 1;
                iRealSpeedX *= -1;
            }
        } else if (actionEvent.getSource() == tfAngle) {
            int v = atoi(tfAngle->getText().c_str());
            tfAngle->setMarkedInvalid(v > 360);
            if (v <= 360) {
                hgeVector vec(iSpeedX, 0);
                vec.Rotate(float(v) / 57.2957795f);
                iSpeedX = vec.x;
                iSpeedY = vec.y;
                iRealSpeedX = iSpeedX;
                if (!bOrient) iRealSpeedX *= -1;
                char tmp[50];
                sprintf(tmp, "%d", iSpeedX);
                tfSpeedX->setText(tmp);
                sprintf(tmp, "%d", iSpeedY);
                tfSpeedY->setText(tmp);
                iAngle = v;
            }
        } else if (actionEvent.getSource() == tfSpeedX) {
            iSpeedX = atoi(tfSpeedX->getText().c_str());
            iRealSpeedX = iSpeedX;
            if (!bOrient) iRealSpeedX *= -1;
            iAngle = CalcAngle(iRealSpeedX, iSpeedY, bOrient);
            UpdateAngleInput();
            iLinearSpeed = sqrt(pow(iSpeedX, 2) + pow(iSpeedY, 2));
            UpdateLinearSpeed();
        } else if (actionEvent.getSource() == tfSpeedY) {
            iSpeedY = atoi(tfSpeedY->getText().c_str());
            iAngle = CalcAngle(iRealSpeedX, iSpeedY, bOrient);
            UpdateAngleInput();
            iLinearSpeed = sqrt(pow(iSpeedX, 2) + pow(iSpeedY, 2));
            UpdateLinearSpeed();
        } else if (actionEvent.getSource() == tfLinSpeed) {
            iLinearSpeed = atoi(tfLinSpeed->getText().c_str());
            hgeVector vec(iLinearSpeed, 0);
            vec.Rotate(float(iAngle) / 57.2957795f);
            printf("%f, %f, %d\n", vec.x, vec.y, iAngle);
            iSpeedX = vec.x;
            iRealSpeedX = iSpeedX;
            if (!bOrient) iRealSpeedX *= -1;
            iSpeedY = vec.y;
            char tmp[50];
            sprintf(tmp, "%d", iSpeedX);
            tfSpeedX->setText(tmp);
            sprintf(tmp, "%d", iSpeedY);
            tfSpeedY->setText(tmp);
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
        mn->RenderArrow(objx, objy, objx + vecDest.x, objy + vecDest.y, 1, !bDraggingArrow);
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        if (mn->conMain->getWidgetAt(mx, my) != mn->vPort->GetWidget()) return;
        if (mx > objx + vecDest.x - 15 && mx < objx + vecDest.x + 15 &&
            my > objy + vecDest.y - 15 && my < objy + vecDest.y + 15) {
            mn->bShowHand = 1;
        }
    }
}
