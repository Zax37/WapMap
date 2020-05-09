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

    class cProcPickRect : public gcn::ActionListener, public SHR::Container {
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

        void validate();

    public:
        cProcPickRect(WWD::Object *hObj);

        ~cProcPickRect();

        void setEnabled(bool b);

        bool IsPicking() { return bPicking; };

        bool isValid();

        bool IsEnabled() { return butPick->isEnabled(); };

        void setAllowEmpty(bool b);

        void setAllowEmptyAxis(bool b);

        void Think();

        int getValue(int i) { return atoi(tf[i]->getText().c_str()); };

        void action(const gcn::ActionEvent &actionEvent);//inherited from actionlistener

        void setActionListener(gcn::ActionListener *al) { hEvList = al; };

        SHR::But *getPickButton() { return butPick; };

        SHR::TextField *getTextField(int i) { return tf[i]; };

        void setType(enProcPickRectType type);

        void setValues(int x1 = -1, int y1 = -1, int x2 = -1, int y2 = -1);

        void setVisible(bool b);
    };
}

#endif
