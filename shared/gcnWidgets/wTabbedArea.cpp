#include "wTabbedArea.h"

#include "guichan/exception.hpp"
#include "guichan/focushandler.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"

#include "wContainer.h"
#include "wTab.h"

#include <algorithm>

namespace SHR {
    TabbedArea::TabbedArea()
            : mSelectedTab(NULL),
              mOpaque(false) {
        setFocusable(true);
        addKeyListener(this);
        addMouseListener(this);

        mTabContainer = new Contener();
        mTabContainer->setOpaque(false);
        mWidgetContainer = new Contener();

        add(mTabContainer);
        add(mWidgetContainer);
    }

    TabbedArea::~TabbedArea() {
        remove(mTabContainer);
        remove(mWidgetContainer);

        delete mTabContainer;
        delete mWidgetContainer;

        for (unsigned int i = 0; i < mTabsToDelete.size(); i++) {
            delete mTabsToDelete[i];
        }
    }

    void TabbedArea::addTab(const std::string &caption, Widget *widget) {
        Tab *tab = new Tab();
        tab->setCaption(caption);
        mTabsToDelete.push_back(tab);

        addTab(tab, widget);
    }

    void TabbedArea::addTab(Tab *tab, Widget *widget) {
        tab->setTabbedArea(this);
        tab->addActionListener(this);

        mTabContainer->add(tab);
        mTabs.push_back(std::pair<Tab *, Widget *>(tab, widget));

        if (mSelectedTab == NULL) {
            setSelectedTab(tab);
        }

        adjustTabPositions();
        adjustSize();
    }

    void TabbedArea::removeTabWithIndex(unsigned int index) {
        if (index >= mTabs.size()) {
            throw GCN_EXCEPTION("No such tab index.");
        }

        removeTab(mTabs[index].first);
    }

    void TabbedArea::removeTab(Tab *tab) {
        int tabIndexToBeSelected = -1;

        if (tab == mSelectedTab) {
            int index = getSelectedTabIndex();

            if (index == (int) mTabs.size() - 1
                && mTabs.size() >= 2) {
                tabIndexToBeSelected = index--;
            } else if (index == (int) mTabs.size() - 1
                       && mTabs.size() == 1) {
                tabIndexToBeSelected = -1;
            } else {
                tabIndexToBeSelected = index;
            }
        }

        std::vector<std::pair<Tab *, Widget *> >::iterator iter;
        for (iter = mTabs.begin(); iter != mTabs.end(); iter++) {
            if (iter->first == tab) {
                mTabContainer->remove(tab);
                mTabs.erase(iter);
                break;
            }
        }

        std::vector<Tab *>::iterator iter2;
        for (iter2 = mTabsToDelete.begin(); iter2 != mTabsToDelete.end(); iter2++) {
            if (*iter2 == tab) {
                mTabsToDelete.erase(iter2);
                delete tab;
                break;
            }
        }

        if (tabIndexToBeSelected == -1) {
            mSelectedTab = NULL;
            mWidgetContainer->clear();
        } else {
            setSelectedTab(tabIndexToBeSelected);
        }

        adjustSize();
        adjustTabPositions();
    }

    bool TabbedArea::isTabSelected(unsigned int index) const {
        if (index >= mTabs.size()) {
            throw GCN_EXCEPTION("No such tab index.");
        }

        return mSelectedTab == mTabs[index].first;
    }

    bool TabbedArea::isTabSelected(Tab *tab) {
        return mSelectedTab == tab;
    }

    void TabbedArea::setSelectedTab(unsigned int index) {
        if (index >= mTabs.size()) {
            throw GCN_EXCEPTION("No such tab index.");
        }

        setSelectedTab(mTabs[index].first);
    }

    void TabbedArea::setSelectedTab(Tab *tab) {
        unsigned int i;
        for (i = 0; i < mTabs.size(); i++) {
            if (mTabs[i].first == mSelectedTab) {
                mWidgetContainer->remove(mTabs[i].second);
            }
        }

        for (i = 0; i < mTabs.size(); i++) {
            if (mTabs[i].first == tab) {
                mSelectedTab = tab;
                mWidgetContainer->add(mTabs[i].second);
            }
        }
    }

    int TabbedArea::getSelectedTabIndex() const {
        unsigned int i;
        for (i = 0; i < mTabs.size(); i++) {
            if (mTabs[i].first == mSelectedTab) {
                return i;
            }
        }

        return -1;
    }

    Tab *TabbedArea::getSelectedTab() {
        return mSelectedTab;
    }

    void TabbedArea::setOpaque(bool opaque) {
        mOpaque = opaque;
    }

    bool TabbedArea::isOpaque() const {
        return mOpaque;
    }

