#include "../editing_ww.h"
#include "../../../shared/commonFunc.h"
#include "../../langID.h"
#include "../../cObjectUserData.h"
#include "../objprop.h"
#include <cmath>
#include "../../cNativeController.h"
#include "../../objEdit/editCurse.h"
#include "../../objEdit/editElevPath.h"
#include "../../objEdit/editCheckpoint.h"
#include "../../objEdit/editWarp.h"
#include "../../objEdit/editWallCannon.h"
#include "../../objEdit/editCrumblingPeg.h"
#include "../../objEdit/editStatue.h"
#include "../../objEdit/editBreakPlank.h"
#include "../../objEdit/editCrate.h"
#include "../../objEdit/editTreasure.h"
#include "../../objEdit/editRope.h"
#include "../../objEdit/editHealth.h"
#include "../../objEdit/editSpecialPowerup.h"
#include "../../objEdit/editTogglePeg.h"
#include "../../objEdit/editCandy.h"
#include "../../objEdit/editSpringboard.h"
#include "../../objEdit/editSoundTrigger.h"
#include "../../objEdit/editProjectile.h"
#include "../../objEdit/editElevator.h"
#include "../../objEdit/editCrabNest.h"
#include "../../objEdit/editShake.h"
#include "../../objEdit/editLaser.h"
#include "../../objEdit/editStalactite.h"
#include "../../objEdit/editAmbient.h"
#include "../../objEdit/editEnemy.h"
#include "../../objEdit/editText.h"
#include "../../objEdit/editFloorSpike.h"
#include "../../databanks/logics.h"
#include "../dialog.h"
#include "../../version.h"

void EditingWW_ObjDeletionCB(WWD::Object *obj) {
    auto *st = (State::EditingWW *) (GV->StateMgr->GetState());
    st->NotifyObjectDeletion(obj);
}

void State::EditingWW::NotifyObjectDeletion(WWD::Object *obj) {
    GetUserDataFromObj(obj)->ClearCellReferences();
    delete GetUserDataFromObj(obj);
    for (int i = 0; i < vObjectsBrushCB.size(); i++)
        if (vObjectsBrushCB[i] == obj) {
            vObjectsBrushCB.erase(vObjectsBrushCB.begin() + i);
            i--;
        }
    for (int i = 0; i < vObjectsHL.size(); i++)
        if (vObjectsHL[i] == obj) {
            vObjectsHL.erase(vObjectsHL.begin() + i);
            i--;
        }
    for (int i = 0; i < vObjectsPicked.size(); i++)
        if (vObjectsPicked[i] == obj) {
            vObjectsPicked.erase(vObjectsPicked.begin() + i);
            i--;
        }
}

void State::EditingWW::ObjectOverlay() {
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    if (iMode == EWW_MODE_OBJECT && iActiveTool == EWW_TOOL_OBJSELAREA &&
        (toolsaMinX != 0 || toolsaMinY != 0 || toolsaMaxX != 0 || toolsaMaxY != 0 || (toolsaAction != TOOL_OBJSA_NONE && toolsaAction != TOOL_OBJSA_PICKALL))) {
        int x1 = toolsaMinX, x2 = toolsaMaxX == 0 ? GetActivePlane()->GetPlaneWidthPx() : toolsaMaxX,
            y1 = toolsaMinY, y2 = toolsaMaxY == 0 ? GetActivePlane()->GetPlaneHeightPx() : toolsaMaxY;

        if (x1 > x2) std::swap(x1, x2);
        if (y1 > y2) std::swap(y1, y2);

        x1 = Wrd2ScrX(GetActivePlane(), x1);
        y1 = Wrd2ScrY(GetActivePlane(), y1);
        x2 = Wrd2ScrX(GetActivePlane(), x2);
        y2 = Wrd2ScrY(GetActivePlane(), y2);

        if (vPort->GetWidget()->isMouseOver()) {
            if (toolsaAction == TOOL_OBJSA_PICKMINX && mx < x1)
                x1 = mx;
            else if (toolsaAction == TOOL_OBJSA_PICKMINY && my < y1)
                y1 = my;
            else if (toolsaAction == TOOL_OBJSA_PICKMAXX && mx > x2)
                x2 = mx;
            else if (toolsaAction == TOOL_OBJSA_PICKMAXY && my > y2)
                y2 = my;
        }

        vPort->ClipScreen();
        RenderAreaRect(WWD::Rect(x1, y1, x2, y2));
    }
    if (bDragSelection && hParser != NULL && iMode == EWW_MODE_OBJECT) {
        int x1 = Wrd2ScrX(GetActivePlane(), iDragSelectionOrigX), x2 = mx,
            y1 = Wrd2ScrY(GetActivePlane(), iDragSelectionOrigY), y2 = my;

        if (x1 > x2) std::swap(x1, x2);
        if (y1 > y2) std::swap(y1, y2);

        if (x2 - x1 > 2 || y2 - y1 > 2) {
            vPort->ClipScreen();
            RenderAreaRect(WWD::Rect(x1, y1, x2, y2));

            if (iActiveTool == EWW_TOOL_OBJSELAREA) {
                int wmX = Scr2WrdX(GetActivePlane(), mx),
                    wmY = Scr2WrdY(GetActivePlane(), my);
                int minX = iDragSelectionOrigX > wmX ? wmX : iDragSelectionOrigX,
                    minY = iDragSelectionOrigY > wmY ? wmY : iDragSelectionOrigY,
                    maxX = iDragSelectionOrigX > wmX ? iDragSelectionOrigX : wmX,
                    maxY = iDragSelectionOrigY > wmY ? iDragSelectionOrigY : wmY;
                GV->fntMyriad16->printf(q.v[2].x + 25, q.v[2].y + 1, HGETEXT_LEFT, "~l~%s: %d, %s: %d", 0,
                                        GETL(Lang_MinX), minX, GETL(Lang_MinY), minY);
                GV->fntMyriad16->printf(q.v[2].x + 24, q.v[2].y, HGETEXT_LEFT, "~w~%s: ~y~%d~w~, %s: ~y~%d", 0,
                                        GETL(Lang_MinX), minX, GETL(Lang_MinY), minY);

                GV->fntMyriad16->printf(q.v[2].x + 25, q.v[2].y + 21, HGETEXT_LEFT, "~l~%s: %d, %s: %d", 0,
                                        GETL(Lang_MaxX), maxX, GETL(Lang_MaxY), maxY);
                GV->fntMyriad16->printf(q.v[2].x + 24, q.v[2].y + 20, HGETEXT_LEFT, "~w~%s: ~y~%d~w~, %s: ~y~%d", 0,
                                        GETL(Lang_MaxX), maxX, GETL(Lang_MaxY), maxY);
            } else {
                int count = vObjectsHL.size();

                if (hge->Input_GetKeyState(HGEK_SHIFT)) {
                    for (auto obj : vObjectsPicked) {
                        if (std::find(vObjectsHL.begin(), vObjectsHL.end(), obj) == vObjectsHL.end())
                            ++count;
                        else --count;
                    }
                }

                GV->fntMyriad16->printf(x2 + 25, y2 + 1, HGETEXT_LEFT, "~l~%s: %d", 0,
                                        GETL(Lang_SelectedObjects), count);
                GV->fntMyriad16->printf(x2 + 24, y2, HGETEXT_LEFT, "~w~%s: ~y~%d", 0,
                                        GETL(Lang_SelectedObjects), count);
            }
        }
    }
    if (((iMode == EWW_MODE_OBJECT && iActiveTool == EWW_TOOL_MOVEOBJECT) ||
         (iMode == EWW_MODE_OBJECT && iActiveTool == EWW_TOOL_EDITOBJ && hEditObj->IsMovingObject()))
        && conMain->getWidgetAt(mx, my) == vPort->GetWidget()) {
        int diffX, diffY;
        if (iActiveTool == EWW_TOOL_MOVEOBJECT) {
            diffX = GetUserDataFromObj(vObjectsPicked[0])->GetX() - vObjectsPicked[0]->GetParam(WWD::Param_LocationX);
            diffY = GetUserDataFromObj(vObjectsPicked[0])->GetY() - vObjectsPicked[0]->GetParam(WWD::Param_LocationY);
        } else if (iActiveTool == EWW_TOOL_EDITOBJ) {
            diffX = hEditObj->GetTempObj()->GetParam(WWD::Param_LocationX) - hEditObj->_iMoveInitX;
            diffY = hEditObj->GetTempObj()->GetParam(WWD::Param_LocationY) - hEditObj->_iMoveInitY;
        }
        GV->fntMyriad16->printf(mx + 26, my + 1, HGETEXT_LEFT, "~l~X: %+d", 0, diffX);
        GV->fntMyriad16->printf(mx + 26, my + 21, HGETEXT_LEFT, "~l~Y: %+d", 0, diffY);
        GV->fntMyriad16->printf(mx + 25, my, HGETEXT_LEFT, "~w~X: ~y~%+d", 0, diffX);
        GV->fntMyriad16->printf(mx + 25, my + 20, HGETEXT_LEFT, "~w~Y: ~y~%+d", 0, diffY);
    }
}

