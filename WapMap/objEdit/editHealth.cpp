#include "editHealth.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include "../databanks/imageSets.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../imguiTheme.h"
#include "../windows/assetBrowserUi.h"
#endif

extern HGE *hge;

namespace ObjEdit {
    static const int iHealthID[] = {24, 26, 27, 25, 20, 21, 19, 34, 28, 29, 30};

    cEditObjHealth::cEditObjHealth(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enHealth;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Health", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 287, 287));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());
#ifdef WAPMAP_ENABLE_IMGUI
        nativeWindow = true;
        win->setVisible(false);
#endif

        win->add(_butAddNext, win->getWidth() - 210, 227);
        win->add(_butSave, win->getWidth() - 110, 227);

        char tmp[24];
        sprintf(tmp, "%p", this);

        for (int i = 0; i < 11; i++) {
            int x = i % 4, y = i / 4;
            rbType[i] = new SHR::RadBut(GV->hGfxInterface, "", tmp,
                                        !strcmp(hTempObj->GetImageSet(),
                                                hState->hInvCtrl->GetItemByID(iHealthID[i]).first.c_str()));
            rbType[i]->adjustSize();
            rbType[i]->addActionListener(hAL);
            win->add(rbType[i], 5 + x * 71, 5 + y * 60 + 30 - rbType[i]->getHeight() / 2);
        }

        if (!hTempObj->GetParam(WWD::Param_LocationZ)) {
            hTempObj->SetParam(WWD::Param_LocationZ, 1000);
            GetUserDataFromObj(hTempObj)->SetZ(1000);
            hState->vPort->MarkToRedraw();
        }

        for (auto & i : rbType)
            if (i->isSelected()) return;

        rbType[0]->setSelected(1);
        hTempObj->SetLogic("HealthPowerup");
        hTempObj->SetImageSet(hState->hInvCtrl->GetItemByID(iHealthID[0]).first.c_str());
    }

    cEditObjHealth::~cEditObjHealth() {
        for (int i = 0; i < 11; i++)
            delete rbType[i];
        delete win;
        hState->vPort->MarkToRedraw();
    }

    void cEditObjHealth::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        }
        for (int i = 0; i < 11; i++)
            if (actionEvent.getSource() == rbType[i]) {
                if (i < 4) hTempObj->SetLogic("HealthPowerup");
                else if (i < 7) hTempObj->SetLogic("AmmoPowerup");
                else if (i == 7) hTempObj->SetLogic("SpecialPowerup");
                else hTempObj->SetLogic("MagicPowerup");
                hTempObj->SetImageSet(hState->hInvCtrl->GetItemByID(iHealthID[i]).first.c_str());
                hState->vPort->MarkToRedraw();
            }
    }

    void cEditObjHealth::Draw() {
        int dx, dy;
        win->getAbsolutePosition(dx, dy);

        for (int i = 1; i < 4; i++)
            hge->Gfx_RenderLine(dx + 5 + i * 71 - 1, dy + 28, dx + 5 + i * 71 - 1, dy + 29 + 267 - 90,
                                GV->colLineBright);
        for (int i = 1; i < 4; i++) {
            hge->Gfx_RenderLine(dx + 4, dy + 29 + i * 60 - 2, dx + win->getWidth(), dy + 29 + i * 60 - 2,
                                GV->colLineDark);
            hge->Gfx_RenderLine(dx + 4, dy + 29 + i * 60 - 1, dx + win->getWidth(), dy + 29 + i * 60 - 2,
                                GV->colLineBright);
        }
        for (int i = 1; i < 4; i++)
            hge->Gfx_RenderLine(dx + 5 + i * 71 - 2, dy + 28, dx + 5 + i * 71 - 2, dy + 29 + 267 - 90, GV->colLineDark);

        int picked = -1;
        for (int i = 0; i < 11; i++)
            if (rbType[i]->isSelected()) {
                picked = i;
                break;
            }

        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        for (int i = 0; i < 11; i++) {
            int x = i % 4, y = i / 4;
            cSprBankAsset* asset = hState->SprBank->GetAssetByID(
                    hState->hInvCtrl->GetItemByID(iHealthID[i]).first.c_str());
            hgeSprite *spr = GV->sprSmiley;
            if (asset) {
                int iframe = hState->hInvCtrl->GetAnimFrame() % asset->GetSpritesCount();
                spr = asset->GetIMGByIterator(iframe)->GetSprite();
            }
            spr->SetColor(0xFFFFFFFF);
            spr->SetFlip(0, 0);
            int grdim = spr->GetWidth();
            if (spr->GetHeight() > grdim) grdim = spr->GetHeight();
            float fScale = 1.0f;
            if (grdim > 48) fScale = 48.0f / float(grdim);
            spr->RenderEx(dx + 12 + x * 71 + 35, dy + 28 + y * 60 + 30 - 4, 0, fScale);
            if (hState->conMain->getWidgetAt(mx, my) == win)
                if (mx > dx + 12 + x * 71 + 35 - 24 && mx < dx + 12 + x * 71 + 35 + 24 &&
                    my > dy + 28 + y * 60 + 30 - 4 - 24 && my < dy + 28 + y * 60 + 30 - 4 + 24) {
                    picked = i;
                    if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                        rbType[i]->setSelected(1);
                        Action(gcn::ActionEvent(rbType[i], ""));
                    }
                }
        }

        wchar_t tmp[64];
        wsprintfW(tmp, L"E_%d", hState->hInvCtrl->GetItemByID(iHealthID[picked]).second);
        GV->fntMyriad16->SetColor(0xFFe1e1e1);
        GV->fntMyriad16->printf(dx + 5, dy + 28 + 267 - 90 + 15, HGETEXT_LEFT, "%s: ~y~%s", 0,
                                GETL2S("EditObj_Inventory", "Effect"), GETL2SV("EditObj_Inventory", tmp));
    }
