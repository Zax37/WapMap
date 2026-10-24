#include "editText.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include "../databanks/imageSets.h"
#include "textLayout.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "nativeTextInput.h"
#endif

extern HGE *hge;

namespace ObjEdit {
    cEditObjText::cEditObjText(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        bAllowDragging = 0;
        iType = ObjEdit::enText;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Text", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 400, 270));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        hTempObj->SetLogic("DoNothing");
        hTempObj->SetImageSet("GAME_FONT");

        areaX = hTempObj->GetParam(WWD::Param_LocationX);
        areaY = hTempObj->GetParam(WWD::Param_LocationY);
        areaW = areaH = 20;

        tbText = new SHR::TextBox(GETL2S("EditObj_Text", "Sample"));
        tbText->setFont(GV->gcnParts.gcnfntMyriad16);
        tbText->setForegroundColor(0xFFFFFF);
        tbText->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tbText->setMinimalSize(300, 100);
        tbText->addActionListener(hAL);

        saText = new SHR::ScrollArea();
        saText->setContent(tbText);
        saText->setBackgroundColor(0x3c3c3c);
        saText->setDimension(gcn::Rectangle(0, 0, 388, 150));
        win->add(saText, 4, 13);

        labAlign = new SHR::Lab(GETL2S("EditObj_Text", "Align"));
        labAlign->adjustSize();
        win->add(labAlign, 5, 176);

        butApply = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Text", "Apply"));
        butApply->setDimension(gcn::Rectangle(0, 0, 100, 33));
        butApply->addActionListener(hAL);
        butApply->setEnabled(0);
        win->add(butApply, 290, 170);

        butAlign[0] = GV->editState->MakeButton(15 + labAlign->getWidth(), 170, Icon_TextAlign_Left, win);
        butAlign[1] = GV->editState->MakeButton(15 + labAlign->getWidth() + 32, 170, Icon_TextAlign_Justify, win);
        butAlign[2] = GV->editState->MakeButton(15 + labAlign->getWidth() + 64, 170, Icon_TextAlign_Right, win);
        butAlign[1]->setIconColor(0x55FFFFFF);
        butAlign[2]->setIconColor(0x55FFFFFF);
        butAlign[0]->setHighlight(true);
        for (int i = 0; i < 3; i++) {
            butAlign[i]->removeActionListener(GV->editState->mainListener);
            butAlign[i]->addActionListener(hAL);
        }
        iAlign = 0;

        GenerateText();
        iDragX = hTempObj->GetParam(WWD::Param_LocationX);
        iDragY = hTempObj->GetParam(WWD::Param_LocationY);

        win->add(_butSave, 150, 220);
#ifdef WAPMAP_ENABLE_IMGUI
        nativeWindow = true;
        win->setVisible(false);
        nativeText = LegacyText::ToUtf8(tbText->getText(), LegacyText::CodePage(GV->Lang->GetCode()));
#endif
    }

    cEditObjText::~cEditObjText() {
        if (!ObjectSaved()) {
            for (int i = 1; i < hObjects.size(); i++) {
                GV->editState->GetActivePlane()->DeleteObject(hObjects[i]);
            }
            hObjects.clear();
            hObjects.push_back(hTempObj);
        }
        for (int i = 0; i < 3; i++)
            delete butAlign[i];
        delete labAlign;
        delete butApply;
        delete saText;
        delete tbText;
        delete win;
        hState->vPort->MarkToRedraw();
    }

    bool cEditObjText::CanGenerate() {
        auto* asset = hState->SprBank->GetAssetByID("GAME_FONT");
        return MapTextLayout::Build(tbText->getText(), areaX, areaY, iAlign, [asset](int frame) {
            auto* image = asset ? asset->GetIMGByID(frame) : nullptr;
            return image && image->GetSprite() ? static_cast<int>(image->GetSprite()->GetWidth()) : 0;
        }).has_value();
    }

