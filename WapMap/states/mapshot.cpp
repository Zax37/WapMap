#include "mapshot.h"
#include "../globals.h"
#include "../version.h"
#include <cmath>
#include "../langID.h"
#include "editing_ww.h"
#include "../returncodes.h"
#include "error.h"
#include "../../shared/commonFunc.h"

extern HGE *hge;

State::MapShot::MapShot(State::EditingWW *mainSt, int planeIterator, float scale, const char *dest, int saveoption,
                        DWORD bg, bool drawobj) {
    fScale = scale / 100.0f;
    iPLid = planeIterator;
    stMain = mainSt;
    pl = stMain->hParser->GetPlane(iPLid);
    targTemp = 0;
    szOutput = new char[strlen(dest) + 1];
    strcpy(szOutput, dest);
    iOutputOption = saveoption;
    dwBg = bg;
    bDrawObjects = drawobj;
    targPreview = 0;
    sprPreview = NULL;

    iPreviewW = pl->GetPlaneWidthPx() * fScale;
    if (iPreviewW > 395) {
        iPreviewW = 395;
        fPreviewScale = float(iPreviewW) / (float(pl->GetPlaneWidthPx()) * fScale);
        iPreviewH = (pl->GetPlaneHeightPx() * fScale) * fPreviewScale;
    } else {
        fPreviewScale = fScale;
        iPreviewH = pl->GetPlaneHeightPx() * fScale;
    }
}

bool State::MapShot::Opaque() {
    return 1;
}