    void TabbedArea::draw(Graphics *graphics) {
        const Color &faceColor = getBaseColor();
        const int alpha = getBaseColor().a;
        Color highlightColor = faceColor + 0x303030;
        highlightColor.a = alpha;
        Color shadowColor = faceColor - 0x303030;
        shadowColor.a = alpha;

        //hge->Gfx_RenderLine(dx, dy+105, dx+winWorld->getWidth()-15, dy+105, 0xFF5c5c5c);
        //hge->Gfx_RenderLine(dx, dy+104, dx+winWorld->getWidth()-15, dy+104, 0xFF1f1f1f);

        // Draw a border.
        /*graphics->setColor(0x5c5c5c);
        graphics->drawLine(0,
                           mTabContainer->getHeight(),
                           0,
                           getHeight() - 2);*/

        if (isOpaque()) {
            graphics->setColor(getBaseColor());
            graphics->fillRectangle(gcn::Rectangle(2, 2,
                                                   getWidth() - 4,
                                                   getHeight() - 4));
        }

        drawChildren(graphics);

        // Draw a line underneath the tabs.
        graphics->setColor(0x1f1f1f);
        graphics->drawLine(1,
                           mTabContainer->getHeight(),
                           getWidth() - 2,
                           mTabContainer->getHeight());
        graphics->setColor(0x5c5c5c);
        graphics->drawLine(1,
                           mTabContainer->getHeight() + 1,
                           getWidth() - 2,
                           mTabContainer->getHeight() + 1);
        // If a tab is selected, remove the line right underneath
        // the selected tab.
        if (mSelectedTab != NULL) {
            graphics->setColor(getBaseColor());
            graphics->drawLine(mSelectedTab->getX() + 1,
                               mTabContainer->getHeight(),
                               mSelectedTab->getX() + mSelectedTab->getWidth() - 2,
                               mTabContainer->getHeight());

        }

        graphics->setColor(0x1f1f1f);
        graphics->drawLine(1,
                           getHeight() - 2,
                           getWidth() - 1,
                           getHeight() - 2);
        graphics->drawLine(getWidth() - 2,
                           mTabContainer->getHeight() + 1,
                           getWidth() - 2,
                           getHeight() - 2);
        graphics->drawLine(1,
                           mTabContainer->getHeight() + 1,
                           1,
                           getHeight() - 2);

        graphics->setColor(0x5c5c5c);
        graphics->drawLine(getWidth() - 1,
                           mTabContainer->getHeight() + 1,
                           getWidth() - 1,
                           getHeight() - 1);
        graphics->drawLine(1,
                           getHeight() - 1,
                           getWidth() - 1,
                           getHeight() - 1);
        graphics->drawLine(2,
                           mTabContainer->getHeight() + 1,
                           2,
                           getHeight() - 3);
    }

    void TabbedArea::logic() {
        logicChildren();
    }

    void TabbedArea::adjustSize() {
        int maxTabHeight = 0;

        for (unsigned int i = 0; i < mTabs.size(); i++) {
            if (mTabs[i].first->getHeight() > maxTabHeight) {
                maxTabHeight = mTabs[i].first->getHeight();
            }
        }

        mTabContainer->setSize(getWidth() - 2,
                               maxTabHeight);

        mWidgetContainer->setPosition(1, maxTabHeight + 1);
        mWidgetContainer->setSize(getWidth() - 2,
                                  getHeight() - maxTabHeight - 2);
    }

    void TabbedArea::adjustTabPositions() {
        int maxTabHeight = 0;
        unsigned int i;
        for (i = 0; i < mTabs.size(); i++) {
            if (mTabs[i].first->getHeight() > maxTabHeight) {
                maxTabHeight = mTabs[i].first->getHeight();
            }
        }

        int x = 0;
        for (i = 0; i < mTabs.size(); i++) {
            Tab *tab = mTabs[i].first;
            tab->setPosition(x, maxTabHeight - tab->getHeight());
            x += tab->getWidth();
        }
    }

    void TabbedArea::setWidth(int width) {
        Widget::setWidth(width);
        adjustSize();
    }

    void TabbedArea::setHeight(int height) {
        Widget::setHeight(height);
        adjustSize();
    }

    void TabbedArea::setSize(int width, int height) {
        Widget::setSize(width, height);
        adjustSize();
    }

    void TabbedArea::setDimension(const Rectangle &dimension) {
        Widget::setDimension(dimension);
        adjustSize();
    }

    void TabbedArea::keyPressed(KeyEvent &keyEvent) {
        if (keyEvent.isConsumed() || !isFocused()) {
            return;
        }

        if (keyEvent.getKey().getValue() == Key::LEFT) {
            int index = getSelectedTabIndex();
            index--;

            if (index < 0) {
                return;
            } else {
                setSelectedTab(mTabs[index].first);
            }

            keyEvent.consume();
        } else if (keyEvent.getKey().getValue() == Key::RIGHT) {
            int index = getSelectedTabIndex();
            index++;

            if (index >= (int) mTabs.size()) {
                return;
            } else {
                setSelectedTab(mTabs[index].first);
            }

            keyEvent.consume();
        }
    }


    void TabbedArea::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.isConsumed()) {
            return;
        }

        if (mouseEvent.getButton() == MouseEvent::LEFT) {
            Widget *widget = mTabContainer->getWidgetAt(mouseEvent.getX(), mouseEvent.getY());
            Tab *tab = dynamic_cast<Tab *>(widget);

            if (tab != NULL) {
                setSelectedTab(tab);
            }
        }

        // Request focus only if the source of the event
        // is not focusble. If the source of the event
        // is focused we don't want to steal the focus.
        if (!mouseEvent.getSource()->isFocusable()) {
            requestFocus();
        }
    }

    void TabbedArea::death(const Event &event) {
        Tab *tab = dynamic_cast<Tab *>(event.getSource());

        if (tab != NULL) {
            removeTab(tab);
        } else {
            BasicContainer::death(event);
        }
    }

    void TabbedArea::action(const ActionEvent &actionEvent) {
        Widget *source = actionEvent.getSource();
        Tab *tab = dynamic_cast<Tab *>(source);

        if (tab == NULL) {
            throw GCN_EXCEPTION("Received an action from a widget that's not a tab!");
        }

        setSelectedTab(tab);
    }
}
