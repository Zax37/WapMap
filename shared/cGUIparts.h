#ifndef H_SHARED_GUIPARTS
#define H_SHARED_GUIPARTS

#include "hge.h"
#include "guichan.hpp"
#include "guichan/hge.hpp"

namespace SHR {
    struct guiParts {
    public:

        hgeSprite *sprCur, *sprCurHand;

        hgeSprite *sprGcnWinBarL, *sprGcnWinBarM, *sprGcnWinBarR, *sprGcnWinBarClose;
        hgeSprite *sprCheckBoxNull, *sprCheckBoxChecked;
        hgeSprite *sprRadio, *sprRadioChecked;

        hgeSprite *sprButBar[2][3], *sprButBarH[2][3], *sprButBarD[2][3], *sprButBarP[2][3];
        hgeSprite *sprButDDL, *sprButDDR, *sprButDDC, *sprArrowWhiteDD, *sprArrowGrayDD;

        hgeSprite *sprPBarFL, *sprPBarFM, *sprPBarFR, *sprPBarEL, *sprPBarEM, *sprPBarER;

        hgeSprite *sprIconInfo, *sprIconWarning, *sprIconError, *sprIconClose;

        hgeSprite *sprCBox, *sprCBoxOn;

        gcn::HGEImageFont *gcnfntMyriad13, *gcnfntMyriad10, *gcnfntSystem;

        hgeSprite *sprWindowBG;
    };
};

#endif
