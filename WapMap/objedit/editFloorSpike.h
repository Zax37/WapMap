#ifndef WAPMAP_EDITFLOORSPIKE_H
#define WAPMAP_EDITFLOORSPIKE_H

#include "../states/objedit.h"

namespace ObjEdit {
    class cEditObjFloorSpike : public cObjEdit {
    private:
        SHR::RadBut *rbType[3];
        SHR::Lab *labTimeOn, *labTimeOff, *labOffset, *labnobjDelay, *labDamage, *labIncrement;
        SHR::TextField *tfTimeOn, *tfTimeOff, *tfOffset, *tfnobjDelay, *tfDamage;
        SHR::Win *winSeries;
        std::vector<std::string> vszDefaultImgsets;

        bool isSaw = false;

        friend class cObjPropVP;

    public:
        cEditObjFloorSpike(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjFloorSpike();

        virtual void Save();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();

        virtual void *GenerateNextObjectData();

        virtual void ApplyDataFromPrevObject(void *ptr);
    };
};


#endif //WAPMAP_EDITFLOORSPIKE_H
