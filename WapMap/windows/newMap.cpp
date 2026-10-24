#include "newMap.h"
#include "../langID.h"
#include "../states/dialog.h"
#include "../version.h"
#include "../states/editing_ww.h"
#include "../states/loadmap.h"
#include "../shared/cSFS.h"
#include "planeGeometry.h"
#include "coordinateNavigation.h"
#include <cstdio>
#ifdef WAPMAP_ENABLE_IMGUI
#include "../imguiTheme.h"
#include "../legacyText.h"
#endif

winNewMap::winNewMap()
    : cWindow(GETL2S("NewMap", "NewMap"), 420, 550),
      labName(std::string(GETL2S("NewMap", "MapName")) + ':'),
      labAuthor(std::string(GETL2S("NewMap", "MapAuthor")) + ':'),
      labPlaneSize(std::string(GETL2S("NewMap", "MainPlaneSize")) + ':'),
      labTarget(std::string(GETL2S("NewMap", "TargetGame")) + ':'),
      labTargetValue("-"),
      tfName(), tfAuthor(), tfPlaneWidth("300"), labX("x"), tfPlaneHeight("100"),
      targetGameTree(), scrollArea(&targetGameTree),
      cbAddBonusPlanes(GV->hGfxInterface, GETL2S("NewMap", "AddBackFrontPlanes"), true),
      butOK(GV->hGfxInterface, GETL2S("NewMap", "Create"))
{
    labName.adjustSize();
    labAuthor.adjustSize();
    labPlaneSize.adjustSize();

    int labsWidthMax = std::max(std::max(labName.getWidth(), labAuthor.getWidth()), labPlaneSize.getWidth()) + 20,
        tfWidth = 380 - labsWidthMax, x = 20, y = 25;

    myWin.add(&labName, x, y);

    tfName.setDimension(gcn::Rectangle(0, 0, tfWidth, 20));
    tfName.addActionListener(this);
    tfName.setMarkedInvalid(true);
    tfName.setMaxLength(58);
    myWin.add(&tfName, x + labsWidthMax, y - 1);

    y += 25;

    myWin.add(&labAuthor, x, y);

    tfAuthor.setDimension(gcn::Rectangle(0, 0, tfWidth, 20));
    tfAuthor.addActionListener(this);
    tfAuthor.setMarkedInvalid(true);
    tfAuthor.setMaxLength(63);
    myWin.add(&tfAuthor, x + labsWidthMax, y - 1);

    y += 25;

    myWin.add(&labPlaneSize, x, y);

    int subW = (tfWidth - 20) / 2;

    tfPlaneWidth.SetNumerical(1, 0);
    tfPlaneWidth.setMaxLength(5);
    tfPlaneWidth.setDimension(gcn::Rectangle(0, 0, subW, 20));
    myWin.add(&tfPlaneWidth, x + labsWidthMax, y - 1);

    labX.adjustSize();
    myWin.add(&labX, x + labsWidthMax + subW + 7, y);

    tfPlaneHeight.SetNumerical(1, 0);
    tfPlaneHeight.setMaxLength(5);
    tfPlaneHeight.setDimension(gcn::Rectangle(0, 0, subW, 20));
    myWin.add(&tfPlaneHeight, x + labsWidthMax + tfWidth - subW, y - 1);

    y += 30;

    for (WWD::GAME game = WWD::GAME::Games_First; game <= WWD::GAME::Games_Last; ++game) {
        auto folder = std::make_shared<SHR::TreeFolder>(WWD::GAME_NAMES[game], GV->sprGamesSmall[game]);

        int levels = 0;
        bool separateLevelIcons = false;
        switch (game) {
            case WWD::Game_Claw:
                levels = 14;
                separateLevelIcons = true;
            break;
            case WWD::Game_Gruntz:
                separateLevelIcons = true;
            case WWD::Game_GetMedieval:
                levels = 8;
            break;
            case WWD::Game_Claw2:
                levels = 1;
            break;
        }

        if (levels) {
            if (separateLevelIcons) {
                wchar_t tmp[16];
                MultiByteToWideChar(CP_UTF8, 0, WWD::GAME_NAMES[game], -1, tmp, 16);
                wchar_t* end = tmp + strlen(WWD::GAME_NAMES[game]);
                for (int i = 1; i <= levels; ++i) {
                    wsprintfW(end, L"_%d", i);
                    folder->AddElement(std::make_shared<SHR::TreeElement>(GETL2SV("NewMap", tmp),
                            GV->sprLevelsMicro16[game - WWD::Games_First][i - 1]));
                }
            } else {
                char tmp[16]; char* num = NULL;
                strcpy(tmp, GETL2S("NewMap", "Area"));
                num = tmp + strlen(tmp);
                for (int i = 1; i <= levels; ++i) {
                    sprintf(num, " %d", i);
                    folder->AddElement(std::make_shared<SHR::TreeElement>(tmp, GV->sprGamesSmall[game]));
                }
            }
        }

        targetGameTree.AddElement(folder);
        if (!GV->gamePaths[game].empty()) {
            folder->SetOpened(true);
        }
    }

    targetGameTree.addActionListener(this);
    targetGameTree.adjustSize();
    scrollArea.setWidth(380);
    scrollArea.setHeight(290);

    myWin.add(&scrollArea, x, y);

    y += 300;

    myWin.add(&labTarget, x, y);
    myWin.add(&labTargetValue, x + labTarget.getWidth() + 5, y);

    y += 30;

    cbAddBonusPlanes.adjustSize();
    cbAddBonusPlanes.setVisible(false);
    myWin.add(&cbAddBonusPlanes, x, y);
    
    butOK.setDimension(gcn::Rectangle(0, 0, 100, 33));
    butOK.addActionListener(this);
    butOK.setEnabled(false);
    myWin.add(&butOK, 300, 480);
}

