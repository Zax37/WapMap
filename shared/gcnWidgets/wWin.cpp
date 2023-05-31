#include "wWin.h"
#include "guichan/exception.hpp"
#include "../../WapMap/globals.h"
#include "../../WapMap/cInterfaceSheet.h"

using namespace gcn;

extern HGE *hge;
extern cInterfaceSheet *_ghGfxInterface;

namespace SHR {
    Win::Win(guiParts *Parts)
            : mMoved(false) {
        setFrameSize(0);
        setPadding(2);
        setTitleBarHeight(16);
        setAlignment(Graphics::CENTER);
        addMouseListener(this);
        setMovable(true);
        setOpaque(0);
        hGfx = Parts;
        bClose = 0;
        bDrawShadow = 1;
        fCloseTimer = 0;
        mMouseOver = 0;
        bHide = 0;
        mAlphaMod = 1.0f;
    }

    Win::Win(guiParts *Parts, const std::string &caption)
            : mMoved(false) {
        setCaption(caption);
        setFrameSize(0);
        setPadding(2);
        setTitleBarHeight(16);
        setAlignment(Graphics::CENTER);
        addMouseListener(this);
        setMovable(true);
        setOpaque(0);
        hGfx = Parts;
        bClose = 0;
        bDrawShadow = 1;
        fCloseTimer = 0;
        mMouseOver = 0;
        bHide = 0;
        mAlphaMod = 1.0f;
    }

    Win::~Win() {
    }

    void Win::setPadding(unsigned int padding) {
        mPadding = padding;
    }

    unsigned int Win::getPadding() const {
        return mPadding;
    }

    void Win::setTitleBarHeight(unsigned int height) {
        mTitleBarHeight = height;
    }

    unsigned int Win::getTitleBarHeight() {
        return mTitleBarHeight;
    }

    void Win::setCaption(const std::string &caption) {
        mCaption = caption;
    }

    const std::string &Win::getCaption() const {
        return mCaption;
    }

    void Win::setAlignment(Graphics::Alignment alignment) {
        mAlignment = alignment;
    }

    Graphics::Alignment Win::getAlignment() const {
        return mAlignment;
    }

    void Win::draw(Graphics *graphics) {
        gcn::Rectangle d = getChildrenArea();

        if (!bHide && mAlphaMod < 1.0f) {
            mAlphaMod += hge->Timer_GetDelta() * 5.0f;
            if (mAlphaMod > 1.0f) mAlphaMod = 1.0f;
        } else if (bHide && (mAlphaMod > 0.0f || isVisible())) {
            mAlphaMod -= hge->Timer_GetDelta() * 5.0f;
            if (mAlphaMod <= 0.0f) {
                mAlphaMod = 0.0f;
                setVisible(0);
            }
        }

        ClipRectangle rect = graphics->getCurrentClipArea();
        int x, y;
        getAbsolutePosition(x, y);

        if (bClose) {
            float mx, my;
            hge->Input_GetMousePos(&mx, &my);
            bool bExitFocus = (mMouseOver && mx > x + getWidth() - 18 && my > y + 5 && mx < x + getWidth() - 4 && my < y + 19);
            if (bExitFocus && fCloseTimer < 0.2f) {
                fCloseTimer += hge->Timer_GetDelta();
                if (fCloseTimer > 0.2f) fCloseTimer = 0.2f;
            } else if (!bExitFocus && fCloseTimer > 0.0f) {
                fCloseTimer -= hge->Timer_GetDelta();
                if (fCloseTimer < 0.0f) fCloseTimer = 0;
            }
        }

        unsigned char alpha = getAlpha();

        graphics->setColor(gcn::Color(0x373737, alpha));
        graphics->fillRectangle(gcn::Rectangle(1, 0, getWidth() - 1, y + 22));

        graphics->setColor(gcn::Color(0x2f2f2f, alpha));
        graphics->fillRectangle(gcn::Rectangle(1, 22, getWidth() - 1, getHeight() - 1));

        graphics->setColor(gcn::Color(0x272727, alpha));
        graphics->drawLine(0, 23, getWidth(), 23);

        //upper bar

        /*quad.v[0].y = quad.v[1].y = y + 5;
        quad.v[2].y = quad.v[3].y = y + 22;
        quad.v[0].col = quad.v[1].col = SETA(0x373737, alpha);
        quad.v[2].col = quad.v[3].col = SETA(0x282828, alpha);
        hge->Gfx_RenderQuad(&quad);

        quad.v[0].col = quad.v[1].col = quad.v[2].col = quad.v[3].col = SETA(0x393939, alpha);
        quad.v[0].y = quad.v[1].y = y + 1;
        quad.v[2].y = quad.v[3].y = y + 5;
        quad.v[0].x = quad.v[3].x = x + 5;
        quad.v[1].x = quad.v[2].x = x + getWidth() - 5;
        hge->Gfx_RenderQuad(&quad);

        quad.v[0].col = quad.v[1].col = quad.v[2].col = quad.v[3].col = SETA(0x2f2f2f, alpha);
        quad.v[0].y = quad.v[1].y = y + getHeight() - 5;
        quad.v[2].y = quad.v[3].y = y + getHeight();
        hge->Gfx_RenderQuad(&quad);*/

        hge->Gfx_RenderLine(x + 1, y + 1, x + getWidth() - 1, y + 1, SETA(0x585858, alpha));
        hge->Gfx_RenderLine(x + 1, y + getHeight() - 1, x + getWidth() - 1, y + getHeight() - 1, SETA(0x2a2a2a, alpha));

        //hge->Gfx_RenderLine(x, y+1, x+getWidth(), y+1, GV->colOutline);
        hge->Gfx_RenderLine(x + 1, y + 1, x + 1, y + getHeight() - 1, SETA(0x565656, alpha));
        //hge->Gfx_RenderLine(x+1, y+getHeight(), x+getWidth(), y+getHeight(), GV->colOutline);
        hge->Gfx_RenderLine(x + getWidth(), y + 1, x + getWidth(), y + getHeight() - 1, SETA(0x565656, alpha));

        //if (bDrawShadow)
            //GV->RenderDropShadow(x, y, getWidth(), getHeight(), alpha);

        if (bClose) {
            unsigned char p = fCloseTimer * 5.0f * 125.0f + 130.0f * getAlphaModifier();
            GV->sprTabCloseButton->SetColor(SETA(0xFFFFFF, p));
            GV->sprTabCloseButton->Render(x + getWidth() - 15, y + 6);
            GV->sprTabCloseButton->SetColor(0xFFFFFFFF);
            //hGfx->sprIconClose->Render(x+getWidth()-22, y+2);
        }

        int textX;
        int textY;

        textY = ((int) getTitleBarHeight() - getFont()->getHeight()) / 2 + 3;

        switch (getAlignment()) {
            case Graphics::LEFT:
                textX = 4;
                break;
            case Graphics::CENTER:
                textX = getWidth() / 2;
                break;
            case Graphics::RIGHT:
                textX = getWidth() - 4;
                break;
            default:
                throw GCN_EXCEPTION("Unknown alignment.");
        }

        graphics->setColor(0xFFFFFF);
        graphics->setFont(getFont());
        //graphics->pushClipArea(gcn::Rectangle(0, 0, getWidth(), getTitleBarHeight() - 1));
        graphics->drawText(getCaption(), textX, textY, getAlignment());
        //graphics->popClipArea();

        hge->Gfx_SetClipping(rect.x, rect.y, rect.width, rect.height);
        drawChildren(graphics);

    }