bool State::EditingWW::ObjectThink(bool pbConsumed) {
    if (iActiveTool == EWW_TOOL_EDITOBJ && hEditObj->Kill()) {
        bool bAddNext = hEditObj->IsAddingNext();
        int objectMoveRelX, objectMoveRelY;
        void *specialPtr = NULL;
        if (bAddNext) {
            objectMoveRelX = vObjectsPicked[0]->GetParam(WWD::Param_LocationX);
            objectMoveRelY = vObjectsPicked[0]->GetParam(WWD::Param_LocationY);
            specialPtr = hEditObj->GenerateNextObjectData();
        }
        std::vector<WWD::Object *> vObjToPick;
        if (hEditObj->iType == ObjEdit::enText && ((ObjEdit::cEditObjText *) hEditObj)->ObjectSaved()) {
            vObjToPick = ((ObjEdit::cEditObjText *) hEditObj)->GetObjects();
        }

        int winX, winY;
        hEditObj->GetWindowPosition(winX, winY);

        SetTool(EWW_TOOL_NONE);
        if (!vObjToPick.empty())
            vObjectsPicked = vObjToPick;
        if (bAddNext) {
            auto *newObj = new WWD::Object(vObjectsPicked[0]);
            newObj->SetParam(WWD::Param_LocationX, Scr2WrdX(GetActivePlane(), vPort->GetX() + vPort->GetWidth() / 2));
            newObj->SetParam(WWD::Param_LocationY, Scr2WrdY(GetActivePlane(), vPort->GetY() + vPort->GetHeight() / 2));
            GetActivePlane()->AddObjectAndCalcID(newObj);
            newObj->SetUserData(new cObjUserData(newObj));
            hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(newObj);
            vObjectsPicked.clear();
            vObjectsPicked.push_back(newObj);
            objContext->EmulateClickID(OBJMENU_EDIT);
            hEditObj->_iMoveInitX = objectMoveRelX;
            hEditObj->_iMoveInitY = objectMoveRelY;
            hEditObj->ApplyDataFromPrevObject(specialPtr);
            bEditObjDelete = true;
            hEditObj->SetWindowPosition(winX, winY);
        } else if (bEditObjDelete) {
            std::vector<WWD::Object*> tmp = vObjectsPicked;
            for (auto& object : tmp) {
                GetActivePlane()->DeleteObject(object);
            }
            bEditObjDelete = false;
        }
        vPort->MarkToRedraw();
    }

    if (iActiveTool == EWW_TOOL_EDITOBJ)
        hEditObj->Think(pbConsumed);

    if (pbConsumed) {
        return true;
    }

    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    if (iActiveTool == EWW_TOOL_ALIGNOBJ) {
        if (vPort->GetWidget()->isMouseOver() && !pbConsumed && hge->Input_KeyDown(HGEK_LBUTTON)) {
            int x = Scr2WrdX(GetActivePlane(), mx), y = Scr2WrdY(GetActivePlane(), my);
            std::vector<WWD::Object *> picked = hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->GetObjectsByArea(
                    x, y, 1, 1);
            if (!picked.empty()) {
                WWD::Object *alignToObj = NULL;
                for (auto & object : vObjectsPicked)
                    if (object == picked[0]) {
                        alignToObj = picked[0];
                        break;
                    }
                if (alignToObj != 0) {
                    for (auto & object : vObjectsPicked) {
                        if (bObjectAlignAxis)
                            object->SetParam(WWD::Param_LocationY,
                                             alignToObj->GetParam(WWD::Param_LocationY));
                        else
                            object->SetParam(WWD::Param_LocationX,
                                             alignToObj->GetParam(WWD::Param_LocationX));
                        GetUserDataFromObj(object)->SyncToObj();
                    }
                    SetTool(EWW_TOOL_NONE);
                    vPort->MarkToRedraw();
                }
            }
        }
    } else if (iActiveTool == EWW_TOOL_OBJSELAREA) {
        if (toolsaAction == TOOL_OBJSA_PICKALL) {
            if (vPort->GetWidget()->isMouseOver() && !pbConsumed && hge->Input_KeyDown(HGEK_LBUTTON) &&
                !bDragSelection) {
                bDragSelection = true;
                iDragSelectionOrigX = Scr2WrdX(GetActivePlane(), mx);
                iDragSelectionOrigY = Scr2WrdY(GetActivePlane(), my);
            } else if (bDragSelection && !hge->Input_GetKeyState(HGEK_LBUTTON)) {
                int wmx = Scr2WrdX(GetActivePlane(), mx), wmy = Scr2WrdY(GetActivePlane(), my);
                toolsaMinX = iDragSelectionOrigX > wmx ? wmx : iDragSelectionOrigX;
                toolsaMinY = iDragSelectionOrigY > wmy ? wmy : iDragSelectionOrigY;
                toolsaMaxX = iDragSelectionOrigX < wmx ? wmx : iDragSelectionOrigX;
                toolsaMaxY = iDragSelectionOrigY < wmy ? wmy : iDragSelectionOrigY;
                bDragSelection = false;
                char label[200];
                sprintf(label, "~w~X1: ~y~%d~w~ Y1: ~y~%d~w~ X2: ~y~%d~w~ Y2: ~y~%d~l~",
                        toolsaMinX, toolsaMinY, toolsaMaxX, toolsaMaxY);
                labtoolSelAreaValues->setCaption(label);
                labtoolSelAreaValues->adjustSize();
                toolsaAction = TOOL_OBJSA_NONE;
            }
        } else if (toolsaAction == TOOL_OBJSA_PICKMINX) {
            if (vPort->GetWidget()->isMouseOver() && !pbConsumed && hge->Input_KeyDown(HGEK_LBUTTON)) {
                int coord = Scr2WrdX(GetActivePlane(), mx);
                if (toolsaMaxX == 0 || toolsaMaxX != 0 && coord < toolsaMaxX)
                    toolsaMinX = coord;
                char label[200];
                sprintf(label, "~w~X1: ~y~%d~w~ Y1: ~y~%d~w~ X2: ~y~%d~w~ Y2: ~y~%d~l~",
                        toolsaMinX, toolsaMinY, toolsaMaxX, toolsaMaxY);
                labtoolSelAreaValues->setCaption(label);
                labtoolSelAreaValues->adjustSize();
                toolsaAction = TOOL_OBJSA_NONE;
            }
        } else if (toolsaAction == TOOL_OBJSA_PICKMINY) {
            if (vPort->GetWidget()->isMouseOver() && !pbConsumed && hge->Input_KeyDown(HGEK_LBUTTON)) {
                int coord = Scr2WrdY(GetActivePlane(), my);
                if (toolsaMaxY == 0 || toolsaMaxY != 0 && coord < toolsaMaxY)
                    toolsaMinY = coord;
                char label[200];
                sprintf(label, "~w~X1: ~y~%d~w~ Y1: ~y~%d~w~ X2: ~y~%d~w~ Y2: ~y~%d~l~",
                        toolsaMinX, toolsaMinY, toolsaMaxX, toolsaMaxY);
                labtoolSelAreaValues->setCaption(label);
                labtoolSelAreaValues->adjustSize();
                toolsaAction = TOOL_OBJSA_NONE;
            }
        } else if (toolsaAction == TOOL_OBJSA_PICKMAXX) {
            if (vPort->GetWidget()->isMouseOver() && !pbConsumed && hge->Input_KeyDown(HGEK_LBUTTON)) {
                int coord = Scr2WrdX(GetActivePlane(), mx);
                if (toolsaMinX == 0 || toolsaMinX != 0 && coord > toolsaMinX)
                    toolsaMaxX = coord;
                char label[200];
                sprintf(label, "~w~X1: ~y~%d~w~ Y1: ~y~%d~w~ X2: ~y~%d~w~ Y2: ~y~%d~l~",
                        toolsaMinX, toolsaMinY, toolsaMaxX, toolsaMaxY);
                labtoolSelAreaValues->setCaption(label);
                labtoolSelAreaValues->adjustSize();
                toolsaAction = TOOL_OBJSA_NONE;
            }
        } else if (toolsaAction == TOOL_OBJSA_PICKMAXY) {
            if (vPort->GetWidget()->isMouseOver() && !pbConsumed && hge->Input_KeyDown(HGEK_LBUTTON)) {
                int coord = Scr2WrdY(GetActivePlane(), my);
                if (toolsaMinY == 0 || toolsaMinY != 0 && coord > toolsaMinY)
                    toolsaMaxY = coord;
                char label[200];
                sprintf(label, "~w~X1: ~y~%d~w~ Y1: ~y~%d~w~ X2: ~y~%d~w~ Y2: ~y~%d~l~",
                        toolsaMinX, toolsaMinY, toolsaMaxX, toolsaMaxY);
                labtoolSelAreaValues->setCaption(label);
                labtoolSelAreaValues->adjustSize();
                toolsaAction = TOOL_OBJSA_NONE;
            }
        }
    } else if (iActiveTool == EWW_TOOL_BRUSHOBJECT) {
        if (vObjectsBrushCB.empty()) {
            SetTool(EWW_TOOL_NONE);
        } else {
            if (bObjBrushDrawing) {
                if (!hge->Input_GetKeyState(HGEK_LBUTTON)) {
                    bObjBrushDrawing = false;
                } else {
                    float distance = DISTANCE(iobjbrLastDrawnX,
                                              iobjbrLastDrawnY,
                                              Scr2WrdX(GetActivePlane(), mx),
                                              Scr2WrdY(GetActivePlane(), my));
                    // float percDist = distance / sliobrDistance->getValue();
                    // if (percDist > 1) percDist = 1;
                    // CURSOR COLORING DEPENDING ON DISTANCE
                    // dwCursorColor = SETR(SETB(0xFF00FF00, int((1 - percDist) * 255)), int((1 - percDist) * 255));
                    if (distance > sliobrDistance->getValue()) {
                        iobjbrLastDrawnX = Scr2WrdX(GetActivePlane(), mx);
                        iobjbrLastDrawnY = Scr2WrdY(GetActivePlane(), my);

                        ObjectBrush(iobjbrLastDrawnX, iobjbrLastDrawnY);
                    }
                }
            }
            if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                iobjbrLastDrawnX = Scr2WrdX(GetActivePlane(), mx);
                iobjbrLastDrawnY = Scr2WrdY(GetActivePlane(), my);
                ObjectBrush(iobjbrLastDrawnX, iobjbrLastDrawnY);
                bObjBrushDrawing = true;
            }
        }
    }
    return false;
}

