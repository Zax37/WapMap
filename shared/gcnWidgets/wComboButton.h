#ifndef SHR_CMBBUT_H
#define SHR_CMBBUT_H

#include <string>
#include <vector>

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
    class ComboBut;

    class ComboButEntry : cTooltip {
    private:
        float fTimer;
        int iID;
        int iWidth;
        hgeSprite *sprIcon;
        std::string strCaption;

        void UpdateWidth();

        friend class ComboBut;

    public:
        ComboButEntry(hgeSprite *ico, std::string cap, const char* tooltip = NULL);

        ComboButEntry(const ComboButEntry& other);

        void SetIcon(hgeSprite *spr);

        void SetCaption(const char* str);

        hgeSprite *GetIcon() const { return sprIcon; };

        const char* GetCaption() const { return strCaption.c_str(); };

        int GetID() { return iID; };

        int GetWidth() { return iWidth; };
    };

    class GCN_CORE_DECLSPEC ComboBut : public gcn::Widget,
                                       public MouseListener,
                                       public FocusListener,
                                       public KeyListener
        //public cTooltip
    {
    public:
        ComboBut(cInterfaceSheet *Parts);

        void adjustSize();

        void draw(Graphics *graphics) override;

        void focusGained(const FocusEvent &event) override;

        void focusLost(const FocusEvent &event) override;

        void mousePressed(MouseEvent &mouseEvent) override;

        void mouseReleased(MouseEvent &mouseEvent) override;

        void mouseEntered(MouseEvent &mouseEvent) override;

        void mouseExited(MouseEvent &mouseEvent) override;

        void keyPressed(KeyEvent &keyEvent) override;

        void addEntry(ComboButEntry n);

        int getSelectedEntryID() { return iSelectedID; };

        void setSelectedEntryID(int i, bool bEvent = 0);

    protected:
        cInterfaceSheet *hGfx;

        bool mHasMouse;
        bool mKeyboardFocus;
        bool mMousePressed;
        int iSelectedID, iFocused;
        std::vector<ComboButEntry> vEntries;
        float fTimer;
    };
}

#endif
