#ifndef H_C_OE_LAVAHAND
#define H_C_OE_LAVAHAND

#include "../states/objedit.h"
#include "procPickRect.h"

namespace ObjEdit {
    class cEditObjLavaHand : public cObjEdit {
    private:
        SHR::Lab *labActivate, *labDelay, *labThrows;
        cProcPickRect *hRectPick;
        SHR::TextField *tfDelay, *tfThrows;

        friend class cObjPropVP;

    protected:
        virtual void _Think(bool bMouseConsumed);

    public:
        cEditObjLavaHand(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjLavaHand();

        virtual void Save();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();

        void HandleEscape() override {
            if (hRectPick->IsPicking()) {
                hRectPick->getPickButton()->simulatePress();
                return;
            }

            cObjEdit::HandleEscape();
        }
    };

}

#endif
