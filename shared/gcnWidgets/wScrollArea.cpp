#include "wScrollArea.h"
#include "guichan/exception.hpp"
#include "guichan/graphics.hpp"

#include "../../WapMap/globals.h"
#include "../../WapMap/cInterfaceSheet.h"

extern HGE* hge;

namespace SHR {
    ScrollArea::ScrollArea() : ScrollArea(NULL, SHOW_AUTO, SHOW_AUTO) {
    }

    ScrollArea::ScrollArea(gcn::Widget *content) : ScrollArea(content, SHOW_AUTO, SHOW_AUTO) {
    }

    ScrollArea::ScrollArea(Widget *content,
                           ScrollPolicy hPolicy,
                           ScrollPolicy vPolicy) {
        mVScroll = 0;
        mHScroll = 0;
        mHPolicy = hPolicy;
        mVPolicy = vPolicy;
        mScrollbarWidth = 12;
        mUpButtonPressed = false;
        mDownButtonPressed = false;
        mLeftButtonPressed = false;
        mRightButtonPressed = false;
        mUpButtonScrollAmount = 10;
        mDownButtonScrollAmount = 10;
        mLeftButtonScrollAmount = 10;
        mRightButtonScrollAmount = 10;
        mIsVerticalMarkerDragged = false;
        mIsHorizontalMarkerDragged = false;
        mOpaque = true;
        mBackgroundColor = 0x131313;

        fTimerV = fTimerH = 0;

        if (content) {
            setContent(content);
        }
        addMouseListener(this);
    }

    ScrollArea::~ScrollArea() {
        setContent(NULL);
    }

    void ScrollArea::setContent(gcn::Widget *widget) {
        if (widget != NULL) {
            clear();
            add(widget, false);
            widget->setPosition(0, 0);
        } else {
            clear();
        }

        checkPolicies();
    }

    gcn::Widget *ScrollArea::getContent() {
        if (mWidgets.size() > 0) {
            return *mWidgets.begin();
        }

        return NULL;
    }

    void ScrollArea::setHorizontalScrollPolicy(ScrollPolicy hPolicy) {
        mHPolicy = hPolicy;
        checkPolicies();
    }

    ScrollArea::ScrollPolicy ScrollArea::getHorizontalScrollPolicy() const {
        return mHPolicy;
    }

    void ScrollArea::setVerticalScrollPolicy(ScrollPolicy vPolicy) {
        mVPolicy = vPolicy;
        checkPolicies();
    }

    ScrollArea::ScrollPolicy ScrollArea::getVerticalScrollPolicy() const {
        return mVPolicy;
    }

    void ScrollArea::setScrollPolicy(ScrollPolicy hPolicy, ScrollPolicy vPolicy) {
        mHPolicy = hPolicy;
        mVPolicy = vPolicy;
        checkPolicies();
    }

    void ScrollArea::setVerticalScrollAmount(int vScroll) {
        int max = getVerticalMaxScroll();

        mVScroll = vScroll;

        if (vScroll > max) {
            mVScroll = max;
        }

        if (vScroll < 0) {
            mVScroll = 0;
        }
    }

    int ScrollArea::getVerticalScrollAmount() const {
        return mVScroll;
    }

    void ScrollArea::setHorizontalScrollAmount(int hScroll) {
        int max = getHorizontalMaxScroll();

        mHScroll = hScroll;

        if (hScroll > max) {
            mHScroll = max;
        } else if (hScroll < 0) {
            mHScroll = 0;
        }
    }

    int ScrollArea::getHorizontalScrollAmount() const {
        return mHScroll;
    }

    void ScrollArea::setScrollAmount(int hScroll, int vScroll) {
        setHorizontalScrollAmount(hScroll);
        setVerticalScrollAmount(vScroll);
    }

    int ScrollArea::getHorizontalMaxScroll() {
        checkPolicies();

        if (getContent() == NULL) {
            return 0;
        }

        int value = getContent()->getWidth() - getChildrenArea().width +
                    2 * getContent()->getFrameSize();

        if (value < 0) {
            return 0;
        }

        return value;
    }

