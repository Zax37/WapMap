#include "winInventoryPickbox.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../databanks/imageSets.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "nativeInventoryUi.h"
#include "../legacyText.h"
#include <imgui_internal.h>
#endif

namespace ObjEdit {
    cInvPickbox::cInvPickbox(bool isWarpNotFunctional): isWarpNotFunctional(isWarpNotFunctional) {
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Inventory", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 254, 346));
        //win->setClose(1);
        win->addActionListener(this);
        //win->add(vpAdv);
        win->setVisible(0);

        sli = new SHR::Slider(631);
        sli->setOrientation(SHR::Slider::VERTICAL);
        sli->setDimension(gcn::Rectangle(0, 0, 11, 271));
        win->add(sli, 240, 10);

        vp = new WIDG::Viewport(this, 0);
        win->add(vp, 5, 5);

        GV->editState->conMain->add(win, 0, 0);
    }

    cInvPickbox::~cInvPickbox() {
        delete vp;
        delete sli;
        delete win;
    }

    void cInvPickbox::Think() {
#ifdef WAPMAP_ENABLE_IMGUI
        if (native) return;
#endif
        if (!win->isVisible()) return;
        int dx, dy;
        win->getAbsolutePosition(dx, dy);
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        int pickedid = -1;
        if (GV->editState->conMain->getWidgetAt(mx, my) == win && mx > dx + 4 && my > dy + 28 && mx < dx + 4 + 240 &&
            my < dy + 28 + 267) {
            sli->setValue(sli->getValue() - hge->Input_GetMouseWheel() * 30);
            mx -= dx + 4;
            my -= dy + 28;
            my += sli->getValue();
            int restx = int(mx) % 60, resty = int(my) % 60;
            if (restx > 5 && restx < 53 && resty > 5 && resty < 53) {
                pickedid = int(my / 60) * 4 + int(mx / 60);
                if (pickedid < InventoryItemsCount) {
                    if (hge->Input_KeyDown(HGEK_LBUTTON))
                        GV->editState->hInvCtrl->SetItemInClipboard(GV->editState->hInvCtrl->GetItemByIt(pickedid),
                                                                    restx - 30, resty - 30);
                }
            }
        }
    }