    void Win::mouseEntered(MouseEvent &mouseEvent) {
        mMouseOver = 1;
    }

    void Win::mouseExited(MouseEvent &mouseEvent) {
        mMouseOver = 0;
    }

	bool Win::gotFocus()
	{
		for (gcn::Widget* child : mWidgets) {
			if (child->isFocused()) return true;
		}
		return false;
	}

    void Win::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.getSource() != this) {
            return;
        }

        if (getParent() != NULL) {
            getParent()->moveToTop(this);
        }

        if (bClose) {
            if (mouseEvent.getX() > getWidth() - 18 &&
                mouseEvent.getX() < getWidth() - 2 &&
                mouseEvent.getY() > 5 &&
                mouseEvent.getY() < 21) {
                setShow(0);
                distributeActionEvent();
            }
        }

        if (!bHide && mVisible) {
            mDragOffsetX = mouseEvent.getX();
            mDragOffsetY = mouseEvent.getY();

            mMoved = mouseEvent.getY() <= 22;
        }
    }

    void Win::mouseReleased(MouseEvent &mouseEvent) {
        mMoved = false;
    }

    void Win::mouseDragged(DragEvent &mouseEvent) {
        if (mouseEvent.isConsumed() || mouseEvent.getSource() != this || bHide) {
            return;
        }

        if (isMovable() && mMoved) {
            int parentX;
            int parentY;

            getParent()->getAbsolutePosition(parentX, parentY);

            int newX = mouseEvent.getX() - mDragOffsetX + getX();
            int newY = mouseEvent.getY() - mDragOffsetY + getY();

            if (parentX + newX < 0) newX = -parentX;
            if (parentY + newY < 0) newY = -parentY;
            if (parentX + newX + mDimension.width > GV->iScreenW) newX = GV->iScreenW - parentX - mDimension.width;
            if (parentY + newY + mDimension.height > GV->iScreenH) newY = GV->iScreenH - parentY - mDimension.height;

            setPosition(newX, newY);
        }

        mouseEvent.consume();
    }

    gcn::Rectangle Win::getChildrenArea() {
        return gcn::Rectangle(getPadding(),
                              getTitleBarHeight(),
                              getWidth() - getPadding() * 2,
                              getHeight() - getPadding() - getTitleBarHeight());
    }

    void Win::setMovable(bool movable) {
        mMovable = movable;
    }

    bool Win::isMovable() const {
        return mMovable;
    }

    void Win::setOpaque(bool opaque) {
        mOpaque = opaque;
    }

    bool Win::isOpaque() {
        return mOpaque;
    }

    void Win::resizeToContent() {
        WidgetListIterator it;

        int w = 0, h = 0;
        for (it = mWidgets.begin(); it != mWidgets.end(); it++) {
            if ((*it)->getX() + (*it)->getWidth() > w) {
                w = (*it)->getX() + (*it)->getWidth();
            }

            if ((*it)->getY() + (*it)->getHeight() > h) {
                h = (*it)->getY() + (*it)->getHeight();
            }
        }

        setSize(w + 2 * getPadding(), h + getPadding() + getTitleBarHeight());
    }
}
