#include "wLink.h"
#include "guichan/exception.hpp"
#include "guichan/graphics.hpp"
#include <hgefont.h>
#include "guichan/hge/hgeimagefont.hpp"
#include <hge.h>
#include <hgesprite.h>

extern HGE *hge;

namespace SHR {
    Link::Link() {
        mAlignment = Graphics::LEFT;
        bMouseOver = mMousePressed = false;
        sprIcon = nullptr;
        fFocusTimer = 0;
    }

    Link::Link(const std::string &caption, hgeSprite *spr) {
        mCaption = caption;
        mAlignment = Graphics::LEFT;

        sprIcon = spr;

        adjustSize();
        bMouseOver = mMousePressed = false;
        addMouseListener(this);
        fFocusTimer = 0;
    }

    const std::string &Link::getCaption() const {
        return mCaption;
    }

    void Link::setCaption(const std::string &caption) {
        mCaption = caption;
    }

    void Link::setAlignment(Graphics::Alignment alignment) {
        mAlignment = alignment;
    }

    Graphics::Alignment Link::getAlignment() const {
        return mAlignment;
    }

    void Link::draw(Graphics *graphics) {
        if (bMouseOver && fFocusTimer < 0.2f) {
            fFocusTimer += hge->Timer_GetDelta();
            if (fFocusTimer > 0.2f) fFocusTimer = 0.2f;
        } else if (!bMouseOver && fFocusTimer > 0.0f) {
            fFocusTimer -= hge->Timer_GetDelta();
            if (fFocusTimer < 0.0f) fFocusTimer = 0.0f;
        }

        int textX;
        int textY = getHeight() / 2 - getFont()->getHeight() / 2;

        switch (getAlignment()) {
            case Graphics::LEFT:
                textX = 0;
                break;
            case Graphics::CENTER:
                textX = getWidth() / 2;
                break;
            case Graphics::RIGHT:
                textX = getWidth();
                break;
            default:
                throw GCN_EXCEPTION("Unknown alignment.");
        }

        int dx, dy;
        getAbsolutePosition(dx, dy);

        hgeFont *fnt = ((HGEImageFont *) getFont())->getHandleHGE();

        unsigned char p = 161.0f + (fFocusTimer * 5.0f * 94.0f);

        fnt->SetColor(ARGB(255, p, p, p));

        int textx = (sprIcon == nullptr ? 0 : sprIcon->GetWidth() + 6);

        if (sprIcon != 0) {
            sprIcon->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
            sprIcon->SetColor(SETA(0xFFFFFF, p));
            sprIcon->Render(dx, dy);
            sprIcon->SetBlendMode(BLEND_DEFAULT);
        }

        fnt->Render(dx + textX + textx, dy + textY, HGETEXT_LEFT, getCaption().c_str(), 0);
    }

    void Link::adjustSize() {
        setWidth(getFont()->getWidth(getCaption()) + (sprIcon != 0 ? sprIcon->GetWidth() + 6 : 0));
        setHeight(getFont()->getHeight());
        if (sprIcon != nullptr) {
            if (sprIcon->GetHeight() > getHeight())
                setHeight(sprIcon->GetHeight());
        }
    }

    void Link::mouseExited(MouseEvent &mouseEvent) {
        bMouseOver = false;
    }

    void Link::mouseEntered(MouseEvent &mouseEvent) {
        bMouseOver = true;
    }

    void Link::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT) {
            mMousePressed = true;
            mouseEvent.consume();
        }
    }

    void Link::mouseReleased(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT
            && mMousePressed
            && bMouseOver) {
            mMousePressed = false;
            distributeActionEvent();
            mouseEvent.consume();
        } else if (mouseEvent.getButton() == MouseEvent::LEFT) {
            mMousePressed = false;
            mouseEvent.consume();
        }
    }

    void Link::mouseDragged(MouseEvent &mouseEvent) {
        mouseEvent.consume();
    }
}
