#include "wContext.h"
#include "guichan/exception.hpp"
#include "guichan/mouseevent.hpp"
#include "../commonFunc.h"
#include "../../WapMap/globals.h"
#include "../../WapMap/cInterfaceSheet.h"

extern HGE *hge;
extern cInterfaceSheet *_ghGfxInterface;

namespace SHR {
    ContextEl::ContextEl(int id, const char *cap, hgeSprite *spr) {
        iID = id;
        sprIcon[0] = spr;
        sprIcon[1] = NULL;
        szCaption = new char[strlen(cap) + 1];
        strcpy(szCaption, cap);
        iAlign = HGETEXT_LEFT;
        bFocused = false;
        hParent = 0;
        hCascade = 0;
        bEnabled = true;
        fTimer = 0;
    }

    Context::Context(guiParts *Parts, hgeFont *font)
            : mHasMouse(false),
              mKeyPressed(false),
              mMousePressed(false) {
        setFocusable(true);
        adjustSize();
        setFrameSize(0);

        addMouseListener(this);
        addFocusListener(this);
        hGfx = Parts;
        iSelectedID = -1;
        iSelected = -1;
        hFont = font;
        iRowHeight = 24;
        addWidgetListener(this);
        mReserveIconSpace[0] = false;
        mReserveIconSpace[1] = true;
        fShowTimer = 0;
        bHide = false;
    }

    void ContextModel::ClearElements() {
        for (auto & vElement : vElements) {
            delete vElement;
        }
        vElements.clear();
    }

    ContextModel::~ContextModel() {
        ClearElements();
    }

