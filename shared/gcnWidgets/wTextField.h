#ifndef SHR_TEXTFIELD_H
#define SHR_TEXTFIELD_H

#include "guichan/keylistener.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"
#include "guichan/widget.hpp"
#include "guichan/focuslistener.hpp"
#include "cField.h"
#include <string>

using namespace gcn;

namespace SHR {
    class GCN_CORE_DECLSPEC TextField :
            public Field,
            public gcn::Widget,
            public MouseListener,
            public KeyListener,
            public FocusListener {
    public:
        TextField();

        TextField(const std::string &text);

        void setText(const std::string &text, bool bGenerateActionEvent = false);

        const std::string& getText() const;

        void adjustSize();

        void adjustHeight();

        void setCaretPosition(unsigned int position);

        unsigned int getCaretPosition() const;

        void fontChanged() override;

        void draw(Graphics *graphics) override;

        void mouseMoved(MouseEvent &mouseEvent) override;

        void mousePressed(MouseEvent &mouseEvent) override;

        void mouseDragged(DragEvent &mouseEvent) override;

        void keyPressed(KeyEvent &keyEvent) override;

        bool IsNumericalOnly() { return bNumerical; };

        void SetNumerical(bool b, bool n = 1) {
            bNumerical = b;
            bAllowNegative = n;
        }

        void setMaxLength(int n);

        bool isMarkedInvalid() { return bMarkedInvalid; };

        void setMarkedInvalid(bool b) { bMarkedInvalid = b; };

        void focusGained(const FocusEvent &event) override;

        void focusLost(const FocusEvent &event) override;

    protected:
        void fixScroll();

        float fFocusTimer;

        int iMaxLength;
        std::string mText;
        unsigned int mCaretPosition;
        int mSelectionPosition;
        int mXScroll;
        bool bNumerical, bAllowNegative, bMarkedInvalid;

        void deleteSelection();
    };
}

#endif // end GCN_TEXTFIELD_HPP
