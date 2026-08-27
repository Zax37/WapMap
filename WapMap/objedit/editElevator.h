#ifndef H_C_OE_ELEVATOR
#define H_C_OE_ELEVATOR

#include "../states/objedit.h"
#include "../../shared/gcnwidgets/wListbox.h"
#include "../../shared/gcnwidgets/wTextDropDown.h"
#include "../../shared/gcnwidgets/wComboButton.h"
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
        SHR::Lab *labSpeedWarning;

        SHR::Lab *labDirection;
        SHR::But *butDirection[8];

        int iDirection;
        WWD::Rect rArea;

        void UpdateDirection(bool init = false);

        void UpdateLogic();
        void UpdateOneWayCheckBoxEnabled();
        void UpdateAllowedDirections();
        void UpdateSpeedWarning();

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
