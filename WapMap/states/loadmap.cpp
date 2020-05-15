#include "loadmap.h"
#include "error.h"
#include "editing_ww.h"
#include "../globals.h"
#include "../../shared/gcnWidgets/wContainer.h"
#include "../../shared/commonFunc.h"
#include "../returncodes.h"
#include "../../shared/cREZ.h"
#include "../../shared/cPID.h"
#include "../databanks/tiles.h"
#include <Windows.h>
#include "../langID.h"
#include <sys/types.h>
#include "../cMDI.h"
#include "../io/cWWDx.h"
#include "../../shared/cProgressInfo.h"
#include "../databanks/imageSets.h"
#include "../databanks/sounds.h"
#include "../databanks/anims.h"
#include "../databanks/logics.h"

extern HGE *hge;

structProgressInfo _ghProgressInfo;

State::LoadMap::LoadMap(void *ptr, uint32_t size, bool addplanes, int mw, int mh, char name[64], char author[64]) {
    szDir = szFilepath = NULL;
    alt_ptr = ptr;
    alt_size = size;
    alt_planes = addplanes;
    alt_width = mw;
    alt_height = mh;
    strcpy(alt_name, name);
    strcpy(alt_author, author);
    szFilename = new char[2];
    szFilename[0] = '-';
    szFilename[1] = '\0';
}

State::LoadMap::LoadMap(const char *pszFilename) {
    //md = new MapData;
    alt_ptr = NULL;
    szDir = SHR::GetDir(pszFilename);
    szFilename = SHR::GetFile(pszFilename);
    szFilepath = new char[strlen(pszFilename) + 1];
    strcpy(szFilepath, pszFilename);

    GV->editState->hMruList->PushNewFile(pszFilename, 1);
    //GV->Console->Printf("Loading map ~y~%s ~w~from directory: ~y~%s~w~.", md->szFilename, md->szDir);
}

State::LoadMap::~LoadMap() {

}

bool State::LoadMap::Opaque() {
    return 0;
}

