#include "editRope.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"

#define SIM_ROPE_DEFAULT_SPEED 3.0f

extern HGE *hge;

namespace ObjEdit {
    cEditObjRope::cEditObjRope(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enTreasure;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Rope", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 200, 160));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        win->setMovable(0);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        win->add(_butAddNext, 0, 100);
        win->add(_butSave, 100, 100);

        cbAnimated = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Rope", "Animated"));
        cbAnimated->adjustSize();
        cbAnimated->addActionListener(hAL);
        cbAnimated->setSelected(!strcmp(hTempObj->GetLogic(), "AniRope"));
        win->add(cbAnimated, 5, 15);

        labSpeed = new SHR::Lab(GETL2S("EditObj_Rope", "Speed"));
        labSpeed->adjustSize();
        win->add(labSpeed, 5, 40);

        char tmp[64];
        int speed = hTempObj->GetParam(WWD::Param_SpeedX);
        if (!speed) speed = SIM_ROPE_DEFAULT_SPEED * 1000;
        sprintf(tmp, "%d", speed);
        tfSpeed = new SHR::TextField(tmp);
        tfSpeed->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tfSpeed->SetNumerical(1, 0);
        win->add(tfSpeed, 90, 40);

        labOffset = new SHR::Lab(GETL2S("EditObj_Rope", "Delay"));
        labOffset->adjustSize();
        win->add(labOffset, 5, 65);

        sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_Speed));
        tfOffset = new SHR::TextField(tmp);
        tfOffset->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tfOffset->SetNumerical(1, 0);
        win->add(tfOffset, 90, 65);

        labSpeed->setVisible(cbAnimated->isSelected());
        tfSpeed->setVisible(cbAnimated->isSelected());
        labOffset->setVisible(cbAnimated->isSelected());
        tfOffset->setVisible(cbAnimated->isSelected());
        if (!cbAnimated->isSelected()) {
            _butSave->setY(40);
            win->setHeight(100);
            win->setY(win->getY() - 60);
        }
    }

    cEditObjRope::~cEditObjRope() {
        delete cbAnimated;
        delete labSpeed;
        delete labOffset;
        delete tfSpeed;
        delete tfOffset;
        delete win;
        hState->vPort->MarkToRedraw(1);
    }

    void cEditObjRope::Save() {
        if (cbAnimated->isSelected()) {
            hTempObj->SetParam(WWD::Param_SpeedX, atoi(tfSpeed->getText().c_str()));
            hTempObj->SetParam(WWD::Param_Speed, atoi(tfOffset->getText().c_str()));
        } else {
            hTempObj->SetParam(WWD::Param_SpeedX, 0);
            hTempObj->SetParam(WWD::Param_Speed, 0);
        }
    }

    void cEditObjRope::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == cbAnimated) {
            if (cbAnimated->isSelected()) {
                hTempObj->SetLogic("AniRope");
            } else {
                hTempObj->SetLogic("DoNothing");
            }
            labSpeed->setVisible(cbAnimated->isSelected());
            tfSpeed->setVisible(cbAnimated->isSelected());
            labOffset->setVisible(cbAnimated->isSelected());
            tfOffset->setVisible(cbAnimated->isSelected());
            _butSave->setY(40 + cbAnimated->isSelected() * 60);
            win->setHeight(100 + cbAnimated->isSelected() * 60);
            win->setY(hState->vPort->GetY() + hState->vPort->GetHeight() - win->getHeight());
        }
    }
}
