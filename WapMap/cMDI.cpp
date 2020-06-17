#include "cMDI.h"
#include "../shared/commonFunc.h"
#include "globals.h"
#include "states/editing_ww.h"
#include "cObjectUserData.h"
#include "langID.h"
#include "cAppMenu.h"
#include "io/cWWDx.h"
#include "version.h"
#include "states/loadmap.h"

extern HGE *hge;

int cTabMDI::MaxTabWidth = 100;

void cMDI::action(const gcn::ActionEvent &actionEvent) {
    if (actionEvent.getSource() == hContext) {
        if (hContext->GetSelectedID() == MDI_CONTEXT_RELOAD) {
            bReloadingMap = 1;
            GV->StateMgr->Push(new State::LoadMap(m_vhDoc[m_iContextMenuFocusedDoc]->hParser->GetFilePath()));
            hContext->setVisible(0);
            return;
        }
        if (hContext->GetSelectedID() == MDI_CONTEXT_PREVIOUSLYCLOSED)
            return;
        for (int i = 0; i < m_vhDoc.size(); i++) {
            if (hContext->GetSelectedID() == MDI_CONTEXT_CLOSEEXCEPTACTIVE && m_iContextMenuFocusedDoc == i)
                continue;
            if (!CloseDocByIt(i)) {
                hContext->setVisible(0);
                return;
            }
            i--;
        }
    } else if (actionEvent.getSource() == hContextClosed) {
        GV->StateMgr->Push(new State::LoadMap(vstrRecentlyClosed[hContextClosed->GetSelectedID()].c_str()));
    }
    hContext->setVisible(0);
}

cMDI::cMDI() {
    m_iActiveDoc = -1;
    bBlock = 0;
    bReloadingMap = 0;
    bUpdateCrashList = 1;
    hContextClosed = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    hContextClosed->adjustSize();
    hContextClosed->addActionListener(this);
    hContextClosed->hide();
    GV->editState->conMain->add(hContextClosed, 400, 400);

    hContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad16);
    hContext->addActionListener(this);
    hContext->hide();
    GV->editState->conMain->add(hContext, 400, 400);
    RebuildContext(1);
    m_iPosY = LAY_MDI_Y;
    m_iAddTabButtonOffset = (LAY_MDI_H - 16) / 2;
    //fAddTabTimer = 0.f;
}

cMDI::~cMDI() {
    while (!m_vhDoc.empty())
        DeleteDocByIt(0);
    FILE *f = fopen("runtime.tmp", "r");
    if (f) {
        fclose(f);
        if (remove("runtime.tmp") != 0) {
            GV->Console->Printf("~r~Error removing runtime temporary cache!");
        }
    }
    delete hContext;
    delete hContextClosed;
    GV->Console->Printf("~g~MDI released.");
}

void cMDI::UpdateCrashList() {
    if (!bUpdateCrashList) return;

    FILE *f = fopen("runtime.tmp", "w");
    if (!f) {
        GV->Console->Printf("~r~Unable to update runtime temporary cache!");
        return;
    }
    for (auto & doc : m_vhDoc) {
        if (strlen(doc->hParser->GetFilePath()) > 0)
            fprintf(f, "%s\n", doc->hParser->GetFilePath());
    }
    fclose(f);
    GV->Console->Printf("~g~Runtime temporary cache updated.");
}

