#include "wCheckbox.h"

#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/key.hpp"

#include <hgeSprite.h>
#include "../../WapMap/cInterfaceSheet.h"

extern HGE *hge;

namespace SHR {

    CBox::CBox() {
        setSelected(false);

        setFocusable(true);
        addMouseListener(this);
        addKeyListener(this);
        addFocusListener(this);
        hGfx = NULL;
        fTimer = 0;
        bMouseOver = false;
        mKeyboardFocus = false;
    }

    CBox::CBox(cInterfaceSheet *Parts, const std::string &caption, bool selected) : CBox() {
        setCaption(caption);
        setSelected(selected);
        adjustSize();

        hGfx = Parts;
    }

    void CBox::draw(Graphics *graphics) {
        int x, y;
        getAbsolutePosition(x, y);
        int buttonY = y + getHeight() / 2 - 8;

        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        bool hl = (mx > x && mx < x + 16 && my > buttonY && my < buttonY + 16 && isVisible());
        if (hl && fTimer < 0.1f) {
            fTimer += hge->Timer_GetDelta();
            if (fTimer > 0.1f) fTimer = 0.1f;
        } else if (!hl && fTimer > 0.0f) {
            fTimer -= hge->Timer_GetDelta();
            if (fTimer < 0.0f) fTimer = 0.0f;
        }

        UpdateTooltip(hl);

        graphics->popClipArea();

        DWORD defcol = SETA(0xFFFFFF, getAlpha());

        if (isEnabled()) {
            hGfx->sprControl[1][2]->SetColor(defcol);
            hGfx->sprControl[1][2]->Render(x, buttonY);
            if (fTimer > 0.0f) {
                hGfx->sprControl[1][3]->SetColor(
                        SETA(0xFFFFFF, (unsigned char) (fTimer * 10.0f * 255.0f * getAlphaModifier())));
                hGfx->sprControl[1][3]->Render(x, buttonY);
            }

            if (mKeyboardFocus) {
                hGfx->sprControl[1][5]->Render(x - 1, buttonY - 1);
            }
        } else {
            hGfx->sprControl[1][4]->SetColor(defcol);
            hGfx->sprControl[1][4]->Render(x, buttonY);
        }
        if (mSelected) {
            hGfx->sprControl[1][isEnabled() ? 0 : 1]->SetColor(defcol);
            hGfx->sprControl[1][isEnabled() ? 0 : 1]->Render(x, buttonY);
        }

        graphics->setFont(getFont());
        graphics->setColor(gcn::Color(0xe1e1e1, isEnabled() ? 0xFF : 0x77));

        graphics->pushClipArea(getDimension());
        graphics->drawText(getCaption(), 20, 0);
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

    void CBox::mouseDragged(DragEvent &mouseEvent) {
        mouseEvent.consume();
    }

    void CBox::adjustSize() {
        int height = getFont()->getHeight();

        setHeight(height < 16 ? 16 : height);
        setWidth(getFont()->getWidth(mCaption) + 20);
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

    void CBox::focusGained(const FocusEvent &event) {
        mKeyboardFocus = event.isKeyboardFocus();
    }

    void CBox::focusLost(const FocusEvent &event) {
        mKeyboardFocus = false;
    }
}

