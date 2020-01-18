#ifndef SHR_TAB_H
#define SHR_TAB_H

#include <map>
#include <string>

#include "wLabel.h"
#include "wTabbedArea.h"

#include "guichan/basiccontainer.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"

using namespace gcn;

namespace SHR
{
    class Label;
    class TabbedArea;

    /**
     * An implementation of a simple tab to be used in a tabbed area.
     *
     * @see TabbedArea
     * @since 0.8.0
     */
    class GCN_CORE_DECLSPEC Tab:
        public BasicContainer,
        public MouseListener
    {
    public:

        /**
         * Constructor.
         */
        Tab();

        /**
         * Destructor.
         */
        virtual ~Tab();

        /**
         * Adjusts the size of the tab to fit the caption. If this tab was
         * added to a TabbedArea, it will also adjust the tab positions.
         */
        void adjustSize();

        /**
         * Sets the tabbed area the tab should be a part of.
         *
         * @param tabbedArea The tabbed area the tab should be a part of.
         * @see getTabbedArea
         */
        void setTabbedArea(TabbedArea* tabbedArea);

        /**
         * Gets the tabbed are the tab is a part of.
         *
         * @return The tabbed are the tab is a part of.
         * @see setTabbedArea
         */
        TabbedArea* getTabbedArea();

        /**
         * Sets the caption of the tab. It's advisable to call
         * adjustSize after setting the caption to make the tab
         * fit the caption.
         *
         * @param caption The caption of the tab.
         * @see getCaption, adjustSize
         */
        void setCaption(const std::string& caption);

        /**
         * Gets the caption of the tab.
         *
         * @return The caption of the tab.
         * @see setCaption
         */
        const std::string& getCaption() const;


        // Inherited from Widget

        virtual void draw(Graphics *graphics);


        // Inherited from MouseListener

        virtual void mouseEntered(MouseEvent& mouseEvent);

        virtual void mouseExited(MouseEvent& mouseEvent);

    protected:
        /**
         * Holds the label of the tab.
         */
        Lab* mLabel;

        /**
         * True if the tab has the mouse, false otherwise.
         */
        bool mHasMouse;

        /**
         * Holds the tabbed area the tab is a part of.
         */
        TabbedArea* mTabbedArea;
    };
}

#endif // end GCN_TABBEDAREA_HPP
