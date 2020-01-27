#include "../editing_ww.h"
#include "../../langID.h"
#include "../../cObjectUserData.h"
#include "../../../shared/gcnWidgets/wComboButton.h"
#include "../../databanks/tiles.h"

extern HGE *hge;

void State::EditingWW::HandleHotkeys() {
    if (iActiveTool == EWW_TOOL_EDITOBJ) {
        if (hge->Input_KeyDown(HGEK_ESCAPE)) {
            hEditObj->SetKill(1);
        }
        return;
    }
    if (!GV->StateMgr->IsAppFocused()) return;
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    bool bFocus = vPort->GetWidget()->isFocused();
    if (bFocus && hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_A)) {
        if (iMode == EWW_MODE_TILE) {
            iTileSelectX1 = iTileSelectY1 = 0;
            iTileSelectX2 = GetActivePlane()->GetPlaneWidth() - 1;
            iTileSelectY2 = GetActivePlane()->GetPlaneHeight() - 1;
            vPort->MarkToRedraw(1);
        } else if (iMode == EWW_MODE_OBJECT) {
            vObjectsPicked = hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->GetObjectsByArea(0, 0,
                                                                                                    GetActivePlane()->GetPlaneWidthPx(),
                                                                                                    GetActivePlane()->GetPlaneHeightPx());
        }
    } else if (iMode == EWW_MODE_OBJECT && vObjectsPicked.size() > 0 &&
               (hge->Input_KeyDown(HGEK_PGUP) || hge->Input_KeyDown(HGEK_PGDN))) {
        int id = (hge->Input_KeyDown(HGEK_PGUP) ? OBJMENU_ZC_INC : OBJMENU_ZC_DEC);
        if (hge->Input_GetKeyState(HGEK_SHIFT))
            id += 2;
        else if (hge->Input_GetKeyState(HGEK_CTRL))
            id += 1;
        objZCoordContext->EmulateClickID(id);
    } else if (iMode == EWW_MODE_OBJECT && hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_F) &&
               hmbObject->butIconSearchObject->isEnabled()) {
        hmbObject->butIconSearchObject->simulatePress();
    } else if (bFocus && hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_X) &&
               vObjectsPicked.size() != 0 && iMode == EWW_MODE_OBJECT) {
        objContext->EmulateClickID(OBJMENU_CUT);
    } else if (bFocus && hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_C) &&
               vObjectsPicked.size() != 0 && iMode == EWW_MODE_OBJECT) {
        objContext->EmulateClickID(OBJMENU_COPY);
    } else if (bFocus && hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_C) && iMode == EWW_MODE_TILE &&
               iTileSelectX1 != -1 && iTileSelectX2 != -1 && iTileSelectY1 != -1 && iTileSelectY2 != -1) {
        tilContext->EmulateClickID(TILMENU_COPY);
    } else if (bFocus && hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_X) && iMode == EWW_MODE_TILE &&
               iTileSelectX1 != -1 && iTileSelectX2 != -1 && iTileSelectY1 != -1 && iTileSelectY2 != -1) {
        tilContext->EmulateClickID(TILMENU_CUT);
    } else if (bFocus && iMode == EWW_MODE_TILE && MDI->GetActiveDoc()->hTileClipboard != NULL &&
               (hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_V) ||
                bFocus && hge->Input_GetKeyState(HGEK_TAB) && hge->Input_KeyDown(HGEK_LBUTTON))) {
        tilContext->EmulateClickID(TILMENU_PASTE);
    } else if (bFocus && iMode == EWW_MODE_TILE && hge->Input_KeyDown(HGEK_DELETE) &&
               iTileSelectX1 != -1 && iTileSelectX2 != -1 && iTileSelectY1 != -1 && iTileSelectY2 != -1) {
        tilContext->EmulateClickID(TILMENU_DELETE);
    } else if (bFocus && hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_V) &&
               vObjectClipboard.size() != 0 && iMode == EWW_MODE_OBJECT) {
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        objContext->setX(mx);
        objContext->setY(my);
        objContext->EmulateClickID(OBJMENU_PASTE);
    } else if (bFocus && hge->Input_KeyDown(HGEK_DELETE) && vObjectsPicked.size() != 0) {
        objContext->EmulateClickID(OBJMENU_DELETE);
    } else if (hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_GetKeyState(HGEK_ALT) && hge->Input_KeyDown(HGEK_P)) {

    } else if (bFocus && iMode == EWW_MODE_TILE &&
               (iActiveTool == EWW_TOOL_BRUSH || iActiveTool == EWW_TOOL_PENCIL || iActiveTool == EWW_TOOL_FILL)) {
        if (hge->Input_KeyDown(HGEK_R))
            iTilePicked = EWW_TILE_ERASE;
        else if (hge->Input_KeyDown(HGEK_P))
            iTilePicked = EWW_TILE_PIPETTE;
        else if (hge->Input_KeyDown(HGEK_F))
            iTilePicked = EWW_TILE_FILL;

        int iOldP = iTilePicked;
        if (hge->Input_KeyDown(HGEK_ADD)) {
            if ((iActiveTool == EWW_TOOL_PENCIL &&
                 iTilePicked + 1 < hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetTilesCount()) ||
                (iActiveTool == EWW_TOOL_BRUSH &&
                 iTilePicked + 1 < hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetBrushesCount())) {
                iTilePicked++;
                vPort->MarkToRedraw(1);
            }
        } else if (hge->Input_KeyDown(HGEK_SUBTRACT)) {
            if (iTilePicked > 0) {
                iTilePicked--;
                vPort->MarkToRedraw(1);
            }
        }
        if (iOldP != iTilePicked && iActiveTool == EWW_TOOL_BRUSH) {
        }
    } else if (iMode == EWW_MODE_OBJECT && vObjectsPicked.size() != 0 &&
               conMain->getWidgetAt(mx, my) == vPort->GetWidget() &&
               (hge->Input_GetKeyState(HGEK_CTRL) || hge->Input_GetKeyState(HGEK_SHIFT) ||
                hge->Input_GetKeyState(HGEK_ALT)) &&
               (hge->Input_KeyDown(HGEK_LEFT) || hge->Input_KeyDown(HGEK_RIGHT) || hge->Input_KeyDown(HGEK_UP) ||
                hge->Input_KeyDown(HGEK_DOWN))) {
        int moveresx = 0, moveresy = 0, moveres = 0;
        if (hge->Input_GetKeyState(HGEK_CTRL)) moveres = 1;
        else if (hge->Input_GetKeyState(HGEK_SHIFT)) moveres = 10;
        else if (hge->Input_GetKeyState(HGEK_ALT)) moveres = 64;

        if (hge->Input_KeyDown(HGEK_LEFT)) moveresx = -moveres;
        else if (hge->Input_KeyDown(HGEK_RIGHT)) moveresx = moveres;
        else if (hge->Input_KeyDown(HGEK_UP)) moveresy = -moveres;
        else if (hge->Input_KeyDown(HGEK_DOWN)) moveresy = moveres;

        for (int i = 0; i < vObjectsPicked.size(); i++) {
            int posx = vObjectsPicked[i]->GetParam(WWD::Param_LocationX),
                    posy = vObjectsPicked[i]->GetParam(WWD::Param_LocationY);
            posx += moveresx;
            posy += moveresy;
            if (posx < 0 || posy < 0 || posx >= GetActivePlane()->GetPlaneWidthPx() ||
                posy >= GetActivePlane()->GetPlaneHeightPx())
                continue;
            vObjectsPicked[i]->SetParam(WWD::Param_LocationX, posx);
            vObjectsPicked[i]->SetParam(WWD::Param_LocationY, posy);
            GetUserDataFromObj(vObjectsPicked[i])->SyncToObj();
        }
        vPort->MarkToRedraw(1);
    } else if (hge->Input_KeyDown(HGEK_M) && iMode == EWW_MODE_OBJECT && vObjectsPicked.size() != 0 &&
               vPort->GetWidget()->isMouseOver()) {
        for (int i = 0; i < vObjectsPicked.size(); i++) {
            if (vObjectsPicked[i]->GetDrawFlags() & WWD::Flag_dr_Mirror)
                vObjectsPicked[i]->SetDrawFlags(
                        (WWD::OBJ_DRAW_FLAGS) (int(vObjectsPicked[i]->GetDrawFlags()) - WWD::Flag_dr_Mirror));
            else
                vObjectsPicked[i]->SetDrawFlags(
                        (WWD::OBJ_DRAW_FLAGS) (int(vObjectsPicked[i]->GetDrawFlags()) + WWD::Flag_dr_Mirror));
        }
        MarkUnsaved();
        vPort->MarkToRedraw(1);
    } else if (bFocus && hge->Input_KeyDown(HGEK_HOME)) {
        fCamX = hParser->GetStartX() - (vPort->GetWidth() / 2 / fZoom);
        fCamY = hParser->GetStartY() - (vPort->GetHeight() / 2 / fZoom);
        /*}else if( hge->Input_GetKeyState(HGEK_SHIFT) && hge->Input_KeyDown(HGEK_TAB) && iMode == EWW_MODE_OBJECT ){
         iObjectPicked--;
         if( iObjectPicked < 0 )
          iObjectPicked = GetActivePlane()->GetObjectsCount()-1;
         fCamX = GetActivePlane()->GetObjectByIterator(iObjectPicked)->GetParam(WWD::Param_LocationX)-(vPort->GetWidth()/2/fZoom);
         fCamY = GetActivePlane()->GetObjectByIterator(iObjectPicked)->GetParam(WWD::Param_LocationY)-(vPort->GetHeight()/2/fZoom);
        }else if( hge->Input_KeyDown(HGEK_TAB) && iMode == EWW_MODE_OBJECT ){
         iObjectPicked++;
         if( iObjectPicked >= GetActivePlane()->GetObjectsCount() )
          iObjectPicked = 0;
         fCamX = GetActivePlane()->GetObjectByIterator(iObjectPicked)->GetParam(WWD::Param_LocationX)-(vPort->GetWidth()/2/fZoom);
         fCamY = GetActivePlane()->GetObjectByIterator(iObjectPicked)->GetParam(WWD::Param_LocationY)-(vPort->GetHeight()/2/fZoom);*/
    } else if (bFocus && hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_D) && iMode == EWW_MODE_OBJECT &&
               vObjectsPicked.size() != 0) {
        ShowAndUpdateDuplicateMenu();
#ifndef CONF_NOSAVE
    } else if (hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_GetKeyState(HGEK_ALT) && hge->Input_KeyDown(HGEK_S)) {

    } else if (hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_S)) {

#endif
    } else if (hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_N)) {

    } else if (hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_T)) {

    } else if (hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_M)) {
        cbutActiveMode->setSelectedEntryID(!cbutActiveMode->getSelectedEntryID(), 1);
        //butIconMode->simulatePress();
        //}else if( hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_O) ){
        /*hPlaneData[GetActivePlaneID()]->bDrawObjects = !hPlaneData[GetActivePlaneID()]->bDrawObjects;
        vPort->MarkToRedraw(1);
        if( hPlaneData[GetActivePlaneID()]->bDrawObjects ){
         SetHint(HINT_OBJON, GETL(Lang_ObjectsOn), GetActivePlane()->GetName());
        }else{
         SetHint(HINT_OBJOFF, GETL(Lang_ObjectsOff), GetActivePlane()->GetName());
        }*/
    } else if (bFocus && hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_G)) {
        hPlaneData[GetActivePlaneID()]->bDrawGrid = !hPlaneData[GetActivePlaneID()]->bDrawGrid;
        vPort->MarkToRedraw(1);
        if (hPlaneData[GetActivePlaneID()]->bDrawGrid) {
            SetHint(GETL(Lang_GridOn), GetActivePlane()->GetName());
        } else {
            SetHint(GETL(Lang_GridOff), GetActivePlane()->GetName());
        }
    } else if (bFocus && hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_B)) {
        hPlaneData[GetActivePlaneID()]->bDrawBoundary = !hPlaneData[GetActivePlaneID()]->bDrawBoundary;
        vPort->MarkToRedraw(1);
        if (hPlaneData[GetActivePlaneID()]->bDrawBoundary) {
            SetHint(GETL(Lang_BoundaryOn), GetActivePlane()->GetName());
        } else {
            SetHint(GETL(Lang_BoundaryOff), GetActivePlane()->GetName());
        }
    } else if (bFocus && iMode == EWW_MODE_OBJECT && vObjectsPicked.size() == 1) {
        if (hge->Input_KeyDown(HGEK_C)) {
            fCamX = GetUserDataFromObj(vObjectsPicked[0])->GetX() - vPort->GetWidth() / 2 / fZoom;
            fCamY = GetUserDataFromObj(vObjectsPicked[0])->GetY() - vPort->GetHeight() / 2 / fZoom;
        } else if (hge->Input_KeyDown(HGEK_P)) {
            OpenObjectWindow(vObjectsPicked[0]);
        } else if (hge->Input_KeyDown(HGEK_E)) {
            if (IsEditableObject(vObjectsPicked[0], NULL)) {
                OpenObjectEdit(vObjectsPicked[0]);
            }
        }
    } else if (hge->Input_KeyDown(HGEK_ESCAPE)) {
        if (iActiveTool == EWW_TOOL_MEASURE || iActiveTool == EWW_TOOL_BRUSHOBJECT) {
            SetTool(EWW_TOOL_NONE);
        }
    }
}
