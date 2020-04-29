#ifndef H_C_OE_WARP
#define H_C_OE_WARP

#include "../states/objedit.h"

namespace ObjEdit {
    class cEditObjWarp : public cObjEdit {
    private:
        SHR::RadBut *rbType[3];
        SHR::But *butPick;
        SHR::TextField *tfX, *tfY;
        float fCpAnimTimer, fScpAnimTimer;
        int iCpAnimFrame, iScpAnimFrame;

        void RebuildWindow();

        SHR::CBox *cbOneTime;

        friend class cObjPropVP;

    public:
        cEditObjWarp(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjWarp();

        virtual void Save();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();

        bool bPick;
    };

}

#endif
