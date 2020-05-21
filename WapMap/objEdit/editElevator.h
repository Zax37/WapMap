#ifndef H_C_OE_ELEVATOR
#define H_C_OE_ELEVATOR

#include "../states/objedit.h"
#include "../../shared/gcnWidgets/wListbox.h"
#include "../../shared/gcnWidgets/wTextDropDown.h"
#include "../../shared/gcnWidgets/wComboButton.h"
#include "procPickRect.h"
#include "procPickXYLockable.h"

namespace ObjEdit {

    class cEditObjElevator : public cObjEdit {
    private:
        SHR::Lab *labType;
        SHR::ComboBut *type;
        SHR::CBox *cbOneWay;

        SHR::Lab *labMoveArea;
        SHR::RadBut *automatic, *manual;
        cProcPickXYLockable *hTravelDistPick;
        cProcPickRect *hRectPick;

        cProcPickXYLockable *hSpeedPick;

        SHR::Lab *labDirection;
        SHR::But *butDirection[8];

        int iDirection;
        WWD::Rect rArea;

        void UpdateDirection(bool init = false);

        void UpdateLogic();
        void UpdateOneWayCheckBoxEnabled();

        friend class cObjPropVP;

    protected:
        virtual void _Think(bool bMouseConsumed);

    public:
        cEditObjElevator(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjElevator();

        void Save() override;

        void Action(const gcn::ActionEvent &actionEvent) override;

        void Draw() override;

        void ObjectMovedInsideEasyEdit(int fromX, int fromY) override;
    };

}

#endif
