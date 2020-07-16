#include "loadmap.h"
#include "dialog.h"
#include "editing_ww.h"
#include "../../shared/commonFunc.h"
#include "../databanks/tiles.h"
#include "../langID.h"
#include "../cMDI.h"
#include "../io/cWWDx.h"
#include "../databanks/imageSets.h"
#include "../databanks/sounds.h"
#include "../databanks/anims.h"
#include "../databanks/logics.h"
#include "../version.h"

structProgressInfo _ghProgressInfo;

State::LoadMap::LoadMap(char *ptr, uint32_t size, bool addPlanes, int mw, int mh, const char* name, const char* author) {
    szDir = szFilepath = NULL;
    alt_ptr = ptr;
    alt_size = size;
    alt_planes = addPlanes;
    alt_width = mw;
    alt_height = mh;
    strcpy(alt_name, name);
    strcpy(alt_author, author);
    szFilename = new char[2];
    szFilename[0] = '-';
    szFilename[1] = '\0';
}

State::LoadMap::LoadMap(const char *pszFilename) {
    alt_ptr = nullptr;
    szDir = SHR::GetDir(pszFilename);
    szFilename = SHR::GetFile(pszFilename);
    szFilepath = new char[strlen(pszFilename) + 1];
    strcpy(szFilepath, pszFilename);

    GV->editState->hMruList->PushNewFile(pszFilename, true);
}

State::LoadMap::~LoadMap() {
}

bool State::LoadMap::Opaque() {
    return false;
}

DocumentData* State::LoadMap::MapLoadTask() {
    WWD::Parser *hParser = nullptr;

    auto *dd = new DocumentData;
    dd->iWapMapBuild = -1;
    dd->iMapBuild = 1;
    dd->strMapVersion = "1.0";
    auto *metaHandler = new cIO_WWDx(dd);

    try {
        if (alt_ptr) {
            membuf buf(alt_ptr, alt_ptr + alt_size);
            std::istream is(&buf);
            if (is.fail()) throw WWD_EXCEPTION(WWD::Error_LoadingMemory);
            hParser = new WWD::Parser(metaHandler);

            hParser->LoadFromStream(&is);
        } else {
            std::ifstream is(szFilepath, std::ios_base::binary | std::ios_base::in);
            if (is.fail()) throw WWD_EXCEPTION(WWD::Error_OpenAccess);
            hParser = new WWD::Parser(szFilepath, metaHandler);

            hParser->LoadFromStream(&is);
        }
    } catch (WWD::Exception &exc) {
#ifdef BUILD_DEBUG
        GV->Console->Printf("~r~WWD exception: (~y~%s~r~:~y~%d~r~) [~y~%d~r~]", exc.szFile, exc.iLine, exc.iErrorCode);
#else
        GV->Console->Printf("~r~WWD exception [~y~%d~r~]", exc.iErrorCode);
#endif
        char tmp[256];
        sprintf(tmp,
                "%s. \n%s:\n%s: %d",
                GETL(Lang_ErrorOpeningFile),
                GETL(Lang_ErrorInfo),
                GETL(Lang_Number),
                exc.iErrorCode);
        GV->StateMgr->Push(new State::Dialog(PRODUCT_NAME, tmp, ST_DIALOG_ICON_ERROR));

        delete hParser;
        delete metaHandler;
        delete dd;
        return nullptr;
    }

    dd->hParser = hParser;
    barWhole->setProgress(100);

    return dd;
}

void State::LoadMap::Init() {
    gui = new gcn::Gui();
    gui->setGraphics(GV->gcnGraphics);
    gui->setInput(GV->gcnInput);

    std::string winName(GETL(Lang_Opening));
    winName += ' ';
    winName += strlen(szFilename) > 50 ? std::string_view(szFilename, 50) : szFilename;
    winName += "...";
    winLoad = new SHR::Win(&GV->gcnParts, winName);
    winLoad->setDimension(gcn::Rectangle(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 200,
                                         hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 100, 400, 200));
    winLoad->setMovable(false);
    winLoad->setVisible(false);
    winLoad->setVisible(true);
    gui->setTop(winLoad);

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

    char tmpDesc[256];
    sprintf(tmpDesc, "%s: ~y~%s~l~", GETL(Lang_LoadingMapFile), szFilename);
    labDesc->setCaption(tmpDesc);
    labDesc->adjustSize();

    mapLoading = std::async(std::launch::async, &LoadMap::MapLoadTask, this);
}

void State::LoadMap::Destroy() {
    delete winLoad;
    delete barWhole;
    delete barAction;
    delete labWhole;
    delete labAction;
    delete labDesc;
    if (alt_ptr) {
        hge->Resource_Free(alt_ptr);
    } else {
        delete[] szFilepath;
        delete[] szDir;
    }
    delete[] szFilename;
    delete gui;
}

