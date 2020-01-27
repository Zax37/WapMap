#ifndef H_C_OE_STATUE
#define H_C_OE_STATUE

#include "../states/objedit.h"
#include "../../shared/gcnWidgets/wInventory.h"

namespace ObjEdit {
    class cInvPickbox;

    class lmEditObjStatueDurability : public gcn::ListModel {
    public:
        std::string getElementAt(int i);

        int getNumberOfElements();
    };

    class cEditObjStatue : public cObjEdit {
    private:
        SHR::RadBut *rbType[2];
        SHR::Win *win;
        cInvPickbox *hInventory;
        lmEditObjStatueDurability *lmDurability;
        SHR::DropDown *ddDurability;
        SHR::Lab *labDurability, *labAlign, *labTreasures;
        SHR::InvTab *invTabs[9];

        SHR::TextField *tfWarpX, *tfWarpY;
        SHR::Lab *labWarpDest;
        SHR::But *butWarpPick;

        bool bPick;
        bool bShowWarpOptions;

        void RebuildWindow();

        friend class cObjPropVP;

    public:
        cEditObjStatue(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjStatue();

        virtual void Save();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();

        bool PreviewWarp() { return bShowWarpOptions; };

        bool Picking() { return bPick; };
    };

}

#endif
