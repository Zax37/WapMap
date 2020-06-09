#ifndef H_SHARED_GUIPARTS
#define H_SHARED_GUIPARTS

#include "hge.h"
#include "guichan.hpp"
#include "guichan/hge.hpp"

namespace SHR {
    struct guiParts {
    public:
        hgeSprite *sprPBarFL, *sprPBarFM, *sprPBarFR, *sprPBarEL, *sprPBarEM, *sprPBarER;

        hgeSprite *sprIconInfo, *sprIconWarning, *sprIconError;

        gcn::HGEImageFont *gcnfntMyriad13, *gcnfntMyriad10, *gcnfntSystem;
    };
};

#endif
