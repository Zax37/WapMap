#include "wTextDropDown.h"
#include "guichan/exception.hpp"
#include "guichan/key.hpp"
#include "wListbox.h"
#include "wScrollArea.h"
#include "../../WapMap/cInterfaceSheet.h"
#include "../commonFunc.h"

#ifdef WAP_MAP
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
        UpdateTooltip(mHasMouse);
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
        if (isFocused() && fFocusTimer < 0.2f) {
            fFocusTimer += hge->Timer_GetDelta();
            if (fFocusTimer > 0.2f) fFocusTimer = 0.2f;
        } else if (!isFocused() && fFocusTimer > 0.0f) {
            fFocusTimer -= hge->Timer_GetDelta();
            if (fFocusTimer < 0.0f) fFocusTimer = 0.0;
        }

        ClipRectangle rect = graphics->getCurrentClipArea();

        renderFrame(x, y, getWidth(), h, getAlpha(), 0);
        if (fFocusTimer > 0)
            renderFrame(x, y, getWidth(), h, (fFocusTimer * 5.0f * 255.0f) * getAlphaModifier(), 1);

        _ghGfxInterface->sprDropDownBarArrow->SetColor(isEnabled() ? 0xFFFFFFFF : 0xFF5e5e5e);
        _ghGfxInterface->sprDropDownBarArrow->SetFlip(0, mDroppedDown);
        _ghGfxInterface->sprDropDownBarArrow->Render(x + getWidth() - 16, y + 7);

        graphics->pushClipArea(gcn::Rectangle(2, 2, getWidth() - 20, getHeight() - 4));

        hgeFont *fnt = ((HGEImageFont *) getFont())->getHandleHGE();
        fnt->SetColor(SETA(0xe1e1e1, (isEnabled() ? 0xFF : 0x77) * getAlpha() / 255.f));
        fnt->Render(x + 5 - mXScroll, ceil(y + h / 2) - 1, HGETEXT_LEFT | HGETEXT_MIDDLE, mText.c_str(), true);

        if (bTextFocused && mSelectionPosition != -1 && mSelectionPosition != mCaretPosition) {
            int starti, endi;
            if (mCaretPosition < mSelectionPosition) {
                starti = mCaretPosition;
                endi = mSelectionPosition;
            } else {
                starti = mSelectionPosition;
                endi = mCaretPosition;
            }
            int startX = x + GV->fntMyriad16->GetStringWidth(mText.substr(0, starti).c_str(), false, true) - mXScroll + 5,
                endX = x + GV->fntMyriad16->GetStringWidth(mText.substr(0, endi).c_str(), false, true) - mXScroll + 5;
            SHR::SetQuad(&q, 0x553393e6, startX, y + 4, endX, y + getFont()->getHeight() + 1);
            hge->Gfx_RenderQuad(&q);
            hge->Gfx_RenderLine(startX, y + 4, startX, y + getFont()->getHeight() + 1, 0xFF3393e6);
            hge->Gfx_RenderLine(endX, y + 4, endX, y + getFont()->getHeight() + 1, 0xFF3393e6);
        } else if (bTextFocused && mCaretPosition != -1) {
            drawCaret(graphics, GV->fntMyriad16->GetStringWidth(mText.substr(0, mCaretPosition).c_str(), false, true) - mXScroll);
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

        if (key.getValue() == Key::DOWN) {
            dropDown();
            keyEvent.consume();
        } else if (key.getValue() == Key::LEFT) {
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
        } else if (key.isCharacter() && key.getValue() != Key::TAB ||
                   key.isNumber()) {
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

        if (key.getValue() != Key::TAB) {
            keyEvent.consume();
        }

        fixScroll();
    }

    void TextDropDown::mouseMoved(MouseEvent &mouseEvent) {
        if (isEnabled() && mouseEvent.getX() < getWidth() - 22) {
            GV->SetCursor(TEXT);
        }
    }

    void TextDropDown::mousePressed(MouseEvent &mouseEvent) {
        int x = mouseEvent.getX(),
            y = mouseEvent.getY();

        if (mouseEvent.getSource() == this) {

            // If we have a mouse press on the widget.
            if (y >= 0
                && y < getHeight()
                && x >= 0
                && x < getWidth()
                && mouseEvent.getButton() == MouseEvent::LEFT
                && !mDroppedDown) {
                if (x < getWidth() - 22) {
                    mCaretPosition = getFont()->getStringIndexAt(mText, x + mXScroll);
                    fixScroll();
                    bTextFocused = true;
                    GV->SetCursor(TEXT);
                }
                else {
                    bTextFocused = false;
                    mCaretPosition = -1;
                    mPushed = true;
                    dropDown();
                    requestModalMouseInputFocus();
                }
                mSelectionPosition = -1;
            }
            // Fold up the listbox if the upper part is clicked after fold down
            else if (y >= 0
                && y < mFoldedUpHeight
                && x >= 0
                && x < getWidth()
                && mouseEvent.getButton() == MouseEvent::LEFT
                && mDroppedDown) {
                if (x < getWidth() - 22) {
                    bTextFocused = true;
                    GV->SetCursor(TEXT);
                }
                mPushed = false;
                foldUp();
                releaseModalMouseInputFocus();
            }
            // If we have a mouse press outside the widget
            else if (0 > y
                || y >= getHeight()
                || x < 0
                || x >= getWidth()) {
                mPushed = false;
                foldUp();
                bTextFocused = false;
            }
        }
    }

    void TextDropDown::mouseReleased(MouseEvent &mouseEvent) {
        int x = mouseEvent.getX(),
            y = mouseEvent.getY();

        if (mIsDragged) {
            mPushed = false;
        }

        // Released outside of widget. Can happen when we have modal input focus.
        if ((0 > y
             || y >= getHeight()
             || x < 0
             || x >= getWidth())
            && mouseEvent.getButton() == MouseEvent::LEFT
            && isModalMouseInputFocused()) {
            releaseModalMouseInputFocus();
        } else if (mouseEvent.getButton() == MouseEvent::LEFT) {
            mPushed = false;
        }

        mIsDragged = false;

        if (isEnabled() && mouseEvent.getX() < getWidth() - 22) {
            GV->SetCursor(TEXT);
        }
    }

    void TextDropDown::mouseDragged(DragEvent &mouseEvent) {
        mIsDragged = true;
        if (mText.length() != 0) {
            mSelectionPosition = getFont()->getStringIndexAt(mText, mouseEvent.getX() + mXScroll);
            fixScroll();
        }

        int h = (mDroppedDown ? mFoldedUpHeight : getHeight());
        int x, y;
        getAbsolutePosition(x, y);

        if (isEnabled() && mHasMouse && !(mouseEvent.getX() > x + getWidth() - 20
                && mouseEvent.getX() < x + getWidth() && mouseEvent.getY() > y && mouseEvent.getY() < y + h)) {
            GV->SetCursor(TEXT);
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
        int i = mListBox->findIndexOf(mText);
        if (i != -1) {
            mListBox->setSelected(i);
            mScrollArea->setVerticalScrollAmount((i + 0.5f) * mListBox->getRowHeight() - mScrollArea->getHeight() / 2);
        }
    }

    void TextDropDown::foldUp() {
        if (mDroppedDown) {
            mDroppedDown = false;
            adjustHeight();
            mInternalFocusHandler.focusNone();
        }
    }

    void TextDropDown::focusLost(const FocusEvent& event) {
        Widget* source = event.getSource();
        foldUp();
        mInternalFocusHandler.focusNone();
        bTextFocused = false;
    }

    void TextDropDown::focusGained(const FocusEvent& event) {
        bTextFocused = true;
    }

    void TextDropDown::death(const Event &event) {
        if (event.getSource() == mScrollArea) {
            mScrollArea = NULL;
        }

        BasicContainer::death(event);
    }

    void TextDropDown::action(const ActionEvent &actionEvent) {
		if (actionEvent.getId() != "WHEEL") {
			foldUp();
		}
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
        if (mDroppedDown) {
            mScrollArea->mouseWheelMovedUp(mouseEvent);
            mouseMoved(mouseEvent);
        } else if (isFocused() && mouseEvent.getSource() == this && !mScrollDisabled) {
            mouseEvent.consume();

            if (mListBox->getSelected() > 0) {
                mListBox->setSelected(mListBox->getSelected() - 1);
            }
        }
    }

    void TextDropDown::mouseWheelMovedDown(MouseEvent &mouseEvent) {
        if (mDroppedDown) {
            mScrollArea->mouseWheelMovedDown(mouseEvent);
            mouseMoved(mouseEvent);
        } else if (isFocused() && mouseEvent.getSource() == this && !mScrollDisabled) {
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
        mHasMouse = true;
    }

    void TextDropDown::mouseExited(MouseEvent &mouseEvent) {
        mHasMouse = false;
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

