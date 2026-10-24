#pragma once
#ifdef WAPMAP_ENABLE_IMGUI
#include "../imguiTheme.h"
#include "../cInterfaceSheet.h"
#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wLabel.h"

namespace AssetBrowserUi {
struct State {
    bool open{}, position{}, resetGrid{}, busy{};
    WWD::Parser* document{};
    gcn::Widget* action{};
    const void* set{};
    const void* asset{};
    void Close() { open = false; action = nullptr; document = nullptr; }
};
inline void Position(const gcn::Rectangle& area, float x, float y) { ImGui::SetCursorPos({area.x + x, area.y + y}); }
inline void Label(HGE& renderer, const hgeFont& font, ImVec2 origin, const gcn::Rectangle& area, SHR::Lab* label) {
    if (label->isLocallyVisible()) WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), renderer, font,
        {origin.x + area.x + label->getX(), origin.y + area.y + label->getY()}, label->getCaption().c_str());
}
inline void Preview(HGE& renderer, hgeSprite* sprite, ImVec2 p, float size, bool stretch, bool checker, bool selected = false, bool hovered = false) {
    auto* draw = ImGui::GetWindowDrawList();
    draw->AddRect({p.x - 2, p.y - 2}, {p.x + size + 2, p.y + size + 2}, IM_COL32(19, 19, 19, 255));
    draw->AddRect({p.x - 1, p.y - 1}, {p.x + size + 1, p.y + size + 1}, selected ? IM_COL32(23, 150, 255, 255) : hovered ? IM_COL32(225, 225, 225, 255) : IM_COL32(86, 86, 86, 255));
    draw->PushClipRect(p, {p.x + size, p.y + size}, true);
    if (checker) WapMapImGui::Sprite(draw, renderer, GV->sprCheckboard, p);
    if (sprite && sprite->GetWidth() > 0 && sprite->GetHeight() > 0) {
        const float scale = stretch ? 1.f : std::min(1.f, std::min(size / sprite->GetWidth(), size / sprite->GetHeight()));
        const float width = stretch ? size : sprite->GetWidth() * scale;
        const float height = stretch ? size : sprite->GetHeight() * scale;
        const ImVec2 start{p.x + (size - width) / 2, p.y + (size - height) / 2};
        WapMapImGui::StretchSprite(draw, renderer, sprite, start, {start.x + width, start.y + height}, IM_COL32_WHITE);
    }
    draw->PopClipRect();
}
inline bool Button(HGE& renderer, const hgeFont& font, const cInterfaceSheet& skin, const gcn::Rectangle& area, SHR::But* button, bool valid) {
    if (!button->isLocallyVisible() || !button->getParent()) return false;
    ImGui::PushID(button); Position(area, static_cast<float>(button->getX()), static_cast<float>(button->getY()));
    ImGui::BeginDisabled(!valid || !button->isLocallyEnabled());
    const auto* icon = button->getIcon(); const auto& caption = button->getCaption();
    const float total = font.GetStringWidth(caption.c_str()) + (icon ? icon->GetWidth() + 4 : 0);
    const bool hit = WapMapImGui::BitmapButton(renderer, font, skin, "", {static_cast<float>(button->getWidth()), static_cast<float>(button->getHeight())});
    const auto p = ImGui::GetItemRectMin(); float x = p.x + (button->getWidth() - total) / 2;
    const auto color = IM_COL32(225, 225, 225, static_cast<int>(255 * ImGui::GetStyle().Alpha));
    if (icon) {
        WapMapImGui::Sprite(ImGui::GetWindowDrawList(), renderer, icon, {x, p.y + (button->getHeight() - icon->GetHeight()) / 2}, color);
        x += icon->GetWidth() + 4;
    }
    WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), renderer, font,
        {x, p.y + (button->getHeight() - font.GetHeight()) / 2 - 2}, caption.c_str(), color);
    ImGui::EndDisabled(); ImGui::PopID(); return hit;
}
template <typename Asset>
Asset* Grid(HGE& renderer, const hgeFont& font, const std::vector<Asset*>& assets, Asset* selected, bool images) {
    const float cellWidth = images ? 100.f : 60.f, cellHeight = images ? 120.f : 80.f, size = images ? 90.f : 48.f;
    const int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / cellWidth));
    const auto start = ImGui::GetCursorPos();
    Asset* clicked = nullptr;
    ImGuiListClipper clipper; clipper.Begin(static_cast<int>((assets.size() + columns - 1) / columns), cellHeight);
    while (clipper.Step()) for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
        for (int column = 0; column < columns; ++column) {
            const size_t index = static_cast<size_t>(row) * columns + column;
            if (index >= assets.size()) break;
            auto* asset = assets[index];
            ImGui::SetCursorPos({start.x + column * cellWidth, start.y + row * cellHeight});
            ImGui::PushID(asset);
            const auto p = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("##asset", {cellWidth, cellHeight}, ImGuiButtonFlags_EnableNav)) clicked = asset;
            Preview(renderer, asset->GetImage() ? asset->GetImage() : GV->sprSmiley,
                {p.x + 5, p.y + 5}, size, !images, true, selected == asset, ImGui::IsItemHovered() || ImGui::IsItemFocused());
            const auto id = std::to_string(asset->GetID());
            WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), renderer, font,
                {p.x + 5 + (size - font.GetStringWidth(id.c_str())) / 2, p.y + size + 10}, id.c_str());
            ImGui::PopID();
        }
        ImGui::SetCursorPos({start.x, start.y + (row + 1) * cellHeight});
    }
    return clicked;
}
}
#endif
