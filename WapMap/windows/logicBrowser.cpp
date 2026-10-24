#include "logicBrowser.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../states/editing_ww.h"
#include "../databanks/logics.h"
#include "../cInterfaceSheet.h"
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../../shared/commonFunc.h"
#include "../langID.h"

winLogicBrowser::winLogicBrowser() : cWindow(GETL2S("Win_LogicBrowser", "WinCaption")) {}
void winLogicBrowser::Open() {
    auto& e = *GV->editState;
    if (!e.hParser || !e.hCustomLogics) return;
    document = e.hParser;
    open = positionOnOpen = true;
    pendingAction = nullptr;
    selectedName.clear();
    const int index = e.lbbrlLogicList->getSelected();
    if (index >= 0 && index < e.hCustomLogics->getNumberOfElements()) selectedName = e.hCustomLogics->GetAssetByIterator(index)->GetName();
    e.winLogicBrowser->setVisible(false);
}
void winLogicBrowser::Close() {
    open = false;
    document = nullptr;
    pendingAction = nullptr;
    selectedName.clear();
    actionName.clear();
    cWindow::Close();
}
void winLogicBrowser::Think() {
    if (!open || busy) return;
    auto& e = *GV->editState;
    if (document != e.hParser || !e.hCustomLogics) { Close(); return; }
    if (!pendingAction || !e._isOnTop()) return;
    auto* action = pendingAction;
    pendingAction = nullptr;
    if (action != e.butbrlNew) {
        int index = -1;
        for (int i = 0; i < e.hCustomLogics->getNumberOfElements(); ++i)
            if (e.hCustomLogics->GetAssetByIterator(i)->GetName() == actionName) { index = i; break; }
        if (index < 0) return;
        e.lbbrlLogicList->setSelected(index);
    }
    // Existing handlers may enter synchronous name/confirmation dialogs.
    // Run before NewFrame and suppress this browser during re-entry.
    busy = true;
    e.mainListener->action(gcn::ActionEvent(action, ""));
    busy = false;
    if (document != e.hParser || !e.hCustomLogics) { Close(); return; }
    const int index = e.lbbrlLogicList->getSelected();
    if (index >= 0 && index < e.hCustomLogics->getNumberOfElements()) selectedName = e.hCustomLogics->GetAssetByIterator(index)->GetName();
}
void winLogicBrowser::DrawImGui() {
    if (!open || busy) return;
    auto& e = *GV->editState;
    if (document != e.hParser || !e.hCustomLogics) { Close(); return; }
    if (!e._isOnTop()) return;
    if (positionOnOpen) {
        auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, {0.5f, 0.5f});
        ImGui::SetNextWindowFocus();
        positionOnOpen = false;
    }
    bool hasSelection = false;
    for (int i = 0; i < e.hCustomLogics->getNumberOfElements(); ++i) {
        if (e.hCustomLogics->GetAssetByIterator(i)->GetName() == selectedName) {
            hasSelection = true;
            break;
        }
    }
    const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
    const auto title = LegacyText::ToUtf8(GETL2S("Win_LogicBrowser", "WinCaption"), cp) + "###WapMapLogicBrowser";
    ImGui::SetNextWindowSize({hasSelection ? 800.0f : 300.0f, 400}, ImGuiCond_Always);
    bool visible = true;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, GETL2S("Win_LogicBrowser", "WinCaption"));
        ImGui::SetCursorPos({8, 32});
        WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, ImGui::GetCursorScreenPos(), GETL2S("Win_LogicBrowser", "SelectCustomLogic"));
        cCustomLogic* selected = nullptr;
        ImGui::SetCursorPos({8, 58});
        ImGui::PushStyleColor(ImGuiCol_ChildBg, WapMapImGui::Grey(0x13));
        if (ImGui::BeginChild("logics", {280, 296}, ImGuiChildFlags_NavFlattened)) {
            for (int i = 0; i < e.hCustomLogics->getNumberOfElements(); ++i) {
                auto* logic = e.hCustomLogics->GetAssetByIterator(i);
                ImGui::PushID(i);
                const auto name = LegacyText::ToUtf8(logic->GetName(), cp);
                if (ImGui::Selectable(name.c_str(), selectedName == logic->GetName())) {
                    selectedName = logic->GetName();
                    e.lbbrlLogicList->setSelected(i);
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", name.c_str());
                ImGui::PopID();
            }
        }
        ImGui::EndChild(); ImGui::PopStyleColor();
        for (int i = 0; i < e.hCustomLogics->getNumberOfElements(); ++i) {
            auto* logic = e.hCustomLogics->GetAssetByIterator(i);
            if (logic->GetName() == selectedName) { selected = logic; break; }
        }
        if ((selected != nullptr) != hasSelection)
            ImGui::SetWindowSize({selected ? 800.0f : 300.0f, 400}, ImGuiCond_Always);
        const auto button = [&](SHR::But* widget, ImVec2 position, ImVec2 size, bool enabled) {
            ImGui::SetCursorPos(position);
            ImGui::PushID(widget);
            ImGui::BeginDisabled(!enabled || pendingAction != nullptr);
            if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, widget->getCaption().c_str(), size)) {
                pendingAction = widget;
                actionName = selectedName;
            }
            if (const auto* icon = widget->getIcon()) {
                const auto p = ImGui::GetItemRectMin();
                WapMapImGui::Sprite(ImGui::GetWindowDrawList(), *hge, icon,
                    {p.x + 6, p.y + (size.y - icon->GetHeight()) / 2},
                    IM_COL32(255, 255, 255, static_cast<int>(255 * ImGui::GetStyle().Alpha)));
            }
            ImGui::EndDisabled(); ImGui::PopID();
        };
        button(e.butbrlNew, {8, 362}, {280, 30}, true);
        if (selected) {
            const auto label = [&](const wchar_t* key, const std::string& value, float y) {
                const auto text = std::string(GETL2SV("Win_LogicBrowser", key)) + " " + value;
                ImGui::SetCursorPos({306, y});
                WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, ImGui::GetCursorScreenPos(), text.c_str());
                ImGui::InvisibleButton("##metadata", {484, 20});
                if (ImGui::IsItemHovered()) {
                    const auto utf8 = LegacyText::ToUtf8(text, cp);
                    ImGui::SetTooltip("%s", utf8.c_str());
                }
            };
            label(L"LogicName", selected->GetName(), 36);
            ImGui::PushID(1); label(L"FilePath", selected->GetPath(), 62); ImGui::PopID();
            const auto& file = selected->GetFile();
            char* formatted = SHR::FormatSize(file.hFeed ? file.hFeed->GetFileSize(file.strPath.c_str()) : selected->GetFileSize());
            ImGui::PushID(2); label(L"FileSize", formatted, 88); ImGui::PopID();
            delete[] formatted;
            ImGui::PushID(3); label(L"FileChecksum", selected->GetHash(), 114); ImGui::PopID();
            if (file.hFeed) {
                formatted = SHR::FormatTimeFromUnix(file.hFeed->GetFileModTime(file.strPath.c_str()));
                ImGui::PushID(4); label(L"ModDate", formatted, 140); ImGui::PopID();
                delete[] formatted;
            }
        }
        if (selected) {
            button(e.butbrlEdit, {306, 202}, {235, 30}, true);
            button(e.butbrlRename, {553, 202}, {235, 30}, true);
            button(e.butbrlBrowseDir, {306, 242}, {235, 30}, true);
            button(e.butbrlDelete, {553, 242}, {235, 30}, true);
        }
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) visible = false;
    }
    ImGui::End();
    if (!visible) Close();
}
#endif