    void Context::draw(Graphics *graphics) {
        if (vElements.empty()) return;

        if (!bHide && fShowTimer < 0.2f) {
            fShowTimer += hge->Timer_GetDelta();
            if (fShowTimer > 0.2f) fShowTimer = 0.2f;
        } else if (bHide && fShowTimer > 0.0f) {
            fShowTimer -= hge->Timer_GetDelta();
            if (fShowTimer <= 0.0f) {
                fShowTimer = 0.0f;
                setVisible(0);
            }
        }

        unsigned char galpha = getAlpha();
        float gtimer = fShowTimer * 5.0f;

        int x, y;
        getAbsolutePosition(x, y);

        hge->Gfx_SetClipping();

        hgeQuad q;
        q.blend = BLEND_DEFAULT;
        q.tex = 0;
        SHR::SetQuad(&q, SETA(0x0F0F0F, galpha), x, y, x + getWidth() - 1, y + getHeight() - 1);
        hge->Gfx_RenderQuad(&q);

        bool anyIcon = mReserveIconSpace[0];
        auto separatorsIt = vSeparators.begin();
        for (int i = 0; i < vElements.size(); i++) {
            if (vElements[i]->GetIcon() != NULL) anyIcon = true;

            if (vElements[i]->IsEnabled() && iSelected == i && vElements[i]->fTimer < 0.2f) {
                vElements[i]->fTimer += hge->Timer_GetDelta();
                if (vElements[i]->fTimer > 0.2f) vElements[i]->fTimer = 0.2f;
            } else if (iSelected != i && vElements[i]->fTimer > 0.0f) {
                vElements[i]->fTimer -= hge->Timer_GetDelta();
                if (vElements[i]->fTimer < 0.0f) vElements[i]->fTimer = 0.0f;
            }
        }

        int yOff = y;
        int borderColor = 0x1d1d1d;

        for (int i = 0; i < vElements.size(); i++) {
            if (separatorsIt != vSeparators.end() && i == *separatorsIt) {
                ++separatorsIt;
                ++y;
                hge->Gfx_RenderLine(x, y, x + getWidth(), y, SETA(borderColor, galpha));
                ++y;
            }

            if (vElements[i]->fTimer > 0.0f) {
                unsigned char p = (vElements[i]->fTimer * 5.0f * 255.0f) * gtimer;
                SHR::SetQuad(&q, SETA(0x178ce1, p), x, y, x + getWidth() - 1, y + iRowHeight);
                hge->Gfx_RenderQuad(&q);
            }

            int xOff = 10;
            if (vElements[i]->GetIcon() != NULL) {
                vElements[i]->GetIcon()->SetColor(SETA(vElements[i]->IsEnabled() ? 0xFFFFFFFF : 0x44FFFFFF, galpha));
                vElements[i]->GetIcon()->Render(x + xOff, y + 4);
                if (vElements[i]->fTimer > 0.0f) {
                    vElements[i]->GetIcon()->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
                    vElements[i]->GetIcon()->SetColor(
                            SETA(0xFFFFFF, (unsigned char) (vElements[i]->fTimer * 5.0f * 255.0f * gtimer)));
                    vElements[i]->GetIcon()->Render(x + xOff, y + 4);
                    vElements[i]->GetIcon()->SetBlendMode(BLEND_DEFAULT);
                }
                vElements[i]->GetIcon()->SetColor(0xFFFFFFFF);
            }
            if (vElements[i]->GetIcon(1) != NULL) {
                vElements[i]->GetIcon()->SetColor(SETA(vElements[i]->IsEnabled() ? 0xFFFFFFFF : 0x44FFFFFF, galpha));
                vElements[i]->GetIcon(1)->Render(x + getWidth() - 25, y + 4);
                if (vElements[i]->fTimer > 0.0f) {
                    vElements[i]->GetIcon(1)->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
                    vElements[i]->GetIcon(1)->SetColor(
                            SETA(0xFFFFFF, (unsigned char) (vElements[i]->fTimer * 5.0f * 255.0f * gtimer)));
                    vElements[i]->GetIcon(1)->Render(x + getWidth() - 25, y + 4);
                    vElements[i]->GetIcon(1)->SetBlendMode(BLEND_DEFAULT);
                }
                vElements[i]->GetIcon(1)->SetColor(0xFFFFFFFF);
            }
            if (anyIcon)
                xOff += 25;
            if (vElements[i]->GetCascade() != 0) {
                GV->sprContextCascadeArrow->SetColor(SETA(0xFFFFFF, galpha));
                GV->sprContextCascadeArrow->Render(x + getWidth() - 20, y + 6);
            }
            DWORD fontcol = SETA(0xFFe1e1e1, galpha);
            if (!vElements[i]->IsEnabled()) {
                fontcol = SETA(0xFFe1e1e1, 0x77 * galpha / 255.f);
            }
            hFont->SetColor(fontcol);
            int textY = y + (iRowHeight - hFont->GetHeight()) / 2;
            if (vElements[i]->GetAlign() == HGETEXT_LEFT) {
                hFont->Render(x + xOff + 2, textY, HGETEXT_LEFT, vElements[i]->GetCaption(), 0);
            } else if (vElements[i]->GetAlign() == HGETEXT_CENTER)
                hFont->Render(x + xOff + (q.v[1].x - q.v[0].x - xOff) / 2, textY, HGETEXT_CENTER,
                              vElements[i]->GetCaption(), 0);
            else if (vElements[i]->GetAlign() == HGETEXT_RIGHT)
                hFont->Render(x + (q.v[1].x - q.v[0].x), textY, HGETEXT_RIGHT, vElements[i]->GetCaption(), 0);

            y += iRowHeight;
        }

        hge->Gfx_RenderLine(x + topLineXOffset, yOff, x + getWidth(), yOff, SETA(borderColor, galpha));
        hge->Gfx_RenderLine(x, y, x + getWidth(), y, SETA(borderColor, galpha));
        hge->Gfx_RenderLine(x, yOff, x, y, SETA(borderColor, galpha));
        hge->Gfx_RenderLine(x + getWidth(), yOff, x + getWidth(), y, SETA(borderColor, galpha));
    }

    void Context::hide() {
        bHide = true;
        fShowTimer = 0;
        setVisible(0);
    }

