#ifndef H_C_OE_STALACTITE
#define H_C_OE_STALACTITE

#include "../states/objedit.h"
#include "procPickRect.h"

namespace ObjEdit {
    class cEditObjStalactite : public cObjEdit {
    private:
        SHR::Lab *labActivate;
        cProcPickRect *hRectPick;

        friend class cObjPropVP;

    protected:
        virtual void _Think(bool bMouseConsumed);

    public:
        cEditObjStalactite(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjStalactite();

        virtual void Action(const gcn::ActionEvent &actionEvent);
    };

}

#endif