#ifdef WAPMAP_ENABLE_IMGUI
    void cEditObjHealth::DrawImGui() {
        if (!BeginNativeWindow()) return;
        int picked = 0;
        for (int i = 0; i < 11; ++i) if (rbType[i]->isSelected()) picked = i;
        int preview = picked;
        const float top = ImGui::GetFrameHeight();
        for (int i = 0; i < 11; ++i) {
            ImGui::PushID(i);
            const float x = 5.f + i % 4 * 71, y = top + 5.f + i / 4 * 60;
            ImGui::SetCursorPos({x, y + 22});
            int choice = picked;
            bool hit = WapMapImGui::BitmapRadio(*hge, *GV->fntMyriad16, *GV->hGfxInterface, "", choice, i);
            ImGui::SetCursorPos({x + 20, y});
            const auto p = ImGui::GetCursorScreenPos();
            hit |= ImGui::InvisibleButton("preview", {48, 54}, ImGuiButtonFlags_EnableNav);
            const bool hover = ImGui::IsItemHovered() || WapMapImGui::IsKeyboardFocused();
            if (hover) preview = i;
            auto* asset = hState->SprBank->GetAssetByID(hState->hInvCtrl->GetItemByID(iHealthID[i]).first.c_str());
            auto* sprite = GV->sprSmiley;
            if (asset && asset->GetSpritesCount() > 0) {
                auto* image = asset->GetIMGByIterator(hState->hInvCtrl->GetAnimFrame() % asset->GetSpritesCount());
                if (image && image->GetSprite()) sprite = image->GetSprite();
            }
            AssetBrowserUi::Preview(*hge, sprite, {p.x, p.y + 3}, 44, false, false, picked == i, hover);
            if (hit && !nativeAction) {
                rbType[i]->setSelected(true);
                nativeAction = rbType[i];
            }
            ImGui::PopID();
        }
        wchar_t key[32];
        wsprintfW(key, L"E_%d", hState->hInvCtrl->GetItemByID(iHealthID[preview]).second);
        const std::string effect = std::string(GETL2S("EditObj_Inventory", "Effect")) + ": " + GETL2SV("EditObj_Inventory", key);
        const auto origin = ImGui::GetWindowPos();
        WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16,
            {origin.x + 5, origin.y + top + 202}, effect.c_str());
        EndNativeWindow();
    }
#endif
}
