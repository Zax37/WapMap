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
#include "states/dialog.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#ifdef WAPMAP_ENABLE_IMGUI
#include <imgui.h>
#include "legacyText.h"
#include "nativeContextMenu.h"
#include "cRulers.h"
#include <imgui_internal.h>
#endif

extern HGE *hge;

int cTabMDI::MaxTabWidth = 100;

void cMDI::action(const gcn::ActionEvent &actionEvent) {
    if (actionEvent.getSource() == hContext) {
        if (hContext->GetSelectedID() == MDI_CONTEXT_RELOAD) {
            if (CanBeClosed(m_iContextMenuFocusedDoc)) {
                bReloadingMap = true;
                GV->StateMgr->Push(new State::LoadMap(m_vhDoc[m_iContextMenuFocusedDoc]->hParser->GetFilePath()));
                hContext->setVisible(false);
            }
            return;
        } else if (hContext->GetSelectedID() == MDI_CONTEXT_PREVIOUSLYCLOSED)
            return;
        bool leaveCurrent = hContext->GetSelectedID() == MDI_CONTEXT_CLOSEEXCEPTACTIVE;
        int closeRight = leaveCurrent ? m_vhDoc.size() - m_iContextMenuFocusedDoc - 1 : m_vhDoc.size();
        int closeLeft = leaveCurrent ? m_iContextMenuFocusedDoc : 0;

        while (closeLeft--) {
            if (!CloseDocByIt(0)) {
                hContext->setVisible(false);
                return;
            }
        }

        while (closeRight--) {
            if (!CloseDocByIt(m_vhDoc.size() - 1)) {
                hContext->setVisible(false);
                return;
            }
        }
    } else if (actionEvent.getSource() == hContextClosed) {
        GV->editState->vstrMapsToLoad.emplace_back(vstrRecentlyClosed[hContextClosed->GetSelectedID()]);
    }
    hContext->setVisible(false);
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

    auto it = std::ranges::find(m_vhAutoFixedMaps, dd->hParser);
    if (it != m_vhAutoFixedMaps.end()) {
        m_vhAutoFixedMaps.erase(it);
        dd->bSaved = false;
    } else {
        dd->bSaved = true;
    }

    dd->fZoom = 1;

    dd->hTab = new cTabMDI();
    dd->hTab->dd = dd;
    dd->hTab->hMDI = this;
    dd->hTab->bFocused = 0;
    dd->hTab->fTimer = 0;

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
#ifdef WAPMAP_ENABLE_IMGUI
    dd->iViewportWidth = std::max(1, GV->editState->vPort->GetWidth());
    dd->iViewportHeight = std::max(1, GV->editState->vPort->GetHeight());
    dd->hViewport = new State::Viewport(GV->editState, 0, 0,
                                        dd->iViewportWidth, dd->iViewportHeight);
#endif
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
    if (vstrRecentlyClosed.empty() || bForceRebuildBase) {
        if (prevclosed || bForceRebuildBase) {
            hContext->ClearElements();
            hContext->AddElement(MDI_CONTEXT_CLOSEEXCEPTACTIVE, GETL2S("MDI", "Context_CloseAllExceptActive"),
                                 GV->sprIcons16[Icon16_XTransparent]);
            hContext->AddElement(MDI_CONTEXT_CLOSEALL, GETL2S("MDI", "Context_CloseAll"), GV->sprIcons16[Icon16_X]);
            hContext->AddElement(MDI_CONTEXT_RELOAD, GETL2S("MDI", "Context_Reload"),
                                 GV->sprIcons16[Icon16_AutoUpdate]);
            hContext->GetElementByID(MDI_CONTEXT_CLOSEEXCEPTACTIVE)->SetEnabled(m_vhDoc.size() > 1);
            hContext->GetElementByID(MDI_CONTEXT_CLOSEALL)->SetEnabled(!m_vhDoc.empty());
            hContext->adjustSize();
        }
    }
    if (vstrRecentlyClosed.empty()) return;
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
        if (ibase != 0 && (icon == WWD::Game_Claw || icon == WWD::Game_Gruntz)) {
            sprIcon = GV->sprLevelsMicro16[icon - WWD::Games_First][ibase - 1];
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
            if (vstrRecentlyClosed.size() >= 10) {
                vstrRecentlyClosed.pop_back();
            }
            vstrRecentlyClosed.insert(vstrRecentlyClosed.begin(), m_vhDoc[i]->hParser->GetFilePath());
        }
    }

    DocumentData *dd = m_vhDoc[i];
    if (m_pRequestedActiveDoc == dd) m_pRequestedActiveDoc = nullptr;
    if (m_pRequestedCloseDoc == dd) m_pRequestedCloseDoc = nullptr;
    if (m_pMapHoveredDoc == dd) m_pMapHoveredDoc = nullptr;
    if (m_pMapCapturedDoc == dd) m_pMapCapturedDoc = nullptr;

    m_vhDoc.erase(m_vhDoc.begin() + i);
    if (m_vhDoc.empty()) SetActiveDocIt(-1);
    else if (m_iActiveDoc > i) m_iActiveDoc--;
    else if (m_iActiveDoc == i && m_iActiveDoc >= m_vhDoc.size()) SetActiveDocIt(m_iActiveDoc - 1);
    else if (m_iActiveDoc == i) SetActiveDocIt(m_iActiveDoc);

    for (; i < m_vhDoc.size(); ++i) {
        --m_vhDoc[i]->hTab->index;
    }

    cIO_WWDx *meta = (cIO_WWDx *) dd->hParser->GetCustomMetaSerializer();
    delete meta;
    delete dd->hTab;
    delete dd->hViewport;
    delete dd->hTilesBank;
    delete dd->hSprBank;
    delete dd->hSndBank;
    delete dd->hAniBank;
    delete dd->hCustomLogicBank;
    delete dd->hDataCtrl;

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
    if (m_vhDoc.empty()) {
        hContext->GetElementByID(MDI_CONTEXT_CLOSEALL)->SetEnabled(false);
    }
    hContext->GetElementByID(MDI_CONTEXT_CLOSEEXCEPTACTIVE)->SetEnabled(m_vhDoc.size() > 1);
    GV->editState->hAppMenu->SyncDocumentClosed();
    RebuildContext(false);

    UpdateMaxTabSize();
}

