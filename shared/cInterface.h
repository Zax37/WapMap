#ifndef H_SHARED_INTERFACE
#define H_SHARED_INTERFACE

#include "cGUIparts.h"

namespace SHR {
    class Interface {
    private:
        guiParts *hGfx;
    public:
        Interface(guiParts *Parts);
    };
}

#endif
