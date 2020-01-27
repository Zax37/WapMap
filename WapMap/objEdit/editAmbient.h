#ifndef H_C_OE_AMBIENT
#define H_C_OE_AMBIENT

#include "../states/objedit.h"
#include "../../shared/gcnWidgets/wTextDropDown.h"
#include "procPickRect.h"

namespace ObjEdit {
    class cEditObjAmbient : public cObjEdit {
    private:
        SHR::Win *win;
        SHR::Lab *labVolume, *labArea, *labPlayType, *labTurnOffTime, *labTurnOnTime, *labTimeMin[2], *labTimeMax[2];
        SHR::TextField *tfVolume, *tfTimeOff[2], *tfTimeOn[2];
        SHR::RadBut *rbAreaType[2], *rbPlayPolicy[2];
        SHR::CBox *cbArea[2];
        SHR::TextDropDown *tddSound;
        cProcPickRect *hPickArea[2];
        cSoundPlayer *hPlayer;

        void EnableControlsSync();

        friend class cObjPropVP;

    protected:
        virtual void _Think(bool bMouseConsumed);

    public:
        cEditObjAmbient(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjAmbient();

        virtual void Save();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();
    };

}

#endif
