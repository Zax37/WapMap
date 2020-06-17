#include "../editing_ww.h"
#include "../../globals.h"
#include "../../../shared/commonFunc.h"
#include "../loadmap.h"
#include "../../langID.h"
#include "../error.h"
#include "../../cObjectUserData.h"
#include "../../returncodes.h"
#include "../../cAppMenu.h"
#include "../../../shared/gcnWidgets/wComboButton.h"
#include "../../cNativeController.h"
#include <cmath>
#include "../../databanks/logics.h"
#include "../../databanks/tiles.h"
#include "../../databanks/sounds.h"
#include "../../databanks/anims.h"
#include "../../databanks/imageSets.h"
#include "../../windows/window.h"
#include "../../version.h"

extern HGE *hge;

namespace State {
    void EditingWWActionListener::action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == m_hOwn->winLogicBrowser) {
            m_hOwn->SyncLogicBrowser();
        } else if (actionEvent.getSource() == m_hOwn->tfbrlRename && actionEvent.getId() == "ENTER" ||
                   actionEvent.getSource() == m_hOwn->butbrlRenameOK) {
            //m_hOwn->hCustomLogics->GetLogicByIterator(m_hOwn->lbbrlLogicList->getSelected())->Rename(m_hOwn->tfbrlRename);
            cCustomLogic *logic = m_hOwn->hCustomLogics->GetLogicByIterator(m_hOwn->lbbrlLogicList->getSelected());
            std::string strOldName = logic->GetName(),
                    strNewName = m_hOwn->tfbrlRename->getText();
            if (m_hOwn->hCustomLogics->RenameLogic(logic, m_hOwn->tfbrlRename->getText())) {
                m_hOwn->SyncLogicBrowser();
                bool bObjFnd = 0;
                for (int i = 0; i < m_hOwn->plMain->GetObjectsCount(); i++)
                    if (!strcmp(m_hOwn->plMain->GetObjectByIterator(i)->GetLogic(), "CustomLogic") &&
                        !strcmp(m_hOwn->plMain->GetObjectByIterator(i)->GetName(), strOldName.c_str())) {
                        bObjFnd = 1;
                        break;
                    }
                if (bObjFnd &&
                    MessageBox(hge->System_GetState(HGE_HWND), GETL2S("Win_LogicBrowser", "RenamePrompt"),
                               PRODUCT_NAME, MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    for (int i = 0; i < m_hOwn->plMain->GetObjectsCount(); i++)
                        if (!strcmp(m_hOwn->plMain->GetObjectByIterator(i)->GetLogic(), "CustomLogic") &&
                            !strcmp(m_hOwn->plMain->GetObjectByIterator(i)->GetName(), strOldName.c_str())) {
                            m_hOwn->plMain->GetObjectByIterator(i)->SetName(strNewName.c_str());
                        }
                    m_hOwn->MarkUnsaved();
                }
            } else {
                MessageBox(hge->System_GetState(HGE_HWND), GETL2S("Win_LogicBrowser", "RenameError"),
                           PRODUCT_NAME, MB_OK | MB_ICONWARNING);
            }
        } else if (actionEvent.getSource() == m_hOwn->butbrlRename) {
            m_hOwn->tfbrlRename->setVisible(1);
            m_hOwn->butbrlRenameOK->setVisible(1);
            m_hOwn->labbrlLogicNameV->setVisible(0);
            m_hOwn->tfbrlRename->setText(
                    m_hOwn->hCustomLogics->GetLogicByIterator(m_hOwn->lbbrlLogicList->getSelected())->GetName());
            m_hOwn->tfbrlRename->requestFocus();
        } else if (actionEvent.getSource() == m_hOwn->butbrlBrowseDir) {
            std::string path = m_hOwn->hDataCtrl->GetFeed(DB_FEED_CUSTOM)->GetAbsoluteLocation() + "/LOGICS";
            ShellExecute(hge->System_GetState(HGE_HWND), "explore", "", "", path.c_str(), SW_SHOWNORMAL);
        }
            /*else if (actionEvent.getSource() == m_hOwn->butbrlNew) {
                // REMOVED
                if (strlen(GV->editState->hParser->GetFilePath()) == 0) {
                    MessageBox(hge->System_GetState(HGE_HWND), GETL2S("Win_LogicBrowser", "NewLogicDocumentSave"), PRODUCT_NAME, MB_OK | MB_ICONERROR);
                    return;
                }
                //GV->StateMgr->Push(new State::CodeEditor(0, 1, ""));
            }*/
        else if (actionEvent.getSource() == m_hOwn->butbrlEdit) {
            cCustomLogic *logic = m_hOwn->hCustomLogics->GetLogicByIterator(m_hOwn->lbbrlLogicList->getSelected());
            GV->editState->hDataCtrl->OpenCodeEditor(logic->GetName());
        }
            //else if (actionEvent.getSource() == m_hOwn->butbrlEditExternal) {
            // REMOVED
            /*cCustomLogic * logic = m_hOwn->hCustomLogics->GetLogicByIterator(m_hOwn->lbbrlLogicList->getSelected());
            std::string path = m_hOwn->hDataCtrl->GetFeed(DB_FEED_CUSTOM)->GetAbsoluteLocation() + "/LOGICS/" + logic->GetName() + ".lua";
            printf("%d\n", ShellExecute(hge->System_GetState(HGE_HWND), "open", path.c_str(), "", "", SW_SHOWNORMAL));*/
            //}
        else if (actionEvent.getSource() == m_hOwn->butbrlDelete) {
            cCustomLogic *logic = m_hOwn->hCustomLogics->GetLogicByIterator(m_hOwn->lbbrlLogicList->getSelected());
            std::string path =
                    m_hOwn->hDataCtrl->GetFeed(DB_FEED_CUSTOM)->GetAbsoluteLocation() + "/LOGICS/" + logic->GetName() +
                    ".lua";
            char tmp[512];
            sprintf(tmp, GETL2S("Win_LogicBrowser", "DeleteWarning"), path.c_str());
            if (MessageBox(hge->System_GetState(HGE_HWND), tmp, PRODUCT_NAME, MB_YESNO | MB_ICONWARNING) == IDYES) {
                logic->DeleteFile();
                m_hOwn->SyncLogicBrowser();
            }
        } else if (actionEvent.getSource() == m_hOwn->hmbTile->butIconSelect) {
            m_hOwn->SetTool(EWW_TOOL_NONE);
        } else if (actionEvent.getSource() == m_hOwn->hmbTile->butIconWriteID) {
            m_hOwn->SetTool(EWW_TOOL_WRITEID);
        } else if (actionEvent.getSource() == m_hOwn->tfWriteID) {
            if (m_hOwn->tfWriteID->getActionEventId() == "ENTER") {
                m_hOwn->TextEditMoveToNextTile(true);
            }
        } else if (actionEvent.getSource() == m_hOwn->hmbTile->butIconPencil ||
                   actionEvent.getSource() == m_hOwn->buttpiModePencil) {
            m_hOwn->SetTool(EWW_TOOL_PENCIL);
        } else if (actionEvent.getSource() == m_hOwn->hmbTile->butIconBrush ||
                   actionEvent.getSource() == m_hOwn->buttpiModeBrush) {
            m_hOwn->SetTool(EWW_TOOL_BRUSH);
        } else if (actionEvent.getSource() == m_hOwn->hmbTile->butIconFill ||
                   actionEvent.getSource() == m_hOwn->buttpiFlood) {
            m_hOwn->SetTool(EWW_TOOL_FILL);
        } else if (actionEvent.getSource() == m_hOwn->buttpiPoint) {
            m_hOwn->iTileDrawMode = EWW_DRAW_POINT;
            if (m_hOwn->iActiveTool == EWW_TOOL_FILL)
                m_hOwn->SetTool(EWW_TOOL_PENCIL);
            m_hOwn->RebuildTilePicker();
        } else if (actionEvent.getSource() == m_hOwn->winTilePicker) {
            m_hOwn->SetTool(EWW_TOOL_NONE);
        } else if (actionEvent.getSource() == m_hOwn->buttpiLine) {
            m_hOwn->iTileDrawMode = EWW_DRAW_LINE;
            if (m_hOwn->iActiveTool == EWW_TOOL_FILL)
                m_hOwn->SetTool(EWW_TOOL_PENCIL);
            m_hOwn->RebuildTilePicker();
        } else if (actionEvent.getSource() == m_hOwn->buttpiRect) {
            m_hOwn->iTileDrawMode = EWW_DRAW_RECT;
            if (m_hOwn->iActiveTool == EWW_TOOL_FILL)
                m_hOwn->SetTool(EWW_TOOL_PENCIL);
            m_hOwn->RebuildTilePicker();
        } else if (actionEvent.getSource() == m_hOwn->buttpiEllipse) {
            m_hOwn->iTileDrawMode = EWW_DRAW_ELLIPSE;
            if (m_hOwn->iActiveTool == EWW_TOOL_FILL)
                m_hOwn->SetTool(EWW_TOOL_PENCIL);
            m_hOwn->RebuildTilePicker();
        } else if (actionEvent.getSource() == m_hOwn->buttpiSpray) {
            m_hOwn->iTileDrawMode = EWW_DRAW_SPRAY;
            if (m_hOwn->iActiveTool == EWW_TOOL_FILL)
                m_hOwn->SetTool(EWW_TOOL_PENCIL);
            m_hOwn->RebuildTilePicker();
        } else if (actionEvent.getSource() == m_hOwn->slitpiPointSize) {
            char tmp[128];
            sprintf(tmp, "%s: %d", GETL2S("TilePicker", "PointSize"), int(m_hOwn->slitpiPointSize->getValue()));
            m_hOwn->labtpiPointSize->setCaption(tmp);
            m_hOwn->labtpiPointSize->adjustSize();
        } else if (actionEvent.getSource() == m_hOwn->slitpiLineThickness) {
            char tmp[128];
            sprintf(tmp, "%s: %d", GETL2S("TilePicker", "LineThickness"), int(m_hOwn->slitpiLineThickness->getValue()));
            m_hOwn->labtpiLineThickness->setCaption(tmp);
            m_hOwn->labtpiLineThickness->adjustSize();
        } else if (actionEvent.getSource() == m_hOwn->slitpiSpraySize) {
            char tmp[128];
            sprintf(tmp, "%s: %d", GETL2S("TilePicker", "SpraySize"), int(m_hOwn->slitpiSpraySize->getValue()));
            m_hOwn->labtpiSpraySize->setCaption(tmp);
            m_hOwn->labtpiSpraySize->adjustSize();
        } else if (actionEvent.getSource() == m_hOwn->slitpiSprayDensity) {
            char tmp[128];
            sprintf(tmp, "%s: %d%%", GETL2S("TilePicker", "SprayDensity"), int(m_hOwn->slitpiSprayDensity->getValue()));
            m_hOwn->labtpiSprayDensity->setCaption(tmp);
            m_hOwn->labtpiSprayDensity->adjustSize();
        } else if (actionEvent.getSource() == m_hOwn->sliZoom) {
            float sliz = 1;
            if (m_hOwn->sliZoom->getValue() > 1) {
                sliz += m_hOwn->sliZoom->getValue() / 5.0f;
            } else if (m_hOwn->sliZoom->getValue() < 1) {
                sliz += m_hOwn->sliZoom->getValue() / 5.0f * 0.9f;
            }
            if (GV->bSmoothZoom) {
                m_hOwn->fDestZoom = sliz;
            } else {
                m_hOwn->fZoom = sliz;
            }
        } else if (actionEvent.getSource() == m_hOwn->cbutActiveMode) {
            if (m_hOwn->cbutActiveMode->getSelectedEntryID() == 1) {
                if (!(m_hOwn->GetActivePlane()->GetFlags() & WWD::Flag_p_MainPlane)) {
                    int pid = -1;
                    for (int i = 0; i < m_hOwn->hParser->GetPlanesCount(); i++)
                        if (m_hOwn->hParser->GetPlane(i) == m_hOwn->plMain) {
                            pid = i;
                            break;
                        }
                    if (pid == -1) return;
                    m_hOwn->hmbTile->ddActivePlane->setSelected(pid);
                }
                m_hOwn->SetTool(EWW_TOOL_NONE);
                m_hOwn->SetMode(EWW_MODE_OBJECT);
            } else {
                m_hOwn->SetTool(EWW_TOOL_NONE);
                m_hOwn->SetMode(EWW_MODE_TILE);
            }
        } else if (actionEvent.getSource() == m_hOwn->winMeasureOpt) {
            m_hOwn->SetTool(EWW_TOOL_NONE);
        } else if (actionEvent.getSource() == m_hOwn->winObjectBrush) {
            m_hOwn->iActiveTool = EWW_TOOL_NONE;
        } else if (actionEvent.getSource() == m_hOwn->welcomeScreenOptions[NewDocument]) {
            GV->editState->NewMap_Open();
        } else if (actionEvent.getSource() == m_hOwn->welcomeScreenOptions[OpenExisting]) {
            GV->editState->OpenDocuments();
        } else if (actionEvent.getSource() == m_hOwn->welcomeScreenOptions[WhatsNew]) {
            char tmp[256];
            sprintf(tmp, "Changelog.html", GV->Lang->GetCode());
            printf(tmp);
            ShellExecute(hge->System_GetState(HGE_HWND), "open", tmp, NULL, NULL, SW_SHOWNORMAL);
        } else if (actionEvent.getSource() == m_hOwn->lbpmPlanes) {
            m_hOwn->SyncPlaneProperties();
        } else if (m_hOwn->FirstRun_data != 0 && actionEvent.getSource() == m_hOwn->FirstRun_data->win) {
            m_hOwn->FirstRun_data->bKill = 1;
        } else if (m_hOwn->FirstRun_data != 0 && actionEvent.getSource() == m_hOwn->FirstRun_data->setClawDir) {
            m_hOwn->FirstRun_Action(0);
        } else if (m_hOwn->FirstRun_data != 0 && actionEvent.getSource() == m_hOwn->FirstRun_data->website) {
            m_hOwn->FirstRun_Action(1);
        } else if (m_hOwn->NewMap_data != 0 && actionEvent.getSource() == m_hOwn->NewMap_data->winNewMap) {
            m_hOwn->NewMap_data->bKill = 1;
        } else if (m_hOwn->NewMap_data != 0 && actionEvent.getSource() == m_hOwn->NewMap_data->butOK) {
            m_hOwn->NewMap_OK();
        } else if (m_hOwn->NewMap_data != 0 && actionEvent.getSource() == m_hOwn->NewMap_data->tfName) {
            m_hOwn->NewMap_data->tfName->setMarkedInvalid(strlen(m_hOwn->NewMap_data->tfName->getText().c_str()) == 0 ||
                                                          !m_hOwn->ValidateLevelName(
                                                                  m_hOwn->NewMap_data->tfName->getText().c_str(), 1));
            m_hOwn->NewMap_Validate();
        } else if (m_hOwn->NewMap_data != 0 && actionEvent.getSource() == m_hOwn->NewMap_data->tfAuthor) {
            m_hOwn->NewMap_data->tfAuthor->setMarkedInvalid(
                    strlen(m_hOwn->NewMap_data->tfAuthor->getText().c_str()) == 0);
            m_hOwn->NewMap_Validate();
        } else if (actionEvent.getSource() == m_hOwn->butpmSave) {
            m_hOwn->SavePlaneProperties();
        } else if (actionEvent.getSource() == m_hOwn->butpmDelete) {
            m_hOwn->DeletePlaneProperties();
        } else if (actionEvent.getSource() == m_hOwn->butpmResUL) {
            m_hOwn->SetAnchorPlaneProperties(1);
        } else if (actionEvent.getSource() == m_hOwn->butpmResU) {
            m_hOwn->SetAnchorPlaneProperties(2);
        } else if (actionEvent.getSource() == m_hOwn->butpmResUR) {
            m_hOwn->SetAnchorPlaneProperties(3);
        } else if (actionEvent.getSource() == m_hOwn->butpmResL) {
            m_hOwn->SetAnchorPlaneProperties(4);
        } else if (actionEvent.getSource() == m_hOwn->butpmResC) {
            m_hOwn->SetAnchorPlaneProperties(5);
        } else if (actionEvent.getSource() == m_hOwn->butpmResR) {
            m_hOwn->SetAnchorPlaneProperties(6);
        } else if (actionEvent.getSource() == m_hOwn->butpmResDL) {
            m_hOwn->SetAnchorPlaneProperties(7);
        } else if (actionEvent.getSource() == m_hOwn->butpmResD) {
            m_hOwn->SetAnchorPlaneProperties(8);
        } else if (actionEvent.getSource() == m_hOwn->butpmResDR) {
            m_hOwn->SetAnchorPlaneProperties(9);
        } else if (actionEvent.getSource() == m_hOwn->tfpmPlaneSizeX ||
                   actionEvent.getSource() == m_hOwn->tfpmPlaneSizeY) {
            m_hOwn->SetAnchorPlaneProperties(m_hOwn->ipmAnchor);
        } else if (m_hOwn->butCrashRetrieve != NULL && actionEvent.getSource() == m_hOwn->butCrashRetrieve) {
            m_hOwn->winWelcome->remove(m_hOwn->conCrashRetrieve);
            delete m_hOwn->conCrashRetrieve;
            m_hOwn->conCrashRetrieve = NULL;
            m_hOwn->winWelcome->add(m_hOwn->conRecentFiles, 0, m_hOwn->winWelcome->getHeight() + 50);
            m_hOwn->winWelcome->setY(
                    m_hOwn->vPort->GetY() + m_hOwn->vPort->GetHeight() / 2 - m_hOwn->winWelcome->getHeight() / 2);
            std::ifstream ifs("runtime.tmp");
            std::string temp;
            while (getline(ifs, temp))
                if (temp.length() > 0)
                    m_hOwn->vstrMapsToLoad.push_back(temp);
            remove("runtime.tmp");
        } else if (actionEvent.getSource() == m_hOwn->butmsSaveAs) {
            OPENFILENAME ofn;
            char szFileopen[512] = "\0";
            char *fname = SHR::GetFile(m_hOwn->tfmsSaveAs->getText().c_str()), *fnext = SHR::GetFileWithoutExt(fname);
            sprintf(szFileopen, "%s", fnext);
            delete[] fname;
            delete[] fnext;
            ZeroMemory((&ofn), sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hge->System_GetState(HGE_HWND);
            ofn.lpstrFilter = "Plik PNG (*.PNG)\0*.png\0Plik JPEG (*.JPG)\0*.jpg\0Plik GIF (*.GIF)\0*.gif\0Wszystkie pliki (*.*)\0*.*\0\0";
            ofn.lpstrFile = szFileopen;
            ofn.nMaxFile = sizeof(szFileopen);
            //((Menu*)Herculo.GameState)->ofn->Flags = OFN_FILEMUSTEXIST; // | OFN_NOCHANGEDIR
            ofn.lpstrDefExt = "png";
            if (strlen(m_hOwn->hParser->GetFilePath()) > 0)
                ofn.lpstrInitialDir = SHR::GetDir(m_hOwn->hParser->GetFilePath());
            else {
                char exe_name[MAX_PATH + 1];
                exe_name[MAX_PATH] = '\0';
                ::GetModuleFileName(NULL, exe_name, MAX_PATH);
                ofn.lpstrInitialDir = SHR::GetDir(exe_name);
            }
            if (GetSaveFileName(&ofn))
                m_hOwn->tfmsSaveAs->setText(szFileopen, 1);
        } else if (actionEvent.getSource() == m_hOwn->hmbTile->ddActivePlane && m_hOwn->hParser != NULL) {
            m_hOwn->SwitchPlane();
        } else if (actionEvent.getSource() == m_hOwn->buttpfcShow) {
            if (m_hOwn->wintpFillColor->getHeight() == 61) {
                m_hOwn->wintpFillColor->setWidth(202);
                m_hOwn->wintpFillColor->setHeight(291);
                m_hOwn->buttpfcShow->setPosition(10, 239);
                m_hOwn->wintpFillColor->setY(m_hOwn->wintpFillColor->getY() - 230);
                m_hOwn->vptpfcPalette->setVisible(1);
                m_hOwn->buttpfcShow->setCaption(GETL(Lang_HideFill));
            } else {
                m_hOwn->wintpFillColor->setWidth(194);
                m_hOwn->wintpFillColor->setHeight(61);
                m_hOwn->wintpFillColor->setY(m_hOwn->wintpFillColor->getY() + 230);
                m_hOwn->buttpfcShow->setPosition(5, 9);
                m_hOwn->vptpfcPalette->setVisible(0);
                m_hOwn->buttpfcShow->setCaption(GETL(Lang_SelectFill));
            }
        } else if (actionEvent.getSource() == m_hOwn->winDuplicate) {
            m_hOwn->iActiveTool = EWW_TOOL_NONE;
        } else if (actionEvent.getSource() == m_hOwn->winSpacing) {
            m_hOwn->iActiveTool = EWW_TOOL_NONE;
        }

        for (int i = 0; i < 10; i++)
            if (actionEvent.getSource() == m_hOwn->lnkLastOpened[i]) {
                m_hOwn->vstrMapsToLoad.push_back(m_hOwn->hMruList->GetRecentlyUsedFile(i));
            }

        if (m_hOwn->hParser != NULL) {
            /*for(int i=0;i<m_hOwn->vObjectWins.size();i++){
             if( actionEvent.getSource() == m_hOwn->vObjectWins[i]->GetWindow() ){
              delete m_hOwn->vObjectWins[i];
              m_hOwn->vObjectWins.erase(m_hOwn->vObjectWins.begin()+i);
              i--;
             }
            }*/
            if (actionEvent.getSource() == m_hOwn->dbAssetsImg || actionEvent.getSource() == m_hOwn->dbFramesImg) {
                m_hOwn->SyncDB_ImageSets();
            } else if (actionEvent.getSource() == m_hOwn->dbAssetsSnd) {
                if (m_hOwn->dbAssetsSnd->getSelected() == -1) {
                    m_hOwn->spdbSounds->SetAsset(NULL);
                    return;
                }
                m_hOwn->spdbSounds->SetAsset(m_hOwn->hSndBank->GetAssetByIterator(m_hOwn->dbAssetsSnd->getSelected()));
                m_hOwn->spdbSounds->Play();
            } else if (actionEvent.getSource() == m_hOwn->dbAssetsAni) {
                if (m_hOwn->dbAssetsAni->getSelected() != -1) {
                    m_hOwn->sadbFramesAni->setVisible(1);
                    ANI::Animation *ani = m_hOwn->hAniBank->GetAssetByIterator(
                            m_hOwn->dbAssetsAni->getSelected())->GetAni();
                    char imgset[256];
                    ZeroMemory(&imgset, 256);
                    if (ani->GetImageset() != NULL)
                        strcpy(imgset, ani->GetImageset());
                    if (strlen(imgset) == 0) {
                        const char *id = m_hOwn->hAniBank->GetAssetByIterator(
                                m_hOwn->dbAssetsAni->getSelected())->GetID();
                        char *idreplace =
                                m_hOwn->hParser->GetGame() == WWD::Game_Gruntz ? SHR::Replace(id, "IMAGEZ_", "")
                                                                               : SHR::Replace(id, "IMAGES_", "");
                        strcpy(imgset, idreplace);
                    } else {
                        char *idreplace =
                                m_hOwn->hParser->GetGame() == WWD::Game_Gruntz ? SHR::Replace(imgset, "IMAGEZ_", "")
                                                                               : SHR::Replace(imgset, "IMAGES_", "");
                        if (!strncmp(idreplace, "LEVEL", 5)) {
                            int levelid = 0;
                            sscanf(idreplace, "LEVEL%d%*s", &levelid);
                            char what[25];
                            sprintf(what, "LEVEL%d", levelid);
                            char *replace2 = SHR::Replace(idreplace, what, "LEVEL");
                            strcpy(imgset, replace2);
                            delete[] replace2;
                        } else
                            strcpy(imgset, idreplace);
                        delete[] idreplace;
                    }
                    if (m_hOwn->SprBank->GetAssetByID(imgset) == NULL) {
                        char *pch;
                        pch = strrchr(imgset, '_');
                        int lastpos = pch - imgset;
                        char *testcpy = new char[lastpos + 1];
                        testcpy[lastpos] = '\0';
                        strncpy(testcpy, imgset, lastpos);
                        if (m_hOwn->SprBank->GetAssetByID(testcpy) != NULL)
                            imgset[lastpos] = '\0';
                        delete[] testcpy;
                    }
                    if (m_hOwn->SprBank->GetAssetByID(imgset) == NULL &&
                        m_hOwn->hParser->GetGame() == WWD::Game_Gruntz) {
                        char *pch;
                        pch = strrchr(imgset, '_');
                        int lastpos = pch - imgset;
                        char *workingcopy = new char[strlen(imgset) + 7], *rest = new char[strlen(imgset) - lastpos +
                                                                                           1];
                        ZeroMemory(&workingcopy, strlen(imgset) + 7);
                        ZeroMemory(&rest, strlen(imgset) - lastpos + 1);
                        strncpy(workingcopy, imgset, lastpos);
                        strcpy(rest, (imgset + lastpos));
                        sprintf(imgset, "%s_SOUTH%s", workingcopy, rest);
                        delete[] workingcopy;
                        delete[] rest;
                        if (m_hOwn->SprBank->GetAssetByID(imgset) == NULL)
                            imgset[strlen(imgset) - 1] = '\0';
                    }
                    m_hOwn->tfdbAniImageSet->setText(imgset);
                }
            } else if (actionEvent.getSource() == m_hOwn->slimsScale) {
                char text[128];
                sprintf(text, "%s: %.1f%%", GETL(Lang_Scale), m_hOwn->slimsScale->getValue());
                m_hOwn->labmsScale->setCaption(text);
                m_hOwn->labmsScale->adjustSize();

                sprintf(text, GETL(Lang_MapShotDimensions),
                        int(m_hOwn->hParser->GetPlane(m_hOwn->ddmsPlane->getSelected())->GetPlaneWidthPx() *
                            (m_hOwn->slimsScale->getValue() / 100)),
                        int(m_hOwn->hParser->GetPlane(m_hOwn->ddmsPlane->getSelected())->GetPlaneHeightPx() *
                            (m_hOwn->slimsScale->getValue() / 100)));
                m_hOwn->labmsDimensions->setCaption(text);
                m_hOwn->labmsDimensions->adjustSize();
            } else if (actionEvent.getSource() == m_hOwn->ddmsPlane) {
                m_hOwn->cbmsDrawObjects->setVisible(
                        m_hOwn->hParser->GetPlane(m_hOwn->ddmsPlane->getSelected())->GetObjectsCount() > 0);

                char text[128];
                sprintf(text, GETL(Lang_MapShotDimensions),
                        int(m_hOwn->hParser->GetPlane(m_hOwn->ddmsPlane->getSelected())->GetPlaneWidthPx() *
                            (m_hOwn->slimsScale->getValue() / 100)),
                        int(m_hOwn->hParser->GetPlane(m_hOwn->ddmsPlane->getSelected())->GetPlaneHeightPx() *
                            (m_hOwn->slimsScale->getValue() / 100)));
                m_hOwn->labmsDimensions->setCaption(text);
                m_hOwn->labmsDimensions->adjustSize();
            } else if (actionEvent.getSource() == m_hOwn->butmsSave) {
                int complevel;
                if (m_hOwn->slimsCompression->getScaleEnd() == 9) {
                    complevel = m_hOwn->slimsCompression->getValue();
                } else {
                    int val = m_hOwn->slimsCompression->getValue();
                    if (val == 0)
                        val = 50;
                    else if (val <= 7)
                        val = 0;

                    complevel = 100 - (val + 7);
                }
                bool dobjects = (m_hOwn->cbmsDrawObjects->isSelected() &&
                                 m_hOwn->hParser->GetPlane(m_hOwn->ddmsPlane->getSelected())->GetObjectsCount() != 0);
                GV->StateMgr->Push(new State::MapShot(m_hOwn, m_hOwn->ddmsPlane->getSelected(),
                                                      m_hOwn->slimsScale->getValue(),
                                                      m_hOwn->tfmsSaveAs->getText().c_str(),
                                                      complevel, m_hOwn->cpmsPicker->GetColor(),
                                                      dobjects));
            } else if (actionEvent.getSource() == m_hOwn->slimsCompression) {
                if (m_hOwn->slimsCompression->getScaleEnd() == 9) { //png
                    int val = m_hOwn->slimsCompression->getValue();
                    char tmp[64];
                    if (val == -1)
                        sprintf(tmp, "%s: %s", GETL2("Mapshot", Lang_Ms_CompressionLevel),
                                GETL2("Mapshot", Lang_Ms_CompressionDefault));
                    else if (val == 0)
                        sprintf(tmp, "%s: %s", GETL2("Mapshot", Lang_Ms_CompressionLevel),
                                GETL2("Mapshot", Lang_Ms_NoCompression));
                    else if (val > 0 && val <= 3)
                        sprintf(tmp, "%s: %s (%d)", GETL2("Mapshot", Lang_Ms_CompressionLevel),
                                GETL2("Mapshot", Lang_Ms_CompressionLow), val);
                    else if (val > 3 && val <= 6)
                        sprintf(tmp, "%s: %s (%d)", GETL2("Mapshot", Lang_Ms_CompressionLevel),
                                GETL2("Mapshot", Lang_Ms_CompressionMedium), val);
                    else if (val > 6 && val <= 9)
                        sprintf(tmp, "%s: %s (%d)", GETL2("Mapshot", Lang_Ms_CompressionLevel),
                                GETL2("Mapshot", Lang_Ms_CompressionHigh), val);
                    m_hOwn->labmsCompression->setCaption(tmp);
                    m_hOwn->labmsCompression->adjustSize();
                } else if (m_hOwn->slimsCompression->getScaleEnd() == 100) { //jpg
                    int val = m_hOwn->slimsCompression->getValue();
                    char tmp[64];
                    if (val == 0)
                        sprintf(tmp, "%s: %s", GETL2("Mapshot", Lang_Ms_CompressionLevel),
                                GETL2("Mapshot", Lang_Ms_CompressionDefault));
                    else if (val <= 7)
                        sprintf(tmp, "%s: %s", GETL2("Mapshot", Lang_Ms_CompressionLevel),
                                GETL2("Mapshot", Lang_Ms_NoCompression));
                    else if (val > 7 && val <= 38)
                        sprintf(tmp, "%s: %s (%d%%)", GETL2("Mapshot", Lang_Ms_CompressionLevel),
                                GETL2("Mapshot", Lang_Ms_CompressionLow), val);
                    else if (val > 38 && val <= 69)
                        sprintf(tmp, "%s: %s (%d%%)", GETL2("Mapshot", Lang_Ms_CompressionLevel),
                                GETL2("Mapshot", Lang_Ms_CompressionMedium), val);
                    else if (val > 69 && val <= 100)
                        sprintf(tmp, "%s: %s (%d%%)", GETL2("Mapshot", Lang_Ms_CompressionLevel),
                                GETL2("Mapshot", Lang_Ms_CompressionHigh), val);
                    m_hOwn->labmsCompression->setCaption(tmp);
                    m_hOwn->labmsCompression->adjustSize();
                } //else wtf
            } else if (actionEvent.getSource() == m_hOwn->tfmsSaveAs) {
                char *ext = SHR::GetExtension(m_hOwn->tfmsSaveAs->getText().c_str());
                if (ext == NULL) {
                    m_hOwn->labmsCompDisc->setVisible(0);
                    m_hOwn->labmsCompression->setVisible(0);
                    m_hOwn->slimsCompression->setVisible(0);
                    m_hOwn->butmsSave->setEnabled(0);
                    return;
                }
                char *extl = SHR::ToLower(ext);

                if (!strcmp(extl, "gif")) {
                    m_hOwn->labmsCompDisc->setVisible(0);
                    m_hOwn->labmsCompression->setVisible(0);
                    m_hOwn->slimsCompression->setVisible(0);
                    m_hOwn->butmsSave->setEnabled(0);
                } else if (!strcmp(extl, "jpg") || !strcmp(extl, "jpeg")) {
                    m_hOwn->labmsCompDisc->setCaption(GETL2("Mapshot", Lang_Ms_DisclaimerJPG));
                    m_hOwn->slimsCompression->setScale(0, 100);
                    m_hOwn->slimsCompression->clearKeyValues();
                    m_hOwn->slimsCompression->addKeyValue(0);
                    m_hOwn->slimsCompression->addKeyValue(7);
                    m_hOwn->slimsCompression->addKeyValue(38);
                    m_hOwn->slimsCompression->addKeyValue(69);
                    m_hOwn->slimsCompression->addKeyValue(100);
                    m_hOwn->slimsCompression->setForceKeyValue(0);
                } else if (!strcmp(extl, "png")) {
                    m_hOwn->labmsCompDisc->setCaption(GETL2("Mapshot", Lang_Ms_DisclaimerPNG));
                    m_hOwn->slimsCompression->setScale(-1, 9);
                    m_hOwn->slimsCompression->clearKeyValues();
                    m_hOwn->slimsCompression->addKeyValue(-1);
                    for (int i = 1; i < 10; i++)
                        m_hOwn->slimsCompression->addKeyValue(i);
                    m_hOwn->slimsCompression->setForceKeyValue(1);
                } else
                    return;

                if (strcmp(extl, "gif") != 0) {
                    m_hOwn->labmsCompDisc->setVisible(1);
                    m_hOwn->labmsCompression->setVisible(1);
                    m_hOwn->slimsCompression->setVisible(1);
                    m_hOwn->slimsCompression->setValue(-1);
                    char tmp[64];
                    sprintf(tmp, "%s: %s", GETL2("Mapshot", Lang_Ms_CompressionLevel),
                            GETL2("Mapshot", Lang_Ms_CompressionDefault));
                    m_hOwn->labmsCompression->setCaption(tmp);
                    m_hOwn->labmsCompression->adjustSize();
                }

                delete[] ext;
                delete[] extl;

                FILE *f = fopen(m_hOwn->tfmsSaveAs->getText().c_str(), "a");
                m_hOwn->butmsSave->setEnabled(f != NULL);
                if (f != NULL) {
                    fclose(f);
                    unlink(m_hOwn->tfmsSaveAs->getText().c_str());
                }
            } else if (actionEvent.getSource() == m_hOwn->butwpSave) {
                m_hOwn->SaveWorldOptions();
            } else if (actionEvent.getSource() == m_hOwn->butwpCancel) {
                m_hOwn->winWorld->setVisible(false);
                m_hOwn->SyncWorldOptionsWithParser();
            } else if (actionEvent.getSource() == m_hOwn->butdOK
                       || (actionEvent.getSource()->getActionEventId() == "ENTER"
                           && (actionEvent.getSource() == m_hOwn->tfdTimes
                               || actionEvent.getSource() == m_hOwn->tfdOffsetX
                               || actionEvent.getSource() == m_hOwn->tfdOffsetY))) {
                std::vector<WWD::Object *> duplicates;

                int count = atoi(m_hOwn->tfdTimes->getText().c_str());
                int offX = atoi(m_hOwn->tfdOffsetX->getText().c_str());
                int offY = atoi(m_hOwn->tfdOffsetY->getText().c_str());
                int offZ = atoi(m_hOwn->tfdChangeZ->getText().c_str());
                if (count < 0) count = 0;
                for (int i = 0; i < count; i++) {
                    for (auto &picked : m_hOwn->vObjectsPicked) {
                        auto *obj = new WWD::Object(picked);
                        obj->SetUserData(new cObjUserData(obj));
                        GetUserDataFromObj(obj)->SetPos(obj->GetParam(WWD::Param_LocationX) + offX * (i + 1),
                                                        obj->GetParam(WWD::Param_LocationY) + offY * (i + 1));
                        obj->SetParam(WWD::Param_LocationZ, obj->GetParam(WWD::Param_LocationZ) + offZ * (i + 1));
                        m_hOwn->GetActivePlane()->AddObjectAndCalcID(obj);
                        duplicates.push_back(obj);
                        //m_hOwn->hPlaneData[m_hOwn->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(obj);
                    }
                }
                if (count != 0) {
                    m_hOwn->UpdateMovedObjectWithRects(duplicates);

                    m_hOwn->vPort->MarkToRedraw(true);
                    m_hOwn->MarkUnsaved();
                }

                m_hOwn->SetTool(EWW_TOOL_NONE);
            } else if (actionEvent.getSource() == m_hOwn->butcamSetToSpawn) {
                m_hOwn->fCamX = m_hOwn->hParser->GetStartX() - (m_hOwn->vPort->GetWidth() / 2 / m_hOwn->fZoom);
                m_hOwn->fCamY = m_hOwn->hParser->GetStartY() - (m_hOwn->vPort->GetHeight() / 2 / m_hOwn->fZoom);
            } else if (actionEvent.getSource() == m_hOwn->butcamSetTo) {
                m_hOwn->fCamX =
                        atoi(m_hOwn->tfcamSetToX->getText().c_str()) - (m_hOwn->vPort->GetWidth() / 2 / m_hOwn->fZoom);
                m_hOwn->fCamY =
                        atoi(m_hOwn->tfcamSetToY->getText().c_str()) - (m_hOwn->vPort->GetHeight() / 2 / m_hOwn->fZoom);
            } else if (actionEvent.getSource() == m_hOwn->tilContext) {
                if (m_hOwn->tilContext->GetSelectedID() == TILMENU_COPY ||
                        m_hOwn->tilContext->GetSelectedID() == TILMENU_CUT) {
                    if (m_hOwn->MDI->GetActiveDoc()->hTileClipboard != NULL) {
                        delete[] m_hOwn->MDI->GetActiveDoc()->hTileClipboard;
                        delete[] m_hOwn->MDI->GetActiveDoc()->hTileClipboardImageSet;
                    }

                    m_hOwn->MDI->GetActiveDoc()->hTileClipboardImageSet = new char[
                    strlen(m_hOwn->GetActivePlane()->GetImageSet(0)) + 1];
                    strcpy(m_hOwn->MDI->GetActiveDoc()->hTileClipboardImageSet,
                           m_hOwn->GetActivePlane()->GetImageSet(0));

                    if (m_hOwn->iTileSelectX2 >= m_hOwn->GetActivePlane()->GetPlaneWidth()) {
                        m_hOwn->iTileSelectX2 = m_hOwn->GetActivePlane()->GetPlaneWidth() - 1;
                    }
                    if (m_hOwn->iTileSelectY2 >= m_hOwn->GetActivePlane()->GetPlaneHeight()) {
                        m_hOwn->iTileSelectY2 = m_hOwn->GetActivePlane()->GetPlaneHeight() - 1;
                    }

                    m_hOwn->MDI->GetActiveDoc()->iTileCBw = m_hOwn->iTileSelectX2 - m_hOwn->iTileSelectX1 + 1;
                    m_hOwn->MDI->GetActiveDoc()->iTileCBh = m_hOwn->iTileSelectY2 - m_hOwn->iTileSelectY1 + 1;

                    m_hOwn->MDI->GetActiveDoc()->hTileClipboard = new WWD::Tile[m_hOwn->MDI->GetActiveDoc()->iTileCBw *
                                                                                m_hOwn->MDI->GetActiveDoc()->iTileCBh];

                    for (int i = 0, y = m_hOwn->iTileSelectY1; y <= m_hOwn->iTileSelectY2; y++)
                        for (int x = m_hOwn->iTileSelectX1; x <= m_hOwn->iTileSelectX2; x++, i++)
                            m_hOwn->MDI->GetActiveDoc()->hTileClipboard[i] = *m_hOwn->GetActivePlane()->GetTile(x, y);

                    if (m_hOwn->tilContext->GetSelectedID() == TILMENU_CUT) {
                        bool bChanges = false;
                        for (int x = m_hOwn->iTileSelectX1; x <= m_hOwn->iTileSelectX2; x++)
                            for (int y = m_hOwn->iTileSelectY1; y <= m_hOwn->iTileSelectY2; y++) {
                                WWD::Tile *tile = m_hOwn->GetActivePlane()->GetTile(x, y);
                                if (!tile->IsInvisible()) {
                                    bChanges = true;
                                    tile->SetInvisible(true);
                                }
                            }
                        if (bChanges) {
                            m_hOwn->vPort->MarkToRedraw(1);
                            m_hOwn->MarkUnsaved();
                        }
                    }
                } else if (m_hOwn->tilContext->GetSelectedID() == TILMENU_PASTE) {
                    bool bChanges = 0;
                    float mx, my;
                    hge->Input_GetMousePos(&mx, &my);
                    int tx = m_hOwn->Scr2WrdX(m_hOwn->GetActivePlane(), mx) / m_hOwn->GetActivePlane()->GetTileWidth(),
                        ty = m_hOwn->Scr2WrdY(m_hOwn->GetActivePlane(), my) / m_hOwn->GetActivePlane()->GetTileHeight();
                    for (int i = 0, y = ty; y < ty + m_hOwn->MDI->GetActiveDoc()->iTileCBh; ++y)
                        for (int x = tx; x < tx + m_hOwn->MDI->GetActiveDoc()->iTileCBw; ++x, ++i) {
                            WWD::Tile *tile = m_hOwn->GetActivePlane()->GetTile(x, y);
                            if (tile && *tile != m_hOwn->MDI->GetActiveDoc()->hTileClipboard[i]) {
                                bChanges = 1;
                                *tile = m_hOwn->MDI->GetActiveDoc()->hTileClipboard[i];
                            }
                        }
                    if (bChanges) {
                        m_hOwn->vPort->MarkToRedraw(1);
                        m_hOwn->MarkUnsaved();
                    }
                } else if (m_hOwn->tilContext->GetSelectedID() == TILMENU_DELETE) {
                    bool bChanges = false;
                    for (int x = m_hOwn->iTileSelectX1; x <= m_hOwn->iTileSelectX2; x++)
                        for (int y = m_hOwn->iTileSelectY1; y <= m_hOwn->iTileSelectY2; y++) {
                            WWD::Tile *tile = m_hOwn->GetActivePlane()->GetTile(x, y);
                            if (tile && !tile->IsInvisible()) {
                                bChanges = 1;
                                tile->SetInvisible(true);
                            }
                        }
                    if (bChanges) {
                        m_hOwn->vPort->MarkToRedraw(1);
                        m_hOwn->MarkUnsaved();
                    }
                }
                m_hOwn->tilContext->setVisible(0);
            } else if (actionEvent.getSource() == m_hOwn->objContext) {
                if (m_hOwn->objContext->GetSelectedID() == OBJMENU_PROPERTIES) {
                    m_hOwn->OpenObjectWindow(m_hOwn->vObjectsPicked[0]);
                } else if (m_hOwn->objContext->GetSelectedID() == OBJMENU_EDITLOGIC) {
                    GV->editState->hDataCtrl->OpenCodeEditor(m_hOwn->vObjectsPicked[0]->GetName());
                } else if (m_hOwn->objContext->GetSelectedID() == OBJMENU_SETSPAWNP) {
                    int orix, oriy;
                    m_hOwn->objContext->getAbsolutePosition(orix, oriy);
                    int x = m_hOwn->Scr2WrdX(m_hOwn->GetActivePlane(), orix),
                            y = m_hOwn->Scr2WrdY(m_hOwn->GetActivePlane(), oriy);
                    if (x != m_hOwn->hParser->GetStartX() || y != m_hOwn->hParser->GetStartY()) {
                        m_hOwn->hStartingPosObj->SetParam(WWD::Param_LocationX, x);
                        m_hOwn->hStartingPosObj->SetParam(WWD::Param_LocationY, y);
                        GetUserDataFromObj(m_hOwn->hStartingPosObj)->SyncToObj();
                        m_hOwn->hParser->SetStartX(x);
                        m_hOwn->hParser->SetStartY(y);
                        m_hOwn->vPort->MarkToRedraw(1);
                        m_hOwn->MarkUnsaved();
                    }
                } else if (m_hOwn->objContext->GetSelectedID() == OBJMENU_TESTFROMHERE) {
                    if (strlen(m_hOwn->MDI->GetActiveDoc()->hParser->GetFilePath()) > 0) {
                        int contextX, contextY;
                        m_hOwn->objContext->getAbsolutePosition(contextX, contextY);
                        int worldX = m_hOwn->Scr2WrdX(m_hOwn->GetActivePlane(), contextX),
                                worldY = m_hOwn->Scr2WrdY(m_hOwn->GetActivePlane(), contextY);
                        m_hOwn->hNativeController->RunGame(m_hOwn->MDI->GetActiveDoc()->hParser->GetFilePath(), worldX,
                                                           worldY);
                    }
                } else if (m_hOwn->objContext->GetSelectedID() == OBJMENU_DUPLICATE) {
                    for (int i = 0; i < m_hOwn->vObjectsPicked.size(); i++)
                        if (m_hOwn->vObjectsPicked[i] == m_hOwn->hStartingPosObj)
                            m_hOwn->vObjectsPicked.erase(m_hOwn->vObjectsPicked.begin() + i);
                    m_hOwn->SetTool(EWW_TOOL_DUPLICATE);
                } else if (m_hOwn->objContext->GetSelectedID() == OBJMENU_MOVE) {
                    m_hOwn->SetTool(EWW_TOOL_MOVEOBJECT);
                    float mx, my;
                    hge->Input_GetMousePos(&mx, &my);
                    if (m_hOwn->vObjectsPicked.size() == 1) {
                        hge->Input_SetMousePos(m_hOwn->Wrd2ScrX(m_hOwn->GetActivePlane(),
                                                                m_hOwn->vObjectsPicked[0]->GetParam(
                                                                        WWD::Param_LocationX)),
                                               m_hOwn->Wrd2ScrY(m_hOwn->GetActivePlane(),
                                                                m_hOwn->vObjectsPicked[0]->GetParam(
                                                                        WWD::Param_LocationY)));
                        m_hOwn->iMoveRelX = m_hOwn->vObjectsPicked[0]->GetParam(WWD::Param_LocationX);
                        m_hOwn->iMoveRelY = m_hOwn->vObjectsPicked[0]->GetParam(WWD::Param_LocationY);
                        m_hOwn->fObjPickLastMx = m_hOwn->Scr2WrdX(m_hOwn->GetActivePlane(), mx);
                        m_hOwn->fObjPickLastMy = m_hOwn->Scr2WrdY(m_hOwn->GetActivePlane(), my);
                    } else {
                        for (int i = 0; i < m_hOwn->vObjectsPicked.size(); i++)
                            //if( m_hOwn->vObjectsPicked[i] == m_hOwn->hStartingPosObj )
                            // m_hOwn->vObjectsPicked.erase(m_hOwn->vObjectsPicked.begin()+i);
                            m_hOwn->iMoveRelX = m_hOwn->Scr2WrdX(m_hOwn->GetActivePlane(), mx);
                        m_hOwn->iMoveRelY = m_hOwn->Scr2WrdY(m_hOwn->GetActivePlane(), my);
                    }
                } else if (m_hOwn->objContext->GetSelectedID() == OBJMENU_DELETE) {
                    for (int i = 0; i < m_hOwn->vObjectsPicked.size(); i++)
                        if (m_hOwn->vObjectsPicked[i] == m_hOwn->hStartingPosObj)
                            m_hOwn->vObjectsPicked.erase(m_hOwn->vObjectsPicked.begin() + i);
                    std::vector<WWD::Object *> tmp = m_hOwn->vObjectsPicked;
                    for (auto &i : tmp) {
                        m_hOwn->GetActivePlane()->DeleteObject(i);
                    }
                    m_hOwn->vPort->MarkToRedraw(true);
                    m_hOwn->MarkUnsaved();
                } else if (m_hOwn->objContext->GetSelectedID() == OBJMENU_COPY
                           || m_hOwn->objContext->GetSelectedID() == OBJMENU_CUT) {
                    bool deleting = m_hOwn->objContext->GetSelectedID() == OBJMENU_CUT;
                    for (auto object : m_hOwn->vObjectClipboard) {
                        delete object;
                    }
                    m_hOwn->vObjectClipboard.clear();
                    auto selectedObjects = m_hOwn->vObjectsPicked;
                    for (auto object : selectedObjects) {
                        if (object != m_hOwn->hStartingPosObj) {
                            auto nObject = new WWD::Object(object);
                            if (deleting) {
                                m_hOwn->GetActivePlane()->DeleteObject(object);
                            }
                            m_hOwn->vObjectClipboard.push_back(nObject);
                        }
                    }
                    if (deleting) {
                        m_hOwn->vObjectsPicked.clear();
                        m_hOwn->vPort->MarkToRedraw(true);
                        m_hOwn->MarkUnsaved();
                    }
                } else if (m_hOwn->objContext->GetSelectedID() == OBJMENU_PASTE) {
                    if (m_hOwn->vObjectClipboard.empty()) return;
                    m_hOwn->vObjectsPicked.clear();
                    float x = int(m_hOwn->fCamX * (m_hOwn->GetActivePlane()->GetMoveModX() / 100.0f) * m_hOwn->fZoom) +
                              m_hOwn->objContext->getX();
                    float y = int(m_hOwn->fCamY * (m_hOwn->GetActivePlane()->GetMoveModY() / 100.0f) * m_hOwn->fZoom) +
                              m_hOwn->objContext->getY() - m_hOwn->vPort->GetY();

                    x = x / m_hOwn->fZoom;
                    y = y / m_hOwn->fZoom;

                    for (auto &clipboardObject : m_hOwn->vObjectClipboard) {
                        float diffX = clipboardObject->GetX() - m_hOwn->vObjectClipboard[0]->GetX(),
                              diffY = clipboardObject->GetY() - m_hOwn->vObjectClipboard[0]->GetY();

                        auto *object = new WWD::Object(clipboardObject);
                        m_hOwn->GetActivePlane()->AddObjectAndCalcID(object);
                        object->SetUserData(new cObjUserData(object));
                        //m_hOwn->hPlaneData[m_hOwn->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(object);
                        m_hOwn->vObjectsPicked.push_back(object);
                        GetUserDataFromObj(object)->SetPos(x + diffX, y + diffY);
                    }

                    m_hOwn->UpdateMovedObjectWithRects(m_hOwn->vObjectsPicked);

                    m_hOwn->MarkUnsaved();
                    m_hOwn->vPort->MarkToRedraw(true);
                } else if (m_hOwn->objContext->GetSelectedID() == OBJMENU_USEASBRUSH) {
                    for (int i = 0; i < m_hOwn->vObjectsPicked.size(); i++)
                        if (m_hOwn->vObjectsPicked[i] == m_hOwn->hStartingPosObj)
                            m_hOwn->vObjectsPicked.erase(m_hOwn->vObjectsPicked.begin() + i);
                    m_hOwn->vObjectsBrushCB = m_hOwn->vObjectsPicked;
                    m_hOwn->SetTool(EWW_TOOL_BRUSHOBJECT);
                } else if (m_hOwn->objContext->GetSelectedID() == OBJMENU_NEWOBJ) {
                    if (m_hOwn->objContext->GetElementByID(OBJMENU_NEWOBJ)->GetCascade()) return;
                    m_hOwn->hmbObject->butIconNewObjEmpty->simulatePress();
                } else if (m_hOwn->objContext->GetSelectedID() == OBJMENU_EDIT) {
                    m_hOwn->OpenObjectEdit(m_hOwn->vObjectsPicked[0]);
                } else if (m_hOwn->objContext->GetSelectedID() == OBJMENU_FLAGS ||
                        m_hOwn->objContext->GetSelectedID() == OBJMENU_ALIGN ||
                        m_hOwn->objContext->GetSelectedID() == OBJMENU_SPECIFICPROP ||
                        m_hOwn->objContext->GetSelectedID() == OBJMENU_ZCOORD ||
                        m_hOwn->objContext->GetSelectedID() == OBJMENU_SPACE) {
                    return;
                }
                m_hOwn->objContext->setVisible(0);
            } else if (actionEvent.getSource() == m_hOwn->objFlagDrawContext ||
                       actionEvent.getSource() == m_hOwn->objFlagDynamicContext ||
                       actionEvent.getSource() == m_hOwn->objFlagAddContext) {
                SHR::Context *hCallingContext = (SHR::Context *) actionEvent.getSource();
                int menupos = hCallingContext->GetSelectedID();
                int flagpos = 0;
                bool valuetoset = 1;
                if (hCallingContext->GetElementByID(menupos)->GetIcon(0) == GV->sprIcons16[Icon16_Applied])
                    valuetoset = 0;
                if (hCallingContext == m_hOwn->objFlagDrawContext) {
                    flagpos = menupos - OBJMENU_FLAGS_DRAW - 1;
                } else if (hCallingContext == m_hOwn->objFlagDynamicContext) {
                    flagpos = menupos - OBJMENU_FLAGS_DYNAMIC - 1;
                } /*else if (hCallingContext == m_hOwn->objFlagAddContext) {
					flagpos = menupos - OBJMENU_FLAGS_ADDITIONAL - 1;
				}*/
                int flagbinaryval = pow(2, flagpos);

                for (size_t obji = 0; obji < m_hOwn->vObjectsPicked.size(); obji++) {
                    if (m_hOwn->vObjectsPicked[obji] == m_hOwn->hStartingPosObj)
                        continue;
                    int aqflag = 0;
                    if (hCallingContext == m_hOwn->objFlagDrawContext) {
                        aqflag = int(m_hOwn->vObjectsPicked[obji]->GetDrawFlags());
                    } else if (hCallingContext == m_hOwn->objFlagDynamicContext) {
                        aqflag = int(m_hOwn->vObjectsPicked[obji]->GetDynamicFlags());
                    } else if (hCallingContext == m_hOwn->objFlagAddContext) {
                        aqflag = int(m_hOwn->vObjectsPicked[obji]->GetAddFlags());
                    }
                    if (valuetoset && (aqflag & flagbinaryval) > 0)
                        continue;
                    if (valuetoset)
                        aqflag += flagbinaryval;
                    else
                        aqflag -= flagbinaryval;
                    if (hCallingContext == m_hOwn->objFlagDrawContext) {
                        m_hOwn->vObjectsPicked[obji]->SetDrawFlags((WWD::OBJ_DRAW_FLAGS) aqflag);
                    } else if (hCallingContext == m_hOwn->objFlagDynamicContext) {
                        m_hOwn->vObjectsPicked[obji]->SetDynamicFlags((WWD::OBJ_DYNAMIC_FLAGS) aqflag);
                    } else if (hCallingContext == m_hOwn->objFlagAddContext) {
                        m_hOwn->vObjectsPicked[obji]->SetAddFlags((WWD::OBJ_ADD_FLAGS) aqflag);
                    }
                    if (hCallingContext == m_hOwn->objFlagDrawContext)
                        GetUserDataFromObj(m_hOwn->vObjectsPicked[obji])->SyncToObj();
                }

                hCallingContext->GetElementByID(menupos)->SetIcon(valuetoset ? GV->sprIcons16[Icon16_Applied] : 0);
                m_hOwn->MarkUnsaved();
                if (hCallingContext == m_hOwn->objFlagDrawContext) {
                    m_hOwn->vPort->MarkToRedraw(1);
                }
            } else if (actionEvent.getSource() == m_hOwn->objmAlignContext) {
                m_hOwn->bObjectAlignAxis = m_hOwn->objmAlignContext->GetSelectedID() == OBJMENU_ALIGN_VERT;
                m_hOwn->SetTool(EWW_TOOL_ALIGNOBJ);
                m_hOwn->objContext->setVisible(0);
            } else if (actionEvent.getSource() == m_hOwn->objmSpaceContext) {
                m_hOwn->bObjectAlignAxis = m_hOwn->objmSpaceContext->GetSelectedID() == OBJMENU_SPACE_VERT;
                m_hOwn->SetTool(EWW_TOOL_SPACEOBJ);
                m_hOwn->objContext->setVisible(0);
            } else if (actionEvent.getSource() == m_hOwn->advcon_Warp) {
                if (m_hOwn->advcon_Warp->GetSelectedID() == OBJMENU_ADV_WARP_GOTO) {
                    int destx = m_hOwn->vObjectsPicked[0]->GetParam(WWD::Param_SpeedX),
                            desty = m_hOwn->vObjectsPicked[0]->GetParam(WWD::Param_SpeedY);
                    m_hOwn->fCamX = destx - m_hOwn->vPort->GetWidth() / 2 / m_hOwn->fZoom;
                    m_hOwn->fCamY = desty - m_hOwn->vPort->GetHeight() / 2 / m_hOwn->fZoom;
                    m_hOwn->objContext->setVisible(0);
                }
            } else if (actionEvent.getSource() == m_hOwn->advcon_Container) {
                if (m_hOwn->advcon_Container->GetSelectedID() == OBJMENU_ADV_CONTAINER_RAND) {
                    bool bstacked = (strstr(m_hOwn->vObjectsPicked[0]->GetLogic(), "Stacked") != 0);
                    int randc = 1;
                    if (bstacked) {
                        bool typicalcnt = (hge->Random_Int(0, 10) < 8);
                        if (typicalcnt)
                            randc = hge->Random_Int(2, 4);
                        else
                            randc = hge->Random_Int(4, 9);
                    }

                    int treasureid = hge->Random_Int(0, 31);
                    if (treasureid == 3) treasureid++;
                    m_hOwn->vObjectsPicked[0]->SetParam(WWD::Param_Powerup,
                                                        m_hOwn->hInvCtrl->GetItemByIt(treasureid).second);
                    for (int i = 0; i < 2; i++) {
                        WWD::Rect r;
                        int vals[4];
                        for (int y = 0; y < 4; y++) {
                            if (i * 4 + y + 1 < randc) {
                                treasureid = hge->Random_Int(0, 31);
                                if (treasureid == 3) treasureid++;
                                vals[y] = m_hOwn->hInvCtrl->GetItemByIt(treasureid).second;
                            } else
                                vals[y] = 0;
                        }
                        r.x1 = vals[0];
                        r.y1 = vals[1];
                        r.x2 = vals[2];
                        r.y2 = vals[3];
                        m_hOwn->vObjectsPicked[0]->SetUserRect(i, r);
                    }
                    m_hOwn->objContext->setVisible(0);
                    m_hOwn->vPort->MarkToRedraw(1);
                }
            } else if (actionEvent.getSource() == m_hOwn->objZCoordContext) {
                bool change = false, close = false;
                int selectedId = m_hOwn->objZCoordContext->GetSelectedID();
                for (auto &pickedObject : m_hOwn->vObjectsPicked) {
                    if (pickedObject == m_hOwn->hStartingPosObj)
                        continue;
                    int z = pickedObject->GetParam(WWD::Param_LocationZ), newZ;
                    if (selectedId == OBJMENU_ZC_INC) newZ = z + 10;
                    else if (selectedId == OBJMENU_ZC_INC2) newZ = z + 100;
                    else if (selectedId == OBJMENU_ZC_INC3) newZ = z + 1000;
                    else if (selectedId == OBJMENU_ZC_DEC) newZ = z - 10;
                    else if (selectedId == OBJMENU_ZC_DEC2) newZ = z - 100;
                    else if (selectedId == OBJMENU_ZC_DEC3) newZ = z - 1000;
                    else if (selectedId == OBJMENU_ZC_BACK) {
                        newZ = 1000;
                        close = true;
                    } else if (selectedId == OBJMENU_ZC_ACTION) {
                        newZ = 4000;
                        close = true;
                    } else if (selectedId == OBJMENU_ZC_FRONT) {
                        newZ = 5000;
                        close = true;
                    }
                    if (newZ != z) change = true;
                    pickedObject->SetParam(WWD::Param_LocationZ, newZ);
                    GetUserDataFromObj(pickedObject)->SetZ(newZ);
                }
                if (change) {
                    m_hOwn->vPort->MarkToRedraw(true);
                    m_hOwn->MarkUnsaved();
                }
                if (close)
                    m_hOwn->objContext->setVisible(false);
            } else if (actionEvent.getSource() == m_hOwn->butspacingOK) {
                int diff = atoi(m_hOwn->tfspacingDistance->getText().c_str());
                std::vector<WWD::Object *> objs = m_hOwn->vObjectsPicked;
                if (m_hOwn->bObjectAlignAxis)
                    sort(objs.begin(), objs.end(), State::ObjSortCoordY);
                else
                    sort(objs.begin(), objs.end(), State::ObjSortCoordX);
                bool bchange = 0;
                for (int i = 1; i < objs.size(); i++) {
                    if (objs[i - 1]->GetParam(WWD::Param_LocationX) + diff != objs[i]->GetParam(WWD::Param_LocationX) ||
                        objs[i - 1]->GetParam(WWD::Param_LocationY) + diff != objs[i]->GetParam(WWD::Param_LocationY))
                        bchange = 1;
                    if (m_hOwn->bObjectAlignAxis)
                        objs[i]->SetParam(WWD::Param_LocationY, objs[i - 1]->GetParam(WWD::Param_LocationY) + diff);
                    else objs[i]->SetParam(WWD::Param_LocationX, objs[i - 1]->GetParam(WWD::Param_LocationX) + diff);
                    if (bchange)
                        GetUserDataFromObj(objs[i])->SyncToObj();
                }
                if (bchange) {
                    m_hOwn->vPort->MarkToRedraw(1);
                    m_hOwn->MarkUnsaved();
                }
                m_hOwn->SetTool(EWW_TOOL_NONE);
            } else if (actionEvent.getSource() == m_hOwn->winSpacing) {
                m_hOwn->SetTool(EWW_TOOL_NONE);
            } else if (actionEvent.getSource() == m_hOwn->sliHor) {
                m_hOwn->fCamX = m_hOwn->sliHor->getValue();
            } else if (actionEvent.getSource() == m_hOwn->sliVer) {
                m_hOwn->fCamY = m_hOwn->sliVer->getScaleEnd() - m_hOwn->sliVer->getValue();
            } else if (actionEvent.getSource() == m_hOwn->sliobrDistance) {
                char tmp[128];
                sprintf(tmp, "%s: %.0fpx", GETL(Lang_BrushInterval), m_hOwn->sliobrDistance->getValue());
                m_hOwn->labobrDistance->setCaption(tmp);
                m_hOwn->labobrDistance->adjustSize();
            } else if (actionEvent.getSource() == m_hOwn->buttpiReloadBrush) {
                m_hOwn->HandleBrushSwitch(m_hOwn->iTilePicked, EWW_TILE_NONE);
                m_hOwn->iTilePicked = EWW_TILE_NONE;
                m_hOwn->hTileset->ReloadBrushes();
                m_hOwn->RebuildTilePicker();
                m_hOwn->gui->focusNone();
            } else if (actionEvent.getSource() == m_hOwn->butmeasClear) {
                m_hOwn->m_vMeasurePoints.clear();
            } else if (actionEvent.getSource() == m_hOwn->tftpTileID) {
                m_hOwn->itpSelectedTile = atoi(m_hOwn->tftpTileID->getText().c_str());
                m_hOwn->SyncAttribMenuWithTile();
            } else if (actionEvent.getSource() == m_hOwn->buttpNext) {
                cTileImageSet *is = m_hOwn->hTileset->GetSet("ACTION");
                int actualiterator = 0;
                for (int i = 0; i < is->GetTilesCount(); i++) {
                    if (is->GetTileByIterator(i)->GetID() == m_hOwn->itpSelectedTile) {
                        actualiterator = i + 1;
                        if (actualiterator >= is->GetTilesCount())
                            actualiterator = 0;
                        break;
                    }
                }
                cTile *tile = is->GetTileByIterator(actualiterator);
                m_hOwn->itpSelectedTile = tile->GetID();
                char tmp[24];
                sprintf(tmp, "%d", tile->GetID());
                m_hOwn->tftpTileID->setText(tmp);
                m_hOwn->SyncAttribMenuWithTile();
            } else if (actionEvent.getSource() == m_hOwn->buttpPrev) {
                cTileImageSet *is = m_hOwn->hTileset->GetSet("ACTION");
                int actualiterator = 0;
                for (int i = 0; i < is->GetTilesCount(); i++) {
                    if (is->GetTileByIterator(i)->GetID() == m_hOwn->itpSelectedTile) {
                        actualiterator = i - 1;
                        if (actualiterator < 0)
                            actualiterator = is->GetTilesCount() - 1;
                        break;
                    }
                }
                cTile *tile = is->GetTileByIterator(actualiterator);
                m_hOwn->itpSelectedTile = tile->GetID();
                char tmp[24];
                sprintf(tmp, "%d", tile->GetID());
                m_hOwn->tftpTileID->setText(tmp);
                m_hOwn->SyncAttribMenuWithTile();
            } else if (actionEvent.getSource() == m_hOwn->buttpZoom) {
                m_hOwn->btpZoomTile = !m_hOwn->btpZoomTile;
            } else if (actionEvent.getSource() == m_hOwn->rbtpSingle) {
                int i = 0;
                for (; i < 5; ++i) {
                    if (m_hOwn->rbtpIn[i]->isSelected()) break;
                }
                delete m_hOwn->hTempAttrib;
                m_hOwn->hTempAttrib = new WWD::SingleTileAttrib(
                        atoi(m_hOwn->tftpW->getText().c_str()),
                        atoi(m_hOwn->tftpH->getText().c_str()),
                        (WWD::TILE_ATTRIB)i);
                m_hOwn->tftpX1->setVisible(0);
                m_hOwn->tftpY1->setVisible(0);
                m_hOwn->tftpX2->setVisible(0);
                m_hOwn->tftpY2->setVisible(0);
                for (int i = 0; i < 5; i++)
                    m_hOwn->rbtpOut[i]->setVisible(0);
                m_hOwn->winTileProp->setHeight(260);
            } else if (actionEvent.getSource() == m_hOwn->rbtpDouble) {
                int i = 0;
                for (; i < 5; ++i) {
                    if (m_hOwn->rbtpIn[i]->isSelected()) break;
                }
                delete m_hOwn->hTempAttrib;
                int w = atoi(m_hOwn->tftpW->getText().c_str()),
                    h = atoi(m_hOwn->tftpH->getText().c_str());
                m_hOwn->hTempAttrib = new WWD::DoubleTileAttrib(w, h, (WWD::TILE_ATTRIB)i,
                        WWD::Attrib_Clear, WWD::Rect(0, 0, w - 1, h - 1));
                m_hOwn->tftpX1->setVisible(true);
                m_hOwn->tftpX1->setText("0");
                m_hOwn->tftpY1->setVisible(true);
                m_hOwn->tftpY1->setText("0");
                m_hOwn->tftpX2->setVisible(true);
                m_hOwn->tftpX2->setText(std::to_string(w - 1));
                m_hOwn->tftpY2->setVisible(true);
                m_hOwn->tftpY2->setText(std::to_string(h - 1));
                for (auto & hRadBut : m_hOwn->rbtpOut)
                    hRadBut->setVisible(1);
                m_hOwn->rbtpOut[0]->setSelected(true);
                m_hOwn->winTileProp->setHeight(350);
            } else if (actionEvent.getSource() == m_hOwn->rbtpMask) {
                delete m_hOwn->hTempAttrib;
                m_hOwn->hTempAttrib = new WWD::MaskTileAttrib(
                        atoi(m_hOwn->tftpW->getText().c_str()),
                        atoi(m_hOwn->tftpH->getText().c_str()));
                m_hOwn->tftpX1->setVisible(false);
                m_hOwn->tftpY1->setVisible(false);
                m_hOwn->tftpX2->setVisible(false);
                m_hOwn->tftpY2->setVisible(false);
                for (auto & hRadBut : m_hOwn->rbtpOut)
                    hRadBut->setVisible(0);
                m_hOwn->winTileProp->setHeight(260);
            } else if (actionEvent.getSource() == m_hOwn->buttpApply) {
                m_hOwn->hParser->SetTileAttribs(m_hOwn->itpSelectedTile, m_hOwn->hTempAttrib);
                if (m_hOwn->bDrawTileProperties) {
                    m_hOwn->vPort->MarkToRedraw(true);
                }
            } else if (actionEvent.getSource() == m_hOwn->tftpW) {
                m_hOwn->hTempAttrib->setWidth(atoi(m_hOwn->tftpW->getText().c_str()));
            } else if (actionEvent.getSource() == m_hOwn->lbbrlLogicList) {
                if (!m_hOwn->bLogicBrowserExpanded) {
                    m_hOwn->ExpandLogicBrowser();
                }
                m_hOwn->SyncLogicBrowser();
            } else if (actionEvent.getSource() == m_hOwn->tftpH) {
                m_hOwn->hTempAttrib->setHeight(atoi(m_hOwn->tftpH->getText().c_str()));
            } else if (actionEvent.getSource() == m_hOwn->tftpX1 ||
                       actionEvent.getSource() == m_hOwn->tftpY1 ||
                       actionEvent.getSource() == m_hOwn->tftpX2 ||
                       actionEvent.getSource() == m_hOwn->tftpY2) {
                ((WWD::DoubleTileAttrib *) m_hOwn->hTempAttrib)->setMask(
                        atoi(m_hOwn->tftpX1->getText().c_str()),
                        atoi(m_hOwn->tftpY1->getText().c_str()),
                        atoi(m_hOwn->tftpX2->getText().c_str()),
                        atoi(m_hOwn->tftpY2->getText().c_str()));
            } else if (actionEvent.getSource() == m_hOwn->buttpShow) {
                m_hOwn->hAppMenu->GetContext(AppMenu_View)->EmulateClickID(APPMEN_VIEW_TILEPROP);
            } else if (actionEvent.getSource() == m_hOwn->hmbObject->butIconSearchObject) {
                m_hOwn->winSearchObj->setVisible(!m_hOwn->winSearchObj->isVisible());
            } else if (actionEvent.getSource() == m_hOwn->hmbObject->butIconNewObjEmpty) {
                WWD::Object *obj = new WWD::Object();
                bool bFromContext = false;
                if (GV->editState->hmbObject->GetContext()->isVisible() &&
                        GV->editState->hmbObject->GetContext()->GetSelectedID() != -1) {
                    obj->SetParam(WWD::Param_LocationX,
                                  m_hOwn->Scr2WrdX(m_hOwn->GetActivePlane(), m_hOwn->objContext->getX()));
                    obj->SetParam(WWD::Param_LocationY,
                                  m_hOwn->Scr2WrdY(m_hOwn->GetActivePlane(), m_hOwn->objContext->getY()));
                    bFromContext = true;
                } else {
                    obj->SetParam(WWD::Param_LocationX, m_hOwn->Scr2WrdX(m_hOwn->GetActivePlane(),
                                                                         m_hOwn->vPort->GetX() +
                                                                         m_hOwn->vPort->GetWidth() / 2));
                    obj->SetParam(WWD::Param_LocationY, m_hOwn->Scr2WrdY(m_hOwn->GetActivePlane(),
                                                                         m_hOwn->vPort->GetY() +
                                                                         m_hOwn->vPort->GetHeight() / 2));
                }
                m_hOwn->GetActivePlane()->AddObjectAndCalcID(obj);
                obj->SetUserData(new cObjUserData(obj));
                m_hOwn->hPlaneData[m_hOwn->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(obj);
                m_hOwn->OpenObjectWindow(obj, !bFromContext);
                m_hOwn->vObjectsPicked.clear();
                m_hOwn->vObjectsPicked.push_back(obj);
                m_hOwn->bEditObjDelete = 1;
            } else if (std::find(m_hOwn->hmbObject->vButtons.begin(), m_hOwn->hmbObject->vButtons.end(),
                                 actionEvent.getSource()) != m_hOwn->hmbObject->vButtons.end()) {
                m_hOwn->CreateObjectWithEasyEdit(actionEvent.getSource());
            } else if (actionEvent.getSource() == m_hOwn->butExtLayerUp) {
                m_hOwn->GetActivePlane()->ResizeAddTiles(0, -1);
            } else if (actionEvent.getSource() == m_hOwn->butExtLayerDown) {
                m_hOwn->GetActivePlane()->ResizeAddTiles(0, 1);
                m_hOwn->fCamY += m_hOwn->GetActivePlane()->GetTileHeight() / m_hOwn->fZoom;
            } else if (actionEvent.getSource() == m_hOwn->butExtLayerLeft) {
                m_hOwn->GetActivePlane()->ResizeAddTiles(-1, 0);
            } else if (actionEvent.getSource() == m_hOwn->butExtLayerRight) {
                m_hOwn->GetActivePlane()->ResizeAddTiles(1, 0);
                m_hOwn->fCamX += m_hOwn->GetActivePlane()->GetTileWidth() / m_hOwn->fZoom;
            } else if (actionEvent.getSource() == m_hOwn->butExtLayerUL) {
                m_hOwn->GetActivePlane()->ResizeAddTiles(-1, -1);
            } else if (actionEvent.getSource() == m_hOwn->butExtLayerUR) {
                m_hOwn->GetActivePlane()->ResizeAddTiles(1, -1);
                m_hOwn->fCamX += m_hOwn->GetActivePlane()->GetTileWidth() / m_hOwn->fZoom;
            } else if (actionEvent.getSource() == m_hOwn->butExtLayerDL) {
                m_hOwn->GetActivePlane()->ResizeAddTiles(-1, 1);
                m_hOwn->fCamY += m_hOwn->GetActivePlane()->GetTileHeight() / m_hOwn->fZoom;
            } else if (actionEvent.getSource() == m_hOwn->butExtLayerDR) {
                m_hOwn->GetActivePlane()->ResizeAddTiles(1, 1);
                m_hOwn->fCamX += m_hOwn->GetActivePlane()->GetTileWidth() / m_hOwn->fZoom;
                m_hOwn->fCamY += m_hOwn->GetActivePlane()->GetTileHeight() / m_hOwn->fZoom;
            } else if (actionEvent.getSource() == m_hOwn->butMicroTileCB) {
                m_hOwn->bForceObjectClipbPreview = false;
                m_hOwn->bForceTileClipbPreview = !m_hOwn->bForceTileClipbPreview;
                m_hOwn->butMicroTileCB->setHighlight(m_hOwn->bForceTileClipbPreview);
                m_hOwn->butMicroObjectCB->setHighlight(false);
            } else if (actionEvent.getSource() == m_hOwn->butMicroObjectCB) {
                m_hOwn->bForceTileClipbPreview = false;
                m_hOwn->bForceObjectClipbPreview = !m_hOwn->bForceObjectClipbPreview;
                m_hOwn->butMicroObjectCB->setHighlight(m_hOwn->bForceObjectClipbPreview);
                m_hOwn->butMicroTileCB->setHighlight(false);
            } else if (actionEvent.getSource() == m_hOwn->cbObjSearchCaseSensitive) {
                m_hOwn->UpdateSearchResults();
            } else if (actionEvent.getSource() == m_hOwn->ddObjSearchTerm) {
                m_hOwn->UpdateSearchResults();
            } else if (actionEvent.getSource() == m_hOwn->butObjSearchSelectAll) {
                m_hOwn->vObjectsPicked.clear();
                for (int i = 0; i < m_hOwn->vObjSearchResults.size(); i++) {
                    WWD::Object *obj = m_hOwn->GetActivePlane()->GetObjectByObjectID(
                            m_hOwn->vObjSearchResults[i].second);
                    if (obj != NULL)
                        m_hOwn->vObjectsPicked.push_back(obj);
                }
            } else if (actionEvent.getSource() == m_hOwn->buttoolSelAreaAll) {
                m_hOwn->toolsaAction = TOOL_OBJSA_PICKALL;
            } else if (actionEvent.getSource() == m_hOwn->buttoolSelAreaPickMinX) {
                m_hOwn->toolsaAction = TOOL_OBJSA_PICKMINX;
            } else if (actionEvent.getSource() == m_hOwn->buttoolSelAreaPickMinY) {
                m_hOwn->toolsaAction = TOOL_OBJSA_PICKMINY;
            } else if (actionEvent.getSource() == m_hOwn->buttoolSelAreaPickMaxX) {
                m_hOwn->toolsaAction = TOOL_OBJSA_PICKMAXX;
            } else if (actionEvent.getSource() == m_hOwn->buttoolSelAreaPickMaxY) {
                m_hOwn->toolsaAction = TOOL_OBJSA_PICKMAXY;
            } else if (actionEvent.getSource() == m_hOwn->buttoolSelAreaOK) {
                int *ret = new int[4];
                ret[0] = m_hOwn->toolsaMinX;
                ret[1] = m_hOwn->toolsaMinY;
                ret[2] = m_hOwn->toolsaMaxX;
                ret[3] = m_hOwn->toolsaMaxY;

                returnCode *rc = new returnCode;
                rc->Type = RC_ObjPropSelectedValues;
                rc->Ptr = (int) (void *) ret;
                m_hOwn->_flipMe((int) rc);
                m_hOwn->SetTool(EWW_TOOL_NONE);
            } else if (actionEvent.getSource() == m_hOwn->wintoolSelArea) {
                returnCode *rc = new returnCode;
                rc->Type = RC_ObjPropSelectedValues;
                rc->Ptr = 0;
                m_hOwn->_flipMe((int) rc);
                m_hOwn->SetTool(EWW_TOOL_NONE);
            }

            for (int i = 0; i < 5; i++) {
                if (actionEvent.getSource() == m_hOwn->rbtpIn[i]) {
                    switch (m_hOwn->hTempAttrib->getType()) {
                        case WWD::AttribType_Single:
                            ((WWD::SingleTileAttrib *) m_hOwn->hTempAttrib)->setAttrib((WWD::TILE_ATTRIB) i);
                            break;
                        case WWD::AttribType_Double:
                            ((WWD::DoubleTileAttrib *) m_hOwn->hTempAttrib)->setInsideAttrib((WWD::TILE_ATTRIB) i);
                            break;
                    }
                } else if (actionEvent.getSource() == m_hOwn->rbtpOut[i]) {
                    ((WWD::DoubleTileAttrib *) m_hOwn->hTempAttrib)->setOutsideAttrib((WWD::TILE_ATTRIB) i);
                }
            }
        }
    }

    EditingWWActionListener::EditingWWActionListener(EditingWW *owner) {
        m_hOwn = owner;
    }

    EditingWWFocusListener::EditingWWFocusListener(EditingWW *owner) {
        m_hOwn = owner;
    }

    void EditingWWFocusListener::focusLost(const FocusEvent& event) {
        if (hge->Input_GetKeyState(HGEK_TAB)) {
            m_hOwn->TextEditMoveToNextTile();
            m_hOwn->tfWriteID->requestFocus();
        } else {
            m_hOwn->conWriteID->setShow(false);
            m_hOwn->iTileWriteIDx = m_hOwn->iTileWriteIDy = -1;
        }
        m_hOwn->vPort->MarkToRedraw(true);
    }

    EditingWWKeyListener::EditingWWKeyListener(EditingWW *owner) {
        m_hOwn = owner;
        lastPressedWasAlt = false;
    }

    void EditingWWKeyListener::keyPressed(KeyEvent &keyEvent) {
        lastPressedWasAlt = false;

        if (keyEvent.getKey() == Key::ESCAPE) {
            if (keyEvent.getType() == KeyEvent::PRESSED) {
                if (m_hOwn->NewMap_data) {
                    m_hOwn->NewMap_Close();
                }
                if (m_hOwn->winWorld->isVisible()) {
                    m_hOwn->winWorld->setVisible(false);
                }
                if (m_hOwn->winpmMain->isVisible()) {
                    m_hOwn->winpmMain->setVisible(false);
                }
                if (m_hOwn->winTileProp->isVisible()) {
                    m_hOwn->winTileProp->setVisible(false);
                }
                if (m_hOwn->winLogicBrowser->isVisible()) {
                    m_hOwn->winLogicBrowser->setVisible(false);
                }

                switch (m_hOwn->iActiveTool) {
                    case EWW_TOOL_DUPLICATE:
                    case EWW_TOOL_BRUSHOBJECT:
                    case EWW_TOOL_ALIGNOBJ:
                    case EWW_TOOL_SPACEOBJ:
                        m_hOwn->SetTool(EWW_TOOL_NONE);
                        break;
                }

                for (cWindow *win : m_hOwn->hWindows) {
                    win->Close();
                }

                if (m_hOwn->MDI->GetActiveDocIt() == -1 && m_hOwn->MDI->GetDocsCount()) {
                    m_hOwn->MDI->BackToLastActive();
                }
            }
            return;
        } else if (keyEvent.getKey() == Key::F11) {
            m_hOwn->ToggleFullscreen();
            return;
        }

        if (keyEvent.isControlPressed()) {
            switch (keyEvent.getKey().getValue()) {
                case 'd':
                    if (m_hOwn->iMode == EWW_MODE_OBJECT && !m_hOwn->vObjectsPicked.empty()) {
                        for (auto &object : m_hOwn->vObjectsPicked) {
                            object = new WWD::Object(object);
                            object->SetUserData(new cObjUserData(object));
                            m_hOwn->plMain->AddObjectAndCalcID(object);
                            GetUserDataFromObj(object)->SyncToObj();
                            m_hOwn->hPlaneData[m_hOwn->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(object);
                        }

                        m_hOwn->SetTool(EWW_TOOL_MOVEOBJECT);
                        m_hOwn->bEditObjDelete = true;
                        m_hOwn->iMoveRelX = m_hOwn->vObjectsPicked[0]->GetParam(WWD::Param_LocationX);
                        m_hOwn->iMoveRelY = m_hOwn->vObjectsPicked[0]->GetParam(WWD::Param_LocationY);
                    }
                    break;
                case 'n':
                    m_hOwn->NewMap_Open();
                    break;
                case 'o':
                    m_hOwn->OpenDocuments();
                    break;
                case 's':
                    if (!m_hOwn->MDI->GetActiveDoc()) return;
                    if (keyEvent.isShiftPressed()) {
                        m_hOwn->SaveAs();
                    } else {
                        m_hOwn->MDI->SaveCurrent();
                    }
                    break;
                case 'w':
                    if (m_hOwn->MDI->GetActiveDoc()) {
                        m_hOwn->MDI->CloseDocByIt(m_hOwn->MDI->GetActiveDocIt());
                    }
                    break;
                case 't':
                    if (keyEvent.isShiftPressed()) {
                        if (m_hOwn->MDI->GetCachedClosedDocsCount() > 0) {
                            GV->StateMgr->Push(new State::LoadMap(m_hOwn->MDI->GetMostRecentlyClosedDoc().c_str()));
                        }
                    }
                    break;
                case 'r':
                    m_hOwn->hRulers->SetVisible(!m_hOwn->hRulers->IsVisible());
                    break;
            }
        } else if (keyEvent.isAltPressed()) {
            switch (keyEvent.getKey().getValue()) {
                case 'd':
                    if (m_hOwn->iMode == EWW_MODE_OBJECT && !m_hOwn->vObjectsPicked.empty()) {
                        m_hOwn->SetTool(EWW_TOOL_DUPLICATE);
                    }
                    break;
                case Key::LEFT_ALT:
                case Key::RIGHT_ALT:
                    lastPressedWasAlt = true;
                    break;
            }
        }
    }

    void EditingWWKeyListener::keyReleased(KeyEvent &keyEvent) {
        if (keyEvent.getKey() == Key::LEFT_ALT && lastPressedWasAlt && !m_hOwn->hAppMenu->isFocused()) {
            m_hOwn->hAppMenu->requestFocus();
            m_hOwn->hAppMenu->keyPressed(keyEvent);
        }
    }

    EditingWWMouseListener::EditingWWMouseListener(State::EditingWW *owner) {
        m_hOwn = owner;
    }

    void EditingWWMouseListener::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.getY() > 0 && mouseEvent.getY() < LAY_APPMENU_H) {
            int rx = mouseEvent.getX() - (hge->System_GetState(HGE_SCREENWIDTH) - LAY_APP_BUTTONS_COUNT * LAY_APP_BUTTON_W - m_hOwn->bMaximized * 2);
            if (rx < 0) {
                if (mouseEvent.getX() > LAY_APPMENU_X + m_hOwn->hAppMenu->getWidth()) {
                    if (mouseEvent.getClickCount() == 2) {
                        m_hOwn->ToggleFullscreen();
                    } else {
                        m_hOwn->bDragWindow = true;
                        POINT mouse;
                        GetCursorPos(&mouse);
                        m_hOwn->iWindowDragX = mouse.x;
                        m_hOwn->iWindowDragY = mouse.y;
                        GetWindowRect(hge->System_GetState(HGE_HWND), &m_hOwn->windowDragStartRect);
                    }
                }
            } else {
                int i = rx / LAY_APP_BUTTON_W;
                switch (i) {
                    case 0:
                        ShowWindow(hge->System_GetState(HGE_HWND), SW_MINIMIZE);
                        break;
                    case 1:
                        m_hOwn->ToggleFullscreen();
                        break;
                    case 2:
                        if (m_hOwn->PromptExit()) {
                            m_hOwn->bExit = true;
                        }
                        break;
                }
            }
        }
    }
}
