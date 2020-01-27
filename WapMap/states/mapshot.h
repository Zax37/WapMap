#ifndef H_STATE_INTRO
#define H_STATE_INTRO

#include "../../shared/cStateMgr.h"
#include "../../shared/gcnWidgets/wWin.h"
#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wProgressBar.h"
#include "../../shared/cWWD.h"
#include "../../shared/cProgressInfo.h"

#ifndef byte
typedef unsigned char byte;
#endif

#include <SFML/Graphics/Image.hpp>

namespace State {

    class EditingWW;

    class MapShot : public SHR::cState {
    private:
        char *szOutput;
        int iOutputOption;
        HTARGET targTemp;
        sf::Image gdDest;
        DWORD dwBg;
        bool bDrawObjects;
        HTARGET targPreview;
        hgeSprite *sprPreview;
        int iPreviewW, iPreviewH;
        float fPreviewScale;

        float fScale;
        int iPLid;
        WWD::Plane *pl;
        EditingWW *stMain;

        void UpdateScene() {
            Think();
            _ForceRender();
        };
    public:
        MapShot(State::EditingWW *mainSt, int planeIterator, float scale, const char *dest, int saveoption, DWORD bg,
                bool drawobj);

        virtual bool Opaque();

        virtual void Init();

        virtual void Destroy();

        virtual bool Think();

        virtual bool Render();

        virtual void GainFocus(int iReturnCode, bool bFlipped);

        gcn::Gui *gui;
        SHR::Win *winLoad;
        SHR::ProgressBar *barWhole, *barAction;
        SHR::Lab *labWhole, *labAction, *labDesc;
    };
};

#endif
