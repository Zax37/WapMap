#ifndef H_C_OE_SPRINGBOARD
#define H_C_OE_SPRINGBOARD

#include "../states/objedit.h"

namespace ObjEdit {
    class cEditObjSpringboard : public cObjEdit {
    private:
        SHR::Lab *labJumpHeight;
        SHR::TextField *tfJumpHeight;
        SHR::But *butPick;
        SHR::Win *win;

        friend class cObjPropVP;

    protected:
        virtual void _Think(bool bMouseConsumed);

    public:
        cEditObjSpringboard(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjSpringboard();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        bool bPicking;
        int iPickY;
    };

}

#endif
