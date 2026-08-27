#include "editBreakPlank.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../databanks/imageSets.h"

extern HGE *hge;

namespace ObjEdit {
    cEditObjBreakPlank::cEditObjBreakPlank(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enBreakPlank;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_BreakPlank", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 200, 130));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        win->add(_butAddNext, 0, 80);
        win->add(_butSave, 100, 80);

        labCounter = new SHR::Lab(GETL2S("EditObj_BreakPlank", "Delay"));
        labCounter->adjustSize();
        win->add(labCounter, 5, 15);

        char tmp[64];
        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_Counter));
        tfCounter = new SHR::TextField(tmp);
        tfCounter->adjustSize();
        tfCounter->setWidth(50);
        win->add(tfCounter, 100, 15);

        labWidth = new SHR::Lab(GETL2S("EditObj_BreakPlank", "Width"));
        labWidth->adjustSize();
        win->add(labWidth, 5, 47);

        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_Width));
        tfWidth = new SHR::TextField(tmp);
        tfWidth->adjustSize();
        tfWidth->setWidth(50);
        tfWidth->addActionListener(hAL);
        win->add(tfWidth, 100, 45);
    }

    cEditObjBreakPlank::~cEditObjBreakPlank() {
        delete tfWidth;
        delete tfCounter;
        delete labCounter;
        delete labWidth;
        delete win;
        hState->vPort->MarkToRedraw();
    }

    void cEditObjBreakPlank::Save() {
        hTempObj->SetParam(WWD::Param_Counter, atoi(tfCounter->getText().c_str()));
    }

    void cEditObjBreakPlank::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == tfWidth) {
            int nw = atoi(tfWidth->getText().c_str());
            hTempObj->SetParam(WWD::Param_Width, nw);
            hState->vPort->MarkToRedraw();
        }
    }

    void cEditObjBreakPlank::_Think(bool bMouseConsumed) {
    }
}
