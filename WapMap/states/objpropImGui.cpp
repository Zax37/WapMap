#include "objprop.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "editing_ww.h"
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../langID.h"
#include "../windows/coordinateNavigation.h"
#include "../windows/assetBrowserUi.h"
#include "../databanks/imageSets.h"
#include <cstdio>

void State::cObjectProp::ThinkImGui() {
    if (nativeBusy || !nativeAction) return;
    auto* source = nativeAction;
    nativeAction = nullptr;
    cRectPropGUI* rectangles[] = {guirectMinMax, guirectMove, guirectHit, guirectAttack, guirectClip, guirectUser[0], guirectUser[1]};
    for (auto* rectangle : rectangles) if (source == rectangle->butClear) {
        rectangle->action(gcn::ActionEvent(source, ""));
        for (auto* field : {rectangle->tfX1, rectangle->tfY1, rectangle->tfX2, rectangle->tfY2}) nativeFields.erase(field);
        return;
    }
    nativeBusy = true;
    hAL->action(gcn::ActionEvent(source, ""));
    nativeBusy = false;
}

void State::cObjectProp::DrawImGui(bool interactive) {
    if ((nativeBusy && interactive) || nativeSuspended || bKill) return;
    const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
    const auto title = LegacyText::ToUtf8(window->getCaption(), cp) + (interactive ? "###WapMapObjectProperties" : "###WapMapObjectPropertiesBackdrop");
    if (interactive && !ImGui::IsPopupOpen(title.c_str())) ImGui::OpenPopup(title.c_str());
    if (interactive && nativePosition) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, {0.5f, 0.5f});
        nativePosition = false;
    } else if (!interactive && nativeHasPosition) {
        ImGui::SetNextWindowPos({nativeWindowPosition[0], nativeWindowPosition[1]}, ImGuiCond_Always);
    }
    ImGui::SetNextWindowSize({static_cast<float>(window->getWidth()), static_cast<float>(window->getHeight())}, ImGuiCond_Always);
    ImGui::SetNextWindowScroll({0, 0});
    bool visible = true;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, IM_COL32(0, 0, 0, 90));
    const auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    const bool shown = interactive ? ImGui::BeginPopupModal(title.c_str(), &visible, flags) :
        ImGui::Begin(title.c_str(), nullptr, flags | ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::PopStyleColor(); ImGui::PopStyleVar();
    if (shown) WapMapImGui::KeepWindowOnScreen();
    if (!shown) {
        if (interactive && !visible) nativeAction = butCancel;
        if (!interactive) ImGui::End();
        return;
    }
    if (interactive) {
        const auto p = ImGui::GetWindowPos();
        nativeWindowPosition = {p.x, p.y}; nativeHasPosition = true;
    }
    ImGui::BeginDisabled(!interactive);
    WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, window->getCaption().c_str());
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {4.f, std::max(0.f, (20.f - GV->fntMyriad16->GetHeight()) / 2.f)});
    if (nativePicker) {
        ImGui::SetCursorPos({10, 34});
        ImGui::PushStyleColor(ImGuiCol_ChildBg, WapMapImGui::Grey(0x13));
        if (ImGui::BeginChild("imageSets", {280, 495}, ImGuiChildFlags_NavFlattened)) {
            auto* bank = hState->SprBank;
            for (int i = 0; bank && i < bank->getNumberOfElements(); ++i) {
                auto* set = bank->GetAssetByIterator(i);
                ImGui::PushID(i);
                const auto text = LegacyText::ToUtf8(set->GetID(), cp);
                if (ImGui::Selectable(text.c_str(), nativeSet == set->GetID())) { nativeSet = set->GetID(); nativeFrame = -1; }
                if (nativeSet == set->GetID()) ImGui::SetItemDefaultFocus();
                ImGui::PopID();
            }
        }
        ImGui::EndChild(); ImGui::PopStyleColor();
        auto* selected = hState->SprBank ? hState->SprBank->GetAssetByID(nativeSet.c_str()) : nullptr;
        ImGui::SetCursorPos({306, 34});
        bool automatic = nativeFrame == -1;
        ImGui::BeginDisabled(!selected || selected->GetSpritesCount() == 0);
        if (WapMapImGui::BitmapCheckbox(*hge, *GV->fntMyriad16, *GV->hGfxInterface, "I: -1", automatic)) {
            nativeFrame = automatic || !selected || selected->GetSpritesCount() == 0 ? -1 : selected->GetIMGByIterator(0)->GetID();
        }
        ImGui::EndDisabled();
        ImGui::SetCursorPos({306, 62});
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});
        if (ImGui::BeginChild("frames", {680, 467}, ImGuiChildFlags_NavFlattened)) {
            const int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / 112));
            const auto start = ImGui::GetCursorPos();
            ImGuiListClipper clipper;
            const int count = selected ? selected->GetSpritesCount() : 0;
            clipper.Begin((count + columns - 1) / columns, 132);
            while (clipper.Step()) for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                for (int column = 0; column < columns; ++column) {
                    const int i = row * columns + column;
                    if (i >= count) break;
                    auto* image = selected->GetIMGByIterator(i);
                    ImGui::PushID(i);
                    ImGui::SetCursorPos({start.x + column * 112.f, start.y + row * 132.f});
                    const auto p = ImGui::GetCursorScreenPos();
                    if (ImGui::InvisibleButton("##frame", {108, 128}, ImGuiButtonFlags_EnableNav)) nativeFrame = image->GetID();
                    AssetBrowserUi::Preview(*hge, image->GetSprite(), {p.x + 4, p.y + 4}, 96, false, true, nativeFrame == image->GetID(), ImGui::IsItemHovered() || WapMapImGui::IsKeyboardFocused());
                    const auto caption = std::to_string(image->GetID());
                    WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, {p.x + 4, p.y + 108}, caption.c_str());
                    ImGui::PopID();
                }
                ImGui::SetCursorPos({start.x, start.y + (row + 1) * 132.f});
                ImGui::Dummy({1, 0});
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::SetCursorPos({770, 543});
        ImGui::BeginDisabled(!selected);
        if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL(Lang_OK), {100, 33})) {
            tddadvImageSet->setText(nativeSet);
            tfadvLocI->setText(std::to_string(nativeFrame));
            nativeFields.erase(tddadvImageSet); nativeFields.erase(tfadvLocI);
            nativePicker = false;
            nativeFocusTarget = tddadvImageSet;
        }
        ImGui::EndDisabled();
        ImGui::SetCursorPos({882, 543});
        if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL(Lang_Cancel), {100, 33})) { nativePicker = false; nativeFocusTarget = tddadvImageSet; }
    } else {
        const auto area = window->getChildrenArea();
        const auto origin = ImGui::GetWindowPos();
        auto* draw = ImGui::GetWindowDrawList();
        const auto line = [&](float x1, float y1, float x2, float y2) {
            draw->AddLine({origin.x + x1, origin.y + area.y + y1}, {origin.x + x2, origin.y + area.y + y2}, IM_COL32(31, 31, 31, 255));
        };
        line(0, 194, 543, 194); line(363, 0, 363, 194);
        line(543, 0, 543, 566); line(715, 0, 715, 566);
        for (int i = 0; i < 7; ++i) line(715, 78.f + i * 74, 997, 78.f + i * 74);
        bool valid = true;
        for (auto* widget : window->getChildren()) {
            if (!widget->isLocallyVisible() || widget == vpAdv) continue;
            ImGui::PushID(widget);
            ImGui::SetCursorPos({static_cast<float>(area.x + widget->getX()), static_cast<float>(area.y + widget->getY())});
            ImGui::BeginDisabled(!widget->isLocallyEnabled() || nativeAction != nullptr);
            if (auto* label = dynamic_cast<SHR::Lab*>(widget)) {
                WapMapImGui::WrappedBitmapMarkupText(*hge, *GV->fntMyriad16, label->getCaption().c_str(), static_cast<float>(std::max(label->getWidth(), 1)));
            } else if (auto* checkbox = dynamic_cast<SHR::CBox*>(widget)) {
                bool selected = checkbox->isSelected();
                if (WapMapImGui::BitmapCheckbox(*hge, *GV->fntMyriad16, *GV->hGfxInterface, checkbox->getCaption().c_str(), selected)) checkbox->setSelected(selected);
            } else if (auto* radio = dynamic_cast<SHR::RadBut*>(widget)) {
                int selected = radio->isSelected() ? 1 : 0;
                if (WapMapImGui::BitmapRadio(*hge, *GV->fntMyriad16, *GV->hGfxInterface, radio->getCaption().c_str(), selected, 1)) {
                    radio->setSelected(true); nativeAction = radio;
                }
            } else if (auto* dropdown = dynamic_cast<SHR::DropDown*>(widget)) {
                auto* model = dropdown->getListModel();
                const int index = dropdown->getSelected();
                const auto text = model && index >= 0 && index < model->getNumberOfElements() ? LegacyText::ToUtf8(model->getElementAt(index), cp) : std::string{};
                ImGui::SetNextItemWidth(static_cast<float>(widget->getWidth()));
                if (ImGui::BeginCombo("##value", text.c_str())) {
                    for (int i = 0; model && i < model->getNumberOfElements(); ++i) {
                        ImGui::PushID(i);
                        const auto item = LegacyText::ToUtf8(model->getElementAt(i), cp);
                        if (ImGui::Selectable(item.c_str(), i == index)) { dropdown->setSelected(i); nativeAction = dropdown; }
                        ImGui::PopID();
                    }
                    ImGui::EndCombo();
                }
            } else if (auto* button = dynamic_cast<SHR::But*>(widget)) {
                ImGui::BeginDisabled(button == butSave && !valid);
                if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, button->getCaption().c_str(), {static_cast<float>(widget->getWidth()), static_cast<float>(widget->getHeight())})) {
                    if (button == butSelListImageSet) {
                        nativeSet = tddadvImageSet->getText();
                        nativeFrame = CoordinateNavigation::Parse(tfadvLocI->getText()).value_or(-1);
                        if (hState->SprBank && !hState->SprBank->GetAssetByID(nativeSet.c_str()) && hState->SprBank->getNumberOfElements() > 0) {
                            nativeSet = hState->SprBank->GetAssetByIterator(0)->GetID();
                            nativeFrame = -1;
                        }
                        nativePicker = true;
                    } else {
                        nativeAction = button;
                        for (auto* rectangle : {guirectMinMax, guirectMove, guirectHit, guirectAttack, guirectClip, guirectUser[0], guirectUser[1]})
                            if (button == rectangle->butPick) ImGui::CloseCurrentPopup();
                    }
                }
                if (const auto* icon = button->getIcon()) {
                    const auto p = ImGui::GetItemRectMin();
                    WapMapImGui::Sprite(ImGui::GetWindowDrawList(), *hge, icon, {p.x + 4, p.y + (widget->getHeight() - icon->GetHeight()) / 2});
                }
                ImGui::EndDisabled();
            } else {
                auto* field = dynamic_cast<SHR::TextField*>(widget);
                auto* combo = dynamic_cast<SHR::TextDropDown*>(widget);
                if (field || combo) {
                    const auto& text = field ? field->getText() : combo->getText();
                    auto [entry, inserted] = nativeFields.try_emplace(widget);
                    auto& draft = entry->second;
                    if (inserted || draft.legacy != text) {
                        const auto utf8 = LegacyText::ToUtf8(text, cp);
                        std::snprintf(draft.text.data(), draft.text.size(), "%s", utf8.c_str());
                        draft.legacy = text;
                    }
                    ImGui::SetNextItemWidth(static_cast<float>(widget->getWidth()) - (combo ? 24.f : 0.f));
                    if (interactive && nativeFocusTarget == widget) { ImGui::SetKeyboardFocusHere(); nativeFocusTarget = nullptr; }
                    const bool edited = ImGui::InputText("##text", draft.text.data(), draft.text.size(), field && field->IsNumericalOnly() ? ImGuiInputTextFlags_CharsDecimal : ImGuiInputTextFlags_None);
                    const auto encoded = LegacyText::FromUtf8(draft.text.data(), cp, std::max(size_t(128), text.size()));
                    const bool accepted = encoded && (!field || !field->IsNumericalOnly() || CoordinateNavigation::Parse(*encoded).has_value());
                    if (!accepted && widget->isLocallyEnabled()) {
                        valid = false;
                        ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(220, 75, 75, 255));
                    }
                    if (edited && accepted) {
                        if (field) field->setText(*encoded); else combo->setText(*encoded);
                        draft.legacy = *encoded;
                    }
                    if (combo) {
                        ImGui::SameLine(0, 0);
                        if (ImGui::ArrowButton("##choices", ImGuiDir_Down)) ImGui::OpenPopup("choices");
                        if (ImGui::BeginPopup("choices")) {
                            auto* model = combo->getListModel();
                            for (int i = 0; model && i < model->getNumberOfElements(); ++i) {
                                ImGui::PushID(i);
                                const auto item = model->getElementAt(i);
                                const auto caption = LegacyText::ToUtf8(item, cp);
                                if (ImGui::Selectable(caption.c_str(), item == combo->getText())) { combo->setText(item); nativeFields.erase(widget); nativeAction = combo; }
                                ImGui::PopID();
                            }
                            ImGui::EndPopup();
                        }
                    }
                }
            }
            ImGui::EndDisabled(); ImGui::PopID();
        }
    }
    if (interactive && (!visible || (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)))) {
        if (nativePicker && visible) { nativePicker = false; nativeFocusTarget = tddadvImageSet; }
        else { nativeAction = butCancel; ImGui::CloseCurrentPopup(); }
    }
    ImGui::PopStyleVar();
    ImGui::EndDisabled();
    if (interactive) ImGui::EndPopup(); else ImGui::End();
}
#endif