DocumentData *cMDI::AddDocument(DocumentData *dd) {
    bool justCreated = strlen(dd->hParser->GetFilePath()) == 0;
    if (justCreated) {
        const char *str = GETL2S("NewMap", "NewDoc");
        int size = strlen(str) - 3;
        dd->szFileName = new char[size];
        strncpy(dd->szFileName, str, size - 1);
        dd->szFileName[size - 1] = 0;
    }
    else {
        dd->szFileName = SHR::GetFile(dd->hParser->GetFilePath());
        strrchr(dd->szFileName, '.')[0] = 0;
    }

    dd->bSaved = true;

    dd->fZoom = 1;

    dd->hTab = new cTabMDI();
    dd->hTab->dd = dd;
    dd->hTab->hMDI = this;
    dd->hTab->bFocused = 0;
    dd->hTab->fTimer = 0;

    dd->hTileClipboardImageSet = NULL;
    dd->hTileClipboard = NULL;
    dd->iTileCBw = dd->iTileCBh = -1;

    dd->iTileSelectX1 = dd->iTileSelectY1 = dd->iTileSelectX2 = dd->iTileSelectY2 = -1;

    GV->editState->hPlaneData.clear();
    for (int i = 0; i < dd->hParser->GetPlanesCount(); i++) {
        dd->hPlaneData.push_back(new State::PlaneData());
        GV->editState->hPlaneData.push_back(dd->hPlaneData.back());
        if (dd->hParser->GetPlane(i)->GetFlags() & WWD::Flag_p_MainPlane)
            dd->iSelectedPlane = i;
        dd->hParser->GetPlane(i)->SetObjectDeletionCallback(&EditingWW_ObjDeletionCB);
        dd->hPlaneData[i]->bDraw = 1;
        dd->hPlaneData[i]->bDrawGrid = 0;
        dd->hPlaneData[i]->bDrawBoundary = 0;
        dd->hPlaneData[i]->bDrawObjects = 1;
        dd->hPlaneData[i]->ObjectData.hQuadTree = NULL;
        dd->hPlaneData[i]->ObjectData.bEmpty = 1;
        if (dd->hParser->GetPlane(i)->GetObjectsCount() != 0 ||
            dd->hParser->GetPlane(i)->GetFlags() & WWD::Flag_p_MainPlane) {
            for (int z = 0; z < dd->hParser->GetPlane(i)->GetObjectsCount(); z++)
                dd->hParser->GetPlane(i)->GetObjectByIterator(z)->SetUserData(
                        new cObjUserData(dd->hParser->GetPlane(i)->GetObjectByIterator(z)));
            dd->hPlaneData[i]->ObjectData.hQuadTree = new cObjectQuadTree(dd->hParser->GetPlane(i), dd->hSprBank);
            dd->hPlaneData[i]->ObjectData.bEmpty = 0;
        }
        /*if (dd->hParser->GetPlane(i)->GetPlaneWidthPx() <= 2048 && dd->hParser->GetPlane(i)->GetPlaneHeightPx() <= 2048) {
            GV->Console->Printf("Creating buffer for plane '~y~%s~w~' (%dx%d).", dd->hParser->GetPlane(i)->GetName(), dd->hParser->GetPlane(i)->GetPlaneWidthPx(), dd->hParser->GetPlane(i)->GetPlaneHeightPx());
            dd->hPlaneData[i]->hRB = new State::cLayerRenderBuffer(GV->editState, GV->editState->vPort, dd->hParser->GetPlane(i));
        }
        else*/
        dd->hPlaneData[i]->hRB = NULL;
        if ((dd->hParser->GetPlane(i)->GetFlags() & WWD::Flag_p_MainPlane)) {
            dd->hStartingPosObj = new WWD::Object();
            dd->hStartingPosObj->SetLogic("_WM_STARTPOS");
            if (dd->hParser->GetGame() == WWD::Game_Gruntz)
                dd->hStartingPosObj->SetImageSet("GAME_GRUNTSELECTEDSPRITE");
            else if (dd->hParser->GetGame() == WWD::Game_Claw || dd->hParser->GetGame() == WWD::Game_Claw2) {
                dd->hStartingPosObj->SetImageSet("CLAW");
                dd->hStartingPosObj->SetParam(WWD::Param_LocationI, 13);
                dd->hStartingPosObj->SetParam(WWD::Param_LocationZ, 4000);
            } else {
                dd->hStartingPosObj->SetImageSet("GAME_CONFIGCONTROLS");
                dd->hStartingPosObj->SetParam(WWD::Param_LocationI, 3);
            }
            dd->hStartingPosObj->SetParam(WWD::Param_LocationX, dd->hParser->GetStartX());
            dd->hStartingPosObj->SetParam(WWD::Param_LocationY, dd->hParser->GetStartY());
            dd->hStartingPosObj->SetUserData(new cObjUserData(dd->hStartingPosObj));
            dd->hPlaneData[i]->ObjectData.hQuadTree->UpdateObject(dd->hStartingPosObj);
        }
    }

    if (bReloadingMap) {
        int pos = m_iContextMenuFocusedDoc;
        DeleteDocByIt(m_iContextMenuFocusedDoc);
        bReloadingMap = false;
        m_vhDoc.insert(m_vhDoc.begin() + pos, dd);
        dd->hTab->index = pos;
    } else {
        dd->hTab->index = m_vhDoc.size();
        m_vhDoc.push_back(dd);
    }
    GV->Console->Printf("~g~Document added.");
    UpdateCrashList();
    GV->anyMapLoaded = true;
    if (!justCreated) {
        bool bRebuild = false;
        for (int i = 0; i < vstrRecentlyClosed.size(); i++) {
            if (!strcmp(dd->hParser->GetFilePath(), vstrRecentlyClosed[i].c_str())) {
                hContextClosed->RemoveElement(i);
                vstrRecentlyClosed.erase(vstrRecentlyClosed.begin() + i);
                bRebuild = true;
            }
        }
        if (bRebuild) {
            RebuildContext(false);
            GV->editState->hAppMenu->SyncRecentlyClosedRebuild();
        }
    }
    hContext->GetElementByID(MDI_CONTEXT_CLOSEALL)->SetEnabled(true);
    hContext->GetElementByID(MDI_CONTEXT_CLOSEEXCEPTACTIVE)->SetEnabled(m_vhDoc.size() > 1);

    UpdateMaxTabSize();

    return dd;
}

