#ifndef H_C_OE_CANDY
#define H_C_OE_CANDY

#include "../states/objedit.h"
#include "../../shared/gcnWidgets/wListbox.h"
#include "../../shared/gcnWidgets/wTextDropDown.h"

namespace ObjEdit {

    class cEditObjCandy : public cObjEdit, gcn::ListModel {
    private:
        SHR::Win *win;
        SHR::Lab *labImageSet, *labAlign, *labAnimation, *labFrame;
        SHR::TextDropDown *tddImageSet;
        SHR::RadBut *rbType[3];
        SHR::CBox *cbAnimated;
        SHR::TextField *tfAnimation;
        SHR::TextField *tfFrame;

        std::vector<std::string> vsDefaultImgsets;

        void UpdateLogic();

        friend class cObjPropVP;

    public:
        cEditObjCandy(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjCandy();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        //virtual void Think(bool bMouseConsumed);
        virtual void Draw();

        //inherited
        std::string getElementAt(int i);

        int getNumberOfElements();
    };

}

#endif
