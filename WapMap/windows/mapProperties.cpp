#include "mapProperties.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../states/editing_ww.h"
#include "../langID.h"
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../cInterfaceSheet.h"
#include <cstdio>

namespace {
constexpr size_t limits[] = {58, 63, 31, 255, 127, 127, 127};
constexpr int rows[] = {15, 40, 65, 100, 125, 150, 175};
}

winMapProperties::winMapProperties() : cWindow(GETL(Lang_MapProperties), 460, 290) {}

void winMapProperties::Open() {
    auto* editor = GV->editState;
    if (!editor->hParser) return;
    document = editor->hParser;
    editor->SyncWorldOptionsWithParser();
    const char* values[] = {document->GetName(), document->GetAuthor(), document->GetDate(),
        document->GetRezPath(), document->GetTilesPath(), document->GetPalettePath(), document->GetExePath()};
    const auto codePage = LegacyText::CodePage(GV->Lang->GetCode());
    for (int field = 0; field < 7; ++field) {
        const auto text = LegacyText::ToUtf8(values[field], codePage);
        std::snprintf(fields[field].data(), fields[field].size(), "%s", text.c_str());
    }
    compression = document->GetFlag(WWD::Flag_w_Compress);
    useZ = document->GetFlag(WWD::Flag_w_UseZCoords);
    editor->winWorld->setVisible(false);
    open = positionOnOpen = true;
}

void winMapProperties::Close() {
    if (open && document == GV->editState->hParser && document)
        GV->editState->SyncWorldOptionsWithParser();
    open = false;
    document = nullptr;
    cWindow::Close();
}

void winMapProperties::DrawImGui() {
    if (!open) return;
    auto* editor = GV->editState;
    if (!document || document != editor->hParser) { Close(); return; }
    // Validation dialogs and object-edit states must render above this panel.
    if (!editor->_isOnTop()) return;
    if (positionOnOpen) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowFocus();
        positionOnOpen = false;
    }
    ImGui::SetNextWindowSize(ImVec2(460.f, 290.f), ImGuiCond_Always);
    const auto codePage = LegacyText::CodePage(GV->Lang->GetCode());
    const auto title = LegacyText::ToUtf8(GETL(Lang_MapProperties), codePage) + "###WapMapMapProperties";
    bool visible = true;
    const bool drawContent = WapMapImGui::BeginFixedWindow(title.c_str(), &visible,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    bool save = false, cancel = !visible;
    std::array<std::optional<std::string>, 7> encoded;
    if (drawContent) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, GETL(Lang_MapProperties));
        const auto children = myWin.getChildrenArea();
        const auto origin = ImGui::GetWindowPos();
        const char* labels[] = {GETL(Lang_Name), GETL(Lang_Author), GETL(Lang_Date), GETL(Lang_PathREZ),
            GETL(Lang_PathTiles), GETL(Lang_PathPAL), GETL(Lang_Exe)};
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
            ImVec2(5.f, std::max(0.f, (20.f - GV->fntMyriad16->GetHeight()) * 0.5f)));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, WapMapImGui::Grey(0x23));
        bool valid = true;
        for (int field = 0; field < 7; ++field) {
            ImGui::PushID(field);
            WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16,
                ImVec2(origin.x + children.x + 5.f, origin.y + children.y + rows[field]), labels[field]);
            ImGui::SetCursorPos(ImVec2(children.x + 120.f, children.y + static_cast<float>(rows[field])));
            ImGui::SetNextItemWidth(330.f);
            ImGui::BeginDisabled(field == 2);
            ImGui::InputText("##field", fields[field].data(), limits[field] * 4 + 1);
            ImGui::EndDisabled();
            if (field != 2) {
                encoded[field] = LegacyText::FromUtf8(fields[field].data(), codePage, limits[field]);
                if (!encoded[field]) {
                    valid = false;
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(220, 75, 75, 255));
                    if (ImGui::IsItemHovered()) {
                        const auto message = LegacyText::ToUtf8(GETL2S("MapProperties", "InvalidText"), codePage);
                        ImGui::SetTooltip("%s (%zu)", message.c_str(), limits[field]);
                    }
                }
            }
            ImGui::PopID();
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);
        auto* draw = ImGui::GetWindowDrawList();
        for (int separator : {93, 203}) {
            draw->AddLine(ImVec2(origin.x, origin.y + children.y + separator),
                ImVec2(origin.x + 460.f, origin.y + children.y + separator), IM_COL32(7, 7, 7, 127));
            draw->AddLine(ImVec2(origin.x, origin.y + children.y + separator + 1.f),
                ImVec2(origin.x + 460.f, origin.y + children.y + separator + 1.f), IM_COL32(86, 86, 86, 127));
        }
        ImGui::SetCursorPos(ImVec2(children.x + 5.f, children.y + 210.f));
        ImGui::PushID("compression");
        WapMapImGui::BitmapCheckbox(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL(Lang_Compress), compression);
        ImGui::PopID();
        ImGui::SetCursorPos(ImVec2(children.x + 250.f, children.y + 210.f));
        ImGui::PushID("z");
        WapMapImGui::BitmapCheckbox(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL(Lang_ZCoord), useZ);
        ImGui::PopID();
        ImGui::BeginDisabled(!valid);
        ImGui::SetCursorPos(ImVec2(children.x + 240.f, children.y + 235.f));
        ImGui::PushID("ok");
        save = WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL(Lang_OK), ImVec2(100.f, 33.f));
        ImGui::PopID();
        ImGui::EndDisabled();
        ImGui::SetCursorPos(ImVec2(children.x + 350.f, children.y + 235.f));
        ImGui::PushID("cancel");
        cancel |= WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL(Lang_Cancel), ImVec2(100.f, 33.f));
        ImGui::PopID();
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
            cancel |= ImGui::IsKeyPressed(ImGuiKey_Escape);
    }
    ImGui::End();
    if (cancel) { Close(); return; }
    if (save) {
        SHR::TextField* targets[] = {editor->tfwpName, editor->tfwpAuthor, editor->tfwpDate,
            editor->tfwpREZ, editor->tfwpTiles, editor->tfwpPalette, editor->tfwpExe};
        for (int field = 0; field < 7; ++field)
            if (field != 2) targets[field]->setText(*encoded[field]);
        editor->cbwpCompress->setSelected(compression);
        editor->cbwpZCoord->setSelected(useZ);
        if (editor->SaveWorldOptions()) Close();
    }
}
#endif
