#include "wTextDropDown.h"

#include "guichan/exception.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/key.hpp"
#include "guichan/listmodel.hpp"
#include "guichan/mouseinput.hpp"
#include "wListbox.h"
#include "wScrollArea.h"
#include "../../WapMap/cInterfaceSheet.h"
#include "../commonFunc.h"

#ifdef WAP_MAP

#include "../../WapMap/globals.h"
#include "../../WapMap/states/editing_ww.h"

#endif


extern HGE *hge;
extern cInterfaceSheet *_ghGfxInterface;

namespace SHR {
    TextDropDown::TextDropDown(std::string defText,
                               gcn::ListModel *listModel,
                               SHR::ScrollArea *scrollArea,
                               SHR::ListBox *listBox) {
        bTextFocused = 0;
        mHasMouse = 0;
        fFocusTimer = 0;
        fButtonTimer = 0;
        mText = defText;
        mScrollDisabled = 0;
        setWidth(100);
        setFocusable(true);
        mDroppedDown = false;
        mPushed = false;
        mIsDragged = false;
        mCaretPosition = 0;
        mXScroll = 0;
        bMarkedInvalid = 0;

        setInternalFocusHandler(&mInternalFocusHandler);

        mInternalScrollArea = (scrollArea == NULL);
        mInternalListBox = (listBox == NULL);

        if (mInternalScrollArea) {
            mScrollArea = new ScrollArea();
        } else {
            mScrollArea = scrollArea;
        }

        if (mInternalListBox) {
            mListBox = new ListBox();
        } else {
            mListBox = listBox;
        }

        mScrollArea->setContent(mListBox);
        add(mScrollArea);

        mListBox->addActionListener(this);
        mListBox->addSelectionListener(this);

        setListModel(listModel);

        if (mListBox->getSelected() < 0) {
            mListBox->setSelected(0);
        }

        addMouseListener(this);
        addKeyListener(this);
        addFocusListener(this);

        adjustHeight();
    }

    TextDropDown::~TextDropDown() {
        if (widgetExists(mListBox)) {
            mListBox->removeActionListener(this);
            mListBox->removeSelectionListener(this);
        }

        if (mInternalScrollArea) {
            delete mScrollArea;
        }

        if (mInternalListBox) {
            delete mListBox;
        }

        setInternalFocusHandler(NULL);
    }

