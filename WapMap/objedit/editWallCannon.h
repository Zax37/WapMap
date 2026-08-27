#ifndef H_C_OE_WALLCANNON
#define H_C_OE_WALLCANNON

#include "../states/objedit.h"

namespace ObjEdit {
    class cEditObjWallCannon : public cObjEdit {
    private:
        SHR::RadBut *rbType[2];
        SHR::Lab *labSpeedX, *labSpeedY, *labDir, *labLinSpeed, *labAngle;
        SHR::TextField *tfSpeedX, *tfSpeedY, *tfAngle, *tfLinSpeed;
        bool bOrient;
        int iAngle, iLinearSpeed, iSpeedX, iSpeedY;
        int iRealSpeedX;
        bool bDraggingArrow;
        bool bSkullCannon;

        int CalcAngle(int iX, int iY, bool bOr);

        void UpdateAngleInput();

        void UpdateLinearSpeed();

        friend class cObjPropVP;

    protected:
        virtual void _Think(bool bConsumed);

    public:
        cEditObjWallCannon(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjWallCannon();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();

        virtual void RenderObjectOverlay();

        virtual void Save();

        void HandleEscape() override {
            if (bDraggingArrow) {
                bDraggingArrow = false;
                bAllowDragging = true;
                iSpeedX = atoi(tfSpeedX->getText().c_str());

                iRealSpeedX = iSpeedX;
                if (!bOrient)
                    iRealSpeedX *= -1;

                iSpeedY = atoi(tfSpeedY->getText().c_str());
                iLinearSpeed = atoi(tfLinSpeed->getText().c_str());
                iAngle = CalcAngle(iRealSpeedX, iSpeedY, bOrient);
                return;
            }

            cObjEdit::HandleEscape();
        }
    };

}

#endif
