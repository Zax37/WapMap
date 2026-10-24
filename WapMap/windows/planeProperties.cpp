#include "planeProperties.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../states/editing_ww.h"
#include "../databanks/tiles.h"
#include "../cInterfaceSheet.h"
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../langID.h"
#include "coordinateNavigation.h"
#include "planeGeometry.h"
#include <cstdio>
#include <limits>

namespace {
std::array<SHR::TextField*, 10> inputs(State::EditingWW& e) {
    return {e.tfpmName, e.tfpmPlaneSizeX, e.tfpmPlaneSizeY, e.tfpmTileSizeX, e.tfpmTileSizeY,
            e.tfpmMovX, e.tfpmMovY, e.tfpmZCoord, e.tfpmOffX, e.tfpmOffY};
}
std::array<SHR::CBox*, 5> checkboxes(State::EditingWW& e) {
    return {e.cbpmFlagMainPlane, e.cbpmFlagNoDraw, e.cbpmFlagWrapX, e.cbpmFlagWrapY, e.cbpmAutoTileSize};
}
}

winPlaneProperties::winPlaneProperties() : cWindow(GETL2S("PlaneProperties", "Caption"), 735, 300) {}

void winPlaneProperties::load(int index) {
    auto& e = *GV->editState;
    selected = index;
    e.lbpmPlanes->setSelected(index);
    e.SyncPlaneProperties();
    planeCount = document->GetPlanesCount();
    plane = index < planeCount ? document->GetPlane(index) : nullptr;
    const auto targets = inputs(e);
    const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
    for (int i = 0; i < 10; ++i) {
        const auto text = i == 0 ? LegacyText::ToUtf8(targets[i]->getText(), cp) : targets[i]->getText();
        std::snprintf(fields[i].data(), fields[i].size(), "%s", text.c_str());
    }
    // Resource banks can be empty when opening a map without its REZ/custom
    // graphics. Keep the map's declared sets available in that case as well.
    tileSets.names.clear();
    const auto addSet = [&](const char* name) {
        if (name && *name && std::find(tileSets.names.begin(), tileSets.names.end(), name) == tileSets.names.end())
            tileSets.names.emplace_back(name);
    };
    for (int i = 0; i < e.hTileset->GetSetsCount(); ++i) addSet(e.hTileset->GetSet(i)->GetName());
    for (int i = 0; i < planeCount; ++i)
        for (int j = 0; j < document->GetPlane(i)->GetImageSetsCount(); ++j)
            addSet(document->GetPlane(i)->GetImageSet(j));
    tileSet = tileSets.names.empty() ? -1 : 0;
    if (plane && plane->GetImageSetsCount()) {
        const auto found = std::find(tileSets.names.begin(), tileSets.names.end(), plane->GetImageSet(0));
        if (found != tileSets.names.end()) tileSet = static_cast<int>(found - tileSets.names.begin());
    }
    anchor = 5;
    const auto boxes = checkboxes(e);
    for (int i = 0; i < 5; ++i) flags[i] = boxes[i]->isSelected();
    e.winpmMain->setVisible(false);
}

void winPlaneProperties::Open() {
    if (!GV->editState->hParser) return;
    document = GV->editState->hParser;
    const int count = document->GetPlanesCount();
    int index = GV->editState->lbpmPlanes->getSelected();
    if (index < 0 || index > count) index = 0;
    load(index);
    open = positionOnOpen = true;
}

void winPlaneProperties::Close() {
    open = false;
    document = nullptr;
    plane = nullptr;
    pending = 0;
    cWindow::Close();
}

