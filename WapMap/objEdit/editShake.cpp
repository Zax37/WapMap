#include "editShake.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"

extern HGE *hge;

namespace ObjEdit {
    cEditObjShake::cEditObjShake(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enShake;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Shake", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 250, 175));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        win->setMovable(0);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        win->add(_butAddNext, win->getWidth() - 110, win->getHeight() - 125);
        win->add(_butSave, win->getWidth() - 110, win->getHeight() - 85);

        labActivate = new SHR::Lab(GETL2S("EditObj_Shake", "ActivationArea"));
        labActivate->adjustSize();
        win->add(labActivate, 5, 10);

        hRectPick = new cProcPickRect(hTempObj);
        hRectPick->setActionListener(hAL);
        hRectPick->setType(PickRect_AttackRect);
        win->add(hRectPick, 5, 30);

        _butSave->setEnabled(hRectPick->isValid());
    }

    cEditObjShake::~cEditObjShake() {
        delete labActivate;
        delete hRectPick;
        delete win;
        hState->vPort->MarkToRedraw();
    }

    void cEditObjShake::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == hRectPick->getPickButton()) {
            bAllowDragging = !hRectPick->IsPicking();
            _butSave->setEnabled(hRectPick->isValid() && !hRectPick->IsPicking());
            return;
        }
    }

    void cEditObjShake::_Think(bool bMouseConsumed) {
        if (!bMouseConsumed)
            hRectPick->Think();
    }

    void cEditObjShake::RenderObjectOverlay() {

        hge->Gfx_RenderLine(0, 0, 100, 100, GV->colLineDark);
    }
}
