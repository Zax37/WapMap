#ifndef SHR_TEXTFIELD_H
#define SHR_TEXTFIELD_H

#include "guichan/keylistener.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"
#include "guichan/widget.hpp"
#include "guichan/focuslistener.hpp"

#include <string>

using namespace gcn;

namespace SHR
{
    class GCN_CORE_DECLSPEC TextField:
        public gcn::Widget,
        public MouseListener,
        public KeyListener,
        public FocusListener
    {
    public:
        TextField();
        TextField(const std::string& text);

        void setText(const std::string& text, bool bGenerateActionEvent = 0);
        const std::string& getText() const;

        void adjustSize();
        void adjustHeight();

        void setCaretPosition(unsigned int position);
        unsigned int getCaretPosition() const;

        virtual void fontChanged();
        virtual void draw(Graphics* graphics);

        virtual void mousePressed(MouseEvent& mouseEvent);
        virtual void mouseDragged(MouseEvent& mouseEvent);

        virtual void keyPressed(KeyEvent& keyEvent);

        bool IsNumericalOnly(){ return bNumerical; };
        void SetNumerical(bool b, bool n = 1){ bNumerical = b; bAllowNegative = n; }
        void setMaxLength(int n);

        bool isMarkedInvalid(){ return bMarkedInvalid; };
        void setMarkedInvalid(bool b){ bMarkedInvalid = b; };

        virtual void focusGained(const Event &event);
        virtual void focusLost(const Event &event);

        static void renderFrame(int dx, int dy, int w, int h, unsigned char alpha, bool version);

    protected:
        virtual void drawCaret(Graphics* graphics, int x);
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