void State::EditingWW::OpenObjectWindow(WWD::Object *obj, bool bMove) {
    if (obj != hStartingPosObj)
        GV->StateMgr->Push(new ObjProp(this, obj, bMove));
}

WWD::Object *State::EditingWW::GetObjUnderWorldPos(int mx, int my) {
    return hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->GetObjectByWorldPosition(mx, my);
}

void State::EditingWW::ShowAndUpdateDuplicateMenu() {
    winDuplicate->setVisible(1);
    tfdTimes->requestFocus();
}

bool State::EditingWW::IsEditableObject(WWD::Object* obj, ObjEdit::cObjEdit** hEdit) {
    if (hParser->GetGame() != WWD::Game_Claw && hParser->GetGame() != WWD::Game_Claw2) return false;
    LogicInfo logicInfo = GetLogicInfo(obj->GetLogic());
    if (!strcmp(obj->GetLogic(), "CursePowerup")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjCurse(obj, this);
    } else if (!strcmp(obj->GetLogic(), "PathElevator")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjElevPath(obj, this);
    } else if (!strcmp(obj->GetLogic(), "Checkpoint") ||
        !strcmp(obj->GetLogic(), "FirstSuperCheckpoint") ||
        !strcmp(obj->GetLogic(), "SecondSuperCheckpoint") ||
        !strcmp(obj->GetLogic(), "BossStager")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjCheckpoint(obj, this);
    } else if (!strcmp(obj->GetLogic(), "SpecialPowerup") &&
        (!strcmp(obj->GetImageSet(), "GAME_WARP") || !strcmp(obj->GetImageSet(), "GAME_VERTWARP")) ||
        !strcmp(obj->GetLogic(), "BossWarp") && !strcmp(obj->GetImageSet(), "GAME_BOSSWARP")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjWarp(obj, this);
    } else if (!strcmp(obj->GetLogic(), "TowerCannonLeft") || !strcmp(obj->GetLogic(), "TowerCannonRight") ||
        !strcmp(obj->GetLogic(), "SkullCannon")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjWallCannon(obj, this);
    } else if (!strcmp(obj->GetLogic(), "CrumblingPeg") || !strcmp(obj->GetLogic(), "CrumblingPegNoRespawn")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjCrumblingPeg(obj, this);
    } else if (!strcmp(obj->GetLogic(), "FrontStatue") || !strcmp(obj->GetLogic(), "BehindStatue")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjStatue(obj, this);
    } else if (!strcmp(obj->GetLogic(), "BreakPlank")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjBreakPlank(obj, this);
    } else if (!strcmp(obj->GetLogic(), "FrontCrate") || !strcmp(obj->GetLogic(), "FrontStackedCrates") ||
        !strcmp(obj->GetLogic(), "BehindCrate") || !strcmp(obj->GetLogic(), "BackStackedCrates")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjCrate(obj, this);
    } else if (!strcmp(obj->GetLogic(), "GlitterlessPowerup") || !strcmp(obj->GetLogic(), "TreasurePowerup")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjTreasure(obj, this);
    } else if (!strcmp(obj->GetLogic(), "AniRope") ||
        !strcmp(obj->GetLogic(), "DoNothing") && !strcmp(obj->GetImageSet(), "LEVEL_ROPE")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjRope(obj, this);
    } else if (!strcmp(obj->GetLogic(), "HealthPowerup") &&
        !(!strcmp(obj->GetImageSet(), "GAME_CATNIPS_NIP1") ||
            !strcmp(obj->GetImageSet(), "GAME_CATNIPS_NIP2")) ||
        !strcmp(obj->GetLogic(), "AmmoPowerup") || !strcmp(obj->GetLogic(), "MagicPowerup") ||
        !strcmp(obj->GetLogic(), "SpecialPowerup") && !strcmp(obj->GetImageSet(), "GAME_DYNAMITE")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjHealth(obj, this);
    } else if (!strcmp(obj->GetLogic(), "SpecialPowerup") || !strcmp(obj->GetLogic(), "HealthPowerup")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjSpecialPowerup(obj, this);
    } else if (!strcmp(obj->GetLogic(), "TogglePeg") || !strcmp(obj->GetLogic(), "TogglePeg2") ||
        !strcmp(obj->GetLogic(), "TogglePeg3") || !strcmp(obj->GetLogic(), "TogglePeg4") || !strcmp(obj->GetLogic(), "SlidingElevator")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjTogglePeg(obj, this);
    } else if (!strcmp(obj->GetLogic(), "FloorSpike") || !strcmp(obj->GetLogic(), "FloorSpike2") ||
        !strcmp(obj->GetLogic(), "FloorSpike3") || !strcmp(obj->GetLogic(), "FloorSpike4") ||
        !strcmp(obj->GetLogic(), "SawBlade") || !strcmp(obj->GetLogic(), "SawBlade2") ||
        !strcmp(obj->GetLogic(), "SawBlade3") || !strcmp(obj->GetLogic(), "SawBlade4")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjFloorSpike(obj, this);
    } else if (!strcmp(obj->GetLogic(), "FrontAniCandy") || !strcmp(obj->GetLogic(), "BehindAniCandy") ||
        !strcmp(obj->GetLogic(), "FrontCandy") || !strcmp(obj->GetLogic(), "BehindCandy") ||
        !strcmp(obj->GetLogic(), "DoNothing") || !strcmp(obj->GetLogic(), "DoNothingNormal") ||
        !strcmp(obj->GetLogic(), "AniCycle") || !strcmp(obj->GetLogic(), "AniCycleNormal") || !strcmp(obj->GetLogic(), "SimpleAnimation")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjCandy(obj, this);
    } else if (!strcmp(obj->GetLogic(), "SpringBoard") || !strcmp(obj->GetLogic(), "GroundBlower") || !strcmp(obj->GetLogic(), "WaterRock")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjSpringboard(obj, this);
    } else if (strstr(obj->GetLogic(), "SoundTrigger")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjSoundTrigger(obj, this);
    } else if (!strcmp(obj->GetLogic(), "TProjectile")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjProjectile(obj, this);
    } else if (!strcmp(obj->GetLogic(), "Elevator") || !strcmp(obj->GetLogic(), "StartElevator") ||
        !strcmp(obj->GetLogic(), "TriggerElevator") || !strcmp(obj->GetLogic(), "StopElevator") ||
        !strcmp(obj->GetLogic(), "OneWayStartElevator") || !strcmp(obj->GetLogic(), "OneWayTriggerElevator") ||
        !strcmp(obj->GetLogic(), "StandardElevator")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjElevator(obj, this);
    } else if (!strcmp(obj->GetLogic(), "CrabNest")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjCrabNest(obj, this);
    } else if (!strcmp(obj->GetLogic(), "Shake")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjShake(obj, this);
    } else if (!strcmp(obj->GetLogic(), "Laser")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjLaser(obj, this);
    } else if (!strcmp(obj->GetLogic(), "Stalactite")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjStalactite(obj, this);
    } else if (!strcmp(obj->GetLogic(), "GlobalAmbientSound") ||
        !strcmp(obj->GetLogic(), "AmbientSound") ||
        !strcmp(obj->GetLogic(), "SpotAmbientSound") ||
        !strcmp(obj->GetLogic(), "AmbientPosSound")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjAmbient(obj, this);
    } else if (logicInfo.IsEnemy()) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjEnemy(obj, this);
    } else if (!strcmp(obj->GetLogic(), "_WM_TEXT")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjText(obj, this);
    } else {
        return false;
    }
    return true;
}