void cMDI::RebuildContext(bool bForceRebuildBase) {
    SHR::ContextEl *prevclosed = hContext->GetElementByID(MDI_CONTEXT_PREVIOUSLYCLOSED);
    if (!vstrRecentlyClosed.size() || bForceRebuildBase) {
        if (prevclosed || bForceRebuildBase) {
            hContext->ClearElements();
            hContext->AddElement(MDI_CONTEXT_CLOSEEXCEPTACTIVE, GETL2S("MDI", "Context_CloseAllExceptActive"),
                                 GV->sprIcons16[Icon16_XTransparent]);
            hContext->AddElement(MDI_CONTEXT_CLOSEALL, GETL2S("MDI", "Context_CloseAll"), GV->sprIcons16[Icon16_X]);
            hContext->AddElement(MDI_CONTEXT_RELOAD, GETL2S("MDI", "Context_Reload"),
                                 GV->sprIcons16[Icon16_AutoUpdate]);
            hContext->GetElementByID(MDI_CONTEXT_CLOSEEXCEPTACTIVE)->SetEnabled(m_vhDoc.size() > 1);
            hContext->GetElementByID(MDI_CONTEXT_CLOSEALL)->SetEnabled(m_vhDoc.size() != 0);
            hContext->adjustSize();
        }
    }
    if (!vstrRecentlyClosed.size()) return;
    if (!prevclosed) {
        hContext->AddElement(MDI_CONTEXT_PREVIOUSLYCLOSED, GETL2S("MDI", "Context_PreviouslyClosed"));
        hContext->adjustSize();
        prevclosed = hContext->GetElementByID(MDI_CONTEXT_PREVIOUSLYCLOSED);
    }

    hContextClosed->ClearElements();
    for (int i = 0; i < vstrRecentlyClosed.size(); i++) {
        char *file = SHR::GetFile(vstrRecentlyClosed[i].c_str());
        hgeSprite *sprIcon = 0;
        int ibase = 0, icon = 0;
        try {
            icon = WWD::GetGameTypeFromFile(vstrRecentlyClosed[i].c_str(), &ibase);
        }
        catch (...) {
            icon = WWD::Game_Unknown;
        }
        if (ibase != 0 && icon == WWD::Game_Claw) {
            sprIcon = GV->sprLevelsMicro16[ibase - 1];
        } else {
            sprIcon = GV->sprGamesSmall[icon];
        }
        hContextClosed->AddElement(i, file, sprIcon);
        delete[] file;
    }
    hContextClosed->adjustSize();

    prevclosed->SetEnabled(1);
    if (prevclosed->GetCascade() == NULL)
        hContext->GetElementByID(MDI_CONTEXT_PREVIOUSLYCLOSED)->SetCascade(hContextClosed);
}

