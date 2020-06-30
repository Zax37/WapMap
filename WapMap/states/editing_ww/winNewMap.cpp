#include "../editing_ww.h"
#include "../../globals.h"
#include "../dialog.h"
#include "../../../shared/commonFunc.h"
#include "../../langID.h"
#include "../loadmap.h"
#include "../../version.h"

extern HGE *hge;

void State::EditingWW::NewMap_Open() {
    if (NewMap_data != 0) {
        conMain->moveToTop(NewMap_data->winNewMap);
        return;
    }
    NewMap_data = new win_NewMap;
    NewMap_data->winNewMap = new SHR::Win(&GV->gcnParts, GETL2S("NewMap", "NewMap"));
    NewMap_data->winNewMap->setDimension(gcn::Rectangle(0, 0, 650, 585));
    NewMap_data->winNewMap->setClose(1);
    NewMap_data->winNewMap->addActionListener(mainListener);
    NewMap_data->winNewMap->setVisible(0);
    NewMap_data->winNewMap->setVisible(1);
    conMain->add(NewMap_data->winNewMap, hge->System_GetState(HGE_SCREENWIDTH) / 2 - 325,
                 hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 292);

    NewMap_data->labSelectBaseLevel = new SHR::Lab(GETL2S("NewMap", "SelectBaseLvl"));
    NewMap_data->labSelectBaseLevel->adjustSize();
    NewMap_data->winNewMap->add(NewMap_data->labSelectBaseLevel, 25, 65);

#ifdef WM_EXT_TILESETS
    NewMap_data->sliBaseLvls = new SHR::Slider(1050 - 440 + 150);
    NewMap_data->sliBaseLvls->setValue(1050 - 440 + 150);
#else
    NewMap_data->sliBaseLvls = new SHR::Slider(1050 - 440);
    NewMap_data->sliBaseLvls->setValue(1050 - 440);
#endif
    NewMap_data->sliBaseLvls->setOrientation(SHR::Slider::VERTICAL);
    NewMap_data->sliBaseLvls->setDimension(gcn::Rectangle(0, 0, 11, 441));
	NewMap_data->sliBaseLvls->setMarkerLength(50);
    NewMap_data->winNewMap->add(NewMap_data->sliBaseLvls, 620, 84);

    NewMap_data->labName = new SHR::Lab(GETL2S("NewMap", "MapName"));
    NewMap_data->labName->adjustSize();
	NewMap_data->winNewMap->add(NewMap_data->labName, 25, 15);

    NewMap_data->labAuthor = new SHR::Lab(GETL2S("NewMap", "MapAuthor"));
    NewMap_data->labAuthor->adjustSize();
    NewMap_data->winNewMap->add(NewMap_data->labAuthor, 325, 15);

    NewMap_data->tfName = new SHR::TextField();
    NewMap_data->tfName->setDimension(gcn::Rectangle(0, 0, 275 - NewMap_data->labName->getWidth(), 20));
    NewMap_data->tfName->addActionListener(mainListener);
    NewMap_data->tfName->setMarkedInvalid(1);
    NewMap_data->winNewMap->add(NewMap_data->tfName, 35 + NewMap_data->labName->getWidth(), 15);

    NewMap_data->tfAuthor = new SHR::TextField();
    NewMap_data->tfAuthor->setDimension(gcn::Rectangle(0, 0, 275 - NewMap_data->labAuthor->getWidth(), 20));
    NewMap_data->tfAuthor->addActionListener(mainListener);
    NewMap_data->tfAuthor->setMarkedInvalid(1);
    NewMap_data->winNewMap->add(NewMap_data->tfAuthor, 335 + NewMap_data->labAuthor->getWidth(), 15);

    NewMap_data->cbAddBonusPlanes = new SHR::CBox(GV->hGfxInterface, GETL2S("NewMap", "AddBackFrontPlanes"), true);
    NewMap_data->cbAddBonusPlanes->adjustSize();
    NewMap_data->winNewMap->add(NewMap_data->cbAddBonusPlanes, 25, 40);

    NewMap_data->labPlaneSize = new SHR::Lab(GETL2S("NewMap", "MainPlaneSize"));
    NewMap_data->labPlaneSize->adjustSize();
    NewMap_data->winNewMap->add(NewMap_data->labPlaneSize, 325, 40);

    NewMap_data->tfPlaneWidth = new SHR::TextField("300");
    NewMap_data->tfPlaneWidth->SetNumerical(1);
    NewMap_data->tfPlaneWidth->setDimension(gcn::Rectangle(0, 0, 50, 20));
    NewMap_data->winNewMap->add(NewMap_data->tfPlaneWidth, 330 + NewMap_data->labPlaneSize->getWidth(), 40);

    NewMap_data->labX = new SHR::Lab("x");
    NewMap_data->labX->adjustSize();
    NewMap_data->winNewMap->add(NewMap_data->labX, 330 + NewMap_data->labPlaneSize->getWidth() + 55, 40);

    NewMap_data->tfPlaneHeight = new SHR::TextField("100");
    NewMap_data->tfPlaneHeight->SetNumerical(1);
    NewMap_data->tfPlaneHeight->setDimension(gcn::Rectangle(0, 0, 50, 20));
    NewMap_data->winNewMap->add(NewMap_data->tfPlaneHeight, 330 + NewMap_data->labPlaneSize->getWidth() + 65, 40);

    NewMap_data->butOK = new SHR::But(GV->hGfxInterface, "OK");
    NewMap_data->butOK->setDimension(gcn::Rectangle(0, 0, 100, 33));
    NewMap_data->butOK->addActionListener(mainListener);
    NewMap_data->butOK->setEnabled(0);
    NewMap_data->winNewMap->add(NewMap_data->butOK, 275, 530);

    NewMap_data->vp = new WIDG::Viewport(vp, VP_NEWMAP);
    NewMap_data->winNewMap->add(NewMap_data->vp);
    for (int i = 0; i < 14; i++)
        NewMap_data->fBaseTimer[i] = 0;

    NewMap_data->iSelectedBase = -1;
    NewMap_data->bKill = 0;
}

