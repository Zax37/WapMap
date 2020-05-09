#include "wCheckbox.h"

#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/key.hpp"
#include "guichan/mouseinput.hpp"

#include <hgeSprite.h>
#include "../../WapMap/cInterfaceSheet.h"

extern HGE *hge;

namespace SHR {

    CBox::CBox() {
        setSelected(false);

        setFocusable(true);
        addMouseListener(this);
        addKeyListener(this);
        hGfx = NULL;
        fTimer = 0;
        bMouseOver = 0;
    }

    CBox::CBox(cInterfaceSheet *Parts, const std::string &caption, bool selected) {
        setCaption(caption);
        setSelected(selected);

        setFocusable(true);
        addMouseListener(this);
        addKeyListener(this);

        adjustSize();

        hGfx = Parts;
        fTimer = 0;
        bMouseOver = 0;
    }

    bool CBox::showHand() {
        return (isEnabled() && bMouseOver);
    }

    void CBox::draw(Graphics *graphics) {
        int x, y;
        getAbsolutePosition(x, y);
        int buty = y + getHeight() / 2 - 8;

        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        bool hl = (mx > x && mx < x + 16 && my > buty && my < buty + 16 && isVisible());
        if (hl && fTimer < 0.1f) {
            fTimer += hge->Timer_GetDelta();
            if (fTimer > 0.1f) fTimer = 0.1f;
        } else if (!hl && fTimer > 0.0f) {
            fTimer -= hge->Timer_GetDelta();
            if (fTimer < 0.0f) fTimer = 0.0f;
        }

        UpdateTooltip(hl);

        graphics->popClipArea();

        DWORD defcol = SETA(0xFFFFFFFF, getAlpha());

        if (isEnabled()) {
            hGfx->sprControl[1][2]->SetColor(defcol);
            hGfx->sprControl[1][2]->Render(x, buty);
            if (fTimer > 0.0f) {
                hGfx->sprControl[1][3]->SetColor(
                        SETA(0xFFFFFFFF, (unsigned char) (fTimer * 10.0f * 255.0f * getAlphaModifier())));
                hGfx->sprControl[1][3]->Render(x, buty);
            }
        } else {
            hGfx->sprControl[1][4]->SetColor(defcol);
            hGfx->sprControl[1][4]->Render(x, buty);
        }
        if (mSelected) {
            hGfx->sprControl[1][isEnabled() ? 0 : 1]->SetColor(defcol);
            hGfx->sprControl[1][isEnabled() ? 0 : 1]->Render(x, buty);
        }

        graphics->setFont(getFont());
        graphics->setColor(isEnabled() ? 0xa1a1a1 : 0x5e5e5e);

        graphics->pushClipArea(getDimension());
        graphics->drawText(getCaption(), 18, 0);
    }

    bool CBox::isSelected() const {
        return mSelected;
    }

    void CBox::setSelected(bool selected) {
        mSelected = selected;
    }

    const std::string &CBox::getCaption() const {
        return mCaption;
    }

    void CBox::setCaption(const std::string &caption) {
        mCaption = caption;
    }

    void CBox::keyPressed(KeyEvent &keyEvent) {
        Key key = keyEvent.getKey();

        if (key.getValue() == Key::ENTER ||
            key.getValue() == Key::SPACE) {
            toggleSelected();
            keyEvent.consume();
        }
    }

    void CBox::mouseClicked(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT) {
            toggleSelected();
        }
    }

    void CBox::mouseDragged(MouseEvent &mouseEvent) {
        mouseEvent.consume();
    }

    void CBox::adjustSize() {
        int height = getFont()->getHeight();

        setHeight(height < 16 ? 16 : height);
        setWidth(getFont()->getWidth(mCaption) + 18);
    }

    void CBox::toggleSelected() {
        mSelected = !mSelected;
        distributeActionEvent();
    }

    void CBox::mouseEntered(MouseEvent &mouseEvent) {
        bMouseOver = 1;
    }

    void CBox::mouseExited(MouseEvent &mouseEvent) {
        bMouseOver = 0;
    }
}

