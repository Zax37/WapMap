#include "../editing_ww.h"
#include "../../globals.h"
#include "../../../shared/commonFunc.h"
#include "../../langID.h"
#include "../../cObjectUserData.h"
#include "../objprop.h"
#include <math.h>

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
#include "../../objEdit/editDialog.h"
#include "../../objEdit/editProjectile.h"
#include "../../objEdit/editElevator.h"
#include "../../objEdit/editCrabNest.h"
#include "../../objEdit/editShake.h"
#include "../../objEdit/editLaser.h"
#include "../../objEdit/editStalactite.h"
#include "../../objEdit/editAmbient.h"
#include "../../objEdit/editEnemy.h"
#include "../../objEdit/editText.h"

#include "../../databanks/logics.h"

extern HGE *hge;

void EditingWW_ObjDeletionCB(WWD::Object *obj) {
    State::EditingWW *st = (State::EditingWW *) (GV->StateMgr->GetState());
    st->NotifyObjectDeletion(obj);
}

void State::EditingWW::NotifyObjectDeletion(WWD::Object *obj) {
    GetUserDataFromObj(obj)->ClearCellReferences();
    delete GetUserDataFromObj(obj);
    for (int i = 0; i < vObjectClipboard.size(); i++)
        if (vObjectClipboard[i] == obj) {
            vObjectClipboard.erase(vObjectClipboard.begin() + i);
            i--;
        }
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
        (toolsaMinX != 0 || toolsaMinY != 0 || toolsaMaxX != 0 || toolsaMaxY != 0 || toolsaAction != TOOL_OBJSA_NONE)) {
        hgeQuad q;
        q.blend = BLEND_DEFAULT;
        q.tex = 0;
        q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 0;
        q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = 0x7700ffd8;
        vPort->ClipScreen();
        q.v[0].x = Wrd2ScrX(GetActivePlane(), toolsaMinX);
        q.v[0].y = Wrd2ScrY(GetActivePlane(), toolsaMinY);
        q.v[2].x = Wrd2ScrX(GetActivePlane(), (toolsaMaxX == 0 ? GetActivePlane()->GetPlaneWidthPx() : toolsaMaxX));
        q.v[2].y = Wrd2ScrY(GetActivePlane(), (toolsaMaxY == 0 ? GetActivePlane()->GetPlaneHeightPx() : toolsaMaxY));

        if (vPort->GetWidget()->isMouseOver()) {
            if (toolsaAction == TOOL_OBJSA_PICKMINX && mx < q.v[2].x)
                q.v[0].x = mx;
            else if (toolsaAction == TOOL_OBJSA_PICKMINY && my < q.v[2].y)
                q.v[0].y = my;
            else if (toolsaAction == TOOL_OBJSA_PICKMAXX && mx > q.v[0].x)
                q.v[2].x = mx;
            else if (toolsaAction == TOOL_OBJSA_PICKMAXY && my > q.v[0].y)
                q.v[2].y = my;
        }

        if (q.v[0].x < vPort->GetX()) q.v[0].x = vPort->GetX();
        if (q.v[0].y < vPort->GetY()) q.v[0].y = vPort->GetY() + 3;
        if (q.v[2].x > vPort->GetX() + vPort->GetWidth()) q.v[2].x = vPort->GetX() + vPort->GetWidth();
        if (q.v[2].y > vPort->GetY() + vPort->GetHeight()) q.v[2].y = vPort->GetY() + vPort->GetHeight();
        q.v[1].x = q.v[2].x;
        q.v[1].y = q.v[0].y;
        q.v[3].x = q.v[0].x;
        q.v[3].y = q.v[2].y;
        hge->Gfx_RenderQuad(&q);
        hge->Gfx_RenderLine(q.v[0].x, q.v[0].y, q.v[1].x, q.v[1].y, 0xFFFF0000);
        hge->Gfx_RenderLine(q.v[1].x, q.v[1].y, q.v[2].x, q.v[2].y, 0xFFFF0000);
        hge->Gfx_RenderLine(q.v[2].x, q.v[2].y, q.v[3].x, q.v[3].y, 0xFFFF0000);
        hge->Gfx_RenderLine(q.v[3].x, q.v[3].y, q.v[0].x, q.v[0].y, 0xFFFF0000);
    }
    if (bObjDragSelection && hParser != NULL && iMode == EWW_MODE_OBJECT) {
        hgeQuad q;
        q.blend = BLEND_DEFAULT;
        q.tex = 0;
        q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 0;
        q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = 0x77ffcc00;
        vPort->ClipScreen();
        q.v[0].x = Wrd2ScrX(GetActivePlane(), iObjDragOrigX);
        q.v[0].y = Wrd2ScrY(GetActivePlane(), iObjDragOrigY);
        q.v[1].x = mx;
        q.v[1].y = q.v[0].y;
        q.v[2].x = mx;
        q.v[2].y = my;
        q.v[3].x = q.v[0].x;
        q.v[3].y = my;
        if (abs(q.v[0].x - q.v[2].x) > 2 && abs(q.v[0].y - q.v[2].y) > 2) {
            hge->Gfx_RenderQuad(&q);
            hge->Gfx_RenderLine(q.v[0].x, q.v[0].y, q.v[1].x, q.v[1].y, 0xFFFF0000);
            hge->Gfx_RenderLine(q.v[1].x, q.v[1].y, q.v[2].x, q.v[2].y, 0xFFFF0000);
            hge->Gfx_RenderLine(q.v[2].x, q.v[2].y, q.v[3].x, q.v[3].y, 0xFFFF0000);
            hge->Gfx_RenderLine(q.v[3].x, q.v[3].y, q.v[0].x, q.v[0].y, 0xFFFF0000);
            if (iActiveTool == EWW_TOOL_OBJSELAREA) {
                int wmx = Scr2WrdX(GetActivePlane(), mx),
                        wmy = Scr2WrdY(GetActivePlane(), my);
                int minx = iObjDragOrigX > wmx ? wmx : iObjDragOrigX,
                        miny = iObjDragOrigY > wmy ? wmy : iObjDragOrigY,
                        maxx = iObjDragOrigX > wmx ? iObjDragOrigX : wmx,
                        maxy = iObjDragOrigY > wmy ? iObjDragOrigY : wmy;
                GV->fntMyriad13->printf(q.v[2].x + 25, q.v[2].y + 1, HGETEXT_LEFT, "~l~%s: %d, %s: %d", 0,
                                        GETL(Lang_MinX), minx, GETL(Lang_MinY), miny);
                GV->fntMyriad13->printf(q.v[2].x + 24, q.v[2].y, HGETEXT_LEFT, "~w~%s: ~y~%d~w~, %s: ~y~%d~l~", 0,
                                        GETL(Lang_MinX), minx, GETL(Lang_MinY), miny);

                GV->fntMyriad13->printf(q.v[2].x + 25, q.v[2].y + 21, HGETEXT_LEFT, "~l~%s: %d, %s: %d", 0,
                                        GETL(Lang_MaxX), maxx, GETL(Lang_MaxY), maxy);
                GV->fntMyriad13->printf(q.v[2].x + 24, q.v[2].y + 20, HGETEXT_LEFT, "~w~%s: ~y~%d~w~, %s: ~y~%d~l~", 0,
                                        GETL(Lang_MaxX), maxx, GETL(Lang_MaxY), maxy);
            } else {
                GV->fntMyriad13->printf(q.v[2].x + 25, q.v[2].y + 1, HGETEXT_LEFT, "~l~%s: %d", 0,
                                        GETL(Lang_SelectedObjects), vObjectsHL.size());
                GV->fntMyriad13->printf(q.v[2].x + 24, q.v[2].y, HGETEXT_LEFT, "~w~%s: ~y~%d~l~", 0,
                                        GETL(Lang_SelectedObjects), vObjectsHL.size());
            }
        }
        hge->Gfx_SetClipping();
    }
    if ((iMode == EWW_MODE_OBJECT && iActiveTool == EWW_TOOL_MOVEOBJECT ||
         iMode == EWW_MODE_OBJECT && iActiveTool == EWW_TOOL_EDITOBJ && hEditObj->IsMovingObject())
        && conMain->getWidgetAt(mx, my) == vPort->GetWidget()) {
        int diffx, diffy;
        if (iActiveTool == EWW_TOOL_MOVEOBJECT) {
            diffx = GetUserDataFromObj(vObjectsPicked[0])->GetX() - vObjectsPicked[0]->GetParam(WWD::Param_LocationX);
            //Scr2WrdX(GetActivePlane(), mx)-iMoveRelX;
            diffy = GetUserDataFromObj(vObjectsPicked[0])->GetY() - vObjectsPicked[0]->GetParam(WWD::Param_LocationY);
            //Scr2WrdY(GetActivePlane(), my)-iMoveRelY;
        } else if (iActiveTool == EWW_TOOL_EDITOBJ) {
            diffx = hEditObj->GetTempObj()->GetParam(WWD::Param_LocationX) - hEditObj->_iMoveInitX;
            diffy = hEditObj->GetTempObj()->GetParam(WWD::Param_LocationY) - hEditObj->_iMoveInitY;
        }
        GV->fntMyriad13->printf(mx + 26, my + 1, HGETEXT_LEFT, "~l~X: %+d", 0, diffx);
        GV->fntMyriad13->printf(mx + 26, my + 21, HGETEXT_LEFT, "~l~Y: %+d", 0, diffy);
        GV->fntMyriad13->printf(mx + 25, my, HGETEXT_LEFT, "~w~X: ~y~%+d", 0, diffx);
        GV->fntMyriad13->printf(mx + 25, my + 20, HGETEXT_LEFT, "~w~Y: ~y~%+d", 0, diffy);
    }
}

