#ifndef SHR_INV_H
#define SHR_INV_H

#include "../cGUIparts.h"

#include <string>

#include "guichan/focuslistener.hpp"
#include "guichan/graphics.hpp"
#include "guichan/keylistener.hpp"
#include "guichan/mouseevent.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"
#include "guichan/widget.hpp"

#include "cTooltip.h"

#include "../../WapMap/cInventoryController.h"

using namespace gcn;

namespace SHR
{
    class GCN_CORE_DECLSPEC InvTab : public gcn::Widget,
                                     public MouseListener
                                     //public cTooltip
    {
    public:
        InvTab(guiParts * Parts);

        void adjustSize();

        virtual void draw(Graphics* graphics);

        virtual void mousePressed(MouseEvent& mouseEvent);
        virtual void mouseReleased(MouseEvent& mouseEvent);
        virtual void mouseEntered(MouseEvent& mouseEvent);
        virtual void mouseExited(MouseEvent& mouseEvent);
        virtual void mouseDragged(MouseEvent& mouseEvent);

        cInventoryItem GetItem(){ return mItem; };
        void ObjectDropped();

        void SetItem(cInventoryItem it){ mItem = it; };

    protected:
        guiParts * hGfx;

        bool mHasMouse;
        bool mKeyPressed;
        bool mMousePressed;
        cInventoryItem mItem;
    };
}

#endif