void State::EditingWW::OpenObjectEdit(WWD::Object *obj) {
    if (IsEditableObject(obj, &hEditObj)) {
        GV->Console->Printf("~w~Opening EasyEdit tool for object #~y~%d ~w~(~y~%s~w~).", obj->GetParam(WWD::Param_ID),
                            obj->GetLogic());
        SetTool(EWW_TOOL_EDITOBJ);
    } else {
        GV->Console->Printf("~r~Tried to open EasyEdit tool for object #~y~%d ~w~(~y~%s~w~), which is not editable.",
                            obj->GetParam(WWD::Param_ID), obj->GetLogic());
    }
}

const char* State::EditingWW::GetDefaultElevatorImageSet() {
    if (hParser->GetBaseLevel() == 1) return "LEVEL_ELEVATORS";
    else if (hParser->GetBaseLevel() == 13 ||
        hParser->GetBaseLevel() == 3)
        return "LEVEL_ELEVATOR1";
    else if (hParser->GetBaseLevel() == 6) return "LEVEL_GRILLELEVATOR";
    else if (hParser->GetBaseLevel() == 10) return "LEVEL_TRAPELEVATOR";
    else if (hParser->GetBaseLevel() == 7) return "LEVEL_AIRCART";
    else return "LEVEL_ELEVATOR";
}