void State::LoadMap::Init() {
    hge->System_SetState(HGE_HIDEMOUSE, 0);
    gui = new gcn::Gui();
    gui->setGraphics(GV->gcnGraphics);
    gui->setInput(GV->gcnInput);

    cParallelLoop *looper = new cParallelLoop();
    looper->SetFPS(20);
    looper->SetCallback(this);

    char str[128];
    sprintf(str, GETL(Lang_Opening), szFilename);
    winLoad = new SHR::Win(&GV->gcnParts, str);
    winLoad->setDimension(gcn::Rectangle(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 200,
                                         hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 100, 400, 200));
    winLoad->setMovable(0);
    winLoad->setVisible(0);
    winLoad->setVisible(1);
    gui->setTop(winLoad);

    _ghProgressInfo.iGlobalProgress = 0;
    _ghProgressInfo.iGlobalEnd = 10;
    _ghProgressInfo.iDetailedProgress = 0;
    _ghProgressInfo.iDetailedEnd = 100;
    _ghProgressInfo.strGlobalCaption = "Loading map document...";
    _ghProgressInfo.strDetailedCaption = "";

    labWhole = new SHR::Lab(GETL(Lang_Progress));
    labWhole->adjustSize();
    winLoad->add(labWhole, 5, 15);

    barWhole = new SHR::ProgressBar(&GV->gcnParts);
    barWhole->setDimension(gcn::Rectangle(0, 0, 380, 31));
    barWhole->setEnd(8);
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

    GV->IF->EnableCursor(1);

    DocumentData *dd = new DocumentData;
    dd->iWapMapBuild = -1;
    dd->iMapBuild = 1;
    dd->strMapVersion = "1.0";

    cIO_WWDx *metahandler = new cIO_WWDx(dd);
    char tmpDesc[256];
    sprintf(tmpDesc, "%s: ~y~%s~l~", GETL(Lang_LoadingMapFile), szFilename);
    labDesc->setCaption(tmpDesc);
    labDesc->adjustSize();

    ParallelTrigger();

    WWD::_ghProgressCallback = looper;
    WWD::_ghProgressInfo = &_ghProgressInfo;
    //UpdateScene();
    WWD::Parser *hParser = NULL;
    try {
        if (alt_ptr != NULL)
            hParser = new WWD::Parser(alt_ptr, alt_size, metahandler);
        else
            hParser = new WWD::Parser(szFilepath, metahandler);
    }
    catch (WWD::Exception &exc) {
#ifdef BUILD_DEBUG
        GV->Console->Printf("~r~WWD exception: (~y~%s~r~:~y~%d~r~) [~y~%d~r~]", exc.szFile, exc.iLine, exc.iErrorCode);
#else
        GV->Console->Printf("~r~WWD exception [~y~%d~r~]", exc.iErrorCode);
#endif
        char tmp[256];
        sprintf(tmp,
                "%s (WWD).\n%s:\n%s: %d",
                GETL(Lang_ErrorOpeningFile),
                GETL(Lang_ErrorInfo),
                GETL(Lang_Number),
                exc.iErrorCode);
        GV->StateMgr->Push(new State::Error(GETL(Lang_ErrorOpening), tmp, ST_ER_ICON_FATAL, ST_ER_BUT_OK, 1));
        return;
    }
    ParallelTrigger();

    if (!GV->editState->ValidateLevelName(hParser->GetName(), 0) && alt_ptr == 0) {
        if (MessageBox(hge->System_GetState(HGE_HWND), GETL2S("Various", "BadNamePrompt"), hParser->GetName(),
                       MB_YESNO | MB_ICONWARNING) == IDYES) {
            char *fixname = GV->editState->FixLevelName(hParser->GetBaseLevel(), hParser->GetName());
            hParser->SetName(fixname);
            delete[] fixname;
        }
    }

    if (alt_ptr != 0) {
        hParser->SetName(alt_name);
        hParser->SetAuthor(alt_author);
        WWD::Plane *mainpl = hParser->GetMainPlane();
        if (!alt_planes) {
            for (int i = 0; i < hParser->GetPlanesCount(); i++) {
                if (hParser->GetPlane(i) != mainpl) {
                    hParser->DeletePlane(i);
                    --i;
                }
            }
        }
        hParser->SetStartX(alt_width * mainpl->GetTileWidth() / 2);
        hParser->SetStartY(alt_height * mainpl->GetTileHeight() / 2);
        mainpl->Resize(alt_width, alt_height);
        hParser->SetFilePath("");
        hParser->UpdateDate();
    }

    if (dd->iWapMapBuild != -1) {
        GV->Console->Printf("~w~Found extended meta tags (generated by WM v~y~%s ~w~b~y~%d~w~).",
                            dd->strWapMapVersion.c_str(), dd->iWapMapBuild);
    }

    GV->Console->Printf("~g~WWD loaded.~w~");


    _ghProgressInfo.iGlobalProgress = 1;
    _ghProgressInfo.strGlobalCaption = "Mounting virtual file system...";
    ParallelTrigger();

    dd->hParser = hParser;

    GV->Console->Printf("~w~Mounting VFS...");
    dd->hDataCtrl = new cDataController(hParser->GetGame(), GV->gamePaths[hParser->GetGame()],
                                        std::string(szDir ? szDir : ""), std::string(szFilename));
    GV->Console->Printf(" ~y~REZ file:       ~g~%s", dd->hDataCtrl->GetFeed(DB_FEED_REZ) == 0 ? "~r~[not mounted]"
                                                                                              : dd->hDataCtrl->GetFeed(
                    DB_FEED_REZ)->GetAbsoluteLocation().c_str());
    GV->Console->Printf(" ~y~Disc directory: ~g~%s", dd->hDataCtrl->GetFeed(DB_FEED_DISC) == 0 ? "~r~[not mounted]"
                                                                                               : dd->hDataCtrl->GetFeed(
                    DB_FEED_DISC)->GetAbsoluteLocation().c_str());
    GV->Console->Printf(" ~y~Custom dir:     ~g~%s", dd->hDataCtrl->GetFeed(DB_FEED_CUSTOM) == 0 ? "~r~[not mounted]"
                                                                                                 : dd->hDataCtrl->GetFeed(
                    DB_FEED_CUSTOM)->GetAbsoluteLocation().c_str());

    _ghProgressInfo.iGlobalProgress = 2;
    _ghProgressInfo.strGlobalCaption = "Creating data banks...";
    ParallelTrigger();

    dd->hTilesBank = new cBankTile(hParser);
    dd->hSprBank = new cBankImageSet(hParser);
    dd->hSndBank = new cBankSound(hParser);
    dd->hAniBank = new cBankAni();
    dd->hCustomLogicBank = new cBankLogic(hParser);

    dd->hDataCtrl->RegisterAssetBank(dd->hTilesBank);
    dd->hDataCtrl->RegisterAssetBank(dd->hSprBank);
    dd->hDataCtrl->RegisterAssetBank(dd->hSndBank);
    dd->hDataCtrl->RegisterAssetBank(dd->hCustomLogicBank);

    _ghProgressInfo.iGlobalProgress = 3;
    _ghProgressInfo.strGlobalCaption = "Registering palette...";
    ParallelTrigger();

    char palettepath[256];
    sprintf(palettepath, "LEVEL%d/PALETTES/MAIN.PAL", hParser->GetBaseLevel());
    if (!dd->hDataCtrl->SetPalette(palettepath)) {
        GV->Console->Printf("~r~Error loading palette: '~y~%s~r~'!", palettepath);
        //error - brak palety
    } else {
        GV->Console->Printf("~g~Palette loaded: '~y~%s~g~'.", palettepath);
    }
    dd->hDataCtrl->SetLooper(looper);

    ParallelTrigger();

    _ghProgressInfo.iGlobalProgress = 4;
    _ghProgressInfo.strGlobalCaption = "Scanning for resources...";
    ParallelTrigger();

    GV->Console->Printf("Creating packages...");
    if (dd->hDataCtrl->GetFeed(DB_FEED_REZ) != 0)
        dd->hDataCtrl->GetFeed(DB_FEED_REZ)->ReadingStart();
    dd->hDataCtrl->CreatePackage("GAME", "GAME", cDC_STANDARD);
    char pathstr[64];
    switch (hParser->GetGame()) {
    case WWD::Game_Claw:
    case WWD::Game_Claw2:
        dd->hDataCtrl->CreatePackage("CLAW", "CLAW", cDC_STANDARD);
        sprintf(pathstr, "LEVEL%d", hParser->GetBaseLevel());
        dd->hDataCtrl->CreatePackage(pathstr, "LEVEL", cDC_STANDARD);
        break;
    case WWD::Game_GetMedieval:
        dd->hDataCtrl->CreatePackage("ENEMIES", "ENEMIES", cDC_STANDARD);
        dd->hDataCtrl->CreatePackage("PLAYERS", "PLAYERS", cDC_STANDARD);
        sprintf(pathstr, "DUNGEON%d", hParser->GetBaseLevel());
        dd->hDataCtrl->CreatePackage(pathstr, "DUNGEON", cDC_STANDARD);
        break;
    case WWD::Game_Gruntz:
        dd->hDataCtrl->CreatePackage("GRUNTZ", "GRUNTZ", cDC_STANDARD);
        sprintf(pathstr, "AREA%d", hParser->GetBaseLevel());
        dd->hDataCtrl->CreatePackage(pathstr, "LEVEL", cDC_STANDARD);
        break;
    }
    dd->hDataCtrl->CreatePackage("", "CUSTOM", cDC_CUSTOM);
    ParallelTrigger();

    //_ghProgressInfo.iGlobalProgress = 5;
    //_ghProgressInfo.strGlobalCaption = "Loading resources...";
    //ParallelTrigger();

    GV->Console->Printf("Updating packages...");
    dd->hDataCtrl->UpdateAllPackages();
    if (dd->hDataCtrl->GetFeed(DB_FEED_REZ) != 0)
        dd->hDataCtrl->GetFeed(DB_FEED_REZ)->ReadingStop();
    /*dd->hDataCtrl->RegisterAssetBank(dd->hSndBank);
    dd->hDataCtrl->RegisterAssetBank(dd->hAniBank);*/

    /*for (int i = 0; i < dd->hSprBank->GetAssetsCount(); i++) {
        cSprBankAsset *as = dd->hSprBank->GetAssetByIterator(i);
        for (int x = 0; x < as->GetSpritesCount(); x++) {
            if (as->GetIMGByIterator(x)->GetSprite() == 0) {
                GV->Console->Printf("%s: %s (%s) null", as->GetID(), as->GetIMGByIterator(x)->GetName(),
                                    as->GetIMGByIterator(x)->GetFile().strPath.c_str());
            }
        }
    }*/

    returnCode *rc = new returnCode;
    rc->Type = RC_LoadMap;
    rc->Ptr = (int) dd;
    _popMe((int) rc);

    WWD::_ghProgressCallback = 0;
    WWD::_ghProgressInfo = 0;
    GV->Console->Printf("~g~Map loaded ~w~(~y~%s~w~ by ~y~%s~w~, base level %d).", hParser->GetName(),
                        hParser->GetAuthor(), hParser->GetBaseLevel());
    looper->SetCallback(0);
}

