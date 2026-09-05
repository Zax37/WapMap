#ifndef H_C_OE_ENEMY
#define H_C_OE_ENEMY

#include "../states/objedit.h"
#include "../../shared/gcnwidgets/wInventory.h"
#include "winInventoryPickbox.h"
#include "procPickRect.h"

namespace ObjEdit {
    class cEditObjEnemy : public cObjEdit {
    private:
        SHR::RadBut *rbFlags[4];
        SHR::Lab *labType, *labBehaviour, *labDamage, *labWarpDest;
        SHR::CBox *cbPatrol;
        SHR::Lab *labTreasures, *labMoveArea;
        SHR::InvTab *invTabs[9];
        SHR::But *butClearArea, *butPickSpeedXY;
        SHR::TextField *tfDamage, *tfSpeedX, *tfSpeedY;
        std::vector<std::pair<std::string, std::string>> vstrpTypes;
        cInvPickbox *hInventory;
        cProcPickRect *hRectPick;
        bool bPickSpeedXY;
        bool bPickGem;
        int speedX, speedY;

        std::vector<SHR::RadBut*> rbType;

        void RebuildWindow();

        void ApplyInventoryToObject();

        friend class cObjPropVP;

    protected:
        virtual void _Think(bool bMouseConsumed) override;

    public:
        cEditObjEnemy(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjEnemy();

        virtual void Save() override;

        virtual void Action(const gcn::ActionEvent &actionEvent) override;

        virtual void Draw() override;

        virtual void RenderObjectOverlay() override;

        static void UpdateEnemyObject(WWD::Object *obj, const std::pair<std::string, std::string>& dataPair);

        bool isPickingXY() { return bPickSpeedXY; }

        void HandleEscape() override {
            if (hRectPick->IsPicking()) { hRectPick->getPickButton()->simulatePress(); return; }
            if (bPickSpeedXY) { butPickSpeedXY->simulatePress(); return; }

            cObjEdit::HandleEscape();
        }
    };

}

#endif
