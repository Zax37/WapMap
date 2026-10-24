#include "paletteBrowser.h"

#include "../shared/commonFunc.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../databanks/palettes.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../imguiTheme.h"
#include "../legacyText.h"
#endif

winPaletteBrowser::winPaletteBrowser() : cWindow(GETL2S("Win_PaletteBrowser", "WinCaption"), 201, 300) {
    lbPalettes = new SHR::ListBox();
    lbPalettes->setWidth(myWin.getWidth() - 9);
    lbPalettes->addActionListener(this);
    myWin.add(lbPalettes, 3, 0);
    vp = new WIDG::Viewport(this, 0);
    myWin.add(vp);
}

winPaletteBrowser::~winPaletteBrowser() {
    delete lbPalettes;
    delete vp;
}

void winPaletteBrowser::UpdatePalettes() {
    lbPalettes->setListModel(GV->editState->hPalettesBank);
    lbPalettes->setSelected(GV->editState->hPalettesBank->getSelectedPaletteIndex());
    myWin.setHeight(225 + lbPalettes->getHeight());
    lbPalettes->setY(205);
}

void winPaletteBrowser::OnDocumentChange() {
#ifdef WAPMAP_ENABLE_IMGUI
    pendingPalette.clear();
    if (!GV->editState->hPalettesBank) { Close(); return; }
#endif
    UpdatePalettes();
}

void winPaletteBrowser::Open() {
#ifdef WAPMAP_ENABLE_IMGUI
    if (!GV->editState->hParser || !GV->editState->hPalettesBank) return;
    nativeOpen = positionOnOpen = true;
    pendingPalette.clear();
    myWin.setVisible(false);
#else
    UpdatePalettes();
    cWindow::Open();
#endif
}

#ifdef WAPMAP_ENABLE_IMGUI
void winPaletteBrowser::Close() {
    nativeOpen = false;
    pendingPalette.clear();
    cWindow::Close();
}

void winPaletteBrowser::Think() {
    if (!nativeOpen || pendingPalette.empty() || !GV->editState->_isOnTop()) return;
    auto* bank = GV->editState->hPalettesBank;
    if (bank) for (int i = 0; i < bank->getNumberOfElements(); ++i) {
        if (bank->getElementAt(i) == pendingPalette) {
            bank->setSelectedPaletteIndex(i);
            break;
        }
    }
    pendingPalette.clear();
}

void winPaletteBrowser::DrawImGui() {
    if (!nativeOpen) return;
    auto& editor = *GV->editState;
    if (!editor.hParser || !editor.hPalettesBank || !editor.hDataCtrl) { Close(); return; }
    if (!editor._isOnTop()) return;
    if (positionOnOpen) {
        auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, {0.5f, 0.5f});
        ImGui::SetNextWindowFocus();
        positionOnOpen = false;
    }
    ImGui::SetNextWindowSize({260, 380}, ImGuiCond_Always);
    const auto title = LegacyText::ToUtf8(GETL2S("Win_PaletteBrowser", "WinCaption"),
        LegacyText::CodePage(GV->Lang->GetCode())) + "###WapMapPalettes";
    bool visible = true;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton,
                                 GETL2S("Win_PaletteBrowser", "WinCaption"));
        auto* draw = ImGui::GetWindowDrawList();
        auto* palette = editor.hDataCtrl->GetPalette();
        if (palette) for (int i = 0; i < 256; ++i) {
            const auto color = palette->GetColor(i);
            ImGui::SetCursorPos({10.f + (i % 16) * 15.f, 32.f + (i / 16) * 15.f});
            ImGui::PushID(i);
            ImGui::InvisibleButton("##color", {14, 14});
            const auto p = ImGui::GetItemRectMin();
            draw->AddRectFilled(p, {p.x + 14, p.y + 14},
                IM_COL32((color >> 16) & 255, (color >> 8) & 255, color & 255, 255));
            if (ImGui::IsItemHovered()) {
                draw->AddRect(p, {p.x + 14, p.y + 14}, IM_COL32_WHITE);
                ImGui::SetTooltip("%d: #%06X", i, static_cast<unsigned>(color & 0xffffff));
            }
            ImGui::PopID();
        }
        ImGui::SetCursorPos({8, 280});
        ImGui::PushStyleColor(ImGuiCol_ChildBg, WapMapImGui::Grey(0x13));
        if (ImGui::BeginChild("palettes", {244, 92}, ImGuiChildFlags_NavFlattened)) {
            auto* bank = editor.hPalettesBank;
            const int selected = bank->getSelectedPaletteIndex();
            for (int i = 0; i < bank->getNumberOfElements(); ++i) {
                ImGui::PushID(i);
                const auto name = bank->getElementAt(i);
                const auto caption = LegacyText::ToUtf8(name, LegacyText::CodePage(GV->Lang->GetCode()));
                if (ImGui::Selectable(caption.c_str(), selected == i)) pendingPalette = name;
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) visible = false;
    }
    ImGui::End();
    if (!visible) Close();
}
#endif

