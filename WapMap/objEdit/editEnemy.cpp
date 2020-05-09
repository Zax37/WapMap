#include "editEnemy.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../databanks/imageSets.h"



extern HGE *hge;

namespace ObjEdit {
    cEditObjEnemy::cEditObjEnemy(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        int base = GV->editState->hParser->GetBaseLevel();
        LogicInfo::GetEnemyLogicPairs(vstrpTypes, base);

        iType = ObjEdit::enEnemy;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Enemy", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 300, 185));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        win->setMovable(0);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        labType = new SHR::Lab(GETL2S("EditObj_Enemy", "Type"));
        labType->adjustSize();
        win->add(labType, 5, 10);

        char tmp[128];
        sprintf(tmp, "a_%p", this);
        for (int i = 0; i < vstrpTypes.size(); i++) {
            bool diff = false;
            char tmp2[128];
            if (i > 0 && vstrpTypes[i].first == vstrpTypes[i - 1].first) {
                sprintf(tmp2, "Logic_%s2", vstrpTypes[i].first.c_str());
                diff = true;
            } else {
                sprintf(tmp2, "Logic_%s", vstrpTypes[i].first.c_str());
            }

            auto typeRB = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Enemy", tmp2), tmp,
                    !strcmp(hTempObj->GetLogic(), vstrpTypes[i].first.c_str()) && (!diff ||
                            (hTempObj->GetUserValue(0) && !strcmp(hTempObj->GetLogic(), "HermitCrab"))
                            || (hTempObj->GetParam(WWD::Param_Smarts) && !strcmp(hTempObj->GetLogic(), "TigerGuard"))
                    ));
            typeRB->adjustSize();
            typeRB->addActionListener(hAL);
            rbType.push_back(typeRB);
            win->add(typeRB, 5, 32 + i * 20);
        }

        int yoffset = 42 + vstrpTypes.size() * 20;

        labBehaviour = new SHR::Lab(GETL2S("EditObj_Enemy", "Behaviour"));
        labBehaviour->adjustSize();
        win->add(labBehaviour, 5, yoffset);

        sprintf(tmp, "c_%p", this);
        for (int i = 0; i < 4; i++) {
            char tmp2[128];
            sprintf(tmp2, "Behaviour_%d", i + 1);
            rbFlags[i] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Enemy", tmp2), tmp, 0);
            rbFlags[i]->adjustSize();
            win->add(rbFlags[i], 5, yoffset + 20 + i * 20);
        }
        yoffset += 100;

        cbPatrol = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Enemy", "Patrol"));
        cbPatrol->adjustSize();
        cbPatrol->addActionListener(hAL);
        cbPatrol->setSelected(hTempObj->GetUserValue(0) == 0);
        win->add(cbPatrol, 5, yoffset);
        yoffset += 30;

        if (hTempObj->GetDynamicFlags() & WWD::Flag_dy_NoHit) {
            rbFlags[2]->setSelected(1);
        } else if (hTempObj->GetDynamicFlags() & WWD::Flag_dy_Safe) {
            rbFlags[1]->setSelected(1);
        } else if (hTempObj->GetDynamicFlags() & WWD::Flag_dy_AutoHitDamage) {
            rbFlags[3]->setSelected(1);
        } else {
            rbFlags[0]->setSelected(1);
        }

