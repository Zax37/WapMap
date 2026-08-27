#include "editStatue.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "winInventoryPickbox.h"
#include "../cObjectUserData.h"

namespace ObjEdit {
    std::string lmEditObjStatueDurability::getElementAt(int i) {
        if (i == 0) return GETL2S("EditObj_Container", "Dur_1");
        else if (i == 1) return GETL2S("EditObj_Container", "Dur_2");
        else if (i == 2) return GETL2S("EditObj_Container", "Dur_3");
    }

    int lmEditObjStatueDurability::getNumberOfElements() {
        return 3;
    }

    cEditObjStatue::cEditObjStatue(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enStatue;
        // bPick = false;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Statue", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 283, 0));
        win->setClose(true);
        win->addActionListener(hAL);
        win->add(vpAdv);
        win->setMovable(false);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        int yOffset = 13, x1 = 5, x2 = 150, butW = 125;

        _butAddNext->setWidth(butW);
        _butSave->setWidth(butW);
        win->add(_butAddNext, x1, win->getHeight() - 60);
        win->add(_butSave, x2, win->getHeight() - 60);

        labAlign = new SHR::Lab(GETL2S("Z_Coord", "Label"));
        labAlign->adjustSize();
        win->add(labAlign, x1, yOffset);

        labDurability = new SHR::Lab(GETL2S("EditObj_Container", "Durability"));
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

        char tmp[24];
        sprintf(tmp, "%p", this);

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
            z = logic[0] == 'B' ? 1100 : 5000;
            hTempObj->SetParam(WWD::Param_LocationZ, z);
			GetUserDataFromObj(hTempObj)->SetZ(z);
			hState->vPort->MarkToRedraw();
        }

        alignment = 2;
        if (logic[0] == 'B' && z < 4000) {
            alignment = 0;
        } else if (logic[0] == 'F' && z >= 4000) {
            alignment = 1;
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
        cbRandomCount->setSelected(true);
        win->add(cbRandomCount, x1, 0);

        butRandomize = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Container", "Randomize"));
        butRandomize->setDimension(gcn::Rectangle(0, 0, butW, 33));
        butRandomize->addActionListener(hAL);
        win->add(butRandomize, x2, win->getHeight() - 120);

//        labWarpDest = new SHR::Lab(GETL2S("EditObj_Container", "WarpDest"));
//        labWarpDest->adjustSize();
//        win->add(labWarpDest, x1, yOffset);
//
//        yOffset += 25;
//
//        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedX));
//        tfWarpX = new SHR::TextField(tmp);
//        tfWarpX->addActionListener(hAL);
//        tfWarpX->setDimension(gcn::Rectangle(0, 0, 100, 20));
//        win->add(tfWarpX, 20, yOffset);
//
//        yOffset += 5;
//
//        butWarpPick = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Warp", "Pick"));
//        butWarpPick->setDimension(gcn::Rectangle(0, 0, butW, 33));
//        butWarpPick->addActionListener(hAL);
//        win->add(butWarpPick, x2, yOffset);
//
//        yOffset += 20;
//
//        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedY));
//        tfWarpY = new SHR::TextField(tmp);
//        tfWarpY->addActionListener(hAL);
//        tfWarpY->setDimension(gcn::Rectangle(0, 0, 100, 20));
//        win->add(tfWarpY, 20, yOffset);

        labTreasures = new SHR::Lab(GETL2S("EditObj_Container", "Treasures"));
        labTreasures->adjustSize();
        win->add(labTreasures, x1, 0);

        for (int y = 0; y < 3; y++)
            for (int x = 0; x < 3; x++) {
                invTabs[y * 3 + x] = new SHR::InvTab(&GV->gcnParts, true);
                invTabs[y * 3 + x]->adjustSize();
                invTabs[y * 3 + x]->addActionListener(hAL);
                win->add(invTabs[y * 3 + x], 65 + x * 54, 115 + y * 60);
            }

        if (hTempObj->GetParam(WWD::Param_Powerup) != 0)
            invTabs[0]->SetItem(hState->hInvCtrl->GetItemByID(hTempObj->GetParam(WWD::Param_Powerup)));

        for (int i = 0; i < 2; i++) {
            WWD::Rect r = hTempObj->GetUserRect(i);
            if (r.x1 != 0) invTabs[i * 4 + 1]->SetItem(hState->hInvCtrl->GetItemByID(r.x1));
            if (r.y1 != 0) invTabs[i * 4 + 2]->SetItem(hState->hInvCtrl->GetItemByID(r.y1));
            if (r.x2 != 0) invTabs[i * 4 + 3]->SetItem(hState->hInvCtrl->GetItemByID(r.x2));
            if (r.y2 != 0) invTabs[i * 4 + 4]->SetItem(hState->hInvCtrl->GetItemByID(r.y2));
        }

        rbType[alignment]->setSelected(true);
        RebuildWindow();

        hInventory = new cInvPickbox(true);
        hInventory->SetPosition(win->getX() + win->getWidth() + 10, hState->vPort->GetY() + hState->vPort->GetHeight() - hInventory->GetHeight());
        hInventory->SetVisible(true);
    }

#define FIRST_SECTION_HEIGHT 120
#define WARP_SECTION_HEIGHT 80

