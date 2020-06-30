#include "editProjectile.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"

extern HGE *hge;

namespace ObjEdit {
    cEditObjProjectile::cEditObjProjectile(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enProjectile;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Projectile", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 515, 175));
        win->setClose(1);
        win->setShadow(0);
        win->addActionListener(hAL);
        win->add(vpAdv);
        win->setMovable(0);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        win->add(_butAddNext, win->getWidth() - 210, win->getHeight() - 60);
        win->add(_butSave, win->getWidth() - 110, win->getHeight() - 60);

        char tmp[50];

        int xoffset = 285;

        labSpeed = new SHR::Lab(GETL2S("EditObj_Projectile", "Speed"));
        labSpeed->adjustSize();
        win->add(labSpeed, xoffset + 5, 15);

        int dir = obj->GetParam(WWD::Param_Direction);
        int speed = hTempObj->GetParam((dir == 8 || dir == 2) ? WWD::Param_SpeedY : WWD::Param_SpeedX);
        if (speed < 0) {
            speed *= -1;
            hTempObj->SetParam((dir == 8 || dir == 2) ? WWD::Param_SpeedY : WWD::Param_SpeedX, speed);
            if (dir == 8) SetDirection(2);
            else if (dir == 2) SetDirection(8);
            else if (dir == 6) SetDirection(4);
            else if (dir == 4) SetDirection(6);
        }
        sprintf(tmp, "%d", speed == 0 ? 100 : speed);
        tfSpeed = new SHR::TextField(tmp);
        tfSpeed->setDimension(gcn::Rectangle(0, 0, 75, 20));
        tfSpeed->SetNumerical(1, 0);
        tfSpeed->addActionListener(hAL);
        win->add(tfSpeed, xoffset + 145, 15);

        labDelay = new SHR::Lab(GETL2S("EditObj_Projectile", "Delay"));
        labDelay->adjustSize();
        win->add(labDelay, xoffset + 5, 40);

        sprintf(tmp, "%d", obj->GetParam(WWD::Param_Speed) == 0 ? 500 : obj->GetParam(WWD::Param_Speed));
        tfDelay = new SHR::TextField(tmp);
        tfDelay->setDimension(gcn::Rectangle(0, 0, 75, 20));
        tfDelay->SetNumerical(1, 0);
        tfDelay->addActionListener(hAL);
        win->add(tfDelay, xoffset + 145, 40);

        labDamage = new SHR::Lab(GETL2S("EditObj_Projectile", "Damage"));
        labDamage->adjustSize();
        win->add(labDamage, xoffset + 5, 65);

        sprintf(tmp, "%d", obj->GetParam(WWD::Param_Damage) == 0 ? 5 : obj->GetParam(WWD::Param_Damage));
        tfDamage = new SHR::TextField(tmp);
        tfDamage->setDimension(gcn::Rectangle(0, 0, 75, 20));
        tfDamage->SetNumerical(1, 0);
        tfDamage->addActionListener(hAL);
        win->add(tfDamage, xoffset + 145, 65);

        xoffset = 143;

        labDir = new SHR::Lab(GETL2S("EditObj_Projectile", "Direction"));
        labDir->adjustSize();
        win->add(labDir, xoffset, 15);
        xoffset += 20;
        butDir[0] = hState->MakeButton(xoffset + 32, 40, Icon_Up, win);
        butDir[1] = hState->MakeButton(xoffset + 64, 40 + 32, Icon_Right, win);
        butDir[2] = hState->MakeButton(xoffset + 32, 40 + 64, Icon_Down, win);
        butDir[3] = hState->MakeButton(xoffset, 40 + 32, Icon_Left, win);
        for (int i = 0; i < 4; i++)
            butDir[i]->addActionListener(hAL);

        if (dir != 8 && dir != 2 && dir != 6 && dir != 4) {
            dir = GV->editState->hParser->GetBaseLevel() == 14 ? 6 : 8;
            hTempObj->SetParam(WWD::Param_Direction, dir);
        }
        if (GV->editState->hParser->GetBaseLevel() == 14) {
            butDir[0]->setEnabled(0);
            butDir[2]->setEnabled(0);
        }
        butDir[0]->setHighlight(dir == 8);
        butDir[1]->setHighlight(dir == 6);
        butDir[2]->setHighlight(dir == 2);
        butDir[3]->setHighlight(dir == 4);

        bPick = 0;

        xoffset = 0;

        labActivRect = new SHR::Lab(GETL2S("EditObj_Projectile", "ActivationRect"));
        labActivRect->adjustSize();
        win->add(labActivRect, xoffset + 5, 15);
        labX1 = new SHR::Lab("X1:");
        labX1->adjustSize();
        win->add(labX1, xoffset + 5, 15 + 22);

        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_MinX));
        tfX1 = new SHR::TextField(tmp);
        tfX1->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tfX1->addActionListener(hAL);
        tfX1->SetNumerical(1, 0);
        win->add(tfX1, xoffset + 30, 15 + 22);

        labY1 = new SHR::Lab("Y1:");
        labY1->adjustSize();
        win->add(labY1, xoffset + 5, 15 + 22 * 2);

        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_MinY));
        tfY1 = new SHR::TextField(tmp);
        tfY1->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tfY1->addActionListener(hAL);
        tfY1->SetNumerical(1, 0);
        win->add(tfY1, xoffset + 30, 15 + 22 * 2);

        labX2 = new SHR::Lab("X2:");
        labX2->adjustSize();
        win->add(labX2, xoffset + 5, 15 + 22 * 3);

        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_MaxX));
        tfX2 = new SHR::TextField(tmp);
        tfX2->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tfX2->addActionListener(hAL);
        tfX2->SetNumerical(1, 0);
        win->add(tfX2, xoffset + 30, 15 + 22 * 3);

        labY2 = new SHR::Lab("Y2:");
        labY2->adjustSize();
        win->add(labY2, xoffset + 5, 15 + 22 * 4);

        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_MaxY));
        tfY2 = new SHR::TextField(tmp);
        tfY2->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tfY2->addActionListener(hAL);
        tfY2->SetNumerical(1, 0);
        win->add(tfY2, xoffset + 30, 15 + 22 * 4);

        butPick = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Projectile", "Pick"));
        butPick->setDimension(gcn::Rectangle(0, 0, 125, 33));
        butPick->addActionListener(hAL);
        win->add(butPick, xoffset + 5, 15 + 22 * 5);

        UpdateRectMarks();
    }

    cEditObjProjectile::~cEditObjProjectile() {
        delete labX1;
        delete labY1;
        delete labX2;
        delete labY2;
        delete butPick;
        delete labActivRect;
        for (int i = 0; i < 4; i++)
            delete butDir[i];
        delete labDir;
        delete tfSpeed;
        delete tfDelay;
        delete tfDamage;
        delete labSpeed;
        delete labDelay;
        delete labDamage;
        delete win;
        hState->vPort->MarkToRedraw();
    }

    void cEditObjProjectile::SetDirection(int dir) {
        hTempObj->SetParam(WWD::Param_Direction, dir);
        if (dir == 8)
            hTempObj->SetUserValue(0, 4);
        else if (dir == 2)
            hTempObj->SetUserValue(0, 3);
        else if (dir == 4)
            hTempObj->SetUserValue(0, 2);
        else if (dir == 6)
            hTempObj->SetUserValue(0, 1);
    }

    void cEditObjProjectile::EnableSave() {
        _butSave->setEnabled(!tfX1->isMarkedInvalid() &&
                             !tfX2->isMarkedInvalid() &&
                             !tfY1->isMarkedInvalid() &&
                             !tfY2->isMarkedInvalid() &&
                             !tfDamage->isMarkedInvalid() &&
                             !tfSpeed->isMarkedInvalid() &&
                             !tfDelay->isMarkedInvalid() &&
                             !bPick);
    }

    void cEditObjProjectile::UpdateRectMarks(bool updv) {
        if (updv) {
            char tmp[25];
            sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_MinX));
            tfX1->setText(tmp);
            sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_MinY));
            tfY1->setText(tmp);
            sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_MaxX));
            tfX2->setText(tmp);
            sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_MaxY));
            tfY2->setText(tmp);
        }
        int x1 = atoi(tfX1->getText().c_str()),
                y1 = atoi(tfY1->getText().c_str()),
                x2 = atoi(tfX2->getText().c_str()),
                y2 = atoi(tfY2->getText().c_str());
        tfX1->setMarkedInvalid(x1 >= x2 || x1 == 0);
        tfY1->setMarkedInvalid(y1 >= y2 || y1 == 0);
        tfX2->setMarkedInvalid(x1 >= x2 || x2 == 0);
        tfY2->setMarkedInvalid(y1 >= y2 || y2 == 0);

        EnableSave();
    }

    void cEditObjProjectile::Save() {
        int dir = hTempObj->GetParam(WWD::Param_Direction);
        hTempObj->SetParam(WWD::Param_SpeedX, (dir == 4 || dir == 6) ? atoi(tfSpeed->getText().c_str()) : 0);
        hTempObj->SetParam(WWD::Param_SpeedY, (dir == 2 || dir == 8) ? atoi(tfSpeed->getText().c_str()) : 0);
        hTempObj->SetParam(WWD::Param_Speed, atoi(tfDelay->getText().c_str()));
        hTempObj->SetParam(WWD::Param_Damage, atoi(tfDamage->getText().c_str()));
    }

    void cEditObjProjectile::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == tfX1) {
            UpdateRectMarks();
            int v = atoi(tfX1->getText().c_str());
            if (v != 0)
                hTempObj->SetParam(WWD::Param_MinX, v);
        } else if (actionEvent.getSource() == tfY1) {
            UpdateRectMarks();
            int v = atoi(tfY1->getText().c_str());
            if (v != 0)
                hTempObj->SetParam(WWD::Param_MinY, v);
        } else if (actionEvent.getSource() == tfX2) {
            UpdateRectMarks();
            int v = atoi(tfX2->getText().c_str());
            if (v != 0)
                hTempObj->SetParam(WWD::Param_MaxX, v);
        } else if (actionEvent.getSource() == tfY2) {
            UpdateRectMarks();
            int v = atoi(tfY2->getText().c_str());
            if (v != 0)
                hTempObj->SetParam(WWD::Param_MaxY, v);
        } else if (actionEvent.getSource() == tfSpeed) {
            tfSpeed->setMarkedInvalid(atoi(tfSpeed->getText().c_str()) == 0);
            EnableSave();
        } else if (actionEvent.getSource() == tfDelay) {
            tfDelay->setMarkedInvalid(atoi(tfDelay->getText().c_str()) == 0);
            EnableSave();
        } else if (actionEvent.getSource() == tfDamage) {
            tfDamage->setMarkedInvalid(atoi(tfDamage->getText().c_str()) == 0);
            EnableSave();
        } else if (actionEvent.getSource() == butPick) {
            bPick = !bPick;
            bAllowDragging = !bPick;
            bDrag = 0;
            butPick->setCaption(GETL2S("EditObj_Projectile", bPick ? "Cancel" : "Pick"));
            tfX1->setEnabled(!bPick);
            tfY1->setEnabled(!bPick);
            tfX2->setEnabled(!bPick);
            tfY2->setEnabled(!bPick);
            tfSpeed->setEnabled(!bPick);
            tfDelay->setEnabled(!bPick);
            tfDamage->setEnabled(!bPick);
            EnableSave();
            butDir[1]->setEnabled(!bPick);
            butDir[3]->setEnabled(!bPick);
            if (GV->editState->hParser->GetBaseLevel() != 14) {
                butDir[0]->setEnabled(!bPick);
                butDir[2]->setEnabled(!bPick);
            }
        }
        for (int i = 0; i < 4; i++) {
            if (actionEvent.getSource() == butDir[i]) {
                char dirstr[24];
                dirstr[0] = '\0';
                if (i == 0) {
                    hTempObj->SetParam(WWD::Param_Direction, 8);
                    hTempObj->SetUserValue(0, 4);
                    strcpy(dirstr, "UP");
                } else if (i == 1) {
                    hTempObj->SetParam(WWD::Param_Direction, 6);
                    hTempObj->SetUserValue(0, 1);
                    if (GV->editState->hParser->GetBaseLevel() == 14) {
                        hTempObj->SetImageSet("LEVEL_SHOOTERS_LAUNCHLEFT");
                        hTempObj->SetAnim(hTempObj->GetImageSet());
                        GV->editState->vPort->MarkToRedraw();
                    } else {
                        strcpy(dirstr, "RIGHT");
                    }
                } else if (i == 2) {
                    hTempObj->SetParam(WWD::Param_Direction, 2);
                    hTempObj->SetUserValue(0, 3);
                    strcpy(dirstr, "DOWN");
                } else if (i == 3) {
                    hTempObj->SetParam(WWD::Param_Direction, 4);
                    hTempObj->SetUserValue(0, 2);
                    if (GV->editState->hParser->GetBaseLevel() == 14) {
                        hTempObj->SetImageSet("LEVEL_SHOOTERS_LAUNCHRIGHT");
                        hTempObj->SetAnim(hTempObj->GetImageSet());
                        GV->editState->vPort->MarkToRedraw();
                    } else {
                        strcpy(dirstr, "LEFT");
                    }
                }
                if (dirstr[0] != '\0') {
                    char logic[255];
                    sprintf(logic, "LEVEL_SHOOTERS_PUFFDART%s", dirstr);
                    hTempObj->SetImageSet(logic);
                    hTempObj->SetAnim(hTempObj->GetImageSet());
                    GV->editState->vPort->MarkToRedraw();
                }
                for (int y = 0; y < 4; y++)
                    butDir[y]->setHighlight(y == i);
            }
        }
    }

    void cEditObjProjectile::_Think(bool bMouseConsumed) {
        if (!bPick) return;

        if (bDrag && !hge->Input_GetKeyState(HGEK_LBUTTON)) {
            bDrag = 0;
            butPick->simulatePress();
            return;
        }

        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        if (bMouseConsumed || hState->conMain->getWidgetAt(mx, my) != hState->vPort->GetWidget())
            return;

        if (!bDrag && hge->Input_KeyDown(HGEK_LBUTTON)) {
            bDrag = 1;
            iDragX = GV->editState->Scr2WrdX(GV->editState->GetActivePlane(), mx);
            iDragY = GV->editState->Scr2WrdY(GV->editState->GetActivePlane(), my);
            hTempObj->SetParam(WWD::Param_MinX, iDragX);
            hTempObj->SetParam(WWD::Param_MinY, iDragY);
            hTempObj->SetParam(WWD::Param_MaxX, iDragX);
            hTempObj->SetParam(WWD::Param_MaxY, iDragY);
            iDragmX = iDragX;
            iDragmY = iDragY;
            return;
        }

        if (!bDrag)
            return;

        int ax = GV->editState->Scr2WrdX(GV->editState->GetActivePlane(), mx),
                ay = GV->editState->Scr2WrdY(GV->editState->GetActivePlane(), my);

        if (ax < iDragX) {
            hTempObj->SetParam(WWD::Param_MinX, ax);
            hTempObj->SetParam(WWD::Param_MaxX, iDragX);
        } else {
            hTempObj->SetParam(WWD::Param_MaxX, ax);
        }
        if (ay < iDragY) {
            hTempObj->SetParam(WWD::Param_MinY, ay);
            hTempObj->SetParam(WWD::Param_MaxY, iDragY);
        } else {
            hTempObj->SetParam(WWD::Param_MaxY, ay);
        }

        if (iDragmX != mx || iDragmY != my) {
            iDragmX = mx;
            iDragmY = my;
            UpdateRectMarks(1);
        }
    }

    void cEditObjProjectile::Draw() {
        int dx, dy;
        win->getAbsolutePosition(dx, dy);

        hge->Gfx_RenderLine(dx + 140, dy + 24, dx + 140, dy + win->getHeight(), GV->colLineDark);
        hge->Gfx_RenderLine(dx + 141, dy + 24, dx + 141, dy + win->getHeight(), GV->colLineBright);

        hge->Gfx_RenderLine(dx + 287, dy + 24, dx + 287, dy + win->getHeight(), GV->colLineDark);
        hge->Gfx_RenderLine(dx + 288, dy + 24, dx + 287, dy + win->getHeight(), GV->colLineBright);

        hge->Gfx_RenderLine(dx + 287, dy + 115, dx + win->getWidth(), dy + 115, GV->colLineDark);
        hge->Gfx_RenderLine(dx + 288, dy + 116, dx + win->getWidth(), dy + 116, GV->colLineBright);
    }
}