void State::EditingWW::CreateObjectWithEasyEdit(gcn::Widget *widg) {
    auto *obj = new WWD::Object();
    if (hmbObject->GetContext()->isVisible() && hmbObject->GetContext()->GetSelectedID() != -1) {
        obj->SetParam(WWD::Param_LocationX, Scr2WrdX(GetActivePlane(), contextX));
        obj->SetParam(WWD::Param_LocationY, Scr2WrdY(GetActivePlane(), contextY));
    } else {
        obj->SetParam(WWD::Param_LocationX, Scr2WrdX(GetActivePlane(), vPort->GetX() + vPort->GetWidth() / 2));
        obj->SetParam(WWD::Param_LocationY, Scr2WrdY(GetActivePlane(), vPort->GetY() + vPort->GetHeight() / 2));
    }
    obj->SetParam(WWD::Param_LocationI, -1);

    bool bDoContext = true;
    if (widg == hmbObject->butIconCurse) {
        obj->SetLogic("CursePowerup");
        obj->SetImageSet("GAME_CURSES_FREEZE");
    } else if (widg == hmbObject->butIconSpikes) {
        if (hParser->GetBaseLevel() == 9) {
            obj->SetLogic("SawBlade");
            obj->SetImageSet("LEVEL_FLOORSAW");
        } else {
            obj->SetLogic("FloorSpike");
            obj->SetImageSet(hParser->GetBaseLevel() == 3 || hParser->GetBaseLevel() == 10 || hParser->GetBaseLevel() == 13 ? "LEVEL_FLOORSPIKES1" : "LEVEL_FLOORSPIKES");
        }
    } else if (widg == hmbObject->butIconElevator) {
        obj->SetLogic("Elevator");
        obj->SetImageSet(GetDefaultElevatorImageSet());
        obj->SetParam(WWD::Param_MinX, obj->GetX());
        obj->SetParam(WWD::Param_MaxX, obj->GetX() + 150);
    } else if (widg == hmbObject->butIconPathElevator) {
        obj->SetLogic("PathElevator");
        obj->SetImageSet(GetDefaultElevatorImageSet());
        obj->SetMoveRect(WWD::Rect(2, 100, 0, 0));
    } else if (widg == hmbObject->butIconCheckpoint) {
        obj->SetLogic("Checkpoint");
        obj->SetImageSet("GAME_CHECKPOINTFLAG");
    } else if (widg == hmbObject->butIconWarp) {
        obj->SetLogic("SpecialPowerup");
        obj->SetImageSet("GAME_WARP");
    } else if (widg == hmbObject->butIconMapPiece) {
        obj->SetLogic("EndOfLevelPowerup");
        obj->SetImageSet("GAME_MAPPIECE");
        bDoContext = false;
    } else if (widg == hmbObject->butIconPowderKeg) {
        obj->SetLogic("PowderKeg");
        obj->SetImageSet("LEVEL_POWDERKEG");
        bDoContext = false;
    } else if (widg == hmbObject->butIconCannon) {
        if (hParser->GetBaseLevel() == 9) {
            obj->SetLogic("SkullCannon");
            obj->SetImageSet("LEVEL_SKULLCANNON");
        } else {
            obj->SetLogic("TowerCannonRight");
            obj->SetImageSet("LEVEL_TOWERCANNONRIGHT");
        }
    } else if (widg == hmbObject->butIconCrumblinPeg) {
        obj->SetLogic("CrumblingPeg");
        if (hParser->GetBaseLevel() == 1)
            obj->SetImageSet("LEVEL_CRUMBLINGPEG");
        else if (hParser->GetBaseLevel() == 3)
            obj->SetImageSet("LEVEL_CRUMBLINPEG1");
        else if (hParser->GetBaseLevel() == 4 || hParser->GetBaseLevel() == 13)
            obj->SetImageSet("LEVEL_CRUMBLINGBUSH");
        else if (hParser->GetBaseLevel() == 6)
            obj->SetImageSet("LEVEL_BREAKINGLEDGE");
        else if (hParser->GetBaseLevel() == 7 || hParser->GetBaseLevel() == 12)
            obj->SetImageSet("LEVEL_CRUMBLINPEG");
        else if (hParser->GetBaseLevel() == 10)
            obj->SetImageSet("LEVEL_CRUMBLEPEG");
        else if (hParser->GetBaseLevel() == 14)
            obj->SetImageSet("LEVEL_CRUMBLINPEG2");
    } else if (widg == hmbObject->butIconStatue) {
        obj->SetLogic("FrontStatue");
        obj->SetImageSet("LEVEL_STATUE");
        obj->SetParam(WWD::Param_LocationZ, 5000);
    } else if (widg == hmbObject->butIconBreakPlank) {
        obj->SetLogic("BreakPlank");
        obj->SetImageSet("LEVEL_BREAKPLANK");
        obj->SetParam(WWD::Param_Counter, 500);
    } else if (widg == hmbObject->butIconCrate) {
        obj->SetLogic("FrontCrate");
        obj->SetParam(WWD::Param_LocationZ, 5000);
        if (hParser->GetBaseLevel() == 11)
            obj->SetImageSet("LEVEL_BREAKJEM");
        else if (hParser->GetBaseLevel() == 14)
            obj->SetImageSet("LEVEL_STATUE");
        else
            obj->SetImageSet("LEVEL_CRATES");
    } else if (widg == hmbObject->butIconTreasure) {
        obj->SetLogic("GlitterlessPowerup");
        obj->SetImageSet("GAME_TREASURE_COINS");
    } else if (widg == hmbObject->butIconRope) {
        obj->SetLogic("AniRope");
        obj->SetImageSet("LEVEL_ROPE");
    } else if (widg == hmbObject->butIconHealth) {
        obj->SetLogic("HealthPowerup");
        obj->SetImageSet("LEVEL_HEALTH");
    } else if (widg == hmbObject->butIconCatnip) {
        obj->SetLogic("SpecialPowerup");
        obj->SetImageSet("GAME_POWERUPS_EXTRALIFE");
    } else if (widg == hmbObject->butIconTogglePeg) {
        obj->SetLogic("TogglePeg");
    } else if (widg == hmbObject->butIconEyeCandy) {
        obj->SetLogic("DoNothing");
        obj->SetParam(WWD::Param_LocationI, -1);
        obj->SetParam(WWD::Param_LocationZ, 990);
    } else if (widg == hmbObject->butIconSpringBoard) {
        obj->SetLogic(hParser->GetBaseLevel() == 6 ? "GroundBlower" : "SpringBoard");
        if (hParser->GetBaseLevel() == 4 || hParser->GetBaseLevel() == 13)
            obj->SetImageSet("LEVEL_SPRINGYFERN");
        else if (hParser->GetBaseLevel() == 6)
            obj->SetImageSet("LEVEL_BLOWING1");
        else if (hParser->GetBaseLevel() == 7 || hParser->GetBaseLevel() == 12)
            obj->SetImageSet("LEVEL_WATERROCK");
        else if (hParser->GetBaseLevel() == 9)
            obj->SetImageSet("LEVEL_SPRINGBOX1");
    } else if (widg == hmbObject->butIconProjectile) {
        obj->SetLogic("TProjectile");
        if (hParser->GetBaseLevel() == 14) {
            obj->SetImageSet("LEVEL_SHOOTERS_LAUNCHLEFT");
            obj->SetAnim("LEVEL_SHOOTERS_LAUNCHLEFT");
        } else {
            obj->SetImageSet("LEVEL_SHOOTERS_PUFFDARTRIGHT");
            obj->SetAnim("LEVEL_SHOOTERS_PUFFDARTRIGHT");
        }
        obj->SetParam(WWD::Param_Speed, 500);
        obj->SetParam(WWD::Param_Direction, 6);
        obj->SetUserValue(0, 1);
    } else if (widg == hmbObject->butIconCrabNest) {
        obj->SetLogic("CrabNest");
        obj->SetImageSet("LEVEL_SANDHOLE");
    } else if (widg == hmbObject->butIconShake) {
        obj->SetLogic("Shake");
        obj->SetImageSet("GAME_SOUNDICON");
        obj->SetDrawFlags(WWD::Flag_dr_NoDraw);
        obj->SetAttackRect(WWD::Rect(std::max(obj->GetParam(WWD::Param_LocationX) - 100, 1),
                                     std::max(obj->GetParam(WWD::Param_LocationY) - 100, 1),
                                     std::min(obj->GetParam(WWD::Param_LocationX) + 100,
                                              GetActivePlane()->GetPlaneWidthPx() - 1),
                                     std::min(obj->GetParam(WWD::Param_LocationY) + 100,
                                              GetActivePlane()->GetPlaneHeightPx() - 1)));
    } else if (widg == hmbObject->butIconStalactite) {
        obj->SetLogic("Stalactite");
        obj->SetImageSet("LEVEL_PROJECTILES_STALACTITE");
        obj->SetUserRect(0, WWD::Rect(std::max(obj->GetParam(WWD::Param_LocationX) - 100, 1),
                                      std::min(obj->GetParam(WWD::Param_LocationY) + 100,
                                               GetActivePlane()->GetPlaneHeightPx() - 1),
                                      std::min(obj->GetParam(WWD::Param_LocationX) + 100,
                                               GetActivePlane()->GetPlaneWidthPx() - 1),
                                      std::min(obj->GetParam(WWD::Param_LocationY) + 300,
                                               GetActivePlane()->GetPlaneHeightPx() - 1)));
    } else if (widg == hmbObject->butIconLaser) {
        obj->SetLogic("Laser");
        obj->SetImageSet("LEVEL_LASER");
        obj->SetParam(WWD::Param_Damage, 10);
        obj->SetParam(WWD::Param_Counter, 1500);
    } else if (widg == hmbObject->butIconSound) {
        obj->SetLogic("GlobalAmbientSound");
        obj->SetImageSet("GAME_SOUNDICON");
        obj->SetDrawFlags(WWD::Flag_dr_NoDraw);
    } else if (widg == hmbObject->butIconDialog) {
        obj->SetLogic("SoundTrigger");
        obj->SetImageSet("GAME_SOUNDICON");
        obj->SetDrawFlags(WWD::Flag_dr_NoDraw);
    } else if (widg == hmbObject->butIconText) {
        obj->SetLogic("_WM_TEXT");
    } else if (widg == hmbObject->butIconEnemy) {
        std::vector<std::pair<std::string, std::string>> vstrpTypes;
        LogicInfo::GetEnemyLogicPairs(vstrpTypes, hParser->GetBaseLevel());
        ObjEdit::cEditObjEnemy::UpdateEnemyObject(obj, vstrpTypes[0]);
    }
    GetActivePlane()->AddObjectAndCalcID(obj);
    obj->SetUserData(new cObjUserData(obj));
    hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(obj);
    vObjectsPicked.clear();
    vObjectsPicked.push_back(obj);
    if (bDoContext) {
        objContext->EmulateClickID(OBJMENU_EDIT);
    } else {
        SetTool(EWW_TOOL_MOVEOBJECT);
        iMoveRelX = obj->GetParam(WWD::Param_LocationX);
        iMoveRelY = obj->GetParam(WWD::Param_LocationY);
        hge->Input_SetMousePos(Wrd2ScrX(GetActivePlane(), iMoveRelX),
                               Wrd2ScrY(GetActivePlane(), iMoveRelY));
    }
    bEditObjDelete = true;
    vPort->MarkToRedraw();
}

