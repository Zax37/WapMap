#include "imageDetails.h"
#include "../globals.h"
#include "../cInterfaceSheet.h"
#include "editing_ww.h"
#include "../cTextureAtlas.h"
#include <algorithm>
#include <cstdio>
#include "../cDataController.h"
#include "../langID.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../imguiTheme.h"
#include "../legacyText.h"
#endif

extern HGE *hge;

namespace State {

    ImageDetails::ImageDetails(cImage *phImage) {
#ifdef WAPMAP_ENABLE_IMGUI
        hImage = phImage;
        return;
#endif
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
#ifdef WAPMAP_ENABLE_IMGUI
        return;
#endif
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
#ifdef WAPMAP_ENABLE_IMGUI
        if (nativeClose) _popMe({});
        return false;
#endif
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
#ifdef WAPMAP_ENABLE_IMGUI
        return RenderImGui();
#endif
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

    void ImageDetails::GainFocus(const ReturnCode& code, bool bFlipped) {

    }

#ifdef WAPMAP_ENABLE_IMGUI
    bool ImageDetails::RenderImGui() {
        if (nativeClose) return false;
        const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
        const char* caption = GETL2S("Win_ImageSetBrowser", "ImageDetails");
        const auto title = LegacyText::ToUtf8(caption, cp) + "###WapMapImageDetails";
        if (!ImGui::IsPopupOpen(title.c_str())) ImGui::OpenPopup(title.c_str());
        const auto* viewport = ImGui::GetMainViewport();
        const ImVec2 size{std::min(600.f, viewport->WorkSize.x - 16.f),
                          std::min(585.f, viewport->WorkSize.y - 16.f)};
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, {0.5f, 0.5f});
        ImGui::SetNextWindowSize(size, ImGuiCond_Always);
        ImGui::SetNextWindowScroll({0, 0});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, IM_COL32(0, 0, 0, 145));
        bool open = true;
        const bool shown = ImGui::BeginPopupModal(title.c_str(), &open,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PopStyleColor(); ImGui::PopStyleVar();
        if (!open) nativeClose = true;
        if (!shown) return false;
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, caption);
        ImGui::SetCursorPos({10, 32});
        ImGui::SetNextItemWidth(150);
        ImGui::SliderInt("##zoom", &nativeZoom, -5, 5, "");
        const float scale = nativeZoom >= 0 ? 1.f + nativeZoom : 1.f / (1.f - nativeZoom);
        char zoom[32]; std::snprintf(zoom, sizeof(zoom), "%.0f%%", scale * 100);
        const auto origin = ImGui::GetWindowPos();
        WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16,
            {origin.x + 172, origin.y + 36}, zoom);
        ImGui::SetCursorPos({5, 64});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        if (ImGui::BeginChild("preview", {size.x - 10, std::max(40.f, size.y - 122)},
                ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar)) {
            auto* sprite = hImage ? hImage->GetImage() : nullptr;
            const auto available = ImGui::GetContentRegionAvail();
            const float width = sprite ? sprite->GetWidth() * scale : 0;
            const float height = sprite ? sprite->GetHeight() * scale : 0;
            const ImVec2 canvas{std::max(available.x, width), std::max(available.y, height)};
            const auto start = ImGui::GetCursorScreenPos();
            auto* draw = ImGui::GetWindowDrawList();
            const auto minimum = draw->GetClipRectMin(), maximum = draw->GetClipRectMax();
            // Tile only the visible area, even when a large image is zoomed in.
            const float step = 100;
            for (float y = minimum.y; y < maximum.y; y += step)
                for (float x = minimum.x; x < maximum.x; x += step)
                    WapMapImGui::Sprite(draw, *hge, GV->sprCheckboard, {x, y});
            if (sprite) {
                const ImVec2 p{start.x + (canvas.x - width) / 2, start.y + (canvas.y - height) / 2};
                WapMapImGui::StretchSprite(draw, *hge, sprite, p, {p.x + width, p.y + height}, IM_COL32_WHITE);
            }
            ImGui::Dummy(canvas);
        }
        ImGui::EndChild(); ImGui::PopStyleVar();
        ImGui::SetCursorPos({(size.x - 150) / 2, size.y - 43});
        if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface,
                GETL2S("Win_ImageImport", "Cancel"), {150, 25})) nativeClose = true;
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
                ImGui::IsKeyPressed(ImGuiKey_Escape)) nativeClose = true;
        if (nativeClose) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        return false;
    }
#endif

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
        GV->sprCheckboard->SetColor(0xFFFFFFFF);
        for (int y = 0; y < drh / 100 + 1; y++)
            for (int x = 0; x < drw / 100 + 1; x++)
                GV->sprCheckboard->Render(drx + x * 100, dry + y * 100);

        hImage->GetImage()->RenderEx(drx + drw / 2 - scrollx, dry + drh / 2 - scrolly, 0, fZoomMod, fZoomMod);
        hge->Gfx_SetClipping();

    }

}