void State::LoadMap::Destroy() {
    hge->System_SetState(HGE_HIDEMOUSE, 1);
    delete winLoad;
    delete barWhole, barAction;
    delete labWhole, labAction, labDesc;
    if (!alt_ptr) {
        delete[] szFilepath;
        delete[] szDir;
    } else {
        hge->Resource_Free(alt_ptr);
    }
    delete[] szFilename;
    delete gui;
}

bool State::LoadMap::Think() {
    try {
        gui->logic();
    }
    catch (gcn::Exception &exc) {
        GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(),
                            exc.getLine());
    }
    return 0;
}

bool State::LoadMap::Render() {
    try {
        gui->draw();
    }
    catch (gcn::Exception &exc) {
        GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(),
                            exc.getLine());
    }
    //GV->IF->Render();
    GV->Console->Render();
    return 0;
}

void State::LoadMap::GainFocus(int iReturnCode, bool bFlipped) {
    delete ((returnCode *) iReturnCode);
    returnCode *rc = new returnCode;
    rc->Type = RC_LoadMap;
    rc->Ptr = 0;
    _popMe((int) rc);
}

void State::LoadMap::ParallelTrigger() {
    labWhole->setCaption(_ghProgressInfo.strGlobalCaption);
    labWhole->adjustSize();
    labDesc->setCaption(_ghProgressInfo.strDetailedCaption);
    labDesc->adjustSize();
    barWhole->setProgress(_ghProgressInfo.iGlobalProgress);
    barWhole->setEnd(_ghProgressInfo.iGlobalEnd);
    barAction->setProgress(_ghProgressInfo.iDetailedProgress);
    barAction->setEnd(_ghProgressInfo.iDetailedEnd);
    Think();
    GV->StateMgr->ManualRenderStart();
    Render();
    GV->StateMgr->ManualRenderStop();
}