    void cEditObjStatue::RebuildWindow() {
//        bool bwarpf = false;
//        for (auto & invTab : invTabs)
//            if (invTab->isVisible() && invTab->GetItem().second == 32) //warp
//                bwarpf = true;
//        bShowWarpOptions = bwarpf;

        for (int y = 0; y < 3; y++)
            for (int x = 0; x < 3; x++) {
                invTabs[y * 3 + x]->setY(115 + y * 54); // + bShowWarpOptions * WARP_SECTION_HEIGHT
            }

//        tfWarpX->setVisible(bShowWarpOptions);
//        tfWarpY->setVisible(bShowWarpOptions);
//        butWarpPick->setVisible(bShowWarpOptions);
//        labWarpDest->setVisible(bShowWarpOptions);

        labTreasures->setY(110); // + WARP_SECTION_HEIGHT * bShowWarpOptions

        win->setHeight(430); // + WARP_SECTION_HEIGHT * bShowWarpOptions
        win->setY(hState->vPort->GetY() + hState->vPort->GetHeight() - win->getHeight());
        _butSave->setY(win->getHeight() - 55);
        _butAddNext->setY(win->getHeight() - 55);
        butRandomize->setY(win->getHeight() - 100);
        cbIncludeSpecials->setY(win->getHeight() - 140);
        cbRandomCount->setY(win->getHeight() - 120);
    }

    cEditObjStatue::~cEditObjStatue() {
        for (auto & i : rbType)
            delete i;
        delete labDurability;
        delete ddDurability;
        delete lmDurability;
        delete labAlign;
        delete labTreasures;
        for (auto & invTab : invTabs)
            delete invTab;
        delete win;
        delete hInventory;
        hState->vPort->MarkToRedraw();
    }

