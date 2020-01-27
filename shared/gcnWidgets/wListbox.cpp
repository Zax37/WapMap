#include "wListbox.h"

#include "guichan/basiccontainer.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/key.hpp"
#include "guichan/listmodel.hpp"
#include "guichan/mouseinput.hpp"
#include "guichan/selectionlistener.hpp"

namespace SHR {
    ListBox::ListBox()
            : mSelected(-1),
              mListModel(NULL),
              mWrappingEnabled(false) {
        mHighlighted = -1;
        setWidth(100);
        setFocusable(true);

        addMouseListener(this);
        addKeyListener(this);
        setBackgroundColor(0x545454);
        setForegroundColor(0x545454);
        setBaseColor(0x545454);
    }

    ListBox::ListBox(ListModel *listModel)
            : mSelected(-1),
              mWrappingEnabled(false) {
        mHighlighted = -1;
        setWidth(100);
        setListModel(listModel);
        setFocusable(true);

        addMouseListener(this);
        addKeyListener(this);
        //setForegroundColor();
        setBackgroundColor(0x545454);
        setForegroundColor(0x545454);
        setBaseColor(0x545454);
    }

    void ListBox::draw(Graphics *graphics) {
        graphics->setColor(0x131313);
        graphics->fillRectangle(Rectangle(0, 0, getWidth(), getHeight()));

        if (mListModel == NULL) {
            return;
        }

        graphics->setColor(0xbababa);
        graphics->setFont(getFont());

        // Check the current clip area so we don't draw unnecessary items
        // that are not visible.
        const ClipRectangle currentClipArea = graphics->getCurrentClipArea();
        int rowHeight = getRowHeight();

        // Calculate the number of rows to draw by checking the clip area.
        // The addition of two makes covers a partial visible row at the top
        // and a partial visible row at the bottom.
        int numberOfRows = currentClipArea.height / rowHeight + 2;

        if (numberOfRows > mListModel->getNumberOfElements()) {
            numberOfRows = mListModel->getNumberOfElements();
        }

        // Calculate which row to start drawing. If the list box
        // has a negative y coordinate value we should check if
        // we should drop rows in the begining of the list as
        // they might not be visible. A negative y value is very
        // common if the list box for instance resides in a scroll
        // area and the user has scrolled the list box downwards.
        int startRow;
        if (getY() < 0) {
            startRow = -1 * (getY() / rowHeight);
        } else {
            startRow = 0;
        }

        int i;
        // The y coordinate where we start to draw the text is
        // simply the y coordinate multiplied with the font height.
        int y = rowHeight * startRow;
        for (i = startRow; i < startRow + numberOfRows; ++i) {
            if (i == mSelected || i == mHighlighted) {
                graphics->setColor(i == mSelected ? 0x1585e2 : 0x454545);
                graphics->fillRectangle(Rectangle(0, y, getWidth(), rowHeight));
            }

            if (i == mSelected)
                graphics->setColor(0xffffff);
            else
                graphics->setColor(i == mHighlighted ? 0xdcdcdc : 0xbababa);

            // If the row height is greater than the font height we
            // draw the text with a center vertical alignment.
            if (rowHeight > getFont()->getHeight()) {
                graphics->drawText(mListModel->getElementAt(i), 1, y + rowHeight / 2 - getFont()->getHeight() / 2);
            } else {
                graphics->drawText(mListModel->getElementAt(i), 1, y);
            }

            y += rowHeight;
        }
    }

    void ListBox::logic() {
        adjustSize();
    }

    int ListBox::getSelected() const {
        return mSelected;
    }

    void ListBox::setSelected(int selected) {
        if (mListModel == NULL) {
            mSelected = -1;
        } else {
            if (selected < 0) {
                mSelected = -1;
            } else if (selected >= mListModel->getNumberOfElements()) {
                mSelected = mListModel->getNumberOfElements() - 1;
            } else {
                mSelected = selected;
            }
        }

        Rectangle scroll;

        if (mSelected < 0) {
            scroll.y = 0;
        } else {
            scroll.y = getRowHeight() * mSelected;
        }

        scroll.height = getRowHeight();
        showPart(scroll);

        distributeValueChangedEvent();
    }

    void ListBox::keyPressed(KeyEvent &keyEvent) {
        Key key = keyEvent.getKey();

        if (key.getValue() == Key::ENTER || key.getValue() == Key::SPACE) {
            mActionEventId = "CLICK";
            distributeActionEvent();
            keyEvent.consume();
        } else if (key.getValue() == Key::UP) {
            setSelected(mSelected - 1);

            if (mSelected == -1) {
                if (mWrappingEnabled) {
                    setSelected(getListModel()->getNumberOfElements() - 1);
                } else {
                    setSelected(0);
                }
            }

            keyEvent.consume();
        } else if (key.getValue() == Key::DOWN) {
            if (mWrappingEnabled
                && getSelected() == getListModel()->getNumberOfElements() - 1) {
                setSelected(0);
            } else {
                setSelected(getSelected() + 1);
            }

            keyEvent.consume();
        } else if (key.getValue() == Key::HOME) {
            setSelected(0);
            keyEvent.consume();
        } else if (key.getValue() == Key::END) {
            setSelected(getListModel()->getNumberOfElements() - 1);
            keyEvent.consume();
        }
    }

    void ListBox::mouseExited(MouseEvent &mouseEvent) {
        mHighlighted = -1;
    }

    void ListBox::mouseMoved(MouseEvent &mouseEvent) {
        mHighlighted = (mouseEvent.getY() / getRowHeight());
    }

    void ListBox::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT) {
            setSelected(mouseEvent.getY() / getRowHeight());
            mActionEventId = "CLICK";
            distributeActionEvent();
        }
    }

    void ListBox::mouseWheelMovedUp(MouseEvent &mouseEvent) {
        if (isFocused()) {
            if (getSelected() > 0) {
                setSelected(getSelected() - 1);
            }

            mouseEvent.consume();
            mActionEventId = "WHEEL";
            distributeActionEvent();
        }
    }

    void ListBox::mouseWheelMovedDown(MouseEvent &mouseEvent) {
        if (isFocused()) {
            setSelected(getSelected() + 1);

            mouseEvent.consume();
            mActionEventId = "WHEEL";
            distributeActionEvent();
        }
    }

    void ListBox::mouseDragged(MouseEvent &mouseEvent) {
        mouseEvent.consume();
    }

    void ListBox::setListModel(ListModel *listModel) {
        mSelected = -1;
        mListModel = listModel;
        adjustSize();
    }

    ListModel *ListBox::getListModel() {
        return mListModel;
    }

    void ListBox::adjustSize() {
        if (mListModel != NULL) {
            setHeight(getRowHeight() * mListModel->getNumberOfElements());
        }
    }

    bool ListBox::isWrappingEnabled() const {
        return mWrappingEnabled;
    }

    void ListBox::setWrappingEnabled(bool wrappingEnabled) {
        mWrappingEnabled = wrappingEnabled;
    }

    void ListBox::addSelectionListener(SelectionListener *selectionListener) {
        mSelectionListeners.push_back(selectionListener);
    }

    void ListBox::removeSelectionListener(SelectionListener *selectionListener) {
        mSelectionListeners.remove(selectionListener);
    }

    void ListBox::distributeValueChangedEvent() {
        SelectionListenerIterator iter;

        for (iter = mSelectionListeners.begin(); iter != mSelectionListeners.end(); ++iter) {
            SelectionEvent event(this);
            (*iter)->valueChanged(event);
        }
    }

    unsigned int ListBox::getRowHeight() const {
        return getFont()->getHeight();
    }
}