    void TextDropDown::draw(Graphics *graphics) {
        int h = (mDroppedDown ? mFoldedUpHeight : getHeight());
        int x, y;
        getAbsolutePosition(x, y);
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        bool buttonfoc = (isEnabled() && mHasMouse && mx > x + getWidth() - 20 && mx < x + getWidth() && my > y &&
                          my < y + h);

        if (mDroppedDown && fButtonTimer < 0.4f) {
            fButtonTimer += hge->Timer_GetDelta();
            if (fButtonTimer > 0.4f) fButtonTimer = 0.4f;
        } else if (!mDroppedDown && fButtonTimer > 0.2f) {
            fButtonTimer -= hge->Timer_GetDelta();
            if (fButtonTimer < 0.2f) fButtonTimer = 0.2f;
        } else if (buttonfoc && fButtonTimer < 0.2f) {
            fButtonTimer += hge->Timer_GetDelta();
            if (fButtonTimer > 0.2f) fButtonTimer = 0.2f;
        } else if (!buttonfoc && fButtonTimer > 0.0f) {
            fButtonTimer -= hge->Timer_GetDelta();
            if (fButtonTimer < 0.0f) fButtonTimer = 0.0f;
        }
        if (isFocused() && bTextFocused && fFocusTimer < 0.2f) {
            fFocusTimer += hge->Timer_GetDelta();
            if (fFocusTimer > 0.2f) fFocusTimer = 0.2f;
        } else if ((!isFocused() || !bTextFocused) && fFocusTimer > 0.0f) {
            fFocusTimer -= hge->Timer_GetDelta();
            if (fFocusTimer < 0.0f) fFocusTimer = 0.0;
        }

        ClipRectangle rect = graphics->getCurrentClipArea();

        hge->Gfx_SetClipping(x - 5, y - 5, getWidth() - 16, h + 10);
        TextField::renderFrame(x, y, getWidth(), h, getAlpha(), 0);
        if (fFocusTimer > 0)
            TextField::renderFrame(x, y, getWidth(), h, (fFocusTimer * 5.0f * 255.0f) * getAlphaModifier(), 1);
        hge->Gfx_SetClipping(rect.x, rect.y, rect.width, rect.height);

        _ghGfxInterface->sprDropDownBar[2]->Render(x + getWidth() - 21, y);
        if (fButtonTimer > 0.0f) {
            _ghGfxInterface->sprDropDownButtonFocused->SetColor(
                    SETA(0xFFFFFFFF, (unsigned char) (fButtonTimer * 2.5f * 255.0f)));
            _ghGfxInterface->sprDropDownButtonFocused->Render(x + getWidth() - 21, y);
        }

        _ghGfxInterface->sprDropDownBarArrow->SetColor(isEnabled() ? 0xFFFFFFFF : 0xFF5e5e5e);
        _ghGfxInterface->sprDropDownBarArrow->SetFlip(0, mDroppedDown);
        _ghGfxInterface->sprDropDownBarArrow->Render(x + getWidth() - 16, y + 7);

        graphics->pushClipArea(gcn::Rectangle(4, 0, getWidth() - 23, getHeight()));

        hgeFont *fnt = ((HGEImageFont *) getFont())->getHandleHGE();
        DWORD fntcol = (isEnabled() ? 0xFF8a8a8a : 0xFF414141);
        if (fFocusTimer > 0) {
            unsigned char p = (138.0f + 117.0f * fFocusTimer * 5.0f);
            fntcol = ARGB(getAlpha(), p, p, p);
        }
        fnt->SetColor(fntcol);
        fnt->Render(x + 5 - mXScroll, y + 1, HGETEXT_LEFT, mText.c_str(), 0);

        if (bTextFocused && mSelectionPosition != -1 && mSelectionPosition != mCaretPosition) {
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
            int startx = x + getFont()->getWidth(mText.substr(0, starti)) - mXScroll + 4;
            int endx = x + getFont()->getWidth(mText.substr(0, endi)) - mXScroll + 4;
            SHR::SetQuad(&q, 0x553393e6, startx, y + 3,
                         endx, y + getFont()->getHeight() + 1);
            hge->Gfx_RenderQuad(&q);
            hge->Gfx_RenderLine(startx, y + 3, startx, y + getFont()->getHeight() + 1, 0xFF3393e6);
            hge->Gfx_RenderLine(endx, y + 3, endx, y + getFont()->getHeight() + 1, 0xFF3393e6);
        } else if (bTextFocused && mCaretPosition != -1) {
            ClipRectangle clipArea = graphics->getCurrentClipArea();
            graphics->setColor(0x3393e6);
            graphics->drawLine(getFont()->getWidth(mText.substr(0, mCaretPosition)) - mXScroll, clipArea.height - 2,
                               getFont()->getWidth(mText.substr(0, mCaretPosition)) - mXScroll, 1);
        }
        graphics->popClipArea();

        if (mDroppedDown) {
            // Draw a border around the children.
            graphics->setColor(0x000000);
            graphics->drawRectangle(gcn::Rectangle(0,
                                                   mFoldedUpHeight,
                                                   getWidth(),
                                                   getHeight() - mFoldedUpHeight));
            drawChildren(graphics);
        }
    }

    void TextDropDown::setSelected(int selected) {
        if (selected >= 0) {
            mListBox->setSelected(selected);
        }
    }

