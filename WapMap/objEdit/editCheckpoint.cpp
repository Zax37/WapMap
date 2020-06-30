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
        win->setDimension(gcn::Rectangle(0, 0, 214, 400));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        win->add(_butAddNext, 5, 345);
        win->add(_butSave, 105, 345);

        fCpAnimTimer = fScpAnimTimer = 0;
        iCpAnimFrame = 8;
        iScpAnimFrame = 10;

        char tmp[24];
        sprintf(tmp, "%p", this);

        rbType[0] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Checkpoint", "Checkpoint"), tmp,
                                    !strcmp(hTempObj->GetLogic(), "Checkpoint"));
        rbType[0]->adjustSize();
        rbType[0]->addActionListener(hAL);
        win->add(rbType[0], 5, 15);
        rbType[1] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Checkpoint", "SuperCheckpoint1"), tmp,
                                    !strcmp(hTempObj->GetLogic(), "FirstSuperCheckpoint"));
        rbType[1]->adjustSize();
        rbType[1]->addActionListener(hAL);
        win->add(rbType[1], 5, 155);
        rbType[2] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Checkpoint", "SuperCheckpoint2"), tmp,
                                    !strcmp(hTempObj->GetLogic(), "SecondSuperCheckpoint"));
        rbType[2]->adjustSize();
        rbType[2]->addActionListener(hAL);
        win->add(rbType[2], 5, 175);
        rbType[3] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Checkpoint", "BossStager"), tmp,
                                    !strcmp(hTempObj->GetLogic(), "BossStager"));
        rbType[3]->adjustSize();
        rbType[3]->addActionListener(hAL);
        win->add(rbType[3], 5, 325);
    }

    cEditObjCheckpoint::~cEditObjCheckpoint() {
        for (int i = 0; i < 2; i++) {
            delete rbType[i];
        }
        delete win;
        hState->vPort->MarkToRedraw();
    }

    void cEditObjCheckpoint::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == rbType[0]) {
            hTempObj->SetImageSet("GAME_CHECKPOINTFLAG");
            hTempObj->SetLogic("Checkpoint");
            hState->vPort->MarkToRedraw();
        } else if (actionEvent.getSource() == rbType[1]) {
            hTempObj->SetImageSet("GAME_SUPERCHECKPOINT");
            hTempObj->SetLogic("FirstSuperCheckpoint");
            hState->vPort->MarkToRedraw();
        } else if (actionEvent.getSource() == rbType[2]) {
            hTempObj->SetImageSet("GAME_SUPERCHECKPOINT");
            hTempObj->SetLogic("SecondSuperCheckpoint");
            hState->vPort->MarkToRedraw();
        } else if (actionEvent.getSource() == rbType[3]) {
            hTempObj->SetImageSet("GAME_SOUNDICON");
            hTempObj->SetLogic("BossStager");
            hState->vPort->MarkToRedraw();
        }
        hTempObj->SetDrawFlags(actionEvent.getSource() == rbType[3] ? WWD::Flag_dr_NoDraw : WWD::Flag_dr_Nothing);
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
                                                                                                              dy + 270);
    }
}