    int ScrollArea::getVerticalMaxScroll() {
        checkPolicies();

        if (getContent() == NULL) {
            return 0;
        }

        int value;

        value = getContent()->getHeight() - getChildrenArea().height +
                2 * getContent()->getFrameSize();

        if (value < 0) {
            return 0;
        }

        return value;
    }

    void ScrollArea::setScrollbarWidth(int width) {
        if (width > 0) {
            mScrollbarWidth = width;
        } else {
            throw GCN_EXCEPTION("Width should be greater then 0.");
        }
    }

    int ScrollArea::getScrollbarWidth() const {
        return mScrollbarWidth;
    }

    void ScrollArea::mousePressed(MouseEvent &mouseEvent) {
        int x = mouseEvent.getX();
        int y = mouseEvent.getY();

        if (getVerticalMarkerDimension().isPointInRect(x, y)) {
            mIsHorizontalMarkerDragged = false;
            mIsVerticalMarkerDragged = true;

            mVerticalMarkerDragOffset = y - getVerticalMarkerDimension().y;
        } else if (getVerticalBarDimension().isPointInRect(x, y)) {
            if (y < getVerticalMarkerDimension().y) {
                setVerticalScrollAmount(getVerticalScrollAmount()
                                        - (int) (getChildrenArea().height * 0.95));
            } else {
                setVerticalScrollAmount(getVerticalScrollAmount()
                                        + (int) (getChildrenArea().height * 0.95));
            }
        } else if (getHorizontalMarkerDimension().isPointInRect(x, y)) {
            mIsHorizontalMarkerDragged = true;
            mIsVerticalMarkerDragged = false;

            mHorizontalMarkerDragOffset = x - getHorizontalMarkerDimension().x;
        } else if (getHorizontalBarDimension().isPointInRect(x, y)) {
            if (x < getHorizontalMarkerDimension().x) {
                setHorizontalScrollAmount(getHorizontalScrollAmount()
                                          - (int) (getChildrenArea().width * 0.95));
            } else {
                setHorizontalScrollAmount(getHorizontalScrollAmount()
                                          + (int) (getChildrenArea().width * 0.95));
            }
        }
    }

    void ScrollArea::mouseReleased(MouseEvent &mouseEvent) {
        mUpButtonPressed = false;
        mDownButtonPressed = false;
        mLeftButtonPressed = false;
        mRightButtonPressed = false;
        mIsHorizontalMarkerDragged = false;
        mIsVerticalMarkerDragged = false;

        mouseEvent.consume();
    }

    void ScrollArea::mouseDragged(DragEvent &mouseEvent) {
        if (mIsVerticalMarkerDragged) {
            int pos = mouseEvent.getY() - getVerticalBarDimension().y - mVerticalMarkerDragOffset;
            int length = getVerticalMarkerDimension().height;

            gcn::Rectangle barDim = getVerticalBarDimension();

            if ((barDim.height - length) > 0) {
                setVerticalScrollAmount((getVerticalMaxScroll() * pos)
                                        / (barDim.height - length));
            } else {
                setVerticalScrollAmount(0);
            }
        }

        if (mIsHorizontalMarkerDragged) {
            int pos = mouseEvent.getX() - getHorizontalBarDimension().x - mHorizontalMarkerDragOffset;
            int length = getHorizontalMarkerDimension().width;

            gcn::Rectangle barDim = getHorizontalBarDimension();

            if ((barDim.width - length) > 0) {
                setHorizontalScrollAmount((getHorizontalMaxScroll() * pos)
                                          / (barDim.width - length));
            } else {
                setHorizontalScrollAmount(0);
            }
        }

        mouseEvent.consume();
    }

    void ScrollArea::mouseExited(gcn::MouseEvent &mouseEvent) {
        for (MouseListener* ml : getContent()->_getMouseListeners()) {
            ml->mouseExited(mouseEvent);
        }
        mouseEvent.consume();
    }

