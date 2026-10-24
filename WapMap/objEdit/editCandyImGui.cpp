#include "editCandy.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../globals.h"
#include "../langID.h"
#include "../databanks/imageSets.h"
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../windows/assetBrowserUi.h"
#include <cmath>
#include <string_view>
extern HGE* hge;

namespace ObjEdit {
constexpr int NativeCandyWidth = 339, NativeCandyGridHeight = 320;
void cEditObjCandy::Think(bool consumed) {
    if (nativeBusy) return;
    standardImgs.clear(); customImgs.clear(); otherImgs.clear();
    auto* bank = hState->SprBank;
    for (int i = 0; i < bank->getNumberOfElements(); ++i) {
        auto* asset = bank->GetAssetByIterator(i);
        const std::string name = asset->GetID();
        if (name.starts_with("LEVEL_") && GV->vstrStandardImagesets.contains(name)) standardImgs.push_back(asset);
        else if (name.starts_with("CUSTOM_")) customImgs.push_back(asset);
        else otherImgs.push_back(asset);
    }
    const bool individual = asFramePick != nullptr;
    asImageSetPick = bank->GetAssetByID(hTempObj->GetImageSet());
    asFramePick = nullptr;
    if (individual && asImageSetPick && asImageSetPick->GetSpritesCount() > 0)
        asFramePick = hTempObj->GetI() < 0 ? asImageSetPick->GetIMGByIterator(0) : asImageSetPick->GetIMGByID(hTempObj->GetI());
    if (tabs->getSelectedTabIndex() != nativeTab) tabs->setSelectedTab(nativeTab);
    if (nativePick) {
        nativePick = false;
        auto* asset = bank->GetAssetByID(nativeSet.c_str());
        auto* frame = asset && nativeIndividual ? asset->GetIMGByID(nativeFrame) : nullptr;
        if (asset && (!nativeIndividual || frame)) PickImage(asset, frame);
    }
    cObjEdit::Think(consumed);
    if (!bKill) _butSave->setEnabled(hTempObj->GetImageSet()[0] != '\0');
}

void cEditObjCandy::DrawImGui() {
    win->setWidth(NativeCandyWidth);
    if (!BeginNativeWindow()) return;
    DrawNativeControls();
    ImGui::SetCursorPos({0, ImGui::GetFrameHeight() + 8});
    if (ImGui::BeginTabBar("categories")) {
        const wchar_t* keys[] = {L"Tab_Standard", L"Tab_Custom", L"Tab_Other"};
        for (int i = 0; i < TABS_COUNT; ++i) {
            const auto caption = LegacyText::ToUtf8(GETL2SV("EditObj_Candy", keys[i]), LegacyText::CodePage(GV->Lang->GetCode()));
            if (ImGui::BeginTabItem(caption.c_str(), nullptr, nativeTabFocus && nativeTab == i ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None)) {
                nativeTab = i;
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    struct Cell { cSprBankAsset* asset; cSprBankAssetIMG* frame; bool individual; };
    std::vector<Cell> cells;
    auto* bank = hState->SprBank;
    for (int i = 0; i < bank->getNumberOfElements(); ++i) {
        auto* asset = bank->GetAssetByIterator(i);
        const std::string name = asset->GetID();
        const int category = name.starts_with("LEVEL_") && GV->vstrStandardImagesets.contains(name) ? TAB_STANDARD : name.starts_with("CUSTOM_") ? TAB_CUSTOM : TAB_OTHER;
        if (category != nativeTab) continue;
        const int count = asset->GetSpritesCount();
        const bool expanded = GV->vstrNANIImagesets.contains(name);
        if (expanded && count > 0) for (int f = 0; f < count; ++f) cells.push_back({asset, asset->GetIMGByIterator(f), true});
        else {
            const int frame = count > 0 ? static_cast<int>(std::fmod(std::max(0.0, static_cast<double>(hge->Timer_GetTime()) * 5), count)) : 0;
            cells.push_back({asset, count > 0 ? asset->GetIMGByIterator(frame) : nullptr, false});
        }
    }
    ImGui::SetCursorPos({2, ImGui::GetFrameHeight() + 33});
    if (nativeTabFocus) ImGui::SetNextWindowScroll({0, static_cast<float>(std::max(0, tabScrollPositions[nativeTab]))});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    const auto gridId = "grid" + std::to_string(nativeTab);
    if (ImGui::BeginChild(gridId.c_str(), {NativeCandyWidth - 4.f, NativeCandyGridHeight - 1.f}, ImGuiChildFlags_NavFlattened)) {
        const auto start = ImGui::GetCursorPos();
        ImGuiListClipper clipper; clipper.Begin(static_cast<int>((cells.size() + 4) / 5), 64);
        while (clipper.Step()) for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
            for (int column = 0; column < 5; ++column) {
                const size_t index = static_cast<size_t>(row) * 5 + column;
                if (index >= cells.size()) break;
                const auto& cell = cells[index];
                ImGui::PushID(cell.asset); ImGui::PushID(cell.individual && cell.frame ? cell.frame->GetID() : -1);
                ImGui::SetCursorPos({start.x + column * 64, start.y + row * 64});
                const auto p = ImGui::GetCursorScreenPos();
                ImGui::BeginDisabled(!cell.frame || nativeAction);
                const bool hit = ImGui::InvisibleButton("image", {64, 64}, ImGuiButtonFlags_EnableNav);
                const bool hovered = ImGui::IsItemHovered() || WapMapImGui::IsKeyboardFocused();
                const bool selected = std::string_view(hTempObj->GetImageSet()) == cell.asset->GetID() &&
                    (!cell.individual || (cell.frame && (hTempObj->GetI() == cell.frame->GetID() || (hTempObj->GetI() < 0 && cell.asset->GetIndexOf(cell.frame) == 0))));
                AssetBrowserUi::Preview(*hge, cell.frame && cell.frame->GetSprite() ? cell.frame->GetSprite() : GV->sprSmiley,
                    {p.x + 2, p.y + 2}, 60, false, true, selected, hovered);
                if (hovered) {
                    auto label = std::string(cell.asset->GetID());
                    if (cell.individual && cell.frame) label += " (" + std::to_string(cell.frame->GetID()) + ")";
                    const auto text = LegacyText::ToUtf8(label, LegacyText::CodePage(GV->Lang->GetCode()));
                    ImGui::SetTooltip("%s", text.c_str());
                }
                if (hit) {
                    nativeSet = cell.asset->GetID(); nativeFrame = cell.frame ? cell.frame->GetID() : -1;
                    nativeIndividual = cell.individual; nativePick = true; nativeAction = imgsCon;
                }
                ImGui::EndDisabled(); ImGui::PopID(); ImGui::PopID();
            }
            ImGui::SetCursorPos({start.x, start.y + (row + 1) * 64});
        }
        if (cells.empty()) {
            const auto p = ImGui::GetWindowPos();
            const auto* label = GETL2S("EditObj_Candy", "NoImages");
            WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16,
                {p.x + (ImGui::GetWindowWidth() - GV->fntMyriad16->GetStringWidth(label)) / 2, p.y + 140}, label);
        }
    }
    ImGui::EndChild(); ImGui::PopStyleVar();
    nativeTabFocus = false;
    EndNativeWindow();
}
}
#endif
