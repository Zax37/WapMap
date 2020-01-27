#include "wButton.h"

#include "guichan/exception.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/key.hpp"
#include "guichan/mouseevent.hpp"
#include "guichan/mouseinput.hpp"
#include "../../WapMap/cInterfaceSheet.h"

extern HGE *hge;

namespace SHR {
    But::But(cInterfaceSheet *Parts)
            : mHasMouse(false),
              mKeyPressed(false),
              mMousePressed(false),
              mAlignment(Graphics::CENTER),
              mSpacing(4) {
        setFocusable(true);
        setFrameSize(0);

        addMouseListener(this);
        addKeyListener(this);
        addFocusListener(this);
        hGfx = Parts;
        sprIcon = NULL;
        colIcon = 0xFFFFFFFF;
        bRenderBG = 1;
        fTimer = 0;
        bBlinkState = 0;
        adjustSize();
    }

    But::But(cInterfaceSheet *Parts, const std::string &caption)
            : mCaption(caption),
              mHasMouse(false),
              mKeyPressed(false),
              mMousePressed(false),
              mAlignment(Graphics::CENTER),
              mSpacing(4) {
        setFocusable(true);
        setFrameSize(0);

        addMouseListener(this);
        addKeyListener(this);
        addFocusListener(this);
        hGfx = Parts;
        sprIcon = NULL;
        colIcon = 0xFFFFFFFF;
        bRenderBG = 1;
        fTimer = 0;
        bBlinkState = 0;
        adjustSize();
    }

    But::But(cInterfaceSheet *Parts, hgeSprite *psprIcon)
            : mHasMouse(false),
              mKeyPressed(false),
              mMousePressed(false),
              mAlignment(Graphics::CENTER),
              mSpacing(4) {
        setFocusable(true);
        setFrameSize(0);

        addMouseListener(this);
        addKeyListener(this);
        addFocusListener(this);
        hGfx = Parts;
        sprIcon = psprIcon;
        colIcon = 0xFFFFFFFF;
        bRenderBG = 1;
        fTimer = 0;
        bBlinkState = 0;
        adjustSize();
    }

    void But::setCaption(const std::string &caption) {
        mCaption = caption;
    }

    const std::string &But::getCaption() const {
        return mCaption;
    }

    void But::setAlignment(Graphics::Alignment alignment) {
        mAlignment = alignment;
    }

    Graphics::Alignment But::getAlignment() const {
        return mAlignment;
    }

    void But::setSpacing(unsigned int spacing) {
        mSpacing = spacing;
    }

    unsigned int But::getSpacing() const {
        return mSpacing;
    }

    void But::drawButton(cInterfaceSheet *hSheet, int iState, int iX, int iY, int iW, int iH, DWORD dwCol) {
        for (int i = 0; i < 9; i++)
            hSheet->sprButton[iState][i]->SetColor(dwCol);
        hSheet->sprButton[iState][0]->Render(iX, iY);
        hSheet->sprButton[iState][1]->RenderStretch(iX + 4, iY, iX + iW - 4, iY + 4);
        hSheet->sprButton[iState][2]->Render(iX + iW - 4, iY);

        hSheet->sprButton[iState][3]->RenderStretch(iX, iY + 4, iX + 4, iY + iH - 4);
        hSheet->sprButton[iState][4]->RenderStretch(iX + 4, iY + 4, iX + iW - 4, iY + iH - 4);
        hSheet->sprButton[iState][5]->RenderStretch(iX + iW - 4, iY + 4, iX + iW, iY + iH - 4);

        hSheet->sprButton[iState][6]->Render(iX, iY + iH - 4);
        hSheet->sprButton[iState][7]->RenderStretch(iX + 4, iY + iH - 4, iX + iW - 4, iY + iH);
        hSheet->sprButton[iState][8]->Render(iX + iW - 4, iY + iH - 4);
    }