    void cEditObjStatue::Save() {
        if (invTabs[0]->GetItem().second == -1)
            hTempObj->SetParam(WWD::Param_Powerup, 0);
        else
            hTempObj->SetParam(WWD::Param_Powerup, invTabs[0]->GetItem().second);
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

    void cEditObjStatue::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = true;
        } else if (actionEvent.getSource() == butRandomize) {
            for (auto &invTab: invTabs) {
                if (cbRandomCount->isSelected() ? hge->Random_Float(0.f, 1.f) >= 0.5f : invTab->GetItem().second != -1) {
                    int id = hge->Random_Int(0, (cbIncludeSpecials->isSelected() ? InventoryItemsCount - 2 : 31));
                    if (id == 3) id++;
                    invTab->SetItem(GV->editState->hInvCtrl->GetItemByIt(id));
                } else if (invTab->GetItem().second != -1) {
                    invTab->SetItem(cInventoryItem("", -1));
                }
            }
            Save();
        } else if (actionEvent.getSource() == rbType[0]) {
            hTempObj->SetLogic("BehindStatue");
            hTempObj->SetParam(WWD::Param_LocationZ, 1010);
			GetUserDataFromObj(hTempObj)->SetZ(1010);
            tfCustomZ->setEnabled(false);
			hState->vPort->MarkToRedraw();
        } else if (actionEvent.getSource() == rbType[1]) {
            hTempObj->SetLogic("FrontStatue");
            hTempObj->SetParam(WWD::Param_LocationZ, 5000);
			GetUserDataFromObj(hTempObj)->SetZ(5000);
            tfCustomZ->setEnabled(false);
			hState->vPort->MarkToRedraw();
        } else if (actionEvent.getSource() == rbType[2]) {
            int z = std::atoi(tfCustomZ->getText().c_str());
            hTempObj->SetLogic(z < 4000 ? "BehindStatue" : "FrontStatue");
            hTempObj->SetParam(WWD::Param_LocationZ, z);
			GetUserDataFromObj(hTempObj)->SetZ(z);
            tfCustomZ->setEnabled(true);
			hState->vPort->MarkToRedraw();
        } else if (actionEvent.getSource() == ddDurability) {
            if (ddDurability->getSelected() == 0)
                hTempObj->SetParam(WWD::Param_Health, 1);
            else if (ddDurability->getSelected() == 1)
                hTempObj->SetParam(WWD::Param_Health, 2);
            else if (ddDurability->getSelected() == 2)
                hTempObj->SetParam(WWD::Param_Health, 999);
//        } else if (actionEvent.getSource() == tfWarpX) {
//            hTempObj->SetParam(WWD::Param_SpeedX, atoi(tfWarpX->getText().c_str()));
//            hState->vPort->MarkToRedraw();
//        } else if (actionEvent.getSource() == tfWarpY) {
//            hTempObj->SetParam(WWD::Param_SpeedY, atoi(tfWarpY->getText().c_str()));
//            hState->vPort->MarkToRedraw();
//        } else if (actionEvent.getSource() == butWarpPick) {
//            bPick = !bPick;
//            tfWarpX->setEnabled(!bPick);
//            tfWarpY->setEnabled(!bPick);
//            for (auto & i : rbType) i->setEnabled(!bPick);
//            for (auto & invTab : invTabs) invTab->setEnabled(!bPick);
//            _butSave->setEnabled(!bPick);
//            butRandomize->setEnabled(!bPick);
//            cbRandomCount->setEnabled(!bPick);
//            cbIncludeSpecials->setEnabled(!bPick);
//            ddDurability->setEnabled(!bPick);
//            tfCustomZ->setEnabled(!bPick && alignment == 2);
//            labAlign->setColor(ddDurability->isEnabled() ? 0xFFe1e1e1 : 0xFFb2b2b2);
//            labTreasures->setColor(ddDurability->isEnabled() ? 0xFFe1e1e1 : 0xFFb2b2b2);
//            labDurability->setColor(ddDurability->isEnabled() ? 0xFFe1e1e1 : 0xFFb2b2b2);
//            butWarpPick->setCaption(GETL2S("EditObj_Warp", (bPick ? "Unpick" : "Pick")));
        } else if (actionEvent.getSource() == tfCustomZ) {
            int z = std::atoi(tfCustomZ->getText().c_str());
            if (!z) z = 1100;
            hTempObj->SetLogic(z < 4000 ? "BehindStatue" : "FrontStatue");
            hTempObj->SetParam(WWD::Param_LocationZ, z);
			GetUserDataFromObj(hTempObj)->SetZ(z);
			hState->vPort->MarkToRedraw();
        } else {
            for (auto &invTab: invTabs)
                if (actionEvent.getSource() == invTab) {
                    RebuildWindow();
                    Save();
                }
        }
    }

    void cEditObjStatue::Draw() {
        hInventory->Think();
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

//        if (bPick) {
//            float mx, my;
//            hge->Input_GetMousePos(&mx, &my);
//            if (hge->Input_KeyDown(HGEK_LBUTTON) &&
//                hState->conMain->getWidgetAt(mx, my) == hState->vPort->GetWidget()) {
//                char tmp[64];
//                sprintf(tmp, "%d", hState->Scr2WrdX(hState->GetActivePlane(), mx));
//                tfWarpX->setText(tmp, true);
//                sprintf(tmp, "%d", hState->Scr2WrdY(hState->GetActivePlane(), my));
//                tfWarpY->setText(tmp, true);
//                butWarpPick->simulatePress();
//            }
//        }
//
//        if (bShowWarpOptions) {
//            hge->Gfx_RenderLine(dx, dy + FIRST_SECTION_HEIGHT + WARP_SECTION_HEIGHT, dx + win->getWidth(), dy + FIRST_SECTION_HEIGHT + WARP_SECTION_HEIGHT, GV->colLineDark);
//            hge->Gfx_RenderLine(dx, dy + FIRST_SECTION_HEIGHT + WARP_SECTION_HEIGHT + 1, dx + win->getWidth(), dy + FIRST_SECTION_HEIGHT + WARP_SECTION_HEIGHT + 1, GV->colLineBright);
//            GV->fntMyriad16->SetColor(0xFFe1e1e1);
//            GV->fntMyriad16->Render(dx + 7, dy + tfWarpX->getY() + tfWarpX->getHeight() - 2, HGETEXT_LEFT, "X:", 0);
//            GV->fntMyriad16->Render(dx + 7, dy + tfWarpY->getY() + tfWarpY->getHeight() - 2, HGETEXT_LEFT, "Y:", 0);
//        }
    }
}
