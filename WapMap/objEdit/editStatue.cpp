#include "editStatue.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include "winInventoryPickbox.h"

extern HGE *hge;

namespace ObjEdit {
    std::string lmEditObjStatueDurability::getElementAt(int i) {
        if (i == 0) return GETL2S("EditObj_Statue", "Dur_1");
        else if (i == 1) return GETL2S("EditObj_Statue", "Dur_2");
        else if (i == 2) return GETL2S("EditObj_Statue", "Dur_3");
    }

    int lmEditObjStatueDurability::getNumberOfElements() {
        return 3;
    }

    cEditObjStatue::cEditObjStatue(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enStatue;
        bPick = 0;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Statue", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 250, 340));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        win->setMovable(0);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        win->add(_butAddNext, 25, 280);
        win->add(_butSave, 125, 280);

        char tmp[24];
        sprintf(tmp, "%p", this);

        labAlign = new SHR::Lab(GETL2S("EditObj_Statue", "Align"));
        labAlign->adjustSize();
        win->add(labAlign, 5, 12);

        rbType[0] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Statue", "Behind"), tmp,
                                    !strcmp(hTempObj->GetLogic(), "BehindStatue"));
        rbType[0]->adjustSize();
        rbType[0]->addActionListener(hAL);
        win->add(rbType[0], 5, 30);
        rbType[1] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Statue", "Front"), tmp,
                                    !strcmp(hTempObj->GetLogic(), "FrontStatue"));
        rbType[1]->adjustSize();
        rbType[1]->addActionListener(hAL);
        win->add(rbType[1], 100, 30);

        labDurability = new SHR::Lab(GETL2S("EditObj_Statue", "Durability"));
        labDurability->adjustSize();
        win->add(labDurability, 5, 55);

        lmDurability = new lmEditObjStatueDurability();

        ddDurability = new SHR::DropDown();
        ddDurability->setDimension(gcn::Rectangle(0, 0, 150, 20));
        ddDurability->setListModel(lmDurability);
        ddDurability->addActionListener(hAL);
        ddDurability->SetGfx(&GV->gcnParts);
        win->add(ddDurability, labDurability->getX() + labDurability->getWidth() + 5, 55);

        if (hTempObj->GetParam(WWD::Param_Health) <= 1)
            ddDurability->setSelected(0);
        else if (hTempObj->GetParam(WWD::Param_Health) < 999)
            ddDurability->setSelected(1);
        else if (hTempObj->GetParam(WWD::Param_Health) >= 999)
            ddDurability->setSelected(2);

        labWarpDest = new SHR::Lab(GETL2S("EditObj_Statue", "WarpDest"));
        labWarpDest->adjustSize();
        win->add(labWarpDest, 5, 85);

        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedX));
        tfWarpX = new SHR::TextField(tmp);
        tfWarpX->addActionListener(hAL);
        tfWarpX->setDimension(gcn::Rectangle(0, 0, 100, 20));
        win->add(tfWarpX, 20, 105);

        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedY));
        tfWarpY = new SHR::TextField(tmp);
        tfWarpY->addActionListener(hAL);
        tfWarpY->setDimension(gcn::Rectangle(0, 0, 100, 20));
        win->add(tfWarpY, 20, 132);

        butWarpPick = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Warp", "Pick"));
        butWarpPick->setDimension(gcn::Rectangle(0, 0, 110, 33));
        butWarpPick->addActionListener(hAL);
        win->add(butWarpPick, 125, 115);

        labTreasures = new SHR::Lab(GETL2S("EditObj_Statue", "Treasures"));
        labTreasures->adjustSize();
        win->add(labTreasures, 5, 90);

        for (int y = 0; y < 3; y++)
            for (int x = 0; x < 3; x++) {
                invTabs[y * 3 + x] = new SHR::InvTab(&GV->gcnParts);
                invTabs[y * 3 + x]->adjustSize();
                invTabs[y * 3 + x]->addActionListener(hAL);
                win->add(invTabs[y * 3 + x], 44 + x * 54, 115 + y * 54);
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

        RebuildWindow();

        hInventory = new cInvPickbox();
        hInventory->SetPosition(hState->vPort->GetX() + hState->vPort->GetWidth() - hInventory->GetWidth(),
                                hState->vPort->GetY() + hState->vPort->GetHeight() - hInventory->GetHeight());
        hInventory->SetVisible(1);
    }

    void cEditObjStatue::RebuildWindow() {
        bool bwarpf = 0;
        for (int i = 0; i < 9; i++)
            if (invTabs[i]->isVisible() && invTabs[i]->GetItem().second == 32) //warp
                bwarpf = 1;
        bShowWarpOptions = bwarpf;

        for (int y = 0; y < 3; y++)
            for (int x = 0; x < 3; x++) {
                invTabs[y * 3 + x]->setY(115 + y * 54 + bShowWarpOptions * 75);
            }

        tfWarpX->setVisible(bShowWarpOptions);
        tfWarpY->setVisible(bShowWarpOptions);
        butWarpPick->setVisible(bShowWarpOptions);
        labWarpDest->setVisible(bShowWarpOptions);

        labTreasures->setY(90 + 75 * bShowWarpOptions);

        win->setHeight(340 + 75 * bShowWarpOptions);
        win->setY(hState->vPort->GetY() + hState->vPort->GetHeight() - win->getHeight());
        _butSave->setY(win->getHeight() - 60);
        _butAddNext->setY(win->getHeight() - 60);
    }

    cEditObjStatue::~cEditObjStatue() {
        for (int i = 0; i < 2; i++)
            delete rbType[i];
        delete labDurability;
        delete ddDurability;
        delete lmDurability;
        delete labAlign;
        delete labTreasures;
        for (int i = 0; i < 9; i++)
            delete invTabs[i];
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
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == rbType[0]) {
            hTempObj->SetLogic("BehindStatue");
            hTempObj->SetParam(WWD::Param_LocationZ, 1010);
        } else if (actionEvent.getSource() == rbType[1]) {
            hTempObj->SetLogic("FrontStatue");
            hTempObj->SetParam(WWD::Param_LocationZ, 5000);
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
            for (int i = 0; i < 2; i++) rbType[i]->setEnabled(!bPick);
            for (int i = 0; i < 9; i++) invTabs[i]->setEnabled(!bPick);
            _butSave->setEnabled(!bPick);
            ddDurability->setEnabled(!bPick);
            butWarpPick->setCaption(GETL2S("EditObj_Warp", (bPick ? "Unpick" : "Pick")));
        }
        for (int i = 0; i < 9; i++)
            if (actionEvent.getSource() == invTabs[i])
                RebuildWindow();
    }

    void cEditObjStatue::Draw() {
        hInventory->Think();
        int dx, dy;
        win->getAbsolutePosition(dx, dy);
        hge->Gfx_RenderLine(dx, dy + 101, dx + win->getWidth(), dy + 101, GV->colLineDark);
        hge->Gfx_RenderLine(dx, dy + 102, dx + win->getWidth(), dy + 102, GV->colLineBright);

        if (bPick) {
            float mx, my;
            hge->Input_GetMousePos(&mx, &my);
            if (hge->Input_KeyDown(HGEK_LBUTTON) &&
                hState->conMain->getWidgetAt(mx, my) == hState->vPort->GetWidget()) {
                char tmp[64];
                sprintf(tmp, "%d", hState->Scr2WrdX(hState->GetActivePlane(), mx));
                tfWarpX->setText(tmp, 1);
                sprintf(tmp, "%d", hState->Scr2WrdY(hState->GetActivePlane(), my));
                tfWarpY->setText(tmp, 1);
                butWarpPick->simulatePress();
            }
        }

        if (bShowWarpOptions) {
            hge->Gfx_RenderLine(dx, dy + 101 + 75, dx + win->getWidth(), dy + 101 + 75, GV->colLineDark);
            hge->Gfx_RenderLine(dx, dy + 102 + 75, dx + win->getWidth(), dy + 102 + 75, GV->colLineBright);
            GV->fntMyriad16->SetColor(0xFFe1e1e1);
            GV->fntMyriad16->Render(dx + 7, dy + 122, HGETEXT_LEFT, "X:", 0);
            GV->fntMyriad16->Render(dx + 7, dy + 149, HGETEXT_LEFT, "Y:", 0);
        }
    }
}
