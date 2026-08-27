#ifndef H_C_OE_LASER
#define H_C_OE_LASER

#include "../states/objedit.h"

namespace ObjEdit {
    class cEditObjLaser : public cObjEdit {
    private:
        SHR::Lab *labTimeOff, *labDmg;
        SHR::TextField *tfTimeOff, *tfDmg;

        friend class cObjPropVP;

    public:
        cEditObjLaser(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjLaser();

        virtual void Save();

        virtual void Action(const gcn::ActionEvent &actionEvent);
    };

}

#endif