void cMDI::DeleteDocByIt(int i) {
    if (i < 0 || i >= m_vhDoc.size()) return;

    if (strlen(m_vhDoc[i]->hParser->GetFilePath()) > 0) {
        bool alreadyThere = false;
        for (auto& closed : vstrRecentlyClosed) {
            if (closed == m_vhDoc[i]->hParser->GetFilePath()) {
                alreadyThere = true;
                break;
            }
        }
        if (!alreadyThere) {
            if (vstrRecentlyClosed.size() == 5) {
                vstrRecentlyClosed.pop_back();
            }
            vstrRecentlyClosed.insert(vstrRecentlyClosed.begin(), m_vhDoc[i]->hParser->GetFilePath());
        }
    }

    DocumentData *dd = m_vhDoc[i];

    m_vhDoc.erase(m_vhDoc.begin() + i);
    if (m_vhDoc.size() == 0) SetActiveDocIt(-1);
    else if (m_iActiveDoc > i) m_iActiveDoc--;
    else if (m_iActiveDoc == i && m_iActiveDoc >= m_vhDoc.size()) SetActiveDocIt(m_iActiveDoc - 1);
    else if (m_iActiveDoc == i) SetActiveDocIt(m_iActiveDoc);
    cIO_WWDx *meta = (cIO_WWDx *) dd->hParser->GetCustomMetaSerializer();
    delete meta;
    delete dd->hTab;
    delete dd->hTilesBank;
    delete dd->hSprBank;
    delete dd->hSndBank;
    delete dd->hAniBank;
    delete dd->hCustomLogicBank;
    delete dd->hDataCtrl;
    if (dd->hTileClipboard != NULL) {
        delete[] dd->hTileClipboard;
        dd->hTileClipboard = NULL;
        delete dd->hTileClipboardImageSet;
        dd->hTileClipboardImageSet = NULL;
    }

    for (int pl = 0; pl < dd->hParser->GetPlanesCount(); pl++) {
        if (!dd->hPlaneData[pl]->ObjectData.bEmpty) {
            for (int z = 0; z < dd->hParser->GetPlane(pl)->GetObjectsCount(); z++)
                delete GetUserDataFromObj(dd->hParser->GetPlane(pl)->GetObjectByIterator(z));
            delete dd->hPlaneData[pl]->ObjectData.hQuadTree;
        }
        delete dd->hPlaneData[pl];
    }
    dd->hPlaneData.clear();
    delete dd->hParser;
    delete[] dd->szFileName;
    delete dd;
    UpdateCrashList();
    if (m_vhDoc.size() == 0) {
        hContext->GetElementByID(MDI_CONTEXT_CLOSEALL)->SetEnabled(0);
    }
    hContext->GetElementByID(MDI_CONTEXT_CLOSEEXCEPTACTIVE)->SetEnabled(m_vhDoc.size() > 1);
    GV->editState->hAppMenu->SyncDocumentClosed();
    RebuildContext(0);

    UpdateMaxTabSize();
}

void cMDI::Think(bool bConsumed) {
    if (bBlock) return;
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    bool mout = false, mAtAdd = false;
    int iFocus = -2;

    if (my > m_iPosY && my < m_iPosY + 25 && !bConsumed) {
        int xoff = 0;
        for (int i = 0;/*-1;*/ i < int(m_vhDoc.size()); i++) {
            cTabMDI *tab = /*(i == -1 ? hDefTab : */m_vhDoc[i]->hTab;
            int w = tab->GetWidth();
            if (mx > xoff && mx <= xoff + w) {
                tab->bFocused = true;
                iFocus = i;
                if (hge->Input_KeyDown(HGEK_LBUTTON) && i != -1) {
                    if (tab->bCloseFocused) {
                        CloseDocByIt(i);
                        if (iFocus == i)
                            iFocus = -2;
                        --i;
                        continue;
                    }
                }
            } else
                tab->bFocused = false;
            xoff += w;
        }
        if (mx > xoff && mx <= xoff + 16) {
            mAtAdd = true;
        }
    } else
        mout = true;

    if (hContext->isVisible() && !(mx > hContext->getX() - 2 && mx < hContext->getX() + hContext->getWidth() + 2 &&
                                   my > hContext->getY() - 2 && my < hContext->getY() + hContext->getHeight() + 2) &&
        !(hContextClosed->isVisible() &&
          mx > hContextClosed->getX() && mx < hContextClosed->getX() + hContextClosed->getWidth() &&
          my > hContextClosed->getY() && my < hContextClosed->getY() + hContextClosed->getHeight()))
        hContext->setVisible(0);

    if (!mout && hge->Input_KeyUp(HGEK_RBUTTON)) {
        m_iContextMenuFocusedDoc = iFocus;
        hContext->GetElementByID(MDI_CONTEXT_CLOSEEXCEPTACTIVE)->SetEnabled(iFocus >= 0 && m_vhDoc.size() > 1);
        hContext->GetElementByID(MDI_CONTEXT_RELOAD)->SetEnabled(iFocus >= 0 && strlen(m_vhDoc[iFocus]->hParser->GetFilePath()) > 0);
        hContext->setPosition(mx, my);
        GV->editState->conMain->moveToTop(hContext);
        hContext->setVisible(1);
    }

    for (auto doc : m_vhDoc) {
        if (mout)
            doc->hTab->bFocused = false;
        doc->hTab->Update();
    }

    if (iFocus != -2) {
        if (hge->Input_KeyDown(HGEK_LBUTTON) && iFocus != m_iActiveDoc) {
            GV->editState->PrepareForDocumentSwitch();
            SetActiveDocIt(iFocus);
        }
    }

    /*if (mAtAdd) {
        if (fAddTabTimer < 0.2) {
            fAddTabTimer += hge->Timer_GetDelta();
            if (fAddTabTimer > 0.2) {
                fAddTabTimer = 0.2;
            }
        }

        if (hge->Input_KeyDown(HGEK_LBUTTON)) {
            GV->editState->PrepareForDocumentSwitch();
            SetActiveDocIt(-1);
        }
    } else {
        if (fAddTabTimer > 0.0) {
            fAddTabTimer -= hge->Timer_GetDelta();
            if (fAddTabTimer < 0.0) {
                fAddTabTimer = 0.0;
            }
        }
    }*/
}

