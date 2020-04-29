#ifndef H_C_OE_CRATE
#define H_C_OE_CRATE

#include "../states/objedit.h"
#include "../../shared/gcnWidgets/wInventory.h"

namespace ObjEdit {
    class cInvPickbox;

    class lmEditObjStatueDurability;

    class cEditObjCrate : public cObjEdit {
    private:
        SHR::RadBut *rbType[2];
        cInvPickbox *hInventory;
        lmEditObjStatueDurability *lmDurability;
        SHR::DropDown *ddDurability;
        SHR::Lab *labDurability, *labAlign, *labTreasures;
        SHR::InvTab *invTabs[9];
        SHR::But *butRandomize;
        SHR::CBox *cbIncludeSpecials, *cbRandomCount;

        SHR::TextField *tfWarpX, *tfWarpY;
        SHR::Lab *labWarpDest;
        SHR::But *butWarpPick;
        bool bStackable, bAlign;
        bool bPick;
        bool bShowWarpOptions;
        int iCratesCount;

        void RebuildWindow();

        void SetLogic(bool bStacked, bool bf);

        void ApplyInventoryToObject();

        friend class cObjPropVP;

    public:
        cEditObjCrate(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjCrate();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();

        bool PreviewWarp() { return bShowWarpOptions; };

        bool Picking() { return bPick; };

        virtual void Save();
    };

}

#endif
