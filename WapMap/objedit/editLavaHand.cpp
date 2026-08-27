#include "editLavaHand.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"

extern HGE *hge;

namespace ObjEdit {
    cEditObjLavaHand::cEditObjLavaHand(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enLavaHand;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_LavaHand", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 385, 180));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        win->setMovable(0);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        win->add(_butAddNext, win->getWidth() - 210, win->getHeight() - 60);
        win->add(_butSave, win->getWidth() - 110, win->getHeight() - 60);

        labActivate = new SHR::Lab(GETL2S("EditObj_LavaHand", "ActivationArea"));
        labActivate->adjustSize();
        win->add(labActivate, 5, 15);

        hRectPick = new cProcPickRect(hTempObj);
        hRectPick->setActionListener(hAL);
        hRectPick->setType(PickRect_MinMax);
        win->add(hRectPick, 5, 35);

        int xoffset = 140;

        labDelay = new SHR::Lab(GETL2S("EditObj_LavaHand", "Delay"));
        labDelay->adjustSize();
        win->add(labDelay, xoffset + 5, 15);

        char tmp[20];
        int delay = obj->GetParam(WWD::Param_Speed);

        sprintf(tmp, "%d", delay < 0 ? 0 : delay);
        tfDelay = new SHR::TextField(tmp);
        tfDelay->setDimension(gcn::Rectangle(0, 0, 75, 20));
        tfDelay->SetNumerical(1, 0);
        tfDelay->addActionListener(hAL);
        win->add(tfDelay, xoffset + 145, 15);

        labThrows = new SHR::Lab(GETL2S("EditObj_LavaHand", "Throws"));
        labThrows->adjustSize();
        win->add(labThrows, xoffset + 5, 40);

        int throws = obj->GetParam(WWD::Param_Smarts);

        sprintf(tmp, "%d", throws == 0 ? 1 : throws);
        tfThrows = new SHR::TextField(tmp);
        tfThrows->setDimension(gcn::Rectangle(0, 0, 75, 20));
        tfThrows->SetNumerical(1, 0);
        tfThrows->addActionListener(hAL);
        win->add(tfThrows, xoffset + 145, 40);

        _butSave->setEnabled(hRectPick->isValid());
    }

    cEditObjLavaHand::~cEditObjLavaHand() {
        delete labActivate;
        delete hRectPick;
        delete labDelay;
        delete labThrows;
        delete tfDelay;
        delete tfThrows;
        delete win;
        hState->vPort->MarkToRedraw();
    }

    void cEditObjLavaHand::Save() {
        hTempObj->SetParam(WWD::Param_Speed, atoi(tfDelay->getText().c_str()));
        int throws = atoi(tfThrows->getText().c_str());
        hTempObj->SetParam(WWD::Param_Smarts, throws == 0 ? 1 : throws);
    }

    void cEditObjLavaHand::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == hRectPick->getPickButton()) {
            bAllowDragging = !hRectPick->IsPicking();
            _butSave->setEnabled(hRectPick->isValid() && !hRectPick->IsPicking());
            return;
        }
    }

    void cEditObjLavaHand::_Think(bool bMouseConsumed) {
        if (!bMouseConsumed)
            hRectPick->Think();
        _butSave->setEnabled(hRectPick->isValid());
    }

    
    void cEditObjLavaHand::Draw() {
        int dx, dy;
        win->getAbsolutePosition(dx, dy);

        hge->Gfx_RenderLine(dx + 140, dy + 24, dx + 140, dy + win->getHeight(), GV->colLineDark);
        hge->Gfx_RenderLine(dx + 141, dy + 24, dx + 141, dy + win->getHeight(), GV->colLineBright);

        hge->Gfx_RenderLine(dx + 140, dy + 125, dx + win->getWidth(), dy + 125, GV->colLineDark);
        hge->Gfx_RenderLine(dx + 141, dy + 126, dx + win->getWidth(), dy + 126, GV->colLineBright);
    }
}
