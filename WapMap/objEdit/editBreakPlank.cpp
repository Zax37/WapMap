#include "editBreakPlank.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
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

        bDragging = bPicked = 0;
    }

    cEditObjBreakPlank::~cEditObjBreakPlank() {
        delete tfWidth;
        delete tfCounter;
        delete labCounter;
        delete labWidth;
        delete win;
        hState->vPort->MarkToRedraw(1);
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
            hState->vPort->MarkToRedraw(1);
        }
    }

    void cEditObjBreakPlank::_Think(bool bMouseConsumed) {
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        bool bMouseIn = 1;
        if (mx < hState->vPort->GetX() || my < hState->vPort->GetY() ||
            mx > hState->vPort->GetX() + hState->vPort->GetWidth() ||
            my > hState->vPort->GetY() + hState->vPort->GetHeight())
            bMouseIn = 0;

        int imgw, imgh;
        hgeSprite *spr = hState->SprBank->GetObjectSprite(hTempObj);
        imgw = spr->GetWidth();
        imgh = spr->GetHeight();
        int origx = hTempObj->GetParam(WWD::Param_LocationX),
                origy = hTempObj->GetParam(WWD::Param_LocationY);
        mx = hState->Scr2WrdX(hState->GetActivePlane(), mx);
        my = hState->Scr2WrdY(hState->GetActivePlane(), my);

        int modwidth = (hTempObj->GetParam(WWD::Param_Width) - 1) * 64;
        if (modwidth < 0) modwidth = 0;

        if (bDragging) {
            int neww = (mx - (origx + imgw / 2)) / 64 + 2;
            if (neww < 0) neww = 0;
            if (neww != hTempObj->GetParam(WWD::Param_Width)) {
                hTempObj->SetParam(WWD::Param_Width, neww);
                hState->vPort->MarkToRedraw(1);
                char tmp[32];
                sprintf(tmp, "%d", neww);
                tfWidth->setText(tmp);
                hState->hPlaneData[hState->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(hTempObj);
            }
            if (!hge->Input_GetKeyState(HGEK_LBUTTON)) {
                bDragging = 0;
            }
        }

        float hsx, hsy;
        spr->GetHotSpot(&hsx, &hsy);
        origx -= hsx - imgw / 2;
        origy -= hsy - imgh / 2;

        if (mx > origx - imgw / 2 + modwidth && mx < origx + imgw / 2 + modwidth &&
            my > origy - imgh / 2 && my < origy + imgh / 2) {
            if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                iClickX = mx;
                iClickY = my;
                if (bPicked) {
                    bDragging = 1;
                }
            } else if (hge->Input_KeyUp(HGEK_LBUTTON)) {
                if (iClickX == mx &&
                    iClickY == my) {
                    bPicked = !bPicked;
                    bAllowDragging = !bPicked;
                    _bDragging = 0;
                    dwHighlightColor = bPicked ? 0xFFFFFF00 : 0xFFFF0000;
                }
            }
            hState->bShowHand = 1;
            if (!bPicked)
                hState->dwCursorColor = 0xFFFFFF00;
        }
    }
}
