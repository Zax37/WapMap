
#include "hge.h"
#include "wTextField.h"

#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/key.hpp"
#include "guichan/mouseinput.hpp"

#ifdef WAP_MAP

#include "../../WapMap/globals.h"

#endif

#ifdef WAP_GRAPHIC
#include "../../WapGraphic/globals.h"
#endif

#include "../commonFunc.h"
#include "../../WapMap/cInterfaceSheet.h"

extern cInterfaceSheet *_ghGfxInterface;
extern HGE *hge;

namespace SHR {
    TextField::TextField() {
        mCaretPosition = 0;
        mXScroll = 0;

        setFocusable(true);

        addMouseListener(this);
        addKeyListener(this);
        addFocusListener(this);
        bNumerical = 0;
        bAllowNegative = 1;
        iMaxLength = 128;
        bMarkedInvalid = 0;
        mSelectionPosition = -1;
        fFocusTimer = 0;
    }

    TextField::TextField(const std::string &text) {
        mCaretPosition = 0;
        mXScroll = 0;

        mText = text;
        adjustSize();

        setFocusable(true);

        addMouseListener(this);
        addKeyListener(this);
        addFocusListener(this);
        bNumerical = 0;
        bAllowNegative = 1;
        iMaxLength = 128;
        bMarkedInvalid = 0;
        mSelectionPosition = -1;
        fFocusTimer = 0;
    }

    void TextField::setText(const std::string &text, bool bGenerateActionEvent) {
        if (text.size() < mCaretPosition) {
            mCaretPosition = text.size();
        }

        mText = text;
        mSelectionPosition = -1;
        if (bGenerateActionEvent) {
            setActionEventId("");
            distributeActionEvent();
        }
    }

    void TextField::renderFrame(int dx, int dy, int w, int h, unsigned char alpha, bool version) {
        for (int i = 0; i < 8; i++)
            _ghGfxInterface->sprTextField[version][i]->SetColor(SETA(0xFFFFFFFF, alpha));

        _ghGfxInterface->sprTextField[version][0]->Render(dx - 3, dy - 3);
        _ghGfxInterface->sprTextField[version][1]->RenderStretch(dx + 4, dy - 3, dx + w - 4, dy + 4);
        _ghGfxInterface->sprTextField[version][2]->Render(dx + w - 4, dy - 3);

        _ghGfxInterface->sprTextField[version][3]->RenderStretch(dx - 3, dy + 4, dx + 1, dy + h - 4);
        _ghGfxInterface->sprTextField[version][4]->RenderStretch(dx + w - 1, dy + 4, dx + w + 3, dy + h - 4);

        _ghGfxInterface->sprTextField[version][5]->Render(dx - 3, dy + h - 4);
        _ghGfxInterface->sprTextField[version][6]->RenderStretch(dx + 4, dy + h - 4, dx + w - 4, dy + h + 3);
        _ghGfxInterface->sprTextField[version][7]->Render(dx + w - 4, dy + h - 4);

        hgeQuad q;
        q.tex = 0;
        q.blend = BLEND_DEFAULT;
        for (int i = 0; i < 4; i++) q.v[i].z = 0;
        q.v[0].col = q.v[1].col = SETA(0xFF0b0b0b, alpha);
        q.v[2].col = q.v[3].col = SETA((version ? 0xFF141414 : 0xFF282828), alpha);
        q.v[0].x = dx + 1;
        q.v[0].y = dy + 4;
        q.v[1].x = dx + w - 1;
        q.v[1].y = dy + 4;
        q.v[2].x = dx + w - 1;
        q.v[2].y = dy + h - 4;
        q.v[3].x = dx + 1;
        q.v[3].y = dy + h - 4;
        hge->Gfx_RenderQuad(&q);
    }

