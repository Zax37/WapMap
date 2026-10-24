#pragma once
#ifdef WAPMAP_ENABLE_IMGUI
#include "../imguiTheme.h"
#include "../globals.h"
#include "../states/editing_ww.h"
#include "../databanks/imageSets.h"
extern HGE* hge;

namespace NativeInventoryUi {
inline constexpr const char* Payload = "WapMapInventoryItem";
inline void Image(const cInventoryItem& item, ImVec2 p, float size, bool warn = false) {
    if (item.first.empty()) return;
    auto* bank = GV->editState->SprBank;
    auto* asset = bank ? bank->GetAssetByID(item.first.c_str()) : nullptr;
    auto* sprite = GV->sprSmiley;
    if (asset && asset->GetSpritesCount() > 0) {
        auto* image = asset->GetIMGByIterator(GV->editState->hInvCtrl->GetAnimFrame() % asset->GetSpritesCount());
        if (image && image->GetSprite()) sprite = image->GetSprite();
    }
    if (!sprite || sprite->GetWidth() <= 0 || sprite->GetHeight() <= 0) return;
    const float scale = std::min(1.f, std::min(size / sprite->GetWidth(), size / sprite->GetHeight()));
    const float width = sprite->GetWidth() * scale, height = sprite->GetHeight() * scale;
    const ImVec2 start{p.x + (size - width) / 2, p.y + (size - height) / 2};
    WapMapImGui::StretchSprite(ImGui::GetWindowDrawList(), *hge, sprite, start,
        {start.x + width, start.y + height}, IM_COL32(255, warn ? 85 : 255, warn ? 85 : 255, static_cast<int>(255 * ImGui::GetStyle().Alpha)));
}
inline int Item(const ImGuiPayload* payload) {
    return payload && payload->IsDataType(Payload) && payload->DataSize == sizeof(int) ? *static_cast<const int*>(payload->Data) : -1;
}
inline void PreviewPayload() {
    const int id = Item(ImGui::GetDragDropPayload());
    if (id < 0) return;
    ImGui::BeginTooltip();
    Image(GV->editState->hInvCtrl->GetItemByID(id), ImGui::GetCursorScreenPos(), 48);
    ImGui::Dummy({48, 48});
    ImGui::EndTooltip();
}
}
#endif