    void cEditObjText::GenerateText() {
        auto* asset = hState->SprBank->GetAssetByID("GAME_FONT");
        const auto layout = MapTextLayout::Build(tbText->getText(), areaX, areaY, iAlign, [asset](int frame) {
            auto* image = asset ? asset->GetIMGByID(frame) : nullptr;
            return image && image->GetSprite() ? static_cast<int>(image->GetSprite()->GetWidth()) : 0;
        });
        if (!layout) { _butSave->setEnabled(false); butApply->setEnabled(false); return; }
        for (size_t i = 1; i < hObjects.size(); ++i) hState->GetActivePlane()->DeleteObject(hObjects[i]);
        hObjects.clear();
        for (const auto& glyph : layout->glyphs) {
            auto* object = hObjects.empty() ? hTempObj : new WWD::Object();
            object->SetLogic("DoNothing");
            object->SetImageSet("GAME_FONT");
            object->SetParam(WWD::Param_LocationI, glyph.frame);
            object->SetParam(WWD::Param_LocationX, glyph.x);
            object->SetParam(WWD::Param_LocationY, glyph.y);
            if (object != hTempObj) {
                hState->GetActivePlane()->AddObjectAndCalcID(object);
                object->SetUserData(new cObjUserData(object));
            }
            GetUserDataFromObj(object)->SyncToObj();
            hState->hPlaneData[hState->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(object);
            hObjects.push_back(object);
        }
        areaW = layout->width;
        areaH = layout->height;
        iDragX = hTempObj->GetParam(WWD::Param_LocationX);
        iDragY = hTempObj->GetParam(WWD::Param_LocationY);
        butApply->setEnabled(false);
        _butSave->setEnabled(true);
        hState->vPort->MarkToRedraw();
    }

    void cEditObjText::ApplyAlign() {
        auto* asset = hState->SprBank->GetAssetByID("GAME_FONT");
        const auto layout = MapTextLayout::Build(tbText->getText(), areaX, areaY, iAlign, [asset](int frame) {
            auto* image = asset ? asset->GetIMGByID(frame) : nullptr;
            return image && image->GetSprite() ? static_cast<int>(image->GetSprite()->GetWidth()) : 0;
        });
        if (!layout || layout->glyphs.size() != hObjects.size()) { GenerateText(); return; }
        for (size_t i = 0; i < hObjects.size(); ++i) {
            hObjects[i]->SetParam(WWD::Param_LocationX, layout->glyphs[i].x);
            hObjects[i]->SetParam(WWD::Param_LocationY, layout->glyphs[i].y);
            GetUserDataFromObj(hObjects[i])->SyncToObj();
            hState->hPlaneData[hState->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(hObjects[i]);
        }
        areaW = layout->width; areaH = layout->height;
        iDragX = hTempObj->GetParam(WWD::Param_LocationX);
        iDragY = hTempObj->GetParam(WWD::Param_LocationY);
        hState->vPort->MarkToRedraw();
    }

    void cEditObjText::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == tbText) {
            const bool valid = CanGenerate();
            butApply->setEnabled(valid);
            _butSave->setEnabled(valid);
        } else if (actionEvent.getSource() == butApply) {
            GenerateText();
            return;
        }
        for (int i = 0; i < 3; i++)
            if (actionEvent.getSource() == butAlign[i]) {
                iAlign = i;
                for (int y = 0; y < 3; y++)
                    { butAlign[y]->setIconColor(y == i ? 0xFFFFFFFF : 0x55FFFFFF); butAlign[y]->setHighlight(y == i); }
                if (butApply->isLocallyEnabled())
                    GenerateText();
                else
                    ApplyAlign();
                return;
            }
    }

