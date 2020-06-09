#ifndef SHR_CBOX_H
#define SHR_CBOX_H

#include <string>
#include "guichan/focuslistener.hpp"
#include "guichan/keylistener.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"
#include "guichan/widget.hpp"

struct cInterfaceSheet;

using namespace gcn;

namespace SHR {
    class GCN_CORE_DECLSPEC CBox :
            public gcn::Widget,
            public MouseListener,
            public KeyListener,
            public FocusListener
    {
    public:
        CBox();

        CBox(cInterfaceSheet *Parts, const std::string &caption, bool selected = false);

        virtual ~CBox() {}

        bool isSelected() const;

        void setSelected(bool selected);

        const std::string &getCaption() const;

        void setCaption(const std::string &caption);

        void adjustSize();

        virtual void draw(Graphics *graphics);

        virtual void keyPressed(KeyEvent &keyEvent);

        virtual void mouseClicked(MouseEvent &mouseEvent);

        virtual void mouseDragged(MouseEvent &mouseEvent);

        virtual void mouseEntered(MouseEvent &mouseEvent);

        virtual void mouseExited(MouseEvent &mouseEvent);

        virtual void focusGained(const FocusEvent& event);
        virtual void focusLost(const FocusEvent& event);

        void SetGfx(cInterfaceSheet *Parts) { hGfx = Parts; };

        void simulateSwitch(bool on) {
            setSelected(on);
            distributeActionEvent();
        }

    protected:
        virtual void toggleSelected();

        bool mSelected;
        std::string mCaption;
        cInterfaceSheet *hGfx;
        float fTimer;
        bool bMouseOver;
        bool mKeyboardFocus;
    };
}

#endif
