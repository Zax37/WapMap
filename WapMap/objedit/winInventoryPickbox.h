#ifndef H_C_OE_INVPICKBOX
#define H_C_OE_INVPICKBOX

#include <guichan/actionlistener.hpp>
#include "../wViewport.h"
#include "../../shared/gcnwidgets/wWin.h"
#include "../../shared/gcnwidgets/wSlider.h"

namespace ObjEdit {
    class cInvPickbox : public WIDG::VpCallback, public gcn::ActionListener {
    private:
        SHR::Win *win;
        SHR::Slider *sli;
        WIDG::Viewport *vp;

        bool isWarpNotFunctional;
    public:
        cInvPickbox(bool isWarpNotFunctional = false);

        ~cInvPickbox();

        void Think();

        void SetVisible(bool b);

        void SetPosition(int x, int y);

        int GetWidth();

        int GetHeight();

        //inherited
        virtual void Draw(int iCode);

        void action(const gcn::ActionEvent &actionEvent);
    };
}

#endif
