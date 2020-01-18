#ifndef SHR_WINMGR_H
#define SHR_WINMGR_H

#include <list>

#include "guichan/basiccontainer.hpp"
#include "guichan/graphics.hpp"
#include "guichan/platform.hpp"

using namespace gcn;

namespace SHR
{
    /**
     * An implementation of a WinMgr able to contain other widgets. A widget's
     * position in the WinMgr is relative to the WinMgr itself and not the screen.
     * A WinMgr is the most common widget to use as the Gui's top widget as makes the Gui
     * able to contain more than one widget.
     *
     * @see Gui::setTop
     */
    class GCN_CORE_DECLSPEC WinMgr: public BasicContainer
    {
    public:

        /**
         * Constructor. A WinMgr is opauqe as default, if you want a
         * none opaque WinMgr call setQpaque(false).
         *
         * @see setOpaque, isOpaque
         */
        WinMgr();

        /**
         * Destructor.
         */
        virtual ~WinMgr();

        /**
         * Sets the WinMgr to be opaque or not. If the WinMgr
         * is opaque its background will be drawn, if it's not opaque
         * its background will not be drawn, and thus making the WinMgr
         * completely transparent.
         *
         * NOTE: This is not the same as to set visibility. A non visible
         *       WinMgr will not itself nor will it draw its content.
         *
         * @param opaque True if the WinMgr should be opaque, false otherwise.
         * @see isOpaque
         */
        void setOpaque(bool opaque);

        /**
         * Checks if the WinMgr is opaque or not.
         *
         * @return True if the WinMgr is opaque, false otherwise.
         * @see setOpaque
         */
        bool isOpaque() const;

        /**
         * Adds a widget to the WinMgr.
         *
         * @param widget The widget to add.
         * @see remove, clear
         */
        virtual void add(Widget* widget);

        /**
         * Adds a widget to the WinMgr and also specifies the widget's
         * position in the WinMgr. The position is relative to the WinMgr
         * and not relative to the screen.
         *
         * @param widget The widget to add.
         * @param x The x coordinate for the widget.
         * @param y The y coordinate for the widget.
         * @see remove, clear
         */
        virtual void add(Widget* widget, int x, int y);

        /**
         * Removes a widget from the WinMgr.
         *
         * @param widget The widget to remove.
         * @throws Exception when the widget has not been added to the
         *                   WinMgr.
         * @see add, clear
         */
        virtual void remove(Widget* widget);

        /**
         * Clears the WinMgr of all widgets.
         *
         * @see add, remove
         */
        virtual void clear();

        /**
         * Finds a widget given an id.
         *
         * @param id The id to find a widget by.
         * @return A widget with a corrosponding id, NULL if no widget
         *         is found.
         * @see Widget::setId
         */
        virtual Widget* findWidgetById(const std::string &id);


        // Inherited from Widget

        virtual void draw(Graphics* graphics);

    protected:
        /**
         * True if the WinMgr is opaque, false otherwise.
         */
        bool mOpaque;
    };
}

#endif // end GCN_WinMgr_HPP
