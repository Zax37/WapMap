#ifndef H_STATE_IMGDETAILS
#define H_STATE_IMGDETAILS

#include "../../shared/cStateMgr.h"
#include "../../shared/gcnwidgets/wWin.h"
#include "../../shared/gcnwidgets/wButton.h"
#include "../../shared/gcnwidgets/wLabel.h"
#include "../../shared/gcnwidgets/wScrollArea.h"
#include "../../shared/gcnwidgets/wSlider.h"
#include "guichan.hpp"
#include "../wViewport.h"

class cImage;

namespace State {

    class ImageDetails
            : public SHR::cState, public gcn::ActionListener, public WIDG::VpCallback/*, public gcn::ListModel*/ {
    public:
        ImageDetails(cImage *phImage);

        ~ImageDetails();

        virtual bool Opaque();

        virtual void Init();

        virtual void Destroy();

        virtual bool Think();

        virtual bool Render();

        virtual void GainFocus(const ReturnCode& code, bool bFlipped);

        void action(const gcn::ActionEvent &actionEvent);

        virtual void Draw(int piCode);

    private:
        gcn::Gui *gui;
        SHR::Win *myWin;
        SHR::ScrollArea *saPreview;
        SHR::Container *conPreview;
        SHR::But *butReturn;
        SHR::Slider *sliZoom;
        float fZoomMod;

        WIDG::Viewport *vpOverlay;

        cImage *hImage;

        //virtual std::string getElementAt(int i);
        //virtual int getNumberOfElements();
    };
};

#endif