    void TextField::draw(Graphics *graphics) {
        if (isFocused() && fFocusTimer < 0.2f) {
            fFocusTimer += hge->Timer_GetDelta();
            if (fFocusTimer > 0.2f) fFocusTimer = 0.2f;
        } else if (!isFocused() && fFocusTimer > 0.0f) {
            fFocusTimer -= hge->Timer_GetDelta();
            if (fFocusTimer < 0.0f) fFocusTimer = 0.0;
        }
        ClipRectangle rect = graphics->getCurrentClipArea();

        int dx, dy;
        getAbsolutePosition(dx, dy);
        //hge->Gfx_SetClipping();

        renderFrame(dx, dy, getWidth(), getHeight(), getAlpha(), 0);
        if (fFocusTimer > 0)
            renderFrame(dx, dy, getWidth(), getHeight(), (fFocusTimer * 5.0f * 255.0f) * getAlphaModifier(), 1);

        graphics->pushClipArea(gcn::Rectangle(2, 2, getWidth() - 4, getHeight() - 4));

        hgeFont *fnt = ((HGEImageFont *) getFont())->getHandleHGE();
        DWORD fntcol = (isEnabled() ? 0xFF8a8a8a : 0xFF414141);
        if (fFocusTimer > 0) {
            unsigned char p = (138.0f + 117.0f * fFocusTimer * 5.0f);
            fntcol = ARGB(getAlpha(), p, p, p);
        }
        fnt->SetColor(fntcol);
        fnt->Render(dx + 5 - mXScroll, dy + 1, HGETEXT_LEFT, mText.c_str(), 0);

        if (mSelectionPosition != -1 && mSelectionPosition != mCaretPosition) {
            hgeQuad q;
            q.tex = 0;
            q.blend = BLEND_DEFAULT;
            int starti, endi;
            if (mCaretPosition < mSelectionPosition) {
                starti = mCaretPosition;
                endi = mSelectionPosition;
            } else {
                starti = mSelectionPosition;
                endi = mCaretPosition;
            }
            int startx = dx + getFont()->getWidth(mText.substr(0, starti)) - mXScroll + 4;
            int endx = dx + getFont()->getWidth(mText.substr(0, endi)) - mXScroll + 4;
            SHR::SetQuad(&q, 0x553393e6, startx, dy + 3,
                         endx, dy + getFont()->getHeight() + 1);
            hge->Gfx_RenderQuad(&q);
            hge->Gfx_RenderLine(startx, dy + 3, startx, dy + getFont()->getHeight() + 1, 0xFF3393e6);
            hge->Gfx_RenderLine(endx, dy + 3, endx, dy + getFont()->getHeight() + 1, 0xFF3393e6);
        } else if (isFocused()) {
            drawCaret(graphics, getFont()->getWidth(mText.substr(0, mCaretPosition)) - mXScroll);
        }

        graphics->popClipArea();
    }

    void TextField::drawCaret(Graphics *graphics, int x) {
        // Check the current clip area as a clip area with a different
        // size than the widget might have been pushed (which is the
        // case in the draw method when we push a clip area after we have
        // drawn a border).
        const gcn::Rectangle clipArea = graphics->getCurrentClipArea();

        graphics->setColor(0x3393e6);
        graphics->drawLine(x + 2, clipArea.height - 2, x + 2, 1);
    }