bool cMDI::IsAnyDocUnsaved() {
    for (auto & doc : m_vhDoc) {
        if (!doc->bSaved)
            return true;
    }
    return false;
}

void cMDI::PrepareDocToSave(int i) {
    m_vhDoc[i]->iWapMapBuild = WA_VER;
    m_vhDoc[i]->strWapMapVersion = WA_VERPURE;
    m_vhDoc[i]->iMapBuild++;
    m_vhDoc[i]->hParser->UpdateDate();
    if (i == GetActiveDocIt())
        GV->editState->SyncWorldOptionsWithParser();
}

bool cMDI::CloseDocByIt(int i) {
    if (!m_vhDoc[i]->bSaved) {
        char text[256];
        sprintf(text, GETL2S("Various", "FileUnsavedPrompt"), m_vhDoc[i]->szFileName);
        int choice = MessageBox(hge->System_GetState(HGE_HWND), text, PRODUCT_NAME, MB_YESNOCANCEL | MB_ICONWARNING);
        if (choice == IDYES) {
            if (strlen(m_vhDoc[i]->hParser->GetFilePath()) == 0) {
                OPENFILENAME ofn;
                char szFileopen[512] = "\0";
                ZeroMemory((&ofn), sizeof(OPENFILENAME));
                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = hge->System_GetState(HGE_HWND);
                ofn.lpstrFilter = "WapWorld Document (*.wwd)\0*.wwd\0Wszystkie pliki (*.*)\0*.*\0\0";
                ofn.lpstrFile = szFileopen;
                ofn.nMaxFile = sizeof(szFileopen);
                ofn.lpstrDefExt = "wwd";
                ofn.lpstrInitialDir = GV->szLastSavePath;
                if (GetSaveFileName(&ofn)) {
                    m_vhDoc[i]->hParser->SetFilePath((const char *) szFileopen);
                    char *lastsave = SHR::GetDir(szFileopen);
                    GV->SetLastSavePath(lastsave);
                    delete[] lastsave;
                } else {
                    return 0;
                }
            }
            try {
                PrepareDocToSave(i);
                m_vhDoc[i]->hParser->CompileToFile(m_vhDoc[i]->hParser->GetFilePath());
            }
            catch (WWD::Exception &exc) {
#ifdef BUILD_DEBUG
                GV->Console->Printf("~r~WWD exception: ~y~%d ~w~(~y~%s~w~:~y~%d~w~)", exc.iErrorCode, exc.szFile, exc.iLine);
#else
                GV->Console->Printf("~r~WWD exception ~y~%d", exc.iErrorCode);
#endif
                sprintf(text, GETL2S("Various", "UnableToSave"), m_vhDoc[i]->szFileName);
                MessageBox(hge->System_GetState(HGE_HWND), text, PRODUCT_NAME, MB_OK | MB_ICONERROR);
            }
        } else if (choice == IDCANCEL) {
            return false;
        }
    }
    DeleteDocByIt(i);
    if (m_vhDoc.empty()) {
        GV->anyMapLoaded = false;
    }
    return true;
}

