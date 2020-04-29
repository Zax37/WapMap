#ifndef H_C_OE_BREAKPLANK
#define H_C_OE_BREAKPLANK

#include "../states/objedit.h"

namespace ObjEdit {
    class cEditObjBreakPlank : public cObjEdit {
    private:
        SHR::TextField *tfCounter, *tfWidth;
        SHR::Lab *labCounter, *labWidth;
        bool bDragging, bPicked;
        int iClickX, iClickY;

        friend class cObjPropVP;

    protected:
        virtual void _Think(bool bMouseConsumed);

    public:
        cEditObjBreakPlank(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjBreakPlank();

        virtual void Save();

        virtual void Action(const gcn::ActionEvent &actionEvent);
    };

}

#endif