void State::EditingWW::NewMap_Close() {
    delete NewMap_data->vp;
    delete NewMap_data->labSelectBaseLevel;
    delete NewMap_data->labAuthor;
    delete NewMap_data->labName;
    delete NewMap_data->labPlaneSize;
    delete NewMap_data->labX;
    delete NewMap_data->sliBaseLvls;
    delete NewMap_data->tfName;
    delete NewMap_data->tfAuthor;
    delete NewMap_data->tfPlaneWidth;
    delete NewMap_data->tfPlaneHeight;
    delete NewMap_data->cbAddBonusPlanes;
    delete NewMap_data->butOK;
    delete NewMap_data->winNewMap;
    delete NewMap_data;
    NewMap_data = 0;
}

void State::EditingWW::NewMap_Think() {
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    bool bMouseOn = (conMain->getWidgetAt(mx, my) == NewMap_data->winNewMap);
    int wx, wy;
    NewMap_data->winNewMap->getAbsolutePosition(wx, wy);
    hge->Gfx_SetClipping(wx + 27, wy + 101, 600, 440);
    int slival = NewMap_data->sliBaseLvls->getScaleEnd() - NewMap_data->sliBaseLvls->getValue();
    int starty = slival / 150;
    if (mx > wx + 27 && mx < wx + 627 && my > wy + 101 && my < wy + 540 && bMouseOn)
        NewMap_data->sliBaseLvls->setValue(NewMap_data->sliBaseLvls->getValue() +
                                           NewMap_data->sliBaseLvls->getStepLength() * hge->Input_GetMouseWheel());
#ifdef WM_EXT_TILESETS
    for (int y = starty; y < starty + 4 && y < 8; y++)
#else
    for (int y = starty; y < starty + 4 && y < 7; y++)
#endif
            for (int x = 0; x < 2; x++) {
                int dx = wx + 27 + x * 300,
                        dy = wy + 101 + (y * 150) - slival;
                int rval = y * 2 + x;
                if (rval > 13) rval += 2;
                GV->sprLevels[rval]->Render(dx, dy);
                if (my > wy + 101 && my < wy + 541 && bMouseOn &&
                    mx > dx && mx < dx + 300 && my > dy && my < dy + 150) {
                    if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                        NewMap_data->iSelectedBase = y * 2 + x;
                        NewMap_Validate();
                    }
                    NewMap_data->fBaseTimer[y * 2 + x] += hge->Timer_GetDelta() * 3;
                    if (NewMap_data->fBaseTimer[y * 2 + x] > 1) NewMap_data->fBaseTimer[y * 2 + x] = 1;
                } else if (NewMap_data->fBaseTimer[y * 2 + x] > 0) {
                    NewMap_data->fBaseTimer[y * 2 + x] -= hge->Timer_GetDelta() * 2;
                    if (NewMap_data->fBaseTimer[y * 2 + x] < 0) NewMap_data->fBaseTimer[y * 2 + x] = 0;
                }
                if (NewMap_data->iSelectedBase == y * 2 + x) {
                    NewMap_data->fBaseTimer[y * 2 + x] = 1;
                    hge->Gfx_RenderLine(dx, dy + 1, dx + 300, dy + 1, TILE_PICK_COLOR);
                    hge->Gfx_RenderLine(dx, dy + 150, dx + 300, dy + 150, TILE_PICK_COLOR);
                    hge->Gfx_RenderLine(dx + 1, dy, dx + 1, dy + 150, TILE_PICK_COLOR);
                    hge->Gfx_RenderLine(dx + 300, dy, dx + 300, dy + 150, TILE_PICK_COLOR);
                }
                if (NewMap_data->fBaseTimer[y * 2 + x] > 0) {
                    char tmp[16];
                    sprintf(tmp, "Baselvl_%d", y * 2 + x + 1);
                    GV->fntMyriad16->SetColor(ARGB(int(255.0f * NewMap_data->fBaseTimer[y * 2 + x]), 0, 0, 0));
                    GV->fntMyriad16->printf(dx + 11, dy + 128, HGETEXT_LEFT, "%d - %s", 0, y * 2 + x + 1,
                                            GETL2S("NewMap", tmp));
                    GV->fntMyriad16->SetColor(ARGB(int(255.0f * NewMap_data->fBaseTimer[y * 2 + x]), 255, 255, 255));
                    GV->fntMyriad16->printf(dx + 10, dy + 127, HGETEXT_LEFT, "%d - %s", 0, y * 2 + x + 1,
                                            GETL2S("NewMap", tmp));
                }
                GV->sprLevels[14]->SetColor(
                        ARGB(255 - int(255.0f * NewMap_data->fBaseTimer[y * 2 + x]), 255, 255, 255));
                GV->sprLevels[14]->Render(dx, dy);
            }
    hge->Gfx_SetClipping();
	hge->Gfx_RenderLine(wx + 27, wy + 101, wx + 27 + 600, wy + 101, GV->colBaseDark);
	hge->Gfx_RenderLine(wx + 27, wy + 101, wx + 27, wy + 101 + 439, GV->colBaseDark);
	hge->Gfx_RenderLine(wx + 27, wy + 101 + 440, wx + 27 + 600, wy + 101 + 440, GV->colBaseDark);
}

