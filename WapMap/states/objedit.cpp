#include "objedit.h"
#include "editing_ww.h"
#include "../cObjectUserData.h"
#include "../langID.h"
#include "../databanks/imageSets.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../windows/assetBrowserUi.h"
#include "../windows/coordinateNavigation.h"
#include "../windows/objectToolGeometry.h"
#include "../objEdit/procPickRect.h"
#include "../objEdit/procPickXYLockable.h"
#include "../../shared/gcnWidgets/wComboButton.h"
#include "../../shared/gcnWidgets/wListbox.h"
#include "../objEdit/nativeInventoryUi.h"
#include "../../shared/gcnWidgets/wTextDropDown.h"
#include "../../shared/gcnWidgets/wInventory.h"
#include <imgui_internal.h>
#include <cstdio>
#endif

extern HGE *hge;

namespace ObjEdit {
    void cObjEditAL::action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == m_hOwn->_butAddNext) {
            if (!m_hOwn->_butAddNext->isLocallyEnabled() || !m_hOwn->_butSave->isLocallyEnabled()) return;
            m_hOwn->Save();
            if (!m_hOwn->_butSave->isLocallyEnabled()) return;
            m_hOwn->SaveChanges();
            m_hOwn->bKill = 1;
            m_hOwn->_bAddNext = 1;
            return;
        } else if (actionEvent.getSource() == m_hOwn->_butSave) {
            if (!m_hOwn->_butSave->isLocallyEnabled()) return;
            m_hOwn->Save();
            if (!m_hOwn->_butSave->isLocallyEnabled()) return;
            m_hOwn->SaveChanges();
            m_hOwn->bKill = 1;
            m_hOwn->hState->bEditObjDelete = 0;
            return;
        }
        m_hOwn->Action(actionEvent);
    }

    cObjEdit::cObjEdit(WWD::Object *obj, State::EditingWW *st) {
        dwHighlightColor = 0xFFFF0000;
        bAllowDragging = 1;
        _bDragging = 0;
        _bAddNext = 0;
        bObjectSaved = 0;
        bChangesMade = 0;
        hOrigObj = obj;
        hState = st;
        hAL = new cObjEditAL(this);
		vpCB = new cObjEditVP(this);
		vpAdv = new WIDG::Viewport(vpCB, 0);
        bKill = 0;
        hTempObj = new WWD::Object(hOrigObj);
        hState->GetActivePlane()->AddObjectAndCalcID(hTempObj);
        hTempObj->SetUserData(new cObjUserData(hTempObj));
        hState->hPlaneData[hState->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(hTempObj);
        hState->vObjectsPicked[0] = hTempObj;

        _butAddNext = new SHR::But(GV->hGfxInterface, GETL2S("WinAddObjBatch", "AddNext"));
        _butAddNext->setDimension(gcn::Rectangle(0, 0, 100, 33));
        _butAddNext->addActionListener(hAL);

        _butSave = new SHR::But(GV->hGfxInterface, GETL(Lang_Save));
        _butSave->setDimension(gcn::Rectangle(0, 0, 100, 33));
        _butSave->addActionListener(hAL);

        _iMoveInitX = -1;
        _iMoveInitY = -1;
    }

    cObjEdit::~cObjEdit() {
        if (hState->bEditObjDelete) {
            if (!bObjectSaved) {
                hState->GetActivePlane()->DeleteObject(hOrigObj);
                hOrigObj = 0;
                hState->vObjectsPicked.clear();
            } else {
                hState->MarkUnsaved();
            }
        }
        hState->GetActivePlane()->DeleteObject(hTempObj);
        delete _butAddNext;
        delete _butSave;
        delete vpAdv;
        delete vpCB;
        delete hAL;
        if (hOrigObj != 0) {
            hState->vObjectsPicked.clear();
            hState->vObjectsPicked.push_back(hOrigObj);
        }
    }

    void cObjEdit::Think(bool bMouseConsumed) {
#ifdef WAPMAP_ENABLE_IMGUI
        if (nativeBusy) return;
        if (nativeWindow && nativeAction) {
            auto* source = nativeAction;
            nativeAction = nullptr;
            nativeBusy = true;
            if (source == nativeInventoryTarget) {
                auto* target = nativeInventoryTarget;
                nativeInventoryTarget = nullptr;
                const auto& children = win->getChildren();
                if (std::find(children.begin(), children.end(), target) == children.end() || !target->isLocallyVisible() || !target->isLocallyEnabled()) {
                    nativeBusy = false; return;
                }
                target->SetItem(nativeInventoryItem < 0 ? cInventoryItem("", -1) : hState->hInvCtrl->GetItemByID(nativeInventoryItem));
            }
            if (source == vpAdv) HandleEscape();
            else if (auto* pair = dynamic_cast<cProcPickXYLockable*>(source->getParent())) {
                pair->action(gcn::ActionEvent(source, ""));
            }
            else if (auto* rectangle = dynamic_cast<cProcPickRect*>(source->getParent())) {
                if (source == rectangle->getPickButton())
                    for (int i = 0; i < 4; ++i) nativeIntegers.erase(rectangle->getTextField(i));
                rectangle->action(gcn::ActionEvent(source, ""));
            }
            else hAL->action(gcn::ActionEvent(source, ""));
            nativeBusy = false;
            if (bKill) return;
        }
#endif
        _Think(bMouseConsumed);
        _butAddNext->setEnabled(_butSave->isLocallyEnabled());
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        bool bMouseIn = 1;
        if (mx < hState->vPort->GetX() || my < hState->vPort->GetY() ||
            mx > hState->vPort->GetX() + hState->vPort->GetWidth() ||
            my > hState->vPort->GetY() + hState->vPort->GetHeight())
            bMouseIn = 0;

        if (!bMouseConsumed && bMouseIn) {
            int wmx = GV->editState->Scr2WrdX(GV->editState->GetActivePlane(), mx),
                wmy = GV->editState->Scr2WrdY(GV->editState->GetActivePlane(), my);
            if (_bDragging) {
                int destx = wmx - _iDragOffX,
                    desty = wmy - _iDragOffY;
                if (hge->Input_GetKeyState(HGEK_CTRL)) {
                    int diffx = wmx - _iMoveInitX,
                        diffy = wmy - _iMoveInitY;
                    int diffmodulox = wmx % 64;
                    int diffmoduloy = wmy % 64;
                    int modx = 0, mody = 0;

                    if (diffmodulox > 16 && diffmodulox < 48) modx += 32;
                    else if (diffmodulox >= 48) modx += 64;

                    if (diffmoduloy > 16 && diffmoduloy < 48) mody += 32;
                    else if (diffmoduloy >= 48) mody += 64;

                    destx = _iMoveInitX + diffx - (diffmodulox - modx);
                    desty = _iMoveInitY + diffy - (diffmoduloy - mody);
                } else if (hge->Input_GetKeyState(HGEK_SHIFT)) {
                    int diffx = wmx - _iMoveInitX,
                            diffy = wmy - _iMoveInitY;
                    float ratio = (diffy == 0 ? 2 : float(diffx) / float(diffy));
                    if (ratio >= -0.50f && ratio <= 0.50)
                        destx = _iMoveInitX;
                    else if (ratio > 1.5f || ratio < -1.5f) {
                        desty = _iMoveInitY;
                    } else {
                        int diff = std::min(abs(diffx), abs(diffy));
                        if (diffy < 0) {
                            if (ratio < -0.5f && ratio > -1.5f) { //upright
                                destx = _iMoveInitX + diff;
                                desty = _iMoveInitY - diff;
                            } else if (ratio > 0.5f && ratio < 1.5f) { //upleft
                                destx = _iMoveInitX - diff;
                                desty = _iMoveInitY - diff;
                            }
                        } else {
                            if (ratio < -0.5f && ratio > -1.5f) { //downleft
                                destx = _iMoveInitX - diff;
                                desty = _iMoveInitY + diff;
                            } else if (ratio > 0.5f && ratio < 1.5f) { //downright
                                destx = _iMoveInitX + diff;
                                desty = _iMoveInitY + diff;
                            }
                        }
                    }
                }
                if (hTempObj->GetParam(WWD::Param_LocationX) != destx ||
                    hTempObj->GetParam(WWD::Param_LocationY) != desty) {
                    int sourceX = hTempObj->GetParam(WWD::Param_LocationX),
                        sourceY = hTempObj->GetParam(WWD::Param_LocationY);
                    GV->editState->vPort->MarkToRedraw();
                    hTempObj->SetParam(WWD::Param_LocationX, destx);
                    hTempObj->SetParam(WWD::Param_LocationY, desty);
                    GetUserDataFromObj(hTempObj)->SyncToObj();
                    ObjectMovedInsideEasyEdit(sourceX, sourceY);
                }
            }
            if (_bDragging && !hge->Input_GetKeyState(HGEK_LBUTTON)) {
                _bDragging = 0;
                _iMoveInitX = -1;
                _iMoveInitY = -1;
            }
            WWD::Rect r = GV->editState->SprBank->GetObjectRenderRect(hTempObj);
            r.x2 += r.x1;
            r.y2 += r.y1;
            if (wmx > r.x1 && wmx < r.x2 && wmy > r.y1 && wmy < r.y2) {
                if (bAllowDragging) {
                    if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                        _bDragging = 1;
                        if (_iMoveInitX == -1)
                            _iMoveInitX = hTempObj->GetParam(WWD::Param_LocationX);
                        if (_iMoveInitY == -1)
                            _iMoveInitY = hTempObj->GetParam(WWD::Param_LocationY);
                        _iDragOffX = wmx - hTempObj->GetParam(WWD::Param_LocationX);
                        _iDragOffY = wmy - hTempObj->GetParam(WWD::Param_LocationY);
                    }
                }
            }
        }
    }

    bool cObjEdit::ChangesMade() {
        if (strcmp(hTempObj->GetName(), hOrigObj->GetName()) != 0)
            return 1;
        if (strcmp(hTempObj->GetLogic(), hOrigObj->GetLogic()) != 0)
            return 1;
        if (strcmp(hTempObj->GetImageSet(), hOrigObj->GetImageSet()) != 0)
            return 1;
        if (strcmp(hTempObj->GetAnim(), hOrigObj->GetAnim()) != 0)
            return 1;

        for (int i = 0; i < 8; i++) {
            if (hTempObj->GetUserValue(i) != hOrigObj->GetUserValue(i))
                return 1;
        }

        for (int i = 0; i < OBJ_PARAMS_CNT; i++) {
            if ((WWD::OBJ_PARAMS) i == WWD::Param_ID) continue;
            if (hTempObj->GetParam((WWD::OBJ_PARAMS) i) != hOrigObj->GetParam((WWD::OBJ_PARAMS) i))
                return 1;
        }

        if (hTempObj->GetMoveRect() != hOrigObj->GetMoveRect())
            return 1;
        if (hTempObj->GetHitRect() != hOrigObj->GetHitRect())
            return 1;
        if (hTempObj->GetAttackRect() != hOrigObj->GetAttackRect())
            return 1;
        if (hTempObj->GetClipRect() != hOrigObj->GetClipRect())
            return 1;
        if (hTempObj->GetUserRect(0) != hOrigObj->GetUserRect(0))
            return 1;
        if (hTempObj->GetUserRect(1) != hOrigObj->GetUserRect(1))
            return 1;

        if (hTempObj->GetDrawFlags() != hOrigObj->GetDrawFlags())
            return 1;
        if (hTempObj->GetDynamicFlags() != hOrigObj->GetDynamicFlags())
            return 1;
        if (hTempObj->GetAddFlags() != hOrigObj->GetAddFlags())
            return 1;
        if (hTempObj->GetHitTypeFlags() != hOrigObj->GetHitTypeFlags())
            return 1;
        if (hTempObj->GetTypeFlags() != hOrigObj->GetTypeFlags())
            return 1;
        if (hTempObj->GetUserFlags() != hOrigObj->GetUserFlags())
            return 1;

        return 0;
    }

	bool cObjEdit::IsAnyInputFocused()
	{
#ifdef WAPMAP_ENABLE_IMGUI
        if (nativeWindow) return nativeInputFocused;
#endif
		return win->gotFocus();
	}

	void cObjEdit::GetWindowPosition(int& x, int& y)
	{
        x = win->getX();
        y = win->getY();
	}

	void cObjEdit::SetWindowPosition(int x, int y)
	{
        win->setX(x);
        win->setY(y);
	}

    void cObjEdit::HandleEscape() {
        if (!GV->editState->hInvCtrl->IsClipboardEmpty()) {
            GV->editState->hInvCtrl->SetClipboardEmpty();
            return;
        }

        if (IsMovingObject()) {
            _bDragging = 0;
            GV->editState->vPort->MarkToRedraw();
            hTempObj->SetParam(WWD::Param_LocationX, _iMoveInitX);
            hTempObj->SetParam(WWD::Param_LocationY, _iMoveInitY);
            GetUserDataFromObj(hTempObj)->SyncToObj();
            return;
        }

        SetKill(true);
    }

    void cObjEdit::SaveChanges() {
        bObjectSaved = true;

        if (!hState->bEditObjDelete) {
            bChangesMade = ChangesMade();
            if (!bChangesMade) return;
        }

        WWD::Object *tmpptr;
        tmpptr = hTempObj;
        hTempObj = hOrigObj;
        hOrigObj = tmpptr;
        int iOrigid = hTempObj->GetParam(WWD::Param_ID);
        hOrigObj->SetParam(WWD::Param_ID, iOrigid);
        hState->MarkUnsaved();
        hState->vPort->MarkToRedraw();
    }

	void cObjEditVP::Draw(int iCode)
	{
		m_hOwn->Draw();
	}