void winNewMap::Draw(int piCode) {

}

void winNewMap::Think() {
#ifdef WAPMAP_ENABLE_IMGUI
    if (!pendingCreate || !nativeOpen || !GV->editState->_isOnTop()) return;
    pendingCreate = false;
    action(gcn::ActionEvent(&butOK, ""));
#endif
}

void winNewMap::action(const ActionEvent &actionEvent) {
    auto element = targetGameTree.GetSelectedElement();

    if (actionEvent.getSource() == &targetGameTree) {
        std::string newCaption("-");
        if (element) {
            newCaption = std::string(element->GetParent()->GetName()) + " - " + element->GetName();
            while (GV->fntMyriad16->GetStringWidth(newCaption.c_str()) > 380 - labTarget.getWidth()) {
                newCaption.resize(newCaption.size() - 5);
                newCaption += "...";
            }
        }
        labTargetValue.setCaption(newCaption);
        labTargetValue.adjustSize();

        cbAddBonusPlanes.setVisible(element && element->GetRoot()->GetIndexOf(element->GetParent()) == WWD::Game_Claw - WWD::Games_First);
    } else if (actionEvent.getSource() == &butOK) {
        if (!element || element->IsDir() || tfName.getText().empty() || tfAuthor.getText().empty()) return;
        const auto width = CoordinateNavigation::Parse(tfPlaneWidth.getText().c_str());
        const auto height = CoordinateNavigation::Parse(tfPlaneHeight.getText().c_str());
        if (!width || !height || *width < 50 || *height < 50 || *width > 99999 || *height > 99999 ||
            !PlaneArithmeticFits(*width, *height, 64, 64)) {
            GV->StateMgr->Push(
                    new State::Dialog(PRODUCT_NAME, GETL2S("NewMap", "InvalidPlaneSize"), ST_DIALOG_ICON_ERROR, ST_DIALOG_BUT_OK));
            return;
        }
        const int plw = *width, plh = *height;

        int game = element->GetRoot()->GetIndexOf(element->GetParent()) + WWD::Games_First;
        int baseLevel = element->GetParent()->GetIndexOf(element) + 1;
        char name[64], author[64];
        char *fixname = GV->editState->FixLevelName(baseLevel, tfName.getText().c_str());
        strcpy(name, fixname);
        delete[] fixname;
        strcpy(author, tfAuthor.getText().c_str());

        if (game == WWD::Game_Claw) {
            int size;
            char tmp[10];
            sprintf(tmp, "lvl%d.wms", baseLevel);
            cSFS_Repository repo("data.sfs");
            char *ptr = repo.GetFileAsRawData(tmp, &size);
            GV->StateMgr->Push(
                    new State::LoadMap(ptr, size, cbAddBonusPlanes.isSelected(), plw, plh, name, author)
            );
        } else {
            auto* parser = new WWD::Parser((WWD::GAME)game, baseLevel, plw, plh);
            parser->SetName(name);
            parser->SetAuthor(author);
            GV->StateMgr->Push(new State::LoadMap(parser));
        }
        Close();
        return;
    }

    butOK.setEnabled(element && !tfName.getText().empty() && !tfAuthor.getText().empty()
        && !tfPlaneWidth.getText().empty() && !tfPlaneHeight.getText().empty());
}

