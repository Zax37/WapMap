#ifndef H_C_PROC_PICKRECT
#define H_C_PROC_PICKRECT

#include "../../shared/gcnWidgets/wContainer.h"
#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wTextField.h"
#include <guichan/actionlistener.hpp>
#include "../../shared/cWWD.h"

namespace ObjEdit {
    enum enProcPickRectType {
        PickRect_MinMax = 0,
        PickRect_AttackRect,
        PickRect_UserRect1
    };

    class cProcPickRect : public gcn::ActionListener {
    private:
        enProcPickRectType iType;
        bool bPicking, bDrag;
        bool bAllowEmpty;
        bool bAllowEmptyAxis;
        SHR::Lab *lab[4];
        SHR::But *butPick;
        SHR::TextField *tf[4];
        WWD::Object *hObj;
        int iDragX, iDragY;
        int iDragmX, iDragmY;
        gcn::ActionListener *hEvList;

        void Validate();

    public:
        cProcPickRect(WWD::Object *hObj);

        ~cProcPickRect();

        void Enable(bool b);

        bool IsPicking() { return bPicking; };

        bool IsValid();

        bool IsEnabled() { return butPick->isEnabled(); };

        void SetAllowEmpty(bool b);

        void SetAllowEmptyAxis(bool b);

        void Think();

        void AddWidgets(SHR::Container *dest, int x, int y);

        int GetValue(int i) { return atoi(tf[i]->getText().c_str()); };

        void action(const gcn::ActionEvent &actionEvent);//inherited from actionlistener

        void SetActionListener(gcn::ActionListener *al) { hEvList = al; };

        SHR::But *GetPickButton() { return butPick; };

        SHR::TextField *GetTextField(int i) { return tf[i]; };

        void SetType(enProcPickRectType type);

        void SetValues(int x1 = -1, int y1 = -1, int x2 = -1, int y2 = -1);

        void SetVisible(bool b);
    };
}

#endif
