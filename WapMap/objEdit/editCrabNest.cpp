#include "editCrabNest.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"

extern HGE *hge;

namespace ObjEdit {
    cEditObjCrabNest::cEditObjCrabNest(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enCrabNest;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_CrabNest", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 140, 420));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        _butAddNext->setWidth(125);
        _butSave->setWidth(125);

        win->add(_butAddNext, 5, win->getHeight() - 89);
        win->add(_butSave, 5, win->getHeight() - 53);

        labNumber = new SHR::Lab(GETL2S("EditObj_CrabNest", "CrabNum"));
        labNumber->adjustSize();
        win->add(labNumber, 5, 16);

        int num = hTempObj->GetUserValue(0);
        if (!num) num = 3;
        else if (num < 0) num = 0;
        else if (num > 9) num = 9;

        tfNumber = new SHR::TextField(std::to_string(num));
        tfNumber->setDimension(gcn::Rectangle(0, 0, 50, 20));
        tfNumber->setMaxLength(1);
        tfNumber->SetNumerical(true, false);
        tfNumber->addActionListener(hAL);
        win->add(tfNumber, labNumber->getWidth() + 10, 15);

        labActivate = new SHR::Lab(GETL2S("EditObj_CrabNest", "ActivationArea"));
        labActivate->adjustSize();
        win->add(labActivate, 5, 180);

        hRectPick = new cProcPickRect(hTempObj);
        hRectPick->setAllowEmpty(1);
        hRectPick->setActionListener(hAL);
        win->add(hRectPick, 5, 205);

        for (int i = 0; i < 9; i++) {
            invTabs[i] = new SHR::InvTab(&GV->gcnParts);
            invTabs[i]->setDimension(gcn::Rectangle(0, 0, 42, 42));
            invTabs[i]->addActionListener(hAL);
            invTabs[i]->SetCrab(num > i);
            win->add(invTabs[i], 5 + (i % 3) * 42, 45 + 42 * (i / 3));
        }

        int taboff = 0;
        if (hTempObj->GetParam(WWD::Param_Powerup) > 0) {
            cInventoryItem it = GV->editState->hInvCtrl->GetItemByID(hTempObj->GetParam(WWD::Param_Powerup));
            if (it.second != -1) {
                invTabs[0]->SetItem(it);
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

        hInventory = new cInvPickbox();
        hInventory->SetPosition(hState->vPort->GetX() + hState->vPort->GetWidth() - hInventory->GetWidth(),
                                hState->vPort->GetY() + hState->vPort->GetHeight() - hInventory->GetHeight());
        hInventory->SetVisible(1);
    }

    cEditObjCrabNest::~cEditObjCrabNest() {
        delete labActivate;
        delete tfNumber;
        delete labNumber;
        delete hRectPick;
        for (int i = 0; i < 9; i++)
            delete invTabs[i];
        delete win;
        delete hInventory;
        hState->vPort->MarkToRedraw();
    }

    void cEditObjCrabNest::Save() {
        ApplyInventoryToObject();
        int num = atoi(tfNumber->getText().c_str());
        if (!num) num = -1;
        hTempObj->SetUserValue(0, num);
        hTempObj->SetParam(WWD::Param_MinX, hRectPick->getValue(0));
        hTempObj->SetParam(WWD::Param_MinY, hRectPick->getValue(1));
        hTempObj->SetParam(WWD::Param_MaxX, hRectPick->getValue(2));
        hTempObj->SetParam(WWD::Param_MaxY, hRectPick->getValue(3));
    }

    void cEditObjCrabNest::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == tfNumber) {
            int num = atoi(tfNumber->getText().c_str());
            for (int i = 0; i < 9; ++i) {
                invTabs[i]->SetCrab(num > i);
            }
        } else if (actionEvent.getSource() == hRectPick->getPickButton()) {
            bAllowDragging = !hRectPick->IsPicking();
            _butSave->setEnabled(!tfNumber->isMarkedInvalid() && hRectPick->isValid() && !hRectPick->IsPicking());
            return;
        } else {
            for (int i = 0; i < 4; i++)
                if (actionEvent.getSource() == hRectPick->getTextField(i)) {
                    if (hRectPick->isValid()) {
                        hTempObj->SetParam(WWD::Param_MinX, hRectPick->getValue(0));
                        hTempObj->SetParam(WWD::Param_MinY, hRectPick->getValue(1));
                        hTempObj->SetParam(WWD::Param_MaxX, hRectPick->getValue(2));
                        hTempObj->SetParam(WWD::Param_MaxY, hRectPick->getValue(3));
                    }
                    return;
                }
        }
    }

    void cEditObjCrabNest::_Think(bool bMouseConsumed) {
        if (!bMouseConsumed)
            hRectPick->Think();
        hInventory->Think();
    }

    void cEditObjCrabNest::ApplyInventoryToObject() {
        int values[9] = {0};

        int i = 0;
        for (auto & invTab : invTabs) {
            if (invTab->GetItem().second != -1) {
                values[i++] = invTab->GetItem().second;
            }
        }

        hTempObj->SetParam(WWD::Param_Powerup, values[0]);

        for (int i = 0; i < 2; i++) {
            WWD::Rect r;
            r.x1 = values[i * 4 + 1];
            r.y1 = values[i * 4 + 2];
            r.x2 = values[i * 4 + 3];
            r.y2 = values[i * 4 + 4];
            hTempObj->SetUserRect(i, r);
        }
    }
}
