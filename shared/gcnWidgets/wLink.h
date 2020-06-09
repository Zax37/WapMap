#ifndef SHR_LINK_H
#define SHR_LINK_H

#include <string>

#include "guichan/graphics.hpp"
#include "guichan/platform.hpp"
#include "guichan/widget.hpp"
#include "guichan/mouselistener.hpp"

using namespace gcn;

class hgeSprite;

namespace SHR {
    /**
     * Implementation of a Link capable of displaying a caption.
     */
    class GCN_CORE_DECLSPEC Link : public gcn::Widget,
                                   public MouseListener {
    public:
        /**
         * Constructor.
         */
        Link();

        /**
         * Constructor. The Link will be automatically resized
         * to fit the caption.
         *
         * @param caption The caption of the Link.
         */
        Link(const std::string &caption, hgeSprite *spr = 0);

        /**
         * Gets the caption of the Link.
         *
         * @return The caption of the Link.
         * @see setCaption
         */
        const std::string &getCaption() const;

        void setIcon(hgeSprite *spr);

        hgeSprite *getIcon() { return sprIcon; };

        /**
         * Sets the caption of the Link. It's advisable to call
         * adjustSize after setting of the caption to adjust the
         * Link's size to fit the caption.
         *
         * @param caption The caption of the Link.
         * @see getCaption, adjustSize
         */
        void setCaption(const std::string &caption);

        /**
         * Sets the alignment of the caption. The alignment is relative
         * to the center of the Link.
         *
         * @param alignemnt The alignment of the caption of the Link.
         * @see getAlignment, Graphics
         */
        void setAlignment(Graphics::Alignment alignment);

        /**
         * Gets the alignment of the caption. The alignment is relative to
         * the center of the Link.
         *
         * @return The alignment of caption of the Link.
         * @see setAlignmentm Graphics
         */
        Graphics::Alignment getAlignment() const;

        /**
         * Adjusts the Link's size to fit the caption.
         */
        void adjustSize();


        // Inherited from Widget

        virtual void draw(Graphics *graphics);

        virtual void mousePressed(MouseEvent &mouseEvent);

        virtual void mouseEntered(MouseEvent &mouseEvent);

        virtual void mouseExited(MouseEvent &mouseEvent);

        virtual void mouseReleased(MouseEvent &mouseEvent);

        virtual void mouseDragged(MouseEvent &mouseEvent);

    protected:
        /**
         * Holds the caption of the Link.
         */
        std::string mCaption;

        /**
         * Holds the alignment of the caption.
         */
        Graphics::Alignment mAlignment;

        float fFocusTimer;
        hgeSprite *sprIcon;

        bool bMouseOver, mMousePressed;
    };
}

#endif // end GCN_Link_HPP
