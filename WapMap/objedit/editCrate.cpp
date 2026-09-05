#include "editCrate.h"
#include "editStatue.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "winInventoryPickbox.h"
#include "../databanks/imageSets.h"
#include "../cObjectUserData.h"

namespace ObjEdit {
    cEditObjCrate::cEditObjCrate(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        bStackable = !(hState->hParser->GetBaseLevel() == 11 || hState->hParser->GetBaseLevel() == 14);
        iType = ObjEdit::enCrate;
        bPick = false;

        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Crate", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 283, 0));
        win->setClose(true);
        win->addActionListener(hAL);
        win->add(vpAdv);
        win->setMovable(false);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        char tmp[64];
        sprintf(tmp, "%p", this);

        int yOffset = 13, x1 = 5, x2 = 150, butW = 125;

        _butAddNext->setWidth(butW);
        _butSave->setWidth(butW);
        win->add(_butAddNext, x1, 0);
        win->add(_butSave, x2, 0);

        labAlign = new SHR::Lab(GETL2S("Z_Coord", "Label"));
        labAlign->adjustSize();
        win->add(labAlign, x1, yOffset);

        labDurability = new SHR::Lab(GETL2S("EditObj_Container", "Durability"));
        labDurability->setColor(0xFFb2b2b2);
        labDurability->adjustSize();
        win->add(labDurability, x2, yOffset);

        yOffset += 22;

        lmDurability = new lmEditObjStatueDurability();

        ddDurability = new SHR::DropDown();
        ddDurability->setDimension(gcn::Rectangle(0, 0, butW, 20));
        ddDurability->setListModel(lmDurability);
        ddDurability->addActionListener(hAL);
        ddDurability->SetGfx(&GV->gcnParts);
        win->add(ddDurability, x2, yOffset);

        if (hTempObj->GetParam(WWD::Param_Health) <= 1)
            ddDurability->setSelected(0);
        else if (hTempObj->GetParam(WWD::Param_Health) < 999)
            ddDurability->setSelected(1);
        else if (hTempObj->GetParam(WWD::Param_Health) >= 999)
            ddDurability->setSelected(2);

        rbType[0] = new SHR::RadBut(GV->hGfxInterface, GETL2S("Z_Coord", "Behind"), tmp);
        rbType[0]->adjustSize();
        rbType[0]->addActionListener(hAL);
        win->add(rbType[0], x1, yOffset);

        yOffset += 22;

        rbType[1] = new SHR::RadBut(GV->hGfxInterface, GETL2S("Z_Coord", "Front"), tmp);
        rbType[1]->adjustSize();
        rbType[1]->addActionListener(hAL);
        win->add(rbType[1], x1, yOffset);

        yOffset += 22;

        rbType[2] = new SHR::RadBut(GV->hGfxInterface, GETL2S("Z_Coord", "Custom"), tmp);
        rbType[2]->adjustSize();
        rbType[2]->addActionListener(hAL);
        win->add(rbType[2], x1, yOffset);

        const char* logic = hTempObj->GetLogic();
        int z = hTempObj->GetParam(WWD::Param_LocationZ);
        if (!z) {
            z = LogicInfo::GetContainerDefaultZ(logic);
			GetUserDataFromObj(hTempObj)->SetZ(z);
            hTempObj->SetParam(WWD::Param_LocationZ, z);
			hState->vPort->MarkToRedraw();
        }

        alignment = 2;
        if (z == LogicInfo::GetContainerDefaultZ(logic)) {
            if (logic[0] == 'B') {
                alignment = 0;
            } else if (logic[0] == 'F') {
                alignment = 1;
            }
        }

        tfCustomZ = new SHR::TextField(std::to_string(z));
        tfCustomZ->addActionListener(hAL);
        tfCustomZ->setDimension(gcn::Rectangle(0, 0, 50, 20));
        tfCustomZ->setEnabled(alignment == 2);
        win->add(tfCustomZ, 12 + rbType[2]->getWidth(), yOffset - 2);

        yOffset += 30;

        cbIncludeSpecials = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Container", "IncludeSpecials"));
        cbIncludeSpecials->adjustSize();
        win->add(cbIncludeSpecials, x1, 0);

        cbRandomCount = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Container", "RandomItemCount"));
        cbRandomCount->adjustSize();
        cbRandomCount->setSelected(bStackable);
        cbRandomCount->setEnabled(bStackable);
        win->add(cbRandomCount, x1, 0);

        butRandomize = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Container", "Randomize"));
        butRandomize->setDimension(gcn::Rectangle(0, 0, butW, 33));
        butRandomize->addActionListener(hAL);
        win->add(butRandomize, x2, win->getHeight() - 120);

        labWarpDest = new SHR::Lab(GETL2S("EditObj_Container", "WarpDest"));
        labWarpDest->adjustSize();
        win->add(labWarpDest, x1, yOffset);

        yOffset += 25;

        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedX));
        tfWarpX = new SHR::TextField(tmp);
        tfWarpX->addActionListener(hAL);
        tfWarpX->setDimension(gcn::Rectangle(0, 0, 100, 20));
        win->add(tfWarpX, 20, yOffset);

        yOffset += 5;

        butWarpPick = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Warp", "Pick"));
        butWarpPick->setDimension(gcn::Rectangle(0, 0, butW, 33));
        butWarpPick->addActionListener(hAL);
        win->add(butWarpPick, x2, yOffset);

        yOffset += 20;

        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedY));
        tfWarpY = new SHR::TextField(tmp);
        tfWarpY->addActionListener(hAL);
        tfWarpY->setDimension(gcn::Rectangle(0, 0, 100, 20));
        win->add(tfWarpY, 20, yOffset);

        labTreasures = new SHR::Lab(GETL2S("EditObj_Container", "Treasures"));
        labTreasures->adjustSize();
        win->add(labTreasures, x1, 0);

        if (hTempObj->GetParam(WWD::Param_Powerup) == 0 && !strstr(logic, "Stacked")) {
            hTempObj->SetParam(WWD::Param_Powerup, 33);
        }

        if (!bStackable) {
            invTabs[0] = new SHR::InvTab(&GV->gcnParts);
            invTabs[0]->adjustSize();
            invTabs[0]->addActionListener(hAL);
            if (hTempObj->GetParam(WWD::Param_Powerup) != 0)
                invTabs[0]->SetItem(hState->hInvCtrl->GetItemByID(hTempObj->GetParam(WWD::Param_Powerup)));
            else invTabs[0]->SetItem(hState->hInvCtrl->GetItemByIt(0));
            win->add(invTabs[0], 44 + 54, 115);
            for (int i = 1; i < 9; i++)
                invTabs[i] = NULL;
            iCratesCount = 1;
        } else {
            for (int i = 0; i < 9; i++) {
                invTabs[i] = new SHR::InvTab(&GV->gcnParts);
                invTabs[i]->setDimension(gcn::Rectangle(0, 0, 32, 32));
                invTabs[i]->addActionListener(hAL);
                if (i > 0)
                    invTabs[i]->setVisible(false);
                win->add(invTabs[i], 60 + 54 + 8 + 5, 115 - 43 * i);
            }
            iCratesCount = 1;
            if (hTempObj->GetParam(WWD::Param_Powerup) != 0)
                invTabs[0]->SetItem(hState->hInvCtrl->GetItemByID(hTempObj->GetParam(WWD::Param_Powerup)));
            for (int i = 0; i < 2; i++) {
                WWD::Rect r = hTempObj->GetUserRect(i);
                if (r.x1 != 0) {
                    invTabs[iCratesCount]->setVisible(true);
                    invTabs[iCratesCount]->SetItem(hState->hInvCtrl->GetItemByID(r.x1));
                    if (invTabs[iCratesCount]->GetItem().second == -1)
                        GV->Console->Printf("~y~Warning:~w~ Found invalid item in container id#%d (%d).",
                                            hTempObj->GetParam(WWD::Param_ID), r.x1);
                    else
                        iCratesCount++;
                }
                if (r.y1 != 0) {
                    invTabs[iCratesCount]->setVisible(true);
                    invTabs[iCratesCount]->SetItem(hState->hInvCtrl->GetItemByID(r.y1));
                    if (invTabs[iCratesCount]->GetItem().second == -1)
                        GV->Console->Printf("~y~Warning:~w~ Found invalid item in container id#%d (%d).",
                                            hTempObj->GetParam(WWD::Param_ID), r.y1);
                    else
                        iCratesCount++;
                }
                if (r.x2 != 0) {
                    invTabs[iCratesCount]->setVisible(true);
                    invTabs[iCratesCount]->SetItem(hState->hInvCtrl->GetItemByID(r.x2));
                    if (invTabs[iCratesCount]->GetItem().second == -1)
                        GV->Console->Printf("~y~Warning:~w~ Found invalid item in container id#%d (%d).",
                                            hTempObj->GetParam(WWD::Param_ID), r.x2);
                    else
                        iCratesCount++;
                }
                if (r.y2 != 0) {
                    invTabs[iCratesCount]->setVisible(true);
                    invTabs[iCratesCount]->SetItem(hState->hInvCtrl->GetItemByID(r.y2));
                    if (invTabs[iCratesCount]->GetItem().second == -1)
                        GV->Console->Printf("~y~Warning:~w~ Found invalid item in container id#%d (%d).",
                                            hTempObj->GetParam(WWD::Param_ID), r.y2);
                    else
                        iCratesCount++;
                }
            }
            if (iCratesCount < 9) {
                invTabs[iCratesCount]->setVisible(true);
                iCratesCount++;
            }
        }

        rbType[alignment]->setSelected(true);
        SetLogic(bStackable && iCratesCount > 2);

        RebuildWindow();

        hInventory = new cInvPickbox();
        hInventory->SetPosition(win->getX() + win->getWidth() + 10, hState->vPort->GetY() + hState->vPort->GetHeight() - hInventory->GetHeight());
        hInventory->SetVisible(true);
    }

    cEditObjCrate::~cEditObjCrate() {
        for (auto & i : rbType)
            delete i;
        delete labDurability;
        delete ddDurability;
        delete butRandomize;
        delete lmDurability;
        delete labAlign;
        delete cbRandomCount;
        delete cbIncludeSpecials;
        delete labTreasures;
        delete butWarpPick;
        delete tfWarpX;
        delete tfWarpY;
        delete labWarpDest;
        for (auto & invTab : invTabs)
            delete invTab;
        delete win;
        delete hInventory;
        hState->vPort->MarkToRedraw();
    }

    void cEditObjCrate::Save() {
        if (!ddDurability->isEnabled())
            hTempObj->SetParam(WWD::Param_Health, 0);
    }

    void cEditObjCrate::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = true;
        } else if (actionEvent.getSource() == ddDurability) {
            if (ddDurability->getSelected() == 0)
                hTempObj->SetParam(WWD::Param_Health, 1);
            else if (ddDurability->getSelected() == 1)
                hTempObj->SetParam(WWD::Param_Health, 2);
            else if (ddDurability->getSelected() == 2)
                hTempObj->SetParam(WWD::Param_Health, 999);
        } else if (actionEvent.getSource() == tfWarpX) {
            hTempObj->SetParam(WWD::Param_SpeedX, atoi(tfWarpX->getText().c_str()));
            hState->vPort->MarkToRedraw();
        } else if (actionEvent.getSource() == tfWarpY) {
            hTempObj->SetParam(WWD::Param_SpeedY, atoi(tfWarpY->getText().c_str()));
            hState->vPort->MarkToRedraw();
        } else if (actionEvent.getSource() == butWarpPick) {
            bPick = !bPick;
            tfWarpX->setEnabled(!bPick);
            tfWarpY->setEnabled(!bPick);
            for (auto & i : rbType) i->setEnabled(!bPick);
            for (auto & invTab : invTabs) invTab->setEnabled(!bPick);
            _butSave->setEnabled(!bPick);
            butRandomize->setEnabled(!bPick);
            cbRandomCount->setEnabled(!bPick);
            cbIncludeSpecials->setEnabled(!bPick);
            SetLogic(iCratesCount > 2);
            tfCustomZ->setEnabled(!bPick && alignment == 2);
            labAlign->setColor(ddDurability->isEnabled() ? 0xFFe1e1e1 : 0xFFb2b2b2);
            labTreasures->setColor(ddDurability->isEnabled() ? 0xFFe1e1e1 : 0xFFb2b2b2);
            labDurability->setColor(ddDurability->isEnabled() ? 0xFFe1e1e1 : 0xFFb2b2b2);
            butWarpPick->setCaption(GETL2SV("EditObj_Warp", (bPick ? L"Unpick" : L"Pick")));
        } else if (actionEvent.getSource() == butRandomize) {
            int iRandNum = 1;
            if (bStackable) {
                if (cbRandomCount->isSelected()) {
                    iRandNum = hge->Random_Int(1, 9);
                } else {
                    iRandNum = 0;
                    for (auto & invTab : invTabs)
                        if (invTab->GetItem().second != -1)
                            iRandNum++;
                    if (iRandNum == 0)
                        iRandNum = 1;
                }
            }
            for (int i = 0; i < 9; i++) {
                if (invTabs[i]) {
                    invTabs[i]->SetItem(cInventoryItem("", -1));
                }
                else {
                    if (iRandNum >= i) {
                        iRandNum = hge->Random_Int(1, i);
                    }
                    break;
                }
            }
            for (int i = 0; i < iRandNum; i++) {
                int id = hge->Random_Int(0, (cbIncludeSpecials->isSelected() ? InventoryItemsCount - 2
                                                                             : 31));
                if (id == 3) id++;
                invTabs[i]->SetItem(GV->editState->hInvCtrl->GetItemByIt(id));
            }
            iCratesCount = iRandNum + (iRandNum < 9 && bStackable);
            RebuildWindow();
            ApplyInventoryToObject();
        } else if (actionEvent.getSource() == tfCustomZ) {
            int z = std::atoi(tfCustomZ->getText().c_str());
			GetUserDataFromObj(hTempObj)->SetZ(z);
            hTempObj->SetParam(WWD::Param_LocationZ, z);
            SetLogic(bStackable && iCratesCount > 2);
			hState->vPort->MarkToRedraw();
        } else {
            for (int i = 0; i < 3; i++) {
                if (actionEvent.getSource() == rbType[i]) {
                    alignment = i;
                    tfCustomZ->setEnabled(alignment == 2);
                    SetLogic(bStackable && iCratesCount > 2);
                    int z = alignment == 2 ? std::atoi(tfCustomZ->getText().c_str()) : LogicInfo::GetContainerDefaultZ(hTempObj->GetLogic());
                    GetUserDataFromObj(hTempObj)->SetZ(z);
                    hTempObj->SetParam(WWD::Param_LocationZ, z);
			        hState->vPort->MarkToRedraw();
                    return;
                }
            }

            if (bStackable) {
                for (int i = 0; i < 9; i++) {
                    if (actionEvent.getSource() == invTabs[i]) {
                        if (invTabs[i]->GetItem().second != -1 && i < 8 && !invTabs[i + 1]->isVisible()) {
                            invTabs[i + 1]->setVisible(true);
                            iCratesCount++;
                            RebuildWindow();
                        } else if (invTabs[i]->GetItem().second == -1 && i < 8) {
                            if (i + 2 == iCratesCount) {
                                for (int x = 8; x > 1; x--) {
                                    if (invTabs[x]->isVisible()) {
                                        if (invTabs[x]->GetItem().second == -1 &&
                                            invTabs[x - 1]->GetItem().second == -1) {
                                            invTabs[x]->setVisible(false);
                                            iCratesCount--;
                                        } else
                                            break;
                                    }
                                }
                            }
                            for (int x = 1; x < iCratesCount - 1; x++) {
                                if (invTabs[x]->GetItem().second == -1) {
                                    for (int y = x; y < iCratesCount - 1; y++)
                                        invTabs[y]->SetItem(invTabs[y + 1]->GetItem());
                                    invTabs[iCratesCount - 1]->setVisible(false);
                                    iCratesCount--;
                                }
                            }
                        }
                        RebuildWindow();
                        hState->vPort->MarkToRedraw();
                        ApplyInventoryToObject();
                        if (alignment != 2) {
                            int z = LogicInfo::GetContainerDefaultZ(hTempObj->GetLogic());
                            GetUserDataFromObj(hTempObj)->SetZ(z);
                            hTempObj->SetParam(WWD::Param_LocationZ, z);
			                hState->vPort->MarkToRedraw();
                        }
                        return;
                    }
                }
            } else {
                if (actionEvent.getSource() == invTabs[0]) {
                    if (invTabs[0]->GetItem().second == -1) {
                        invTabs[0]->SetItem(GV->editState->hInvCtrl->GetItemByIt(0));
                    }
                    RebuildWindow();
                    hState->vPort->MarkToRedraw();
                    ApplyInventoryToObject();
                }
            }
        }
    }

    void cEditObjCrate::Draw() {
        hInventory->Think();
        #define FIRST_SECTION_HEIGHT 120
        #define WARP_SECTION_HEIGHT 80
        int dx, dy;
        win->getAbsolutePosition(dx, dy);
        hge->Gfx_RenderLine(dx, dy + FIRST_SECTION_HEIGHT, dx + win->getWidth(), dy + FIRST_SECTION_HEIGHT, GV->colLineDark);
        hge->Gfx_RenderLine(dx, dy + FIRST_SECTION_HEIGHT + 1, dx + win->getWidth(), dy + FIRST_SECTION_HEIGHT + 1, GV->colLineBright);

        hge->Gfx_RenderLine(dx, dy + win->getHeight() - 131, dx + win->getWidth(), dy + win->getHeight() - 131,
                            GV->colLineDark);
        hge->Gfx_RenderLine(dx, dy + win->getHeight() - 130, dx + win->getWidth(), dy + win->getHeight() - 130,
                            GV->colLineBright);

        hge->Gfx_RenderLine(dx, dy + win->getHeight() - 45, dx + win->getWidth(), dy + win->getHeight() - 45,
                            GV->colLineDark);
        hge->Gfx_RenderLine(dx, dy + win->getHeight() - 44, dx + win->getWidth(), dy + win->getHeight() - 44,
                            GV->colLineBright);

        if (bPick) {
            float mx, my;
            hge->Input_GetMousePos(&mx, &my);
            if (hge->Input_KeyDown(HGEK_LBUTTON) &&
                hState->conMain->getWidgetAt(mx, my) == hState->vPort->GetWidget()) {
                char tmp[64];
                sprintf(tmp, "%d", hState->Scr2WrdX(hState->GetActivePlane(), mx));
                tfWarpX->setText(tmp, true);
                sprintf(tmp, "%d", hState->Scr2WrdY(hState->GetActivePlane(), my));
                tfWarpY->setText(tmp, true);
                butWarpPick->simulatePress();
            }
        }

        if (bShowWarpOptions) {
            hge->Gfx_RenderLine(dx, dy + FIRST_SECTION_HEIGHT + WARP_SECTION_HEIGHT, dx + win->getWidth(), dy + FIRST_SECTION_HEIGHT + WARP_SECTION_HEIGHT, GV->colLineDark);
            hge->Gfx_RenderLine(dx, dy + FIRST_SECTION_HEIGHT + WARP_SECTION_HEIGHT + 1, dx + win->getWidth(), dy + FIRST_SECTION_HEIGHT + WARP_SECTION_HEIGHT + 1, GV->colLineBright);
            GV->fntMyriad16->SetColor(0xFFe1e1e1);
            GV->fntMyriad16->Render(dx + 7, dy + tfWarpX->getY() + tfWarpX->getHeight() - 2, HGETEXT_LEFT, "X:", 0);
            GV->fntMyriad16->Render(dx + 7, dy + tfWarpY->getY() + tfWarpY->getHeight() - 2, HGETEXT_LEFT, "Y:", 0);
        }

        if (bStackable) {
            for (int i = 8; i >= 0; i--)
                if (invTabs[i]->isVisible()) {
                    hState->SprBank->GetObjectSprite(hTempObj)->Render(dx + 132,
                                                                       dy + 180 + i * 43 + WARP_SECTION_HEIGHT * bShowWarpOptions);
                }
        }
    }

    void cEditObjCrate::SetLogic(bool bStacked) {
        if (bStackable && !bStacked) {
            if (invTabs[0]->GetItem().second == -1 || invTabs[0]->GetItem().second == 32) {
                bStacked = true;
            }
        }

        const char* logic;
        bool isBack = !alignment || (alignment == 2 && hTempObj->GetParam(WWD::Param_LocationZ) < 4000);
        if (bStacked) {
            logic = isBack ? "BackStackedCrates" : "FrontStackedCrates";
        } else {
            logic = isBack ? "BehindCrate" : "FrontCrate";
        }

        hTempObj->SetLogic(logic);
        ddDurability->setEnabled(!bStacked && !bPick);
        labDurability->setColor(ddDurability->isEnabled() ? 0xFFe1e1e1 : 0xFFb2b2b2);
    }

    void cEditObjCrate::RebuildWindow() {
        if (bStackable)
            for (int i = 0; i < 9; i++)
                invTabs[i]->setVisible(i < iCratesCount);

        bool bwarpf = false;
        if (bStackable) {
            for (auto & invTab : invTabs)
                if (invTab->isVisible() && invTab->GetItem().second == 32) //warp
                    bwarpf = true;
        } else
            bwarpf = invTabs[0]->GetItem().second == 32;
        bShowWarpOptions = bwarpf;

        tfWarpX->setVisible(bShowWarpOptions);
        tfWarpY->setVisible(bShowWarpOptions);
        butWarpPick->setVisible(bShowWarpOptions);
        labWarpDest->setVisible(bShowWarpOptions);

        labTreasures->setY(110 + WARP_SECTION_HEIGHT * bShowWarpOptions);

        for (int i = 0; i < iCratesCount; i++)
            invTabs[i]->setY(151 + (iCratesCount - i - 1) * 43 + WARP_SECTION_HEIGHT * bShowWarpOptions);

        win->setHeight(355 + (iCratesCount - 1) * 43 + WARP_SECTION_HEIGHT * bShowWarpOptions);
        win->setY(hState->vPort->GetY() + hState->vPort->GetHeight() - win->getHeight());
        _butSave->setY(win->getHeight() - 55);
        _butAddNext->setY(win->getHeight() - 55);
        butRandomize->setY(win->getHeight() - 100);
        cbIncludeSpecials->setY(win->getHeight() - 140);
        cbRandomCount->setY(win->getHeight() - 120);
        hState->vPort->MarkToRedraw();

        SetLogic(iCratesCount > 2);
        if (!ddDurability->isEnabled())
            ddDurability->setSelected(0);
        labDurability->setColor(ddDurability->isEnabled() ? 0xFFe1e1e1 : 0xFFb2b2b2);
    }

    void cEditObjCrate::ApplyInventoryToObject() {
        if (invTabs[0]->GetItem().second == -1)
            hTempObj->SetParam(WWD::Param_Powerup, 0);
        else
            hTempObj->SetParam(WWD::Param_Powerup, invTabs[0]->GetItem().second);
        if (bStackable) {
            for (int i = 0; i < 2; i++) {
                WWD::Rect r;
                if (invTabs[i * 4 + 1]->GetItem().second == -1) r.x1 = 0;
                else r.x1 = invTabs[i * 4 + 1]->GetItem().second;
                if (invTabs[i * 4 + 2]->GetItem().second == -1) r.y1 = 0;
                else r.y1 = invTabs[i * 4 + 2]->GetItem().second;
                if (invTabs[i * 4 + 3]->GetItem().second == -1) r.x2 = 0;
                else r.x2 = invTabs[i * 4 + 3]->GetItem().second;
                if (invTabs[i * 4 + 4]->GetItem().second == -1) r.y2 = 0;
                else r.y2 = invTabs[i * 4 + 4]->GetItem().second;
                hTempObj->SetUserRect(i, r);
            }
        }
        SetLogic(bStackable && invTabs[2]->isVisible());
    }
}