void State::MapShot::Init() {
    gui = new gcn::Gui();
    gui->setGraphics(GV->gcnGraphics);
    gui->setInput(GV->gcnInput);

    winLoad = new SHR::Win(&GV->gcnParts, GETL(Lang_MakingMapshot));
    winLoad->setDimension(gcn::Rectangle(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 200,
                                         hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 100, 400, 200));
    winLoad->setMovable(0);
    gui->setTop(winLoad);

    labWhole = new SHR::Lab(GETL(Lang_Progress));
    labWhole->adjustSize();
    winLoad->add(labWhole, 5, 15);

    barWhole = new SHR::ProgressBar(&GV->gcnParts);
    barWhole->setDimension(gcn::Rectangle(0, 0, 380, 31));
    barWhole->setEnd(1000);
    winLoad->add(barWhole, 5, 40);

    labAction = new SHR::Lab(GETL(Lang_ActualOperation));
    labAction->adjustSize();
    winLoad->add(labAction, 5, 80);

    barAction = new SHR::ProgressBar(&GV->gcnParts);
    barAction->setDimension(gcn::Rectangle(0, 0, 380, 31));
    barAction->setEnd(100);
    winLoad->add(barAction, 5, 105);

    labDesc = new SHR::Lab("");
    labDesc->adjustSize();
    winLoad->add(labDesc, 5, 140);

    int totalw = pl->GetPlaneWidthPx() * fScale, totalh = pl->GetPlaneHeightPx() * fScale;
    int tempw = std::min(totalw, 2048), temph = std::min(totalh, 2048);

    char labtmp[256];
    labAction->setCaption(GETL2("Mapshot", Lang_Ms_AllocatingDestinationSurface));
    labAction->adjustSize();
    labWhole->setCaption(GETL2("Mapshot", Lang_Ms_PreparingMemory));
    labWhole->adjustSize();
    barWhole->setProgress(0);
    barWhole->setEnd(4);
    barAction->setProgress(1);
    barAction->setEnd(3);
    UpdateScene();

    gdDest.create(totalw, totalh, sf::Color::Transparent);
    GV->Console->Printf("~w~Mapshot: ~g~GD surface allocated (~y~%d~g~x~y~%d ~g~px).", totalw, totalh);

    labAction->setCaption(GETL2("Mapshot", Lang_Ms_AllocatingTemporarySurface));
    labAction->adjustSize();
    barAction->setProgress(2);
    barAction->setEnd(3);
    UpdateScene();

    targTemp = hge->Target_Create(tempw, temph, 0);
    if (!targTemp) {
        GV->Console->Printf("~w~Mapshot: ~r~Error allocating temporary surface (~y~%d~r~x~y~%d ~r~px).", tempw, temph);
        GV->StateMgr->Push(new State::Error(GETL2("Mapshot", Lang_Ms_Error), GETL2("Mapshot", Lang_Ms_NotEnoughMem),
                                            ST_ER_ICON_WARNING, ST_ER_BUT_OK, 1));
        return;
    } else
        GV->Console->Printf("~w~Mapshot: ~g~Temporary surface allocated (~y~%d~g~x~y~%d ~g~px).", tempw, temph);

    int passx = ceil(float(totalw) / float(tempw)), passy = ceil(float(totalh) / float(temph));
    int passescount = passx * passy;
    GV->Console->Printf("~w~Mapshot: Going through %d passes (~y~%d ~w~horizontally, ~y~%d ~w~vertically).",
                        passescount, passx, passy);
    labWhole->setCaption(GETL2("Mapshot", Lang_Ms_Drawing));
    labWhole->adjustSize();
    barWhole->setProgress(1);
    barAction->setEnd(passescount);

    float fCamXinit = stMain->fCamX, fCamYinit = stMain->fCamY;
    int iVPw = stMain->vPort->GetWidth(), iVPh = stMain->vPort->GetHeight();
    int iVPx = stMain->vPort->GetX(), iVPy = stMain->vPort->GetY();

    PlaneData *origPlaneData = new PlaneData[stMain->hParser->GetPlanesCount()];
    for (int i = 0; i < stMain->hParser->GetPlanesCount(); i++) {
        origPlaneData[i] = *stMain->hPlaneData[i];
        stMain->hPlaneData[i]->bDraw = (i == iPLid);
        stMain->hPlaneData[i]->bDrawObjects = (i == iPLid && !stMain->hPlaneData[i]->ObjectData.bEmpty && bDrawObjects);
    }

    stMain->vPort->SetPos(0, 0);
    stMain->vPort->Resize(tempw, temph);

    float fOriginalZoom = stMain->fZoom;

    stMain->fZoom = fScale;

    DWORD origbg = stMain->dwBG;
    stMain->dwBG = dwBg;

    for (int y = 0; y < passy; y++)
        for (int x = 0; x < passx; x++) {
            barAction->setProgress(y * passx + x);
            sprintf(labtmp, "%s %d/%d", GETL2("Mapshot", Lang_Ms_Chunk), y * passx + x + 1, passescount);
            labAction->setCaption(labtmp);
            labAction->adjustSize();
            labDesc->setCaption(GETL2("Mapshot", Lang_Ms_Drawing));
            labDesc->adjustSize();
            UpdateScene();

            stMain->fCamX = x * tempw * (1 / fScale);
            stMain->fCamY = y * temph * (1 / fScale);

            stMain->vPort->MarkToRedraw(1);
            stMain->vPort->Update();

            hge->Gfx_BeginScene(targTemp);

            hge->Gfx_Clear(0x01000000);
            GV->sprBlank->RenderStretch(0, 0, tempw, temph);

            stMain->vPort->Render();
            if (x == 0 && y == 0) {
                hgeFont *font = GV->fntMyriad80;
                char mark[64];
                sprintf(mark, "GENERATED BY WAPMAP V%s", WA_VERPURE);
                if (GV->fntMyriad80->GetStringWidth(mark) + 5 > tempw) {
                    sprintf(mark, "Generated by WapMap v%s", WA_VERPURE);
                    font = GV->fntMyriad20;
                }
                font->SetColor(0xFF000000);
                font->Render(7, 7, HGETEXT_LEFT, mark);
                font->SetColor(0xFFFFFFFF);
                font->Render(5, 5, HGETEXT_LEFT, mark);
            }
            hge->Gfx_EndScene();

            bool previewcreated = 0;
            if (targPreview == 0) {
                targPreview = hge->Target_Create(iPreviewW, iPreviewH, 0);
                sprPreview = new hgeSprite(hge->Target_GetTexture(targPreview), 0, 0, iPreviewW, iPreviewH);
                sprPreview->SetHotSpot(sprPreview->GetWidth() / 2, 0);
                previewcreated = 1;
            }

            hge->Gfx_BeginScene(targPreview);
            if (previewcreated)
                hge->Gfx_Clear(0xFF3e3e3e);
            HTEXTURE tmptex = hge->Target_GetTexture(targTemp);
            hgeSprite *tmpspr = new hgeSprite(tmptex, 0, 0, tempw, temph);
            tmpspr->RenderEx(x * tempw * fPreviewScale, y * temph * fPreviewScale, 0, fPreviewScale);
            printf("render %f, %f, %f\n", x * tempw * fPreviewScale, y * temph * fPreviewScale, fPreviewScale);
            delete tmpspr;
            hge->Gfx_EndScene();

            labDesc->setCaption(GETL2("Mapshot", Lang_Ms_Processing));
            labDesc->adjustSize();
            UpdateScene();

            HSURFACE surf = hge->Target_GetSurface(targTemp);
            DWORD *data = hge->Surface_Lock(surf);

            for (int ty = 0; ty < temph; ty++)
                for (int tx = 0; tx < tempw; tx++) {
                    if (x * tempw + tx >= totalw || y * temph + ty >= totalh) continue;
                    DWORD dwcol = data[ty * tempw + tx];
                    sf::Color col = sf::Color(GETR(dwcol), GETG(dwcol), GETB(dwcol)/*, (255-GETA(dwcol))/2*/);
                    gdDest.setPixel(x * tempw + tx, y * temph + ty, col);
                }
            hge->Texture_Unlock(tmptex);
            hge->Texture_Free(tmptex);
        }

    hge->Target_Free(targTemp);
    targTemp = 0;

    stMain->dwBG = origbg;

    stMain->fCamX = fCamXinit;
    stMain->fCamY = fCamYinit;
    stMain->vPort->SetPos(iVPx, iVPy);
    stMain->vPort->Resize(iVPw, iVPh);
    stMain->fZoom = fOriginalZoom;
    for (int i = 0; i < stMain->hParser->GetPlanesCount(); i++) {
        stMain->hPlaneData[i]->bDraw = origPlaneData[i].bDraw;
        stMain->hPlaneData[i]->bDrawObjects = origPlaneData[i].bDrawObjects;
    }
    delete[] origPlaneData;

    labWhole->setCaption(GETL2("Mapshot", Lang_Ms_Saving));
    labWhole->adjustSize();
    barWhole->setProgress(3);
    barAction->setProgress(1);
    barAction->setEnd(4);
    labDesc->setCaption("");
    labDesc->adjustSize();
    labAction->setCaption("");
    labAction->adjustSize();
    UpdateScene();

    FILE *hFile = fopen(szOutput, "wb");
    char *filename = SHR::GetFile(szOutput);

    GV->Console->Printf("~w~Mapshot: Saving as ~y~%s~w~.", szOutput);
    if (!gdDest.saveToFile(filename)) {
        GV->Console->Printf("Saving failed!!!", szOutput);
    }
    delete[] filename;

    fclose(hFile);

    returnCode *rc = new returnCode;
    rc->Type = RC_MapShot;
    rc->Ptr = 1;
    _popMe((int) rc);
    GV->Console->Printf("~w~Mapshot: ~g~Done.");
}

void State::MapShot::Destroy() {
    if (targTemp != 0)
        hge->Target_Free(targTemp);
    delete[] szOutput;

    if (sprPreview != NULL)
        delete sprPreview;
    if (targPreview != 0)
        hge->Target_Free(targPreview);
}

bool State::MapShot::Think() {
    GV->Console->Think();
    try {
        gui->logic();
    }
    catch (gcn::Exception &exc) {
        GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(),
                            exc.getLine());
    }
    if (sprPreview != NULL) {
        winLoad->setHeight(200 + iPreviewH);
    }
    return 0;
}

bool State::MapShot::Render() {
    try {
        gui->draw();
    }
    catch (gcn::Exception &exc) {
        GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(),
                            exc.getLine());
    }
    if (sprPreview != NULL)
        sprPreview->Render(hge->System_GetState(HGE_SCREENWIDTH) / 2, hge->System_GetState(HGE_SCREENHEIGHT) / 2 + 80);
    GV->Console->Render();
    return 0;
}

void State::MapShot::GainFocus(int iReturnCode, bool bFlipped) {
    returnCode *rc = new returnCode;
    rc->Type = RC_MapShot;
    rc->Ptr = 0;
    _popMe((int) rc);
}
