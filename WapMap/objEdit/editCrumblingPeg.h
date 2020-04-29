#ifndef H_C_OE_CRUMBLINGPEG
#define H_C_OE_CRUMBLINGPEG

#include "../states/objedit.h"

namespace ObjEdit {
    class cEditObjCrumblingPeg : public cObjEdit {
    private:
        SHR::RadBut *rbType[2];
        SHR::CBox *cbNoRespawn;

        friend class cObjPropVP;

    public:
        cEditObjCrumblingPeg(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjCrumblingPeg();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();
    };

}

#endif