void cMDI::Render() {
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    hge->Gfx_RenderLine(0, m_iPosY, hge->System_GetState(HGE_SCREENWIDTH), m_iPosY, GV->colOutline);
    hge->Gfx_SetClipping();
    int xOffset = 0;
    for (int i = 0; i < m_vhDoc.size(); i++)
        xOffset += m_vhDoc[i]->hTab->Render(xOffset, m_iPosY, bBlock, m_iActiveDoc == i);
    m_vhDoc.back()->hTab->RenderSeparator(xOffset, m_iPosY, true);

    xOffset += m_iAddTabButtonOffset;

    /*if (fAddTabTimer < 0.2) {
        GV->sprTabAddButton->Render(xOffset, m_iPosY + m_iAddTabButtonOffset);
    }
    if (fAddTabTimer > 0.0) {
        GV->sprTabAddButtonFocused->SetColor(SETA(0xFFFFFF, fAddTabTimer * 5.f * 255.f));
        GV->sprTabAddButtonFocused->Render(xOffset, m_iPosY + m_iAddTabButtonOffset);
    }*/
}

void cMDI::SetActiveDoc(DocumentData *doc) {
    for (int i = 0; i < m_vhDoc.size(); i++) {
        if (m_vhDoc[i] == doc) {
            m_iActiveDoc = i;
            GV->editState->DocumentSwitched();
            return;
        }
    }
}

void cMDI::SetActiveDocIt(int it) {
    m_iLastActiveDoc = m_iActiveDoc;
    m_iActiveDoc = it;
    GV->editState->DocumentSwitched();
}

void cMDI::SaveCurrent() {
    if (!GetActiveDoc()->bSaved) {
        try {
            PrepareDocToSave(GetActiveDocIt());
            GV->editState->hParser->CompileToFile(GV->editState->hParser->GetFilePath());
            GetActiveDoc()->bSaved = true;

            char tmp[128];
            char *filename = SHR::GetFile(GV->editState->hParser->GetFilePath());
            sprintf(tmp, "%s - %s", filename, WA_TITLEBAR);
            delete[] filename;
            hge->System_SetState(HGE_TITLE, tmp);
        }
        catch (WWD::Exception &exc) {
#ifdef BUILD_DEBUG
            GV->Console->Printf("~r~WWD exception: ~y~%d ~w~(~y~%s~w~:~y~%d~w~)", exc.iErrorCode, exc.szFile, exc.iLine);
#else
            GV->Console->Printf("~r~WWD exception ~y~%d", exc.iErrorCode);
#endif
        }
    }
}

void cMDI::UpdateMaxTabSize() {
    cTabMDI::MaxTabWidth = m_vhDoc.empty() ? hge->System_GetState(HGE_SCREENWIDTH) : (hge->System_GetState(HGE_SCREENWIDTH) - 16 - m_iAddTabButtonOffset * 2) / m_vhDoc.size();
}

cTabMDI::cTabMDI() {
    dd = 0;
    bFocused = false;
    fTimer = 0;
    fCloseTimer = 0;
}

cTabMDI::~cTabMDI() {

}

void cTabMDI::Update() {
    /*if (bSelected && fTimer < 0.5f) {
        fTimer += hge->Timer_GetDelta();
        if (fTimer > 0.5f) fTimer = 0.5f;
        return;
    } else if (!bSelected && fTimer > 0.25f) {
        fTimer -= hge->Timer_GetDelta();
        if (fTimer < 0.25f) fTimer = 0.25f;
    }*/

    if (!bSelected && bFocused && fTimer < 0.2f) {
        fTimer += hge->Timer_GetDelta();
        if (fTimer > 0.2f) fTimer = 0.2f;
    } else if ((bSelected || !bFocused) && fTimer > 0) {
        fTimer -= hge->Timer_GetDelta();
        if (fTimer < 0) fTimer = 0;
    }
}

int cTabMDI::GetWidth() {
    /*if (dd == 0)
        return 36 ;*/

    std::string desc(dd->szFileName);
    if (!dd->bSaved) desc.push_back('*');
    return std::min(int(GV->fntMyriad16->GetStringWidth(desc.c_str()) + 55), MaxTabWidth);
}