    void TextDropDown::keyPressed(KeyEvent &keyEvent) {
        bool bSelection = mSelectionPosition != -1 && mSelectionPosition != mCaretPosition;
        Key key = keyEvent.getKey();
        if (mCaretPosition < 0) {
            keyEvent.consume();
            return;
        }

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
        }

            /*else if (key.getValue() == Key::ENTER)
            {
                distributeActionEvent();
            }*/

        else if (key.getValue() == Key::HOME) {
            mCaretPosition = 0;
            mSelectionPosition = -1;
        } else if (key.getValue() == Key::END) {
            mCaretPosition = mText.size();
            mSelectionPosition = -1;
        } else if (key.getValue() == 22 && keyEvent.isControlPressed()) { //paste
            if (bSelection) deleteSelection();
            int pastepos = mCaretPosition;
            char *cb = SHR::GetClipboard();
            if (cb != NULL) {
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
        } else if (key.isCharacter() && key.getValue() != Key::TAB ||
                   key.isNumber()) {
            if (bSelection) {
                int caretpos = (mSelectionPosition < mCaretPosition ? mSelectionPosition : mCaretPosition);
                deleteSelection();
                mCaretPosition = caretpos;
            }
            mText.insert(mCaretPosition, std::string(1, (char) key.getValue()));
            ++mCaretPosition;
            distributeActionEvent();
        }

        if (key.getValue() != Key::TAB) {
            keyEvent.consume();
        }

        fixScroll();
    }

    void TextDropDown::mousePressed(MouseEvent &mouseEvent) {
        // If we have a mouse press on the widget.
        if (0 <= mouseEvent.getY()
            && mouseEvent.getY() < getHeight()
            && mouseEvent.getX() >= 0
            && mouseEvent.getX() < getWidth()
            && mouseEvent.getButton() == MouseEvent::LEFT
            && !mDroppedDown
            && mouseEvent.getSource() == this) {
            if (mouseEvent.getX() < getWidth() - 22) {
                mCaretPosition = getFont()->getStringIndexAt(mText, mouseEvent.getX() + mXScroll);
                fixScroll();
                bTextFocused = 1;
            } else {
                bTextFocused = 0;
                mCaretPosition = -1;
                mPushed = true;
                dropDown();
                requestModalMouseInputFocus();
            }
            mSelectionPosition = -1;
        }
            // Fold up the listbox if the upper part is clicked after fold down
        else if (0 <= mouseEvent.getY()
                 && mouseEvent.getY() < mFoldedUpHeight
                 && mouseEvent.getX() >= 0
                 && mouseEvent.getX() < getWidth()
                 && mouseEvent.getButton() == MouseEvent::LEFT
                 && mDroppedDown
                 && mouseEvent.getSource() == this) {
            if (mouseEvent.getX() < getWidth() - 22) bTextFocused = 1;
            mPushed = false;
            foldUp();
            releaseModalMouseInputFocus();
        }
            // If we have a mouse press outside the widget
        else if (0 > mouseEvent.getY()
                 || mouseEvent.getY() >= getHeight()
                 || mouseEvent.getX() < 0
                 || mouseEvent.getX() >= getWidth()) {
            mPushed = false;
            foldUp();
            bTextFocused = 0;
        }
    }

    void TextDropDown::mouseReleased(MouseEvent &mouseEvent) {
        if (mIsDragged) {
            mPushed = false;
        }

        // Released outside of widget. Can happen when we have modal input focus.
        if ((0 > mouseEvent.getY()
             || mouseEvent.getY() >= getHeight()
             || mouseEvent.getX() < 0
             || mouseEvent.getX() >= getWidth())
            && mouseEvent.getButton() == MouseEvent::LEFT
            && isModalMouseInputFocused()) {
            releaseModalMouseInputFocus();

            if (mIsDragged) {
                foldUp();
            }
        } else if (mouseEvent.getButton() == MouseEvent::LEFT) {
            mPushed = false;
        }

        mIsDragged = false;
    }

