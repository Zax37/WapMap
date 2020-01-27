#include "editCrabNest.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"

extern HGE *hge;

namespace ObjEdit {
    cEditObjCrabNest::cEditObjCrabNest(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enCrabNest;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_CrabNest", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 250, 205));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        win->setMovable(0);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());


        win->add(_butAddNext, win->getWidth() - 110, win->getHeight() - 125);
        win->add(_butSave, win->getWidth() - 110, win->getHeight() - 85);

        labNumber = new SHR::Lab(GETL2S("EditObj_CrabNest", "CrabNum"));
        labNumber->adjustSize();
        win->add(labNumber, 5, 15);

        int num = hTempObj->GetUserValue(0);
        if (!num) num = 3;
        char tmp[25];
        sprintf(tmp, "%d", num);
        tfNumber = new SHR::TextField(tmp);
        tfNumber->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tfNumber->SetNumerical(1, 0);
        tfNumber->addActionListener(hAL);
        win->add(tfNumber, 140, 15);
        tfNumber->setMarkedInvalid(num == 0 || num > 10);

        labActivate = new SHR::Lab(GETL2S("EditObj_CrabNest", "ActivationArea"));
        labActivate->adjustSize();
        win->add(labActivate, 5, 40);

        hRectPick = new cProcPickRect(hTempObj);
        hRectPick->SetAllowEmpty(1);
        hRectPick->AddWidgets(win, 5, 60);
        hRectPick->SetActionListener(hAL);

        _butSave->setEnabled(!tfNumber->isMarkedInvalid() && hRectPick->IsValid());
    }

    cEditObjCrabNest::~cEditObjCrabNest() {
        delete labActivate;
        delete tfNumber;
        delete labNumber;
        delete hRectPick;
        delete win;
        hState->vPort->MarkToRedraw(1);
    }

    void cEditObjCrabNest::Save() {
        hTempObj->SetUserValue(0, atoi(tfNumber->getText().c_str()));
        hTempObj->SetParam(WWD::Param_MinX, hRectPick->GetValue(0));
        hTempObj->SetParam(WWD::Param_MinY, hRectPick->GetValue(1));
        hTempObj->SetParam(WWD::Param_MaxX, hRectPick->GetValue(2));
        hTempObj->SetParam(WWD::Param_MaxY, hRectPick->GetValue(3));
    }

    void cEditObjCrabNest::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == tfNumber) {
            int v = atoi(tfNumber->getText().c_str());
            tfNumber->setMarkedInvalid(v == 0 || v > 10);
            _butSave->setEnabled(!tfNumber->isMarkedInvalid() && hRectPick->IsValid());
            return;
        } else if (actionEvent.getSource() == hRectPick->GetPickButton()) {
            bAllowDragging = !hRectPick->IsPicking();
            _butSave->setEnabled(!tfNumber->isMarkedInvalid() && hRectPick->IsValid() && !hRectPick->IsPicking());
            return;
        }
        for (int i = 0; i < 4; i++)
            if (actionEvent.getSource() == hRectPick->GetTextField(i)) {
                if (hRectPick->IsValid()) {
                    hTempObj->SetParam(WWD::Param_MinX, hRectPick->GetValue(0));
                    hTempObj->SetParam(WWD::Param_MinY, hRectPick->GetValue(1));
                    hTempObj->SetParam(WWD::Param_MaxX, hRectPick->GetValue(2));
                    hTempObj->SetParam(WWD::Param_MaxY, hRectPick->GetValue(3));
                }
                return;
            }
    }

    void cEditObjCrabNest::_Think(bool bMouseConsumed) {
        if (!bMouseConsumed)
            hRectPick->Think();
    }
}
