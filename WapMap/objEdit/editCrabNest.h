#ifndef H_C_OE_CRABNEST
#define H_C_OE_CRABNEST

#include "../states/objedit.h"
#include "procPickRect.h"
#include "winInventoryPickbox.h"
#include "../../shared/gcnWidgets/wInventory.h"

namespace ObjEdit {
    class cEditObjCrabNest : public cObjEdit {
    private:
        SHR::Lab *labNumber, *labActivate;
        SHR::TextField *tfNumber;
        cProcPickRect *hRectPick;
        cInvPickbox *hInventory;
        SHR::InvTab *invTabs[9];

        friend class cObjPropVP;

    protected:
        virtual void _Think(bool bMouseConsumed);

        void ApplyInventoryToObject();

    public:
        cEditObjCrabNest(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjCrabNest();

        virtual void Save();

        virtual void Action(const gcn::ActionEvent &actionEvent);
    };

}

#endif