void winPaletteBrowser::Draw(int piCode) {
    PID::Palette* palette = GV->editState->hDataCtrl->GetPalette();
    int startX, startY;
    myWin.getAbsolutePosition(startX, startY);
    startX += 5;
    startY += 27;
    int i = 0;

    #define TILE_SIZE 12
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++, i++) {
            SHR::SetQuad(&q, SETA(palette->GetColor(i), myWin.getAlpha()), x*TILE_SIZE+startX, y*TILE_SIZE+startY,
                 x*TILE_SIZE+startX+TILE_SIZE-1, y*TILE_SIZE+startY+TILE_SIZE-1);
            hge->Gfx_RenderQuad(&q);
        }
    }
}

void winPaletteBrowser::action(const ActionEvent &actionEvent) {
    if (actionEvent.getSource() == lbPalettes) {
        GV->editState->hPalettesBank->setSelectedPaletteIndex(lbPalettes->getSelected());
    }
}

#ifdef WAPMAP_ENABLE_IMGUI
winFillColor::winFillColor() : cWindow(GETL(Lang_FillColor)) {}
bool winFillColor::Eligible() const {
    const auto& e = *GV->editState;
    return e.hParser && e.hDataCtrl && e.iTilePicked == EWW_TILE_FILL &&
        (e.iActiveTool == EWW_TOOL_PENCIL || e.iActiveTool == EWW_TOOL_FILL ||
         e.iActiveTool == EWW_TOOL_BRUSH || e.iActiveTool == EWW_TOOL_WRITEID);
}
void winFillColor::Open() {
    if (!Eligible()) return;
    document = GV->editState->hParser;
    open = positionOnOpen = true;
    stopRequested = false;
    pendingColor = -1;
    GV->editState->wintpFillColor->setVisible(false);
}
void winFillColor::Close() {
    open = stopRequested = false;
    resizePosition = false;
    document = nullptr;
    pendingPlane = nullptr;
    pendingColor = -1;
    cWindow::Close();
}
void winFillColor::Think() {
    if (!Eligible()) { Close(); return; }
    if (!open) Open();
    auto& e = *GV->editState;
    if (document != e.hParser) { Close(); return; }
    if (!e._isOnTop()) return;
    if (stopRequested) { e.SetTool(EWW_TOOL_NONE); Close(); return; }
    if (pendingColor >= 0) {
        auto* plane = e.GetActivePlane();
        if (plane == pendingPlane && pendingColor < 256 && plane->GetFillColor() != pendingColor) {
            plane->SetFillColor(pendingColor);
            e.vPort->MarkToRedraw();
            e.MarkUnsaved();
        }
        pendingColor = -1;
    }
}
void winFillColor::DrawImGui() {
    if (!Eligible()) { Close(); return; }
    if (!open) Open();
    auto& e = *GV->editState;
    if (document != e.hParser) { Close(); return; }
    if (!e._isOnTop()) return;
    const float height = expanded ? 324.f : 70.f;
    if (positionOnOpen) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos({viewport->Pos.x + e.vPort->GetX() + 10.f,
            viewport->Pos.y + e.vPort->GetY() + e.vPort->GetHeight() - height - 10.f}, ImGuiCond_FirstUseEver);
        positionOnOpen = false;
    } else if (resizePosition) {
        ImGui::SetNextWindowPos({nextX, nextY}, ImGuiCond_Always);
        resizePosition = false;
    }
    ImGui::SetNextWindowSize({226, height}, ImGuiCond_Always);
    const auto title = LegacyText::ToUtf8(GETL(Lang_FillColor), LegacyText::CodePage(GV->Lang->GetCode())) + "###WapMapFillColor";
    if (WapMapImGui::BeginFixedWindow(title.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, nullptr, GETL(Lang_FillColor));
        const auto toggle = [&]() {
            const auto p = ImGui::GetWindowPos();
            expanded = !expanded;
            nextX = p.x; nextY = p.y + height - (expanded ? 324.f : 70.f);
            resizePosition = true;
        };
        auto* plane = e.GetActivePlane();
        auto* palette = e.hDataCtrl->GetPalette();
        int preview = std::clamp(plane->GetFillColor(), 0, 255);
        bool hovering = false;
        int hoveredColor = -1, focusedColor = -1;
        if (expanded && palette) {
            for (int i = 0; i < 256; ++i) {
                ImGui::PushID(i);
                ImGui::SetCursorPos({8.f + (i % 16) * 13.f, 32.f + (i / 16) * 13.f});
                if (ImGui::InvisibleButton("##color", {12, 12}, ImGuiButtonFlags_EnableNav)) { pendingColor = i; pendingPlane = plane; }
                const auto p = ImGui::GetItemRectMin();
                const DWORD color = palette->GetColor(i);
                auto* draw = ImGui::GetWindowDrawList();
                draw->AddRectFilled(p, {p.x + 12, p.y + 12}, IM_COL32(GETR(color), GETG(color), GETB(color), 255));
                const bool highlighted = ImGui::IsItemHovered() || WapMapImGui::IsKeyboardFocused();
                if (highlighted || plane->GetFillColor() == i)
                    draw->AddRect({p.x - 1, p.y - 1}, {p.x + 12, p.y + 12}, highlighted ? IM_COL32_WHITE : IM_COL32(23, 150, 255, 255));
                if (ImGui::IsItemHovered()) hoveredColor = i;
                if (WapMapImGui::IsKeyboardFocused()) focusedColor = i;
                ImGui::PopID();
            }
            if (hoveredColor >= 0 || focusedColor >= 0) {
                preview = hoveredColor >= 0 ? hoveredColor : focusedColor;
                hovering = true;
            }
            ImGui::SetCursorPos({8, 252});
            const auto caption = std::string(hovering ? GETL(Lang_SelectedFillColor) : GETL(Lang_ActualFillColor)) + ": #" + std::to_string(preview);
            WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, ImGui::GetCursorScreenPos(), caption.c_str());
            const auto origin = ImGui::GetWindowPos();
            const DWORD color = palette->GetColor(preview);
            ImGui::GetWindowDrawList()->AddRectFilled({origin.x + 8, origin.y + 275}, {origin.x + 218, origin.y + 287}, IM_COL32(GETR(color), GETG(color), GETB(color), 255));
        }
        ImGui::SetCursorPos({8, expanded ? 294.f : 32.f});
        if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, expanded ? GETL(Lang_HideFill) : GETL(Lang_SelectFill), {210, 25})) toggle();
        if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            if (expanded) toggle(); else stopRequested = true;
        }
    }
    ImGui::End();
}
#endif