void cMDI::Think(bool bConsumed) {
#ifdef WAPMAP_ENABLE_IMGUI
    ProcessImGuiActions();
    return;
#endif
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
        if (mx + hContext->getWidth() >= hge->System_GetState(HGE_SCREENWIDTH)) {
            hContext->setPosition(mx - hContext->getWidth(), my);
        } else {
            hContext->setPosition(mx, my);
        }
        GV->editState->conMain->moveToTop(hContext);
        hContext->setVisible(true);
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

bool cMDI::CanBeClosed(int i) {
    if (!m_vhDoc[i]->bSaved) {
        char text[256];
        sprintf(text, GETL2S("Various", "FileUnsavedPrompt"), m_vhDoc[i]->szFileName);
        switch (State::MessageBox(PRODUCT_NAME, text, ST_DIALOG_ICON_WARNING, ST_DIALOG_BUT_YESNOCANCEL)) {
            case RETURN_YES:
                if (strlen(m_vhDoc[i]->hParser->GetFilePath()) == 0) {
                    GV->editState->SaveAs();
                } else {
                    PrepareDocToSave(i);
                    m_vhDoc[i]->hParser->CompileToFile(m_vhDoc[i]->hParser->GetFilePath());
                }
                break;
            case RETURN_CANCEL:
                return false;
        }
    }
    return true;
}

bool cMDI::CloseDocByIt(int i) {
    if (!CanBeClosed(i)) return false;

    DeleteDocByIt(i);
    if (m_vhDoc.empty()) {
        GV->anyMapLoaded = false;
    }
    return true;
}

void cMDI::Render() {
#ifdef WAPMAP_ENABLE_IMGUI
    return;
#endif
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

#ifdef WAPMAP_ENABLE_IMGUI
void cMDI::DrawDocumentNavigation(DocumentData* doc, ImVec2 origin, ImVec2 imageMin, ImVec2 size, bool interactive) {
    auto* editor=GV->editState;
    auto* plane=doc->hParser->GetMainPlane();
    const float zoom=std::max(0.001f,doc->fZoom);
    const float bar=ImGui::GetStyle().ScrollbarSize;
    const ImVec2 end{imageMin.x+size.x,imageMin.y+size.y};
    auto* draw=ImGui::GetWindowDrawList();
    const auto background=IM_COL32(45,45,45,255),line=IM_COL32(145,145,145,255);
    ImGui::BeginDisabled(!interactive || (bBlock && doc!=GetActiveDoc()));
    float camera[2]{doc->fCamX,doc->fCamY};
    bool changed=false;
    for(int axis=0;axis<2;++axis) {
        const float visible=(axis?size.y:size.x)/zoom;
        const float extent=static_cast<float>(axis?plane->GetPlaneHeightPx():plane->GetPlaneWidthPx());
        const ImS64 contents=static_cast<ImS64>(std::ceil(std::max(visible,extent+2.f*WORKSPACE_MARGINS_SIZE)));
        const ImS64 page=std::max<ImS64>(1,static_cast<ImS64>(visible));
        ImS64 scroll=static_cast<ImS64>(std::round(camera[axis]+WORKSPACE_MARGINS_SIZE));
        const ImRect bounds=axis?ImRect(end.x,imageMin.y,end.x+bar,end.y):
                                     ImRect(imageMin.x,end.y,end.x,end.y+bar);
        if(ImGui::ScrollbarEx(bounds,ImGui::GetID(axis?"##map-scroll-y":"##map-scroll-x"),
                              axis?ImGuiAxis_Y:ImGuiAxis_X,&scroll,page,contents)) {
            camera[axis]=static_cast<float>(scroll)-WORKSPACE_MARGINS_SIZE;
            changed=true;
        }
    }
    draw->AddRectFilled(end,{end.x+bar,end.y+bar},background);
    if(changed) {
        m_pNavigationDoc=doc;
        m_fNavigationX=camera[0]; m_fNavigationY=camera[1];
    }
    if(editor->hRulers->IsVisible()) {
        draw->AddRectFilled(origin,{end.x,imageMin.y},background);
        draw->AddRectFilled(origin,{imageMin.x,end.y},background);
        for(int axis=0;axis<2;++axis) {
            const bool vertical=axis==1;
            const ImVec2 start=vertical?ImVec2(origin.x,imageMin.y):ImVec2(imageMin.x,origin.y);
            const ImVec2 rulerSize=vertical?ImVec2(imageMin.x-origin.x,size.y):ImVec2(size.x,imageMin.y-origin.y);
            ImGui::SetCursorScreenPos(start);
            ImGui::InvisibleButton(vertical?"##ruler-y":"##ruler-x",rulerSize);
            if(interactive && !bBlock && editor->bShowGuideLines && editor->iActiveTool==EWW_TOOL_NONE && ImGui::IsItemActivated()) {
                m_pGuideDragDoc=doc;
                m_bGuideHorizontal=!vertical;
                m_pRequestedActiveDoc=doc;
            }
            const float tile=static_cast<float>(vertical?plane->GetTileHeight():plane->GetTileWidth());
            const bool tiles=editor->GetActiveMode()==EWW_MODE_TILE;
            float step=tiles?std::max(1.f,tile):10.f;
            while(step*zoom<60.f) step*=2.f;
            const float cam=vertical?doc->fCamY:doc->fCamX;
            const float length=vertical?size.y:size.x;
            draw->PushClipRect(start,{start.x+rulerSize.x,start.y+rulerSize.y},true);
            const int first=static_cast<int>(std::floor(cam/step));
            for(int tick=first;tick<=first+static_cast<int>(length/(step*zoom))+2;++tick) {
                const float world=tick*step;
                const float offset=(world-cam)*zoom;
                const ImVec2 mark=vertical?ImVec2(imageMin.x,start.y+offset):ImVec2(start.x+offset,imageMin.y);
                draw->AddLine(vertical?ImVec2(mark.x-6,mark.y):ImVec2(mark.x,mark.y-6),mark,line);
                char text[32];
                const float value=tiles?world/std::max(1.f,tile):world;
                if(vertical && std::abs(value)>=1000.f) std::snprintf(text,sizeof(text),"%.1fk",value/1000.f);
                else std::snprintf(text,sizeof(text),"%.0f",value);
                if(vertical) draw->AddText({origin.x+2,mark.y+2},line,text);
                else draw->AddText({mark.x+3,origin.y+2},line,text);
            }
            draw->PopClipRect();
        }
        draw->AddLine({imageMin.x,origin.y},{imageMin.x,end.y},IM_COL32(80,80,80,255));
        draw->AddLine({origin.x,imageMin.y},{end.x,imageMin.y},IM_COL32(80,80,80,255));
    }
    ImGui::EndDisabled();
    if(m_pGuideDragDoc==doc) {
        const auto mouse=ImGui::GetMousePos();
        const bool inside=mouse.x>=imageMin.x && mouse.x<end.x && mouse.y>=imageMin.y && mouse.y<end.y;
        if(!interactive) m_pGuideDragDoc=nullptr;
        else if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            if(inside) {
                m_pGuideActionDoc=doc;
                m_guideAction.bOrient=m_bGuideHorizontal;
                m_guideAction.iPos=static_cast<int>(std::round(m_bGuideHorizontal?
                    doc->fCamY+(mouse.y-imageMin.y)/zoom:doc->fCamX+(mouse.x-imageMin.x)/zoom));
                if(m_guideAction.iPos<0) m_pGuideActionDoc=nullptr;
            }
            m_pGuideDragDoc=nullptr;
        } else if(ImGui::IsMouseDown(ImGuiMouseButton_Left) && inside) {
            draw->AddLine(m_bGuideHorizontal?ImVec2(imageMin.x,mouse.y):ImVec2(mouse.x,imageMin.y),
                          m_bGuideHorizontal?ImVec2(end.x,mouse.y):ImVec2(mouse.x,end.y),IM_COL32(40,190,255,255));
        } else if(!ImGui::IsMouseDown(ImGuiMouseButton_Left)) m_pGuideDragDoc=nullptr;
    }
}

void cMDI::DrawImGuiViews(const unsigned int dockspaceId) {
    const bool interactive=GV->editState->_isOnTop();
    DocumentData* requestedContextDoc=nullptr;
    m_pMapHoveredDoc = nullptr;
    m_iMainViewportDockedDocCount = 0;
    const auto& io = ImGui::GetIO();
    const auto codePage = LegacyText::CodePage(GV->Lang->GetCode());
    const ImGuiID mainViewportId = ImGui::GetMainViewport()->ID;
    bool mapWindowFocused = false;
    ImGuiWindowClass documentViewportClass;
    documentViewportClass.ViewportFlagsOverrideSet = ImGuiViewportFlags_CanHostOtherWindows;
    documentViewportClass.ViewportFlagsOverrideClear = ImGuiViewportFlags_NoTaskBarIcon;
    const auto& platformViewports = ImGui::GetPlatformIO().Viewports;
    for (int viewportIndex = 0; viewportIndex < platformViewports.Size; ++viewportIndex) {
        const auto* viewport = platformViewports[viewportIndex];
        if (viewport->ID == mainViewportId) continue;
        const bool containsMap = std::any_of(m_vhDoc.begin(), m_vhDoc.end(),
            [viewport](const DocumentData* doc) { return doc->iImGuiViewportId == viewport->ID; });
        if (containsMap) {
            ImGui::DockSpaceOverViewport(0, viewport, ImGuiDockNodeFlags_PassthruCentralNode,
                                         &documentViewportClass);
        }
    }
    for (auto* doc : m_vhDoc) {
        const char* filePath = doc->hParser->GetFilePath();
        std::string stableId;
        if (filePath && *filePath) stableId = filePath;
        else stableId = "untitled-" + std::to_string(reinterpret_cast<std::uintptr_t>(doc));
        auto title = LegacyText::ToUtf8(doc->szFileName, codePage);
        if (!doc->bSaved) title.push_back('*');
        title += "###MapView-" + stableId;

        ImGui::PushID(stableId.c_str());
        ImGuiWindowClass documentWindowClass;
        documentWindowClass.ViewportFlagsOverrideSet = ImGuiViewportFlags_CanHostOtherWindows;
        documentWindowClass.ViewportFlagsOverrideClear = ImGuiViewportFlags_NoTaskBarIcon;
        ImGui::SetNextWindowClass(&documentWindowClass);
        ImGui::SetNextWindowDockID(dockspaceId, ImGuiCond_FirstUseEver);
        bool open = doc->bImGuiOpen;
        ImGui::PushStyleColor(ImGuiCol_ResizeGrip, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, IM_COL32(0, 0, 0, 0));
        const auto flags=ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | (interactive?0:
            ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{0,0});
        const bool visible = ImGui::Begin(title.c_str(), interactive?&open:nullptr, flags);
        ImGui::PopStyleVar();
        // Begin exposes the title bar or dock tab even for inactive tabs.
        if(interactive && !bBlock && ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
            requestedContextDoc=doc;
        doc->iImGuiViewportId = ImGui::GetWindowViewport()->ID;
        doc->bImGuiOpen = open;
        if (open && ImGui::IsWindowDocked() &&
            ImGui::GetWindowViewport()->ID == mainViewportId)
            ++m_iMainViewportDockedDocCount;
        if (visible && open) {
            if (interactive && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
                mapWindowFocused = true;
                m_pRequestedActiveDoc = doc;
            }
            const ImVec2 available = ImGui::GetContentRegionAvail();
            const ImVec2 origin=ImGui::GetCursorScreenPos();
            const float ruler=GV->editState->hRulers->IsVisible()?24.f:0.f;
            const float scrollbar=ImGui::GetStyle().ScrollbarSize;
            const ImVec2 imageSize{std::max(1.f,available.x-ruler-scrollbar),std::max(1.f,available.y-ruler-scrollbar)};
            doc->iViewportWidth = static_cast<int>(imageSize.x);
            doc->iViewportHeight = static_cast<int>(imageSize.y);
            ImGui::SetCursorScreenPos({origin.x+ruler,origin.y+ruler});
            const bool composite = doc == GetActiveDoc();
            const unsigned int texture = doc->hViewport
                ? doc->hViewport->GetTextureHandle(composite)
                : 0;
            if (texture) {
                ImGui::Image(static_cast<ImTextureID>(texture),
                             imageSize,
                             {0.0f, 1.0f}, {1.0f, 0.0f});
                const ImVec2 imageMin = ImGui::GetItemRectMin();
                if (interactive && ImGui::IsItemHovered()) {
                    m_pMapHoveredDoc = doc;
                    m_fMapInputOriginX = imageMin.x;
                    m_fMapInputOriginY = imageMin.y;
                    m_iMapInputViewportId = ImGui::GetWindowViewport()->ID;
                    if (io.MouseWheel != 0.0f) {
                        m_iPendingMapWheel += static_cast<int>(std::round(io.MouseWheel));
                    }
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) ||
                        ImGui::IsMouseClicked(ImGuiMouseButton_Right) ||
                        ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
                        m_pMapCapturedDoc = doc;
                        m_fMapCaptureOriginX = imageMin.x;
                        m_fMapCaptureOriginY = imageMin.y;
                        m_iMapCaptureViewportId = ImGui::GetWindowViewport()->ID;
                        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                            m_iPendingMapMouseDown |= 1u << ImGuiMouseButton_Left;
                        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                            m_iPendingMapMouseDown |= 1u << ImGuiMouseButton_Right;
                        if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
                            m_iPendingMapMouseDown |= 1u << ImGuiMouseButton_Middle;
                    }
                }
            } else {
                ImGui::TextUnformatted("Map viewport texture is unavailable.");
            }
            DrawDocumentNavigation(doc,origin,{origin.x+ruler,origin.y+ruler},imageSize,interactive);
        }
        ImGui::End();
        ImGui::PopStyleColor(3);
        ImGui::PopID();
        if (interactive && !open) m_pRequestedCloseDoc = doc;
    }
    if(requestedContextDoc) {
        m_pContextDoc=requestedContextDoc;
        ImGui::OpenPopup("###DocumentContext");
    }
    if(ImGui::BeginPopup("###DocumentContext")) {
        if(std::find(m_vhDoc.begin(),m_vhDoc.end(),m_pContextDoc)==m_vhDoc.end() || !interactive || bBlock) {
            ImGui::CloseCurrentPopup();
        } else {
            hContext->GetElementByID(MDI_CONTEXT_CLOSEEXCEPTACTIVE)->SetEnabled(m_vhDoc.size()>1 && !bBlock);
            hContext->GetElementByID(MDI_CONTEXT_CLOSEALL)->SetEnabled(!bBlock);
            hContext->GetElementByID(MDI_CONTEXT_RELOAD)->SetEnabled(!bBlock && strlen(m_pContextDoc->hParser->GetFilePath())>0);
            ImGui::BeginDisabled(bBlock || m_pContextAction!=nullptr);
            WapMapImGui::ContextMenu(hContext,*hge,[&](SHR::Context* context,int id) {
                m_pContextAction=context;
                m_iContextActionId=id;
                m_pContextActionDoc=m_pContextDoc;
            });
            ImGui::EndDisabled();
        }
        ImGui::EndPopup();
    }
    hge->Input_SetKeyboardCaptureOverride(mapWindowFocused);
    if (!mapWindowFocused) m_pRequestedActiveDoc = nullptr;
}