    void TextField::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT) {
            mCaretPosition = getFont()->getStringIndexAt(mText, mouseEvent.getX() + mXScroll);
            mSelectionPosition = -1;
            fixScroll();
        }
    }

    void TextField::mouseDragged(MouseEvent &mouseEvent) {
        if (mText.length() != 0) {
            mSelectionPosition = getFont()->getStringIndexAt(mText, mouseEvent.getX() + mXScroll);
            fixScroll();
        }
        mouseEvent.consume();
    }

    void TextField::deleteSelection() {
        int min = (mCaretPosition < mSelectionPosition ? mCaretPosition : mSelectionPosition);
        int max = (mCaretPosition < mSelectionPosition ? mSelectionPosition : mCaretPosition);
        mText.erase(min, max - min);
        if (mCaretPosition == max)
            mCaretPosition -= max - min;
        mSelectionPosition = -1;
    }

    void TextField::keyPressed(KeyEvent &keyEvent) {
        Key key = keyEvent.getKey();
        bool bSelection = mSelectionPosition != -1 && mSelectionPosition != mCaretPosition;

        if (key.getValue() == Key::LEFT && mCaretPosition > 0) {
            if (keyEvent.isShiftPressed()) {
                if (mSelectionPosition > 0)
                    mSelectionPosition--;
                else if (mSelectionPosition == -1 && mCaretPosition != 0)
                    mSelectionPosition = mCaretPosition - 1;
            } else {
                --mCaretPosition;
                mSelectionPosition = -1;
            }
        } else if (key.getValue() == Key::RIGHT && mCaretPosition < mText.size()) {
            if (keyEvent.isShiftPressed()) {
                if (mSelectionPosition < mText.length() && mSelectionPosition != -1)
                    mSelectionPosition++;
                else if (mSelectionPosition == -1 && mCaretPosition < mText.length())
                    mSelectionPosition = mCaretPosition + 1;
            } else {
                ++mCaretPosition;
                mSelectionPosition = -1;
            }
        } else if (key.getValue() == Key::DELETE && (mCaretPosition < mText.size() || bSelection)) {
            if (bSelection) {
                deleteSelection();
            } else {
                mText.erase(mCaretPosition, 1);
            }
            setActionEventId("");
            distributeActionEvent();
        } else if (key.getValue() == Key::BACKSPACE && (mCaretPosition > 0 || mCaretPosition == 0 && bSelection)) {
            if (bSelection) {
                deleteSelection();
            } else {
                mText.erase(mCaretPosition - 1, 1);
                --mCaretPosition;
            }
            setActionEventId("");
            distributeActionEvent();
        } else if (key.getValue() == Key::ENTER) {
            setActionEventId("ENTER");
            distributeActionEvent();
        } else if (key.getValue() == Key::HOME) {
            mCaretPosition = 0;
            mSelectionPosition = -1;
        } else if (key.getValue() == Key::END) {
            mCaretPosition = mText.size();
            mSelectionPosition = -1;
        }

            /*else if( hge->Input_GetKeyState(HGEK_CTRL) ){
             if( key.getValue() == 'v' || key.getValue() == 'x' || key.getValue() == 'c' ){
              //printf("op\n");
             }
             //printf("wklej!\n");
            }*/
        else if (key.getValue() == 22 && keyEvent.isControlPressed()) { //paste
            if (bSelection) deleteSelection();
            int pastepos = mCaretPosition;
            char *cb = SHR::GetClipboard();
            if (cb != NULL) {
                bool bAllow = mText.length() + strlen(cb) < iMaxLength;
                if (bNumerical) {
                    for (int i = 0; i < strlen(cb); i++) {
                        if (cb[i] < 48 && cb[i] != 45 || cb[i] > 57 ||
                            (cb[i] == 45 && (i != 0 || pastepos != 0) || !bAllowNegative))
                            bAllow = 0;
                    }
                }
                if (bAllow) {
                    char *nlfix = new char[strlen(cb) + 1];
                    int offset = 0;
                    for (int i = 0; i <= strlen(cb); i++) {
                        if (cb[i] != 0 && cb[i] == 13 && cb[i + 1] == 10) {
                            i += 2;
                            offset += 2;
                        }
                        if (i <= strlen(cb))
                            nlfix[i - offset] = cb[i];
                    }
                    mText.insert(pastepos, nlfix);
                    mCaretPosition += strlen(nlfix);
                    delete[] nlfix;
                }
                delete[] cb;
            }
        } else if (key.getValue() == 3 && keyEvent.isControlPressed() && bSelection) { //copy
            int start = (mSelectionPosition < mCaretPosition ? mSelectionPosition : mCaretPosition),
                    end = (mSelectionPosition < mCaretPosition ? mCaretPosition : mSelectionPosition);
            SHR::SetClipboard(mText.substr(start, end - start).c_str());
        } else if (key.getValue() == 24 && keyEvent.isControlPressed() && bSelection) { //cut
            int start = (mSelectionPosition < mCaretPosition ? mSelectionPosition : mCaretPosition),
                    end = (mSelectionPosition < mCaretPosition ? mCaretPosition : mSelectionPosition);
            SHR::SetClipboard(mText.substr(start, end - start).c_str());
            deleteSelection();
        } else if (key.getValue() == 1 && keyEvent.isControlPressed()) { //select all
            mCaretPosition = 0;
            mSelectionPosition = mText.length();
        } else if (key.isCharacter() && key.getValue() != Key::TAB && !bNumerical ||
                   key.isNumber() ||
                   key.getValue() == '-' && bNumerical && (mCaretPosition == 0 || mSelectionPosition == 0) &&
                   bAllowNegative) {
            if (mText.length() < iMaxLength) {
                if (bSelection) {
                    int caretpos = (mSelectionPosition < mCaretPosition ? mSelectionPosition : mCaretPosition);
                    deleteSelection();
                    mCaretPosition = caretpos;
                }
                mText.insert(mCaretPosition, std::string(1, (char) key.getValue()));
                ++mCaretPosition;
                setActionEventId("");
                distributeActionEvent();
            }
        }

        if (key.getValue() != Key::TAB) {
            keyEvent.consume();
        }

        fixScroll();
    }

    void TextField::adjustSize() {
        setWidth(getFont()->getWidth(mText) + 7);
        adjustHeight();

        fixScroll();
    }

    void TextField::adjustHeight() {
        setHeight(getFont()->getHeight() + 4);
    }

    void TextField::focusGained(const Event &event) {
    }

    void TextField::focusLost(const Event &event) {
        mSelectionPosition = -1;
    }

    void TextField::fixScroll() {
        if (isFocused()) {
            int caretX = getFont()->getWidth(mText.substr(0, mCaretPosition));

            if (caretX - mXScroll >= getWidth() - 4) {
                mXScroll = caretX - getWidth() + 4;
            } else if (caretX - mXScroll <= 0) {
                mXScroll = caretX - getWidth() / 2;

                if (mXScroll < 0) {
                    mXScroll = 0;
                }
            }
        }
    }

    void TextField::setCaretPosition(unsigned int position) {
        if (position > mText.size()) {
            mCaretPosition = mText.size();
        } else {
            mCaretPosition = position;
        }

        fixScroll();
    }

    unsigned int TextField::getCaretPosition() const {
        return mCaretPosition;
    }

    const std::string &TextField::getText() const {
        return mText;
    }

    void TextField::fontChanged() {
        fixScroll();
    }

    void TextField::setMaxLength(int n) {
        iMaxLength = n;
    }
}