bool State::EditingWW::AreObjectSpecificOptionsAvailable(WWD::Object *obj, SHR::Context **conMod) {
    if (!strcmp(obj->GetLogic(), "SpecialPowerup") &&
        (!strcmp(obj->GetImageSet(), "GAME_WARP") || !strcmp(obj->GetImageSet(), "GAME_VERTWARP")) ||
        !strcmp(obj->GetLogic(), "BossWarp")) {
        if (conMod != 0)
            *conMod = advcon_Warp;
        return true;
    } else if (!strcmp(obj->GetLogic(), "FrontCrate") || !strcmp(obj->GetLogic(), "FrontStackedCrates") ||
               !strcmp(obj->GetLogic(), "BehindCrate") || !strcmp(obj->GetLogic(), "BackStackedCrates")) {
        if (conMod != 0)
            *conMod = advcon_Container;
        return true;
    }
    return false;
}

std::vector<cInventoryItem> State::EditingWW::GetContainerItems(WWD::Object *obj) {
    return std::vector<cInventoryItem>();
}

void State::EditingWW::UpdateSearchResults() {
    vObjSearchResults.clear();
    if (szObjSearchBuffer == NULL || strlen(szObjSearchBuffer) == 0) {
        winSearchObj->setHeight(135);
        labObjSearchResults->setVisible(false);
        butObjSearchSelectAll->setVisible(false);
        sliSearchObj->setVisible(false);
        return;
    }
    char *comp2 = szObjSearchBuffer;
    if (!cbObjSearchCaseSensitive->isSelected()) comp2 = SHR::ToLower(szObjSearchBuffer);
    for (int i = 0; i < GetActivePlane()->GetObjectsCount(); i++) {
        const char *comp = NULL;
        if (ddObjSearchTerm->getSelected() == 1) comp = GetActivePlane()->GetObjectByIterator(i)->GetLogic();
        else if (ddObjSearchTerm->getSelected() == 2) comp = GetActivePlane()->GetObjectByIterator(i)->GetImageSet();
        else if (ddObjSearchTerm->getSelected() == 3) comp = GetActivePlane()->GetObjectByIterator(i)->GetAnim();
        else comp = GetActivePlane()->GetObjectByIterator(i)->GetName();

        if (!cbObjSearchCaseSensitive->isSelected())
            comp = (const char *) SHR::ToLower(comp);

        if (strstr(comp, comp2) != NULL) {
            vObjSearchResults.emplace_back(i, GetActivePlane()->GetObjectByIterator(i)->GetParam(WWD::Param_ID));

            if (!cbObjSearchCaseSensitive->isSelected())
                delete[](char *) comp;
        }
    }
    if (!cbObjSearchCaseSensitive->isSelected()) delete[] comp2;
    int normalHeight = int(135 + vObjSearchResults.size() * 140);
    sliSearchObj->setVisible(normalHeight > 515);
    sliSearchObj->setScaleEnd(vObjSearchResults.size() * 140 - 387);
    sliSearchObj->setValue(vObjSearchResults.size() * 140 - 387);
    int winHeight = std::min(normalHeight, 515);
    winSearchObj->setHeight(winHeight);
    if (vObjSearchResults.empty()) {
        labObjSearchResults->setCaption(GETL2S("ObjectSearch", "NoResults"));
    } else {
        char tmp[128];
        sprintf(tmp, "%s: ~y~%d~l~", GETL2S("ObjectSearch", "FoundResults"), vObjSearchResults.size());
        labObjSearchResults->setCaption(tmp);
    }
    butObjSearchSelectAll->setVisible(!vObjSearchResults.empty());
    labObjSearchResults->adjustSize();
    labObjSearchResults->setVisible(true);
}

