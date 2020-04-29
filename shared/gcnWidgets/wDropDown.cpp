#include "wDropDown.h"

#include "guichan/exception.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/key.hpp"
#include "guichan/listmodel.hpp"
#include "guichan/mouseinput.hpp"
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
        }

        addMouseListener(this);
        addKeyListener(this);
        addFocusListener(this);

        adjustHeight();

        fButtonTimer = 0;
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

        Color faceColor = getBaseColor();
        Color highlightColor, shadowColor;
        int alpha = getBaseColor().a;
        highlightColor = faceColor + 0x303030;
        highlightColor.a = alpha;
        shadowColor = faceColor - 0x303030;
        shadowColor.a = alpha;

        ClipRectangle rect = graphics->getCurrentClipArea();
        int x, y;
        getAbsolutePosition(x, y);

        //hge->Gfx_SetClipping();
        /* hGfx->sprButDDL->RenderStretch(x, y, x+3, y+h);
         hGfx->sprButDDC->RenderStretch(x+3, y, x+getWidth()-26, y+h);
         hGfx->sprButDDR->RenderStretch(x+getWidth()-26, y, x+getWidth(), y+h);*/

        _ghGfxInterface->sprDropDownBar[0]->Render(x, y);
        _ghGfxInterface->sprDropDownBar[1]->RenderStretch(x + 13, y, x + getWidth() - 20, y + h);
        _ghGfxInterface->sprDropDownBar[2]->Render(x + getWidth() - 21, y);

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

        if (fButtonTimer > 0.0f) {
            _ghGfxInterface->sprDropDownButtonFocused->SetColor(
                    SETA(0xFFFFFFFF, (unsigned char) (fButtonTimer * 2.5f * 255.0f)));
            _ghGfxInterface->sprDropDownButtonFocused->Render(x + getWidth() - 21, y);
        }

        _ghGfxInterface->sprDropDownBarArrow->SetColor(isEnabled() ? 0xFFFFFFFF : 0xFF5e5e5e);
        _ghGfxInterface->sprDropDownBarArrow->SetFlip(0, mDroppedDown);
        _ghGfxInterface->sprDropDownBarArrow->Render(x + getWidth() - 16, y + 7);

        // Push a clip area so the other drawings don't need to worry
        // about the border.
        graphics->pushClipArea(gcn::Rectangle(1, 1, getWidth() - 2, h - 2));
        //hge->Gfx_SetClipping();
        const gcn::Rectangle currentClipArea = graphics->getCurrentClipArea();

        if (mListBox->getListModel()
            && mListBox->getSelected() >= 0) {
            graphics->setColor(isEnabled() ? 0xa1a1a1 : 0x5e5e5e);
            graphics->setFont(getFont());

            graphics->drawText(mListBox->getListModel()->getElementAt(mListBox->getSelected()), 5, 0);
        }

        //drawButton(graphics);
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
        RenderTooltip();
    }

    void DropDown::mouseEntered(MouseEvent &mouseEvent) {
        mHasMouse = 1;
    }

    void DropDown::mouseExited(MouseEvent &mouseEvent) {
        mHasMouse = 0;
    }

    bool DropDown::showHand() {
        int x, y;
        getAbsolutePosition(x, y);
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        return (mHasMouse && isEnabled() && mx > x + getWidth() - 20 && mx < x + getWidth() && my > y &&
                my < y + getHeight());
    }

    int DropDown::getSelected() const {
        return mListBox->getSelected();
    }

    void DropDown::setSelected(int selected) {
        if (selected >= 0) {
            mListBox->setSelected(selected);
        }
    }

    void DropDown::keyPressed(KeyEvent &keyEvent) {
        if (keyEvent.isConsumed())
            return;

        Key key = keyEvent.getKey();

        /*if ((key.getValue() == Key::ENTER || key.getValue() == Key::SPACE)
            && !mDroppedDown)
        {
            dropDown();
            keyEvent.consume();
        }
        else if (key.getValue() == Key::UP && !mScrollDisabled )
        {
            setSelected(getSelected() - 1);
            keyEvent.consume();
        }
        else if (key.getValue() == Key::DOWN && !mScrollDisabled)
        {
            setSelected(getSelected() + 1);
            keyEvent.consume();
        }*/
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
            int h = getParent()->getHeightEx() - 15 - 24 - getY();

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
            mFoldedUpHeight = getHeight();
            adjustHeight();

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

    void DropDown::focusLost(const Event &event) {
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
        distributeValueChangedEvent();
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

