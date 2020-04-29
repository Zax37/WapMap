#ifndef H_C_OE_SPECPOWERUP
#define H_C_OE_SPECPOWERUP

#include "../states/objedit.h"

namespace ObjEdit {
    class cEditObjSpecialPowerup : public cObjEdit {
    private:
        SHR::RadBut *rbType[8];
        SHR::Lab *labTime;
        SHR::CBox *cbDontDisappear;
        SHR::TextField *tfTime;

        friend class cObjPropVP;

    public:
        cEditObjSpecialPowerup(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjSpecialPowerup();

        virtual void Save();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();
    };

}

#endif
