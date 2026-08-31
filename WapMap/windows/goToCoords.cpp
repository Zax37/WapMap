#include "goToCoords.h"
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
    static bool isFirst = true;

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

    isFirst = false;
}

void winGoToCoords::Open() {
    setCurCamPos();
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
    int currentX = editState->vPort->GetWidth() / 2.0 / editState->fZoom + editState->fCamX;
    int currentY = editState->vPort->GetHeight() / 2.0 / editState->fZoom + editState->fCamY;

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
    bool worldCoords = rbCoords[COORDS_INPUT_WORLD]->isSelected();
    auto *editState = GV->editState;

    WWD::Plane *mainPlane = editState->hParser->GetMainPlane();
    int tileWidth = mainPlane->GetTileWidth(), tileHeight = mainPlane->GetTileHeight();

    int pickedX, pickedY;
    if (!worldCoords) {
        std::string strX = tfX[COORDS_INPUT_TILES]->getText();
        if (strX.empty())
            strX = "0";
        std::string strY = tfY[COORDS_INPUT_TILES]->getText();
        if (strY.empty())
            strY = "0";
        pickedX = std::stoi(strX.c_str()) * tileWidth + tileWidth/2;
        pickedY = std::stoi(strY.c_str()) * tileHeight + tileHeight/2;

    } else {
        std::string strX = tfX[COORDS_INPUT_WORLD]->getText();
        if (strX.empty())
            strX = "0";
        std::string strY = tfY[COORDS_INPUT_WORLD]->getText();
        if (strY.empty())
            strY = "0";
        pickedX = std::stoi(strX.c_str());
        pickedY = std::stoi(strY.c_str());
    }
    
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
}