    void ScrollArea::draw(Graphics *graphics) {
        drawBackground(graphics);

        int dx, dy;
        getAbsolutePosition(dx, dy);

        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        /*if (mHBarVisible && mVBarVisible) {
            graphics->setColor(getBaseColor());
            graphics->fillRectangle(gcn::Rectangle(getWidth() - mScrollbarWidth,
                                              getHeight() - mScrollbarWidth,
                                              mScrollbarWidth,
                                              mScrollbarWidth));
        }*/

        drawChildren(graphics);

        if (mVBarVisible) {
            GV->SliderDrawBar(dx + getWidth() - 16, dy - 1, true, getHeight() + 2, 0, 0xFFFFFFFF);

            gcn::Rectangle markDim = getVerticalMarkerDimension();
            markDim.y -= 1;
            markDim.height += 2;
            bool markerFocus = markDim.isPointInRect(mx - dx, my - dy);

            if (markerFocus && hge->Input_GetKeyState(HGEK_LBUTTON) && fTimerV < 0.4f) {
                fTimerV += hge->Timer_GetDelta();
                if (fTimerV > 0.4f) fTimerV = 0.4f;
            } else if (!(markerFocus && hge->Input_GetKeyState(HGEK_LBUTTON)) && fTimerV > 0.2f) {
                fTimerV -= hge->Timer_GetDelta();
                if (fTimerV < 0.2f) fTimerV = 0.2f;
            } else if (isFocused() && fTimerV < 0.2f) {
                fTimerV += hge->Timer_GetDelta();
                if (fTimerV > 0.2f) fTimerV = 0.2f;
            } else if (!isFocused() && fTimerV > 0.0f) {
                fTimerV -= hge->Timer_GetDelta();
                if (fTimerV < 0.0f) fTimerV = 0.0f;
            }

            GV->SliderDrawBar(dx + getWidth() - 16, dy + markDim.y, true, markDim.height, 1, 0xFFFFFFFF);
            if (fTimerV > 0.0f) {
                GV->SliderDrawBar(dx + getWidth() - 16, dy + markDim.y, true, markDim.height, 2,
                        SETA(0xFFFFFF, (unsigned char) (fTimerV * 2.5f * 255.0f)));
            }
        }

        if (mHBarVisible) {
            GV->SliderDrawBar(dx - 1, dy + getHeight() - 16, false, getWidth() + 2, 0, 0xFFFFFFFF);

            gcn::Rectangle markDim = getHorizontalMarkerDimension();
            markDim.x -= 1;
            markDim.width += 2;
            bool markerFocus = markDim.isPointInRect(mx - dx, my - dy);

            if (markerFocus && hge->Input_GetKeyState(HGEK_LBUTTON) && fTimerH < 0.4f) {
                fTimerH += hge->Timer_GetDelta();
                if (fTimerH > 0.4f) fTimerH = 0.4f;
            } else if (!(markerFocus && hge->Input_GetKeyState(HGEK_LBUTTON)) && fTimerH > 0.2f) {
                fTimerH -= hge->Timer_GetDelta();
                if (fTimerH < 0.2f) fTimerH = 0.2f;
            } else if (markerFocus && fTimerH < 0.2f) {
                fTimerH += hge->Timer_GetDelta();
                if (fTimerH > 0.2f) fTimerH = 0.2f;
            } else if (!markerFocus && fTimerH > 0.0f) {
                fTimerH -= hge->Timer_GetDelta();
                if (fTimerH < 0.0f) fTimerH = 0.0f;
            }

            GV->SliderDrawBar(dx + markDim.x, dy + getHeight() - 16, false, markDim.width, 1, 0xFFFFFFFF);
            if (fTimerH > 0.0f) {
                GV->SliderDrawBar(dx + markDim.x, dy + getHeight() - 16, false, markDim.width, 2,
                        SETA(0xFFFFFF, (unsigned char) (fTimerH * 2.5f * 255.0f)));
            }
        }
    }