    void TextDropDown::mouseDragged(MouseEvent &mouseEvent) {
        mIsDragged = true;
        if (mText.length() != 0) {
            mSelectionPosition = getFont()->getStringIndexAt(mText, mouseEvent.getX() + mXScroll);
            fixScroll();
        }
        mouseEvent.consume();
    }

    void TextDropDown::setListModel(ListModel *listModel) {
        mListBox->setListModel(listModel);

        if (mListBox->getSelected() < 0) {
            mListBox->setSelected(0);
        }

        adjustHeight();
    }

    ListModel *TextDropDown::getListModel() {
        return mListBox->getListModel();
    }

    void TextDropDown::adjustHeight() {
        if (mScrollArea == NULL) {
            throw GCN_EXCEPTION("Scroll area has been deleted.");
        }

        if (mListBox == NULL) {
            throw GCN_EXCEPTION("List box has been deleted.");
        }

        int listBoxHeight = mListBox->getHeight();

        // We add 2 for the border
        int h2 = 20;

        setHeight(h2);

        // The addition/subtraction of 2 compensates for the seperation lines
        // seperating the selected element view and the scroll area.

        if (mDroppedDown && getParent()) {
            int h = getParent()->getHeightEx() - 20 - getY();

            if (listBoxHeight > h - h2 - 2) {
                mScrollArea->setHeight(h - h2 - 2);
                setHeight(h);
            } else {
                setHeight(listBoxHeight + h2 + 2);
                mScrollArea->setHeight(listBoxHeight);
            }
        }

        mScrollArea->setWidth(getWidth());
        // Resize the ListBox to exactly fit the ScrollArea.
        mListBox->setWidth(mScrollArea->getChildrenArea().width);
        mScrollArea->setPosition(0, 0);
    }

    void TextDropDown::dropDown() {
        if (!mDroppedDown) {
            mDroppedDown = true;
            mFoldedUpHeight = getHeight();
            adjustHeight();

            if (getParent()) {
                getParent()->moveToTop(this);
            }
        }

        mListBox->requestFocus();
    }

    void TextDropDown::foldUp() {
        if (mDroppedDown) {
            mDroppedDown = false;
            adjustHeight();
            mInternalFocusHandler.focusNone();
        }
    }

    void TextDropDown::focusLost(const Event &event) {
        foldUp();
        mInternalFocusHandler.focusNone();
    }


    void TextDropDown::death(const Event &event) {
        if (event.getSource() == mScrollArea) {
            mScrollArea = NULL;
        }

        BasicContainer::death(event);
    }

    void TextDropDown::action(const ActionEvent &actionEvent) {
        foldUp();
        releaseModalMouseInputFocus();
        mActionEventId = "SELECTION";
        distributeActionEvent();
        mActionEventId = "";
    }

    gcn::Rectangle TextDropDown::getChildrenArea() {
        if (mDroppedDown) {
            // Calculate the children area (with the one pixel border in mind)
            return gcn::Rectangle(1,
                                  mFoldedUpHeight + 1,
                                  getWidth() - 2,
                                  getHeight() - mFoldedUpHeight - 2);
        }

        return gcn::Rectangle();
    }

    void TextDropDown::setBaseColor(const Color &color) {
        if (mInternalScrollArea) {
            mScrollArea->setBaseColor(color);
        }

        if (mInternalListBox) {
            mListBox->setBaseColor(color);
        }

        Widget::setBaseColor(color);
    }

    void TextDropDown::setBackgroundColor(const Color &color) {
        if (mInternalScrollArea) {
            mScrollArea->setBackgroundColor(color);
        }

        if (mInternalListBox) {
            mListBox->setBackgroundColor(color);
        }

        Widget::setBackgroundColor(color);
    }

