#ifndef H_C_OE_STATUE
#define H_C_OE_STATUE

#include "../states/objedit.h"
#include "../../shared/gcnwidgets/wInventory.h"

namespace ObjEdit {
    class cInvPickbox;

    class lmEditObjStatueDurability : public gcn::ListModel {
    public:
        std::string getElementAt(int i);

        int getNumberOfElements();
    };

    class cEditObjStatue : public cObjEdit {
    private:
        SHR::RadBut *rbType[3];
        cInvPickbox *hInventory;
        lmEditObjStatueDurability *lmDurability;
        SHR::DropDown *ddDurability;
        SHR::Lab *labDurability, *labAlign, *labTreasures;
        SHR::InvTab *invTabs[9];
        SHR::But *butRandomize;
        SHR::CBox *cbIncludeSpecials, *cbRandomCount;

        SHR::TextField *tfCustomZ;

        int alignment;

        void RebuildWindow();

        friend class cObjPropVP;

    public:
        cEditObjStatue(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjStatue();

        virtual void Save();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();
    };

}

#endif
