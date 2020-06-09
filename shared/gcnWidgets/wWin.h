#ifndef SHR_WIN_H
#define SHR_WIN_H

#include <string>

#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"
#include "wContainer.h"

#include "../cGUIparts.h"

using namespace gcn;

namespace SHR {
    class GCN_CORE_DECLSPEC Win : public Container,
                                  public MouseListener {
    public:
        Win(guiParts *Parts);

        Win(guiParts *Parts, const std::string &caption);

        virtual ~Win();

        void setCaption(const std::string &caption);

        const std::string &getCaption() const;

        void setAlignment(Graphics::Alignment alignment);

        Graphics::Alignment getAlignment() const;

        void setPadding(unsigned int padding);

        unsigned int getPadding() const;

        void setTitleBarHeight(unsigned int height);

        unsigned int getTitleBarHeight();

        void setMovable(bool movable);

        bool isMovable() const;

        void setOpaque(bool opaque);

        bool isOpaque();

        void setClose(bool pbEnable) { bClose = pbEnable; };

        bool isClose() { return bClose; };

        virtual void resizeToContent();

        virtual gcn::Rectangle getChildrenArea();

        virtual void draw(Graphics *graphics);

        virtual void mousePressed(MouseEvent &mouseEvent);

        virtual void mouseDragged(MouseEvent &mouseEvent);

        virtual void mouseReleased(MouseEvent &mouseEvent);

        virtual void mouseEntered(MouseEvent &mouseEvent);

        virtual void mouseExited(MouseEvent &mouseEvent);

        bool hasShadow() { return bDrawShadow; };

        void setShadow(bool b) { bDrawShadow = b; };

		bool gotFocus();

		void unDrag() {
            mMoved = false;
            _getFocusHandler()->setDraggedWidget(NULL);
		}

    protected:
        guiParts *hGfx;
        std::string mCaption;
        Graphics::Alignment mAlignment;
        unsigned int mPadding;
        unsigned int mTitleBarHeight;
        bool mMovable;
        bool mOpaque;
        int mDragOffsetX;
        int mDragOffsetY;
        bool mMoved;
        bool mMouseOver;
        bool bClose;
        bool bDrawShadow;
        float fCloseTimer;
    };
}

#endif // end GCN_Win_HPP
