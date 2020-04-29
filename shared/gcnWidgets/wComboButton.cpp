#include "wComboButton.h"
#include "wButton.h"
#include "guichan/exception.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/key.hpp"
#include "guichan/mouseevent.hpp"
#include "guichan/mouseinput.hpp"
#include "../../WapMap/cInterfaceSheet.h"

#include "../../WapMap/globals.h"

extern HGE *hge;

namespace SHR {
    ComboButEntry::ComboButEntry(hgeSprite *ico, std::string cap) {
        sprIcon = ico;
        strCaption = cap;
        fTimer = 0;
        iID = -1;
        UpdateWidth();
    }


    void ComboButEntry::SetIcon(hgeSprite *spr) {
        sprIcon = spr;
        UpdateWidth();
    }

    void ComboButEntry::SetCaption(std::string str) {
        strCaption = str;
        UpdateWidth();
    }


    void ComboButEntry::UpdateWidth() {
        iWidth = 4 + 16 + 4 + GV->fntMyriad13->GetStringWidth(strCaption.c_str());
    }

    ComboBut::ComboBut(cInterfaceSheet *Parts) {
        setFocusable(true);
        setFrameSize(0);

        addMouseListener(this);
        addFocusListener(this);
        hGfx = Parts;
        fTimer = 0;
        iSelectedID = -1;
        adjustSize();
        mMousePressed = 0;
        iFocused = -1;
    }

    void ComboBut::draw(Graphics *graphics) {
        UpdateTooltip(mHasMouse);
        int x, y;
        getAbsolutePosition(x, y);

        iFocused = -1;
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        if (mx > x && my > y && mx < x + getWidth() && my < y + getHeight()) {
            int sx = mx - x;
            int optx = 0;
            for (size_t i = 0; i < vEntries.size(); i++) {
                int w = vEntries[i].GetWidth();
                if (sx < optx + w) {
                    iFocused = i;
                    break;
                }
                optx += w;
            }
        }

        But::drawButton(hGfx, 2, x, y, getWidth(), getHeight(), 0xFFFFFFFF);

        int lx = x + 5;
        for (int i = 0; i < vEntries.size(); i++) {
            if (iSelectedID == i && vEntries[i].fTimer < 0.4f) {
                vEntries[i].fTimer += hge->Timer_GetDelta() * 2;
                if (vEntries[i].fTimer > 0.4f) vEntries[i].fTimer = 0.4f;
            } else if (iSelectedID != i && vEntries[i].fTimer > 0.2f) {
                vEntries[i].fTimer -= hge->Timer_GetDelta() * 2;
                if (vEntries[i].fTimer < 0.2f) vEntries[i].fTimer = 0.2f;
            } else if (iFocused == i && vEntries[i].fTimer < 0.2f) {
                vEntries[i].fTimer += hge->Timer_GetDelta();
                if (vEntries[i].fTimer > 0.2f) vEntries[i].fTimer = 0.2f;
            } else if (iFocused != i && iSelectedID != i && vEntries[i].fTimer > 0.0f) {
                vEntries[i].fTimer -= hge->Timer_GetDelta();
                if (vEntries[i].fTimer < 0.0f) vEntries[i].fTimer = 0.0f;
            }
            unsigned char p = (vEntries[i].fTimer * 2.5f * 255.0f);

            if (vEntries[i].fTimer > 0.0f) {
                hge->Gfx_SetClipping(lx - (i == 0 ? 5 : 2), y, vEntries[i].GetWidth() + (i == 0 ? 2 : i == vEntries.size() - 1 ? 1 : 0), getHeight());
                But::drawButton(hGfx, 3, x, y, getWidth(), getHeight(), SETA(0xFFFFFFFF, p));
                hge->Gfx_SetClipping();
            }

            if (vEntries[i].sprIcon != 0) {
                int addOX = (i > 0 ? 1 : 0);
                vEntries[i].sprIcon->Render(lx + addOX, y);
                lx += 17 + addOX;
            }

            GV->fntMyriad13->SetColor(ARGB(255, p, p, p));
            GV->fntMyriad13->Render(lx, y + 1, HGETEXT_LEFT, vEntries[i].strCaption.c_str(), 0);
            lx += GV->fntMyriad13->GetStringWidth(vEntries[i].strCaption.c_str());

            lx += 6;
        }
        RenderTooltip();
    }

    void ComboBut::setSelectedEntryID(int i, bool bEvent) {
        iSelectedID = (i < int(vEntries.size()) ? i : -1);
        if (bEvent)
            distributeActionEvent();
    }

    void ComboBut::adjustSize() {
        int w = 4;
        for (int i = 0; i < vEntries.size(); i++)
            w += vEntries[i].GetWidth();
        setWidth(w);
        setHeight(20);
    }

    void ComboBut::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT) {
            int x, y;
            getAbsolutePosition(x, y);
            mMousePressed = true;
            mouseEvent.consume();
            iSelectedID = -1;
            float mx, my;
            hge->Input_GetMousePos(&mx, &my);
            int sx = mx - x;
            int optx = 0;
            for (size_t i = 0; i < vEntries.size(); i++) {
                int w = vEntries[i].GetWidth();
                if (sx < optx + w) {
                    iSelectedID = i;
                    break;
                }
                optx += w;
            }
            distributeActionEvent();
        }
    }

    void ComboBut::mouseExited(MouseEvent &mouseEvent) {
        mHasMouse = false;
        mMousePressed = 0;
    }

    void ComboBut::mouseEntered(MouseEvent &mouseEvent) {
        mHasMouse = true;
    }

    void ComboBut::mouseReleased(MouseEvent &mouseEvent) {
        mMousePressed = 0;
    }


    void ComboBut::focusLost(const Event &event) {
        mMousePressed = false;
        mKeyPressed = false;
    }


    bool ComboBut::showHand() {
        return iFocused != iSelectedID;
    }

    void ComboBut::addEntry(ComboButEntry n) {
        n.iID = vEntries.size();
        vEntries.push_back(n);
        adjustSize();
    }
}
