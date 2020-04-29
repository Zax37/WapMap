#ifndef H_C_OE_ENEMY
#define H_C_OE_ENEMY

#include "../states/objedit.h"
#include "../../shared/gcnWidgets/wInventory.h"
#include "winInventoryPickbox.h"
#include "procPickRect.h"

namespace ObjEdit {
    class cEditObjEnemy : public cObjEdit {
    private:
        SHR::RadBut *rbType[5], *rbType2[2], *rbFlags[4];
        SHR::Lab *labType, *labType2, *labBehaviour, *labDamage, *labWarpDest;
        SHR::CBox *cbPatrol;
        SHR::Lab *labTreasures, *labMoveArea;
        SHR::InvTab *invTabs[9];
        SHR::But *butClearArea, *butPickSpeedXY;
        SHR::TextField *tfDamage, *tfSpeedX, *tfSpeedY;
        std::vector<std::pair<std::string, std::string> > vstrpTypes;
        cInvPickbox *hInventory;
        cProcPickRect *hRectPick;
        bool bPickSpeedXY;
        bool bPickGem;

        void RebuildWindow();

        void ApplyInventoryToObject();

        friend class cObjPropVP;

    protected:
        virtual void _Think(bool bMouseConsumed);

    public:
        cEditObjEnemy(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjEnemy();

        virtual void Save();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();

        virtual void RenderObjectOverlay();
    };

}

#endif
