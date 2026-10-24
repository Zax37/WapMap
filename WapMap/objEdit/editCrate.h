#ifndef H_C_OE_CRATE
#define H_C_OE_CRATE

#include "../states/objedit.h"
#include "../../shared/gcnWidgets/wInventory.h"

namespace ObjEdit {
    class cInvPickbox;

    class lmEditObjStatueDurability;

    class cEditObjCrate : public cObjEdit {
    private:
        SHR::RadBut *rbType[3];
        cInvPickbox *hInventory;
        lmEditObjStatueDurability *lmDurability;
        SHR::DropDown *ddDurability;
        SHR::Lab *labDurability, *labAlign, *labTreasures;
        SHR::InvTab *invTabs[9];
        SHR::But *butRandomize;
        SHR::CBox *cbIncludeSpecials, *cbRandomCount;

        SHR::TextField *tfCustomZ, *tfWarpX, *tfWarpY;
        SHR::Lab *labWarpDest;
        SHR::But *butWarpPick;
        bool bStackable;
        bool bPick;
        bool bShowWarpOptions;
        int iCratesCount;
        int alignment;

        void RebuildWindow();

        void SetLogic(bool bStacked);

        void ApplyInventoryToObject();

        friend class cObjPropVP;

    protected:
        void _Think(bool bMouseConsumed) override;

    public:
        cEditObjCrate(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjCrate();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();
#ifdef WAPMAP_ENABLE_IMGUI
        void DrawImGui() override;
#endif

        bool PreviewWarp() { return bShowWarpOptions; };

        bool Picking() { return bPick; };

        virtual void Save();

        void HandleEscape() override {
            if (bPick) { butWarpPick->simulatePress(); return; }
            cObjEdit::HandleEscape();
        }
    };

}

#endif
