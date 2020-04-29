#ifndef H_C_OE_CHECKPOINT
#define H_C_OE_CHECKPOINT

#include "../states/objedit.h"

namespace ObjEdit {
    class cEditObjCheckpoint : public cObjEdit {
    private:
        SHR::RadBut *rbType[2];
        float fCpAnimTimer, fScpAnimTimer;
        int iCpAnimFrame, iScpAnimFrame;

        friend class cObjPropVP;

    public:
        cEditObjCheckpoint(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjCheckpoint();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();
    };

}

#endif