void winNewMap::Open() {
    tfName.setText("");
    tfAuthor.setText("");
    tfPlaneWidth.setText("300");
    tfPlaneHeight.setText("100");
    targetGameTree.SetSelectedElement(NULL);
    scrollArea.setVerticalScrollAmount(0);
    cbAddBonusPlanes.setSelected(true);
#ifdef WAPMAP_ENABLE_IMGUI
    fields = {};
    std::snprintf(fields[2].data(), fields[2].size(), "300");
    std::snprintf(fields[3].data(), fields[3].size(), "100");
    nativeOpen = positionOnOpen = resetTreeScroll = true;
    pendingCreate = false;
    labTargetValue.setCaption("-");
    labTargetValue.adjustSize();
    cbAddBonusPlanes.setVisible(false);
    myWin.setVisible(false);
#else
    cWindow::Open();
#endif
}

void winNewMap::Close() {
#ifdef WAPMAP_ENABLE_IMGUI
    nativeOpen = pendingCreate = false;
#endif
    cWindow::Close();
}

#ifdef WAPMAP_ENABLE_IMGUI
void winNewMap::DrawImGui() {
    if (!nativeOpen || !GV->editState->_isOnTop()) return;
    if (positionOnOpen) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, {0.5f, 0.5f});
        ImGui::SetNextWindowFocus(); positionOnOpen = false;
    }
    ImGui::SetNextWindowSize({420.f, 550.f}, ImGuiCond_Always);
    const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
    const auto title = LegacyText::ToUtf8(GETL2S("NewMap", "NewMap"), cp) + "###WapMapNewMap";
    bool visible = true, create = false;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar, false)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, GETL2S("NewMap", "NewMap"));
        const auto origin = ImGui::GetWindowPos(); const auto area = myWin.getChildrenArea();
        auto* draw = ImGui::GetWindowDrawList();
        const auto position = [&](float x, float y) { ImGui::SetCursorPos({area.x + x, area.y + y}); };
        const auto label = [&](SHR::Lab& widget) {
            WapMapImGui::BitmapText(draw, *hge, *GV->fntMyriad16,
                {origin.x + area.x + widget.getX(), origin.y + area.y + widget.getY()}, widget.getCaption().c_str());
        };
        for (auto* lab : {&labName, &labAuthor, &labPlaneSize, &labX, &labTarget, &labTargetValue}) label(*lab);
        SHR::TextField* targets[] = {&tfName, &tfAuthor, &tfPlaneWidth, &tfPlaneHeight};
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {5.f, std::max(0.f, (20.f - GV->fntMyriad16->GetHeight()) / 2)});
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, WapMapImGui::Grey(0x23));
        for (int i = 0; i < 4; ++i) {
            ImGui::PushID(i); position(static_cast<float>(targets[i]->getX()), static_cast<float>(targets[i]->getY()));
            ImGui::SetNextItemWidth(static_cast<float>(targets[i]->getWidth()));
            ImGui::InputText("##field", fields[i].data(), i < 2 ? fields[i].size() : 6,
                i < 2 ? ImGuiInputTextFlags_None : ImGuiInputTextFlags_CharsDecimal);
            if (i < 2 && !LegacyText::FromUtf8(fields[i].data(), cp, i == 0 ? 58 : 63)) {
                draw->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(220, 75, 75, 255));
                if (ImGui::IsItemHovered()) {
                    const auto message = LegacyText::ToUtf8(GETL2S("MapProperties", "InvalidText"), cp);
                    ImGui::SetTooltip("%s (%d)", message.c_str(), i == 0 ? 58 : 63);
                }
            }
            ImGui::PopID();
        }
        ImGui::PopStyleColor(); ImGui::PopStyleVar(3);
        position(static_cast<float>(scrollArea.getX()), static_cast<float>(scrollArea.getY()));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, WapMapImGui::Grey(0x13));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.f, 0.f});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.f, 0.f});
        if (ImGui::BeginChild("games", {380.f, 290.f})) {
            if (resetTreeScroll) { ImGui::SetScrollY(0); resetTreeScroll = false; }
            ImGui::SetCursorPos({0, 2});
            for (int game = 0; game < targetGameTree.GetElementsCount(); ++game) {
                auto folder = std::static_pointer_cast<SHR::TreeFolder>(targetGameTree.GetElement(game));
                ImGui::PushID(game);
                auto p = ImGui::GetCursorScreenPos();
                if (ImGui::InvisibleButton("##folder", {ImGui::GetContentRegionAvail().x, 20.f}, ImGuiButtonFlags_EnableNav))
                    folder->SetOpened(!folder->IsOpened());
                if (ImGui::IsItemFocused()) {
                    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) folder->SetOpened(true);
                    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) folder->SetOpened(false);
                }
                auto* treeDraw = ImGui::GetWindowDrawList();
                if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
                    treeDraw->AddRectFilled(p, {p.x + ImGui::GetContentRegionAvail().x, p.y + 20}, IM_COL32(255, 255, 255, 34));
                const auto white = IM_COL32(225, 225, 225, 255);
                treeDraw->AddRect({p.x + 7, p.y + 5}, {p.x + 17, p.y + 15}, white);
                treeDraw->AddLine({p.x + 8, p.y + 10}, {p.x + 15, p.y + 10}, white);
                if (!folder->IsOpened()) treeDraw->AddLine({p.x + 12, p.y + 6}, {p.x + 12, p.y + 13}, white);
                WapMapImGui::Sprite(treeDraw, *hge, folder->GetIcon(), {p.x + 24, p.y + 2});
                WapMapImGui::BitmapText(treeDraw, *hge, *GV->fntMyriad16, {p.x + 44, p.y + 3}, folder->GetName());
                if (folder->IsOpened()) {
                    const float connectorX = p.x + 12;
                    const float connectorTop = p.y + 15;
                    treeDraw->AddLine({connectorX, connectorTop}, {connectorX, p.y + 20 * (folder->GetElementsCount() + 0.5f)}, white);
                    for (int level = 0; level < folder->GetElementsCount(); ++level) {
                        ImGui::PushID(level); auto entry = folder->GetElement(level);
                        p = ImGui::GetCursorScreenPos();
                        if (ImGui::InvisibleButton("##level", {ImGui::GetContentRegionAvail().x, 20.f}, ImGuiButtonFlags_EnableNav)) {
                            targetGameTree.SetSelectedElement(entry.get());
                        }
                        if (targetGameTree.GetSelectedElement() == entry.get() || ImGui::IsItemHovered() || ImGui::IsItemFocused())
                            treeDraw->AddRectFilled({p.x + 42, p.y}, {p.x + ImGui::GetContentRegionAvail().x, p.y + 20},
                                targetGameTree.GetSelectedElement() == entry.get() ? IM_COL32(62, 62, 62, 255) : IM_COL32(255, 255, 255, 34));
                        treeDraw->AddLine({connectorX, p.y + 10}, {p.x + 40, p.y + 10}, white);
                        WapMapImGui::Sprite(treeDraw, *hge, entry->GetIcon(), {p.x + 44, p.y + 2});
                        WapMapImGui::BitmapText(treeDraw, *hge, *GV->fntMyriad16, {p.x + 64, p.y + 3}, entry->GetName());
                        ImGui::PopID();
                    }
                }
                ImGui::PopID();
            }
        }
        ImGui::EndChild(); ImGui::PopStyleVar(2); ImGui::PopStyleColor();
        if (cbAddBonusPlanes.isLocallyVisible()) {
            position(static_cast<float>(cbAddBonusPlanes.getX()), static_cast<float>(cbAddBonusPlanes.getY()));
            bool bonus = cbAddBonusPlanes.isSelected();
            WapMapImGui::BitmapCheckbox(*hge, *GV->fntMyriad16, *GV->hGfxInterface, cbAddBonusPlanes.getCaption().c_str(), bonus);
            cbAddBonusPlanes.setSelected(bonus);
        }
        const auto name = LegacyText::FromUtf8(fields[0].data(), cp, 58), author = LegacyText::FromUtf8(fields[1].data(), cp, 63);
        const auto width = CoordinateNavigation::Parse(fields[2].data()), height = CoordinateNavigation::Parse(fields[3].data());
        const bool valid = name && author && !name->empty() && !author->empty() && targetGameTree.GetSelectedElement() &&
            width && height && *width > 0 && *height > 0 && PlaneArithmeticFits(*width, *height, 64, 64);
        ImGui::BeginDisabled(!valid || pendingCreate);
        position(static_cast<float>(butOK.getX()), static_cast<float>(butOK.getY()));
        create = WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, butOK.getCaption().c_str(), {100, 33});
        ImGui::EndDisabled();
        if (create) {
            tfName.setText(*name); tfAuthor.setText(*author);
            tfPlaneWidth.setText(fields[2].data()); tfPlaneHeight.setText(fields[3].data());
        }
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) visible = false;
    }
    ImGui::End();
    if (!visible) Close(); else if (create) pendingCreate = true;
}
#endif
