#include "winInventoryPickbox.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../databanks/imageSets.h"

extern HGE *hge;

namespace ObjEdit {
    cInvPickbox::cInvPickbox() {
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Inventory", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 256, 346));
        //win->setClose(1);
        win->addActionListener(this);
        //win->add(vpAdv);
        win->setVisible(0);

        sli = new SHR::Slider(631);
        sli->setOrientation(SHR::Slider::VERTICAL);
        sli->setDimension(gcn::Rectangle(0, 0, 11, 271));
        win->add(sli, 240, 10);

        vp = new WIDG::Viewport(this, 0);
        win->add(vp, 5, 5);

        GV->editState->conMain->add(win, 0, 0);
    }

    cInvPickbox::~cInvPickbox() {
        delete vp;
        delete sli;
        delete win;
    }

    void cInvPickbox::Think() {
        if (!win->isVisible()) return;
        int dx, dy;
        win->getAbsolutePosition(dx, dy);
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        int pickedid = -1;
        if (GV->editState->conMain->getWidgetAt(mx, my) == win && mx > dx + 4 && my > dy + 28 && mx < dx + 4 + 252 &&
            my < dy + 28 + 267) {
            sli->setValue(sli->getValue() - hge->Input_GetMouseWheel() * 30);
            mx -= dx + 4;
            my -= dy + 28;
            my += sli->getValue();
            int restx = int(mx) % 60, resty = int(my) % 60;
            if (restx > 5 && restx < 53 && resty > 5 && resty < 53) {
                pickedid = int(my / 60) * 4 + int(mx / 60);
                if (pickedid < InventoryItemsCount) {
                    if (hge->Input_KeyDown(HGEK_LBUTTON))
                        GV->editState->hInvCtrl->SetItemInClipboard(GV->editState->hInvCtrl->GetItemByIt(pickedid),
                                                                    restx - 30, resty - 30);
                }
            }
        }
    }

    void cInvPickbox::Draw(int iCode) {
        //240x900
        int dx, dy;
        win->getAbsolutePosition(dx, dy);

        hge->Gfx_RenderLine(dx + 3, dy + 27, dx + 242, dy + 27, GV->colLineDark);
        hge->Gfx_RenderLine(dx + 3, dy + 28, dx + 242, dy + 28, GV->colLineBright);
        hge->Gfx_RenderLine(dx + 3, dy + 27 + 269, dx + 242, dy + 27 + 269, GV->colLineDark);
        hge->Gfx_RenderLine(dx + 3, dy + 28 + 269, dx + 242, dy + 28 + 269, GV->colLineBright);

        hge->Gfx_RenderLine(dx + 3, dy + 26, dx + 3, dy + 27 + 269, GV->colLineDark);
        hge->Gfx_RenderLine(dx + 4, dy + 28, dx + 4, dy + 28 + 267, GV->colLineBright);

        hge->Gfx_SetClipping(dx + 4, dy + 28, 252, 267);

        int off = sli->getValue();
        int lineoff = off % 60;
        for (int i = 1; i < 4; i++)
            hge->Gfx_RenderLine(dx + 5 + i * 60 - 1, dy + 28, dx + 5 + i * 60 - 1, dy + 29 + 267, GV->colLineBright);
        for (int i = 1; i < 6; i++) {
            hge->Gfx_RenderLine(dx + 4, dy + 29 + i * 60 - 2 - lineoff, dx + 4 + 238, dy + 29 + i * 60 - 2 - lineoff,
                                GV->colLineDark);
            hge->Gfx_RenderLine(dx + 4, dy + 29 + i * 60 - 1 - lineoff, dx + 4 + 238, dy + 29 + i * 60 - 2 - lineoff,
                                GV->colLineBright);
        }
        for (int i = 1; i < 4; i++)
            hge->Gfx_RenderLine(dx + 5 + i * 60 - 2, dy + 28, dx + 5 + i * 60 - 2, dy + 29 + 267, GV->colLineDark);
        int startrow = off / 60;
        int endrow = startrow + 6;
        if (endrow > 15) endrow = 15;
        for (int y = startrow; y < endrow; y++)
            for (int x = 0; x < 4; x++) {
                int id = y * 4 + x;
                if (id < InventoryItemsCount) {
                    cSprBankAsset *asset = GV->editState->SprBank->GetAssetByID(
                            GV->editState->hInvCtrl->GetItemByIt(id).first.c_str());
                    hgeSprite* spr = GV->sprSmiley;
                    if (asset) {
                        int iframe = GV->editState->hInvCtrl->GetAnimFrame() % asset->GetSpritesCount();
                        spr = asset->GetIMGByIterator(iframe)->GetSprite();
                    }
                    spr->SetColor(0xFFFFFFFF);
                    spr->SetFlip(0, 0, 1);
                    int grdim = spr->GetWidth();
                    if (spr->GetHeight() > grdim) grdim = spr->GetHeight();
                    float fScale = 1.0f;
                    if (grdim > 48) fScale = 48.0f / float(grdim);
                    spr->RenderEx(dx + 4 + x * 60 + 30, dy + 28 + y * 60 + 30 - off, 0, fScale);
                }
            }
        hge->Gfx_SetClipping();
        int pickedid = -1;
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        if (GV->editState->conMain->getWidgetAt(mx, my) == win && mx > dx + 4 && mx > dy + 28 && mx < dx + 4 + 252 &&
            my < dy + 28 + 267) {
            mx -= dx + 4;
            my -= dy + 28;
            my += off;
            int restx = int(mx) % 60, resty = int(my) % 60;
            if (restx > 5 && restx < 53 && resty > 5 && resty < 53) {
                pickedid = int(my / 60) * 4 + int(mx / 60);
                if (pickedid < InventoryItemsCount)
                    pickedid = GV->editState->hInvCtrl->GetItemByIt(pickedid).second;
                else
                    pickedid = -1;
            }
        }
        if (pickedid != -1) {
            GV->fntMyriad16->SetColor(0xFFFFFFFF);
            GV->fntMyriad16->printf(dx + 5, dy + 28 + 267 + 5, HGETEXT_LEFT,
                                    "%s: (%s #~y~%d~w~)", 0,
                                    GETL2S("EditObj_Inventory", "Effect"),
                                    GETL2S("EditObj_Inventory", "ItemID"),
                                    pickedid);
            char tmp[32];
            sprintf(tmp, "E_%d", pickedid);
            GV->fntMyriad16->printf(dx + 5, dy + 28 + 267 + 7 + 15, HGETEXT_LEFT,
                                    "~y~%s~w~", 0,
                                    GETL2S("EditObj_Inventory", tmp));
        }
    }

    void cInvPickbox::action(const gcn::ActionEvent &actionEvent) {

    }

    void cInvPickbox::SetVisible(bool b) {
        win->setVisible(b);
    }

    void cInvPickbox::SetPosition(int x, int y) {
        win->setPosition(x, y);
    }

    int cInvPickbox::GetWidth() {
        return win->getWidth();
    }

    int cInvPickbox::GetHeight() {
        return win->getHeight();
    }
}
