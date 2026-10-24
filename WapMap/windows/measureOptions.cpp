#include "measureOptions.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../states/editing_ww.h"
#include "../cInterfaceSheet.h"
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../langID.h"

winMeasureOptions::winMeasureOptions() : cWindow(GETL(Lang_MeasureOptions), 300, 100) {}
void winMeasureOptions::Open() {
    document = GV->editState->hParser;
    if (!document) return;
    open = positionOnOpen = true;
    clearRequested = stopRequested = false;
    GV->editState->winMeasureOpt->setVisible(false);
}
void winMeasureOptions::Close() {
    open = clearRequested = stopRequested = false;
    document = nullptr;
    cWindow::Close();
}
void winMeasureOptions::Think() {
    if (!open) return;
    auto& e = *GV->editState;
    if (document != e.hParser || e.iActiveTool != EWW_TOOL_MEASURE) { Close(); return; }
    if (!e._isOnTop()) return;
    if (stopRequested) { e.SetTool(EWW_TOOL_NONE); return; }
    if (clearRequested) { e.m_vMeasurePoints.clear(); clearRequested = false; e.vPort->MarkToRedraw(); }
}
void winMeasureOptions::DrawImGui() {
    if (!open) return;
    auto& e = *GV->editState;
    if (document != e.hParser || e.iActiveTool != EWW_TOOL_MEASURE) { Close(); return; }
    if (!e._isOnTop()) return;
    if (positionOnOpen) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos({viewport->Pos.x + e.vPort->GetX() + e.vPort->GetWidth() - 310.f,
            viewport->Pos.y + e.vPort->GetY() + e.vPort->GetHeight() - 110.f}, ImGuiCond_FirstUseEver);
        positionOnOpen = false;
    }
    ImGui::SetNextWindowSize({300, 100}, ImGuiCond_Always);
    const auto title = LegacyText::ToUtf8(GETL(Lang_MeasureOptions), LegacyText::CodePage(GV->Lang->GetCode())) + "###WapMapMeasureOptions";
    bool visible = true;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, GETL(Lang_MeasureOptions));
        const auto area = myWin.getChildrenArea();
        ImGui::SetCursorPos({area.x + 5.f, area.y + 15.f});
        bool absolute = e.cbmeasAbsoluteDistance->isSelected();
        if (WapMapImGui::BitmapCheckbox(*hge, *GV->fntMyriad16, *GV->hGfxInterface, e.cbmeasAbsoluteDistance->getCaption().c_str(), absolute)) {
            e.cbmeasAbsoluteDistance->setSelected(absolute); e.vPort->MarkToRedraw();
        }
        auto* button = e.butmeasClear;
        ImGui::SetCursorPos({static_cast<float>(area.x + button->getX()), static_cast<float>(area.y + button->getY())});
        if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, "", {static_cast<float>(button->getWidth()), static_cast<float>(button->getHeight())})) clearRequested = true;
        const auto p = ImGui::GetItemRectMin(); auto* icon = button->getIcon();
        if (icon) WapMapImGui::Sprite(ImGui::GetWindowDrawList(), *hge, icon,
            {p.x + (button->getWidth() - icon->GetWidth()) / 2, p.y + (button->getHeight() - icon->GetHeight()) / 2});
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) visible = false;
    }
    ImGui::End();
    if (!visible) stopRequested = true;
}
#endif
