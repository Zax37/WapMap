#include "objectToolOptions.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../states/editing_ww.h"
#include "../cInterfaceSheet.h"
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../langID.h"
#include "coordinateNavigation.h"
#include "objectToolGeometry.h"
#include <limits>
#include <cstdio>

winObjectToolOptions::winObjectToolOptions(bool duplicates)
    : cWindow(duplicates ? GETL(Lang_Duplicating) : GETL2S("WinSpacing", "WinCaption")), duplicate(duplicates) {}

void winObjectToolOptions::Open() {
    auto& e = *GV->editState;
    if (!e.hParser) return;
    document = e.hParser;
    open = positionOnOpen = true;
    applyRequested = stopRequested = false;
    SHR::TextField* fields[] = {duplicate ? e.tfdTimes : e.tfspacingDistance, e.tfdOffsetX, e.tfdOffsetY, e.tfdChangeZ};
    for (int i = 0; i < (duplicate ? 4 : 1); ++i)
        std::snprintf(values[i].data(), values[i].size(), "%s", fields[i]->getText().c_str());
    (duplicate ? e.winDuplicate : e.winSpacing)->setVisible(false);
}

void winObjectToolOptions::Close() {
    open = applyRequested = stopRequested = false;
    document = nullptr;
    cWindow::Close();
}

bool winObjectToolOptions::Valid() const {
    auto& e = *GV->editState;
    if (document != e.hParser || e.vObjectsPicked.empty()) return false;
    if (!duplicate && e.vObjectsPicked.size() < 2) return false;
    std::array<int, 4> parsed{};
    for (int i = 0; i < (duplicate ? 4 : 1); ++i) {
        const auto value = CoordinateNavigation::Parse(values[i].data());
        if (!value) return false;
        parsed[i] = *value;
    }
    if (duplicate) {
        if (parsed[0] <= 0) return false;
        for (auto* obj : e.vObjectsPicked) {
            const auto params = std::array{WWD::Param_LocationX, WWD::Param_LocationY, WWD::Param_LocationZ};
            for (int axis = 0; axis < 3; ++axis) {
                if (!ObjectToolGeometry::CanOffset(obj->GetParam(params[axis]), parsed[axis + 1], parsed[0])) return false;
            }
        }
    } else {
        const auto param = e.bObjectAlignAxis ? WWD::Param_LocationY : WWD::Param_LocationX;
        int first = e.vObjectsPicked.front()->GetParam(param);
        for (auto* obj : e.vObjectsPicked) first = std::min(first, obj->GetParam(param));
        if (!ObjectToolGeometry::Fits(std::int64_t(first) + std::int64_t(parsed[0]) * static_cast<std::int64_t>(e.vObjectsPicked.size() - 1))) return false;
    }
    return true;
}

void winObjectToolOptions::Think() {
    if (!open) return;
    auto& e = *GV->editState;
    if (document != e.hParser || e.iActiveTool != (duplicate ? EWW_TOOL_DUPLICATE : EWW_TOOL_SPACEOBJ)) { Close(); return; }
    if (!e._isOnTop()) return;
    if (stopRequested) { e.SetTool(EWW_TOOL_NONE); return; }
    if (!applyRequested) return;
    applyRequested = false;
    if (!Valid()) return;
    SHR::TextField* fields[] = {duplicate ? e.tfdTimes : e.tfspacingDistance, e.tfdOffsetX, e.tfdOffsetY, e.tfdChangeZ};
    for (int i = 0; i < (duplicate ? 4 : 1); ++i) fields[i]->setText(values[i].data());
    e.mainListener->action(gcn::ActionEvent(duplicate ? e.butdOK : e.butspacingOK, ""));
}