        labTreasures = new SHR::Lab(GETL2S("EditObj_Enemy", "Treasures"));
        labTreasures->adjustSize();
        win->add(labTreasures, 140, yoffset);
        for (int y = 0; y < 3; y++)
            for (int x = 0; x < 3; x++) {
                invTabs[y * 3 + x] = new SHR::InvTab(&GV->gcnParts);
                invTabs[y * 3 + x]->adjustSize();
                invTabs[y * 3 + x]->addActionListener(hAL);
                win->add(invTabs[y * 3 + x], 140 + x * 50, yoffset + y * 50);
            }
        int taboff = 0;
        if (hTempObj->GetParam(WWD::Param_Powerup) > 0) {
            cInventoryItem it = GV->editState->hInvCtrl->GetItemByID(hTempObj->GetParam(WWD::Param_Powerup));
            if (it.second != -1) {
                invTabs[taboff]->SetItem(it);
                taboff++;
            }
        }
        for (int i = 0; i < 2; i++) {
            WWD::Rect r = hTempObj->GetUserRect(i);
            cInventoryItem it = GV->editState->hInvCtrl->GetItemByID(r.x1);
            if (it.second != -1) {
                invTabs[taboff]->SetItem(it);
                taboff++;
            }
            it = GV->editState->hInvCtrl->GetItemByID(r.y1);
            if (it.second != -1) {
                invTabs[taboff]->SetItem(it);
                taboff++;
            }
            it = GV->editState->hInvCtrl->GetItemByID(r.x2);
            if (it.second != -1) {
                invTabs[taboff]->SetItem(it);
                taboff++;
            }
            it = GV->editState->hInvCtrl->GetItemByID(r.y2);
            if (it.second != -1) {
                invTabs[taboff]->SetItem(it);
                taboff++;
            }
        }

        labMoveArea = new SHR::Lab(GETL2S("EditObj_Enemy", "MoveArea"));
        labMoveArea->adjustSize();
        win->add(labMoveArea, 5, yoffset);

        hRectPick = new cProcPickRect(hTempObj);
        hRectPick->setActionListener(hAL);
        hRectPick->setType(PickRect_MinMax);
        hRectPick->setAllowEmpty(1);
        hRectPick->setAllowEmptyAxis(1);
        hRectPick->getPickButton()->addActionListener(hAL);
        win->add(hRectPick, 5, yoffset + 20);

        butClearArea = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Enemy", "Clear"));
        butClearArea->setDimension(gcn::Rectangle(0, 0, 125, 33));
        butClearArea->addActionListener(hAL);
        win->add(butClearArea, 5, yoffset + 20 + 88 + 36);