void cMDI::RouteImGuiInput() {
    // The map input widget is placed at (0,0) in legacy coordinates. Leaving
    // it visible over the main window intercepts welcome-screen links.
    struct MapInputVisibility {
        cMDI* mdi;
        bool visible=false;
        ~MapInputVisibility() {
            if(auto* viewport=GV->editState->vPort)
                viewport->GetWidget()->setVisible(visible);
        }
    } inputVisibility{this};
    DocumentData* doc = m_pMapCapturedDoc ? m_pMapCapturedDoc : m_pMapHoveredDoc;
    if (!doc) return;
    if (std::find(m_vhDoc.begin(), m_vhDoc.end(), doc) == m_vhDoc.end()) {
        m_pMapCapturedDoc = nullptr;
        m_pMapHoveredDoc = nullptr;
        return;
    }
    if (!doc->hViewport) return;

    const auto& io = ImGui::GetIO();
    bool left = io.MouseDown[ImGuiMouseButton_Left];
    bool right = io.MouseDown[ImGuiMouseButton_Right];
    bool middle = io.MouseDown[ImGuiMouseButton_Middle];
    if (bBlock && doc != GetActiveDoc()) {
        if (m_pMapCapturedDoc && !left && !right && !middle) m_pMapCapturedDoc = nullptr;
        return;
    }
    const float originX = m_pMapCapturedDoc ? m_fMapCaptureOriginX : m_fMapInputOriginX;
    const float originY = m_pMapCapturedDoc ? m_fMapCaptureOriginY : m_fMapInputOriginY;
    float mouseX = io.MousePos.x;
    float mouseY = io.MousePos.y;
    const auto* mainViewport = ImGui::GetMainViewport();
    hge->Input_GetPhysicalMouseButtons(&left, &right, &middle);
    left = left || (m_iPendingMapMouseDown & (1u << ImGuiMouseButton_Left));
    right = right || (m_iPendingMapMouseDown & (1u << ImGuiMouseButton_Right));
    middle = middle || (m_iPendingMapMouseDown & (1u << ImGuiMouseButton_Middle));
    if (m_pMapCapturedDoc
            ? m_iMapCaptureViewportId == mainViewport->ID
            : m_iMapInputViewportId == mainViewport->ID) {
        hge->Input_GetPhysicalMousePos(&mouseX, &mouseY);
        mouseX += mainViewport->Pos.x - originX;
        mouseY += mainViewport->Pos.y - originY;
    } else {
        hge->Input_GetPhysicalMouseScreenPos(&mouseX, &mouseY);
        mouseX -= originX;
        mouseY -= originY;
    }
    if (!m_pMapCapturedDoc &&
        (mouseX < 0.0f || mouseY < 0.0f ||
         mouseX >= static_cast<float>(doc->iViewportWidth) ||
         mouseY >= static_cast<float>(doc->iViewportHeight))) {
        m_pMapHoveredDoc = nullptr;
        m_iPendingMapWheel = 0;
        return;
    }
    const float scaleX = static_cast<float>(doc->hViewport->GetWidth()) /
                         std::max(1, doc->iViewportWidth);
    const float scaleY = static_cast<float>(doc->hViewport->GetHeight()) /
                         std::max(1, doc->iViewportHeight);
    inputVisibility.visible=doc==GetActiveDoc();
    hge->Input_SetMouseStateOverride(mouseX * scaleX, mouseY * scaleY,
                                     left, right, middle, m_iPendingMapWheel);
    m_iPendingMapMouseDown = 0;
    m_iPendingMapWheel = 0;

    if (m_pMapCapturedDoc && !left && !right && !middle) m_pMapCapturedDoc = nullptr;
}

