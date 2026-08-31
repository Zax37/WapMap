#ifndef H_C_OE_SPRINGBOARD
#define H_C_OE_SPRINGBOARD

#include "../states/objedit.h"

namespace ObjEdit {
    class cEditObjSpringboard : public cObjEdit {
    private:
        SHR::Lab *labJumpHeight;
        SHR::TextField *tfJumpHeight;
        SHR::But *butPick;

        friend class cObjPropVP;

    protected:
        virtual void _Think(bool bMouseConsumed) override;

    public:
        cEditObjSpringboard(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjSpringboard();

        virtual void Action(const gcn::ActionEvent &actionEvent) override;

        bool bPicking;
        int iPickY;

        void HandleEscape() override {
            if (bPicking) { butPick->simulatePress(); return; }

            cObjEdit::HandleEscape();
        }
    };

}

#endif
