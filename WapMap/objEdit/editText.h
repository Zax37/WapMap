#ifndef H_C_OE_TEXT
#define H_C_OE_TEXT

#include "../states/objedit.h"
#include "../../shared/gcnWidgets/wTextBox.h"

namespace ObjEdit {
    class cEditObjText : public cObjEdit {
    private:
        std::vector<WWD::Object *> hObjects;
        SHR::TextBox *tbText;
        SHR::ScrollArea *saText;
        int areaX, areaY, areaW, areaH;
        int iDragX, iDragY;
        SHR::Lab *labAlign;
        SHR::But *butApply;
        SHR::But *butAlign[3];
        int iAlign;

        void GenerateText();

        void ApplyAlign();

        int GetCharFrame(char c);

        friend class cObjPropVP;

    protected:
        virtual void _Think(bool bMouseConsumed);

    public:
        cEditObjText(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjText();

        virtual void Save();

        virtual void Action(const gcn::ActionEvent &actionEvent);

        virtual void Draw();

        virtual void RenderObjectOverlay();

        std::vector<WWD::Object *> GetObjects() { return hObjects; };
    };

}

#endif
