#include "../editing_ww.h"
#include "../../langID.h"
#include "../../cObjectUserData.h"
#include "../../../shared/gcnWidgets/wComboButton.h"
#include "../../databanks/tiles.h"

extern HGE *hge;

void State::EditingWW::HandleMirrorAndInvertHotkeys()
{
	if (iActiveTool == EWW_TOOL_EDITOBJ) {
		if (hEditObj->IsAnyInputFocused()) return;
	}
	int flag = hge->Input_KeyDown(HGEK_I) ? WWD::Flag_dr_Invert : WWD::Flag_dr_Mirror;

	for (auto & object : vObjectsPicked) {
		if (object->GetDrawFlags() & flag)
			object->SetDrawFlags(
			(WWD::OBJ_DRAW_FLAGS) (int(object->GetDrawFlags()) - flag));
		else
			object->SetDrawFlags(
			(WWD::OBJ_DRAW_FLAGS) (int(object->GetDrawFlags()) + flag));

		GetUserDataFromObj(object)->SyncToObj();
	}

	MarkUnsaved();
	vPort->MarkToRedraw();
}

void State::EditingWW::HandleHotkeys() {
    if (iActiveTool == EWW_TOOL_EDITOBJ) {
        if (hge->Input_KeyDown(HGEK_ESCAPE)) {
            hEditObj->SetKill(1);
		} else if (hge->Input_KeyDown(HGEK_M) || hge->Input_KeyDown(HGEK_I)) {
			HandleMirrorAndInvertHotkeys();
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
            vPort->MarkToRedraw();
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
               ((hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_V)) ||
                (bFocus && hge->Input_GetKeyState(HGEK_ALT) && hge->Input_KeyDown(HGEK_LBUTTON)))) {
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        contextX = mx;
        contextY = my;
        tilContext->EmulateClickID(TILMENU_PASTE);
    } else if (bFocus && iMode == EWW_MODE_TILE && hge->Input_KeyDown(HGEK_DELETE) &&
               iTileSelectX1 != -1 && iTileSelectX2 != -1 && iTileSelectY1 != -1 && iTileSelectY2 != -1) {
        tilContext->EmulateClickID(TILMENU_DELETE);
    } else if (bFocus && hge->Input_KeyDown(HGEK_DELETE) && vObjectsPicked.size() != 0) {
        objContext->EmulateClickID(OBJMENU_DELETE);
    } else if (hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_GetKeyState(HGEK_ALT) && hge->Input_KeyDown(HGEK_P)) {

    } else if (iMode == EWW_MODE_TILE && (iActiveTool == EWW_TOOL_BRUSH || iActiveTool == EWW_TOOL_PENCIL || iActiveTool == EWW_TOOL_FILL)) {
        if (hge->Input_KeyDown(HGEK_R)) {
            SetTool(EWW_TOOL_PENCIL);
            iTilePicked = EWW_TILE_ERASE;
            vPort->MarkToRedraw();
        }
        else if (hge->Input_KeyDown(HGEK_P) && (iActiveTool == EWW_TOOL_PENCIL || iActiveTool == EWW_TOOL_FILL)) {
            iTilePicked = EWW_TILE_PIPETTE;
            vPort->MarkToRedraw();
        }
        else if (hge->Input_KeyDown(HGEK_F)) {
            SetTool(EWW_TOOL_PENCIL);
            iTilePicked = EWW_TILE_FILL;
            vPort->MarkToRedraw();
        }

        int iOldP = iTilePicked;
        if (hge->Input_KeyDown(HGEK_ADD)) {
            if ((iActiveTool == EWW_TOOL_PENCIL &&
                 iTilePicked + 1 < hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetTilesCount()) ||
                (iActiveTool == EWW_TOOL_BRUSH &&
                 iTilePicked + 1 < hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetBrushesCount())) {
                iTilePicked++;
                vPort->MarkToRedraw();
            }
        } else if (hge->Input_KeyDown(HGEK_SUBTRACT)) {
            if (iTilePicked > 0) {
                iTilePicked--;
                vPort->MarkToRedraw();
            }
        }
        if (iOldP != iTilePicked && iActiveTool == EWW_TOOL_BRUSH) {
        }
    } else if (iMode == EWW_MODE_OBJECT && !vObjectsPicked.empty() &&
               vPort->GetWidget()->isFocused() &&
               (hge->Input_GetKeyState(HGEK_CTRL) || hge->Input_GetKeyState(HGEK_SHIFT) ||
                hge->Input_GetKeyState(HGEK_ALT)) &&
               (hge->Input_KeyDown(HGEK_LEFT) || hge->Input_KeyDown(HGEK_RIGHT) || hge->Input_KeyDown(HGEK_UP) ||
                hge->Input_KeyDown(HGEK_DOWN))) {
        int moveResX = 0, moveResY = 0, moveRes = 0;
        if (hge->Input_GetKeyState(HGEK_CTRL)) moveRes = 1;
        else if (hge->Input_GetKeyState(HGEK_SHIFT)) moveRes = 10;
        else if (hge->Input_GetKeyState(HGEK_ALT)) moveRes = 64;

        if (hge->Input_KeyDown(HGEK_LEFT)) moveResX = -moveRes;
        else if (hge->Input_KeyDown(HGEK_RIGHT)) moveResX = moveRes;
        else if (hge->Input_KeyDown(HGEK_UP)) moveResY = -moveRes;
        else if (hge->Input_KeyDown(HGEK_DOWN)) moveResY = moveRes;

        for (auto & object : vObjectsPicked) {
            int posX = object->GetParam(WWD::Param_LocationX),
                posY = object->GetParam(WWD::Param_LocationY);
            posX += moveResX;
            posY += moveResY;
            if (posX < 0 || posY < 0 || posX >= GetActivePlane()->GetPlaneWidthPx() ||
                posY >= GetActivePlane()->GetPlaneHeightPx())
                continue;
            object->SetParam(WWD::Param_LocationX, posX);
            object->SetParam(WWD::Param_LocationY, posY);
            GetUserDataFromObj(object)->SyncToObj();
        }
        vPort->MarkToRedraw();
        MarkUnsaved();
    } else if ((hge->Input_KeyDown(HGEK_M) || hge->Input_KeyDown(HGEK_I)) && iMode == EWW_MODE_OBJECT && !vObjectsPicked.empty() && vPort->GetWidget()->isMouseOver()) {
		HandleMirrorAndInvertHotkeys();
    } else if (bFocus && hge->Input_KeyDown(HGEK_HOME)) {
        fCamX = hParser->GetStartX() - (vPort->GetWidth() / 2 / fZoom);
        fCamY = hParser->GetStartY() - (vPort->GetHeight() / 2 / fZoom);
    } else if (bFocus && hParser && hge->Input_KeyDown(HGEK_END)) {
        for (int i = 0; i < hParser->GetMainPlane()->GetObjectsCount(); ++i) {
            auto object = hParser->GetMainPlane()->GetObjectByIterator(i);

            if (!strcmp(object->GetLogic(), "BossStager") || !strcmp(object->GetLogic(), "EndOfLevelPowerup")
                || (!strcmp(object->GetLogic(), "SpecialPowerup")
                    && (!strcmp(object->GetImageSet(), "GAME_MAPPIECE")
                        || !strcmp(object->GetImageSet(), "LEVEL_GEM")))) {
                fCamX = object->GetX() - (vPort->GetWidth() / 2 / fZoom);
                fCamY = object->GetY() - (vPort->GetHeight() / 2 / fZoom);

                return;
            }
        }
    } else if (hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_M)) {
        cbutActiveMode->setSelectedEntryID(!cbutActiveMode->getSelectedEntryID(), 1);
    } else if (bFocus && hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_G)) {
        hPlaneData[GetActivePlaneID()]->bDrawGrid = !hPlaneData[GetActivePlaneID()]->bDrawGrid;
        vPort->MarkToRedraw();
    } else if (bFocus && hge->Input_GetKeyState(HGEK_CTRL) && hge->Input_KeyDown(HGEK_B)) {
        hPlaneData[GetActivePlaneID()]->bDrawBoundary = !hPlaneData[GetActivePlaneID()]->bDrawBoundary;
        vPort->MarkToRedraw();
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