bool State::LoadMap::Think() {
    GV->Console->Think();
    try {
        gui->logic();
    } catch (gcn::Exception &exc) {
        GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(),
                            exc.getLine());
    }

    if (mapLoading.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) {
        auto dd = mapLoading.get();

        if (dd) {
            if (alt_ptr) {
                dd->hParser->SetName(alt_name);
                dd->hParser->SetAuthor(alt_author);
                auto mainPlane = dd->hParser->GetMainPlane();
                if (!alt_planes) {
                    for (int i = 0; i < dd->hParser->GetPlanesCount(); i++) {
                        if (dd->hParser->GetPlane(i) != mainPlane) {
                            dd->hParser->DeletePlane(i);
                            --i;
                        }
                    }
                }
                dd->hParser->SetStartX(alt_width * mainPlane->GetTileWidth() / 2);
                dd->hParser->SetStartY(alt_height * mainPlane->GetTileHeight() / 2);
                mainPlane->Resize(alt_width, alt_height, 0, 0, true);
                dd->hParser->SetFilePath("");
                dd->hParser->UpdateDate();
            } /*else if (dd->hParser->GetGame() == WWD::Game_Claw && !GV->editState->ValidateLevelName(dd->hParser->GetName(), false)) {
                if (State::MessageBox(PRODUCT_NAME, GETL2S("Various", "BadNamePrompt"),
                                      ST_DIALOG_ICON_WARNING, ST_DIALOG_BUT_YESNO) == RETURN_YES) {
                    char *fixname = GV->editState->FixLevelName(dd->hParser->GetBaseLevel(), dd->hParser->GetName());
                    dd->hParser->SetName(fixname);
                    delete[] fixname;
                }
            }*/

            if (dd->iWapMapBuild != -1) {
                GV->Console->Printf("~w~Found extended meta tags (generated by WM v~y~%s ~w~b~y~%d~w~).",
                                    dd->strWapMapVersion.c_str(), dd->iWapMapBuild);
            }

            GV->Console->Printf("~g~WWD loaded.~w~");


            GV->Console->Printf("~w~Mounting VFS...");

            dd->hDataCtrl = new cDataController(dd->hParser->GetGame(), GV->gamePaths[dd->hParser->GetGame()],
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
            dd->hTilesBank = new cBankTile(dd->hParser);
            dd->hSprBank = new cBankImageSet(dd->hParser);
            dd->hSndBank = new cBankSound(dd->hParser);
            dd->hAniBank = new cBankAni();
            dd->hCustomLogicBank = new cBankLogic(dd->hParser);

            dd->hDataCtrl->RegisterAssetBank(dd->hTilesBank);
            dd->hDataCtrl->RegisterAssetBank(dd->hSprBank);
            dd->hDataCtrl->RegisterAssetBank(dd->hSndBank);
            dd->hDataCtrl->RegisterAssetBank(dd->hCustomLogicBank);

            char palettePath[256];
            sprintf(palettePath, "LEVEL%d/PALETTES/MAIN.PAL", dd->hParser->GetBaseLevel());
            if (!dd->hDataCtrl->SetPalette(palettePath)) {
                GV->Console->Printf("~r~Error loading palette: '~y~%s~r~'!", palettePath);
            }
            else {
                GV->Console->Printf("~g~Palette loaded: '~y~%s~g~'.", palettePath);
            }

            GV->Console->Printf("Creating packages...");
            if (dd->hDataCtrl->GetFeed(DB_FEED_REZ) != 0)
                dd->hDataCtrl->GetFeed(DB_FEED_REZ)->ReadingStart();
            dd->hDataCtrl->CreatePackage("GAME", "GAME", cDC_STANDARD);
            char dataSourceName[64];
            switch (dd->hParser->GetGame()) {
            case WWD::Game_Claw:
            case WWD::Game_Claw2:
                dd->hDataCtrl->CreatePackage("CLAW", "CLAW", cDC_STANDARD);
                sprintf(dataSourceName, "LEVEL%d", dd->hParser->GetBaseLevel());
                dd->hDataCtrl->CreatePackage(dataSourceName, "LEVEL", cDC_STANDARD);
                break;
            case WWD::Game_GetMedieval:
                dd->hDataCtrl->CreatePackage("ENEMIES", "ENEMIES", cDC_STANDARD);
                dd->hDataCtrl->CreatePackage("PLAYERS", "PLAYERS", cDC_STANDARD);
                sprintf(dataSourceName, "DUNGEON%d", dd->hParser->GetBaseLevel());
                dd->hDataCtrl->CreatePackage(dataSourceName, "DUNGEON", cDC_STANDARD);
                break;
            case WWD::Game_Gruntz:
                dd->hDataCtrl->CreatePackage("GRUNTZ", "GRUNTZ", cDC_STANDARD);
                sprintf(dataSourceName, "AREA%d", dd->hParser->GetBaseLevel());
                dd->hDataCtrl->CreatePackage(dataSourceName, "LEVEL", cDC_STANDARD);
                break;
            }
            dd->hDataCtrl->CreatePackage("", "CUSTOM", cDC_CUSTOM);

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

            GV->Console->Printf("~g~Map loaded ~w~(~y~%s~w~ by ~y~%s~w~, base level %d).", dd->hParser->GetName(),
                dd->hParser->GetAuthor(), dd->hParser->GetBaseLevel());
        }

        _popMe({ ReturnCodeType::LoadMap, (int)dd });
    }

    return false;
}

bool State::LoadMap::Render() {
    try {
        gui->draw();
    } catch (gcn::Exception &exc) {
        GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(),
                            exc.getLine());
    }
    GV->Console->Render();
    return false;
}

/*void State::LoadMap::ParallelTrigger() {
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
}*/
