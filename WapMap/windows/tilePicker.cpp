#include "tilePicker.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../states/editing_ww.h"
#include "../databanks/tiles.h"
#include "../cBrush.h"
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../langID.h"
#include "assetBrowserUi.h"
#include "coordinateNavigation.h"
#include "tilePropertyGeometry.h"
#include <cstdio>

namespace {
void Attributes(WWD::TileAttrib* attribute, ImVec2 p, float size) {
    if (!attribute || !TilePropertyGeometry::ValidSize(attribute->getWidth(), attribute->getHeight(), attribute->getType() == WWD::AttribType_Mask)) return;
    const int w = attribute->getWidth(), h = attribute->getHeight();
    const float sx = size / w, sy = size / h;
    auto* draw = ImGui::GetWindowDrawList();
    draw->PushClipRect(p, {p.x + size, p.y + size}, true);
    const auto paint = [&](int x1, int y1, int x2, int y2, WWD::TILE_ATTRIB value) {
        x1 = std::clamp(x1, 0, w); x2 = std::clamp(x2, 0, w);
        y1 = std::clamp(y1, 0, h); y2 = std::clamp(y2, 0, h);
        if (value == WWD::Attrib_Clear || x2 <= x1 || y2 <= y1) return;
        const ImU32 color = value == WWD::Attrib_Solid ? IM_COL32(0, 0, 255, 119) :
            value == WWD::Attrib_Ground ? IM_COL32(0, 255, 0, 119) :
            value == WWD::Attrib_Climb ? IM_COL32(255, 255, 0, 119) :
            value == WWD::Attrib_Death ? IM_COL32(255, 0, 0, 119) : IM_COL32(255, 255, 255, 119);
        draw->AddRectFilled({p.x + x1 * sx, p.y + y1 * sy}, {p.x + x2 * sx, p.y + y2 * sy}, color);
    };
    if (attribute->getType() == WWD::AttribType_Single) paint(0, 0, w, h, static_cast<WWD::SingleTileAttrib*>(attribute)->getAttrib());
    else if (attribute->getType() == WWD::AttribType_Double) {
        auto* a = static_cast<WWD::DoubleTileAttrib*>(attribute); const auto r = a->getMask();
        const int x1 = std::clamp(r.x1, 0, w), y1 = std::clamp(r.y1, 0, h);
        const int x2 = static_cast<int>(std::clamp(std::int64_t(r.x2) + 1, std::int64_t(0), std::int64_t(w)));
        const int y2 = static_cast<int>(std::clamp(std::int64_t(r.y2) + 1, std::int64_t(0), std::int64_t(h)));
        paint(x1, y1, x2, y2, a->getInsideAttrib());
        paint(0, 0, w, y1, a->getOutsideAttrib()); paint(0, y2, w, h, a->getOutsideAttrib());
        paint(0, y1, x1, y2, a->getOutsideAttrib()); paint(x2, y1, w, y2, a->getOutsideAttrib());
    } else {
        const auto* data = static_cast<WWD::MaskTileAttrib*>(attribute)->getData();
        for (int y = 0; data && y < h; ++y) for (int x = 0; x < w;) {
            const int start = x; const auto value = data[y * w + x++];
            while (x < w && data[y * w + x] == value) ++x;
            paint(start, y, x, y + 1, value);
        }
    }
    draw->PopClipRect();
}
}
winTilePicker::winTilePicker() : cWindow(GETL2S("TilePicker", "WinCaption")) {}
bool winTilePicker::Eligible() const {
    auto& e = *GV->editState;
    return e.hParser && (e.iActiveTool == EWW_TOOL_PENCIL || e.iActiveTool == EWW_TOOL_FILL || e.iActiveTool == EWW_TOOL_BRUSH);
}
void winTilePicker::Open() {
    if (!Eligible()) return;
    if (!open || document != GV->editState->hParser) positionOnOpen = true;
    open = true; document = GV->editState->hParser;
    stopRequested = pickRequested = false; pendingAction = nullptr;
    previousHeight = GV->editState->vPort->GetHeight();
    GV->editState->winTilePicker->setVisible(false);
}
void winTilePicker::Close() {
    open = stopRequested = pickRequested = false; pendingAction = nullptr;
    document = nullptr; pendingPlane = nullptr; pendingSet = nullptr; pendingBrush = nullptr;
    drafts.clear(); cWindow::Close();
}
void winTilePicker::Think() {
    if (!Eligible()) { Close(); return; }
    if (!open) Open();
    auto& e = *GV->editState;
    if (document != e.hParser) { Close(); return; }
    if (!e._isOnTop()) return;
    if (stopRequested) { e.SetTool(EWW_TOOL_NONE); Close(); return; }
    if (pendingAction) {
        auto* action = pendingAction; pendingAction = nullptr;
        drafts.clear();
        e.mainListener->action(gcn::ActionEvent(action, ""));
        e.winTilePicker->setVisible(false);
    }
    if (!Eligible()) { Close(); return; }
    auto* set = e.hTileset ? e.hTileset->GetSet(e.GetActivePlane()->GetImageSet(0)) : nullptr;
    if (e.iActiveTool == EWW_TOOL_BRUSH && (!set || e.iTilePicked < EWW_TILE_NONE || e.iTilePicked >= set->GetBrushesCount())) {
        e.iTilePicked = EWW_TILE_NONE; e.RebuildTilePicker(); drafts.clear();
    } else if (e.iActiveTool != EWW_TOOL_BRUSH && e.iTilePicked >= 0 && (!set || e.iTilePicked >= set->GetTilesCount())) {
        e.iTilePicked = EWW_TILE_NONE; e.RebuildTilePicker();
    }
    if (pickRequested) {
        pickRequested = false;
        if (set && set == pendingSet && e.GetActivePlane() == pendingPlane && brushPick == (e.iActiveTool == EWW_TOOL_BRUSH)) {
            int picked = EWW_TILE_NONE;
            if (brushPick) {
                for (int i = 0; i < set->GetBrushesCount(); ++i) if (set->GetBrushByIterator(i) == pendingBrush) picked = i;
            } else if (pendingTile < 0) picked = pendingTile;
            else for (int i = 0; i < set->GetTilesCount(); ++i) if (set->GetTileByIterator(i)->GetID() == pendingTile) { picked = i; break; }
            if (picked != EWW_TILE_NONE) {
                const int old = e.iTilePicked; e.iTilePicked = picked;
                if (brushPick) e.HandleBrushSwitch(old, picked);
                else e.RebuildTilePicker();
                e.vTileGhosting.clear(); e.vPort->MarkToRedraw(); drafts.clear();
            }
        }
    }
    if (previousHeight != e.vPort->GetHeight()) {
        previousHeight = e.vPort->GetHeight(); e.RebuildTilePicker();
        if (e.iActiveTool == EWW_TOOL_BRUSH && set && e.iTilePicked >= 0 && e.iTilePicked < set->GetBrushesCount()) e.HandleBrushSwitch(e.iTilePicked, e.iTilePicked);
    }
}
void winTilePicker::DrawImGui() {
    if (!Eligible()) { Close(); return; }
    if (!open) Open();
    auto& e = *GV->editState;
    if (document != e.hParser || !e._isOnTop()) return;
    const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
    const auto title = LegacyText::ToUtf8(GETL2S("TilePicker", "WinCaption"), cp) + "###WapMapTilePicker";
    if (positionOnOpen) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos({viewport->Pos.x + e.vPort->GetX(), viewport->Pos.y + e.vPort->GetY()}, ImGuiCond_FirstUseEver);
        positionOnOpen = false;
    }
    const float height = static_cast<float>(e.winTilePicker->getHeight());
    ImGui::SetNextWindowSize({250, height}, ImGuiCond_Always);
    bool visible = true;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, GETL2S("TilePicker", "WinCaption"));
        const auto area = e.winTilePicker->getChildrenArea();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {4, 1});
        for (auto* widget : e.winTilePicker->getChildren()) {
            if (!widget->isLocallyVisible() || widget == e.vptpi || widget == e.slitpiPicker) continue;
            ImGui::PushID(widget);
            ImGui::SetCursorPos({static_cast<float>(area.x + widget->getX()), static_cast<float>(area.y + widget->getY())});
            if (widget == e.cbtpiShowTileID) ImGui::SetCursorPos({8, height - 54});
            if (widget == e.cbtpiShowProperties) ImGui::SetCursorPos({8, height - 28});
            if (widget == e.buttpiReloadBrush) ImGui::SetCursorPos({5, height - 40});
            if (widget == e.labtpiReloadBrush) ImGui::SetCursorPos({40, height - 34});
            ImGui::BeginDisabled(!widget->isLocallyEnabled() || pendingAction != nullptr);
            if (auto* label = dynamic_cast<SHR::Lab*>(widget)) WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, ImGui::GetCursorScreenPos(), label->getCaption().c_str());
            else if (auto* button = dynamic_cast<SHR::But*>(widget)) {
                if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, button->getCaption().c_str(), {static_cast<float>(button->getWidth()), static_cast<float>(button->getHeight())})) pendingAction = button;
                const auto p = ImGui::GetItemRectMin();
                if (const auto* icon = button->getIcon()) WapMapImGui::Sprite(ImGui::GetWindowDrawList(), *hge, icon, {p.x + (button->getWidth() - icon->GetWidth()) / 2, p.y + (button->getHeight() - icon->GetHeight()) / 2});
                if (button->isHighlighted()) ImGui::GetWindowDrawList()->AddRect(p, ImGui::GetItemRectMax(), IM_COL32(23, 150, 255, 255));
            } else if (auto* box = dynamic_cast<SHR::CBox*>(widget)) {
                bool value = box->isSelected();
                if (WapMapImGui::BitmapCheckbox(*hge, *GV->fntMyriad16, *GV->hGfxInterface, box->getCaption().c_str(), value)) box->setSelected(value);
            } else if (auto* slider = dynamic_cast<SHR::Slider*>(widget)) {
                int value = static_cast<int>(std::lround(slider->getValue())); ImGui::SetNextItemWidth(static_cast<float>(slider->getWidth()));
                if (ImGui::SliderInt("##value", &value, static_cast<int>(slider->getScaleStart()), static_cast<int>(slider->getScaleEnd()))) { slider->setValue(value); pendingAction = slider; }
            } else if (auto* field = dynamic_cast<SHR::TextField*>(widget)) {
                auto [entry, added] = drafts.try_emplace(widget); auto& draft = entry->second;
                if (added || draft.legacy != field->getText()) { const auto text = LegacyText::ToUtf8(field->getText(), cp); std::snprintf(draft.text.data(), draft.text.size(), "%s", text.c_str()); draft.legacy = field->getText(); }
                ImGui::SetNextItemWidth(static_cast<float>(field->getWidth()));
                const bool changed = ImGui::InputText("##text", draft.text.data(), draft.text.size(), field->IsNumericalOnly() ? ImGuiInputTextFlags_CharsDecimal : ImGuiInputTextFlags_None);
                const auto encoded = LegacyText::FromUtf8(draft.text.data(), cp, 128);
                const bool valid = encoded && (!field->IsNumericalOnly() || CoordinateNavigation::Parse(*encoded).has_value());
                if (!valid) ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(220, 75, 75, 255));
                if (changed && valid) { field->setText(*encoded); draft.legacy = *encoded; }
            } else if (auto* dropdown = dynamic_cast<SHR::DropDown*>(widget)) {
                auto* model = dropdown->getListModel(); const int selected = dropdown->getSelected();
                const auto text = model && selected >= 0 && selected < model->getNumberOfElements() ? LegacyText::ToUtf8(model->getElementAt(selected), cp) : std::string{};
                ImGui::SetNextItemWidth(static_cast<float>(dropdown->getWidth()));
                if (ImGui::BeginCombo("##choice", text.c_str())) {
                    for (int i = 0; model && i < model->getNumberOfElements(); ++i) { ImGui::PushID(i); const auto label = LegacyText::ToUtf8(model->getElementAt(i), cp); if (ImGui::Selectable(label.c_str(), i == selected)) dropdown->setSelected(i); ImGui::PopID(); }
                    ImGui::EndCombo();
                }
            }
            ImGui::EndDisabled(); ImGui::PopID();
        }
        ImGui::PopStyleVar();
        const bool brushes = e.iActiveTool == EWW_TOOL_BRUSH;
        auto* set = e.hTileset ? e.hTileset->GetSet(e.GetActivePlane()->GetImageSet(0)) : nullptr;
        ImGui::SetCursorPos({5, static_cast<float>(e.iTilePickerOffUp + 16)});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {4, 4}); ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});
        ImGui::PushStyleColor(ImGuiCol_ChildBg, WapMapImGui::Grey(0x13));
        if (ImGui::BeginChild("tiles", {240, static_cast<float>(std::max(40, e.slitpiPicker->getHeight()))}, ImGuiChildFlags_NavFlattened)) {
            const int count = set ? (brushes ? set->GetBrushesCount() : set->GetTilesCount() + 3) : 0;
            const int columns = brushes ? 1 : 4;
            const float rowHeight = brushes ? 60.f : e.cbtpiShowTileID->isSelected() ? 74.f : 56.f;
            const auto start = ImGui::GetCursorPos(); ImGuiListClipper clipper; clipper.Begin((count + columns - 1) / columns, rowHeight);
            while (clipper.Step()) for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                for (int column = 0; column < columns; ++column) {
                    const int item = row * columns + column; if (item >= count) break;
                    ImGui::PushID(item); ImGui::SetCursorPos({start.x + column * 54.f, start.y + row * rowHeight});
                    const auto p = ImGui::GetCursorScreenPos();
                    const int index = brushes ? item : item - 3;
                    auto* brush = brushes ? set->GetBrushByIterator(index) : nullptr;
                    auto* tile = !brushes && index >= 0 ? set->GetTileByIterator(index) : nullptr;
                    const bool selected = e.iTilePicked == (brushes || index >= 0 ? index : index - 1);
                    if (ImGui::InvisibleButton("##asset", {brushes ? ImGui::GetContentRegionAvail().x : 52.f, rowHeight - 4}, ImGuiButtonFlags_EnableNav)) {
                        pendingSet = set; pendingPlane = e.GetActivePlane(); pickRequested = true; brushPick = brushes;
                        if (brushes) pendingBrush = brush; else pendingTile = tile ? tile->GetID() : index - 1;
                    }
                    const bool hovered = ImGui::IsItemHovered() || WapMapImGui::IsKeyboardFocused();
                    hgeSprite* sprite = tile ? tile->GetImage() : nullptr;
                    if (brush) { auto* icon = set->GetTile(brush->GetIcon()); sprite = icon ? icon->GetImage() : GV->gcnParts.sprIconError; }
                    AssetBrowserUi::Preview(*hge, sprite, {p.x + 2, p.y + 2}, 48, true, false, selected, hovered);
                    if (!brushes && index < 0) {
                        if (index == -1 && e.hDataCtrl->GetPalette()) {
                            const DWORD color = e.hDataCtrl->GetPalette()->GetColor(std::clamp(e.GetActivePlane()->GetFillColor(), 0, 255));
                            ImGui::GetWindowDrawList()->AddRectFilled({p.x + 3, p.y + 3}, {p.x + 49, p.y + 49}, IM_COL32(GETR(color), GETG(color), GETB(color), 255));
                        } else WapMapImGui::Sprite(ImGui::GetWindowDrawList(), *hge, GV->sprIcons[index == -3 ? Icon_Pipette : Icon_Erase], {p.x + 10, p.y + 10});
                    }
                    if (tile && e.cbtpiShowProperties->isSelected() && tile->GetID() >= 0 && tile->GetID() < document->GetTileAttribsCount()) Attributes(document->GetTileAttribs(tile->GetID()), {p.x + 2, p.y + 2}, 48);
                    if (tile && e.cbtpiShowTileID->isSelected()) { const auto id = std::to_string(tile->GetID()); WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, {p.x + 2, p.y + 53}, id.c_str()); }
                    if (brush) { WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, {p.x + 58, p.y + 15}, brush->GetName()); if (hovered) { const auto text = LegacyText::ToUtf8(brush->GetName(), cp); ImGui::SetTooltip("%s", text.c_str()); } }
                    ImGui::PopID();
                }
                ImGui::SetCursorPos({start.x, start.y + (row + 1) * rowHeight}); ImGui::Dummy({1, 0});
            }
        }
        ImGui::EndChild(); ImGui::PopStyleColor(); ImGui::PopStyleVar(2);
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) stopRequested = true;
    }
    ImGui::End(); if (!visible) stopRequested = true;
}
#endif