bool State::EditingWW::UpdateMovedObjectWithRects(std::vector<WWD::Object *>& vector, bool prompt) {
    bool reCalculate = false;
    if (prompt) {
        for (auto object : vector) {
            if (object->ShouldPromptForRectChange(hParser)) {
                switch (State::MessageBox(PRODUCT_NAME, GETL2S("Various", "MsgRecalcMinMaxXY"),
                                      ST_DIALOG_ICON_QUESTION, ST_DIALOG_BUT_YESNOCANCEL)) {
                    case RETURN_YES:
                        reCalculate = true;
                    break;
                    case RETURN_CANCEL:
                        for (auto obj : vector) {
                            GetUserDataFromObj(obj)->SyncToObj();
                        }
                        return false;
                }
                break;
            }
        }
    } else {
        reCalculate = true;
    }
    for (auto obj : vector) {
        if (reCalculate || (strstr(obj->GetLogic(), "Elevator") && strcmp(obj->GetLogic(), "SlidingElevator") != 0)) {
            int diffX = obj->GetX() - obj->GetParam(WWD::Param_LocationX),
                diffY = obj->GetY() - obj->GetParam(WWD::Param_LocationY);

            if (!strcmp(obj->GetLogic(), "Shake")) {
                WWD::Rect attackRect = obj->GetAttackRect();
                if (attackRect.x1 != 0) attackRect.x1 += diffX;
                if (attackRect.y1 != 0) attackRect.y1 += diffY;
                if (attackRect.x2 != 0) attackRect.x2 += diffX;
                if (attackRect.y2 != 0) attackRect.y2 += diffY;
                obj->SetAttackRect(attackRect);
            } else {
                if (obj->GetParam(WWD::Param_MinX) != 0)
                    obj->SetParam(WWD::Param_MinX,
                                  obj->GetParam(WWD::Param_MinX) + diffX);
                if (obj->GetParam(WWD::Param_MinY) != 0)
                    obj->SetParam(WWD::Param_MinY,
                                  obj->GetParam(WWD::Param_MinY) + diffY);
                if (obj->GetParam(WWD::Param_MaxX) != 0)
                    obj->SetParam(WWD::Param_MaxX,
                                  obj->GetParam(WWD::Param_MaxX) + diffX);
                if (obj->GetParam(WWD::Param_MaxY) != 0)
                    obj->SetParam(WWD::Param_MaxY,
                                  obj->GetParam(WWD::Param_MaxY) + diffY);
            }
        }
        obj->SetParam(WWD::Param_LocationX, obj->GetX());
        obj->SetParam(WWD::Param_LocationY, obj->GetY());
        hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(obj);
        if (obj == hStartingPosObj) {
            hParser->SetStartX(obj->GetX());
            hParser->SetStartY(obj->GetY());
        }
    }
    return true;
}

