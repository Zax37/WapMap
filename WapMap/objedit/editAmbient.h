#ifndef H_C_OE_AMBIENT
#define H_C_OE_AMBIENT

#include "../states/objedit.h"
#include "../../shared/gcnwidgets/wTextDropDown.h"
#include "procPickRect.h"

namespace ObjEdit {
    class cEditObjAmbient : public cObjEdit {
    private:
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
        virtual void _Think(bool bMouseConsumed) override;

    public:
        cEditObjAmbient(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjAmbient();

        virtual void Save() override;

        virtual void Action(const gcn::ActionEvent &actionEvent) override;

        virtual void Draw() override;

        void HandleEscape() override {
            for (int i = 0; i < 2; i++) if (hPickArea[i]->IsPicking()) {
                hPickArea[i]->getPickButton()->simulatePress();
                return;
            }

            cObjEdit::HandleEscape();
        }
    };

}

#endif