    void cInvPickbox::Draw(int iCode) {
        //240x900
        int dx, dy;
        win->getAbsolutePosition(dx, dy);

        hge->Gfx_RenderLine(dx + 3, dy + 27, dx + 242, dy + 27, GV->colLineDark);
        hge->Gfx_RenderLine(dx + 3, dy + 28, dx + 242, dy + 28, GV->colLineBright);
        hge->Gfx_RenderLine(dx + 3, dy + 27 + 269, dx + 242, dy + 27 + 269, GV->colLineDark);
        hge->Gfx_RenderLine(dx + 3, dy + 28 + 269, dx + 242, dy + 28 + 269, GV->colLineBright);

        hge->Gfx_RenderLine(dx + 3, dy + 26, dx + 3, dy + 27 + 269, GV->colLineDark);
        hge->Gfx_RenderLine(dx + 4, dy + 28, dx + 4, dy + 28 + 267, GV->colLineBright);

        hge->Gfx_SetClipping(dx + 4, dy + 28, 252, 267);

        int off = sli->getValue();
        int lineoff = off % 60;
        for (int i = 1; i < 4; i++)
            hge->Gfx_RenderLine(dx + 5 + i * 60 - 1, dy + 28, dx + 5 + i * 60 - 1, dy + 29 + 267, GV->colLineBright);
        for (int i = 1; i < 6; i++) {
            hge->Gfx_RenderLine(dx + 4, dy + 29 + i * 60 - 2 - lineoff, dx + 4 + 238, dy + 29 + i * 60 - 2 - lineoff,
                                GV->colLineDark);
            hge->Gfx_RenderLine(dx + 4, dy + 29 + i * 60 - 1 - lineoff, dx + 4 + 238, dy + 29 + i * 60 - 2 - lineoff,
                                GV->colLineBright);
        }
        for (int i = 1; i < 4; i++)
            hge->Gfx_RenderLine(dx + 5 + i * 60 - 2, dy + 28, dx + 5 + i * 60 - 2, dy + 29 + 267, GV->colLineDark);
        int startrow = off / 60;
        int endrow = startrow + 6;
        if (endrow > 15) endrow = 15;
        for (int y = startrow; y < endrow; y++)
            for (int x = 0; x < 4; x++) {
                int id = y * 4 + x;
                if (id < InventoryItemsCount) {
                    auto item = GV->editState->hInvCtrl->GetItemByIt(id);
                    cSprBankAsset *asset = GV->editState->SprBank->GetAssetByID(item.first.c_str());
                    hgeSprite* spr = GV->sprSmiley;
                    if (asset) {
                        int iframe = GV->editState->hInvCtrl->GetAnimFrame() % asset->GetSpritesCount();
                        spr = asset->GetIMGByIterator(iframe)->GetSprite();
                    }
                    spr->SetColor(isWarpNotFunctional && item.second == 32 ? 0xFFFF5555 : 0xFFFFFFFF);
                    spr->SetFlip(0, 0, 1);
                    int grdim = spr->GetWidth();
                    if (spr->GetHeight() > grdim) grdim = spr->GetHeight();
                    float fScale = 1.0f;
                    if (grdim > 48) fScale = 48.0f / float(grdim);
                    spr->RenderEx(dx + 4 + x * 60 + 30, dy + 28 + y * 60 + 30 - off, 0, fScale);
                }
            }
        hge->Gfx_SetClipping();
        int pickedid = -1;
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        if (GV->editState->conMain->getWidgetAt(mx, my) == win && mx > dx + 4 && mx > dy + 28 && mx < dx + 4 + 240 &&
            my < dy + 28 + 267) {
            mx -= dx + 4;
            my -= dy + 28;
            my += off;
            int restx = int(mx) % 60, resty = int(my) % 60;
            if (restx > 5 && restx < 53 && resty > 5 && resty < 53) {
                pickedid = int(my / 60) * 4 + int(mx / 60);
                if (pickedid < InventoryItemsCount)
                    pickedid = GV->editState->hInvCtrl->GetItemByIt(pickedid).second;
                else
                    pickedid = -1;
            }
        }
        if (pickedid != -1) {
            GV->fntMyriad16->SetColor(0xFFFFFFFF);
            GV->fntMyriad16->printf(dx + 5, dy + 28 + 267 + 5, HGETEXT_LEFT,
                                    "%s: (%s #~y~%d~w~)", 0,
                                    GETL2S("EditObj_Inventory", "Effect"),
                                    GETL2S("EditObj_Inventory", "ItemID"),
                                    pickedid);
            wchar_t tmp[32];
            wsprintfW(tmp, L"E_%d", pickedid);
            if (pickedid == 32 && isWarpNotFunctional) {
                tmp[4] = 'B';
                tmp[5] = 0;
            }
            GV->fntMyriad16->printf(dx + 5, dy + 28 + 267 + 7 + 15, HGETEXT_LEFT,
                                    "~y~%s~w~", 0,
                                    GETL2SV("EditObj_Inventory", tmp));
        }
    }

    void cInvPickbox::action(const gcn::ActionEvent &actionEvent) {

    }

    void cInvPickbox::SetVisible(bool b) {
#ifdef WAPMAP_ENABLE_IMGUI
        if (native) { nativeVisible = b; return; }
#endif
        win->setVisible(b);
    }

    void cInvPickbox::SetPosition(int x, int y) {
        win->setPosition(x, y);
#ifdef WAPMAP_ENABLE_IMGUI
        nativePosition = true;
#endif
    }

    int cInvPickbox::GetWidth() {
        return win->getWidth();
    }

