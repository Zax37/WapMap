#include "objectSearchWindow.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "objectSearch.h"
#include "../states/editing_ww.h"
#include "../databanks/imageSets.h"
#include "../cObjectUserData.h"
#include "../cInterfaceSheet.h"
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../langID.h"
#include <unordered_set>

winObjectSearch::winObjectSearch() : cWindow(GETL(Lang_SearchObject)) {}
void winObjectSearch::Open() {
    if (!GV->editState->hParser) return;
    document = GV->editState->hParser;
    open = positionOnOpen = true;
    pendingSelection.clear();
    GV->editState->winSearchObj->setVisible(false);
}
void winObjectSearch::Close() {
    open = false;
    pendingSelection.clear();
    pendingPlane = nullptr;
    document = nullptr;
    cWindow::Close();
}
void winObjectSearch::Think() {
    if (!open) return;
    auto& e = *GV->editState;
    if (document != e.hParser || !e.hParser || e.GetActiveMode() != EWW_MODE_OBJECT) { Close(); return; }
    if (!e._isOnTop() || pendingSelection.empty()) return;
    auto* plane = e.GetActivePlane();
    if (plane != pendingPlane) { pendingSelection.clear(); return; }
    std::unordered_set<WWD::Object*> live;
    for (int i = 0; i < plane->GetObjectsCount(); ++i) live.insert(plane->GetObjectByIterator(i));
    e.vObjectsPicked.clear();
    for (auto* object : pendingSelection) if (live.contains(object)) e.vObjectsPicked.push_back(object);
    pendingSelection.clear();
    if (navigate && !e.vObjectsPicked.empty()) {
        auto* object = e.vObjectsPicked.front();
        e.NavigateToPoint(object->GetX(), object->GetY());
    }
    e.vPort->MarkToRedraw();
}
void winObjectSearch::DrawImGui() {
    if (!open) return;
    auto& e = *GV->editState;
    if (document != e.hParser || !e.hParser || e.GetActiveMode() != EWW_MODE_OBJECT) { Close(); return; }
    if (!e._isOnTop()) return;
    if (positionOnOpen) {
        auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, {0.5f, 0.5f});
        ImGui::SetNextWindowFocus();
        positionOnOpen = false;
    }
    const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
    const auto title = LegacyText::ToUtf8(GETL(Lang_SearchObject), cp) + "###WapMapObjectSearch";
    ImGui::SetNextWindowSize({490, 550}, ImGuiCond_Always);
    bool visible = true;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, GETL(Lang_SearchObject));
        ImGui::SetCursorPos({8, 34});
        WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, ImGui::GetCursorScreenPos(), e.labobjseaInfo->getCaption().c_str());
        ImGui::SetCursorPos({static_cast<float>(e.labobjseaInfo->getWidth() + 16), 32});
        ImGui::SetNextItemWidth(470.f - e.labobjseaInfo->getWidth() - 16);
        auto* model = e.ddObjSearchTerm->getListModel();
        term = std::clamp(term, 0, model->getNumberOfElements() - 1);
        const auto preview = LegacyText::ToUtf8(model->getElementAt(term), cp);
        if (ImGui::BeginCombo("##term", preview.c_str())) {
            for (int i = 0; i < model->getNumberOfElements(); ++i) {
                const auto label = LegacyText::ToUtf8(model->getElementAt(i), cp);
                if (ImGui::Selectable(label.c_str(), term == i)) term = i;
            }
            ImGui::EndCombo();
        }
        ImGui::SetCursorPos({8, 60}); ImGui::SetNextItemWidth(474);
        ImGui::InputText("##query", query.data(), query.size());
        const auto encoded = LegacyText::FromUtf8(query.data(), cp, 1024);
        if (!encoded || (term == 4 && !encoded->empty() && !CoordinateNavigation::Parse(*encoded)))
            ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(220, 75, 75, 255));
        ImGui::SetCursorPos({8, 90});
        ImGui::BeginDisabled(term == 4);
        WapMapImGui::BitmapCheckbox(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL2S("ObjectSearch", "CaseSensitive"), caseSensitive);
        ImGui::EndDisabled();
        auto* plane = e.GetActivePlane();
        std::vector<WWD::Object*> results;
        if (encoded && !encoded->empty() && plane) for (int i = 0; i < plane->GetObjectsCount(); ++i) {
            auto* object = plane->GetObjectByIterator(i);
            const char* text = term == 1 ? object->GetLogic() : term == 2 ? object->GetImageSet() : term == 3 ? object->GetAnim() : object->GetName();
            if (term == 4 ? ObjectSearch::MatchId(object->GetParam(WWD::Param_ID), *encoded) : ObjectSearch::Match(text, *encoded, caseSensitive)) results.push_back(object);
        }
        ImGui::SetCursorPos({8, 120});
        const auto count = results.empty() ? std::string(GETL2S("ObjectSearch", "NoResults")) :
            std::string(GETL2S("ObjectSearch", "FoundResults")) + ": " + std::to_string(results.size());
        WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, ImGui::GetCursorScreenPos(), count.c_str());
        ImGui::SetCursorPos({300, 90});
        ImGui::BeginDisabled(results.empty());
        if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL2S("ObjectSearch", "SelectAll"), {182, 26})) {
            pendingPlane = plane; pendingSelection = results; navigate = false;
        }
        ImGui::EndDisabled();
        ImGui::SetCursorPos({8, 148});
        ImGui::PushStyleColor(ImGuiCol_ChildBg, WapMapImGui::Grey(0x13));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {6, 0});
        if (ImGui::BeginChild("results", {474, 394}, ImGuiChildFlags_NavFlattened)) {
            ImGuiListClipper clipper;
            clipper.Begin(static_cast<int>(results.size()), 174);
            while (clipper.Step()) for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                auto* object = results[i];
                ImGui::PushID(i);
                const auto row = ImGui::GetCursorPos();
                const auto p = ImGui::GetCursorScreenPos();
                auto* draw = ImGui::GetWindowDrawList();
                if (GV->sprCheckboard) WapMapImGui::StretchSprite(draw, *hge, GV->sprCheckboard, p, {p.x + 130, p.y + 130}, IM_COL32_WHITE);
                auto* sprite = e.SprBank ? e.SprBank->GetObjectSprite(object) : nullptr;
                if (sprite && sprite->GetWidth() > 0 && sprite->GetHeight() > 0) {
                    float x, y, w, h;
                    sprite->GetTextureRect(&x, &y, &w, &h);
                    const auto texture = sprite->GetTexture();
                    const float tw = hge->Texture_GetWidth(texture), th = hge->Texture_GetHeight(texture);
                    if (tw > 0 && th > 0) {
                        ImVec2 uv0{x / tw, y / th}, uv1{(x + w) / tw, (y + h) / th};
                        if (GetUserDataFromObj(object)->GetFlipX()) std::swap(uv0.x, uv1.x);
                        if (GetUserDataFromObj(object)->GetFlipY()) std::swap(uv0.y, uv1.y);
                        const float scale = std::min(1.f, 130.f / std::max(w, h));
                        const ImVec2 start{p.x + (130 - w * scale) / 2, p.y + (130 - h * scale) / 2};
                        draw->AddImage(static_cast<ImTextureID>(hge->Texture_GetNativeHandle(texture)), start,
                            {start.x + w * scale, start.y + h * scale}, uv0, uv1);
                    }
                }
                const auto label = [&](const wchar_t* key, const std::string& value, int line) {
                    const auto text = std::string(GETL2SV("ObjectSearch", key)) + ": " + value;
                    WapMapImGui::BitmapText(draw, *hge, *GV->fntMyriad16, {p.x + 138, p.y + line * 20.f}, text.c_str());
                };
                label(L"ID", std::to_string(object->GetParam(WWD::Param_ID)) + " (#" + std::to_string(i + 1) + ")", 0);
                label(L"Name", object->GetName(), 1); label(L"Logic", object->GetLogic(), 2);
                label(L"ImageSet", object->GetImageSet(), 3); label(L"Anim", object->GetAnim(), 4);
                label(L"Pos", std::to_string(object->GetX()) + " x " + std::to_string(object->GetY()), 5);
                ImGui::SetCursorPos({row.x + 138, row.y + 134});
                if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL2S("ObjectSearch", "GoToObject"), {190, 30})) {
                    pendingPlane = plane; pendingSelection = {object}; navigate = true;
                }
                ImGui::SetCursorPos({row.x, row.y + 172});
                ImGui::Dummy({1, 2});
                ImGui::PopID();
            }
        }
        ImGui::EndChild(); ImGui::PopStyleVar(); ImGui::PopStyleColor();
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) visible = false;
    }
    ImGui::End();
    if (!visible) Close();
}
#endif
