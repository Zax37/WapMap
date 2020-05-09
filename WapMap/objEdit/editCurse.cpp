#include "editCurse.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../databanks/imageSets.h"

extern HGE *hge;

namespace ObjEdit {
    static const char *CURSES[] = {"GAME_CURSES_FREEZE",
                                   "GAME_CURSES_TREASURE",
                                   "GAME_CURSES_HEALTH",
                                   "GAME_CURSES_LIFE",
                                   "GAME_CURSES_AMMO",
                                   "GAME_CURSES_MAGIC"};

    cEditObjCurse::cEditObjCurse(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enCurse;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Curse", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 800 - 11, 160));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        win->add(_butAddNext, 810 - 11 - 215, 108);
        win->add(_butSave, 810 - 11 - 115, 108);
        fCurseAnim = 0.0f;
        iFrame = 0;

        for (int i = 0; i < 6; i++) {
            char tmp[12];
            sprintf(tmp, "%p", this);
            char tmp2[32];
            sprintf(tmp2, "Curse%d", i);
            rbCurses[i] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Curse", tmp2), tmp,
                                          !(strcmp(obj->GetImageSet(), CURSES[i])));
            rbCurses[i]->adjustSize();
            rbCurses[i]->addActionListener(hAL);
            //rbCurses[i]->setHeight(15);
            win->add(rbCurses[i], i * 131 + (65 - rbCurses[i]->getWidth() / 2), 80);
        }
        bool bAnySelected = 0;
        for (int i = 0; i < 6; i++)
            if (rbCurses[i]->isSelected()) {
                bAnySelected = 1;
                break;
            }
        if (!bAnySelected) {
            rbCurses[0]->setSelected(1);
            hTempObj->SetImageSet(CURSES[0]);
            hState->vPort->MarkToRedraw(1);
        }
    }

    cEditObjCurse::~cEditObjCurse() {
        for (int i = 0; i < 6; i++)
            delete rbCurses[i];
        delete win;
        hState->vPort->MarkToRedraw(1);
    }

    void cEditObjCurse::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        }
        for (int i = 0; i < 6; i++)
            if (actionEvent.getSource() == rbCurses[i]) {
                hTempObj->SetImageSet(CURSES[i]);
                hState->vPort->MarkToRedraw(1);
                return;
            }
    }

    void cEditObjCurse::Draw() {
        int dx, dy;
        win->getAbsolutePosition(dx, dy);
        fCurseAnim += hge->Timer_GetDelta();
        while (fCurseAnim > 0.1f) {
            fCurseAnim -= 0.1f;
            iFrame++;
            if (iFrame > 3)
                iFrame = 0;
        }
        hge->Gfx_RenderLine(dx, dy + 120, dx + win->getWidth(), dy + 120, 0xFF5c5c5c);
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        for (int i = 0; i < 6; i++) { //131
            hgeSprite *spr = hState->SprBank->GetAssetByID(CURSES[i])->GetIMGByIterator((iFrame + i) % 4)->GetSprite();
            spr->SetColor(0xFFFFFFFF);
            spr->Render(dx + i * 131 + 65, dy + 60);
            if (mx > dx + i * 131 + 65 - 30 && mx < dx + i * 131 + 65 + 30 && my > dy + 60 - 30 && my < dy + 60 + 30) {
                hState->bShowHand = 1;
                if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                    rbCurses[i]->setSelected(1);
                    Action(gcn::ActionEvent(rbCurses[i], ""));
                }
            }
            hge->Gfx_RenderLine(dx + i * 131, dy + 24, dx + i * 131, dy + 120,0xFF1f1f1f);
            hge->Gfx_RenderLine(dx + i * 131 + 1, dy + 24, dx + i * 131 + 1, dy + 120, 0xFF5c5c5c);
        }
        hge->Gfx_RenderLine(dx, dy + 121, dx + win->getWidth(), dy + 121, 0xFF1f1f1f);
        GV->fntMyriad13->SetColor(0xFFa1a1a1);
        char tmp[64];
        for (int i = 0; i < 6; i++)
            if (rbCurses[i]->isSelected()) {
                sprintf(tmp, "CurseDesc%d", i);
                break;
            }
        GV->fntMyriad13->printf(dx + 15, dy + 132, HGETEXT_LEFT, "%s: ~y~%s~l~", 0,
                                GETL2S("EditObj_Curse", "CurseEffect"), GETL2S("EditObj_Curse", tmp), 0);
    }
}