void winPlaneProperties::DrawImGui() {
    if (!open || busy) return;
    auto& e = *GV->editState;
    if (document != e.hParser || !document) { Close(); return; }
    // Index-based legacy save/delete calls must never target a different plane.
    if (document->GetPlanesCount() != planeCount ||
        (plane && (selected >= planeCount || document->GetPlane(selected) != plane))) { Close(); return; }
    if (!e._isOnTop()) return;
    if (positionOnOpen) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowFocus();
        positionOnOpen = false;
    }
    ImGui::SetNextWindowSize(ImVec2(735.f, 300.f), ImGuiCond_Always);
    const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
    const auto title = LegacyText::ToUtf8(GETL2S("PlaneProperties", "Caption"), cp) + "###WapMapPlaneProperties";
    bool visible = true;
    bool save = false, remove = false;
    const bool content = WapMapImGui::BeginFixedWindow(title.c_str(), &visible,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    if (content) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, GETL2S("PlaneProperties", "Caption"));
        const auto origin = ImGui::GetWindowPos();
        const auto area = myWin.getChildrenArea();
        const auto label = [&](const char* text, float x, float y) {
            WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16,
                ImVec2(origin.x + area.x + x, origin.y + area.y + y), text);
        };
        const auto position = [&](float x, float y) { ImGui::SetCursorPos(ImVec2(area.x + x, area.y + y)); };
        label(GETL2S("PlaneProperties", "SelectPlane"), 8, 16);
        position(8, 40);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, WapMapImGui::Grey(0x13));
        if (ImGui::BeginChild("planes", ImVec2(200.f, 235.f))) {
            for (int i = 0; i <= planeCount; ++i) {
                ImGui::PushID(i);
                const char* text = i == planeCount ? GETL2S("PlaneProperties", "NewPlane") : document->GetPlane(i)->GetName();
                const auto utf8 = LegacyText::ToUtf8(text, cp);
                if (ImGui::Selectable(utf8.c_str(), selected == i)) load(i);
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
        const auto targets = inputs(e);
        auto name = LegacyText::FromUtf8(fields[0].data(), cp, 63);
        bool numeric = true;
        std::array<int, 10> numbers{};
        for (int i = 1; i < 10; ++i) {
            const auto number = CoordinateNavigation::Parse(fields[i].data());
            if (!number) numeric = false;
            else numbers[i] = *number;
        }
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
            ImVec2(5.f, std::max(0.f, (20.f - GV->fntMyriad16->GetHeight()) * 0.5f)));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, WapMapImGui::Grey(0x23));
        for (int i = 0; i < 8; ++i) {
            ImGui::PushID(i);
            position(static_cast<float>(targets[i]->getX()), static_cast<float>(targets[i]->getY()));
            ImGui::SetNextItemWidth(static_cast<float>(targets[i]->getWidth()));
            const size_t capacity = i == 0 ? fields[i].size() : (i <= 2 ? 6 : (i <= 4 ? 5 : 32));
            ImGui::InputText("##input", fields[i].data(), capacity,
                i == 0 ? ImGuiInputTextFlags_None : ImGuiInputTextFlags_CharsDecimal);
            if ((i == 0 && !name) || (i != 0 && !CoordinateNavigation::Parse(fields[i].data())))
                ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(220, 75, 75, 255));
            ImGui::PopID();
        }
        // Native combo keeps keyboard/scroll behavior; labels use the imported font.
        position(static_cast<float>(e.ddpmTileSet->getX()), static_cast<float>(e.ddpmTileSet->getY()));
        ImGui::SetNextItemWidth(static_cast<float>(e.ddpmTileSet->getWidth()));
        const int sets = tileSets.getNumberOfElements();
        const bool validSet = tileSet >= 0 && tileSet < sets;
        const auto preview = validSet ? LegacyText::ToUtf8(tileSets.getElementAt(tileSet), cp) : std::string{};
        if (ImGui::BeginCombo("##tileset", preview.c_str())) {
            for (int i = 0; i < sets; ++i) {
                ImGui::PushID(i);
                const auto text = LegacyText::ToUtf8(tileSets.getElementAt(i), cp);
                if (ImGui::Selectable(text.c_str(), tileSet == i)) tileSet = i;
                if (tileSet == i) ImGui::SetItemDefaultFocus();
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);
        name = LegacyText::FromUtf8(fields[0].data(), cp, 63);
        numeric = true;
        for (int i = 1; i < 10; ++i) {
            const auto number = CoordinateNavigation::Parse(fields[i].data());
            if (!number) numeric = false;
            else numbers[i] = *number;
        }
        SHR::Lab* labels[] = {e.labpmName, e.labpmTileSet, e.labpmPlaneSize, e.labpmAnchor, e.labpmWidth,
            e.labpmHeight, e.labpmTileSize, e.labpmTileSize_x, e.labpmMovement, e.labpmMovX, e.labpmMovY, e.labpmZCoord, e.labpmFlags};
        for (auto* widget : labels) label(widget->getCaption().c_str(), static_cast<float>(widget->getX()), static_cast<float>(widget->getY()));
        const auto boxes = checkboxes(e);
        for (int i = 0; i < 5; ++i) {
            ImGui::PushID(i);
            position(static_cast<float>(boxes[i]->getX()), static_cast<float>(boxes[i]->getY()));
            WapMapImGui::BitmapCheckbox(*hge, *GV->fntMyriad16, *GV->hGfxInterface, boxes[i]->getCaption().c_str(), flags[i]);
            ImGui::PopID();
        }
        SHR::But* anchors[] = {e.butpmResUL, e.butpmResU, e.butpmResUR, e.butpmResL, e.butpmResC,
            e.butpmResR, e.butpmResDL, e.butpmResD, e.butpmResDR};
        // Only update legacy drafts/icons here, never the actual plane.
        e.lbpmPlanes->setSelected(selected);
        e.tfpmPlaneSizeX->setText(fields[1].data());
        e.tfpmPlaneSizeY->setText(fields[2].data());
        e.SetAnchorPlaneProperties(anchor);
        ImGui::BeginDisabled(!plane);
        for (int i = 0; i < 9; ++i) {
            ImGui::PushID(i + 100);
            position(static_cast<float>(anchors[i]->getX()), static_cast<float>(anchors[i]->getY()));
            if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, "", ImVec2(18, 18))) anchor = i + 1;
            const auto p = ImGui::GetItemRectMin();
            WapMapImGui::Sprite(ImGui::GetWindowDrawList(), *hge, anchors[i]->getIcon(), ImVec2(p.x + 1.f, p.y));
            ImGui::PopID();
        }
        ImGui::EndDisabled();
        const bool safeDimensions = PlaneArithmeticFits(numbers[1], numbers[2], numbers[3], numbers[4]);
        ImGui::BeginDisabled(!name || !numeric || !safeDimensions || tileSet < 0 || tileSet >= sets);
        position(500, 245);
        ImGui::PushID("save");
        save = WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL(Lang_Save), ImVec2(100, 33));
        ImGui::PopID();
        ImGui::EndDisabled();
        ImGui::BeginDisabled(!plane);
        position(610, 245);
        ImGui::PushID("delete");
        remove = WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL(Lang_Delete), ImVec2(100, 33));
        ImGui::PopID();
        ImGui::EndDisabled();
        for (float divider : {220.f, 491.f})
            ImGui::GetWindowDrawList()->AddLine(ImVec2(origin.x + divider, origin.y + 32.f),
                ImVec2(origin.x + divider, origin.y + 292.f), IM_COL32(86, 86, 86, 127));
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) visible = false;
    }
    ImGui::End();
    if (!visible) { Close(); return; }
    if (save) {
        const auto targets = inputs(e);
        targets[0]->setText(*LegacyText::FromUtf8(fields[0].data(), cp, 63));
        for (int i = 1; i < 10; ++i) targets[i]->setText(fields[i].data());
        const auto boxes = checkboxes(e);
        for (int i = 0; i < 5; ++i) boxes[i]->setSelected(flags[i]);
        e.lbpmPlanes->setSelected(selected);
        e.SetAnchorPlaneProperties(anchor);
        pending = 1;
    } else if (remove) {
        pending = 2;
    }
}

void winPlaneProperties::Think() {
    if (!pending || busy || !open) return;
    auto& e = *GV->editState;
    if (document != e.hParser || !document || document->GetPlanesCount() != planeCount ||
        (plane && document->GetPlane(selected) != plane)) { Close(); return; }
    if (!e._isOnTop()) return;
    const int action = pending;
    pending = 0;
    busy = true;
    e.lbpmPlanes->setSelected(selected);
    if (action == 1) {
        e.ddpmTileSet->setListModel(&tileSets);
        e.ddpmTileSet->setSelected(tileSet);
        const bool saved = e.SavePlaneProperties();
        e.ddpmTileSet->setListModel(e.hTileset);
        if (saved && open && document == e.hParser) {
            const int savedAnchor = e.ipmAnchor;
            load(e.lbpmPlanes->getSelected());
            anchor = savedAnchor;
        }
    } else {
        e.DeletePlaneProperties();
        if (open && document == e.hParser && document->GetPlanesCount() != planeCount)
            load(std::min(selected, document->GetPlanesCount()));
    }
    busy = false;
}
#endif
