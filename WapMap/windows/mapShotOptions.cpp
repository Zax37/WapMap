#include "mapShotOptions.h"
#include "mapShotValidation.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../states/editing_ww.h"
#include "../cInterfaceSheet.h"
#include "../cColorPicker.h"
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../langID.h"
#include "../states/dialog.h"
#include <filesystem>
#include <cstdio>

winMapShotOptions::winMapShotOptions() : cWindow(GETL(Lang_MapShot)) {}
void winMapShotOptions::Open() {
    auto& e = *GV->editState;
    if (!e.hParser) return;
    document = e.hParser;
    open = positionOnOpen = true;
    browseRequested = saveRequested = false;
    const auto text = LegacyText::ToUtf8(e.tfmsSaveAs->getText(), CP_ACP);
    std::snprintf(path.data(), path.size(), "%s", text.c_str());
    e.winMapShot->setVisible(false);
}
void winMapShotOptions::Close() {
    open = browseRequested = saveRequested = false;
    document = nullptr;
    cWindow::Close();
}
void winMapShotOptions::Think() {
    if (!open || busy) return;
    auto& e = *GV->editState;
    if (document != e.hParser || !e.hParser) { Close(); return; }
    if (!e._isOnTop()) return;
    if (browseRequested) {
        browseRequested = false;
        auto initial = std::filesystem::u8path(path.data()).wstring();
        std::array<wchar_t, 4097> selected{};
        std::copy_n(initial.begin(), std::min(initial.size(), selected.size() - 1), selected.begin());
        OPENFILENAMEW dialog{};
        dialog.lStructSize = sizeof(dialog);
        dialog.hwndOwner = hge->System_GetState(HGE_HWND);
        dialog.lpstrFilter = L"PNG (*.png)\0*.png\0JPEG (*.jpg;*.jpeg)\0*.jpg;*.jpeg\0\0";
        dialog.lpstrFile = selected.data(); dialog.nMaxFile = static_cast<DWORD>(selected.size());
        dialog.lpstrDefExt = L"png";
        dialog.Flags = OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
        busy = true;
        const bool picked = GetSaveFileNameW(&dialog);
        busy = false;
        if (picked) {
            const auto utf8 = std::filesystem::path(selected.data()).u8string();
            std::snprintf(path.data(), path.size(), "%s", reinterpret_cast<const char*>(utf8.c_str()));
            if (const auto encoded = LegacyText::FromUtf8(path.data(), CP_ACP, 4096)) e.tfmsSaveAs->setText(*encoded, true);
        }
    }
    if (saveRequested) {
        saveRequested = false;
        const auto encoded = LegacyText::FromUtf8(path.data(), CP_ACP, 4096);
        const int plane = e.ddmsPlane->getSelected();
        if (!encoded || encoded->empty() || plane < 0 || plane >= e.hParser->GetPlanesCount() || !e.butmsSave->isLocallyEnabled()) return;
        auto* selectedPlane = e.hParser->GetPlane(plane);
        if (!MapShotValidation::CanSave(*encoded) || !MapShotValidation::Dimensions(selectedPlane->GetPlaneWidthPx(), selectedPlane->GetPlaneHeightPx(), e.slimsScale->getValue())) return;
        if (GetFileAttributesA(encoded->c_str()) != INVALID_FILE_ATTRIBUTES) {
            busy = true;
            const int result = State::MessageBox(GETL(Lang_MapShot), GETL2S("MapshotNative", "Overwrite"), ST_DIALOG_ICON_QUESTION, ST_DIALOG_BUT_YESNO);
            busy = false;
            if (result != RETURN_YES || document != e.hParser) return;
        }
        e.tfmsSaveAs->setText(*encoded);
        // Existing progress/export state remains the owner of rendering and IO.
        e.mainListener->action(gcn::ActionEvent(e.butmsSave, ""));
    }
}
void winMapShotOptions::DrawImGui() {
    if (!open || busy) return;
    auto& e = *GV->editState;
    if (document != e.hParser || !e.hParser) { Close(); return; }
    if (!e._isOnTop()) return;
    if (positionOnOpen) {
        auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, {0.5f, 0.5f});
        ImGui::SetNextWindowFocus(); positionOnOpen = false;
    }
    ImGui::SetNextWindowSize({520, 340}, ImGuiCond_Always);
    const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
    const auto title = LegacyText::ToUtf8(GETL(Lang_MapShot), cp) + "###WapMapMapshotOptions";
    bool visible = true;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, GETL(Lang_MapShot));
        const auto label = [&](const char* text, float x, float y) {
            ImGui::SetCursorPos({x, y});
            WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, ImGui::GetCursorScreenPos(), text);
        };
        label(GETL(Lang_SaveAs), 8, 38);
        ImGui::SetCursorPos({105, 34}); ImGui::SetNextItemWidth(355);
        const bool edited = ImGui::InputText("##path", path.data(), path.size());
        const auto encoded = LegacyText::FromUtf8(path.data(), CP_ACP, 4096);
        if (!encoded) ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(220, 75, 75, 255));
        if (edited && encoded) e.tfmsSaveAs->setText(*encoded, true);
        ImGui::SetCursorPos({470, 31});
        if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, "...", {34, 28})) browseRequested = true;
        label(GETL(Lang_Scale), 8, 76);
        ImGui::SetCursorPos({105, 70}); ImGui::SetNextItemWidth(400);
        float scale = e.slimsScale->getValue();
        if (ImGui::SliderFloat("##scale", &scale, 0.1f, 100.f, "%.1f%%")) {
            e.slimsScale->setValue(scale);
            e.mainListener->action(gcn::ActionEvent(e.slimsScale, ""));
        }
        label(GETL(Lang_Plane), 8, 110);
        ImGui::SetCursorPos({105, 104}); ImGui::SetNextItemWidth(240);
        auto* model = e.ddmsPlane->getListModel();
        const int index = e.ddmsPlane->getSelected();
        const auto preview = index >= 0 && index < model->getNumberOfElements() ? LegacyText::ToUtf8(model->getElementAt(index), cp) : std::string{};
        if (ImGui::BeginCombo("##plane", preview.c_str())) {
            for (int i = 0; i < model->getNumberOfElements(); ++i) {
                const auto text = LegacyText::ToUtf8(model->getElementAt(i), cp);
                if (ImGui::Selectable(text.c_str(), i == index)) {
                    e.ddmsPlane->setSelected(i);
                    e.mainListener->action(gcn::ActionEvent(e.ddmsPlane, ""));
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SetCursorPos({354, 108});
        ImGui::BeginDisabled(index < 0 || index >= e.hParser->GetPlanesCount() || e.hParser->GetPlane(index)->GetObjectsCount() == 0);
        bool drawObjects = e.cbmsDrawObjects->isSelected();
        if (WapMapImGui::BitmapCheckbox(*hge, *GV->fntMyriad16, *GV->hGfxInterface, e.cbmsDrawObjects->getCaption().c_str(), drawObjects)) e.cbmsDrawObjects->setSelected(drawObjects);
        ImGui::EndDisabled();
        label(e.labmsDimensions->getCaption().c_str(), 8, 140);
        label(GETL2("Mapshot", Lang_Ms_BackgroundColor), 8, 176);
        const DWORD background = e.cpmsPicker->GetColor();
        float rgb[] = {GETR(background) / 255.f, GETG(background) / 255.f, GETB(background) / 255.f};
        ImGui::SetCursorPos({8, 200}); ImGui::SetNextItemWidth(240);
        if (ImGui::ColorEdit3("##background", rgb)) e.cpmsPicker->SetColor(ARGB(255, static_cast<int>(rgb[0] * 255), static_cast<int>(rgb[1] * 255), static_cast<int>(rgb[2] * 255)));
        if (e.slimsCompression->isLocallyVisible()) {
            label(e.labmsCompression->getCaption().c_str(), 268, 176);
            ImGui::SetCursorPos({268, 200}); ImGui::SetNextItemWidth(236);
            int compression = static_cast<int>(e.slimsCompression->getValue());
            if (ImGui::SliderInt("##compression", &compression, static_cast<int>(e.slimsCompression->getScaleStart()), static_cast<int>(e.slimsCompression->getScaleEnd()))) {
                e.slimsCompression->setValue(compression);
                e.mainListener->action(gcn::ActionEvent(e.slimsCompression, ""));
            }
            ImGui::SetCursorPos({268, 232});
            WapMapImGui::WrappedBitmapMarkupText(*hge, *GV->fntMyriad16, e.labmsCompDisc->getCaption().c_str(), 236);
        }
        ImGui::SetCursorPos({8, 306});
        bool directory = e.cbmsOpenFileDirectory->isSelected();
        if (WapMapImGui::BitmapCheckbox(*hge, *GV->fntMyriad16, *GV->hGfxInterface, e.cbmsOpenFileDirectory->getCaption().c_str(), directory)) e.cbmsOpenFileDirectory->setSelected(directory);
        ImGui::SetCursorPos({404, 297});
        const bool validSize = index >= 0 && index < e.hParser->GetPlanesCount() && MapShotValidation::Dimensions(
            e.hParser->GetPlane(index)->GetPlaneWidthPx(), e.hParser->GetPlane(index)->GetPlaneHeightPx(), scale).has_value();
        ImGui::BeginDisabled(!encoded || !e.butmsSave->isLocallyEnabled() || !validSize);
        if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL(Lang_Save), {100, 33})) saveRequested = true;
        ImGui::EndDisabled();
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) visible = false;
    }
    ImGui::End();
    if (!visible) Close();
}
#endif
