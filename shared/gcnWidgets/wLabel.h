#ifndef SHR_LAB_H
#define SHR_LAB_H

#include <string>

#include "guichan/graphics.hpp"
#include "guichan/platform.hpp"
#include "guichan/widget.hpp"

#ifndef byte
typedef unsigned char byte;
typedef unsigned long DWORD;
#endif

using namespace gcn;

namespace SHR {
    /**
     * Implementation of a Lab capable of displaying a caption.
     */
    class GCN_CORE_DECLSPEC Lab : public gcn::Widget {
    public:
        /**
         * Constructor.
         */
        Lab();

        /**
         * Constructor. The Lab will be automatically resized
         * to fit the caption.
         *
         * @param caption The caption of the Lab.
         */
        Lab(const std::string &caption);

        /**
         * Gets the caption of the Lab.
         *
         * @return The caption of the Lab.
         * @see setCaption
         */
        const std::string &getCaption() const;

        /**
         * Sets the caption of the Lab. It's advisable to call
         * adjustSize after setting of the caption to adjust the
         * Lab's size to fit the caption.
         *
         * @param caption The caption of the Lab.
         * @see getCaption, adjustSize
         */
        void setCaption(const std::string &caption);

        /**
         * Sets the alignment of the caption. The alignment is relative
         * to the center of the Lab.
         *
         * @param alignemnt The alignment of the caption of the Lab.
         * @see getAlignment, Graphics
         */
        void setAlignment(Graphics::Alignment alignment);

        /**
         * Gets the alignment of the caption. The alignment is relative to
         * the center of the Lab.
         *
         * @return The alignment of caption of the Lab.
         * @see setAlignmentm Graphics
         */
        Graphics::Alignment getAlignment() const;


        void adjustSize();

        void setColor(DWORD col) { mColor = col; };

        DWORD getColor() { return mColor; };


        // Inherited from Widget

        virtual void draw(Graphics *graphics);

    protected:

        std::string mCaption;
        Graphics::Alignment mAlignment;
        DWORD mColor;
    };
}

#endif // end GCN_Lab_HPP