        labDamage = new SHR::Lab(GETL2S("EditObj_Enemy", "Damage"));
        win->add(labDamage, 5, yoffset + 20 + 88 + 70 + 9);

        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_Damage));
        tfDamage = new SHR::TextField(tmp);
        tfDamage->setDimension(gcn::Rectangle(0, 0, 150, 20));
        tfDamage->SetNumerical(1, 0);
        win->add(tfDamage, 140, yoffset + 20 + 88 + 70 + 6);

        labWarpDest = new SHR::Lab("");
        labWarpDest->adjustSize();
        win->add(labWarpDest, 5, yoffset + 20 + 88 + 70 + 6);

        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedX));
        tfSpeedX = new SHR::TextField(tmp);
        tfSpeedX->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tfSpeedX->SetNumerical(1, 0);
        win->add(tfSpeedX, 25, yoffset + 20 + 88 + 70 + 4 + 20);

        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedY));
        tfSpeedY = new SHR::TextField(tmp);
        tfSpeedY->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tfSpeedY->SetNumerical(1, 0);
        win->add(tfSpeedY, 25, yoffset + 20 + 88 + 70 + 4 + 42);

        butPickSpeedXY = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Enemy", "Pick"));
        butPickSpeedXY->setDimension(gcn::Rectangle(0, 0, 125, 33));
        butPickSpeedXY->addActionListener(hAL);
        win->add(butPickSpeedXY, 150, yoffset + 20 + 88 + 70 + 4 + 24);

        win->add(_butAddNext, 50, 15);
        win->add(_butSave, 150, 15);

        hInventory = new cInvPickbox();
        hInventory->SetPosition(hState->vPort->GetX() + hState->vPort->GetWidth() - hInventory->GetWidth(),
                                hState->vPort->GetY() + hState->vPort->GetHeight() - hInventory->GetHeight());

        bPickSpeedXY = 0;

        RebuildWindow();
    }

    cEditObjEnemy::~cEditObjEnemy() {
        delete butPickSpeedXY;
        delete labDamage;
        delete tfDamage;
        delete labWarpDest;
        delete tfSpeedX;
        delete tfSpeedY;
        delete butClearArea;
        delete labMoveArea;
        delete hRectPick;
        delete hInventory;
        delete cbPatrol;
        delete labBehaviour;
        for (int i = 0; i < 4; i++)
            delete rbFlags[i];
        for (auto& t : rbType)
            delete t;
        delete labType;
        delete win;
    }

    void cEditObjEnemy::ApplyInventoryToObject() {
        if (!invTabs[0]->isVisible() || invTabs[0]->GetItem().second == -1)
            hTempObj->SetParam(WWD::Param_Powerup, 0);
        else
            hTempObj->SetParam(WWD::Param_Powerup, invTabs[0]->GetItem().second);
        for (int i = 0; i < 2; i++) {
            WWD::Rect r;
            if (!invTabs[i * 4 + 1]->isVisible() || invTabs[i * 4 + 1]->GetItem().second == -1) r.x1 = 0;
            else r.x1 = invTabs[i * 4 + 1]->GetItem().second;
            if (!invTabs[i * 4 + 2]->isVisible() || invTabs[i * 4 + 2]->GetItem().second == -1) r.y1 = 0;
            else r.y1 = invTabs[i * 4 + 2]->GetItem().second;
            if (!invTabs[i * 4 + 3]->isVisible() || invTabs[i * 4 + 3]->GetItem().second == -1) r.x2 = 0;
            else r.x2 = invTabs[i * 4 + 3]->GetItem().second;
            if (!invTabs[i * 4 + 4]->isVisible() || invTabs[i * 4 + 4]->GetItem().second == -1) r.y2 = 0;
            else r.y2 = invTabs[i * 4 + 4]->GetItem().second;
            hTempObj->SetUserRect(i, r);
        }
    }

    void cEditObjEnemy::RebuildWindow() {
        bool bHaveTreasures = 1;
        if (!strcmp(hTempObj->GetLogic(), "Fish") || !strcmp(hTempObj->GetLogic(), "Gabriel") ||
            !strcmp(hTempObj->GetLogic(), "Aquatis") || !strcmp(hTempObj->GetLogic(), "RedTail") ||
            !strcmp(hTempObj->GetLogic(), "Omar") || !strcmp(hTempObj->GetLogic(), "Rat"))
            bHaveTreasures = 0;

        hInventory->SetVisible(bHaveTreasures);

        int addheight = 170;
        if (bHaveTreasures) {
            if (!strcmp(hTempObj->GetLogic(), "HermitCrab")) {
                labTreasures->setVisible(1);
                invTabs[0]->setVisible(1);
                invTabs[0]->setPosition(190, 30 + vstrpTypes.size() * 20 + 40 + 135 + 40);
                for (int i = 1; i < 9; i++) {
                    invTabs[i]->setVisible(0);
                }
            } else {
                labTreasures->setVisible(1);
                for (int i = 0; i < 9; i++) {
                    invTabs[i]->setVisible(1);
                    invTabs[i]->setPosition(140 + (i % 3) * 50,
                                            30 + vstrpTypes.size() * 20 + 30 + 135 + 50 * int(i / 3));
                }
            }
        } else {
            labTreasures->setVisible(0);
            for (int i = 0; i < 9; i++)
                invTabs[i]->setVisible(0);
        }
        ApplyInventoryToObject();
        for (int i = 0; i < 9; i++)
            invTabs[i]->setEnabled(!hRectPick->IsPicking() && !bPickSpeedXY);

        bool dmgvis = !strcmp(hTempObj->GetLogic(), "Fish") || !strcmp(hTempObj->GetLogic(), "HermitCrab");
        if (dmgvis && !labDamage->isVisible()) {
            tfDamage->setText(!strcmp(hTempObj->GetLogic(), "Fish") ? "10" : "5");
        }
        labDamage->setVisible(dmgvis);
        tfDamage->setVisible(dmgvis);

        tfDamage->setEnabled(!hRectPick->IsPicking() && !bPickSpeedXY);
        labDamage->setColor(tfDamage->isEnabled() ? 0xFFa1a1a1 : 0xFF222222);

        bool warpvis = 0;
        for (int i = 0; i < 9; i++)
            if (invTabs[i]->GetItem().second == 32 && invTabs[i]->isVisible()) {
                warpvis = 1;
                break;
            }
        if (warpvis && !strcmp(hTempObj->GetLogic(), "HermitCrab"))
            warpvis = 0;
        bool gemvis = (!strcmp(hTempObj->GetLogic(), "Raux") ||
                       !strcmp(hTempObj->GetLogic(), "Katherine") ||
                       !strcmp(hTempObj->GetLogic(), "Wolvington") ||
                       !strcmp(hTempObj->GetLogic(), "Gabriel") ||
                       !strcmp(hTempObj->GetLogic(), "Marrow") ||
                       !strcmp(hTempObj->GetLogic(), "Aquatis") ||
                       !strcmp(hTempObj->GetLogic(), "Omar") ||
                       !strcmp(hTempObj->GetLogic(), "RedTail"));
        bPickGem = gemvis;
        labWarpDest->setVisible(warpvis || gemvis);
        tfSpeedX->setVisible(warpvis || gemvis);
        tfSpeedY->setVisible(warpvis || gemvis);
        tfSpeedX->setEnabled(!hRectPick->IsPicking() && !bPickSpeedXY);
        tfSpeedY->setEnabled(!hRectPick->IsPicking() && !bPickSpeedXY);
        labWarpDest->setColor(tfSpeedX->isEnabled() ? 0xFFa1a1a1 : 0xFF222222);
        butPickSpeedXY->setVisible(warpvis || gemvis);
        butPickSpeedXY->setEnabled(!hRectPick->IsPicking());
        if (warpvis || gemvis) {
            labWarpDest->setCaption(GETL2S("EditObj_Enemy", (gemvis ? "GemDest" : "WarpDest")));
            labWarpDest->adjustSize();
        }

        win->setHeight(95 + 150 + addheight + vstrpTypes.size() * 20 + dmgvis * 28 + (warpvis || gemvis) * 68);
        win->setY(GV->editState->vPort->GetY());
        _butSave->setY(win->getHeight() - 55);
        _butAddNext->setY(win->getHeight() - 55);

        _butSave->setEnabled(!hRectPick->IsPicking() && !bPickSpeedXY &&
                             !tfSpeedX->isMarkedInvalid() && !tfSpeedY->isMarkedInvalid());

        for (auto& t : rbType)
            t->setEnabled(!hRectPick->IsPicking() && !bPickSpeedXY);
        for (int i = 0; i < 4; i++)
            rbFlags[i]->setEnabled(!hRectPick->IsPicking() && !bPickSpeedXY);

        if (hRectPick->IsPicking() || bPickSpeedXY) {
            cbPatrol->setEnabled(0);
        } else {
            if (!strcmp(hTempObj->GetLogic(), "Seagull") || !strcmp(hTempObj->GetLogic(), "Fish") || !strcmp(hTempObj->GetLogic(), "HermitCrab")) {
                cbPatrol->setSelected(1);
                cbPatrol->setEnabled(0);
            } else {
                cbPatrol->setEnabled(1);
            }
        }

        butPickSpeedXY->setCaption(GETL2S("EditObj_Enemy", bPickSpeedXY ? "Cancel" : "Pick"));

        labMoveArea->setColor(cbPatrol->isSelected() && !bPickSpeedXY ? 0xFFa1a1a1 : 0xEE000000);
        hRectPick->setEnabled(cbPatrol->isSelected() && !bPickSpeedXY);
        butClearArea->setEnabled(cbPatrol->isSelected() && !bPickSpeedXY && !hRectPick->IsPicking());
        hTempObj->SetParam(WWD::Param_MinX, cbPatrol->isSelected() ? hRectPick->getValue(0) : 0);
        hTempObj->SetParam(WWD::Param_MinY, cbPatrol->isSelected() ? hRectPick->getValue(1) : 0);
        hTempObj->SetParam(WWD::Param_MaxX, cbPatrol->isSelected() ? hRectPick->getValue(2) : 0);
        hTempObj->SetParam(WWD::Param_MaxY, cbPatrol->isSelected() ? hRectPick->getValue(3) : 0);
    }

    void cEditObjEnemy::Save() {
        int flags = 0;
        if (hTempObj->GetDynamicFlags() & WWD::Flag_dy_AlwaysActive)
            flags += (int) WWD::Flag_dy_AlwaysActive;
        if (rbFlags[1]->isSelected())
            flags += (int) WWD::Flag_dy_Safe;
        else if (rbFlags[2]->isSelected())
            flags += (int) WWD::Flag_dy_NoHit;
        else if (rbFlags[3]->isSelected())
            flags += (int) WWD::Flag_dy_AutoHitDamage;
        hTempObj->SetDynamicFlags((WWD::OBJ_DYNAMIC_FLAGS) flags);
        if (!strcmp(hTempObj->GetLogic(), "Seagull") || !strcmp(hTempObj->GetLogic(), "Fish"))
            hTempObj->SetUserValue(0, 0);
        else if (strcmp(hTempObj->GetLogic(), "HermitCrab") != 0)
            hTempObj->SetUserValue(0, !cbPatrol->isSelected());

        int iWarpIndex = -1;
        for (int i = 0; i < 9; i++)
            if (invTabs[i]->GetItem().second == 32) {
                iWarpIndex = i;
                break;
            }
        if (iWarpIndex != -1 && iWarpIndex != 0) {
            cInventoryItem tmp;
            tmp = invTabs[0]->GetItem();
            invTabs[0]->SetItem(invTabs[iWarpIndex]->GetItem());
            invTabs[iWarpIndex]->SetItem(tmp);
        }
        ApplyInventoryToObject();
        hTempObj->SetParam(WWD::Param_Damage, tfDamage->isVisible() ? atoi(tfDamage->getText().c_str()) : 0);
        if (tfSpeedX->isVisible()) {
            hTempObj->SetParam(WWD::Param_SpeedX, atoi(tfSpeedX->getText().c_str()));
            hTempObj->SetParam(WWD::Param_SpeedY, atoi(tfSpeedY->getText().c_str()));
        } else {
            hTempObj->SetParam(WWD::Param_SpeedX, 0);
            hTempObj->SetParam(WWD::Param_SpeedY, 0);
        }
    }

    void cEditObjEnemy::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == butClearArea) {
            hRectPick->setValues(0, 0, 0, 0);
            return;
        } else if (actionEvent.getSource() == cbPatrol) {
            RebuildWindow();
            return;
        } else if (actionEvent.getSource() == hRectPick->getPickButton()) {
            RebuildWindow();
            return;
        } else if (actionEvent.getSource() == butPickSpeedXY) {
            bPickSpeedXY = !bPickSpeedXY;
            RebuildWindow();
        } else if (actionEvent.getSource() == tfSpeedX) {
            int v = atoi(tfSpeedX->getText().c_str());
            bool marked = tfSpeedX->isMarkedInvalid();
            tfSpeedX->setMarkedInvalid(v == 0 || v >= GV->editState->GetActivePlane()->GetPlaneWidthPx());
            if (tfSpeedX->isMarkedInvalid() != marked)
                RebuildWindow();
            return;
        } else if (actionEvent.getSource() == tfSpeedY) {
            int v = atoi(tfSpeedY->getText().c_str());
            bool marked = tfSpeedX->isMarkedInvalid();
            tfSpeedY->setMarkedInvalid(v == 0 || v >= GV->editState->GetActivePlane()->GetPlaneWidthPx());
            if (tfSpeedY->isMarkedInvalid() != marked)
                RebuildWindow();
            return;
        }
        for (int i = 0; i < rbType.size(); ++i)
            if (actionEvent.getSource() == rbType[i]) {
                UpdateEnemyObject(hTempObj, vstrpTypes[i]);
                GV->editState->vPort->MarkToRedraw(true);
                RebuildWindow();
                return;
            }
        for (auto & invTab : invTabs)
            if (actionEvent.getSource() == invTab) {
                RebuildWindow();
            }
    }

    void cEditObjEnemy::_Think(bool bMouseConsumed) {
        if (!bMouseConsumed)
            hRectPick->Think();
        hInventory->Think();
        if (bPickSpeedXY) {
            int posx, posy;
            bool save = 0;
            float mx, my;
            hge->Input_GetMousePos(&mx, &my);
            if (bMouseConsumed || mx < GV->editState->vPort->GetX() || my < GV->editState->vPort->GetY() ||
                mx > GV->editState->vPort->GetX() + GV->editState->vPort->GetWidth() ||
                my > GV->editState->vPort->GetY() + GV->editState->vPort->GetHeight()) {
                posx = hTempObj->GetParam(WWD::Param_SpeedX);
                posy = hTempObj->GetParam(WWD::Param_SpeedY);
            } else {
                int wmx = GV->editState->Scr2WrdX(GV->editState->GetActivePlane(), mx),
                        wmy = GV->editState->Scr2WrdY(GV->editState->GetActivePlane(), my);
                posx = wmx;
                posy = wmy;
                if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                    hTempObj->SetParam(WWD::Param_SpeedX, wmx);
                    hTempObj->SetParam(WWD::Param_SpeedY, wmy);
                    save = 1;
                }
            }
            char tmp[64];
            sprintf(tmp, "%d", posx);
            tfSpeedX->setText(tmp);
            sprintf(tmp, "%d", posy);
            tfSpeedY->setText(tmp);
            if (save) {
                bPickSpeedXY = 0;
                RebuildWindow();
            }
        }
    }

    void cEditObjEnemy::RenderObjectOverlay() {
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        if (bPickSpeedXY && mx > GV->editState->vPort->GetX() && my > GV->editState->vPort->GetY() &&
            mx < GV->editState->vPort->GetX() + GV->editState->vPort->GetWidth() &&
            my < GV->editState->vPort->GetY() + GV->editState->vPort->GetHeight() &&
            GV->editState->conMain->getWidgetAt(mx, my) == GV->editState->vPort->GetWidget()) {
            int wmx = GV->editState->Scr2WrdX(GV->editState->GetActivePlane(), mx),
                wmy = GV->editState->Scr2WrdY(GV->editState->GetActivePlane(), my);
            hgeSprite *spr;
            if (bPickGem)
                spr = GV->editState->SprBank->GetAssetByID("LEVEL_GEM")->GetIMGByIterator(0)->GetSprite();
            else
                spr = GV->editState->SprBank->GetAssetByID("CLAW")->GetIMGByID(401)->GetSprite();
            spr->SetColor(0xBBFFFFFF);
            spr->RenderEx(mx, my, 0, GV->editState->fZoom);
            mx += (spr->GetWidth() / 2 + 15) * GV->editState->fZoom;
            GV->fntMyriad13->printf(mx + 1, my + 1, HGETEXT_LEFT, "~l~%s %d, %d", 0,
                                    GETL2S("EditObj_Enemy", bPickGem ? "GemDest" : "WarpDest"), wmx, wmy);
            GV->fntMyriad13->printf(mx, my, HGETEXT_LEFT, "~w~%s ~y~%d~w~, ~y~%d", 0,
                                    GETL2S("EditObj_Enemy", bPickGem ? "GemDest" : "WarpDest"), wmx, wmy);
        }
    }

    void cEditObjEnemy::Draw() {
        int dx, dy;
        win->getAbsolutePosition(dx, dy);
        dy += 24;

        hge->Gfx_RenderLine(dx, dy + 30 + vstrpTypes.size() * 20, dx + win->getWidth(),
                            dy + 30 + vstrpTypes.size() * 20, GV->colLineDark);
        hge->Gfx_RenderLine(dx, dy + 31 + vstrpTypes.size() * 20, dx + win->getWidth(),
                            dy + 31 + vstrpTypes.size() * 20, GV->colLineBright);

        hge->Gfx_RenderLine(dx, dy + 160 + vstrpTypes.size() * 20, dx + win->getWidth(),
                            dy + 160 + vstrpTypes.size() * 20, GV->colLineDark);
        hge->Gfx_RenderLine(dx, dy + 161 + vstrpTypes.size() * 20, dx + win->getWidth(),
                            dy + 161 + vstrpTypes.size() * 20, GV->colLineBright);

        hge->Gfx_RenderLine(dx, dy + 160 + 178 + 5 + vstrpTypes.size() * 20, dx + win->getWidth(),
                            dy + 160 + 178 + 5 + vstrpTypes.size() * 20, GV->colLineDark);
        hge->Gfx_RenderLine(dx, dy + 161 + 178 + 5 + vstrpTypes.size() * 20, dx + win->getWidth(),
                            dy + 161 + 178 + 5 + vstrpTypes.size() * 20, GV->colLineBright);

        if (labTreasures->isVisible()) {
            hge->Gfx_RenderLine(dx + 137, dy + 160 + vstrpTypes.size() * 20, dx + 137,
                                dy + 161 + 178 + 3 + vstrpTypes.size() * 20, GV->colLineDark);
            hge->Gfx_RenderLine(dx + 138, dy + 160 + vstrpTypes.size() * 20, dx + 138,
                                dy + 161 + 178 + 3 + vstrpTypes.size() * 20, GV->colLineBright);
        }

        if (tfDamage->isVisible()) {
            hge->Gfx_RenderLine(dx, dy + 160 + 178 + 28 + 8 + vstrpTypes.size() * 20, dx + win->getWidth(),
                                dy + 160 + 28 + 178 + 8 + vstrpTypes.size() * 20, GV->colLineDark);
            hge->Gfx_RenderLine(dx, dy + 161 + 178 + 28 + 8 + vstrpTypes.size() * 20, dx + win->getWidth(),
                                dy + 161 + 28 + 178 + 8 + vstrpTypes.size() * 20, GV->colLineBright);
        } else if (tfSpeedX->isVisible()) {
            GV->fntMyriad13->SetColor(tfSpeedX->isEnabled() ? 0xFFa1a1a1 : 0xFF222222);
            GV->fntMyriad13->Render(dx + 8, dy + 160 + 170 + 28 + 10 + vstrpTypes.size() * 20, HGETEXT_LEFT, "X:", 0);
            GV->fntMyriad13->Render(dx + 8, dy + 160 + 170 + 48 + 12 + vstrpTypes.size() * 20, HGETEXT_LEFT, "Y:", 0);
            hge->Gfx_RenderLine(dx, dy + 160 + 178 + 68 + 8 + vstrpTypes.size() * 20, dx + win->getWidth(),
                                dy + 160 + 178 + 68 + 8 + vstrpTypes.size() * 20, GV->colLineDark);
            hge->Gfx_RenderLine(dx, dy + 161 + 178 + 68 + 8 + vstrpTypes.size() * 20, dx + win->getWidth(),
                                dy + 161 + 178 + 68 + 8 + vstrpTypes.size() * 20, GV->colLineBright);
        }
    }

    void cEditObjEnemy::UpdateEnemyObject(WWD::Object *obj, const std::pair<std::string, std::string>& dataPair) {
        if (dataPair.second == "LEVEL_TIGERWHITE") {
            obj->SetParam(WWD::Param_Smarts, 1);
        } else {
            obj->SetParam(WWD::Param_Smarts, 0);

            if (dataPair.second == "LEVEL_BOMBERCRAB") {
                obj->SetUserValue(0, 1);
            } else {
                obj->SetUserValue(0, 0);
            }
        }

        obj->SetLogic(dataPair.first.c_str());
        obj->SetImageSet(dataPair.second.c_str());
    }
}
