#include "editCheckpoint.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include "../databanks/imageSets.h"

extern HGE *hge;

namespace ObjEdit {
    cEditObjCheckpoint::cEditObjCheckpoint(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enCheckpoint;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Checkpoint", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 214, 361));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        win->add(_butAddNext, 5, 305);
        win->add(_butSave, 105, 305);

        fCpAnimTimer = fScpAnimTimer = 0;
        iCpAnimFrame = 8;
        iScpAnimFrame = 10;

        char tmp[24];
        sprintf(tmp, "%p", this);

        rbType[0] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Checkpoint", "Checkpoint"), tmp,
                                    !strcmp(hTempObj->GetImageSet(), "GAME_CHECKPOINTFLAG"));
        rbType[0]->adjustSize();
        rbType[0]->addActionListener(hAL);
        win->add(rbType[0], 5, 10);
        rbType[1] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Checkpoint", "SuperCheckpoint"), tmp,
                                    !strcmp(hTempObj->GetImageSet(), "GAME_SUPERCHECKPOINT"));
        rbType[1]->adjustSize();
        rbType[1]->addActionListener(hAL);
        win->add(rbType[1], 5, 150);

        bool bOrient = 1;
        if (hTempObj->GetDrawFlags() & WWD::Flag_dr_Mirror && !(hTempObj->GetDrawFlags() & WWD::Flag_dr_Invert) ||
            !(hTempObj->GetDrawFlags() & WWD::Flag_dr_Mirror) && hTempObj->GetDrawFlags() & WWD::Flag_dr_Invert)
            bOrient = 0;

        if (!rbType[0]->isSelected() && !rbType[1]->isSelected()) {
            rbType[0]->setSelected(1);
            hTempObj->SetLogic("Checkpoint");
            hTempObj->SetImageSet("GAME_CHECKPOINTFLAG");
            hState->vPort->MarkToRedraw(1);
        }
    }

    cEditObjCheckpoint::~cEditObjCheckpoint() {
        for (int i = 0; i < 2; i++) {
            delete rbType[i];
        }
        delete win;
        hState->vPort->MarkToRedraw(1);
    }

    void cEditObjCheckpoint::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == rbType[0]) {
            hTempObj->SetImageSet("GAME_CHECKPOINTFLAG");
            hTempObj->SetLogic("Checkpoint");
            hState->vPort->MarkToRedraw(1);
        } else if (actionEvent.getSource() == rbType[1]) {
            hTempObj->SetImageSet("GAME_SUPERCHECKPOINT");
            hTempObj->SetLogic("FirstSuperCheckpoint");
            hState->vPort->MarkToRedraw(1);
        }
    }

    void cEditObjCheckpoint::Draw() {
        int dx, dy;
        win->getAbsolutePosition(dx, dy);
        fCpAnimTimer += hge->Timer_GetDelta();
        while (fCpAnimTimer > 0.15) {
            fCpAnimTimer -= 0.1;
            iCpAnimFrame++;
            if (iCpAnimFrame > 12)
                iCpAnimFrame = 8;
        }
        fScpAnimTimer += hge->Timer_GetDelta();
        while (fScpAnimTimer > 0.15) {
            fScpAnimTimer -= 0.1;
            iScpAnimFrame++;
            if (iScpAnimFrame > 12)
                iScpAnimFrame = 8;
        }

        hState->SprBank->GetAssetByID("GAME_CHECKPOINTFLAG")->GetIMGByID(iCpAnimFrame)->GetSprite()->Render(dx + 124,
                                                                                                            dy + 100);
        hState->SprBank->GetAssetByID("GAME_SUPERCHECKPOINT")->GetIMGByID(iScpAnimFrame)->GetSprite()->Render(dx + 124,
                                                                                                              dy + 250);
        GV->fntMyriad13->SetColor(0xFF000000);
    }
}