void cMDI::ProcessImGuiActions() {
    if(m_pNavigationDoc && !bBlock) m_pRequestedActiveDoc=m_pNavigationDoc;
    if(m_pContextAction) {
        auto* context=m_pContextAction;
        auto* doc=m_pContextActionDoc;
        const int id=m_iContextActionId;
        m_pContextAction=nullptr;
        m_pContextActionDoc=nullptr;
        const auto it=std::find(m_vhDoc.begin(),m_vhDoc.end(),doc);
        if(!bBlock && it!=m_vhDoc.end() && (context==hContext || context==hContextClosed)) {
            auto* entry=context->GetElementByID(id);
            if(entry && entry->IsEnabled()) {
                const int index=static_cast<int>(it-m_vhDoc.begin());
                if(index!=m_iActiveDoc) {
                    GV->editState->PrepareForDocumentSwitch();
                    SetActiveDocIt(index);
                }
                m_iContextMenuFocusedDoc=index;
                m_pRequestedActiveDoc=nullptr;
                context->EmulateClickID(id);
                hContext->hide();
                hContextClosed->hide();
                return;
            }
        }
    }
    if (m_pRequestedActiveDoc) {
        const auto it = std::find(m_vhDoc.begin(), m_vhDoc.end(), m_pRequestedActiveDoc);
        if (!bBlock && it != m_vhDoc.end()) {
            const int requestedIndex = static_cast<int>(it - m_vhDoc.begin());
            if (requestedIndex != m_iActiveDoc) {
                GV->editState->PrepareForDocumentSwitch();
                SetActiveDocIt(requestedIndex);
            }
            if (m_pRequestedActiveDoc->hViewport &&
                (m_pMapHoveredDoc==m_pRequestedActiveDoc || m_pMapCapturedDoc==m_pRequestedActiveDoc)) {
                m_pRequestedActiveDoc->hViewport->GetWidget()->setVisible(true);
                m_pRequestedActiveDoc->hViewport->GetWidget()->requestFocus();
            }
        }
        m_pRequestedActiveDoc = nullptr;
    }

    if(m_pNavigationDoc) {
        auto* doc=m_pNavigationDoc;
        m_pNavigationDoc=nullptr;
        if(doc==GetActiveDoc()) {
            doc->fCamX=GV->editState->fCamX=m_fNavigationX;
            doc->fCamY=GV->editState->fCamY=m_fNavigationY;
            doc->hViewport->MarkToRedraw();
        }
    }
    if(m_pGuideActionDoc) {
        if(std::find(m_vhDoc.begin(),m_vhDoc.end(),m_pGuideActionDoc)!=m_vhDoc.end())
            m_pGuideActionDoc->vGuides.push_back(m_guideAction);
        m_pGuideActionDoc=nullptr;
    }
    if (m_pRequestedCloseDoc) {
        DocumentData* closingDoc = m_pRequestedCloseDoc;
        const auto it = std::find(m_vhDoc.begin(), m_vhDoc.end(), closingDoc);
        m_pRequestedCloseDoc = nullptr;
        if (it != m_vhDoc.end()) {
            const int closingIndex = static_cast<int>(it - m_vhDoc.begin());
            if (!CloseDocByIt(closingIndex)) closingDoc->bImGuiOpen = true;
        }
    }
}
#endif

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

bool cMDI::IsDocAlreadyOpen(const std::string& path) {
    for (auto dd : m_vhDoc) {
        if (path == dd->hParser->GetFilePath()) {
            GV->editState->PrepareForDocumentSwitch();
            SetActiveDoc(dd);
            return true;
        }
    }
    return false;
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
    if ((dd->hParser->GetGame() == WWD::Game_Claw || dd->hParser->GetGame() == WWD::Game_Gruntz) && dd->hParser->GetBaseLevel() != 0) {
        GV->sprLevelsMicro16[dd->hParser->GetGame() - WWD::Games_First][dd->hParser->GetBaseLevel() - 1]->SetColor(0xFFFFFFFF);
        GV->sprLevelsMicro16[dd->hParser->GetGame() - WWD::Games_First][dd->hParser->GetBaseLevel() - 1]->Render(x + 9, y + 4);
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
