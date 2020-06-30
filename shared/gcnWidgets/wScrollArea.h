#ifndef W_SAREA_HPP
#define W_SAREA_HPP

#include <string>

#include "guichan/basiccontainer.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"

namespace SHR {
    /**
     * Implementation if a scrollable area used to view widgets larger than the scroll area.
     * A scroll area can be customized to always show scroll bars or to show them only when
     * necessary.
     */
    class GCN_CORE_DECLSPEC ScrollArea :
            public gcn::BasicContainer,
            public gcn::MouseListener {
    public:

        /**
         * Scrollpolicies for the horizontal and vertical scrollbar.
         * The policies are:
         *
         * SHOW_ALWAYS - Always show the scrollbars no matter what.
         * SHOW_NEVER  - Never show the scrollbars no matter waht.
         * SHOW_AUTO   - Show the scrollbars only when needed. That is if the
         *               content grows larger then the ScrollArea.
         */
        enum ScrollPolicy {
            SHOW_ALWAYS = 0,
            SHOW_NEVER,
            SHOW_AUTO
        };

        /**
         * Constructor.
         */
        ScrollArea();

        /**
         * Constructor.
         *
         * @param content The content of the scroll area.
         */
        ScrollArea(Widget *content);

        /**
         * Constructor.
         *
         * @param content The content of the scroll area.
         * @param hPolicy The policy for the horizontal scrollbar. See enum with
         *                policies.
         * @param vPolicy The policy for the vertical scrollbar. See enum with
         *                policies.
         */
        ScrollArea(Widget *content,
                   ScrollPolicy hPolicy,
                   ScrollPolicy vPolicy);

        /**
         * Destructor.
         */
        virtual ~ScrollArea();

        /**
         * Sets the content.
         *
         * @param widget The content of the scroll area.
         */
        void setContent(Widget *widget);

        /**
         * Gets the content.
         *
         * @return The content of the scroll area.
         */
        Widget *getContent();

        /**
         * Sets the horizontal scrollbar policy. See enum with policies.
         *
         * @param hPolicy The policy for the horizontal scrollbar.
         * @see getHorizontalScrollPolicy
         */
        void setHorizontalScrollPolicy(ScrollPolicy hPolicy);

        /**
         * Gets the horizontal scrollbar policy. See enum with policies.
         *
         * @return The policy for the horizontal scrollbar policy.
         * @see setHorizontalScrollPolicy, setScrollPolicy
         */
        ScrollPolicy getHorizontalScrollPolicy() const;

        /**
         * Sets the vertical scrollbar policy. See enum with policies.
         *
         * @param vPolicy The policy for the vertical scrollbar.
         * @see getVerticalScrollPolicy
         */
        void setVerticalScrollPolicy(ScrollPolicy vPolicy);

        /**
         * Gets the vertical scrollbar policy. See enum with policies.
         *
         * @return The policy for the vertical scrollbar.
         * @see setVerticalScrollPolicy, setScrollPolicy
         */
        ScrollPolicy getVerticalScrollPolicy() const;

        /**
         * Sets the horizontal and vertical scrollbar policy.
         *
         * @param hPolicy The policy for the horizontal scrollbar.
         * @param vPolicy The policy for the vertical scrollbar.
         * @see getVerticalScrollPolicy, getHorizontalScrollPolicy
         */
        void setScrollPolicy(ScrollPolicy hPolicy, ScrollPolicy vPolicy);

        /**
         * Sets the amount to scroll vertically.
         *
         * @param vScroll The amount to scroll.
         * @see getVerticalScrollAmount
         */
        void setVerticalScrollAmount(int vScroll);

        /**
         * Gets the amount that is scrolled vertically.
         *
         * @return The scroll amount on vertical scroll.
         * @see setVerticalScrollAmount, setScrollAmount
         */
        int getVerticalScrollAmount() const;

        /**
         * Sets the amount to scroll horizontally.
         *
         * @param hScroll The amount to scroll.
         * @see getHorizontalScrollAmount
         */
        void setHorizontalScrollAmount(int hScroll);

        /**
         * Gets the amount that is scrolled horizontally.
         *
         * @return The scroll amount on horizontal scroll.
         * @see setHorizontalScrollAmount, setScrollAmount
         */
        int getHorizontalScrollAmount() const;

        /**
         * Sets the amount to scroll horizontally and vertically.
         *
         * @param hScroll The amount to scroll on horizontal scroll.
         * @param vScroll The amount to scroll on vertical scroll.
         * @see getHorizontalScrollAmount, getVerticalScrollAmount
         */
        void setScrollAmount(int hScroll, int vScroll);

        /**
         * Gets the maximum amount of horizontal scroll.
         *
         * @return The horizontal max scroll.
         */
        int getHorizontalMaxScroll();

        /**
         * Gets the maximum amount of vertical scroll.
         *
         * @return The vertical max scroll.
         */
        int getVerticalMaxScroll();

        /**
         * Sets the width of the scroll bars.
         *
         * @param width The width of the scroll bars.
         * @see getScrollbarWidth
         */
        void setScrollbarWidth(int width);

        /**
         * Gets the width of the scroll bars.
         *
         * @return the width of the ScrollBar.
         * @see setScrollbarWidth
         */
        int getScrollbarWidth() const;

        /**
         * Sets the scroll area to be opaque, that is sets the scoll area
         * to display its background.
         *
         * @param opaque True if the scoll area should be opaque, false otherwise.
         */
        void setOpaque(bool opaque);

        /**
         * Checks if the scroll area is opaque, that is if the scroll area
         * displays its background.
         *
         * @return True if the scroll area is opaque, false otherwise.
         */
        bool isOpaque() const;


        // Inherited from BasicContainer

        virtual void showWidgetPart(Widget *widget, gcn::Rectangle area);

        virtual gcn::Rectangle getChildrenArea();

        virtual Widget *getWidgetAt(int x, int y);


        // Inherited from Widget

        virtual void draw(gcn::Graphics *graphics);

        virtual void logic();

        void setWidth(int width);

        void setHeight(int height);

        void setDimension(const gcn::Rectangle &dimension);


        // Inherited from MouseListener

        void mousePressed(gcn::MouseEvent &mouseEvent) override;

        void mouseReleased(gcn::MouseEvent &mouseEvent) override;

        void mouseDragged(gcn::DragEvent &dragEvent) override;

        void mouseWheelMovedUp(gcn::MouseEvent &mouseEvent) override;

        void mouseWheelMovedDown(gcn::MouseEvent &mouseEvent) override;

        void mouseExited(gcn::MouseEvent& mouseEvent) override;

    protected:
        /**
         * Draws the background of the scroll area, that is
         * the area behind the content.
         *
         * @param graphics a Graphics object to draw with.
         */
        virtual void drawBackground(gcn::Graphics *graphics);

        /**
         * Checks the policies for the scroll bars.
         */
        virtual void checkPolicies();

        /**
         * Gets the vertical scrollbar dimension.
         *
         * @return the dimension of the vertical scrollbar.
         */
        gcn::Rectangle getVerticalBarDimension();

        /**
         * Gets the horizontal scrollbar dimension.
         *
         * @return the dimension of the horizontal scrollbar.
         */
        gcn::Rectangle getHorizontalBarDimension();

        /**
         * Gets the vertical marker dimension.
         *
         * @return the dimension of the vertical marker.
         */
        gcn::Rectangle getVerticalMarkerDimension();

        /**
         * Gets the horizontal marker dimension.
         *
         * @return the dimension of the horizontal marker.
         */
        gcn::Rectangle getHorizontalMarkerDimension();

        /**
         * Holds the vertical scroll amount.
         */
        int mVScroll;

        /**
         * Holds the horizontal scroll amount.
         */
        int mHScroll;

        /**
         * Holds the width of the scroll bars.
         */
        int mScrollbarWidth;

        /**
         * Holds the horizontal scroll bar policy.
         */
        ScrollPolicy mHPolicy;

        /**
         * Holds the vertical scroll bar policy.
         */
        ScrollPolicy mVPolicy;

        /**
         * True if the vertical scroll bar is visible, false otherwise.
         */
        bool mVBarVisible;

        /**
         * True if the horizontal scroll bar is visible, false otherwise.
         */
        bool mHBarVisible;

        /**
         * True if the up button is pressed, false otherwise.
         */
        bool mUpButtonPressed;

        /**
         * True if the down button is pressed, false otherwise.
         */
        bool mDownButtonPressed;

        /**
         * True if the left button is pressed, false otherwise.
         */
        bool mLeftButtonPressed;

        /**
         * True if the right button is pressed, false otherwise.
         */
        bool mRightButtonPressed;

        /**
         * Holds the up button scroll amount.
         */
        int mUpButtonScrollAmount;

        /**
         * Holds the down button scroll amount.
         */
        int mDownButtonScrollAmount;

        /**
         * Holds the left button scroll amount.
         */
        int mLeftButtonScrollAmount;

        /**
         * Holds the right button scroll amount.
         */
        int mRightButtonScrollAmount;

        /**
         * True if the vertical marked is dragged.
         */
        bool mIsVerticalMarkerDragged;

        /**
         * True if the horizontal marked is dragged.
         */
        bool mIsHorizontalMarkerDragged;

        /**
         * Holds the horizontal markers drag offset.
         */
        int mHorizontalMarkerDragOffset;

        /**
         * Holds the vertical markers drag offset.
         */
        int mVerticalMarkerDragOffset;

        /**
         * True if the scroll area should be opaque (that is
         * display its background), false otherwise.
         */
        bool mOpaque;

        float fTimerV, fTimerH;
    };
}

#endif // end GCN_SCROLLAREA_HPP