    int cInvPickbox::GetHeight() {
        return win->getHeight();
    }
#ifdef WAPMAP_ENABLE_IMGUI
    void cInvPickbox::UseNative() {
        if (native) return;
        nativeVisible = win->isLocallyVisible();
        native = true;
        win->setVisible(false);
        win->setY(win->getY() - 30);
        win->setHeight(376);
    }

    void cInvPickbox::DrawImGui(int& selection) {
        if (!native || !nativeVisible) return;
        const auto title = LegacyText::ToUtf8(win->getCaption(), LegacyText::CodePage(GV->Lang->GetCode())) +
            "###InventoryPicker" + std::to_string(reinterpret_cast<std::uintptr_t>(this));
        if (nativePosition) {
            const auto* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos({viewport->Pos.x + win->getX(), viewport->Pos.y + win->getY()}, ImGuiCond_FirstUseEver);
            nativePosition = false;
        }
        ImGui::SetNextWindowSize({254, 376}, ImGuiCond_Always);
        if (WapMapImGui::BeginFixedWindow(title.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing)) {
            WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, nullptr, win->getCaption().c_str());
            int preview = selection;
            ImGui::SetCursorPos({5, 32});
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
            if (ImGui::BeginChild("items", {244, 272}, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened)) {
                const auto start = ImGui::GetCursorPos();
                ImGuiListClipper clipper;
                clipper.Begin((InventoryItemsCount + 3) / 4, 56);
                while (clipper.Step()) for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                    for (int column = 0; column < 4; ++column) {
                        const int index = row * 4 + column;
                        if (index >= InventoryItemsCount) break;
                        const auto item = GV->editState->hInvCtrl->GetItemByIt(index);
                        ImGui::PushID(index);
                        ImGui::SetCursorPos({start.x + column * 56 + 1, start.y + row * 56 + 1});
                        const auto p = ImGui::GetCursorScreenPos();
                        if (ImGui::InvisibleButton("item", {54, 54}, ImGuiButtonFlags_EnableNav)) selection = item.second;
                        const bool hovered = ImGui::IsItemHovered() || WapMapImGui::IsKeyboardFocused();
                        if (hovered) preview = item.second;
                        auto* draw = ImGui::GetWindowDrawList();
                        draw->AddRect(p, {p.x + 54, p.y + 54}, hovered || selection == item.second ? IM_COL32(23, 150, 255, 255) : IM_COL32(75, 75, 75, 255));
                        NativeInventoryUi::Image(item, {p.x + 3, p.y + 3}, 48, isWarpNotFunctional && item.second == 32);
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
                            selection = -1;
                            ImGui::SetDragDropPayload(NativeInventoryUi::Payload, &item.second, sizeof(item.second));
                            ImGui::EndDragDropSource();
                        }
                        ImGui::PopID();
                    }
                    ImGui::SetCursorPos({start.x, start.y + (row + 1) * 56});
                }
                ImGui::Dummy({1, 0});
            }
            ImGui::EndChild(); ImGui::PopStyleVar();
            if (preview >= 0) {
                const auto origin = ImGui::GetWindowPos();
                const std::string info = std::string(GETL2S("EditObj_Inventory", "Effect")) + ": (" + GETL2S("EditObj_Inventory", "ItemID") + " #" + std::to_string(preview) + ")";
                WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, {origin.x + 5, origin.y + 312}, info.c_str());
                wchar_t key[32]; wsprintfW(key, isWarpNotFunctional && preview == 32 ? L"E_%dB" : L"E_%d", preview);
                ImGui::SetCursorPos({5, 332});
                WapMapImGui::WrappedBitmapText(*hge, *GV->fntMyriad16, GETL2SV("EditObj_Inventory", key), 244);
            }
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                selection = -1;
                if (const auto* payload = ImGui::GetDragDropPayload(); payload && payload->IsDataType(NativeInventoryUi::Payload)) ImGui::ClearDragDrop();
            }
        }
        ImGui::End();
    }
#endif
}