    void But::draw(Graphics *graphics) {
        UpdateTooltip(mHasMouse);

        if ((isPressed() || bBlinkState) && fTimer < 0.3f) {
            fTimer += hge->Timer_GetDelta();
            if (fTimer > 0.3f) fTimer = 0.3f;
        } else if (!(isPressed() || bBlinkState) && fTimer > 0.2f) {
            fTimer -= hge->Timer_GetDelta();
            if (fTimer < 0.2f) fTimer = 0.2f;
        } else if (isEnabled() && (mHasMouse || isFocused()) && fTimer < 0.2f) {
            fTimer += hge->Timer_GetDelta();
            if (fTimer > 0.2f) fTimer = 0.2f;
        } else if ((!mHasMouse || !isEnabled()) && fTimer > 0.0f) {
            fTimer -= hge->Timer_GetDelta();
            if (fTimer < 0.0f) fTimer = 0.0f;
        } else if (isEnabled() && fTimer < 0.0f) {
            fTimer += hge->Timer_GetDelta();
            if (fTimer > 0.0f) fTimer = 0.0f;
        } else if (!isEnabled() && fTimer > -0.2f) {
            fTimer -= hge->Timer_GetDelta();
            if (fTimer < -0.2f) fTimer = -0.2f;
        }

        ClipRectangle rect = graphics->getCurrentClipArea();
        int x, y;
        getAbsolutePosition(x, y);

        if (bRenderBG) {
            int st = 2;
            if (fTimer == -0.2f) st = 0;
            else if ((isPressed() || bBlinkState) && fTimer == 0.3f) st = 3;
            //else if( fTimer == 0.2f ) st = 3;

            drawButton(hGfx, st, x, y, getWidth(), getHeight(), SETA(0xFFFFFFFF, getAlpha()));
            if (fTimer != 0) {
                unsigned char p = 0;
                if (fTimer > 0.2f)
                    p = (fTimer - 0.2f) * 10.0f * 125.0f + 130.0f * getAlphaModifier();
                else if (fTimer < 0.0f)
                    p = (fTimer * -1) * 5.0f * 255.0f * getAlphaModifier();
                else
                    p = fTimer * 5.0f * 125.0f * getAlphaModifier();
                drawButton(hGfx, (fTimer < 0 ? 0 : 3), x, y, getWidth(), getHeight(), SETA(0xFFFFFFFF, p));
            }
        }
        if (getCaption().length() > 0) {
            hge->Gfx_SetClipping(rect.x, rect.y, rect.width, rect.height);

            unsigned char p = 0;
            if (isEnabled() && fTimer >= 0) {
                if (fTimer > 0.2f)
                    p = (fTimer - 0.2f) * 10.0f * 55.0f + 200.0f;
                else
                    p = fTimer * 5.0f * 200.0f;
            } else {
                p = (fTimer * -1) * 5.0f * 45.0f;
            }
            graphics->setColor(RGB(p, p, p));

            int textX;
            int textY = getHeight() / 2 - getFont()->getHeight() / 2 - 2;

            int iconW = (sprIcon == 0 ? 0 : sprIcon->GetWidth());
            int textW = getFont()->getWidth(getCaption());
            int iconX = 0;

            switch (getAlignment()) {
                case Graphics::LEFT:
                    textX = mSpacing + iconW + 2 * (iconW > 0);
                    iconX = textX - iconW - 2;
                    break;
                case Graphics::CENTER:
                    textX = getWidth() / 2 + iconW / 2;
                    iconX = textX - textW / 2 - iconW - 2;
                    break;
                case Graphics::RIGHT:
                    textX = getWidth() - mSpacing;
                    iconX = textX - textW - 2 - iconW;
                    break;
                default:
                    throw GCN_EXCEPTION("Unknown alignment.");
            }

            if (sprIcon != 0) {
                int iconY = getHeight() / 2 - sprIcon->GetHeight() / 2;
                sprIcon->SetColor((!isEnabled() && !bRenderBG) ? 0x55FFFFFF : SETA(colIcon, getAlpha()));
                sprIcon->Render(x + iconX, y + iconY);
                if (fTimer > 0.0f && isEnabled()) {
                    sprIcon->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
                    unsigned char a = fTimer * 5.0f * 255.0f * getAlphaModifier();
                    sprIcon->SetColor(SETA(0xFFFFFFFF, a));
                    sprIcon->Render(x + iconX, y + iconY);
                    sprIcon->SetBlendMode(BLEND_DEFAULT);
                }
                sprIcon->SetColor(0xFFFFFFFF);
            }

            graphics->setFont(getFont());

            if (isPressed()) {
                graphics->drawText(getCaption(), textX + 1, textY + 1, getAlignment());
            } else {
                graphics->drawText(getCaption(), textX, textY, getAlignment());

                /*if (isFocused())
                {
                    graphics->drawRectangle(Rectangle(2, 2, getWidth() - 4,
                                                      getHeight() - 4));
                }*/
            }
        } else if (sprIcon != 0) {
            sprIcon->SetColor((!isEnabled() && !bRenderBG) ? 0x55FFFFFF : SETA(colIcon, getAlpha()));
            sprIcon->RenderStretch(x, y, x + getWidth(), y + getHeight());
            if (fTimer > 0.0f && isEnabled()) {
                sprIcon->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
                unsigned char a = fTimer * 5.0f * 255.0f * getAlphaModifier();
                sprIcon->SetColor(SETA(0xFFFFFFFF, a));
                sprIcon->RenderStretch(x, y, x + getWidth(), y + getHeight());
                sprIcon->SetBlendMode(BLEND_DEFAULT);
            }
            hge->Gfx_SetClipping(rect.x, rect.y, rect.width, rect.height);
            sprIcon->SetColor(0xFFFFFFFF);
        }
        hge->Gfx_SetClipping();
        RenderTooltip();
    }

