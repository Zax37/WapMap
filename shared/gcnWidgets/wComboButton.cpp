#include "wComboButton.h"
#include "wButton.h"
#include "guichan/exception.hpp"
#include "guichan/graphics.hpp"
#include "guichan/mouseevent.hpp"
#include "guichan/mouseinput.hpp"
#include "../../WapMap/cInterfaceSheet.h"
#include "../../WapMap/globals.h"

extern HGE *hge;

namespace SHR {
    ComboButEntry::ComboButEntry(hgeSprite *ico, std::string cap, const char* tooltip) {
        sprIcon = ico;
        strCaption = cap;
        if (tooltip) {
            SetTooltip(tooltip);
        }
        fTimer = 0;
        iID = -1;
        UpdateWidth();
    }

    ComboButEntry::ComboButEntry(const ComboButEntry& other)
        : ComboButEntry(other.GetIcon(), other.GetCaption(), other.GetTooltip()) { }

    void ComboButEntry::SetIcon(hgeSprite *spr) {
        sprIcon = spr;
        UpdateWidth();
    }

    void ComboButEntry::SetCaption(const char* str) {
        strCaption = str;
        UpdateWidth();
    }


    void ComboButEntry::UpdateWidth() {
        iWidth = 10 + (sprIcon == NULL ? 0 : 16) + (int)GV->fntMyriad16->GetStringWidth(strCaption.c_str());
    }

    ComboBut::ComboBut(cInterfaceSheet *Parts) {
        setFocusable(true);
        setFrameSize(0);

        addKeyListener(this);
        addMouseListener(this);
        addFocusListener(this);
        hGfx = Parts;
        fTimer = 0;
        iSelectedID = -1;
        adjustSize();
        mMousePressed = false;
        iFocused = -1;
    }

    void ComboBut::draw(Graphics *graphics) {
        int x, y;
        getAbsolutePosition(x, y);

        iFocused = -1;
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        if (mx > x && my > y && mx < x + getWidth() && my < y + getHeight()) {
            int sx = mx - x;
            int optx = 0;
            size_t i = 0;
            for (; i < vEntries.size(); i++) {
                int w = vEntries[i].GetWidth();
                if (!i) w += 2;
                if (sx < optx + w) {
                    vEntries[i].UpdateTooltip(true);
                    iFocused = i;
                    break;
                } else {
                    vEntries[i].UpdateTooltip(false);
                }
                optx += w;
            }
            for (++i; i < vEntries.size(); i++) {
                vEntries[i].UpdateTooltip(false);
            }
        }

        unsigned alpha = getAlpha();
        if (isFocused() && mKeyboardFocus) {
            But::drawButton(hGfx, 3, x, y,getWidth(), getHeight(), SETA(0xFFFFFF, alpha));
        }
        But::drawButton(hGfx, 2, x + 1, y + 1, getWidth() - 2, getHeight() - 2, SETA(0xFFFFFF, alpha));

        int lx = x + 6;
        for (int i = 0; i < vEntries.size(); i++) {
            if (iSelectedID == i && vEntries[i].fTimer < 0.4f) {
                vEntries[i].fTimer += hge->Timer_GetDelta() * 2;
                if (vEntries[i].fTimer > 0.4f) vEntries[i].fTimer = 0.4f;
            } else if (iSelectedID != i && vEntries[i].fTimer > 0.2f) {
                vEntries[i].fTimer -= hge->Timer_GetDelta() * 2;
                if (vEntries[i].fTimer < 0.2f) vEntries[i].fTimer = 0.0f;
            } else if (iFocused == i && vEntries[i].fTimer < 0.2f && isEnabled()) {
                vEntries[i].fTimer += hge->Timer_GetDelta();
                if (vEntries[i].fTimer > 0.2f) vEntries[i].fTimer = 0.2f;
            } else if (iFocused != i && iSelectedID != i && vEntries[i].fTimer > 0.0f) {
                vEntries[i].fTimer -= hge->Timer_GetDelta();
                if (vEntries[i].fTimer < 0.0f) vEntries[i].fTimer = 0.0f;
            }

            if (vEntries[i].fTimer > 0.0f) {
                if (vEntries[i].fTimer <= 0.2f)
                    for (int j = 0; j < 9; ++j)
                        GV->hGfxInterface->sprButton[0][j]->SetBlendMode(BLEND_COLORADD);

                hge->Gfx_SetClipping(lx - (i == 0 ? 5 : 2), y, vEntries[i].GetWidth() + (i == 0 ? 2 : i == vEntries.size() - 1 ? 1 : 0), getHeight());
                But::drawButton(hGfx, 0, x, y, getWidth(), getHeight(), SETA(0xFFFFFF, vEntries[i].fTimer * 2.5f * alpha / (vEntries[i].fTimer <= 0.2f ? 7.f : 1.5f)));
                hge->Gfx_SetClipping();

                for (int j = 0; j < 9; ++j)
                    GV->hGfxInterface->sprButton[0][j]->SetBlendMode(BLEND_DEFAULT);
            }

            if (vEntries[i].sprIcon != 0) {
                int addOX = (i > 0 ? 1 : 0);
                vEntries[i].sprIcon->Render(lx + addOX, y + (getHeight() - vEntries[i].sprIcon->GetHeight()) / 2 - 1);
                lx += 17 + addOX;
            } else if (i > 0) {
                lx += 3;
            } else {
                lx += 1;
            }

            GV->fntMyriad16->SetColor(SETA(0xe1e1e1, isEnabled() ? 0xFF : 0x77));
            GV->fntMyriad16->Render(lx, y + getHeight() / 2 - 2, HGETEXT_LEFT | HGETEXT_MIDDLE, vEntries[i].strCaption.c_str(), 0);
            lx += GV->fntMyriad16->GetStringWidth(vEntries[i].strCaption.c_str());

            lx += i == 0 ? 8 : 7;
        }
    }