    void ScrollArea::drawBackground(Graphics *graphics) {
        if (isOpaque()) {
            Color color(getBackgroundColor());
            color.a = getAlpha();
            graphics->setColor(color);
            graphics->fillRectangle(getChildrenArea());
        }
    }

    void ScrollArea::logic() {
        checkPolicies();

        setVerticalScrollAmount(getVerticalScrollAmount());
        setHorizontalScrollAmount(getHorizontalScrollAmount());

        if (getContent() != NULL) {
            getContent()->setPosition(-mHScroll + getContent()->getFrameSize(),
                                      -mVScroll + getContent()->getFrameSize());
            getContent()->logic();
        }
    }

    void ScrollArea::checkPolicies() {
        int w = getWidth();
        int h = getHeight();

        mHBarVisible = false;
        mVBarVisible = false;


        if (!getContent()) {
            mHBarVisible = (mHPolicy == SHOW_ALWAYS);
            mVBarVisible = (mVPolicy == SHOW_ALWAYS);
            return;
        }

        if (mHPolicy == SHOW_AUTO &&
            mVPolicy == SHOW_AUTO) {
            if (getContent()->getWidth() <= w
                && getContent()->getHeight() <= h) {
                mHBarVisible = false;
                mVBarVisible = false;
            }

            if (getContent()->getWidth() > w) {
                mHBarVisible = true;
            }

            if ((getContent()->getHeight() > h)
                || (mHBarVisible && getContent()->getHeight() > h - mScrollbarWidth)) {
                mVBarVisible = true;
            }

            if (mVBarVisible && getContent()->getWidth() > w - mScrollbarWidth) {
                mHBarVisible = true;
            }

            return;
        }

        switch (mHPolicy) {
            case SHOW_NEVER:
                mHBarVisible = false;
                break;

            case SHOW_ALWAYS:
                mHBarVisible = true;
                break;

            case SHOW_AUTO:
                if (mVPolicy == SHOW_NEVER) {
                    mHBarVisible = getContent()->getWidth() > w;
                } else // (mVPolicy == SHOW_ALWAYS)
                {
                    mHBarVisible = getContent()->getWidth() > w - mScrollbarWidth;
                }
                break;

            default:
                throw GCN_EXCEPTION("Horizontal scroll policy invalid.");
        }

        switch (mVPolicy) {
            case SHOW_NEVER:
                mVBarVisible = false;
                break;

            case SHOW_ALWAYS:
                mVBarVisible = true;
                break;

            case SHOW_AUTO:
                if (mHPolicy == SHOW_NEVER) {
                    mVBarVisible = getContent()->getHeight() > h;
                } else // (mHPolicy == SHOW_ALWAYS)
                {
                    mVBarVisible = getContent()->getHeight() > h - mScrollbarWidth;
                }
                break;
            default:
                throw GCN_EXCEPTION("Vertical scroll policy invalid.");
        }
    }

    gcn::Rectangle ScrollArea::getChildrenArea() {
        gcn::Rectangle area = gcn::Rectangle(0,
                                   0,
                                   mVBarVisible ? getWidth() - mScrollbarWidth : getWidth(),
                                   mHBarVisible ? getHeight() - mScrollbarWidth : getHeight());

        if (area.width < 0 || area.height < 0)
            return gcn::Rectangle();

        return area;
    }

    gcn::Rectangle ScrollArea::getVerticalBarDimension() {
        if (!mVBarVisible) {
            return gcn::Rectangle(0, 0, 0, 0);
        }

        if (mHBarVisible) {
            return gcn::Rectangle(getWidth() - mScrollbarWidth, 0, mScrollbarWidth, getHeight() - mScrollbarWidth);
        }

        return gcn::Rectangle(getWidth() - mScrollbarWidth, 0, mScrollbarWidth, getHeight());
    }