    void TextDropDown::setForegroundColor(const Color &color) {
        if (mInternalScrollArea) {
            mScrollArea->setForegroundColor(color);
        }

        if (mInternalListBox) {
            mListBox->setForegroundColor(color);
        }

        Widget::setForegroundColor(color);
    }

    void TextDropDown::setFont(Font *font) {
        if (mInternalScrollArea) {
            mScrollArea->setFont(font);
        }

        if (mInternalListBox) {
            mListBox->setFont(font);
        }

        Widget::setFont(font);
    }

    void TextDropDown::mouseWheelMovedUp(MouseEvent &mouseEvent) {
        if (isFocused() && mouseEvent.getSource() == this && !mScrollDisabled) {
            mouseEvent.consume();

            if (mListBox->getSelected() > 0) {
                mListBox->setSelected(mListBox->getSelected() - 1);
            }
        }
    }

    void TextDropDown::mouseWheelMovedDown(MouseEvent &mouseEvent) {
        if (isFocused() && mouseEvent.getSource() == this && !mScrollDisabled) {
            mouseEvent.consume();

            mListBox->setSelected(mListBox->getSelected() + 1);
        }
    }

    void TextDropDown::setSelectionColor(const Color &color) {
        Widget::setSelectionColor(color);

        if (mInternalListBox) {
            mListBox->setSelectionColor(color);
        }
    }

    void TextDropDown::valueChanged(const SelectionEvent &event) {
        distributeValueChangedEvent();
        setText(mListBox->getListModel()->getElementAt(mListBox->getSelected()));
        fixScroll();
    }

    void TextDropDown::addSelectionListener(SelectionListener *selectionListener) {
        mSelectionListeners.push_back(selectionListener);
    }

    void TextDropDown::removeSelectionListener(SelectionListener *selectionListener) {
        mSelectionListeners.remove(selectionListener);
    }

    void TextDropDown::distributeValueChangedEvent() {
        SelectionListenerIterator iter;

        for (iter = mSelectionListeners.begin(); iter != mSelectionListeners.end(); ++iter) {
            SelectionEvent event(this);
            (*iter)->valueChanged(event);
        }
    }

    void TextDropDown::setText(const std::string &text, bool bGenerateActionEvent) {
        if (text.size() < mCaretPosition && mCaretPosition != -1) {
            mCaretPosition = text.size();
        }

        mText = text;
        if (bGenerateActionEvent)
            distributeActionEvent();
    }

    const std::string &TextDropDown::getText() const {
        return mText;
    }

    void TextDropDown::fixScroll() {
        //if (isFocused())
        //{
        int caretX = mCaretPosition == -1 ? 0 : getFont()->getWidth(mText.substr(0, mCaretPosition));

        if (caretX - mXScroll >= getWidth() - 25 - 4) {
            mXScroll = caretX - (getWidth() - 25) + 4;
        } else if (caretX - mXScroll <= 0) {
            mXScroll = caretX - (getWidth() - 25) / 2;

            if (mXScroll < 0) {
                mXScroll = 0;
            }
        }
        //}
    }

    void TextDropDown::mouseEntered(MouseEvent &mouseEvent) {
        mHasMouse = 1;
    }

    void TextDropDown::mouseExited(MouseEvent &mouseEvent) {
        mHasMouse = 0;
    }

    bool TextDropDown::showHand() {
        if (!mHasMouse || !isEnabled()) return 0;
        int x, y;
        getAbsolutePosition(x, y);
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        return (mx > x + getWidth() - 20 && mx < x + getWidth() && my > y && my < y + getHeight());
    }

    void TextDropDown::deleteSelection() {
        int min = (mCaretPosition < mSelectionPosition ? mCaretPosition : mSelectionPosition);
        int max = (mCaretPosition < mSelectionPosition ? mSelectionPosition : mCaretPosition);
        mText.erase(min, max - min);
        if (mCaretPosition == max)
            mCaretPosition -= max - min;
        mSelectionPosition = -1;
    }
}

