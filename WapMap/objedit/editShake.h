#ifndef H_C_OE_SHAKE
#define H_C_OE_SHAKE

#include "../states/objedit.h"
#include "procPickRect.h"

namespace ObjEdit {
    class cEditObjShake : public cObjEdit {
    private:
        SHR::Lab *labActivate;
        cProcPickRect *hRectPick;

        friend class cObjPropVP;

    protected:
        virtual void _Think(bool bMouseConsumed) override;

    public:
        cEditObjShake(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjShake();

        virtual void Action(const gcn::ActionEvent &actionEvent) override;

        virtual void RenderObjectOverlay() override;

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
