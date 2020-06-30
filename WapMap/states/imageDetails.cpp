#include "imageDetails.h"
#include "../globals.h"
#include "../cInterfaceSheet.h"
#include "editing_ww.h"
#include "../cTextureAtlas.h"
#include <algorithm>
#include "../cDataController.h"
#include "../langID.h"

extern HGE *hge;

namespace State {

    ImageDetails::ImageDetails(cImage *phImage) {
        gui = new gcn::Gui();
        gui->setGraphics(GV->gcnGraphics);
        gui->setInput(GV->gcnInput);

        int yOffset = 20;

        myWin = new SHR::Win(&GV->gcnParts, "aaa");
        //int height = hge->System_GetState(HGE_SCREENHEIGHT)-200;
        myWin->setDimension(gcn::Rectangle(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 300,
                                           hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 250 - yOffset / 2, 600,
                                           565 + yOffset));
        myWin->setMovable(0);
        myWin->setClose(1);
        myWin->addActionListener(this);
        conPreview = new SHR::Container();
        conPreview->setDimension(gcn::Rectangle(0, 0, 100, 100));
        conPreview->setOpaque(0);

        saPreview = new SHR::ScrollArea(conPreview, SHR::ScrollArea::SHOW_ALWAYS, SHR::ScrollArea::SHOW_ALWAYS);
        saPreview->setDimension(gcn::Rectangle(0, 0, 590, 400));
        saPreview->setOpaque(0);
        myWin->add(saPreview, 5, 35 + yOffset);

        conPreview->setWidth(saPreview->getChildrenArea().width);

        vpOverlay = new WIDG::Viewport(this, 0);
        conPreview->add(vpOverlay, 0, 0);

        butReturn = new SHR::But(GV->hGfxInterface, GETL2S("Win_ImageImport", "Cancel"));
        butReturn->setDimension(gcn::Rectangle(0, 0, 150, 25));
        butReturn->addActionListener(this);
        myWin->add(butReturn, 300 + 5, myWin->getHeight() - 50);

        sliZoom = new SHR::Slider(-5, 5);
        sliZoom->setDimension(gcn::Rectangle(0, 0, 150, 20));
        sliZoom->setStyle(SHR::Slider::POINTER);
        sliZoom->addActionListener(this);
        for (int i = -5; i <= 5; i++)
            sliZoom->addKeyValue(i);
        myWin->add(sliZoom, 5, 5);

        fZoomMod = 1;

        gui->setTop(myWin);

        hImage = phImage;
    }

    ImageDetails::~ImageDetails() {
        delete sliZoom;
        delete vpOverlay;
        delete conPreview;
        delete saPreview;
        delete butReturn;
        delete myWin;
        delete gui;
    }

    bool ImageDetails::Opaque() {
        return 0;
    }

    void ImageDetails::Init() {

    }

    void ImageDetails::Destroy() {

    }

    bool ImageDetails::Think() {
        GV->Console->Think();
        try {
            gui->logic();
        }
        catch (gcn::Exception &exc) {
            GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(),
                                exc.getFilename().c_str(), exc.getLine());
        }
        return false;
    }

    bool ImageDetails::Render() {
        try {
            gui->draw();
        }
        catch (gcn::Exception &exc) {
            GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(),
                                exc.getFilename().c_str(), exc.getLine());
        }
        GV->Console->Render();
        return false;
    }

    void ImageDetails::GainFocus(ReturnCode<void> code, bool bFlipped) {

    }

    void ImageDetails::action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == myWin || actionEvent.getSource() == butReturn) {
            _popMe({});
            return;
        } else if (actionEvent.getSource() == sliZoom) {
            float fVal = sliZoom->getValue();
            if (fVal >= 0)
                fZoomMod = 1 + fVal;
            else if (fVal < 0) {
                fZoomMod = 0.2f + 0.8f * ((-fVal) / 5.0f);
            }
            conPreview->setWidth(hImage->GetImage()->GetWidth() * fZoomMod);
            conPreview->setHeight(hImage->GetImage()->GetHeight() * fZoomMod);
        }
    }

    void ImageDetails::Draw(int piCode) {
        int dx, dy;
        myWin->getAbsolutePosition(dx, dy);
        gcn::Rectangle rchild = saPreview->getChildrenArea();
        int drx, dry, drw, drh;
        saPreview->getAbsolutePosition(drx, dry);
        drx += rchild.x;
        dry += rchild.y;
        drw = rchild.width;
        drh = rchild.height;

        int scrollx = saPreview->getHorizontalScrollAmount(),
                scrolly = saPreview->getVerticalScrollAmount();

        hge->Gfx_SetClipping(drx, dry, drw, drh);
        for (int y = 0; y < drh / 100 + 1; y++)
            for (int x = 0; x < drw / 100 + 1; x++)
                GV->sprCheckboard->Render(drx + x * 100, dry + y * 100);

        hImage->GetImage()->RenderEx(drx + drw / 2 - scrollx, dry + drh / 2 - scrolly, 0, fZoomMod, fZoomMod);
        hge->Gfx_SetClipping();

    }

}
