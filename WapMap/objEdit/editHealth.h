#ifndef H_C_OE_HEALTH
#define H_C_OE_HEALTH

#include "../states/objedit.h"

namespace ObjEdit {
    class cEditObjHealth : public cObjEdit {
    private:
        SHR::RadBut *rbType[11];
        SHR::Win *win;

        friend class cObjPropVP;

    public:
        cEditObjHealth(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjHealth();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();
    };

}

#endif
