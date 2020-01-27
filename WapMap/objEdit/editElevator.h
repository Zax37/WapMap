#ifndef H_C_OE_ELEVATOR
#define H_C_OE_ELEVATOR

#include "../states/objedit.h"
#include "../../shared/gcnWidgets/wListbox.h"
#include "../../shared/gcnWidgets/wTextDropDown.h"
#include "procPickRect.h"

namespace ObjEdit {

    class cEditObjElevator : public cObjEdit {
    private:
        SHR::Win *winType, *winProp;
        SHR::Lab *labType, *labTypeDesc[4];
        SHR::RadBut *rbType[4];
        SHR::Lab *labSpeedX, *labSpeedY, *labSpeedLinear, *labAngle;
        SHR::TextField *tfSpeedX, *tfSpeedY, *tfSpeedLinear, *tfAngle;
        SHR::Lab *labDirection;
        SHR::But *butDirection[8];
        SHR::Lab *labMoveArea;
        SHR::RadBut *rbMoveAreaType[2];
        SHR::But *butPickSpeed;
        SHR::CBox *cbOneWay;
        SHR::Lab *labOneWay;
        cProcPickRect *hRectPick;
        bool bRectangleArea;
        bool bAreaAxis;
        int iDirection;
        WWD::Rect rArea;

        void RebuildWindow();

        friend class cObjPropVP;

    protected:
        virtual void _Think(bool bMouseConsumed);

    public:
        cEditObjElevator(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjElevator();

        virtual void Save();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();
    };

}

#endif
