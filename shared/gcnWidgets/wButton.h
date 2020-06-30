#ifndef SHR_BUT_H
#define SHR_BUT_H

#include <string>

#include "guichan/focuslistener.hpp"
#include "guichan/graphics.hpp"
#include "guichan/keylistener.hpp"
#include "guichan/mouseevent.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"
#include "guichan/widget.hpp"

#include "cTooltip.h"

#include <hgeSprite.h>

#define GCNBUTBLINKDELAY 0.5f

using namespace gcn;

struct cInterfaceSheet;

namespace SHR {
    class GCN_CORE_DECLSPEC But : public gcn::Widget,
                                  public MouseListener,
                                  public KeyListener,
                                  public FocusListener
        //public cTooltip
    {
    public:
        But(cInterfaceSheet *Parts);

        But(cInterfaceSheet *Parts, const std::string &caption);

        But(cInterfaceSheet *Parts, hgeSprite *psprIcon);

        void setCaption(const std::string &caption);

        const std::string &getCaption() const;

        void setAlignment(Graphics::Alignment alignment);

        Graphics::Alignment getAlignment() const;

        void setSpacing(unsigned int spacing);

        unsigned int getSpacing() const;

        void adjustSize();

        void logic() override;

        void draw(Graphics *graphics) override;

        void focusGained(const FocusEvent &event) override;

        void focusLost(const FocusEvent &event) override;

        void mousePressed(MouseEvent &mouseEvent) override;

        void mouseReleased(MouseEvent &mouseEvent) override;

        void mouseEntered(MouseEvent &mouseEvent) override;

        void mouseExited(MouseEvent &mouseEvent) override;

        void mouseDragged(DragEvent &mouseEvent) override;

        void keyPressed(KeyEvent &keyEvent) override;

        void keyReleased(KeyEvent &keyEvent) override;

        hgeSprite *getIcon() { return sprIcon; };

        void setIcon(hgeSprite *spr) { sprIcon = spr; };

        bool isPressed() const;

        void simulatePress();

        void setHighlight(bool b) { bBlinkState = b; };

        bool isHighlighted() { return bBlinkState; };

        bool mouseOver() { return mHasMouse; };

        void setIconColor(DWORD col) { colIcon = col; };

        DWORD getIconColor() { return colIcon; };

        void setRenderBG(bool b) { bRenderBG = b; };

        bool isRenderingBG() { return bRenderBG; };

        static void drawButton(cInterfaceSheet *hSheet, int iState, int iX, int iY, int iW, int iH, DWORD dwCol);

    protected:
        cInterfaceSheet *hGfx;
        std::string mCaption;

        hgeSprite *sprIcon;
        DWORD colIcon;
        bool bBlinkState;
        bool mHasMouse;
        bool mKeyboardFocus;
        bool mMousePressed;
        Graphics::Alignment mAlignment;
        unsigned int mSpacing;
        float fTimer;
        bool bRenderBG;
    };
}

#endif