    void Context::adjustSize() {
        int w = 0;
        bool banyhasicon[2] = {0, 0};
        for (auto & vElement : vElements) {
            int w2 = hFont->GetStringWidth(vElement->GetCaption());
            if (vElement->GetIcon() != NULL) banyhasicon[0] = 1;
            if (vElement->GetIcon(1) != NULL) banyhasicon[1] = 1;
            if (vElement->GetCascade() != 0)
                w2 += 12;
            w = std::max(w, w2);
        }
        for (int i = 0; i < 2; i++)
            if (banyhasicon[i] || mReserveIconSpace[i])
                w += 20;
        setWidth(w + 40);
        setHeight(vElements.size() * iRowHeight + 1 + vSeparators.size() * 2);
    }

    void Context::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT) {
            mMousePressed = true;
            mouseEvent.consume();
        }
    }

    void Context::mouseMoved(MouseEvent &mouseEvent) {
        if (bHide) return;
        int n = mouseEvent.getY() / iRowHeight;
        if (n >= vElements.size()) n = vElements.size() - 1;
        if (n == iSelected) return;
        if (iSelected >= vElements.size()) iSelected = -1;
        bool switchcascade = !(iSelected != -1 && vElements[iSelected]->GetCascade() == vElements[n]->GetCascade());
        if (iSelected != -1) vElements[iSelected]->SetFocused(0, switchcascade);
        vElements[n]->SetFocused(1, switchcascade);
        OpenSubContext(n);
        bool dist = (iSelected != n);
        iSelectedID = vElements[n]->GetID();
        iSelected = n;
        if (dist)
            distributeValueChangedEvent();
    }

    void Context::mouseExited(MouseEvent &mouseEvent) {
        mHasMouse = false;
        if (vElements.size() > iSelected) {
            if (iSelected != -1 && vElements[iSelected]->GetCascade() != 0) {
                int dx, dy, dw, dh;
                float mx, my;
                vElements[iSelected]->GetCascade()->getAbsolutePosition(dx, dy);
                dw = vElements[iSelected]->GetCascade()->getWidth();
                dh = vElements[iSelected]->GetCascade()->getHeight();
                hge->Input_GetMousePos(&mx, &my);
                if (mx >= dx && mx < dx + dw && my >= dy && my < dy + dh)
                    return;
            }
            if (iSelected != -1) {
                vElements[iSelected]->SetFocused(0);
                distributeValueChangedEvent();
            }
        }
        iSelectedID = -1;
        iSelected = -1;
    }

    void Context::mouseEntered(MouseEvent &mouseEvent) {
        mHasMouse = true;
    }

    void Context::mouseReleased(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT
            && mMousePressed
            && mHasMouse) {
            mMousePressed = false;
            mouseEvent.consume();
            if (iSelected != -1 && vElements[iSelected]->IsEnabled()) {
                distributeActionEvent();
            }
        } else if (mouseEvent.getButton() == MouseEvent::LEFT) {
            mMousePressed = false;
            mouseEvent.consume();
        }
    }

    void Context::mouseDragged(MouseEvent &mouseEvent) {
        mouseEvent.consume();
    }

    void Context::focusLost(const FocusEvent &event) {
        mMousePressed = false;
        mKeyPressed = false;
        setVisible(0);
    }

    void Context::AddElement(int id, const char *cap, hgeSprite *spr, ContextEl *after) {
        if (after != NULL) {
            for (int i = 0; i < vElements.size(); i++) {
                if (vElements[i] == after) {
                    vElements.insert(vElements.begin() + i + 1, new ContextEl(id, cap, spr));
                    vElements[i + 1]->SetParent(this);
                    return;
                }
            }
        }
        vElements.push_back(new ContextEl(id, cap, spr));
        vElements.back()->SetParent(this);
    }

    void Context::RemoveElement(int i) {
        delete vElements[i];
        vElements.erase(vElements.begin() + i);
    }

    void Context::ClearElements() {
        for (auto & vElement : vElements) {
            delete vElement;
        }
        vElements.clear();
    }

    void Context::EmulateClickID(int id) {
        int selection = iSelectedID;

        iSelectedID = id;
        distributeActionEvent();

        iSelectedID = selection;
    }

    void ContextModel::AddElement(int id, const char *cap, hgeSprite *spr) {
        vElements.push_back(new ContextEl(id, cap, spr));
    }

    ContextEl *ContextModel::GetElementByID(int id) {
        for (auto & vElement : vElements) {
            if (vElement->GetID() == id)
                return vElement;
        }
        return 0;
    }

    ContextEl *Context::GetElementByID(int id) {
        for (auto & vElement : vElements) {
            if (vElement->GetID() == id)
                return vElement;
        }
        return 0;
    }

    ContextEl *Context::GetSelectedElement() {
        if (iSelected == -1) return 0;
        return vElements[iSelected];
    }

    void ContextEl::SetCaption(const char *cap) {
        delete[] szCaption;
        szCaption = new char[strlen(cap) + 1];
        strcpy(szCaption, cap);
    }

    void ContextEl::SetCascade(Context *con) {
        if (hCascade != NULL) {
            hCascade->setVisible(0);
        }
        hCascade = con;
        SetFocused(bFocused, false);
    }

    void ContextEl::SetFocused(bool b, bool switchcascade) {
        if (switchcascade && hCascade != 0) {
            hCascade->setVisible(b);
        }
        bFocused = b;
    }

    void Context::OpenSubContext(int i) {
        if (i < 0 || i > vElements.size() || !vElements[i]->GetCascade()) return;
        int destX = getX() + getWidth(), destY = getY() + iRowHeight * i;
        vElements[i]->GetCascade()->setPosition(destX, destY);
        if (destX + vElements[i]->GetCascade()->getWidth() > hge->System_GetState(HGE_SCREENWIDTH))
            vElements[i]->GetCascade()->setX(destX - vElements[i]->GetCascade()->getWidth() - getWidth());
        if (destY + vElements[i]->GetCascade()->getHeight() > hge->System_GetState(HGE_SCREENHEIGHT))
            vElements[i]->GetCascade()->setY(destY - vElements[i]->GetCascade()->getHeight() + iRowHeight);
        vElements[i]->GetCascade()->requestMoveToTop();
        vElements[i]->GetCascade()->bHide = false;
        vElements[i]->GetCascade()->setVisible(true);
    }

    void Context::SetModel(ContextModel *m) {
        vElements = m->vElements;
        for (auto & vElement : vElements)
            vElement->SetParent(this);
    };

    void Context::widgetShown(const Event &event) {
        bHide = false;
        if (iSelected != -1 && iSelected < vElements.size()) vElements[iSelected]->SetFocused(0);
        iSelected = -1;
        iSelectedID = -1;
    }

    void Context::widgetHidden(const Event &event) {
        if (!bHide) {
            mVisible = true;
            bHide = true;
            for (auto & vElement : vElements) {
                if (vElement->GetCascade() != 0)
                    vElement->GetCascade()->setVisible(false);
            }
        }
        if (bHide && fShowTimer == 0.0f) {
            for (auto & vElement : vElements)
                vElement->fTimer = 0;
            bHide = false;
            topLineXOffset = 0;
        }
    }

    void Context::addSelectionListener(SelectionListener *selectionListener) {
        mSelectionListeners.push_back(selectionListener);
    }

    void Context::removeSelectionListener(SelectionListener *selectionListener) {
        mSelectionListeners.remove(selectionListener);
    }

    void Context::distributeValueChangedEvent() {
        SelectionListenerIterator iter;

        for (iter = mSelectionListeners.begin(); iter != mSelectionListeners.end(); ++iter) {
            SelectionEvent event(this);
            (*iter)->valueChanged(event);
        }
    }

    void Context::SelectPrev() {
        if (iSelected <= 0) {
            iSelected = vElements.size();
        }

        while (!vElements[--iSelected]->IsEnabled()) {
            if (iSelected == 0) {
                iSelected = vElements.size();
            }
        }

        iSelectedID = vElements[iSelected]->GetID();
    }

    void Context::SelectNext() {
        if (iSelected >= vElements.size() - 1) {
            iSelected = -1;
        }

        while (!vElements[++iSelected]->IsEnabled()) {
            if (iSelected == vElements.size() - 1) {
                iSelected = -1;
            }
        }

        iSelectedID = vElements[iSelected]->GetID();
    }

    void Context::AddSeparator() {
        vSeparators.push_back(vElements.size());
    }

}