#ifdef WAPMAP_ENABLE_IMGUI
    bool cObjEdit::BeginNativeWindow() {
        if (!nativeWindow || bKill || nativeBusy) return false;
        // Some old forms sized the body without accounting for the title bar.
        // Keep the footer inside the native window instead of clipping it or
        // introducing a few pixels of root-window scrolling.
        const auto bottom = [&](auto&& self, SHR::Container* container, int offset) -> int {
            int result = offset;
            for (auto* widget : container->getChildren()) {
                if (!widget->isLocallyVisible() || widget == vpAdv) continue;
                if (auto* rectangle = dynamic_cast<cProcPickRect*>(widget))
                    result = std::max(result, self(self, rectangle, offset + rectangle->getY()));
                else result = std::max(result, offset + widget->getY() + widget->getHeight());
            }
            return result;
        };
        const int requiredHeight = static_cast<int>(ImGui::GetFrameHeight()) + bottom(bottom, win, 0) + 4;
        if (requiredHeight > win->getHeight()) {
            if (nativePosition) win->setY(win->getY() - (requiredHeight - win->getHeight()));
            win->setHeight(requiredHeight);
        }
        const auto title = LegacyText::ToUtf8(win->getCaption(), LegacyText::CodePage(GV->Lang->GetCode())) +
            "###ObjectEditor" + std::to_string(reinterpret_cast<std::uintptr_t>(this));
        if (nativePosition) {
            const auto* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos({viewport->Pos.x + win->getX(), viewport->Pos.y + win->getY()}, ImGuiCond_FirstUseEver);
            nativePosition = false;
        }
        const bool bounded = nativeHeightLimit > 0 && nativeHeightLimit < win->getHeight();
        ImGui::SetNextWindowSize({win->getWidth() + (bounded ? ImGui::GetStyle().ScrollbarSize : 0.f), bounded ? nativeHeightLimit : static_cast<float>(win->getHeight())}, ImGuiCond_Always);
        nativeFooterY = -1;
        bool open = true;
        const bool shown = WapMapImGui::BeginFixedWindow(title.c_str(), &open,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        if (!open) nativeAction = win;
        nativeInputFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        if (!shown) { ImGui::End(); return false; }
        const auto p = ImGui::GetWindowPos(), main = ImGui::GetMainViewport()->Pos;
        win->setPosition(static_cast<int>(p.x - main.x), static_cast<int>(p.y - main.y));
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, win->getCaption().c_str());
        nativeValid = true;
        return true;
    }

    void cObjEdit::DrawNativeControls(SHR::Container* container, int offsetX, int offsetY) {
        if (!container) container = win;
        const float top = ImGui::GetFrameHeight();
        for (auto* widget : container->getChildren()) {
            if (!widget->isLocallyVisible() || widget == _butSave || widget == _butAddNext) continue;
            ImGui::PushID(widget);
            const int x = offsetX + widget->getX(), y = offsetY + widget->getY();
            ImGui::SetCursorPos({static_cast<float>(x), top + y});
            ImGui::BeginDisabled(!widget->isLocallyEnabled() || nativeAction);
            if (auto* label = dynamic_cast<SHR::Lab*>(widget)) {
                WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16,
                    ImGui::GetCursorScreenPos(), label->getCaption().c_str());
            } else if (auto* radio = dynamic_cast<SHR::RadBut*>(widget)) {
                int choice = radio->isSelected() ? 1 : 0;
                if (WapMapImGui::BitmapRadio(*hge, *GV->fntMyriad16, *GV->hGfxInterface, radio->getCaption().c_str(), choice, 1)) {
                    radio->setSelected(true); nativeAction = radio;
                }
            } else if (auto* check = dynamic_cast<SHR::CBox*>(widget)) {
                bool value = check->isSelected();
                if (WapMapImGui::BitmapCheckbox(*hge, *GV->fntMyriad16, *GV->hGfxInterface, check->getCaption().c_str(), value)) {
                    check->setSelected(value); nativeAction = check;
                }
            } else if (auto* list = dynamic_cast<SHR::ListBox*>(widget)) {
                auto* model = list->getListModel();
                ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(18, 18, 18, 255));
                if (ImGui::BeginChild("##list", {static_cast<float>(list->getWidth()), static_cast<float>(list->getHeight())}, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened)) {
                    for (int i = 0; model && i < model->getNumberOfElements(); ++i) {
                        ImGui::PushID(i);
                        const bool selected = i == list->getSelected();
                        ImGui::PushItemFlag(ImGuiItemFlags_NoTabStop, !selected);
                        const auto caption = LegacyText::ToUtf8(model->getElementAt(i), LegacyText::CodePage(GV->Lang->GetCode()));
                        if (ImGui::Selectable(caption.c_str(), selected, ImGuiSelectableFlags_SelectOnNav)) {
                            list->setSelected(i, false); nativeAction = list;
                        }
                        ImGui::PopItemFlag();
                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
            } else if (auto* segments = dynamic_cast<SHR::ComboBut*>(widget)) {
                const auto& entries = segments->getEntries();
                for (size_t i = 0; i < entries.size(); ++i) {
                    if (i) ImGui::SameLine(0, 1);
                    ImGui::PushID(static_cast<int>(i));
                    const bool selected = segments->getSelectedEntryID() == static_cast<int>(i);
                    if (selected) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
                    const auto caption = LegacyText::ToUtf8(entries[i].GetCaption(), LegacyText::CodePage(GV->Lang->GetCode()));
                    if (ImGui::Button(caption.c_str(), {static_cast<float>(entries[i].GetWidth()), 20})) {
                        segments->setSelectedEntryID(static_cast<int>(i)); nativeAction = segments;
                    }
                    if (selected) ImGui::PopStyleColor();
                    if (entries[i].GetEntryTooltip() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
                        const auto tooltip = LegacyText::ToUtf8(entries[i].GetEntryTooltip(), LegacyText::CodePage(GV->Lang->GetCode()));
                        ImGui::SetTooltip("%s", tooltip.c_str());
                    }
                    ImGui::PopID();
                }
            } else if (auto* combo = dynamic_cast<SHR::DropDown*>(widget)) {
                auto* model = combo->getListModel();
                const int selected = combo->getSelected();
                const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
                const auto caption = model && selected >= 0 && selected < model->getNumberOfElements() ? LegacyText::ToUtf8(model->getElementAt(selected), cp) : std::string();
                ImGui::SetNextItemWidth(static_cast<float>(combo->getWidth()));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {4, std::max(0.f, (20 - GV->fntMyriad16->GetHeight()) / 2)});
                if (ImGui::BeginCombo("##choice", caption.c_str())) {
                    for (int i = 0; model && i < model->getNumberOfElements(); ++i) {
                        ImGui::PushID(i);
                        const auto label = LegacyText::ToUtf8(model->getElementAt(i), cp);
                        if (ImGui::Selectable(label.c_str(), i == selected)) { combo->setSelected(i, false); nativeAction = combo; }
                        ImGui::PopID();
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopStyleVar();
            } else if (auto* combo = dynamic_cast<SHR::TextDropDown*>(widget)) {
                const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
                auto [entry, inserted] = nativeTexts.try_emplace(widget);
                auto& draft = entry->second;
                if (inserted || draft.legacy != combo->getText()) {
                    draft.legacy = combo->getText();
                    const auto text = LegacyText::ToUtf8(draft.legacy, cp);
                    std::snprintf(draft.text.data(), draft.text.size(), "%s", text.c_str());
                }
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {4, std::max(0.f, (20 - GV->fntMyriad16->GetHeight()) / 2)});
                ImGui::SetNextItemWidth(static_cast<float>(combo->getWidth()) - 24);
                const bool edited = ImGui::InputText("##text", draft.text.data(), draft.text.size());
                const auto encoded = LegacyText::FromUtf8(draft.text.data(), cp, std::max(size_t(128), combo->getText().size()));
                if ((!encoded || combo->isMarkedInvalid()) && combo->isLocallyEnabled()) {
                    nativeValid = false;
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(220, 75, 75, 255));
                }
                if (edited && encoded) { combo->setText(*encoded); draft.legacy = *encoded; nativeAction = combo; }
                ImGui::SameLine(0, 0);
                if (ImGui::ArrowButton("##list", ImGuiDir_Down)) ImGui::OpenPopup("choices");
                if (ImGui::BeginPopup("choices")) {
                    auto* model = combo->getListModel();
                    for (int i = 0; model && i < model->getNumberOfElements(); ++i) {
                        ImGui::PushID(i);
                        const auto value = model->getElementAt(i), caption = LegacyText::ToUtf8(value, cp);
                        if (ImGui::Selectable(caption.c_str(), value == combo->getText())) {
                            combo->setText(value); nativeTexts.erase(widget); nativeAction = combo;
                        }
                        ImGui::PopID();
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopStyleVar();
            } else if (auto* field = dynamic_cast<SHR::TextField*>(widget)) {
                auto [entry, inserted] = nativeIntegers.try_emplace(field);
                auto& draft = entry->second;
                if (inserted || draft.legacy != field->getText()) {
                    draft.legacy = field->getText();
                    std::snprintf(draft.text.data(), draft.text.size(), "%s", draft.legacy.c_str());
                }
                ImGui::SetNextItemWidth(static_cast<float>(field->getWidth()));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {4, std::max(0.f, (20 - GV->fntMyriad16->GetHeight()) / 2)});
                const bool edited = ImGui::InputText("##integer", draft.text.data(), draft.text.size(), ImGuiInputTextFlags_CharsDecimal);
                ImGui::PopStyleVar();
                const auto number = CoordinateNavigation::Parse(draft.text.data());
                const bool valid = number && (!field->IsNumericalOnly() || field->AllowsNegative() || *number >= 0);
                if ((!valid || field->isMarkedInvalid()) && field->isLocallyEnabled()) {
                    nativeValid = false;
                    ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(220, 75, 75, 255));
                }
                if (edited && valid) {
                    field->setText(draft.text.data()); draft.legacy = field->getText(); nativeAction = field;
                }
            } else if (auto* button = dynamic_cast<SHR::But*>(widget)) {
                const gcn::Rectangle area(offsetX, static_cast<int>(top) + offsetY, win->getWidth(), win->getHeight());
                if (AssetBrowserUi::Button(*hge, *GV->fntMyriad16, *GV->hGfxInterface, area, button, !nativeAction)) nativeAction = button;
                if (button->isHighlighted()) ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::GetColorU32(ImGuiCol_CheckMark));
            } else if (auto* rectangle = dynamic_cast<cProcPickRect*>(widget)) {
                DrawNativeControls(rectangle, x, y);
            } else if (auto* pair = dynamic_cast<cProcPickXYLockable*>(widget)) {
                DrawNativeControls(pair, x, y);
            } else if (auto* slot = dynamic_cast<SHR::InvTab*>(widget)) {
                const auto p = ImGui::GetCursorScreenPos();
                const ImVec2 size{static_cast<float>(slot->getWidth()), static_cast<float>(slot->getHeight())};
                const auto item = slot->GetItem();
                const bool clicked = ImGui::InvisibleButton("##slot", size, ImGuiButtonFlags_EnableNav);
                const bool hovered = ImGui::IsItemHovered() || WapMapImGui::IsKeyboardFocused();
                auto* draw = ImGui::GetWindowDrawList();
                draw->AddRectFilled(p, {p.x + size.x, p.y + size.y}, IM_COL32(30, 30, 30, 255));
                draw->AddRect(p, {p.x + size.x, p.y + size.y}, hovered ? IM_COL32(23, 150, 255, 255) : IM_COL32(75, 75, 75, 255));
                const float imageSize = std::min(size.x, size.y) - 4;
                if (slot->HasCrab()) NativeInventoryUi::Image({"LEVEL_HERMITCRAB", -1}, {p.x + 2, p.y + 2}, imageSize);
                NativeInventoryUi::Image(item, {p.x + 2, p.y + 2}, imageSize, slot->IsWarpNotFunctional() && item.second == 32);
                const auto assign = [&](int id) { nativeInventoryTarget = slot; nativeInventoryItem = id; nativeAction = slot; };
                if (clicked) {
                    if (nativeInventorySelection >= 0) { assign(nativeInventorySelection); nativeInventorySelection = -1; }
                    else if (item.second >= 0) { nativeInventorySelection = item.second; assign(-1); }
                }
                if (!nativeAction && ((ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) ||
                        (WapMapImGui::IsKeyboardFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete)))) assign(-1);
                if (!nativeAction && item.second >= 0 && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
                    const auto* payload = ImGui::GetDragDropPayload();
                    const bool first = !payload || payload->DataSize == 0;
                    ImGui::SetDragDropPayload(NativeInventoryUi::Payload, &item.second, sizeof(item.second), ImGuiCond_Once);
                    if (first) { nativeInventorySelection = -1; assign(-1); }
                    ImGui::EndDragDropSource();
                }
                if (!nativeAction && ImGui::BeginDragDropTarget()) {
                    if (const auto* payload = ImGui::AcceptDragDropPayload(NativeInventoryUi::Payload)) {
                        const int id = NativeInventoryUi::Item(payload);
                        if (id >= 0) assign(id);
                    }
                    ImGui::EndDragDropTarget();
                }
            }
            ImGui::EndDisabled(); ImGui::PopID();
        }
    }

    void cObjEdit::DrawNativeInventory(const int* items, SHR::RadBut* const* choices, int count, float effectY) {
        int selected = 0;
        for (int i = 0; i < count; ++i) if (choices[i]->isSelected()) selected = i;
        int preview = selected;
        const float top = ImGui::GetFrameHeight();
        for (int i = 0; i < count; ++i) {
            ImGui::PushID(choices[i]);
            ImGui::SetCursorPos({choices[i]->getX() + 20.f, top + choices[i]->getY() - 22.f});
            const auto p = ImGui::GetCursorScreenPos();
            ImGui::BeginDisabled(nativeAction != nullptr);
            const bool hit = ImGui::InvisibleButton("##preview", {48, 54}, ImGuiButtonFlags_EnableNav);
            const bool hover = ImGui::IsItemHovered() || WapMapImGui::IsKeyboardFocused();
            if (hover) preview = i;
            auto* asset = hState->SprBank->GetAssetByID(hState->hInvCtrl->GetItemByID(items[i]).first.c_str());
            auto* sprite = GV->sprSmiley;
            if (asset && asset->GetSpritesCount() > 0) {
                auto* image = asset->GetIMGByIterator(hState->hInvCtrl->GetAnimFrame() % asset->GetSpritesCount());
                if (image && image->GetSprite()) sprite = image->GetSprite();
            }
            AssetBrowserUi::Preview(*hge, sprite, {p.x, p.y + 3}, 44, false, false, selected == i, hover);
            if (hit) { choices[i]->setSelected(true); nativeAction = choices[i]; }
            ImGui::EndDisabled(); ImGui::PopID();
        }
        if (effectY >= 0) {
            wchar_t key[32]; wsprintfW(key, L"E_%d", hState->hInvCtrl->GetItemByID(items[preview]).second);
            const std::string effect = std::string(GETL2S("EditObj_Inventory", "Effect")) + ": " + GETL2SV("EditObj_Inventory", key);
            const auto origin = ImGui::GetWindowPos();
            WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16,
                {origin.x + 7, origin.y + top + effectY}, effect.c_str());
        }
    }

    void cObjEdit::DrawImGui() {
        if (!BeginNativeWindow()) return;
        DrawNativeControls();
        EndNativeWindow();
    }

    void cObjEdit::EndNativeWindow() {
        for (auto* button : {_butAddNext, _butSave}) {
            const gcn::Rectangle area(0, static_cast<int>(nativeFooterY >= 0 ? nativeFooterY - button->getY() : ImGui::GetFrameHeight()), win->getWidth(), win->getHeight());
            if (button->getParent() == win && AssetBrowserUi::Button(*hge, *GV->fntMyriad16, *GV->hGfxInterface, area, button, !nativeAction && nativeValid)) nativeAction = button;
        }
        if (nativeInputFocused && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            const auto* payload = ImGui::GetDragDropPayload();
            if (nativeInventorySelection >= 0 || (payload && payload->IsDataType(NativeInventoryUi::Payload))) {
                nativeInventorySelection = -1;
                if (payload && payload->IsDataType(NativeInventoryUi::Payload)) ImGui::ClearDragDrop();
            } else nativeAction = vpAdv;
        }
        ImGui::End();
    }

    void cObjEdit::DrawNativeSeries(SHR::Win* panel, SHR::TextField* delay, SHR::TextField* increment) {
        const auto title = LegacyText::ToUtf8(panel->getCaption(), LegacyText::CodePage(GV->Lang->GetCode())) +
            "###ObjectEditorSeries" + std::to_string(reinterpret_cast<std::uintptr_t>(this));
        const auto main = ImGui::GetMainViewport()->Pos;
        ImGui::SetNextWindowPos({main.x + panel->getX(), main.y + panel->getY()}, ImGuiCond_FirstUseEver);
        const float height = std::max(static_cast<float>(panel->getHeight()),
            ImGui::GetFrameHeight() + _butAddNext->getY() + _butAddNext->getHeight() + 4);
        ImGui::SetNextWindowSize({static_cast<float>(panel->getWidth()), height}, ImGuiCond_Always);
        if (WapMapImGui::BeginFixedWindow(title.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing)) {
            WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, nullptr, panel->getCaption().c_str());
            DrawNativeControls(panel);
            const auto start = CoordinateNavigation::Parse(delay->getText()), step = CoordinateNavigation::Parse(increment->getText());
            const bool fits = start && step && ObjectToolGeometry::CanOffset(*start, *step, 1);
            const gcn::Rectangle area(0, static_cast<int>(ImGui::GetFrameHeight()), panel->getWidth(), panel->getHeight());
            if (AssetBrowserUi::Button(*hge, *GV->fntMyriad16, *GV->hGfxInterface, area, _butAddNext, !nativeAction && nativeValid && fits)) nativeAction = _butAddNext;
            if (!fits) {
                const auto p = ImGui::GetWindowPos();
                ImGui::GetWindowDrawList()->AddRect({p.x + increment->getX(), p.y + area.y + increment->getY()},
                    {p.x + increment->getX() + increment->getWidth(), p.y + area.y + increment->getY() + increment->getHeight()}, IM_COL32(220, 75, 75, 255));
            }
            const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
            nativeInputFocused |= focused;
            if (focused && ImGui::IsKeyPressed(ImGuiKey_Escape)) nativeAction = vpAdv;
        }
        ImGui::End();
    }

    void cObjEdit::DrawNativeImageChoices(const std::vector<std::string>& sets, SHR::RadBut* const* choices, int frame) {
        const float top = ImGui::GetFrameHeight();
        for (size_t i = 0; i < sets.size(); ++i) {
            auto* radio = choices[i];
            if (!radio || !radio->isLocallyVisible()) continue;
            ImGui::PushID(radio);
            ImGui::SetCursorPos({radio->getX() + 22.f, top + 13});
            const auto p = ImGui::GetCursorScreenPos();
            ImGui::BeginDisabled(nativeAction || !radio->isLocallyEnabled());
            const bool hit = ImGui::InvisibleButton("##preview", {48, 48}, ImGuiButtonFlags_EnableNav);
            auto* asset = hState->SprBank->GetAssetByID(sets[i].c_str());
            auto* image = asset && asset->GetSpritesCount() > 0 ? asset->GetIMGByIterator(frame % asset->GetSpritesCount()) : nullptr;
            auto* sprite = image && image->GetSprite() ? image->GetSprite() : GV->sprSmiley;
            AssetBrowserUi::Preview(*hge, sprite, p, 48, false, false, radio->isSelected(),
                ImGui::IsItemHovered() || WapMapImGui::IsKeyboardFocused());
            if (hit) { radio->setSelected(true); nativeAction = radio; }
            ImGui::EndDisabled(); ImGui::PopID();
        }
    }
#endif
}