void State::EditingWW::NewMap_OK() {
    int plw = atoi(NewMap_data->tfPlaneWidth->getText().c_str()), plh = atoi(
            NewMap_data->tfPlaneHeight->getText().c_str());
    if (plw < 50 || plh < 50) {
        GV->StateMgr->Push(
                new State::Dialog(PRODUCT_NAME, GETL2S("NewMap", "InvalidPlaneSize"), ST_DIALOG_ICON_ERROR, ST_DIALOG_BUT_OK));
        return;
    }
    //repo.PrintStructure();
    void *ptr;
    int size;
    char tmp[32];
    //sprintf(tmp, "res/lvl%d.wms", NewMap_data->iSelectedBase + 1);
    sprintf(tmp, "lvl%d.wms", NewMap_data->iSelectedBase + 1);
    cSFS_Repository repo("data.sfs");
    //ptr = hge->Resource_Load(tmp, &size);
    ptr = repo.GetFileAsRawData(tmp, &size);
    char name[64], author[64];
    char *fixname = FixLevelName(NewMap_data->iSelectedBase + 1, NewMap_data->tfName->getText().c_str());
    strncpy(name, fixname, 64);
    delete[] fixname;
    strncpy(author, NewMap_data->tfAuthor->getText().c_str(), 64);
    GV->StateMgr->Push(
            new State::LoadMap(ptr, size, NewMap_data->cbAddBonusPlanes->isSelected(), plw, plh, name, author));
    NewMap_data->bKill = 1;
}

void State::EditingWW::NewMap_Validate() {
    NewMap_data->butOK->setEnabled(!NewMap_data->tfName->isMarkedInvalid() &&
                                   !NewMap_data->tfAuthor->isMarkedInvalid() &&
                                   NewMap_data->iSelectedBase >= 0 && NewMap_data->iSelectedBase < 14);
}
