#include "goToCoords.h"
#include "coordinateNavigation.h"
#include "../cInterfaceSheet.h"
#include <cstdio>
#ifdef WAPMAP_ENABLE_IMGUI
#include "../imguiTheme.h"
#include "../legacyText.h"
#endif
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"

extern HGE *hge;

#define COORDS_INPUT_WORLD 0
#define COORDS_INPUT_TILES 1

winGoToCoords::winGoToCoords() : cWindow(GETL2S("Win_GoToCoords", "WinCaption"), 160, 228) {
    createCoordsInput(GETL2S("Win_GoToCoords", "WorldCoords"), COORDS_INPUT_WORLD, 0);
    createCoordsInput(GETL2S("Win_GoToCoords", "TileCoords"), COORDS_INPUT_TILES, 85);

    butGo = new SHR::But(GV->hGfxInterface, GETL2S("Win_GoToCoords", "Go"));
    butGo->setDimension(gcn::Rectangle(0, 0, 60, 25));
    butGo->addActionListener(this);
    butGo->setEnabled(true);
    myWin.add(butGo, 90, 178);
}

void winGoToCoords::createCoordsInput(const char* label, int i, int offsetY) {
    const bool isFirst = i == COORDS_INPUT_WORLD;

    char tmp[32];
    sprintf(tmp, "%p", this);
    rbCoords[i] = new SHR::RadBut(GV->hGfxInterface, label, tmp, isFirst);
    rbCoords[i]->adjustSize();
    rbCoords[i]->addActionListener(this);
    myWin.add(rbCoords[i], 10, 15 + offsetY);

    labX[i] = new SHR::Lab("X:");
    labX[i]->adjustSize();
    myWin.add(labX[i], 10, 40 + offsetY);

    tfX[i] = new SHR::TextField("0");
    tfX[i]->setDimension(gcn::Rectangle(0, 0, 120, 20));
    tfX[i]->SetNumerical(1, 0);
    tfX[i]->addActionListener(this);
    tfX[i]->setEnabled(isFirst);
    myWin.add(tfX[i], 25, 40 + offsetY);

    labY[i] = new SHR::Lab("Y:");
    labY[i]->adjustSize();
    myWin.add(labY[i], 10, 65 + offsetY);

    tfY[i] = new SHR::TextField("0");
    tfY[i]->setDimension(gcn::Rectangle(0, 0, 120, 20));
    tfY[i]->SetNumerical(1, 0);
    tfY[i]->addActionListener(this);
    tfY[i]->setEnabled(isFirst);
    myWin.add(tfY[i], 25, 65 + offsetY);

}

void winGoToCoords::Open() {
    if (!GV->editState->hParser) return;
    setCurCamPos();
#ifdef WAPMAP_ENABLE_IMGUI
    syncImGuiFields();
    myWin.setVisible(false);
    imguiOpen = imguiPositionOnOpen = true;
    return;
#endif
    myWin.setPosition(160, LAY_VIEWPORT_Y + 40);
    myWin.setVisible(true);
    myWin.getParent()->moveToTop(&myWin);
}

void winGoToCoords::action(const ActionEvent &actionEvent) {
    bool worldCoords = rbCoords[COORDS_INPUT_WORLD]->isSelected();

    tfX[COORDS_INPUT_WORLD]->setEnabled(worldCoords);
    tfY[COORDS_INPUT_WORLD]->setEnabled(worldCoords);

    tfX[COORDS_INPUT_TILES]->setEnabled(!worldCoords);
    tfY[COORDS_INPUT_TILES]->setEnabled(!worldCoords);

    if (actionEvent.getSource() == butGo)
        goToCoords();
}

void winGoToCoords::setCurCamPos() {
    auto *editState = GV->editState;
    int currentX = editState->vPort->GetWidth() / 2 / editState->fZoom + editState->fCamX;
    int currentY = editState->vPort->GetHeight() / 2 / editState->fZoom + editState->fCamY;

    char temp[32];

    sprintf(temp, "%d", currentX);
    tfX[COORDS_INPUT_WORLD]->setText(temp);
    sprintf(temp, "%d", currentY);
    tfY[COORDS_INPUT_WORLD]->setText(temp);

    WWD::Plane *mainPlane = editState->hParser->GetMainPlane();
    int tileWidth = mainPlane->GetTileWidth(), tileHeight = mainPlane->GetTileHeight();

    sprintf(temp, "%d", (int)(currentX/tileWidth));
    tfX[COORDS_INPUT_TILES]->setText(temp);
    sprintf(temp, "%d", (int)(currentY/tileHeight));
    tfY[COORDS_INPUT_TILES]->setText(temp);
}

void winGoToCoords::goToCoords() {
    const int mode = rbCoords[COORDS_INPUT_WORLD]->isSelected() ? COORDS_INPUT_WORLD : COORDS_INPUT_TILES;
    navigateToCoords(tfX[mode]->getText().c_str(), tfY[mode]->getText().c_str(), mode == COORDS_INPUT_WORLD);
}