    gcn::Rectangle ScrollArea::getHorizontalBarDimension() {
        if (!mHBarVisible) {
            return gcn::Rectangle(0, 0, 0, 0);
        }

        if (mVBarVisible) {
            return gcn::Rectangle(0, getHeight() - mScrollbarWidth, getWidth() - mScrollbarWidth, mScrollbarWidth);
        }

        return gcn::Rectangle(0, getHeight() - mScrollbarWidth, getWidth(), mScrollbarWidth);
    }

    gcn::Rectangle ScrollArea::getVerticalMarkerDimension() {
        if (!mVBarVisible) {
            return gcn::Rectangle(0, 0, 0, 0);
        }

        int length, pos;
        gcn::Rectangle barDim = getVerticalBarDimension();

        if (getContent() && getContent()->getHeight() != 0) {
            length = (barDim.height * getChildrenArea().height)
                     / getContent()->getHeight();
        } else {
            length = barDim.height;
        }

        if (length < mScrollbarWidth) {
            length = mScrollbarWidth;
        }

        if (length > barDim.height) {
            length = barDim.height;
        }

        if (getVerticalMaxScroll() != 0) {
            pos = ((barDim.height - length) * getVerticalScrollAmount())
                  / getVerticalMaxScroll();
        } else {
            pos = 0;
        }

        return gcn::Rectangle(barDim.x, barDim.y + pos, mScrollbarWidth, length);
    }

    gcn::Rectangle ScrollArea::getHorizontalMarkerDimension() {
        if (!mHBarVisible) {
            return gcn::Rectangle(0, 0, 0, 0);
        }

        int length, pos;
        gcn::Rectangle barDim = getHorizontalBarDimension();

        if (getContent() && getContent()->getWidth() != 0) {
            length = (barDim.width * getChildrenArea().width)
                     / getContent()->getWidth();
        } else {
            length = barDim.width;
        }

        if (length < mScrollbarWidth) {
            length = mScrollbarWidth;
        }

        if (length > barDim.width) {
            length = barDim.width;
        }

        if (getHorizontalMaxScroll() != 0) {
            pos = ((barDim.width - length) * getHorizontalScrollAmount())
                  / getHorizontalMaxScroll();
        } else {
            pos = 0;
        }

        return gcn::Rectangle(barDim.x + pos, barDim.y, length, mScrollbarWidth);
    }

    void ScrollArea::showWidgetPart(gcn::Widget *widget, gcn::Rectangle area) {
        if (widget != getContent()) {
            throw GCN_EXCEPTION("Widget not content widget");
        }

        BasicContainer::showWidgetPart(widget, area);

        setHorizontalScrollAmount(getContent()->getFrameSize() - getContent()->getX());
        setVerticalScrollAmount(getContent()->getFrameSize() - getContent()->getY());
    }

    gcn::Widget *ScrollArea::getWidgetAt(int x, int y) {
        if (getChildrenArea().isPointInRect(x, y)) {
            return getContent();
        }

        return NULL;
    }

    void ScrollArea::mouseWheelMovedUp(MouseEvent &mouseEvent) {
        if (mouseEvent.isConsumed()) {
            return;
        }

        setVerticalScrollAmount(getVerticalScrollAmount() - getChildrenArea().height / 8);

        mouseEvent.consume();
    }

    void ScrollArea::mouseWheelMovedDown(MouseEvent &mouseEvent) {
        if (mouseEvent.isConsumed()) {
            return;
        }

        setVerticalScrollAmount(getVerticalScrollAmount() + getChildrenArea().height / 8);

        mouseEvent.consume();
    }

    void ScrollArea::setWidth(int width) {
        Widget::setWidth(width);
        checkPolicies();
    }

    void ScrollArea::setHeight(int height) {
        Widget::setHeight(height);
        checkPolicies();
    }

    void ScrollArea::setDimension(const gcn::Rectangle &dimension) {
        Widget::setDimension(dimension);
        checkPolicies();
    }

    void ScrollArea::setOpaque(bool opaque) {
        mOpaque = opaque;
    }


    bool ScrollArea::isOpaque() const {
        return mOpaque;
    }
}