    void But::adjustSize() {
        if (sprIcon != 0) {
            setWidth(sprIcon->GetWidth());
            setHeight(sprIcon->GetHeight());
        } else {
            setWidth(getFont()->getWidth(mCaption) + 2 * mSpacing);
            setHeight(getFont()->getHeight() + 2 * mSpacing);
        }
    }

    bool But::isPressed() const {
        if (mMousePressed) {
            return mHasMouse;
        } else {
            return mKeyPressed;
        }
    }

    void But::simulatePress() {
        distributeActionEvent();
    }

    void But::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT) {
            mMousePressed = true;
            mouseEvent.consume();
        }
    }

    void But::mouseExited(MouseEvent &mouseEvent) {
        mHasMouse = false;
    }

    void But::mouseEntered(MouseEvent &mouseEvent) {
        mHasMouse = true;
    }

    void But::mouseReleased(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT
            && mMousePressed
            && mHasMouse) {
            mMousePressed = false;
            distributeActionEvent();
            mouseEvent.consume();
        } else if (mouseEvent.getButton() == MouseEvent::LEFT) {
            mMousePressed = false;
            mouseEvent.consume();
        }
    }

    void But::mouseDragged(MouseEvent &mouseEvent) {
        mouseEvent.consume();
    }

    void But::keyPressed(KeyEvent &keyEvent) {
        Key key = keyEvent.getKey();

        if (key.getValue() == Key::ENTER
            || key.getValue() == Key::SPACE) {
            mKeyPressed = true;
            keyEvent.consume();
        }
    }

    void But::keyReleased(KeyEvent &keyEvent) {
        Key key = keyEvent.getKey();

        if ((key.getValue() == Key::ENTER
             || key.getValue() == Key::SPACE)
            && mKeyPressed) {
            mKeyPressed = false;
            distributeActionEvent();
            keyEvent.consume();
        }
    }

    void But::focusLost(const Event &event) {
        mMousePressed = false;
        mKeyPressed = false;
    }
}
