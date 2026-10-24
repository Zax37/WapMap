#ifndef H_C_OE_INVPICKBOX
#define H_C_OE_INVPICKBOX

#include <guichan/actionlistener.hpp>
#include "../wViewport.h"
#include "../../shared/gcnWidgets/wWin.h"
#include "../../shared/gcnWidgets/wSlider.h"

namespace ObjEdit {
    class cInvPickbox : public WIDG::VpCallback, public gcn::ActionListener {
    private:
        SHR::Win *win;
        SHR::Slider *sli;
        WIDG::Viewport *vp;

        bool isWarpNotFunctional;
#ifdef WAPMAP_ENABLE_IMGUI
        bool native = false, nativeVisible = false, nativePosition = true;
#endif
    public:
        cInvPickbox(bool isWarpNotFunctional = false);

        ~cInvPickbox();

        void Think();

        void SetVisible(bool b);

        void SetPosition(int x, int y);
#ifdef WAPMAP_ENABLE_IMGUI
        void UseNative();
        void DrawImGui(int& selection);
#endif

        int GetWidth();

        int GetHeight();

        //inherited
        virtual void Draw(int iCode);

        void action(const gcn::ActionEvent &actionEvent);
    };
}

#endif