bool winGoToCoords::navigateToCoords(const char* x, const char* y, bool world) {
    auto* editState = GV->editState;
    if (!editState->hParser || !editState->_isOnTop()) return false;
    auto* mainPlane = editState->hParser->GetMainPlane();
    const int tileWidth = mainPlane->GetTileWidth(), tileHeight = mainPlane->GetTileHeight();
    const auto point = CoordinateNavigation::Resolve(x, y, world, tileWidth, tileHeight);
    if (!point) return false;
    const int pickedX = point->x, pickedY = point->y;

    editState->NavigateToPoint(pickedX, pickedY);

    char temp[32];

    sprintf(temp, "%d", pickedX);
    tfX[COORDS_INPUT_WORLD]->setText(temp);
    sprintf(temp, "%d", pickedY);
    tfY[COORDS_INPUT_WORLD]->setText(temp);

    sprintf(temp, "%d", (int)(pickedX/tileWidth));
    tfX[COORDS_INPUT_TILES]->setText(temp);
    sprintf(temp, "%d", (int)(pickedY/tileHeight));
    tfY[COORDS_INPUT_TILES]->setText(temp);
    return true;
}

void winGoToCoords::OnDocumentChange() {
    if (!GV->editState->hParser) {
        Close();
    } else if (isOpened()) {
        setCurCamPos();
#ifdef WAPMAP_ENABLE_IMGUI
        syncImGuiFields();
#endif
    }
}

#ifdef WAPMAP_ENABLE_IMGUI
void winGoToCoords::Close() {
    imguiOpen = false;
    cWindow::Close();
}

void winGoToCoords::syncImGuiFields() {
    for (int mode = 0; mode < 2; ++mode) {
        std::snprintf(imguiCoords[mode][0], sizeof(imguiCoords[mode][0]), "%s", tfX[mode]->getText().c_str());
        std::snprintf(imguiCoords[mode][1], sizeof(imguiCoords[mode][1]), "%s", tfY[mode]->getText().c_str());
    }
}

void winGoToCoords::DrawImGui() {
    if (!imguiOpen) return;
    if (!GV->editState->hParser) { Close(); return; }
    if (imguiPositionOnOpen) {
        auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + 160.f, viewport->Pos.y + LAY_VIEWPORT_Y + 40.f),
                                ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowFocus();
        imguiPositionOnOpen = false;
    }
    ImGui::SetNextWindowSize(ImVec2(160.f, 228.f), ImGuiCond_Always);
    const auto label = LegacyText::ToUtf8(GETL2S("Win_GoToCoords", "WinCaption"),
        LegacyText::CodePage(GV->Lang->GetCode())) + "###WapMapGoToCoords";
    if (WapMapImGui::BeginFixedWindow(label.c_str(), &imguiOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton,
                                 GETL2S("Win_GoToCoords", "WinCaption"));
        const bool canNavigate = GV->editState->_isOnTop();
        ImGui::BeginDisabled(!canNavigate);
        const auto children = myWin.getChildrenArea();
        const float verticalPadding = std::max(0.f, (20.f - GV->fntMyriad16->GetHeight()) * 0.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.f, verticalPadding));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, WapMapImGui::Grey(0x23));
        bool coordinateSubmitted = false;
        for (int mode = 0; mode < 2; ++mode) {
            ImGui::SetCursorPos(ImVec2(children.x + 10.f, children.y + 15.f + mode * 85.f));
            ImGui::PushID(mode);
            WapMapImGui::BitmapRadio(*hge, *GV->fntMyriad16, *GV->hGfxInterface,
                mode == 0 ? GETL2S("Win_GoToCoords", "WorldCoords") : GETL2S("Win_GoToCoords", "TileCoords"),
                imguiCoordsMode, mode);
            ImGui::BeginDisabled(imguiCoordsMode != mode);
            for (int axis = 0; axis < 2; ++axis) {
                ImGui::PushID(axis);
                ImGui::SetCursorPos(ImVec2(children.x + 10.f, children.y + 40.f + mode * 85.f + axis * 25.f));
                WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16,
                    ImGui::GetCursorScreenPos(), axis == 0 ? "X:" : "Y:",
                    IM_COL32(225, 225, 225, static_cast<int>(255.f * ImGui::GetStyle().Alpha)));
                ImGui::SetCursorPos(ImVec2(children.x + 25.f, children.y + 40.f + mode * 85.f + axis * 25.f));
                ImGui::SetNextItemWidth(120.f);
                coordinateSubmitted |= ImGui::InputText("##coordinate", imguiCoords[mode][axis], sizeof(imguiCoords[mode][axis]),
                                 ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll |
                                 ImGuiInputTextFlags_EnterReturnsTrue);
                ImGui::PopID();
            }
            ImGui::EndDisabled();
            ImGui::PopID();

        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);
        auto* plane = GV->editState->hParser->GetMainPlane();
        const bool valid = CoordinateNavigation::Resolve(imguiCoords[imguiCoordsMode][0],
            imguiCoords[imguiCoordsMode][1], imguiCoordsMode == 0,
            plane->GetTileWidth(), plane->GetTileHeight()).has_value();
        ImGui::BeginDisabled(!valid);
        ImGui::SetCursorPos(ImVec2(children.x + 90.f, children.y + 178.f));
        const bool go = WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface,
            GETL2S("Win_GoToCoords", "Go"), ImVec2(60.f, 25.f));
        ImGui::EndDisabled();
        if (valid && canNavigate && (go || coordinateSubmitted)) {
            if (navigateToCoords(imguiCoords[imguiCoordsMode][0], imguiCoords[imguiCoordsMode][1], imguiCoordsMode == 0))
                syncImGuiFields();
        }
        ImGui::EndDisabled();
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape))
            imguiOpen = false;
    }
    ImGui::End();
    if (!imguiOpen) cWindow::Close();
}
#endif