void State::EditingWW::ObjectBrush(int x, int y) {
    std::vector<WWD::Object*> createdObjects;
    int scatterX = atoi(tfobrDispX->getText().c_str()),
        scatterY = atoi(tfobrDispY->getText().c_str());
    int scatVX, scatVY;
    for (int i = 0; i < vObjectsBrushCB.size(); i++) {
        if (i == 0 || cbobrApplyScatterSeparately->isSelected()) {
            scatVX = hge->Random_Int(-scatterX, scatterX);
            scatVY = hge->Random_Int(-scatterY, scatterY);
        }
        auto *obj = new WWD::Object(vObjectsBrushCB[i]);
        obj->SetUserData(new cObjUserData(obj));
        int diffX = vObjectsBrushCB[i]->GetParam(WWD::Param_LocationX)
                  - vObjectsBrushCB[0]->GetParam(WWD::Param_LocationX),
            diffY = vObjectsBrushCB[i]->GetParam(WWD::Param_LocationY)
                  - vObjectsBrushCB[0]->GetParam(WWD::Param_LocationY);
        GetUserDataFromObj(obj)->SetPos(x + diffX + scatVX,
                                        y + diffY + scatVY);
        GetActivePlane()->AddObjectAndCalcID(obj);
        createdObjects.push_back(obj);
    }

    if (cbobrUpdateRects->isSelected()) {
        UpdateMovedObjectWithRects(createdObjects, false);
    } else {
        for (auto & obj : createdObjects) {
            obj->SetParam(WWD::Param_LocationX, obj->GetX());
            obj->SetParam(WWD::Param_LocationY, obj->GetY());
            hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(obj);
        }
    }

    MarkUnsaved();
    vPort->MarkToRedraw();
}

void State::EditingWW::OnResize() {
    if (!bMaximized) {
        GV->iScreenW = hge->System_GetState(HGE_SCREENWIDTH);
        GV->iScreenH = hge->System_GetState(HGE_SCREENHEIGHT);
    }
    FixInterfacePositions();
}

void State::EditingWW::FlipObjects(std::vector<WWD::Object *>& objects, bool horizontally, bool vertically) {
    if (objects.size() < 2 || (!horizontally && !vertically)) return;
    int minX = objects[0]->GetX(), minY = objects[0]->GetY(), maxX = minX, maxY = minY;

    for (int i = 1; i < objects.size(); ++i) {
        if (objects[i]->GetX() > maxX) maxX = objects[i]->GetX();
        else if (objects[i]->GetX() < minX) minX = objects[i]->GetX();

        if (objects[i]->GetY() > maxY) maxY = objects[i]->GetY();
        else if (objects[i]->GetY() < minY) minY = objects[i]->GetY();
    }

    int x = minX + maxX, y = minY + maxY;

    for (WWD::Object *object : objects) {
        if (horizontally) GetUserDataFromObj(object)->SetX(x - object->GetX());
        if (vertically) GetUserDataFromObj(object)->SetY(y - object->GetY());
    }

    if (UpdateMovedObjectWithRects(objects, true)) {
        MarkUnsaved();
        vPort->MarkToRedraw();
    }
}