bool State::EditingWW::ObjectThink(bool pbConsumed) {
    if (bOpenObjContext)
        if (fObjContextX != fCamX || fObjContextY != fCamY) {
            bOpenObjContext = 0;
        }

    if (iActiveTool == EWW_TOOL_EDITOBJ && hEditObj->Kill()) {
        /*if( bEditObjDelete && hEditObj->ObjectSaved() ){
         SetTool(EWW_TOOL_MOVEOBJECTBATCH);
         iMoveRelX = vObjectsPicked[0]->GetParam(WWD::Param_LocationX);
         iMoveRelY = vObjectsPicked[0]->GetParam(WWD::Param_LocationY);
         hge->Input_SetMousePos(Wrd2ScrX(GetActivePlane(), iMoveRelX),
                                Wrd2ScrY(GetActivePlane(), iMoveRelY));
        }else*/
        bool bAddNext = hEditObj->IsAddingNext();
        int objmoverelx, objmoverely;
        void *specialptr = NULL;
        if (bAddNext) {
            objmoverelx = vObjectsPicked[0]->GetParam(WWD::Param_LocationX);
            objmoverely = vObjectsPicked[0]->GetParam(WWD::Param_LocationY);
            specialptr = hEditObj->GenerateNextObjectData();
        }
        std::vector<WWD::Object *> vObjToPick;
        if (hEditObj->iType == ObjEdit::enText && ((ObjEdit::cEditObjText *) hEditObj)->ObjectSaved()) {
            vObjToPick = ((ObjEdit::cEditObjText *) hEditObj)->GetObjects();
        }
        SetTool(EWW_TOOL_NONE);
        if (vObjToPick.size() > 0)
            vObjectsPicked = vObjToPick;
        bEditObjDelete = 0;
        if (bAddNext) {
            WWD::Object *nobj = new WWD::Object(vObjectsPicked[0]);
            nobj->SetParam(WWD::Param_LocationX, Scr2WrdX(GetActivePlane(), vPort->GetX() + vPort->GetWidth() / 2));
            nobj->SetParam(WWD::Param_LocationY, Scr2WrdY(GetActivePlane(), vPort->GetY() + vPort->GetHeight() / 2));
            GetActivePlane()->AddObjectAndCalcID(nobj);
            nobj->SetUserData(new cObjUserData(nobj));
            hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(nobj);
            vObjectsPicked.clear();
            vObjectsPicked.push_back(nobj);
            bEditObjDelete = 1;
            objContext->EmulateClickID(OBJMENU_EDIT);
            hEditObj->_iMoveInitX = objmoverelx;
            hEditObj->_iMoveInitY = objmoverely;
            hEditObj->ApplyDataFromPrevObject(specialptr);
        }
        vPort->MarkToRedraw(1);
    }

    if (iActiveTool == EWW_TOOL_EDITOBJ)
        hEditObj->Think(pbConsumed);

    if (pbConsumed) {
        if (objContext->isVisible()) {
            if ((fObjContextX - fCamX) != 0 || (fObjContextY - fCamY) != 0) {
                objContext->setX(objContext->getX() + (fObjContextX - fCamX));
                objContext->setY(objContext->getY() + (fObjContextY - fCamY));
                fObjContextX = fCamX;
                fObjContextY = fCamY;
            }
        }
        return 1;
    }

    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    if (iActiveTool == EWW_TOOL_ALIGNOBJ) {
        if (vPort->GetWidget()->isMouseOver() && !pbConsumed && hge->Input_KeyDown(HGEK_LBUTTON)) {
            int x = Scr2WrdX(GetActivePlane(), mx), y = Scr2WrdY(GetActivePlane(), my);
            std::vector<WWD::Object *> picked = hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->GetObjectsByArea(
                    x, y, 1, 1);
            if (picked.size() != 0) {
                WWD::Object *aligntoobj = NULL;
                for (int i = 0; i < vObjectsPicked.size(); i++)
                    if (vObjectsPicked[i] == picked[0]) {
                        aligntoobj = picked[0];
                        break;
                    }
                if (aligntoobj != 0) {
                    for (int i = 0; i < vObjectsPicked.size(); i++) {
                        if (bObjectAlignAxis)
                            vObjectsPicked[i]->SetParam(WWD::Param_LocationY,
                                                        aligntoobj->GetParam(WWD::Param_LocationY));
                        else
                            vObjectsPicked[i]->SetParam(WWD::Param_LocationX,
                                                        aligntoobj->GetParam(WWD::Param_LocationX));
                        GetUserDataFromObj(vObjectsPicked[i])->SyncToObj();
                    }
                    SetTool(EWW_TOOL_NONE);
                    pbConsumed = 1;
                    vPort->MarkToRedraw(1);
                }
            }
        }
    } else if (iActiveTool == EWW_TOOL_NONE) {

        /*if( fObjPickLastMx != mx || fObjPickLastMy != my ){
         bObjPickMoved = 1;
        }*/
        if (vPort->GetWidget()->isMouseOver() && !pbConsumed && hge->Input_KeyDown(HGEK_LBUTTON) &&
            !bObjDragSelection) {
            bObjDragSelection = 1;
            iObjDragOrigX = Scr2WrdX(GetActivePlane(), mx);
            iObjDragOrigY = Scr2WrdY(GetActivePlane(), my);
        } else if (hge->Input_GetKeyState(HGEK_LBUTTON) && bObjDragSelection && vPort->GetWidget()->isMouseOver() &&
                   !pbConsumed) {
            int actx = Scr2WrdX(GetActivePlane(), mx),
                    acty = Scr2WrdY(GetActivePlane(), my);
            int x = std::min(iObjDragOrigX, actx),
                    y = std::min(iObjDragOrigY, acty),
                    w = std::max(iObjDragOrigX, actx) - x,
                    h = std::max(iObjDragOrigY, acty) - y;
            vObjectsHL = hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->GetObjectsByArea(x, y, w, h);
            vObjectsForbidHL.clear();
            if (hge->Input_GetKeyState(HGEK_CTRL)) {
                for (int i = 0; i < vObjectsPicked.size(); i++)
                    for (int y = 0; y < vObjectsHL.size(); y++)
                        if (vObjectsPicked[i] == vObjectsHL[y]) {
                            vObjectsForbidHL.push_back(vObjectsHL[y]);
                            vObjectsHL.erase(vObjectsHL.begin() + y);
                        }
            } else if (hge->Input_GetKeyState(HGEK_SHIFT)) {
                for (int y = 0; y < vObjectsHL.size(); y++) {
                    bool bfound = 0;
                    for (int i = 0; i < vObjectsPicked.size(); i++)
                        if (vObjectsPicked[i] == vObjectsHL[y]) {
                            bfound = 1;
                            break;
                        }
                    if (bfound)
                        vObjectsHL.erase(vObjectsHL.begin() + y);
                }
            } else if (hge->Input_GetKeyState(HGEK_ALT)) {
                for (int y = 0; y < vObjectsHL.size(); y++) {
                    for (int i = 0; i < vObjectsPicked.size(); i++)
                        if (vObjectsPicked[i] == vObjectsHL[y]) {
                            vObjectsForbidHL.push_back(vObjectsHL[y]);
                            vObjectsHL.erase(vObjectsHL.begin() + y);
                            y--;
                        }
                }
                vObjectsHL.clear();
            }
        } else if (bObjDragSelection) {
            bObjDragSelection = 0;
            int actx = Scr2WrdX(GetActivePlane(), mx),
                    acty = Scr2WrdY(GetActivePlane(), my);
            if (actx == iObjDragOrigX && acty == iObjDragOrigY && vObjectsHL.size() > 1) {
                int selid = -1;
                for (int i = 0; i < vObjectsPicked.size(); i++)
                    for (int y = 0; y < vObjectsHL.size(); y++)
                        if (vObjectsPicked[i] == vObjectsHL[y])
                            selid = y;
                if (!hge->Input_GetKeyState(HGEK_CTRL) && !hge->Input_GetKeyState(HGEK_SHIFT) &&
                    !hge->Input_GetKeyState(HGEK_ALT))
                    vObjectsPicked.clear();
                if (selid == -1) {
                    int maxz = -100000, selit = -1;
                    for (int i = 0; i < vObjectsHL.size(); i++)
                        if (vObjectsHL[i]->GetParam(WWD::Param_LocationZ) > maxz) {
                            selit = i;
                            maxz = vObjectsHL[i]->GetParam(WWD::Param_LocationZ);
                        }
                    vObjectsPicked.push_back(vObjectsHL[selit]);
                } else
                    vObjectsPicked.push_back(vObjectsHL[(selid + 1) % vObjectsHL.size()]);
            } else {
                if (!hge->Input_GetKeyState(HGEK_CTRL) && !hge->Input_GetKeyState(HGEK_SHIFT) &&
                    !hge->Input_GetKeyState(HGEK_ALT))
                    vObjectsPicked = vObjectsHL;
                else {
                    for (int i = 0; i < vObjectsHL.size(); i++) {
                        bool found = 0;
                        for (int j = 0; j < vObjectsPicked.size(); j++) {
                            if (vObjectsPicked[j] == vObjectsHL[i])
                                found = 1;
                        }
                        if (!found)
                            vObjectsPicked.push_back(vObjectsHL[i]);
                    }
                    for (int i = 0; i < vObjectsForbidHL.size(); i++)
                        for (int j = 0; j < vObjectsPicked.size(); j++) {
                            if (vObjectsPicked[j] == vObjectsForbidHL[i]) {
                                vObjectsPicked.erase(vObjectsPicked.begin() + j);
                                j--;
                            }
                        }
                }
            }
            vObjectsHL.clear();
            vObjectsForbidHL.clear();
        }

        if (hge->Input_KeyUp(HGEK_LBUTTON)) {
            DWORD time = GetTickCount();
            if (vObjectsPicked.size() == 1) {
                int mwx = Scr2WrdX(GetActivePlane(), mx), mwy = Scr2WrdY(GetActivePlane(), my);
                std::vector<WWD::Object *> mouseobj = hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->GetObjectsByArea(
                        mwx, mwy, mwx + 1, mwy + 1);
                bool found = 0;
                for (int i = 0; i < mouseobj.size(); i++)
                    if (mouseobj[i] == vObjectsPicked[0]) {
                        found = 1;
                        break;
                    }
                if (found) {
                    cObjUserData *ud = GetUserDataFromObj(vObjectsPicked[0]);
                    int objcnt = 0;
                    for (int i = 0; i < ud->GetReferencedCellsCount(); i++)
                        objcnt += ud->GetReferencedCell(i)->GetObjectsCount();
                    if (fDoubleClickTimer >= 0 && fDoubleClickTimer < 0.5 && iDoubleClickX == mx &&
                        iDoubleClickY == my) {
                        if (IsEditableObject(vObjectsPicked[0], NULL))
                            OpenObjectEdit(vObjectsPicked[0]);
                        else
                            OpenObjectWindow(vObjectsPicked[0]);
                        fDoubleClickTimer = -1;
                    } else {
                        fDoubleClickTimer = 0;
                        iDoubleClickX = mx;
                        iDoubleClickY = my;
                    }
                }
            } else
                fDoubleClickTimer = -1;
        }

        if (hge->Input_KeyDown(HGEK_LBUTTON) || hge->Input_KeyDown(HGEK_RBUTTON) || hge->Input_KeyUp(HGEK_RBUTTON)) {
            if (objContext->isVisible()) {
                if (!(mx > objContext->getX() &&
                      mx < objContext->getX() + objContext->getWidth() &&
                      my > objContext->getY() &&
                      my < objContext->getY() + objContext->getHeight())) {
                    objContext->setVisible(0);
                    return 0;
                }
            }

            if ((hge->Input_KeyDown(HGEK_RBUTTON) || hge->Input_KeyUp(HGEK_RBUTTON)) && !bObjDragSelection) {
                std::vector<WWD::Object *> selobjs = hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->GetObjectsByArea(
                        Scr2WrdX(GetActivePlane(), mx), Scr2WrdY(GetActivePlane(), my), 1, 1);
                sort(selobjs.begin(), selobjs.end(), ObjSortCoordZ);
                if (selobjs.size() != NULL) {
                    WWD::Object *selobj = 0;
                    bool ok = 0;
                    for (int y = 0; y < selobjs.size(); y++) {
                        for (int i = 0; i < vObjectsPicked.size(); i++)
                            if (selobjs[y] == vObjectsPicked[i]) {
                                ok = 1;
                                selobj = selobjs[y];
                                break;
                            }
                        if (ok) break;
                    }
                    if (ok && conMain->getWidgetAt(mx, my) == vPort->GetWidget()) {
                        if (hge->Input_KeyDown(HGEK_RBUTTON)) {
                            fObjPropMouseX = mx;
                            fObjPropMouseY = my;
                        } else if (hge->Input_KeyUp(HGEK_RBUTTON) && mx == fObjPropMouseX && my == fObjPropMouseY) {
                            bOpenObjContext = 1;
                            fObjContextX = fCamX;
                            fObjContextY = fCamY;
                            if (vObjectsPicked.size() == 1) {
                                if (vObjectsPicked[0] == hStartingPosObj)
                                    objContext->SetModel(conmodSpawnPoint);
                                else {
                                    SHR::Context *con = (SHR::Context *) 1;
                                    bool spec = AreObjectSpecificOptionsAvailable(vObjectsPicked[0], &con);
                                    if (IsEditableObject(vObjectsPicked[0]))
                                        objContext->SetModel(conmodEditableObject);
                                    else
                                        objContext->SetModel(conmodObject);
                                    if (!strcmp(vObjectsPicked[0]->GetLogic(), "CustomLogic") &&
                                        hCustomLogics->GetLogicByName(vObjectsPicked[0]->GetName()) != 0) {
                                        objContext->AddElement(OBJMENU_EDITLOGIC, GETL2S("Various", "ContextEditLogic"),
                                                               GV->sprIcons16[Icon16_Code],
                                                               objContext->GetElementByID(OBJMENU_PROPERTIES));
                                    }
                                    if (spec) {
                                        objContext->AddElement(OBJMENU_SPECIFICPROP, GETL2S("Various", "ContextAdv"),
                                                               GV->sprIcons16[Icon16_PropertyTree],
                                                               objContext->GetElementByID(OBJMENU_PROPERTIES));
                                        objContext->GetElementByID(OBJMENU_SPECIFICPROP)->SetCascade(con);
                                    }
                                }
                            } else
                                objContext->SetModel(conmodObjectMultiple);

                            for (int flag = 0; flag < 14; flag++) {
                                SHR::Context *hmyContext = objFlagDrawContext;
                                int flagtype = 0, flagpos = flag, menupos = OBJMENU_FLAGS_DRAW + flag + 1;
                                if (flag > 3 && flag < 8) {
                                    flagtype = 1;
                                    flagpos -= 4;
                                    menupos = OBJMENU_FLAGS_DYNAMIC + flagpos + 1;
                                    hmyContext = objFlagDynamicContext;
                                } else if (flag > 7) {
                                    flagtype = 2;
                                    flagpos -= 8;
                                    menupos = OBJMENU_FLAGS_ADDITIONAL + flagpos + 1;
                                    hmyContext = objFlagAddContext;
                                }
                                int binaryval = pow(2, flagpos);
                                bool valset = 0;
                                unsigned char fval = 0;

                                for (size_t obji = 0; obji < vObjectsPicked.size(); obji++) {
                                    if (vObjectsPicked[obji] == hStartingPosObj) continue;
                                    int aqflag;
                                    if (flagtype == 0) aqflag = int(vObjectsPicked[obji]->GetDrawFlags());
                                    else if (flagtype == 1) aqflag = int(vObjectsPicked[obji]->GetDynamicFlags());
                                    else if (flagtype == 2) aqflag = int(vObjectsPicked[obji]->GetAddFlags());
                                    bool flagval = (aqflag & binaryval);
                                    if (!valset) {
                                        fval = flagval;
                                        valset = 1;
                                    } else {
                                        if (flagval != fval)
                                            fval = 2;
                                    }
                                }

                                hgeSprite *ico = 0;
                                if (fval == 1) ico = GV->sprIcons16[Icon16_Applied];
                                else if (fval == 2) ico = GV->sprIcons16[Icon16_AppliedPartially];

                                hmyContext->GetElementByID(menupos)->SetIcon(ico, 0);
                            }

                            objContext->adjustSize();
                            objContext->setPosition(mx, my);
                            if (mx + objContext->getWidth() > hge->System_GetState(HGE_SCREENWIDTH))
                                objContext->setX(mx - objContext->getWidth());
                            if (my + objContext->getHeight() > hge->System_GetState(HGE_SCREENHEIGHT))
                                objContext->setY(my - objContext->getHeight());
                            objContext->setVisible(1);
                        }
                    } else {
                        objContext->setVisible(0);
                    }
                } else {
                    //vObjectsPicked.clear();
                    objContext->setVisible(0);
                }

                if (!objContext->isVisible() && !bObjDragSelection) {
                    if (bOpenObjContext && hge->Input_KeyUp(HGEK_RBUTTON) &&
                        conMain->getWidgetAt(mx, my) == vPort->GetWidget()) {
                        objContext->setVisible(1);
                        bOpenObjContext = 0;
                    } else if (hge->Input_KeyDown(HGEK_RBUTTON)) {
                        bOpenObjContext = 1;
                        fObjContextX = fCamX;
                        fObjContextY = fCamY;
                    }

                    objContext->setPosition(mx, my);
                    fObjContextX = fCamX;
                    fObjContextY = fCamY;
                    if (vObjectClipboard.size() != 0) {
                        char ncap[256];
                        if (vObjectClipboard.size() == 1)
                            sprintf(ncap, "%s: ~y~%s~l~", GETL(Lang_Paste), vObjectClipboard[0]->GetLogic());
                        else
                            sprintf(ncap, "%s: ~y~%s~l~", GETL(Lang_Paste), GETL(Lang_ManyObjects));
                        conmodPaste->GetElementByID(OBJMENU_PASTE)->SetCaption(ncap);

                        conmodPaste->GetElementByID(OBJMENU_TESTFROMHERE)->SetEnabled(hNativeController->IsValid() &&
                                                                                      hNativeController->IsCrazyHookAvailable() &&
                                                                                      strlen(hParser->GetFilePath()) >
                                                                                      0);

                        objContext->SetModel(conmodPaste);
                    } else {
                        conmodAtEmpty->GetElementByID(OBJMENU_TESTFROMHERE)->SetEnabled(hNativeController->IsValid() &&
                                                                                        hNativeController->IsCrazyHookAvailable() &&
                                                                                        strlen(hParser->GetFilePath()) >
                                                                                        0);
                        objContext->SetModel(conmodAtEmpty);
                    }
                    objContext->adjustSize();
                }

            }
        }

        if (objContext->isVisible()) {
            if ((fObjContextX - fCamX) != 0 || (fObjContextY - fCamY) != 0) {
                objContext->setX(objContext->getX() + (fObjContextX - fCamX));
                objContext->setY(objContext->getY() + (fObjContextY - fCamY));
                fObjContextX = fCamX;
                fObjContextY = fCamY;
            }
        }

        fObjPickLastMx = mx;
        fObjPickLastMy = my;
    } else if (iActiveTool == EWW_TOOL_OBJSELAREA) {
        if (toolsaAction == TOOL_OBJSA_PICKALL) {
            if (vPort->GetWidget()->isMouseOver() && !pbConsumed && hge->Input_KeyDown(HGEK_LBUTTON) &&
                !bObjDragSelection) {
                bObjDragSelection = 1;
                iObjDragOrigX = Scr2WrdX(GetActivePlane(), mx);
                iObjDragOrigY = Scr2WrdY(GetActivePlane(), my);
            } else if (bObjDragSelection && !hge->Input_GetKeyState(HGEK_LBUTTON)) {
                int wmx = Scr2WrdX(GetActivePlane(), mx), wmy = Scr2WrdY(GetActivePlane(), my);
                toolsaMinX = iObjDragOrigX > wmx ? wmx : iObjDragOrigX;
                toolsaMinY = iObjDragOrigY > wmy ? wmy : iObjDragOrigY;
                toolsaMaxX = iObjDragOrigX < wmx ? wmx : iObjDragOrigX;
                toolsaMaxY = iObjDragOrigY < wmy ? wmy : iObjDragOrigY;
                bObjDragSelection = 0;
                char label[200];
                sprintf(label, "X1: ~y~%d~l~, Y1: ~y~%d~l~, X2: ~y~%d~l~, Y2: ~y~%d~l~",
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
                sprintf(label, "X1: ~y~%d~l~, Y1: ~y~%d~l~, X2: ~y~%d~l~, Y2: ~y~%d~l~",
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
                sprintf(label, "X1: ~y~%d~l~, Y1: ~y~%d~l~, X2: ~y~%d~l~, Y2: ~y~%d~l~",
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
                sprintf(label, "X1: ~y~%d~l~, Y1: ~y~%d~l~, X2: ~y~%d~l~, Y2: ~y~%d~l~",
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
                sprintf(label, "X1: ~y~%d~l~, Y1: ~y~%d~l~, X2: ~y~%d~l~, Y2: ~y~%d~l~",
                        toolsaMinX, toolsaMinY, toolsaMaxX, toolsaMaxY);
                labtoolSelAreaValues->setCaption(label);
                labtoolSelAreaValues->adjustSize();
                toolsaAction = TOOL_OBJSA_NONE;
            }
        }
    } else if (iActiveTool == EWW_TOOL_MOVEOBJECT) {
        //If object was moved.
        if ((fObjPickLastMx != Scr2WrdX(GetActivePlane(), mx) || fObjPickLastMy != Scr2WrdY(GetActivePlane(), my)) &&
            !hge->Input_GetKeyState(HGEK_RBUTTON)) {
            //Move vector.
            int wmx = Scr2WrdX(GetActivePlane(), mx),
                    wmy = Scr2WrdY(GetActivePlane(), my);
            int diffx = wmx - iMoveRelX, diffy = wmy - iMoveRelY;
            //For every selected object
            for (int i = 0; i < vObjectsPicked.size(); i++) {
                //Get base (initial) object coords
                int basex = vObjectsPicked[i]->GetParam(WWD::Param_LocationX), basey = vObjectsPicked[i]->GetParam(
                        WWD::Param_LocationY);
                //Align on X and Y axis when SHIFT is hold down.
                if (hge->Input_GetKeyState(HGEK_SHIFT)) {
                    float ratio;
                    if (diffy != 0)
                        ratio = float(diffx) / float(diffy);
                    else
                        ratio = 2;
                    if (ratio >= -0.50f && ratio <= 0.50)
                        GetUserDataFromObj(vObjectsPicked[i])->SetPos(basex, basey + diffy);
                    else if (ratio > 1.5f || ratio < -1.5f)
                        GetUserDataFromObj(vObjectsPicked[i])->SetPos(basex + diffx, basey);
                    else {
                        int diff = std::min(abs(diffx), abs(diffy));
                        if (diffy < 0) {
                            if (ratio < -0.5f && ratio > -1.5f) //upright
                                GetUserDataFromObj(vObjectsPicked[i])->SetPos(basex + diff, basey - diff);
                            else if (ratio > 0.5f && ratio < 1.5f) //upleft
                                GetUserDataFromObj(vObjectsPicked[i])->SetPos(basex - diff, basey - diff);
                        } else {
                            if (ratio < -0.5f && ratio > -1.5f) //downleft
                                GetUserDataFromObj(vObjectsPicked[i])->SetPos(basex - diff, basey + diff);
                            else if (ratio > 0.5f && ratio < 1.5f) //downright
                                GetUserDataFromObj(vObjectsPicked[i])->SetPos(basex + diff, basey + diff);
                        }
                    }
                    //Align to grid/tile center when CONTROL is hold down.
                } else if (hge->Input_GetKeyState(HGEK_CTRL)) {
                    int diffmodulox = (basex + diffx) % 64;
                    int diffmoduloy = (basey + diffy) % 64;
                    int modx = 0, mody = 0;

                    if (diffmodulox > 16 && diffmodulox < 48) modx += 32;
                    else if (diffmodulox >= 48) modx += 64;

                    if (diffmoduloy > 16 && diffmoduloy < 48) mody += 32;
                    else if (diffmoduloy >= 48) mody += 64;

                    diffx -= diffmodulox - modx;
                    diffy -= diffmoduloy - mody;
                    //Apply
                    GetUserDataFromObj(vObjectsPicked[i])->SetPos(basex + diffx, basey + diffy);
                    //No align.
                } else {
                    //Stick to guides
                    if (MDI->GetActiveDoc()->vGuides.size() > 0 && vObjectsPicked.size() == 1) {
                        bool xalign = 0, yalign = 0;
                        for (size_t i = 0; i < GV->editState->MDI->GetActiveDoc()->vGuides.size(); i++) {
                            stGuideLine gl = GV->editState->MDI->GetActiveDoc()->vGuides[i];
                            if (gl.iPos < 0) continue;
                            if (gl.bOrient == GUIDE_HORIZONTAL && !yalign && abs(wmy - gl.iPos) < 10 ||
                                gl.bOrient == GUIDE_VERTICAL && !xalign && abs(wmx - gl.iPos) < 10) {
                                if (gl.bOrient == GUIDE_HORIZONTAL) {
                                    diffy = wmy - iMoveRelY + gl.iPos - wmy;
                                    yalign = 1;
                                } else {
                                    diffx = wmx - iMoveRelX + gl.iPos - wmx;
                                    xalign = 1;
                                }
                            }
                        }
                    }
                    GetUserDataFromObj(vObjectsPicked[i])->SetPos(basex + diffx, basey + diffy);
                }
            }
            fObjPickLastMx = Scr2WrdX(GetActivePlane(), mx);
            fObjPickLastMy = Scr2WrdY(GetActivePlane(), my);
            vPort->MarkToRedraw(1);
        }
        if (hge->Input_KeyDown(HGEK_LBUTTON)) {
            bool bRecalc = 0;
            for (int i = 0; i < vObjectsPicked.size(); i++) {
                if (vObjectsPicked[i]->GetParam(WWD::Param_MinX) != 0 ||
                    vObjectsPicked[i]->GetParam(WWD::Param_MinY) != 0 ||
                    vObjectsPicked[i]->GetParam(WWD::Param_MaxX) != 0 ||
                    vObjectsPicked[i]->GetParam(WWD::Param_MaxY) != 0 ||
                    (!strcmp(vObjectsPicked[i]->GetLogic(), "Shake") &&
                     (vObjectsPicked[i]->GetAttackRect().x1 != 0 ||
                      vObjectsPicked[i]->GetAttackRect().y1 != 0 ||
                      vObjectsPicked[i]->GetAttackRect().x2 != 0 ||
                      vObjectsPicked[i]->GetAttackRect().y2 != 0))) {
                    if (MessageBox(hge->System_GetState(HGE_HWND), GETL2S("Various", "MsgRecalcMinMaxXY"),
                                   GETL(Lang_Message), MB_YESNO | MB_ICONINFORMATION) == IDYES) {
                        bRecalc = 1;
                    }
                    break;
                }
            }
            for (int i = 0; i < vObjectsPicked.size(); i++) {
                if (bRecalc) {
                    int diffx = GetUserDataFromObj(vObjectsPicked[i])->GetX() -
                                vObjectsPicked[i]->GetParam(WWD::Param_LocationX),
                            diffy = GetUserDataFromObj(vObjectsPicked[i])->GetY() -
                                    vObjectsPicked[i]->GetParam(WWD::Param_LocationY);
                    if (!strcmp(vObjectsPicked[i]->GetLogic(), "Shake")) {
                        WWD::Rect atrect = vObjectsPicked[i]->GetAttackRect();
                        if (atrect.x1 != 0) atrect.x1 += diffx;
                        if (atrect.y1 != 0) atrect.y1 += diffy;
                        if (atrect.x2 != 0) atrect.x2 += diffx;
                        if (atrect.y2 != 0) atrect.y2 += diffy;
                        vObjectsPicked[i]->SetAttackRect(atrect);
                    } else {
                        if (vObjectsPicked[i]->GetParam(WWD::Param_MinX) != 0)
                            vObjectsPicked[i]->SetParam(WWD::Param_MinX,
                                                        vObjectsPicked[i]->GetParam(WWD::Param_MinX) + diffx);
                        if (vObjectsPicked[i]->GetParam(WWD::Param_MinY) != 0)
                            vObjectsPicked[i]->SetParam(WWD::Param_MinY,
                                                        vObjectsPicked[i]->GetParam(WWD::Param_MinY) + diffy);
                        if (vObjectsPicked[i]->GetParam(WWD::Param_MaxX) != 0)
                            vObjectsPicked[i]->SetParam(WWD::Param_MaxX,
                                                        vObjectsPicked[i]->GetParam(WWD::Param_MaxX) + diffx);
                        if (vObjectsPicked[i]->GetParam(WWD::Param_MaxY) != 0)
                            vObjectsPicked[i]->SetParam(WWD::Param_MaxY,
                                                        vObjectsPicked[i]->GetParam(WWD::Param_MaxY) + diffy);
                    }
                }
                vObjectsPicked[i]->SetParam(WWD::Param_LocationX, GetUserDataFromObj(vObjectsPicked[i])->GetX());
                vObjectsPicked[i]->SetParam(WWD::Param_LocationY, GetUserDataFromObj(vObjectsPicked[i])->GetY());
                hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(vObjectsPicked[i]);
                if (vObjectsPicked[i] == hStartingPosObj) {
                    hParser->SetStartX(GetUserDataFromObj(vObjectsPicked[i])->GetX());
                    hParser->SetStartY(GetUserDataFromObj(vObjectsPicked[i])->GetY());
                }
            }
            SetTool(EWW_TOOL_NONE);
            MarkUnsaved();
        } else if (hge->Input_KeyDown(HGEK_ESCAPE)) {
            if (bEditObjDelete)
                objContext->EmulateClickID(OBJMENU_DELETE);
            else
                for (int i = 0; i < vObjectsPicked.size(); i++)
                    GetUserDataFromObj(vObjectsPicked[i])->SyncToObj();
            SetTool(EWW_TOOL_NONE);
            vPort->MarkToRedraw(1);
            bEditObjDelete = 0;
        }
    } else if (iActiveTool == EWW_TOOL_BRUSHOBJECT) {
        if (vObjectsBrushCB.size() == 0) {
            SetTool(EWW_TOOL_NONE);
        } else {
            if (bObjBrushDrawing) {
                if (!hge->Input_GetKeyState(HGEK_LBUTTON)) {
                    bObjBrushDrawing = 0;
                } else {
                    float distance = DISTANCE(iobjbrLastDrawnX,
                                              iobjbrLastDrawnY,
                                              Scr2WrdX(GetActivePlane(), mx),
                                              Scr2WrdY(GetActivePlane(), my));
                    float percdist = distance / sliobrDistance->getValue();
                    if (percdist > 1) percdist = 1;
                    dwCursorColor = SETR(SETB(0xFF00FF00, int((1 - percdist) * 255)), int((1 - percdist) * 255));
                    if (distance > sliobrDistance->getValue()) {
                        iobjbrLastDrawnX = Scr2WrdX(GetActivePlane(), mx);
                        iobjbrLastDrawnY = Scr2WrdY(GetActivePlane(), my);
                        int scatterx = atoi(tfobrDispX->getText().c_str()),
                                scattery = atoi(tfobrDispY->getText().c_str());
                        int scatvx = hge->Random_Int(-scatterx, scatterx),
                                scatvy = hge->Random_Int(-scattery, scattery);
                        for (int i = 0; i < vObjectsBrushCB.size(); i++) {
                            if (i > 0 && cbobrApplyScatterSeparately->isSelected()) {
                                scatvx = hge->Random_Int(-scatterx, scatterx),
                                        scatvy = hge->Random_Int(-scattery, scattery);
                            }
                            WWD::Object *obj = new WWD::Object(vObjectsBrushCB[i]);
                            int diffx = vObjectsBrushCB[i]->GetParam(WWD::Param_LocationX) -
                                        vObjectsBrushCB[0]->GetParam(WWD::Param_LocationX),
                                    diffy = vObjectsBrushCB[i]->GetParam(WWD::Param_LocationY) -
                                            vObjectsBrushCB[0]->GetParam(WWD::Param_LocationY);
                            obj->SetParam(WWD::Param_LocationX, Scr2WrdX(GetActivePlane(), mx) + diffx + scatvx);
                            obj->SetParam(WWD::Param_LocationY, Scr2WrdY(GetActivePlane(), my) + diffy + scatvy);
                            GetActivePlane()->AddObjectAndCalcID(obj);
                            obj->SetUserData(new cObjUserData(obj));
                            hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(obj);
                        }
                        MarkUnsaved();
                        vPort->MarkToRedraw(1);
                    }
                }
            }
            if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                iobjbrLastDrawnX = Scr2WrdX(GetActivePlane(), mx);
                iobjbrLastDrawnY = Scr2WrdY(GetActivePlane(), my);
                int scatterx = atoi(tfobrDispX->getText().c_str()),
                        scattery = atoi(tfobrDispY->getText().c_str());
                int scatvx = hge->Random_Int(-scatterx, scatterx),
                        scatvy = hge->Random_Int(-scattery, scattery);
                for (int i = 0; i < vObjectsBrushCB.size(); i++) {
                    if (i > 0 && cbobrApplyScatterSeparately->isSelected()) {
                        scatvx = hge->Random_Int(-scatterx, scatterx),
                                scatvy = hge->Random_Int(-scattery, scattery);
                    }
                    WWD::Object *obj = new WWD::Object(vObjectsBrushCB[i]);
                    int diffx = vObjectsBrushCB[i]->GetParam(WWD::Param_LocationX) -
                                vObjectsBrushCB[0]->GetParam(WWD::Param_LocationX),
                            diffy = vObjectsBrushCB[i]->GetParam(WWD::Param_LocationY) -
                                    vObjectsBrushCB[0]->GetParam(WWD::Param_LocationY);
                    obj->SetParam(WWD::Param_LocationX, Scr2WrdX(GetActivePlane(), mx) + diffx + scatvx);
                    obj->SetParam(WWD::Param_LocationY, Scr2WrdY(GetActivePlane(), my) + diffy + scatvy);
                    GetActivePlane()->AddObjectAndCalcID(obj);
                    obj->SetUserData(new cObjUserData(obj));
                    hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(obj);
                }
                MarkUnsaved();
                bObjBrushDrawing = 1;
            }
            vPort->MarkToRedraw(1);
        }

        if (objContext->isVisible()) {
            if ((fObjContextX - fCamX) != 0 || (fObjContextY - fCamY) != 0) {
                objContext->setX(objContext->getX() + (fObjContextX - fCamX));
                objContext->setY(objContext->getY() + (fObjContextY - fCamY));
                fObjContextX = fCamX;
                fObjContextY = fCamY;
            }
        }

        fObjPickLastMx = mx;
        fObjPickLastMy = my;
        //bObjectMenuRollout
    }
    return 0;
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
    //objdSource = GetActivePlane()->GetObjectByIterator(iObjectPicked);
    char tmp[128];
    /*if( objdSource->GetName()[0] != '\0' )
     sprintf(tmp, "%s: ~y~%d~l~ (~y~%s~l~)", GETL(Lang_SourceObjectID), objdSource->GetParam(WWD::Param_ID), objdSource->GetName());
    else*/
    if (vObjectsPicked.size() == 1) {
        objdSource = vObjectsPicked[0];
        sprintf(tmp, "%s: ~y~%d~l~", GETL(Lang_SourceObjectID), objdSource->GetParam(WWD::Param_ID));
    } else {
        objdSource = NULL;
        sprintf(tmp, "%s: ~y~%s~l~", GETL(Lang_SourceObjectID), GETL(Lang_ManyObjects));
    }
    labdCopiedObjectID->setCaption(tmp);
    labdCopiedObjectID->adjustSize();
    tfdTimes->requestFocus();
}

int State::EditingWW::GetObjectType(WWD::Object *obj) {
    char *loweredlogic = SHR::ToLower(obj->GetLogic());
    //char * loweredgfx = SHR::ToLower(obj->GetImageSet());
    int ret = OBJ_UNKNOWN;
    if (strstr(loweredlogic, "sound") != NULL)
        ret = OBJ_SOUND;
    else if (strstr(loweredlogic, "curse") != NULL)
        ret = OBJ_CURSE;
    else if (strstr(loweredlogic, "treasure") != NULL)
        ret = OBJ_TREASURE;
    else if (IsObjectEnemy(obj))
        ret = OBJ_ENEMY;
    delete[] loweredlogic;
    //delete []
    return ret;
}

bool State::EditingWW::IsEditableObject(WWD::Object *obj, ObjEdit::cObjEdit **hEdit) {
    char *loweredlogic = SHR::ToLower(obj->GetLogic());
    bool ret = 0;
    if (strstr(loweredlogic, "curse")) {
        /*!strcmp(obj->GetLogic(), "StartElevator") ||
        !strcmp(obj->GetLogic(), "TriggerElevator") ||
        !strcmp(obj->GetLogic(), "OneWayStartElevator") ||
        !strcmp(obj->GetLogic(), "OneWayTriggerElevator") ||
        !strcmp(obj->GetLogic(), "PathElevator") ||
        !strcmp(obj->GetLogic(), "StandardElevator") ){*/
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjCurse(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "PathElevator")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjElevPath(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "Checkpoint") ||
               !strcmp(obj->GetLogic(), "FirstSuperCheckpoint") ||
               !strcmp(obj->GetLogic(), "SecondSuperCheckpoint")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjCheckpoint(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "SpecialPowerup") &&
               (!strcmp(obj->GetImageSet(), "GAME_WARP") || !strcmp(obj->GetImageSet(), "GAME_VERTWARP")) ||
               !strcmp(obj->GetLogic(), "BossWarp") && !strcmp(obj->GetImageSet(), "GAME_BOSSWARP")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjWarp(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "TowerCannonLeft") || !strcmp(obj->GetLogic(), "TowerCannonRight") ||
               !strcmp(obj->GetLogic(), "SkullCannon")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjWallCannon(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "CrumblingPeg") || !strcmp(obj->GetLogic(), "CrumblingPegNoRespawn")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjCrumblingPeg(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "FrontStatue") || !strcmp(obj->GetLogic(), "BehindStatue")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjStatue(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "BreakPlank")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjBreakPlank(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "FrontCrate") || !strcmp(obj->GetLogic(), "FrontStackedCrates") ||
               !strcmp(obj->GetLogic(), "BehindCrate") || !strcmp(obj->GetLogic(), "BackStackedCrates")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjCrate(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "GlitterlessPowerup") || !strcmp(obj->GetLogic(), "TreasurePowerup")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjTreasure(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "AniRope") ||
               !strcmp(obj->GetLogic(), "DoNothing") && !strcmp(obj->GetImageSet(), "LEVEL_ROPE")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjRope(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "HealthPowerup") &&
               !(!strcmp(obj->GetImageSet(), "GAME_CATNIPS_NIP1") ||
                 !strcmp(obj->GetImageSet(), "GAME_CATNIPS_NIP2")) ||
               !strcmp(obj->GetLogic(), "AmmoPowerup") || !strcmp(obj->GetLogic(), "MagicPowerup") ||
               !strcmp(obj->GetLogic(), "SpecialPowerup") && !strcmp(obj->GetImageSet(), "GAME_DYNAMITE")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjHealth(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "SpecialPowerup") || !strcmp(obj->GetLogic(), "HealthPowerup")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjSpecialPowerup(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "TogglePeg") || !strcmp(obj->GetLogic(), "TogglePeg2") ||
               !strcmp(obj->GetLogic(), "TogglePeg3") || !strcmp(obj->GetLogic(), "TogglePeg4")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjTogglePeg(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "FrontAniCandy") || !strcmp(obj->GetLogic(), "BehindAniCandy") ||
               !strcmp(obj->GetLogic(), "FrontCandy") || !strcmp(obj->GetLogic(), "BehindCandy") ||
               !strcmp(obj->GetLogic(), "DoNothing") || !strcmp(obj->GetLogic(), "DoNothingNormal") ||
               !strcmp(obj->GetLogic(), "AniCycle")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjCandy(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "SpringBoard") || !strcmp(obj->GetLogic(), "GroundBlower")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjSpringboard(obj, this);
        ret = 1;
    } else if (strstr(obj->GetLogic(), "SoundTrigger")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjDialog(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "TProjectile")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjProjectile(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "Elevator") || !strcmp(obj->GetLogic(), "StartElevator") ||
               !strcmp(obj->GetLogic(), "TriggerElevator") || !strcmp(obj->GetLogic(), "StopElevator") ||
               !strcmp(obj->GetLogic(), "OneWayStartElevator") || !strcmp(obj->GetLogic(), "OneWayTriggerElevator") ||
               !strcmp(obj->GetLogic(), "StandardElevator")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjElevator(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "CrabNest")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjCrabNest(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "Shake")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjShake(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "Laser")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjLaser(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "Stalactite")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjStalactite(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "GlobalAmbientSound") ||
               !strcmp(obj->GetLogic(), "AmbientSound") ||
               !strcmp(obj->GetLogic(), "SpotAmbientSound") ||
               !strcmp(obj->GetLogic(), "AmbientPosSound")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjAmbient(obj, this);
        ret = 1;
    } else if (IsObjectEnemy(obj)) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjEnemy(obj, this);
        ret = 1;
    } else if (!strcmp(obj->GetLogic(), "_WM_TEXT")) {
        if (hEdit != 0) *hEdit = new ObjEdit::cEditObjText(obj, this);
        ret = 1;
    }
    delete[] loweredlogic;
    return ret;
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

bool State::EditingWW::IsObjectEnemy(WWD::Object *obj) {
    if (!strcmp(obj->GetLogic(), "Rat") ||
        !strcmp(obj->GetLogic(), "Soldier") ||
        !strcmp(obj->GetLogic(), "Officer") ||
        !strcmp(obj->GetLogic(), "Raux") ||
        !strcmp(obj->GetLogic(), "RobberThief") ||
        !strcmp(obj->GetLogic(), "CutThroat") ||
        !strcmp(obj->GetLogic(), "Katherine") ||
        !strcmp(obj->GetLogic(), "Seagull") ||
        !strcmp(obj->GetLogic(), "TownGuard1") ||
        !strcmp(obj->GetLogic(), "TownGuard2") ||
        !strcmp(obj->GetLogic(), "Wolvington") ||
        !strcmp(obj->GetLogic(), "HermitCrab") ||
        !strcmp(obj->GetLogic(), "BearSailor") ||
        !strcmp(obj->GetLogic(), "RedTailPirate") ||
        !strcmp(obj->GetLogic(), "Gabriel") ||
        !strcmp(obj->GetLogic(), "PegLeg") || //yup, pirate with guns
        !strcmp(obj->GetLogic(), "CrazyHook") ||
        !strcmp(obj->GetLogic(), "Marrow") ||
        !strcmp(obj->GetLogic(), "Fish") ||
        !strcmp(obj->GetLogic(), "Mercat") ||
        !strcmp(obj->GetLogic(), "Mermaid") ||
        !strcmp(obj->GetLogic(), "Aquatis") ||
        !strcmp(obj->GetLogic(), "Chameleon") ||
        !strcmp(obj->GetLogic(), "RedTail") ||
        !strcmp(obj->GetLogic(), "TigerGuard") ||
        !strcmp(obj->GetLogic(), "Omar")) {
        return 1;
    }
    return 0;
}

void State::EditingWW::CreateObjectWithEasyEdit(gcn::Widget *widg) {
    WWD::Object *obj = new WWD::Object();
    if (hmbObject->GetContext()->isVisible() && hmbObject->GetContext()->GetSelectedID() != -1) {
        obj->SetParam(WWD::Param_LocationX, Scr2WrdX(GetActivePlane(), objContext->getX()));
        obj->SetParam(WWD::Param_LocationY, Scr2WrdY(GetActivePlane(), objContext->getY()));
    } else {
        obj->SetParam(WWD::Param_LocationX, Scr2WrdX(GetActivePlane(), vPort->GetX() + vPort->GetWidth() / 2));
        obj->SetParam(WWD::Param_LocationY, Scr2WrdY(GetActivePlane(), vPort->GetY() + vPort->GetHeight() / 2));
    }
    obj->SetParam(WWD::Param_LocationI, -1);

    bool bDoContext = 1;
    if (widg == hmbObject->butIconCurse) {
        obj->SetLogic("CursePowerup");
        obj->SetImageSet("GAME_CURSES_FREEZE");
    } else if (widg == hmbObject->butIconPathElevator) {
        obj->SetLogic("PathElevator");
        if (hParser->GetBaseLevel() == 1) obj->SetImageSet("LEVEL_ELEVATORS");
        else if (hParser->GetBaseLevel() == 13 ||
                 hParser->GetBaseLevel() == 3)
            obj->SetImageSet("LEVEL_ELEVATOR1");
        else if (hParser->GetBaseLevel() == 6) obj->SetImageSet("LEVEL_GRILLELEVATOR");
        else if (hParser->GetBaseLevel() == 10) obj->SetImageSet("LEVEL_TRAPELEVATOR");
        else if (hParser->GetBaseLevel() == 7) obj->SetImageSet("LEVEL_AIRCART");
        else if (hParser->GetBaseLevel() == 2 ||
                 hParser->GetBaseLevel() == 4 ||
                 hParser->GetBaseLevel() == 5 ||
                 hParser->GetBaseLevel() == 9 ||
                 hParser->GetBaseLevel() > 10)
            obj->SetImageSet("LEVEL_ELEVATOR");
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
        bDoContext = 0;
    } else if (widg == hmbObject->butIconPowderKeg) {
        obj->SetLogic("PowderKeg");
        obj->SetImageSet("LEVEL_POWDERKEG");
        bDoContext = 0;
    } else if (widg == hmbObject->butIconWallCannon) {
        obj->SetLogic("TowerCannonRight");
        obj->SetImageSet("LEVEL_TOWERCANNONRIGHT");
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
        obj->SetLogic("FrontCandy");
        obj->SetParam(WWD::Param_LocationI, -1);
        obj->SetParam(WWD::Param_LocationZ, 5000);
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
    } else if (widg == hmbObject->butIconCannon) {
        if (hParser->GetBaseLevel() == 2 || hParser->GetBaseLevel() == 8) {
            obj->SetLogic("PunkRat");
            obj->SetImageSet("LEVEL_PUNKRAT");
            bDoContext = 0;
        } else {
            obj->SetLogic("SkullCannon");
            obj->SetImageSet("LEVEL_SKULLCANNON");
        }
    } else if (widg == hmbObject->butIconSound) {
        obj->SetLogic("GlobalAmbientSound");
        obj->SetImageSet("GAME_SOUNDICON");
        obj->SetDrawFlags(WWD::Flag_dr_NoDraw);
    } else if (widg == hmbObject->butIconDialog) {
        obj->SetLogic("SoundTrigger");
        obj->SetImageSet("GAME_SOUNDICON");
        obj->SetDrawFlags(WWD::Flag_dr_NoDraw);
    } else if (widg == hmbObject->butIconEnemy[0]) {
        if (hParser->GetBaseLevel() == 1 || hParser->GetBaseLevel() == 2) {
            obj->SetLogic("Officer");
            obj->SetImageSet("LEVEL_OFFICER");
        } else if (hParser->GetBaseLevel() == 3 || hParser->GetBaseLevel() == 4) {
            obj->SetLogic("RobberThief");
            obj->SetImageSet("LEVEL_ROBBERTHIEF");
        } else if (hParser->GetBaseLevel() == 5 || hParser->GetBaseLevel() == 6) {
            obj->SetLogic("TownGuard1");
            obj->SetImageSet("LEVEL_TOWNGUARD1");
        } else if (hParser->GetBaseLevel() == 7 || hParser->GetBaseLevel() == 8 || hParser->GetBaseLevel() == 13) {
            obj->SetLogic("RedTailPirate");
            obj->SetImageSet("LEVEL_REDTAILPIRATE");
        } else if (hParser->GetBaseLevel() == 9 || hParser->GetBaseLevel() == 10) {
            obj->SetLogic("PegLeg");
            obj->SetImageSet("LEVEL_PEGLEG");
        } else if (hParser->GetBaseLevel() == 11 || hParser->GetBaseLevel() == 12) {
            obj->SetLogic("Mercat");
            obj->SetImageSet("LEVEL_MERCAT");
        } else if (hParser->GetBaseLevel() == 14) {
            obj->SetLogic("TigerGuard");
            obj->SetImageSet("LEVEL_TIGER");
        }
    } else if (widg == hmbObject->butIconEnemy[1]) {
        if (hParser->GetBaseLevel() == 1 || hParser->GetBaseLevel() == 2) {
            obj->SetLogic("Soldier");
            obj->SetImageSet("LEVEL_SOLDIER");
        } else if (hParser->GetBaseLevel() == 3 || hParser->GetBaseLevel() == 4) {
            obj->SetLogic("CutThroat");
            obj->SetImageSet("LEVEL_CUTTHROAT");
        } else if (hParser->GetBaseLevel() == 5 || hParser->GetBaseLevel() == 6) {
            obj->SetLogic("TownGuard2");
            obj->SetImageSet("LEVEL_TOWNGUARD2");
        } else if (hParser->GetBaseLevel() == 7 || hParser->GetBaseLevel() == 8 || hParser->GetBaseLevel() == 13) {
            obj->SetLogic("BearSailor");
            obj->SetImageSet("LEVEL_BEARSAILOR");
        } else if (hParser->GetBaseLevel() == 9 || hParser->GetBaseLevel() == 10) {
            obj->SetLogic("CrazyHook");
            obj->SetImageSet("LEVEL_CRAZYHOOK");
        } else if (hParser->GetBaseLevel() == 11 || hParser->GetBaseLevel() == 12) {
            obj->SetLogic("Siren");
            obj->SetImageSet("LEVEL_SIREN");
        } else if (hParser->GetBaseLevel() == 14) {
            obj->SetLogic("TigerGuard");
            obj->SetImageSet("LEVEL_TIGER");
            obj->SetParam(WWD::Param_Smarts, 1);
        }
    } else if (widg == hmbObject->butIconEnemy[2]) {
        if (hParser->GetBaseLevel() == 2) {
            obj->SetLogic("Raux");
            obj->SetImageSet("LEVEL_RAUX");
        } else if (hParser->GetBaseLevel() < 5) {
            obj->SetLogic("Rat");
            obj->SetImageSet("LEVEL_RAT");
        } else if (hParser->GetBaseLevel() == 5 || hParser->GetBaseLevel() == 6 ||
                   hParser->GetBaseLevel() == 9 || hParser->GetBaseLevel() == 10 ||
                   hParser->GetBaseLevel() == 7 || hParser->GetBaseLevel() == 8) {
            obj->SetLogic("Seagull");
            obj->SetImageSet("LEVEL_SEAGULL");
        } else if (hParser->GetBaseLevel() == 11 || hParser->GetBaseLevel() == 12) {
            obj->SetLogic("Fish");
            obj->SetImageSet("LEVEL_FISH");
        } else if (hParser->GetBaseLevel() == 13 || hParser->GetBaseLevel() == 14) {
            obj->SetLogic("Chameleon");
            obj->SetImageSet("LEVEL_CHAMELEON");
        }
    } else if (widg == hmbObject->butIconEnemy[3]) {
        if (hParser->GetBaseLevel() == 4) {
            obj->SetLogic("Katherine");
            obj->SetImageSet("LEVEL_KATHERINE");
        } else if (hParser->GetBaseLevel() == 6) {
            obj->SetLogic("Rat");
            obj->SetImageSet("LEVEL_RAT");
        } else if (hParser->GetBaseLevel() == 7) {
            obj->SetLogic("HermitCrab");
            obj->SetImageSet("LEVEL_HERMITCRAB");
        } else if (hParser->GetBaseLevel() == 10) {
            obj->SetLogic("Marrow");
            obj->SetImageSet("LEVEL_MARROW");
        } else if (hParser->GetBaseLevel() == 12) {
            obj->SetLogic("Aquatis");
            obj->SetImageSet("LEVEL_KINGAQUATIS");
        } else if (hParser->GetBaseLevel() == 13) {
            obj->SetLogic("RedTail");
            obj->SetImageSet("LEVEL_REDTAIL");
        } else if (hParser->GetBaseLevel() == 14) {
            obj->SetLogic("Omar");
            obj->SetImageSet("LEVEL_OMAR");
        } else if (hParser->GetBaseLevel() == 8) {
            obj->SetLogic("Gabriel");
            obj->SetImageSet("LEVEL_GABRIEL");
        }
    } else if (widg == hmbObject->butIconEnemy[4]) {
        if (hParser->GetBaseLevel() == 6) {
            obj->SetLogic("Wolvington");
            obj->SetImageSet("LEVEL_WOLVINGTON");
        }
    } else if (widg == hmbObject->butIconText) {
        obj->SetLogic("_WM_TEXT");
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
    bEditObjDelete = 1;
    vPort->MarkToRedraw(1);
}

bool State::EditingWW::AreObjectSpecificOptionsAvailable(WWD::Object *obj, SHR::Context **conMod) {
    if (!strcmp(obj->GetLogic(), "SpecialPowerup") &&
        (!strcmp(obj->GetImageSet(), "GAME_WARP") || !strcmp(obj->GetImageSet(), "GAME_VERTWARP")) ||
        !strcmp(obj->GetLogic(), "BossWarp")) {
        if (conMod != 0)
            *conMod = advcon_Warp;
        return 1;
    } else if (!strcmp(obj->GetLogic(), "FrontCrate") || !strcmp(obj->GetLogic(), "FrontStackedCrates") ||
               !strcmp(obj->GetLogic(), "BehindCrate") || !strcmp(obj->GetLogic(), "BackStackedCrates")) {
        if (conMod != 0)
            *conMod = advcon_Container;
        return 1;
    }
    return 0;
}

std::vector<cInventoryItem> State::EditingWW::GetContenerItems(WWD::Object *obj) {
    return std::vector<cInventoryItem>();
}

void State::EditingWW::UpdateSearchResults() {
    vObjSearchResults.clear();
    if (szObjSearchBuffer == NULL || strlen(szObjSearchBuffer) == 0) {
        winSearchObj->setHeight(135);
        labObjSearchResults->setVisible(0);
        butObjSearchSelectAll->setVisible(0);
        sliSearchObj->setVisible(0);
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
            vObjSearchResults.push_back(
                    std::pair<int, int>(i, GetActivePlane()->GetObjectByIterator(i)->GetParam(WWD::Param_ID)));

            if (!cbObjSearchCaseSensitive->isSelected())
                delete[](char *) comp;
        }
    }
    if (!cbObjSearchCaseSensitive->isSelected()) delete[] comp2;
    int normalheight = int(135 + vObjSearchResults.size() * 140);
    sliSearchObj->setVisible(normalheight > 515);
    sliSearchObj->setScaleEnd(vObjSearchResults.size() * 140 - 387);
    sliSearchObj->setValue(vObjSearchResults.size() * 140 - 387);
    int winheight = winSearchObj->getHeight();
    winheight = std::min(normalheight, 515);
    winSearchObj->setHeight(winheight);
    if (vObjSearchResults.size() == 0) {
        labObjSearchResults->setCaption(GETL2S("ObjectSearch", "NoResults"));
    } else {
        char tmp[128];
        sprintf(tmp, "%s: ~y~%d~l~", GETL2S("ObjectSearch", "FoundResults"), vObjSearchResults.size());
        labObjSearchResults->setCaption(tmp);
    }
    butObjSearchSelectAll->setVisible(vObjSearchResults.size() > 0);
    labObjSearchResults->adjustSize();
    labObjSearchResults->setVisible(1);
}
