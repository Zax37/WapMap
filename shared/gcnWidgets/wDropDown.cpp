#include "wDropDown.h"

#include "guichan/exception.hpp"
#include "guichan/key.hpp"
#include "wListbox.h"
#include "wScrollArea.h"
#include "../../WapMap/cInterfaceSheet.h"

extern cInterfaceSheet *_ghGfxInterface;
extern HGE *hge;

namespace SHR {
    DropDown::DropDown(gcn::ListModel *listModel,
                       SHR::ScrollArea *scrollArea,
                       SHR::ListBox *listBox) {
        mHasMouse = 0;
        mScrollDisabled = 0;
        setWidth(100);
        setFocusable(true);
        mDroppedDown = false;
        mPushed = false;
        mIsDragged = false;

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
        } else {
            mSelected = mListBox->getSelected();
        }

        addMouseListener(this);
        addKeyListener(this);
        addFocusListener(this);

        adjustHeight();

        fFocusTimer = 0;
    }

    DropDown::~DropDown() {
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

    void DropDown::draw(Graphics *graphics) {
        UpdateTooltip(mHasMouse);
        int h;

        if (mDroppedDown) {
            h = mFoldedUpHeight;
        } else {
            h = getHeight();
        }

        unsigned char alpha = getAlpha();

        ClipRectangle rect = graphics->getCurrentClipArea();
        int x, y;
        getAbsolutePosition(x, y);

        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        bool buttonfoc = (isEnabled() && mHasMouse && mx > x + getWidth() - 20 && mx < x + getWidth() && my > y &&
                          my < y + h);

        if (mDroppedDown && fFocusTimer < 0.4f) {
            fFocusTimer += hge->Timer_GetDelta();
            if (fFocusTimer > 0.4f) fFocusTimer = 0.4f;
        } else if (!mDroppedDown && fFocusTimer > 0.2f) {
            fFocusTimer -= hge->Timer_GetDelta();
            if (fFocusTimer < 0.2f) fFocusTimer = 0.2f;
        } else if (isFocused() && fFocusTimer < 0.2f) {
            fFocusTimer += hge->Timer_GetDelta();
            if (fFocusTimer > 0.2f) fFocusTimer = 0.2f;
        } else if (!isFocused() && fFocusTimer > 0.0f) {
            fFocusTimer -= hge->Timer_GetDelta();
            if (fFocusTimer < 0.0f) fFocusTimer = 0.0f;
        }

        renderFrame(x, y, getWidth(), getHeight(), getAlpha(), 0);
        if (fFocusTimer > 0)
            renderFrame(x, y, getWidth(), getHeight(), (fFocusTimer * 2.5f * 255.0f) * getAlphaModifier(), 1);

        _ghGfxInterface->sprDropDownBarArrow->SetColor(SETA(isEnabled() ? 0xFFFFFFFF : 0xFF5e5e5e, alpha));
        _ghGfxInterface->sprDropDownBarArrow->SetFlip(0, mDroppedDown);
        _ghGfxInterface->sprDropDownBarArrow->Render(x + getWidth() - 16, y + 7);

        // Push a clip area so the other drawings don't need to worry
        // about the border.
        graphics->pushClipArea(gcn::Rectangle(1, 1, getWidth() - 2, h - 2));
        const gcn::Rectangle currentClipArea = graphics->getCurrentClipArea();

        if (mListBox->getListModel() && mSelected >= 0) {
            graphics->setColor(gcn::Color(0xe1e1e1, (isEnabled() ? 0xFF : 0x77) * alpha / 255.f));
            graphics->setFont(getFont());

            graphics->drawText(mListBox->getListModel()->getElementAt(mSelected), 5, 2);
        }

        graphics->popClipArea();

        if (mDroppedDown) {
            // Draw a border around the children.
            /*graphics->setColor(gcn::Color(0x000000, alpha));
            graphics->drawRectangle(gcn::Rectangle(0,
                                                   mFoldedUpHeight,
                                                   getWidth(),
                                                   getHeight() - mFoldedUpHeight));*/
            drawChildren(graphics);
            hge->Gfx_RenderLine(x, y + mFoldedUpHeight + 1, x + getWidth(), y + mFoldedUpHeight + 1, SETA(0x178ce1, alpha));
        }
    }

    void DropDown::mouseEntered(MouseEvent &mouseEvent) {
        mHasMouse = 1;
    }

    void DropDown::mouseExited(MouseEvent &mouseEvent) {
        mHasMouse = 0;
    }

    int DropDown::getSelected() const {
        return mSelected;
    }

    void DropDown::setSelected(int selected, bool generatingEvent) {
        if (selected >= 0) {
            mSelected = selected;
            mListBox->setSelected(selected, generatingEvent);
        }
    }

    void DropDown::keyPressed(KeyEvent &keyEvent) {
        if (keyEvent.isConsumed())
            return;

        Key key = keyEvent.getKey();

        if (mDroppedDown) {
            if (key.getValue() == Key::ESCAPE) {
                foldUp();
                keyEvent.consume();
            }
        } else {
            if (key.getValue() == Key::ENTER || key.getValue() == Key::SPACE || key.getValue() == Key::DOWN) {
                dropDown();
                keyEvent.consume();
            }
        }
    }

    void DropDown::mousePressed(MouseEvent &mouseEvent) {
        // If we have a mouse press on the widget.
        if (0 <= mouseEvent.getY()
            && mouseEvent.getY() < getHeight()
            && mouseEvent.getX() >= 0
            && mouseEvent.getX() < getWidth()
            && mouseEvent.getButton() == MouseEvent::LEFT
            && !mDroppedDown
            && mouseEvent.getSource() == this) {
            mPushed = true;
            dropDown();
            requestModalMouseInputFocus();
        }
            // Fold up the listbox if the upper part is clicked after fold down
        else if (0 <= mouseEvent.getY()
                 && mouseEvent.getY() < mFoldedUpHeight
                 && mouseEvent.getX() >= 0
                 && mouseEvent.getX() < getWidth()
                 && mouseEvent.getButton() == MouseEvent::LEFT
                 && mDroppedDown
                 && mouseEvent.getSource() == this) {
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
            releaseModalMouseInputFocus();
        }
    }

    void DropDown::mouseReleased(MouseEvent &mouseEvent) {
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

    void DropDown::mouseDragged(MouseEvent &mouseEvent) {
        mIsDragged = true;

        mouseEvent.consume();
    }

    void DropDown::setListModel(ListModel *listModel) {
        mListBox->setListModel(listModel);

        if (mListBox->getSelected() < 0) {
            mListBox->setSelected(0);
        }

        adjustHeight();
    }

    ListModel *DropDown::getListModel() {
        return mListBox->getListModel();
    }

    void DropDown::adjustHeight() {
        if (mScrollArea == NULL) {
            throw GCN_EXCEPTION("Scroll area has been deleted.");
        }

        if (mListBox == NULL) {
            throw GCN_EXCEPTION("List box has been deleted.");
        }

        int listBoxHeight = mListBox->getHeight();

        // We add 2 for the border
        int h2 = getFont()->getHeight() + 2;
        if (h2 < 22) h2 = 22;

        setHeight(h2);

        // The addition/subtraction of 2 compensates for the seperation lines
        // seperating the selected element view and the scroll area.

        if (mDroppedDown && getParent()) {
            int h = getParent()->getHeightEx() - getY();

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

    void DropDown::dropDown() {
        if (!mDroppedDown) {
            mDroppedDown = true;
            mListBox->setSelected(mSelected, false);
            mFoldedUpHeight = getHeight();
            adjustHeight();
            mScrollArea->setVerticalScrollAmount((mSelected + 0.5f) * mListBox->getRowHeight() - mScrollArea->getHeight() / 2);

            if (getParent()) {
                getParent()->moveToTop(this);
            }
        }

        mListBox->requestFocus();
    }

    void DropDown::foldUp() {
        if (mDroppedDown) {
            mDroppedDown = false;
            adjustHeight();
            mInternalFocusHandler.focusNone();
        }
    }

    void DropDown::focusLost(const FocusEvent &event) {
        foldUp();
        mInternalFocusHandler.focusNone();
    }


    void DropDown::death(const Event &event) {
        if (event.getSource() == mScrollArea) {
            mScrollArea = NULL;
        }

        BasicContainer::death(event);
    }

    void DropDown::action(const ActionEvent &actionEvent) {
        if (actionEvent.getId() != "WHEEL")
            foldUp();
        if (actionEvent.getId() == "CLICK") {
            mSelected = mListBox->getSelected();
            distributeValueChangedEvent();
        }
        releaseModalMouseInputFocus();
        distributeActionEvent();
    }

    gcn::Rectangle DropDown::getChildrenArea() {
        if (mDroppedDown) {
            // Calculate the children area (with the one pixel border in mind)
            return gcn::Rectangle(1,
                                  mFoldedUpHeight + 1,
                                  getWidth() - 2,
                                  getHeight() - mFoldedUpHeight - 2);
        }

        return gcn::Rectangle();
    }

    void DropDown::setBaseColor(const Color &color) {
        if (mInternalScrollArea) {
            mScrollArea->setBaseColor(color);
        }

        if (mInternalListBox) {
            mListBox->setBaseColor(color);
        }

        Widget::setBaseColor(color);
    }

    void DropDown::setBackgroundColor(const Color &color) {
        if (mInternalScrollArea) {
            mScrollArea->setBackgroundColor(color);
        }

        if (mInternalListBox) {
            mListBox->setBackgroundColor(color);
        }

        Widget::setBackgroundColor(color);
    }

    void DropDown::setForegroundColor(const Color &color) {
        if (mInternalScrollArea) {
            mScrollArea->setForegroundColor(color);
        }

        if (mInternalListBox) {
            mListBox->setForegroundColor(color);
        }

        Widget::setForegroundColor(color);
    }

    void DropDown::setFont(Font *font) {
        if (mInternalScrollArea) {
            mScrollArea->setFont(font);
        }

        if (mInternalListBox) {
            mListBox->setFont(font);
        }

        Widget::setFont(font);
    }

    void DropDown::mouseWheelMovedUp(MouseEvent &mouseEvent) {
        if (isFocused() && mouseEvent.getSource() == this && !mScrollDisabled && mDroppedDown) {
            mouseEvent.consume();

            if (mListBox->getSelected() > 0) {
                mListBox->setSelected(mListBox->getSelected() - 1);
            }
        }
    }

    void DropDown::mouseWheelMovedDown(MouseEvent &mouseEvent) {
        if (isFocused() && mouseEvent.getSource() == this && !mScrollDisabled && mDroppedDown) {
            mouseEvent.consume();
            mListBox->setSelected(mListBox->getSelected() + 1);
        }
    }

    void DropDown::setSelectionColor(const Color &color) {
        Widget::setSelectionColor(color);

        if (mInternalListBox) {
            mListBox->setSelectionColor(color);
        }
    }

    void DropDown::valueChanged(const SelectionEvent &event) {
        /*if (!mDroppedDown) {
            mSelected = mListBox->getSelected();
            distributeValueChangedEvent();
        }*/
    }

    void DropDown::addSelectionListener(SelectionListener *selectionListener) {
        mSelectionListeners.push_back(selectionListener);
    }

    void DropDown::removeSelectionListener(SelectionListener *selectionListener) {
        mSelectionListeners.remove(selectionListener);
    }

    void DropDown::distributeValueChangedEvent() {
        SelectionListenerIterator iter;

        for (iter = mSelectionListeners.begin(); iter != mSelectionListeners.end(); ++iter) {
            SelectionEvent event(this);
            (*iter)->valueChanged(event);
        }
    }
}