int cTabMDI::Render(int x, int y, bool bdisabled, bool bselected) {
    bSelected = bselected;
    std::string desc(dd->szFileName);
    if (!dd->bSaved) desc.push_back('*');
    int sw = GV->fntMyriad16->GetStringWidth(desc.c_str());
    int tabWidth = GetWidth();

    if (bselected || fTimer > 0) {
        hgeQuad q;
        q.tex = 0;
        q.blend = BLEND_DEFAULT;
        SHR::SetQuad(&q, 0xFFFFFFFF, x, y, x + GetWidth(), y + LAY_MDI_H);
        q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = bselected ? 0xFF0F0F0F : SETA(0xFFFFFF, fTimer * 0x70);
        hge->Gfx_RenderQuad(&q);
    }

    if (!bselected) {
        RenderSeparator(x, y);
    }

    float mx, my;
    hge->Input_GetMousePos(&mx, &my);

    bCloseFocused = (!bdisabled && mx > x + tabWidth - 16 && my > y + 5 && mx < x + tabWidth - 5 && my < y + LAY_MDI_H - 5 &&
                         GV->editState->conMain->getWidgetAt(mx, my) == NULL);

    if (bCloseFocused && fCloseTimer < 0.2f) {
        fCloseTimer += hge->Timer_GetDelta();
        if (fCloseTimer > 0.2f) fCloseTimer = 0.2f;
    } else if (!bCloseFocused && fCloseTimer > 0.0f) {
        fCloseTimer -= hge->Timer_GetDelta();
        if (fCloseTimer < 0.0f) fCloseTimer = 0.0f;
    }

    hge->Gfx_SetClipping(x, y, tabWidth - 18, LAY_MDI_H);
    if (dd->hParser->GetGame() == WWD::Game_Claw && dd->hParser->GetBaseLevel() != 0) {
        GV->sprLevelsMicro16[dd->hParser->GetBaseLevel() - 1]->SetColor(0xFFFFFFFF);
        GV->sprLevelsMicro16[dd->hParser->GetBaseLevel() - 1]->Render(x + 9, y + 4);
    } else {
        GV->sprGamesSmall[dd->hParser->GetGame()]->SetColor(0xFFFFFFFF);
        GV->sprGamesSmall[dd->hParser->GetGame()]->Render(x + 9, y + 4);
    }

    GV->fntMyriad16->SetColor(SETA(0xe1e1e1, bdisabled ? 0x77 : 0xFF));
    GV->fntMyriad16->Render(x + 31, y + 5, HGETEXT_LEFT, desc.c_str(), 0);
    hge->Gfx_SetClipping();

    GV->sprTabCloseButton->SetColor(SETA(0xFFFFFF, bdisabled ? 0x77 : 0xFF));
    GV->sprTabCloseButton->Render(x + tabWidth - 16, y + 7);

    if (fCloseTimer > 0.f) {
        GV->sprTabCloseButtonFocused->SetColor(SETA(0xFFFFFF, fCloseTimer * 5.f * 128.f));
        GV->sprTabCloseButtonFocused->Render(x + tabWidth - 18, y + 6);
    }

    return tabWidth;
}

void cTabMDI::RenderSeparator(int x, int y, bool right) {
    if (index && (right ? !bSelected : !hMDI->m_vhDoc[index - 1]->hTab->bSelected)) {
        int alpha = 255 - (right ? fTimer : std::max(fTimer, hMDI->m_vhDoc[index - 1]->hTab->fTimer)) * 4 * 255;
        hge->Gfx_RenderLine(x, y + 2, x, y + LAY_MDI_H - 2, SETA(GV->colLineBright, alpha));
    }
}

void cTabMDI::RenderBG(int x, int y, int w, int st, bool bfirst, bool bclosed) {
    int xoff = 0;
    if (bfirst) {
        GV->hGfxInterface->sprBreadcrumb[st][0]->Render(x, y);
        xoff = 5;
    } else {
        GV->hGfxInterface->sprBreadcrumb[st][3]->Render(x, y);
        xoff = 14;
    }
    GV->hGfxInterface->sprBreadcrumb[st][1]->RenderStretch(x/* + xoff*/, y, x + w - 1 /*- (bclosed || bfirst) * 4 - 4*/, y + 26);
    if (bclosed)
        GV->hGfxInterface->sprBreadcrumb[st][4]->Render(x + w - 8, y);
    else
        GV->hGfxInterface->sprBreadcrumb[st][2]->Render(x + w - (bfirst * 4) - 4, y);
}