void winObjectToolOptions::DrawImGui() {
    if (!open) return;
    auto& e = *GV->editState;
    if (document != e.hParser || !e.hParser) { Close(); return; }
    if (!e._isOnTop()) return;
    if (positionOnOpen) {
        auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, {0.5f, 0.5f});
        ImGui::SetNextWindowFocus();
    }
    auto* legacy = duplicate ? e.winDuplicate : e.winSpacing;
    ImGui::SetNextWindowSize({static_cast<float>(legacy->getWidth()), static_cast<float>(legacy->getHeight() + (duplicate ? 22 : 0))}, ImGuiCond_Always);
    const auto title = LegacyText::ToUtf8(legacy->getCaption(), LegacyText::CodePage(GV->Lang->GetCode())) +
        (duplicate ? "###WapMapDuplicate" : "###WapMapSpacing");
    bool visible = true;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, legacy->getCaption().c_str());
        SHR::Lab* labels[] = {duplicate ? e.labdTimes : e.labspacingDistance, e.labdOffsetX, e.labdOffsetY, e.labdChangeZ};
        const auto area = legacy->getChildrenArea();
        bool submitted = false;
        for (int i = 0; i < (duplicate ? 4 : 1); ++i) {
            ImGui::PushID(i);
            ImGui::SetCursorPos({area.x + 5.f, area.y + 17.f + i * 25.f});
            WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, ImGui::GetCursorScreenPos(), labels[i]->getCaption().c_str());
            ImGui::SetCursorPos({area.x + (duplicate ? 125.f : 115.f), area.y + 15.f + i * 25.f});
            ImGui::SetNextItemWidth(75);
            if (i == 0 && positionOnOpen) ImGui::SetKeyboardFocusHere();
            submitted |= ImGui::InputText("##value", values[i].data(), values[i].size(),
                ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::PopID();
        }
        positionOnOpen = false;
        const bool valid = Valid();
        ImGui::SetCursorPos({duplicate ? 68.f : 75.f, area.y + (duplicate ? 120.f : 40.f)});
        ImGui::BeginDisabled(!valid || applyRequested);
        const bool clicked = WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL(Lang_OK), {75, 25});
        ImGui::EndDisabled();
        if (valid && (clicked || submitted)) applyRequested = true;
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) visible = false;
    }
    ImGui::End();
    if (!visible) stopRequested = true;
}
winObjectBrushOptions::winObjectBrushOptions() : cWindow(GETL(Lang_BrushProperties)) {}

void winObjectBrushOptions::Open() {
    auto& e = *GV->editState;
    if (!e.hParser) return;
    document = e.hParser;
    open = positionOnOpen = true;
    stopRequested = false;
    std::snprintf(scatter[0].data(), scatter[0].size(), "%s", e.tfobrDispX->getText().c_str());
    std::snprintf(scatter[1].data(), scatter[1].size(), "%s", e.tfobrDispY->getText().c_str());
    e.winObjectBrush->setVisible(false);
}

void winObjectBrushOptions::Close() {
    open = stopRequested = false;
    document = nullptr;
    cWindow::Close();
}

void winObjectBrushOptions::Think() {
    if (!open) return;
    auto& e = *GV->editState;
    if (document != e.hParser || e.iActiveTool != EWW_TOOL_BRUSHOBJECT) { Close(); return; }
    if (stopRequested && e._isOnTop()) e.SetTool(EWW_TOOL_NONE);
}

