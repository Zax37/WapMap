#ifndef H_C_OE_CURSE
#define H_C_OE_CURSE

#include "../states/objedit.h"

namespace ObjEdit {
    class cEditObjCurse : public cObjEdit {
    private:
        SHR::RadBut *rbCurses[6];
        float fCurseAnim;
        byte iFrame;

        friend class cObjPropVP;

    public:
        cEditObjCurse(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjCurse();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();
    };

}

#endif