    void ComboBut::setSelectedEntryID(int i, bool bEvent) {
        iSelectedID = (i < int(vEntries.size()) ? i : -1);
        if (iSelectedID != -1 && vEntries[i].fTimer < 0.2f) {
            vEntries[i].fTimer = 0.201f;
        }
        if (bEvent)
            distributeActionEvent();
    }

    void ComboBut::adjustSize() {
        int w = 4;
        for (auto & vEntry : vEntries)
            w += vEntry.GetWidth();
        setWidth(w);
        setHeight(24);
    }

    void ComboBut::mousePressed(MouseEvent &mouseEvent) {
        if (iFocused != -1 && mouseEvent.getButton() == MouseEvent::LEFT) {
            mMousePressed = true;
            mouseEvent.consume();
            iSelectedID = iFocused;
            distributeActionEvent();
        }
    }

    void ComboBut::mouseExited(MouseEvent &mouseEvent) {
        mHasMouse = false;
        mMousePressed = false;
    }

    void ComboBut::mouseEntered(MouseEvent &mouseEvent) {
        mHasMouse = true;
    }

    void ComboBut::mouseReleased(MouseEvent &mouseEvent) {
        mMousePressed = false;
    }

    void ComboBut::focusGained(const FocusEvent& event) {
        mKeyboardFocus = event.isKeyboardFocus();
    }

    void ComboBut::focusLost(const FocusEvent& event) {
        mMousePressed = false;
        mKeyboardFocus = false;
    }

    void ComboBut::addEntry(ComboButEntry n) {
        n.iID = vEntries.size();
        vEntries.push_back(n);
        adjustSize();
        if (n.iID == 0) {
            n.fTimer = 0.4;
        }
    }

    void ComboBut::keyPressed(KeyEvent &keyEvent) {
        if(keyEvent.getType() == KeyEvent::PRESSED) {
            if (keyEvent.getKey() == Key::LEFT) {
                if (iSelectedID > 0) {
                    --iSelectedID;
                    distributeActionEvent();
                }
            } else if (keyEvent.getKey() == Key::RIGHT) {
                if (iSelectedID < vEntries.size() - 1) {
                    ++iSelectedID;
                    distributeActionEvent();
                }
            }
        }
    }
}
