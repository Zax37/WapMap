#include "hge.h"
#include "wTextField.h"
#include "../../WapMap/globals.h"
#include "../commonFunc.h"
#include "../../WapMap/cInterfaceSheet.h"

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
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        bool hasMouse = mx > dx && my > dy && mx < dx + getWidth() && my < dy + getHeight();
        UpdateTooltip(hasMouse);

        renderFrame(dx, dy, getWidth(), getHeight(), getAlpha(), 0);
        if (fFocusTimer > 0)
            renderFrame(dx, dy, getWidth(), getHeight(), (fFocusTimer * 5.0f * 255.0f) * getAlphaModifier(), 1);

        graphics->pushClipArea(gcn::Rectangle(2, 2, getWidth() - 4, getHeight() - 4));

        hgeFont *fnt = ((HGEImageFont *) getFont())->getHandleHGE();
        fnt->SetColor(SETA(0xe1e1e1, (isEnabled() ? 0xFF : 0x77) * getAlpha() / 255.f));
        fnt->Render(dx + 5 - mXScroll, ceil(dy + getHeight() / 2) - 1, HGETEXT_LEFT | HGETEXT_MIDDLE, mText.c_str(), true);

        if (mSelectionPosition != -1 && mSelectionPosition != mCaretPosition) {
            int starti, endi;
            if (mCaretPosition < mSelectionPosition) {
                starti = mCaretPosition;
                endi = mSelectionPosition;
            } else {
                starti = mSelectionPosition;
                endi = mCaretPosition;
            }
            int startX = dx + GV->fntMyriad16->GetStringWidth(mText.substr(0, starti).c_str(), false, true) - mXScroll + 5;
            int endX = dx + GV->fntMyriad16->GetStringWidth(mText.substr(0, endi).c_str(), false, true) - mXScroll + 5;
            SHR::SetQuad(&q, 0x553393e6, startX, dy + 4, endX, dy + getFont()->getHeight() + 1);
            hge->Gfx_RenderQuad(&q);
            hge->Gfx_RenderLine(startX, dy + 4, startX, dy + getFont()->getHeight() + 1, 0xFF3393e6);
            hge->Gfx_RenderLine(endX, dy + 4, endX, dy + getFont()->getHeight() + 1, 0xFF3393e6);
        } else if (isFocused()) {
            drawCaret(graphics, GV->fntMyriad16->GetStringWidth(mText.substr(0, mCaretPosition).c_str(), false, true) - mXScroll);
        }

        graphics->popClipArea();
    }

    void TextField::mouseMoved(MouseEvent &mouseEvent) {
        if (isEnabled()) {
            GV->SetCursor(TEXT);
        }
    }

    void TextField::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT) {
            mCaretPosition = getFont()->getStringIndexAt(mText, mouseEvent.getX() + mXScroll);
            mSelectionPosition = -1;
            fixScroll();
        }

        if (isEnabled()) {
            GV->SetCursor(TEXT);
        }
    }

    void TextField::mouseDragged(DragEvent &mouseEvent) {
        if (mText.length() != 0) {
            mSelectionPosition = getFont()->getStringIndexAt(mText, mouseEvent.getX() + mXScroll);
            fixScroll();
        }
        mouseEvent.consume();

        if (isEnabled()) {
            GV->SetCursor(TEXT);
        }
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

        if (key.getValue() == Key::LEFT) {
            if (keyEvent.isShiftPressed()) {
                if (mSelectionPosition > 0)
                    mSelectionPosition--;
                else if (mSelectionPosition == -1 && mCaretPosition != 0)
                    mSelectionPosition = mCaretPosition - 1;
            } else if (mCaretPosition > 0) {
                if (mSelectionPosition == -1) --mCaretPosition;
                else {
                    if (mSelectionPosition < mCaretPosition) mCaretPosition = mSelectionPosition;
                    mSelectionPosition = -1;
                }
            } else mSelectionPosition = -1;
        } else if (key.getValue() == Key::RIGHT) {
            if (keyEvent.isShiftPressed()) {
                if (mSelectionPosition < mText.length() && mSelectionPosition != -1)
                    mSelectionPosition++;
                else if (mSelectionPosition == -1 && mCaretPosition < mText.length())
                    mSelectionPosition = mCaretPosition + 1;
            } else if (mCaretPosition < mText.size()) {
                if (mSelectionPosition == -1) ++mCaretPosition;
                else {
                    if (mSelectionPosition > mCaretPosition) mCaretPosition = mSelectionPosition;
                    mSelectionPosition = -1;
                }
            } else mSelectionPosition = -1;
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
            if (keyEvent.isShiftPressed()) {
                mSelectionPosition = 0;
            } else {
                mCaretPosition = 0;
                mSelectionPosition = -1;
            }
        } else if (key.getValue() == Key::END) {
            if (keyEvent.isShiftPressed()) {
                mSelectionPosition = mText.size();
            } else {
                mCaretPosition = mText.size();
                mSelectionPosition = -1;
            }
        } else if (key.getValue() == 'v' && keyEvent.isControlPressed()) { //paste
            if (bSelection) deleteSelection();
            int pastepos = mCaretPosition;
            char *cb = SHR::GetClipboard();
            if (cb != NULL) {
                bool bAllow = mText.length() + strlen(cb) < iMaxLength;
                if (bNumerical) {
                    if (cb[0] == '-' && (!bAllowNegative || pastepos != 0)) bAllow = false;
                    else if (cb[0] < '0' || cb[0] > '9') bAllow = false;
                    else for (int i = 1; i < strlen(cb); i++) {
                        if (cb[i] < '0' || cb[i] > '9') {
                            bAllow = false;
                            break;
                        }
                    }
                }
                if (bAllow) {
                    char *nlfix = new char[strlen(cb) + 1];
                    int offset = 0;
                    for (int i = 0; i <= strlen(cb); i++) {
                        if (cb[i] == 13 && cb[i + 1] == 10) {
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
                setActionEventId("");
                distributeActionEvent();
            }
        } else if (key.getValue() == 'c' && keyEvent.isControlPressed() && bSelection) { //copy
            int start = (mSelectionPosition < mCaretPosition ? mSelectionPosition : mCaretPosition),
                end = (mSelectionPosition < mCaretPosition ? mCaretPosition : mSelectionPosition);
            SHR::SetClipboard(mText.substr(start, end - start).c_str());
        } else if (key.getValue() == 'x' && keyEvent.isControlPressed() && bSelection) { //cut
            int start = (mSelectionPosition < mCaretPosition ? mSelectionPosition : mCaretPosition),
                end = (mSelectionPosition < mCaretPosition ? mCaretPosition : mSelectionPosition);
            SHR::SetClipboard(mText.substr(start, end - start).c_str());
            deleteSelection();
            setActionEventId("");
            distributeActionEvent();
        } else if (key.getValue() == 'a' && keyEvent.isControlPressed()) { //select all
            mCaretPosition = 0;
            mSelectionPosition = mText.length();
        } else if (key.isCharacter() && key.getValue() != Key::TAB && !bNumerical ||
                   key.isNumber() ||
                   key.getValue() == '-' && bNumerical && (mCaretPosition == 0 || mSelectionPosition == 0) &&
                   bAllowNegative) {
            if (bSelection || mText.length() < iMaxLength) {
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

    void TextField::focusGained(const FocusEvent &event) {
    }

    void TextField::focusLost(const FocusEvent &event) {
        mSelectionPosition = -1;

        if (bNumerical && mText.empty()) {
            mText = "0";
        }
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