void winObjectBrushOptions::DrawImGui() {
    if (!open) return;
    auto& e = *GV->editState;
    if (document != e.hParser || !e.hParser) { Close(); return; }
    if (!e._isOnTop()) return;
    if (positionOnOpen) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos({viewport->Pos.x + e.vPort->GetX(),
            viewport->Pos.y + e.vPort->GetY() + e.vPort->GetHeight() - 185.f}, ImGuiCond_FirstUseEver);
        positionOnOpen = false;
    }
    ImGui::SetNextWindowSize({330, 185}, ImGuiCond_Always);
    const auto title = LegacyText::ToUtf8(GETL(Lang_BrushProperties), LegacyText::CodePage(GV->Lang->GetCode())) + "###WapMapObjectBrush";
    bool visible = true;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, GETL(Lang_BrushProperties));
        const auto area = e.winObjectBrush->getChildrenArea();
        const auto position = [&](gcn::Widget* widget) {
            ImGui::SetCursorPos({static_cast<float>(area.x + widget->getX()), static_cast<float>(area.y + widget->getY())});
        };
        for (auto* checkbox : {e.cbobrUpdateRects, e.cbobrApplyScatterSeparately}) {
            position(checkbox);
            ImGui::PushID(checkbox);
            const bool separate = checkbox == e.cbobrApplyScatterSeparately;
            ImGui::BeginDisabled(separate && e.vObjectsBrushCB.size() <= 1);
            bool value = checkbox->isSelected();
            if (WapMapImGui::BitmapCheckbox(*hge, *GV->fntMyriad16, *GV->hGfxInterface, checkbox->getCaption().c_str(), value)) checkbox->setSelected(value);
            ImGui::EndDisabled();
            ImGui::PopID();
        }
        SHR::TextField* fields[] = {e.tfobrDispX, e.tfobrDispY};
        SHR::Lab* labels[] = {e.labobrDispX, e.labobrDispY};
        for (int axis = 0; axis < 2; ++axis) {
            position(labels[axis]);
            WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, ImGui::GetCursorScreenPos(), labels[axis]->getCaption().c_str());
            position(fields[axis]);
            ImGui::PushID(axis);
            ImGui::SetNextItemWidth(static_cast<float>(fields[axis]->getWidth()));
            ImGui::InputText("##scatter", scatter[axis].data(), scatter[axis].size(), ImGuiInputTextFlags_CharsDecimal);
            const auto value = CoordinateNavigation::Parse(scatter[axis].data());
            if (value && *value >= 0) fields[axis]->setText(std::to_string(*value));
            else ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(220, 75, 75, 255));
            ImGui::PopID();
        }
        position(e.labobrDistance);
        WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, ImGui::GetCursorScreenPos(), e.labobrDistance->getCaption().c_str());
        position(e.sliobrDistance);
        ImGui::SetNextItemWidth(315);
        float distance = e.sliobrDistance->getValue();
        if (ImGui::SliderFloat("##distance", &distance, 10.f, 700.f, "%.0f px")) {
            e.sliobrDistance->setValue(distance);
            e.mainListener->action(gcn::ActionEvent(e.sliobrDistance, ""));
        }
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) visible = false;
    }
    ImGui::End();
    if (!visible) stopRequested = true;
}
winObjectAreaOptions::winObjectAreaOptions() : cWindow(GETL2S("ObjProp", "SelVals")) {}
void winObjectAreaOptions::Open() {
    auto& e = *GV->editState;
    if (!e.hParser) return;
    document = e.hParser;
    open = positionOnOpen = true;
    closeRequested = false;
    pendingAction = nullptr;
    e.wintoolSelArea->setShow(false);
}
void winObjectAreaOptions::Close() {
    open = closeRequested = false;
    pendingAction = nullptr;
    document = nullptr;
    cWindow::Close();
}
void winObjectAreaOptions::Think() {
    if (!open) return;
    auto& e = *GV->editState;
    if (document != e.hParser || e.iActiveTool != EWW_TOOL_OBJSELAREA) { Close(); return; }
    if (!e._isOnTop()) return;
    if (closeRequested) {
        if (e.toolsaCameFromObjProps) e.mainListener->action(gcn::ActionEvent(e.wintoolSelArea, ""));
        else e.SetTool(EWW_TOOL_NONE);
        return;
    }
    if (pendingAction) {
        auto* action = pendingAction;
        pendingAction = nullptr;
        if (action == e.buttoolSelAreaOK && !e.toolsaCameFromObjProps) e.SetTool(EWW_TOOL_NONE);
        else e.mainListener->action(gcn::ActionEvent(action, ""));
    }
}
void winObjectAreaOptions::DrawImGui() {
    if (!open) return;
    auto& e = *GV->editState;
    if (document != e.hParser || !e.hParser) { Close(); return; }
    if (!e._isOnTop()) return;
    if (positionOnOpen) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos({viewport->Pos.x + e.vPort->GetX() + e.vPort->GetWidth() - 460.f,
            viewport->Pos.y + e.vPort->GetY() + e.vPort->GetHeight() - 200.f}, ImGuiCond_FirstUseEver);
        positionOnOpen = false;
    }
    ImGui::SetNextWindowSize({450, 190}, ImGuiCond_Always);
    const auto title = LegacyText::ToUtf8(GETL2S("ObjProp", "SelVals"), LegacyText::CodePage(GV->Lang->GetCode())) + "###WapMapObjectArea";
    bool visible = true;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, GETL2S("ObjProp", "SelVals"));
        char coordinates[192];
        std::snprintf(coordinates, sizeof(coordinates), "X1: %d   Y1: %d   X2: %d   Y2: %d", e.toolsaMinX, e.toolsaMinY, e.toolsaMaxX, e.toolsaMaxY);
        ImGui::SetCursorPos({8, 34});
        WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, ImGui::GetCursorScreenPos(), coordinates);
        SHR::But* buttons[] = {e.buttoolSelAreaPickMinX, e.buttoolSelAreaPickMaxX, e.buttoolSelAreaPickMinY,
            e.buttoolSelAreaPickMaxY, e.buttoolSelAreaAll, e.buttoolSelAreaOK};
        const int actions[] = {TOOL_OBJSA_PICKMINX, TOOL_OBJSA_PICKMAXX, TOOL_OBJSA_PICKMINY, TOOL_OBJSA_PICKMAXY, TOOL_OBJSA_PICKALL, TOOL_OBJSA_NONE};
        for (int i = 0; i < 6; ++i) {
            ImGui::SetCursorPos({8.f + (i % 2) * 224.f, 62.f + (i / 2) * 40.f});
            ImGui::PushID(i);
            const bool active = i < 5 && e.toolsaAction == actions[i];
            ImGui::BeginDisabled(active || pendingAction != nullptr || closeRequested);
            if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, buttons[i]->getCaption().c_str(), {210, 33})) pendingAction = buttons[i];
            ImGui::EndDisabled();
            if (active) ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(23, 150, 255, 255));
            ImGui::PopID();
        }
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            pendingAction = nullptr;
            if (e.toolsaAction != TOOL_OBJSA_NONE) {
                e.toolsaAction = TOOL_OBJSA_NONE;
                e.bDragSelection = false;
                e.UpdateSelectAreaWindowButtons();
            } else closeRequested = true;
        }
    }
    ImGui::End();
    if (!visible) { closeRequested = true; pendingAction = nullptr; }
}
#endif