    void cEditObjText::_Think(bool bMouseConsumed) {
        if (!bMouseConsumed) {
            float mx, my;
            hge->Input_GetMousePos(&mx, &my);
            if (hState->conMain->getWidgetAt(mx, my) != hState->vPort->GetWidget()) return;
            int wmx = GV->editState->Scr2WrdX(GV->editState->GetActivePlane(), mx),
                wmy = GV->editState->Scr2WrdY(GV->editState->GetActivePlane(), my);
            bool mouseOver = (wmx > areaX && wmy > areaY && wmx < areaX + areaW && wmy < areaY + areaH);
            if (mouseOver) {
                if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                    _bDragging = 1;
                    _iMoveInitX = hTempObj->GetParam(WWD::Param_LocationX);
                    _iMoveInitY = hTempObj->GetParam(WWD::Param_LocationY);
                    _iDragOffX = wmx - hTempObj->GetParam(WWD::Param_LocationX);
                    _iDragOffY = wmy - hTempObj->GetParam(WWD::Param_LocationY);
                    iDragX = hTempObj->GetParam(WWD::Param_LocationX);
                    iDragY = hTempObj->GetParam(WWD::Param_LocationY);
                }
            }
        }
    }

    void cEditObjText::Save() {
        if (butApply->isLocallyEnabled())
            GenerateText();
    }

    void cEditObjText::Think(bool consumed) {
        cObjEdit::Think(consumed);
        if (bKill) return;
        if (_bDragging && !hge->Input_GetKeyState(HGEK_LBUTTON)) _bDragging = false;
        const auto dx = std::int64_t(hTempObj->GetParam(WWD::Param_LocationX)) - iDragX;
        const auto dy = std::int64_t(hTempObj->GetParam(WWD::Param_LocationY)) - iDragY;
        if (!dx && !dy) return;
        bool valid = MapTextLayout::Fits(std::int64_t(areaX) + dx) && MapTextLayout::Fits(std::int64_t(areaY) + dy) &&
            MapTextLayout::Fits(std::int64_t(areaX) + dx + areaW) && MapTextLayout::Fits(std::int64_t(areaY) + dy + areaH);
        for (size_t i = 1; i < hObjects.size(); ++i)
            valid = valid && MapTextLayout::Fits(std::int64_t(hObjects[i]->GetParam(WWD::Param_LocationX)) + dx) &&
                MapTextLayout::Fits(std::int64_t(hObjects[i]->GetParam(WWD::Param_LocationY)) + dy);
        if (!valid) {
            hTempObj->SetParam(WWD::Param_LocationX, iDragX); hTempObj->SetParam(WWD::Param_LocationY, iDragY);
            GetUserDataFromObj(hTempObj)->SyncToObj();
            hState->hPlaneData[hState->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(hTempObj);
            hState->vPort->MarkToRedraw(); _bDragging = false; return;
        }
        for (size_t i = 1; i < hObjects.size(); ++i) {
            hObjects[i]->SetParam(WWD::Param_LocationX, static_cast<int>(hObjects[i]->GetParam(WWD::Param_LocationX) + dx));
            hObjects[i]->SetParam(WWD::Param_LocationY, static_cast<int>(hObjects[i]->GetParam(WWD::Param_LocationY) + dy));
            GetUserDataFromObj(hObjects[i])->SyncToObj();
            hState->hPlaneData[hState->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(hObjects[i]);
        }
        iDragX = hTempObj->GetParam(WWD::Param_LocationX); iDragY = hTempObj->GetParam(WWD::Param_LocationY);
        hState->hPlaneData[hState->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(hTempObj);
        areaX = static_cast<int>(areaX + dx); areaY = static_cast<int>(areaY + dy);
        hState->vPort->MarkToRedraw();
    }
#ifdef WAPMAP_ENABLE_IMGUI
    void cEditObjText::DrawImGui() {
        if (!BeginNativeWindow()) return;
        ImGui::SetCursorPos({4, ImGui::GetFrameHeight() + 13});
        ImGui::BeginDisabled(nativeAction != nullptr);
        const bool edited = MapTextInput::Multiline("##mapText", nativeText, {388, 150});
        const auto encoded = LegacyText::FromUtf8(nativeText, LegacyText::CodePage(GV->Lang->GetCode()), nativeText.size());
        if (!encoded || (!edited && !_butSave->isLocallyEnabled())) {
            nativeValid = false;
            ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(220, 75, 75, 255));
        }
        if (edited && encoded) { tbText->setText(*encoded); nativeAction = tbText; }
        ImGui::EndDisabled();
        DrawNativeControls();
        EndNativeWindow();
    }
#endif

    void cEditObjText::RenderObjectOverlay() {
        int ax1 = GV->editState->Wrd2ScrX(GV->editState->GetActivePlane(), areaX),
            ay1 = GV->editState->Wrd2ScrY(GV->editState->GetActivePlane(), areaY),
            ax2 = GV->editState->Wrd2ScrX(GV->editState->GetActivePlane(), areaX + areaW),
            ay2 = GV->editState->Wrd2ScrY(GV->editState->GetActivePlane(), areaY + areaH);
        hge->Gfx_RenderLine(ax1, ay1, ax2, ay1, 0xFFFF0000);
        hge->Gfx_RenderLine(ax1, ay2, ax2, ay2, 0xFFFF0000);
        hge->Gfx_RenderLine(ax1, ay1, ax1, ay2, 0xFFFF0000);
        hge->Gfx_RenderLine(ax2, ay1, ax2, ay2, 0xFFFF0000);
    }

    void cEditObjText::Draw() {
        saText->setBackgroundColor(tbText->isFocused() ? 0x434343 : 0x3c3c3c);
        int dx, dy;
        win->getAbsolutePosition(dx, dy);

        hge->Gfx_RenderLine(dx + 5, dy + 28, dx + 395, dy + 28, GV->colLineDark);
        hge->Gfx_RenderLine(dx + 6, dy + 29, dx + 394, dy + 29, GV->colLineBright);
        hge->Gfx_RenderLine(dx + 6, dy + 28 + 152, dx + 394, dy + 28 + 152, GV->colLineBright);
        hge->Gfx_RenderLine(dx + 5, dy + 29 + 152, dx + 395, dy + 29 + 152, GV->colLineDark);

        hge->Gfx_RenderLine(dx + 5, dy + 27, dx + 5, dy + 29 + 152, GV->colLineDark);
        hge->Gfx_RenderLine(dx + 6, dy + 28, dx + 6, dy + 28 + 152, GV->colLineBright);

        hge->Gfx_RenderLine(dx + 5 + 390, dy + 28, dx + 5 + 390, dy + 28 + 152, GV->colLineBright);
        hge->Gfx_RenderLine(dx + 6 + 390, dy + 27, dx + 6 + 390, dy + 29 + 152, GV->colLineDark);

        hge->Gfx_RenderLine(dx, dy + 225, dx + win->getWidth(), dy + 225, GV->colLineDark);
        hge->Gfx_RenderLine(dx, dy + 226, dx + win->getWidth(), dy + 226, GV->colLineBright);
    }
}
