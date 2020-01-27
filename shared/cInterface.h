#ifndef H_SHARED_INTERFACE
#define H_SHARED_INTERFACE

#include "cGUIparts.h"

namespace SHR {
    class Interface {
    private:
        guiParts *hGfx;
        bool bShowMouse;
    public:
        Interface(guiParts *Parts);

        void Think();

        void Render(bool pbHand = 0, DWORD dwCol = 0xFFFFFFFF);

        void EnableCursor(bool bShow) { bShowMouse = bShow; };

        bool IsMouseEnabled() { return bShowMouse; };

        hgeSprite *GetCursor();
    };
}

#endif
