#include "wContext.h"

#include "guichan/exception.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/key.hpp"
#include "guichan/mouseevent.hpp"
#include "guichan/mouseinput.hpp"
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
        bFocused = 0;
        hParent = 0;
        hCascade = 0;
        bEnabled = 1;
        fTimer = 0;
    }

    Context::Context(guiParts *Parts, hgeFont *font)
            : mHasMouse(false),
              mKeyPressed(false),
              mMousePressed(false) {
        //setFocusable(false);
        adjustSize();
        setFrameSize(0);

        addMouseListener(this);
        addFocusListener(this);
        hGfx = Parts;
        iSelected = -1;
        iSelectedIt = -1;
        hFont = font;
        iRowHeight = 18;
        addWidgetListener(this);
        mReserveIconSpace[0] = 0;
        mReserveIconSpace[1] = 1;
        fShowTimer = 0;
        bHide = 0;
    }

    void ContextModel::ClearElements() {
        for (int i = 0; i < vElements.size(); i++) {
            delete vElements[i];
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

        unsigned char galpha = (fShowTimer * 5.0f * 255.0f);
        float gtimer = fShowTimer * 5.0f;

        ClipRectangle rect = graphics->getCurrentClipArea();
        int x, y;
        getAbsolutePosition(x, y);

        hge->Gfx_SetClipping();

        GV->RenderDropShadow(x, y, getWidth(), getHeight());

        //hge->Gfx_SetClipping(rect.x, rect.y, rect.width, rect.height);

        hgeQuad q;
        q.blend = BLEND_DEFAULT;
        q.tex = 0;
        q.v[0].col = q.v[1].col = SETA(0xFF373737, galpha);
        q.v[2].col = q.v[3].col = SETA(0xFF282828, galpha);
        q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;
        q.v[0].x = x;
        q.v[0].y = y + 5;
        q.v[1].x = q.v[0].x + getWidth() - 1;
        q.v[1].y = q.v[0].y;
        q.v[2].x = q.v[1].x;
        q.v[2].y = q.v[0].y + getHeight() - 10;
        q.v[3].x = q.v[0].x;
        q.v[3].y = q.v[2].y;
        hge->Gfx_RenderQuad(&q);

        for (int i = 0; i < 6; i++)
            _ghGfxInterface->sprDropDown[i > 2][0][i % 3]->SetColor(SETA(0xFFFFFFFF, galpha));

        _ghGfxInterface->sprDropDown[0][0][0]->Render(x, y);
        _ghGfxInterface->sprDropDown[0][0][1]->RenderStretch(x + 5, y, x + getWidth() - 5, y + 5);
        _ghGfxInterface->sprDropDown[0][0][2]->Render(x + getWidth() - 5, y);

        _ghGfxInterface->sprDropDown[1][0][0]->Render(x, y + getHeight() - 5);
        _ghGfxInterface->sprDropDown[1][0][1]->RenderStretch(x + 5, y + getHeight() - 5, x + getWidth() - 5,
                                                             y + getHeight());
        _ghGfxInterface->sprDropDown[1][0][2]->Render(x + getWidth() - 5, y + getHeight() - 5);

        bool banyhasicon = mReserveIconSpace[0];
        for (int i = 0; i < vElements.size(); i++) {
            if (vElements[i]->GetIcon() != NULL) banyhasicon = 1;

            if (vElements[i]->IsEnabled() && iSelectedIt == i && vElements[i]->fTimer < 0.2f) {
                vElements[i]->fTimer += hge->Timer_GetDelta();
                if (vElements[i]->fTimer > 0.2f) vElements[i]->fTimer = 0.2f;
            } else if (iSelectedIt != i && vElements[i]->fTimer > 0.0f) {
                vElements[i]->fTimer -= hge->Timer_GetDelta();
                if (vElements[i]->fTimer < 0.0f) vElements[i]->fTimer = 0.0f;
            }
            //hge->Gfx_RenderLine(x, y+2+(i+1)*iRowHeight, x+getWidth(), y+2+(i+1)*iRowHeight, 0xFFFF0000);
        }

        for (int i = 0; i < vElements.size(); i++) {
            if (vElements[i]->fTimer > 0.0f) {
                bool first = (i == 0),
                        last = (i == vElements.size() - 1);
                unsigned char p = (vElements[i]->fTimer * 5.0f * 255.0f) * gtimer;
                hgeQuad q2;
                q2.blend = BLEND_DEFAULT;
                q2.tex = 0;
                q2.v[0].z = q2.v[1].z = q2.v[2].z = q2.v[3].z = 1;
                q2.v[0].col = q2.v[1].col = SETA((first ? 0xFF178ce1 : 0xFF1792e1), p);
                q2.v[2].col = q2.v[3].col = SETA((last ? 0xFF1478e2 : 0xFF1371e3), p);
                q2.v[0].x = x;
                q2.v[0].y = y + i * iRowHeight + 1 + 4 * first;
                q2.v[1].x = x + getWidth() - 1;
                q2.v[1].y = q2.v[0].y;
                q2.v[2].x = q2.v[1].x;
                q2.v[2].y = q2.v[0].y + iRowHeight + 1 - 3 * last - 4 * first;
                q2.v[3].x = q2.v[0].x;
                q2.v[3].y = q2.v[2].y;

                hge->Gfx_RenderQuad(&q2);
                //hge->Gfx_RenderLine(q2.v[0].x, q2.v[0].y-1, q2.v[1].x, q2.v[0].y-1, 0xFF45a9e7);
                if (!last) {
                    hge->Gfx_RenderLine(q2.v[0].x, q2.v[2].y + 1, q2.v[1].x, q2.v[2].y + 1, SETA(0xFF181818, p));
                    hge->Gfx_RenderLine(q2.v[0].x, q2.v[2].y + 2, q2.v[1].x, q2.v[2].y + 2, SETA(0xFF262626, p));
                }
                if (i == 0) {
                    for (int z = 0; z < 3; z++)
                        _ghGfxInterface->sprDropDown[0][1][z]->SetColor(SETA(0xFFFFFFFF, p));
                    _ghGfxInterface->sprDropDown[0][1][0]->Render(x, y);
                    _ghGfxInterface->sprDropDown[0][1][1]->RenderStretch(x + 5, y, x + getWidth() - 5, y + 5);
                    _ghGfxInterface->sprDropDown[0][1][2]->Render(x + getWidth() - 5, y);
                }
                if (i == vElements.size() - 1) {
                    for (int z = 0; z < 3; z++)
                        _ghGfxInterface->sprDropDown[1][1][z]->SetColor(SETA(0xFFFFFFFF, p));
                    _ghGfxInterface->sprDropDown[1][1][0]->Render(x, y + getHeight() - 5);
                    _ghGfxInterface->sprDropDown[1][1][1]->RenderStretch(x + 5, y + getHeight() - 5, x + getWidth() - 5,
                                                                         y + getHeight());
                    _ghGfxInterface->sprDropDown[1][1][2]->Render(x + getWidth() - 5, y + getHeight() - 5);
                }
            }

            int xoff = 5;
            if (vElements[i]->GetIcon() != NULL) {
                vElements[i]->GetIcon()->SetColor(SETA(vElements[i]->IsEnabled() ? 0xFFFFFFFF : 0x44FFFFFF, galpha));
                vElements[i]->GetIcon()->Render(x + 4, y + iRowHeight * i + 3);
                if (vElements[i]->fTimer > 0.0f) {
                    vElements[i]->GetIcon()->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
                    vElements[i]->GetIcon()->SetColor(
                            SETA(0xFFFFFFFF, (unsigned char) (vElements[i]->fTimer * 5.0f * 255.0f * gtimer)));
                    vElements[i]->GetIcon()->Render(x + 4, y + iRowHeight * i + 3);
                    vElements[i]->GetIcon()->SetBlendMode(BLEND_DEFAULT);
                }
                vElements[i]->GetIcon()->SetColor(0xFFFFFFFF);
            }
            if (vElements[i]->GetIcon(1) != NULL) {
                vElements[i]->GetIcon()->SetColor(SETA(vElements[i]->IsEnabled() ? 0xFFFFFFFF : 0x44FFFFFF, galpha));
                vElements[i]->GetIcon(1)->Render(x + getWidth() - 20, y + iRowHeight * i + 3);
                if (vElements[i]->fTimer > 0.0f) {
                    vElements[i]->GetIcon(1)->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
                    vElements[i]->GetIcon(1)->SetColor(
                            SETA(0xFFFFFFFF, (unsigned char) (vElements[i]->fTimer * 5.0f * 255.0f * gtimer)));
                    vElements[i]->GetIcon(1)->Render(x + getWidth() - 20, y + iRowHeight * i + 3);
                    vElements[i]->GetIcon(1)->SetBlendMode(BLEND_DEFAULT);
                }
                vElements[i]->GetIcon(1)->SetColor(0xFFFFFFFF);
            }
            if (banyhasicon)
                xoff = 20;
            if (vElements[i]->GetCascade() != 0) {
                GV->sprContextCascadeArrow->SetColor(SETA(0xFFFFFFFF, galpha));
                GV->sprContextCascadeArrow->Render(x + getWidth() - 13, y + iRowHeight * i + 4);
            }
            DWORD fontcol = SETA(0xFFb2b2b2, galpha);
            if (!vElements[i]->IsEnabled()) {
                fontcol = SETA(0xFF171717, galpha);
            } else if (vElements[i]->fTimer > 0.0f) {
                unsigned char byte = 178.0f + (vElements[i]->fTimer * 5.0f * 77.0f);
                fontcol = ARGB(galpha, byte, byte, byte);
            }
            hFont->SetColor(fontcol);
            int ifonty = y + iRowHeight * i + iRowHeight / 2 - hFont->GetHeight() / 2 + 2;
            if (vElements[i]->GetAlign() == HGETEXT_LEFT) {
                hFont->Render(x + xoff + 2, ifonty, HGETEXT_LEFT, vElements[i]->GetCaption(), 0);
            } else if (vElements[i]->GetAlign() == HGETEXT_CENTER)
                hFont->Render(x + xoff + (q.v[1].x - q.v[0].x - xoff) / 2, ifonty, HGETEXT_CENTER,
                              vElements[i]->GetCaption(), 0);
            else if (vElements[i]->GetAlign() == HGETEXT_RIGHT)
                hFont->Render(x + (q.v[1].x - q.v[0].x), ifonty, HGETEXT_RIGHT, vElements[i]->GetCaption(), 0);
        }
        //hge->Gfx_RenderLine(x+5, y, x+getWidth()-5, y, 0xFF5c5c5c);
        //hge->Gfx_RenderLine(x+5, y+getHeight(), x+getWidth()-5, y+getHeight(), 0xFF5c5c5c);
        hge->Gfx_RenderLine(x, y + 5, x, y + getHeight() - 5, SETA(0xFF5c5c5c, galpha));
        hge->Gfx_RenderLine(x + getWidth(), y + 5, x + getWidth(), y + getHeight() - 5, SETA(0xFF5c5c5c, galpha));
    }

    void Context::hide() {
        bHide = 1;
        fShowTimer = 0;
        setVisible(0);
    }

    void Context::adjustSize() {
        int w = 0;
        bool banyhasicon[2] = {0, 0};
        for (int i = 0; i < vElements.size(); i++) {
            int w2 = hFont->GetStringWidth(vElements[i]->GetCaption());
            if (vElements[i]->GetIcon() != NULL) banyhasicon[0] = 1;
            if (vElements[i]->GetIcon(1) != NULL) banyhasicon[1] = 1;
            if (vElements[i]->GetCascade() != 0)
                w2 += 12;
            w = std::max(w, w2);
        }
        for (int i = 0; i < 2; i++)
            if (banyhasicon[i] || mReserveIconSpace[i])
                w += 20;
        setWidth(w);
        setHeight(vElements.size() * iRowHeight + 4);
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
        if (n == iSelectedIt) return;
        if (iSelectedIt >= vElements.size()) iSelectedIt = -1;
        bool switchcascade = !(iSelectedIt != -1 && vElements[iSelectedIt]->GetCascade() == vElements[n]->GetCascade());
        if (iSelectedIt != -1) vElements[iSelectedIt]->SetFocused(0, switchcascade);
        vElements[n]->SetFocused(1, switchcascade);
        if (vElements[n]->GetCascade() != 0) {
            int destx = getX() + getWidth(), desty = getY() + iRowHeight * n + 1;
            vElements[n]->GetCascade()->setPosition(destx, desty);
            if (destx + vElements[n]->GetCascade()->getWidth() > hge->System_GetState(HGE_SCREENWIDTH))
                vElements[n]->GetCascade()->setX(destx - vElements[n]->GetCascade()->getWidth() - getWidth());
            if (desty + vElements[n]->GetCascade()->getHeight() > hge->System_GetState(HGE_SCREENHEIGHT))
                vElements[n]->GetCascade()->setY(desty - vElements[n]->GetCascade()->getHeight() + iRowHeight);
            vElements[n]->GetCascade()->requestMoveToTop();
            vElements[n]->GetCascade()->bHide = false;
        }
        bool dist = (iSelectedIt != n);
        iSelected = vElements[n]->GetID();
        iSelectedIt = n;
        if (dist)
            distributeValueChangedEvent();
    }

    void Context::mouseExited(MouseEvent &mouseEvent) {
        mHasMouse = false;
        if (vElements.size() > iSelectedIt) {
            if (iSelectedIt != -1 && vElements[iSelectedIt]->GetCascade() != 0) {
                int dx, dy, dw, dh;
                float mx, my;
                vElements[iSelectedIt]->GetCascade()->getAbsolutePosition(dx, dy);
                dw = vElements[iSelectedIt]->GetCascade()->getWidth();
                dh = vElements[iSelectedIt]->GetCascade()->getHeight();
                hge->Input_GetMousePos(&mx, &my);
                if (mx >= dx && mx < dx + dw && my >= dy && my < dy + dh)
                    return;
            }
            if (iSelectedIt != -1) {
                vElements[iSelectedIt]->SetFocused(0);
                distributeValueChangedEvent();
            }
        }
        iSelected = -1;
        iSelectedIt = -1;
    }

    void Context::mouseEntered(MouseEvent &mouseEvent) {
        mHasMouse = true;
    }

    bool Context::showHand() {
        bool enabled = 1;
        if (iSelectedIt != -1 && vElements.size() > iSelectedIt && !vElements[iSelectedIt]->IsEnabled())
            enabled = 0;
        return mHasMouse && enabled;
    }

    void Context::mouseReleased(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT
            && mMousePressed
            && mHasMouse) {
            mMousePressed = false;
            mouseEvent.consume();
            if (iSelectedIt != -1 && vElements[iSelectedIt]->IsEnabled()) {
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

    void Context::focusLost(const Event &event) {
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
        for (int i = 0; i < vElements.size(); i++) {
            delete vElements[i];
        }
        vElements.clear();
    }

    void Context::EmulateClickID(int id) {
        int oldsel = iSelected;

        iSelected = id;
        distributeActionEvent();

        iSelected = oldsel;
    }

    void ContextModel::AddElement(int id, const char *cap, hgeSprite *spr) {
        vElements.push_back(new ContextEl(id, cap, spr));
    }

    ContextEl *ContextModel::GetElementByID(int id) {
        for (int i = 0; i < vElements.size(); i++) {
            if (vElements[i]->GetID() == id)
                return vElements[i];
        }
        return 0;
    }

    ContextEl *Context::GetElementByID(int id) {
        for (int i = 0; i < vElements.size(); i++) {
            if (vElements[i]->GetID() == id)
                return vElements[i];
        }
        return 0;
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
        SetFocused(bFocused);
    }

    void ContextEl::SetFocused(bool b, bool switchcascade) {
        if (switchcascade && hCascade != 0) {
            hCascade->setVisible(b);
        }
        bFocused = b;
    }

    void Context::SetModel(ContextModel *m) {
        vElements = m->vElements;
        for (int i = 0; i < vElements.size(); i++)
            vElements[i]->SetParent(this);
    };

    void Context::widgetShown(const Event &event) {
        bHide = 0;
        if (iSelectedIt != -1 && iSelectedIt < vElements.size()) vElements[iSelectedIt]->SetFocused(0);
        iSelectedIt = -1;
        iSelected = -1;
    }

    void Context::widgetHidden(const Event &event) {
        if (!bHide) {
            mVisible = 1;
            bHide = 1;
            for (int i = 0; i < vElements.size(); i++) {
                if (vElements[i]->GetCascade() != 0)
                    vElements[i]->GetCascade()->setVisible(0);
            }
        }
        if (bHide && fShowTimer == 0.0f) {
            for (int i = 0; i < vElements.size(); i++)
                vElements[i]->fTimer = 0;
            bHide = 0;
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

}
