#include "editTreasure.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include "../databanks/imageSets.h"

extern HGE *hge;

namespace ObjEdit {
    static const int iTreasuresID[] = {33, 1, 6, 2, 3, 4, 5, 41, 42, 43, 44, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
                                       18, 45, 46, 47, 48, 49, 50, 51, 52};

    cEditObjTreasure::cEditObjTreasure(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enTreasure;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Treasure", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 287, 580));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        win->setShadow(0);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        win->add(_butAddNext, win->getWidth() - 110, 495);
        win->add(_butSave, win->getWidth() - 110, 528);

        char tmp[24];
        sprintf(tmp, "%p", this);

        for (int i = 0; i < 31; i++) {
            int x, y;
            if (i < 3) {
                x = i;
                y = 0;
            }
            else {
                x = (i + 1) % 4;
                y = (i + 1) / 4;
            }
            rbType[i] = new SHR::RadBut(GV->hGfxInterface, "", tmp,
                                        !strcmp(hTempObj->GetImageSet(),
                                                hState->hInvCtrl->GetItemByID(iTreasuresID[i]).first.c_str()));
            rbType[i]->adjustSize();
            rbType[i]->addActionListener(hAL);
            win->add(rbType[i], 5 + x * 71, 5 + y * 60 + 30 - rbType[i]->getHeight() / 2 + 9);
        }

        bool bAny = 0;
        for (int i = 0; i < 31; i++)
            if (rbType[i]->isSelected()) {
                bAny = 1;
                break;
            }
        if (!bAny) {
            rbType[0]->setSelected(1);
            hTempObj->SetImageSet(hState->hInvCtrl->GetItemByID(iTreasuresID[0]).first.c_str());
        }

        cbAddGlitter = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Treasure", "AddGlitter"));
        cbAddGlitter->adjustSize();
        cbAddGlitter->addActionListener(hAL);
        cbAddGlitter->setSelected(!strcmp(hTempObj->GetLogic(), "TreasurePowerup"));
        win->add(cbAddGlitter, 5, 507);
    }

    cEditObjTreasure::~cEditObjTreasure() {
        for (int i = 0; i < 31; i++)
            delete rbType[i];
        delete win;
        hState->vPort->MarkToRedraw(1);
    }

    void cEditObjTreasure::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == cbAddGlitter) {
            if (cbAddGlitter->isSelected()) {
                hTempObj->SetLogic("TreasurePowerup");
            } else {
                hTempObj->SetLogic("GlitterlessPowerup");
            }
        }
        for (int i = 0; i < 31; i++)
            if (actionEvent.getSource() == rbType[i]) {
                hTempObj->SetImageSet(hState->hInvCtrl->GetItemByID(iTreasuresID[i]).first.c_str());
                hState->vPort->MarkToRedraw(1);
            }
    }

    void cEditObjTreasure::Draw() {
        int dx, dy;
        win->getAbsolutePosition(dx, dy);

        for (int i = 1; i < 4; i++)
            hge->Gfx_RenderLine(dx + 5 + i * 71 - 1, dy + 28, dx + 5 + i * 71 - 1, dy + 29 + 267 + 210,
                                GV->colLineBright);
        for (int i = 1; i < 9; i++) {
            hge->Gfx_RenderLine(dx + 4, dy + 29 + i * 60 - 2, dx + win->getWidth(), dy + 29 + i * 60 - 2,
                                GV->colLineDark);
            hge->Gfx_RenderLine(dx + 4, dy + 29 + i * 60 - 1, dx + win->getWidth(), dy + 29 + i * 60 - 2,
                                GV->colLineBright);
        }
        for (int i = 1; i < 4; i++)
            hge->Gfx_RenderLine(dx + 5 + i * 71 - 2, dy + 28, dx + 5 + i * 71 - 2, dy + 29 + 267 + 210,
                                GV->colLineDark);

        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        for (int i = 0; i < 31; i++) {
            int x, y;
            if (i < 3) {
                x = i;
                y = 0;
            }
            else {
                x = (i + 1) % 4;
                y = (i + 1) / 4;
            }
            cSprBankAsset *asset = hState->SprBank->GetAssetByID(
                    hState->hInvCtrl->GetItemByID(iTreasuresID[i]).first.c_str());
            int iframe = hState->hInvCtrl->GetAnimFrame() % asset->GetSpritesCount();
            hgeSprite *spr = asset->GetIMGByIterator(iframe)->GetSprite();
            spr->SetColor(0xFFFFFFFF);
            spr->SetFlip(0, 0, true);
            int grdim = spr->GetWidth();
            if (spr->GetHeight() > grdim) grdim = spr->GetHeight();
            float fScale = 1.0f;
            if (grdim > 48) fScale = 48.0f / float(grdim);
            spr->RenderEx(dx + 12 + x * 71 + 35, dy + 28 + y * 60 + 30 - 4, 0, fScale);
            if (hState->conMain->getWidgetAt(mx, my) == win)
                if (mx > dx + 12 + x * 71 + 35 - 24 && mx < dx + 12 + x * 71 + 35 + 24 &&
                    my > dy + 28 + y * 60 + 30 - 4 - 24 && my < dy + 28 + y * 60 + 30 - 4 + 24) {
                    hState->bShowHand = 1;
                    if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                        rbType[i]->setSelected(1);
                        Action(gcn::ActionEvent(rbType[i], ""));
                    }
                }
        }
    }
}
