#include "../editing_ww.h"
#include "../../globals.h"
#include "../../conf.h"
#include "../error.h"
#include "../../../shared/commonFunc.h"
#include "../../langID.h"
#include "../../cObjectUserData.h"
#include "../../databanks/tiles.h"

extern HGE * hge;

void State::EditingWW::DrawPlaneProperties()
{
	int dx, dy;
	winpmMain->getAbsolutePosition(dx, dy);

	if (lbpmPlanes->getSelected() != -1) {
		hge->Gfx_RenderLine(dx + 254 - 35, dy + 106, dx + winpmMain->getWidth(), dy + 106, 0xFF1f1f1f);
		hge->Gfx_RenderLine(dx + 254 - 35, dy + 107, dx + winpmMain->getWidth(), dy + 107, 0xFF5c5c5c);

		//hge->Gfx_RenderLine(dx+254-35, dy+163, dx+254+276-35, dy+163, 0xFF1f1f1f);
		//hge->Gfx_RenderLine(dx+254-35, dy+164, dx+254+276-35, dy+164, 0xFF5c5c5c);

		hge->Gfx_RenderLine(dx + 254 - 35, dy + 235, dx + winpmMain->getWidth(), dy + 235, 0xFF1f1f1f);
		hge->Gfx_RenderLine(dx + 254 - 35, dy + 236, dx + winpmMain->getWidth(), dy + 236, 0xFF5c5c5c);

		hge->Gfx_RenderLine(dx + 254 - 35, dy + 24, dx + 254 - 35, dy + winpmMain->getHeight(), 0xFF1f1f1f);
		hge->Gfx_RenderLine(dx + 255 - 35, dy + 24, dx + 255 - 35, dy + winpmMain->getHeight(), 0xFF5c5c5c);

		hge->Gfx_RenderLine(dx + 253 + 277 - 35, dy + 24, dx + 253 + 277 - 35, dy + 235, 0xFF1f1f1f);
		hge->Gfx_RenderLine(dx + 254 + 277 - 35, dy + 24, dx + 254 + 277 - 35, dy + 235, 0xFF5c5c5c);

		if (atoi(tfpmPlaneSizeX->getText().c_str()) != ipmSizeX || atoi(tfpmPlaneSizeY->getText().c_str()) != ipmSizeY) {
			ipmSizeX = atoi(tfpmPlaneSizeX->getText().c_str());
			ipmSizeY = atoi(tfpmPlaneSizeY->getText().c_str());
			SetAnchorPlaneProperties(ipmAnchor);
		}
	}

}

void State::EditingWW::SyncPlaneProperties()
{
	if (hParser != NULL && lbpmPlanes->getSelected() == hParser->GetPlanesCount()) {
		if (winpmMain->getWidth() != 735) {
			winpmMain->setX(winpmMain->getX() - (735 / 2 - 225 / 2));
		}
		winpmMain->setWidth(735);
		labpmName->setVisible(1);
		tfpmName->setVisible(1);
		tfpmName->setText("");
		labpmTileSet->setVisible(1);
		ddpmTileSet->setVisible(1);
		ddpmTileSet->setSelected(0);
		labpmPlaneSize->setVisible(1);
		tfpmPlaneSizeX->setVisible(1);
		tfpmPlaneSizeX->setText("50");
		tfpmPlaneSizeY->setVisible(1);
		tfpmPlaneSizeY->setText("50");
		labpmTileSize->setVisible(1);
		labpmTileSize_x->setVisible(1);
		labpmMovX->setVisible(1);
		labpmMovY->setVisible(1);
		tfpmTileSizeX->setVisible(1);
		tfpmTileSizeX->setText("64");
		tfpmTileSizeY->setVisible(1);
		tfpmTileSizeY->setText("64");
		butpmResC->setVisible(1);
		butpmResD->setVisible(1);
		butpmResDL->setVisible(1);
		butpmResDR->setVisible(1);
		butpmResL->setVisible(1);
		butpmResR->setVisible(1);
		butpmResU->setVisible(1);
		butpmResUL->setVisible(1);
		butpmResUR->setVisible(1);

		SetAnchorPlaneProperties(5);
		butpmResC->setEnabled(0);
		butpmResD->setEnabled(0);
		butpmResDL->setEnabled(0);
		butpmResDR->setEnabled(0);
		butpmResL->setEnabled(0);
		butpmResR->setEnabled(0);
		butpmResU->setEnabled(0);
		butpmResUL->setEnabled(0);
		butpmResUR->setEnabled(0);

		labpmAnchor->setVisible(1);
		labpmWidth->setVisible(1);
		labpmHeight->setVisible(1);
		labpmZCoord->setVisible(1);
		tfpmZCoord->setVisible(1);
		tfpmZCoord->setText("0");
		labpmMovement->setVisible(1);
		tfpmMovX->setVisible(1);
		tfpmMovX->setText("100");
		tfpmMovY->setText("100");
		tfpmMovY->setVisible(1);
		labpmFlags->setVisible(1);
		cbpmFlagMainPlane->setVisible(1);
		cbpmFlagMainPlane->setSelected(0);
		cbpmFlagWrapX->setVisible(1);
		cbpmFlagWrapX->setSelected(0);
		cbpmFlagWrapY->setVisible(1);
		cbpmFlagWrapY->setSelected(0);
		cbpmFlagNoDraw->setVisible(1);
		cbpmFlagNoDraw->setSelected(0);
		cbpmAutoTileSize->setVisible(1);
		cbpmAutoTileSize->setSelected(0);
		butpmSave->setVisible(1);
		butpmDelete->setVisible(1);
		butpmDelete->setEnabled(0);
	}
	else if (hParser != NULL && lbpmPlanes->getSelected() < hParser->GetPlanesCount() && lbpmPlanes->getSelected() >= 0) {
		if (winpmMain->getWidth() != 735) {
			winpmMain->setX(winpmMain->getX() - (735 / 2 - 225 / 2));
		}
		winpmMain->setWidth(735);
		WWD::Plane * pl = hParser->GetPlane(lbpmPlanes->getSelected());
		labpmName->setVisible(1);
		tfpmName->setVisible(1);
		tfpmName->setText(pl->GetName());
		labpmTileSet->setVisible(1);
		ddpmTileSet->setVisible(1);
		ddpmTileSet->setListModel(hTileset);

		for (int i = 0; i < hTileset->GetSetsCount(); i++) {
			if (!strcmp(pl->GetImageSet(0), hTileset->GetSet(i)->GetName())) {
				ddpmTileSet->setSelected(i);
				break;
			}
		}

		char tmp[128];

		labpmPlaneSize->setVisible(1);
		tfpmPlaneSizeX->setVisible(1);
		tfpmPlaneSizeY->setVisible(1);

		sprintf(tmp, "%d", pl->GetPlaneWidth());
		tfpmPlaneSizeX->setText(tmp);
		sprintf(tmp, "%d", pl->GetPlaneHeight());
		tfpmPlaneSizeY->setText(tmp);

		labpmTileSize->setVisible(1);
		labpmTileSize_x->setVisible(1);
		labpmMovX->setVisible(1);
		labpmMovY->setVisible(1);
		tfpmTileSizeX->setVisible(1);
		tfpmTileSizeY->setVisible(1);

		sprintf(tmp, "%d", pl->GetTileWidth());
		tfpmTileSizeX->setText(tmp);
		sprintf(tmp, "%d", pl->GetTileHeight());
		tfpmTileSizeY->setText(tmp);

		butpmResC->setVisible(1);
		butpmResD->setVisible(1);
		butpmResDL->setVisible(1);
		butpmResDR->setVisible(1);
		butpmResL->setVisible(1);
		butpmResR->setVisible(1);
		butpmResU->setVisible(1);
		butpmResUL->setVisible(1);
		butpmResUR->setVisible(1);
		butpmResC->setEnabled(1);
		butpmResD->setEnabled(1);
		butpmResDL->setEnabled(1);
		butpmResDR->setEnabled(1);
		butpmResL->setEnabled(1);
		butpmResR->setEnabled(1);
		butpmResU->setEnabled(1);
		butpmResUL->setEnabled(1);
		butpmResUR->setEnabled(1);
		SetAnchorPlaneProperties(5);

		labpmAnchor->setVisible(1);
		labpmWidth->setVisible(1);
		labpmHeight->setVisible(1);
		labpmZCoord->setVisible(1);
		tfpmZCoord->setVisible(1);
		sprintf(tmp, "%d", pl->GetZCoord());
		tfpmZCoord->setText(tmp);
		labpmMovement->setVisible(1);
		tfpmMovX->setVisible(1);
		sprintf(tmp, "%d", pl->GetMoveModX());
		tfpmMovX->setText(tmp);
		sprintf(tmp, "%d", pl->GetMoveModY());
		tfpmMovY->setText(tmp);
		tfpmMovY->setVisible(1);
		labpmFlags->setVisible(1);
		cbpmFlagMainPlane->setVisible(1);
		cbpmFlagMainPlane->setSelected(pl->GetFlags() & WWD::Flag_p_MainPlane);
		cbpmFlagWrapX->setVisible(1);
		cbpmFlagWrapX->setSelected(pl->GetFlags() & WWD::Flag_p_XWrapping);
		cbpmFlagWrapY->setVisible(1);
		cbpmFlagWrapY->setSelected(pl->GetFlags() & WWD::Flag_p_YWrapping);
		cbpmFlagNoDraw->setVisible(1);
		cbpmFlagNoDraw->setSelected(pl->GetFlags() & WWD::Flag_p_NoDraw);
		cbpmAutoTileSize->setVisible(1);
		cbpmAutoTileSize->setSelected(pl->GetFlags() & WWD::Flag_p_AutoTileSize);
		butpmSave->setVisible(1);
		butpmDelete->setVisible(1);
		butpmDelete->setEnabled(1);
	}
	else {
		winpmMain->setWidth(225);
		labpmName->setVisible(0);
		tfpmName->setVisible(0);
		labpmTileSet->setVisible(0);
		ddpmTileSet->setVisible(0);
		labpmPlaneSize->setVisible(0);
		tfpmPlaneSizeX->setVisible(0);
		tfpmPlaneSizeY->setVisible(0);
		labpmTileSize->setVisible(0);
		labpmTileSize_x->setVisible(0);
		labpmMovX->setVisible(0);
		labpmMovY->setVisible(0);
		tfpmTileSizeX->setVisible(0);
		tfpmTileSizeY->setVisible(0);
		labpmAnchor->setVisible(0);
		butpmResC->setVisible(0);
		butpmResD->setVisible(0);
		butpmResDL->setVisible(0);
		butpmResDR->setVisible(0);
		butpmResL->setVisible(0);
		butpmResR->setVisible(0);
		butpmResU->setVisible(0);
		butpmResUL->setVisible(0);
		butpmResUR->setVisible(0);
		labpmWidth->setVisible(0);
		labpmHeight->setVisible(0);
		labpmZCoord->setVisible(0);
		tfpmZCoord->setVisible(0);
		labpmMovement->setVisible(0);
		tfpmMovX->setVisible(0);
		tfpmMovY->setVisible(0);
		labpmFlags->setVisible(0);
		cbpmFlagMainPlane->setVisible(0);
		cbpmFlagWrapX->setVisible(0);
		cbpmFlagWrapY->setVisible(0);
		cbpmFlagNoDraw->setVisible(0);
		cbpmAutoTileSize->setVisible(0);
		butpmSave->setVisible(0);
		butpmDelete->setVisible(0);
	}

	//lbpmPlanes->setHeight(hParser->GetPlanesCount()*19);
	//conpmPlanes->setHeight(hParser->GetPlanesCount()*19);
}

void State::EditingWW::SavePlaneProperties()
{
	WWD::Plane * pl = NULL;
	bool bAddingNew = (lbpmPlanes->getSelected() == hParser->GetPlanesCount());
	if (!bAddingNew)
		pl = hParser->GetPlane(lbpmPlanes->getSelected());
	for (int i = 0; i < hParser->GetPlanesCount(); i++) {
		if (!strcmp(tfpmName->getText().c_str(), hParser->GetPlane(i)->GetName()) && lbpmPlanes->getSelected() != i) {
			GV->StateMgr->Push(new State::Error(GETL(Lang_Message), GETL2S("PlaneProperties", "NameTaken"), ST_ER_ICON_WARNING, ST_ER_BUT_OK, 0));
			return;
		}
	}
	if (!bAddingNew && (pl->GetFlags() & WWD::Flag_p_MainPlane) && !cbpmFlagMainPlane->isSelected()) {
		GV->StateMgr->Push(new State::Error(GETL(Lang_Message), GETL2S("PlaneProperties", "MainPlaneUncheck"), ST_ER_ICON_WARNING, ST_ER_BUT_OK, 0));
		return;
	}
	if (tfpmName->getText().length() == 0) {
		GV->StateMgr->Push(new State::Error(GETL(Lang_Message), GETL2S("PlaneProperties", "NoPlaneName"), ST_ER_ICON_WARNING, ST_ER_BUT_OK, 0));
		return;
	}
	if (tfpmName->getText().length() > 64) {
		GV->StateMgr->Push(new State::Error(GETL(Lang_Message), GETL2S("PlaneProperties", "PlaneNameTooLong"), ST_ER_ICON_WARNING, ST_ER_BUT_OK, 0));
		return;
	}
	if (atoi(tfpmPlaneSizeX->getText().c_str()) <= 0 || atoi(tfpmPlaneSizeY->getText().c_str()) <= 0) {
		GV->StateMgr->Push(new State::Error(GETL(Lang_Message), GETL2S("PlaneProperties", "InvalidPlaneSize"), ST_ER_ICON_WARNING, ST_ER_BUT_OK, 0));
		return;
	}
	if (atoi(tfpmMovX->getText().c_str()) < 0 || atoi(tfpmMovY->getText().c_str()) < 0) {
		GV->StateMgr->Push(new State::Error(GETL(Lang_Message), GETL2S("PlaneProperties", "InvalidMoveMod"), ST_ER_ICON_WARNING, ST_ER_BUT_OK, 0));
		return;
	}
	if (atoi(tfpmTileSizeX->getText().c_str()) <= 0 || atoi(tfpmTileSizeY->getText().c_str()) <= 0) {
		GV->StateMgr->Push(new State::Error(GETL(Lang_Message), GETL2S("PlaneProperties", "InvalidTileSize"), ST_ER_ICON_WARNING, ST_ER_BUT_OK, 0));
		return;
	}
	bool bChange = 0, bZCoordchange = 0, bDimChange = 0;
	if (bAddingNew) {
		bChange = 1;
		pl = new WWD::Plane();
		pl->SetName(tfpmName->getText().c_str());
		pl->SetMoveModX(atoi(tfpmMovX->getText().c_str()));
		pl->SetMoveModY(atoi(tfpmMovY->getText().c_str()));
		pl->SetTileWidth(atoi(tfpmTileSizeX->getText().c_str()));
		pl->SetTileHeight(atoi(tfpmTileSizeY->getText().c_str()));
		pl->SetZCoord(atoi(tfpmZCoord->getText().c_str()));
		pl->SetFlag(WWD::Flag_p_NoDraw, cbpmFlagNoDraw->isSelected());
		pl->SetFlag(WWD::Flag_p_XWrapping, cbpmFlagWrapX->isSelected());
		pl->SetFlag(WWD::Flag_p_YWrapping, cbpmFlagWrapY->isSelected());
		pl->SetFlag(WWD::Flag_p_AutoTileSize, cbpmAutoTileSize->isSelected());
		pl->SetFlag(WWD::Flag_p_MainPlane, 0);
		pl->Resize(atoi(tfpmPlaneSizeX->getText().c_str()), atoi(tfpmPlaneSizeY->getText().c_str()));
		pl->SetFillColor(0);
		pl->AddImageSet(ddpmTileSet->getListModel()->getElementAt(ddpmTileSet->getSelected()).c_str());
		hParser->AddPlane(pl);
		bZCoordchange = 1;
		bDimChange = 1;

		PlaneData * pd = new PlaneData();
		pd->bDraw = 1;
		pd->bDrawGrid = 0;
		pd->bDrawBoundary = 0;
		pd->bDrawObjects = 1;
		pd->ObjectData.hQuadTree = NULL;
		pd->ObjectData.bEmpty = 1;
		pd->hRB = NULL;
		hPlaneData.push_back(pd);
	}
	else {
		if (strcmp(tfpmName->getText().c_str(), pl->GetName()) != 0) {
			bChange = 1;
			pl->SetName(tfpmName->getText().c_str());
		}

		if (atoi(tfpmPlaneSizeX->getText().c_str()) != pl->GetPlaneWidth() ||
			atoi(tfpmPlaneSizeY->getText().c_str()) != pl->GetPlaneHeight()) {
			bChange = 1;
			bDimChange = 1;
			pl->ResizeAnchor(atoi(tfpmPlaneSizeX->getText().c_str()), atoi(tfpmPlaneSizeY->getText().c_str()), ipmAnchor);
		}

		bool bischange = 0;
		if (strcmp(ddpmTileSet->getListModel()->getElementAt(ddpmTileSet->getSelected()).c_str(), pl->GetImageSet(0)) != 0) {
			pl->ClearImageSets();
			pl->AddImageSet(ddpmTileSet->getListModel()->getElementAt(ddpmTileSet->getSelected()).c_str());
			bChange = 1;
		}

		if (atoi(tfpmMovX->getText().c_str()) != pl->GetMoveModX()) {
			bChange = 1;
			pl->SetMoveModX(atoi(tfpmMovX->getText().c_str()));
		}
		if (atoi(tfpmMovY->getText().c_str()) != pl->GetMoveModY()) {
			bChange = 1;
			pl->SetMoveModY(atoi(tfpmMovY->getText().c_str()));
		}
		if (atoi(tfpmTileSizeX->getText().c_str()) != pl->GetTileWidth()) {
			bChange = 1;
			pl->SetTileWidth(atoi(tfpmTileSizeX->getText().c_str()));
		}
		if (atoi(tfpmTileSizeY->getText().c_str()) != pl->GetTileHeight()) {
			bChange = 1;
			pl->SetTileHeight(atoi(tfpmTileSizeY->getText().c_str()));
		}
		if (atoi(tfpmZCoord->getText().c_str()) != pl->GetZCoord()) {
			bChange = 1;
			bZCoordchange = 1;
			pl->SetZCoord(atoi(tfpmZCoord->getText().c_str()));
		}
		if (cbpmFlagNoDraw->isSelected() != pl->GetFlag(WWD::Flag_p_NoDraw)) {
			bChange = 1;
			pl->SetFlag(WWD::Flag_p_NoDraw, cbpmFlagNoDraw->isSelected());
		}
		if (cbpmFlagWrapX->isSelected() != pl->GetFlag(WWD::Flag_p_XWrapping)) {
			bChange = 1;
			pl->SetFlag(WWD::Flag_p_XWrapping, cbpmFlagWrapX->isSelected());
		}
		if (cbpmFlagWrapY->isSelected() != pl->GetFlag(WWD::Flag_p_YWrapping)) {
			bChange = 1;
			pl->SetFlag(WWD::Flag_p_YWrapping, cbpmFlagWrapY->isSelected());
		}
		if (cbpmAutoTileSize->isSelected() != pl->GetFlag(WWD::Flag_p_AutoTileSize)) {
			bChange = 1;
			pl->SetFlag(WWD::Flag_p_AutoTileSize, cbpmAutoTileSize->isSelected());
		}
	}

	if (bDimChange) {
		if (pl->GetPlaneWidthPx() <= 2048 && pl->GetPlaneHeightPx() <= 2048) {
			GV->Console->Printf("Creating buffer for plane '~y~%s~w~' (%dx%d).", pl->GetName(), pl->GetPlaneWidthPx(), pl->GetPlaneHeightPx());
			if (hPlaneData[lbpmPlanes->getSelected()]->hRB != NULL)
				delete hPlaneData[lbpmPlanes->getSelected()]->hRB;
			hPlaneData[lbpmPlanes->getSelected()]->hRB = new State::cLayerRenderBuffer(this, vPort, pl);
		}
		else {
			if (hPlaneData[lbpmPlanes->getSelected()]->hRB != NULL) {
				delete hPlaneData[lbpmPlanes->getSelected()]->hRB;
				GV->Console->Printf("Deleting buffer for plane '~y~%s~w~' (%dx%d).", pl->GetName(), pl->GetPlaneWidthPx(), pl->GetPlaneHeightPx());
			}
			hPlaneData[lbpmPlanes->getSelected()]->hRB = NULL;
		}
	}

	if (cbpmFlagMainPlane->isSelected() != pl->GetFlag(WWD::Flag_p_MainPlane)) {
		bool bDo = 1;
		for (int i = 0; i < hParser->GetPlanesCount(); i++) {
			if (hParser->GetPlane(i)->GetFlag(WWD::Flag_p_MainPlane)) {
				if (hParser->GetPlane(i)->GetObjectsCount() > 0) {
					//int ret = ErrorStandalone(GETL(Lang_Message), GETL2S("PlaneProperties", "MainPlaneObjectsDrop"), ST_ER_ICON_WARNING, ST_ER_BUT_OKCANCEL, 1);
					char tmp[256];
					sprintf(tmp, GETL2S("PlaneProperties", "MainPlaneObjectsDrop"), hParser->GetPlane(i)->GetName(), hParser->GetPlane(i)->GetObjectsCount());
					int ret = MessageBox(hge->System_GetState(HGE_HWND), tmp, GETL(Lang_Message), MB_ICONWARNING | MB_OKCANCEL);
					bDo = (ret == IDOK);
				}
				if (bDo) {
					hParser->GetPlane(i)->SetFlag(WWD::Flag_p_MainPlane, 0);
					while (hParser->GetPlane(i)->GetObjectsCount() > 0)
						hParser->GetPlane(i)->DeleteObjectByIterator(0);
					hPlaneData[i]->ObjectData.bEmpty = 1;
					delete hPlaneData[i]->ObjectData.hQuadTree;
					hPlaneData[i]->ObjectData.hQuadTree = NULL;
				}
				break;
			}
		}
		if (bDo) {
			bChange = 1;
			pl->SetFlag(WWD::Flag_p_MainPlane, 1);
			plMain = pl;
			hPlaneData[lbpmPlanes->getSelected()]->ObjectData.bEmpty = 0;
			hPlaneData[lbpmPlanes->getSelected()]->ObjectData.hQuadTree = new cObjectQuadTree(pl, SprBank);
			if (hParser->GetStartX() > pl->GetPlaneWidthPx() || hParser->GetStartY() > pl->GetPlaneHeightPx()) {
				char tmp[256];
				sprintf(tmp, GETL2S("PlaneProperties", "StartPosMoved"), hParser->GetStartX(), hParser->GetStartY(), pl->GetPlaneWidthPx() / 2,
					pl->GetPlaneHeightPx() / 2);
				hParser->SetStartX(pl->GetPlaneWidthPx() / 2);
				hParser->SetStartY(pl->GetPlaneHeightPx() / 2);
				MessageBox(hge->System_GetState(HGE_HWND), tmp, GETL(Lang_Message), MB_ICONINFORMATION | MB_OK);
			}
			//delete hStartingPosObj;
			hStartingPosObj = new WWD::Object();
			hStartingPosObj->SetLogic("_WM_STARTPOS");
			if (hParser->GetGame() == WWD::Game_Gruntz)
				hStartingPosObj->SetImageSet("GAME_GRUNTSELECTEDSPRITE");
			else if (hParser->GetGame() == WWD::Game_Claw) {
				hStartingPosObj->SetImageSet("CLAW");
				hStartingPosObj->SetParam(WWD::Param_LocationI, 13);
				hStartingPosObj->SetParam(WWD::Param_LocationZ, 4000);
			}
			else {
				hStartingPosObj->SetImageSet("GAME_CONFIGCONTROLS");
				hStartingPosObj->SetParam(WWD::Param_LocationI, 3);
			}
			hStartingPosObj->SetParam(WWD::Param_LocationX, hParser->GetStartX());
			hStartingPosObj->SetParam(WWD::Param_LocationY, hParser->GetStartY());
			hStartingPosObj->SetUserData(new cObjUserData(hStartingPosObj));
			hPlaneData[lbpmPlanes->getSelected()]->ObjectData.hQuadTree->UpdateObject(hStartingPosObj);
		}
	}

	if (bZCoordchange) {
		std::string strSelectedPl = ddActivePlane->getListModel()->getElementAt(ddActivePlane->getSelected());

		int iNewPlIt = lbpmPlanes->getSelected();
		std::string strNewPl = tfpmName->getText();

		hParser->SortPlanes();

		for (int i = 0; i < hParser->GetPlanesCount(); i++) {
			if (!strcmp(hParser->GetPlane(i)->GetName(), strSelectedPl.c_str())) {
				ddActivePlane->setSelected(i);
			}

			if (!strcmp(hParser->GetPlane(i)->GetName(), strNewPl.c_str())) {
				PlaneData * buf = hPlaneData[iNewPlIt];
				hPlaneData.erase(hPlaneData.begin() + iNewPlIt);
				hPlaneData.insert(hPlaneData.begin() + i, buf);
				lbpmPlanes->setSelected(i);
			}
		}
	}

	if (bChange) {
		SyncPlaneProperties();
		MarkUnsaved();
	}
}

void State::EditingWW::DeletePlaneProperties()
{
	if (hParser->GetPlane(lbpmPlanes->getSelected())->GetFlags() & WWD::Flag_p_MainPlane) {
		GV->StateMgr->Push(new State::Error(GETL(Lang_Message), GETL2S("PlaneProperties", "MainPlaneDelete"), ST_ER_ICON_WARNING, ST_ER_BUT_OK, 0));
		return;
	}
	int is = lbpmPlanes->getSelected();
	if (is <= ddActivePlane->getSelected()) {
		if (ddActivePlane->getSelected() > 0)
			ddActivePlane->setSelected(ddActivePlane->getSelected() - 1);
		else
			ddActivePlane->setSelected(ddActivePlane->getSelected() + 1);
	}
	if (!hPlaneData[is]->ObjectData.bEmpty) {
		for (int z = 0; z < hParser->GetPlane(is)->GetObjectsCount(); z++)
			delete GetUserDataFromObj(hParser->GetPlane(is)->GetObjectByIterator(z));
		delete hPlaneData[is]->ObjectData.hQuadTree;
	}
	delete hPlaneData[is]->hRB;
	delete hPlaneData[is];
	hPlaneData.erase(hPlaneData.begin() + is);
	hParser->DeletePlane(is);
	MarkUnsaved();
	vPort->MarkToRedraw(1);
	SyncPlaneProperties();
}

void State::EditingWW::SetAnchorPlaneProperties(int anchor)
{
	bool bXb, bYb;
	if (lbpmPlanes->getSelected() == hParser->GetPlanesCount()) {
		bXb = bYb = 1;
	}
	else {
		bXb = atoi(tfpmPlaneSizeX->getText().c_str()) >= hParser->GetPlane(lbpmPlanes->getSelected())->GetPlaneWidth();
		bYb = atoi(tfpmPlaneSizeY->getText().c_str()) >= hParser->GetPlane(lbpmPlanes->getSelected())->GetPlaneHeight();
	}
	ipmAnchor = anchor;
	if (anchor == 1) {
		butpmResUL->setIcon(GV->sprIcons[Icon_Anchor]);
		butpmResU->setIcon(bXb ? GV->sprIcons[Icon_Right] : GV->sprIcons[Icon_Left]);
		butpmResUR->setIcon(NULL);

		butpmResL->setIcon(bYb ? GV->sprIcons[Icon_Down] : GV->sprIcons[Icon_Up]);
		butpmResC->setIcon(bXb&&bYb ? GV->sprIcons[Icon_DownRight] : GV->sprIcons[Icon_UpLeft]);
		butpmResR->setIcon(NULL);

		butpmResDL->setIcon(NULL);
		butpmResD->setIcon(NULL);
		butpmResDR->setIcon(NULL);
	}
	else if (anchor == 2) {
		butpmResUL->setIcon(bXb ? GV->sprIcons[Icon_Left] : GV->sprIcons[Icon_Right]);
		butpmResU->setIcon(GV->sprIcons[Icon_Anchor]);
		butpmResUR->setIcon(bXb ? GV->sprIcons[Icon_Right] : GV->sprIcons[Icon_Left]);

		butpmResL->setIcon(bXb&&bYb ? GV->sprIcons[Icon_DownLeft] : GV->sprIcons[Icon_UpRight]);
		butpmResC->setIcon(bYb ? GV->sprIcons[Icon_Down] : GV->sprIcons[Icon_Up]);
		butpmResR->setIcon(bXb&&bYb ? GV->sprIcons[Icon_DownRight] : GV->sprIcons[Icon_UpLeft]);

		butpmResDL->setIcon(NULL);
		butpmResD->setIcon(NULL);
		butpmResDR->setIcon(NULL);
	}
	else if (anchor == 3) {
		butpmResUL->setIcon(NULL);
		butpmResU->setIcon(bXb ? GV->sprIcons[Icon_Left] : GV->sprIcons[Icon_Right]);
		butpmResUR->setIcon(GV->sprIcons[Icon_Anchor]);

		butpmResL->setIcon(NULL);
		butpmResC->setIcon(bXb&&bYb ? GV->sprIcons[Icon_DownLeft] : GV->sprIcons[Icon_UpRight]);
		butpmResR->setIcon(bYb ? GV->sprIcons[Icon_Down] : GV->sprIcons[Icon_Up]);

		butpmResDL->setIcon(NULL);
		butpmResD->setIcon(NULL);
		butpmResDR->setIcon(NULL);

	}
	else if (anchor == 4) {
		butpmResUL->setIcon(bYb ? GV->sprIcons[Icon_Up] : GV->sprIcons[Icon_Down]);
		butpmResU->setIcon(bXb&&bYb ? GV->sprIcons[Icon_UpRight] : GV->sprIcons[Icon_DownLeft]);
		butpmResUR->setIcon(NULL);

		butpmResL->setIcon(GV->sprIcons[Icon_Anchor]);
		butpmResC->setIcon(bXb ? GV->sprIcons[Icon_Right] : GV->sprIcons[Icon_Left]);
		butpmResR->setIcon(NULL);

		butpmResDL->setIcon(bYb ? GV->sprIcons[Icon_Down] : GV->sprIcons[Icon_Up]);
		butpmResD->setIcon(bXb&&bYb ? GV->sprIcons[Icon_DownRight] : GV->sprIcons[Icon_UpLeft]);
		butpmResDR->setIcon(NULL);
	}
	else if (anchor == 5) {
		butpmResUL->setIcon(bXb&&bYb ? GV->sprIcons[Icon_UpLeft] : GV->sprIcons[Icon_DownRight]);
		butpmResU->setIcon(bYb ? GV->sprIcons[Icon_Up] : GV->sprIcons[Icon_Down]);
		butpmResUR->setIcon(bXb&&bYb ? GV->sprIcons[Icon_UpRight] : GV->sprIcons[Icon_DownLeft]);

		butpmResL->setIcon(bXb ? GV->sprIcons[Icon_Left] : GV->sprIcons[Icon_Right]);
		butpmResC->setIcon(GV->sprIcons[Icon_Anchor]);
		butpmResR->setIcon(bXb ? GV->sprIcons[Icon_Right] : GV->sprIcons[Icon_Left]);

		butpmResDL->setIcon(bXb&&bYb ? GV->sprIcons[Icon_DownLeft] : GV->sprIcons[Icon_UpRight]);
		butpmResD->setIcon(bYb ? GV->sprIcons[Icon_Down] : GV->sprIcons[Icon_Up]);
		butpmResDR->setIcon(bXb&&bYb ? GV->sprIcons[Icon_DownRight] : GV->sprIcons[Icon_UpLeft]);
	}
	else if (anchor == 6) {
		butpmResUL->setIcon(NULL);
		butpmResU->setIcon(bXb&&bYb ? GV->sprIcons[Icon_UpLeft] : GV->sprIcons[Icon_DownRight]);
		butpmResUR->setIcon(bYb ? GV->sprIcons[Icon_Up] : GV->sprIcons[Icon_Down]);

		butpmResL->setIcon(NULL);
		butpmResC->setIcon(bXb ? GV->sprIcons[Icon_Left] : GV->sprIcons[Icon_Right]);
		butpmResR->setIcon(GV->sprIcons[Icon_Anchor]);

		butpmResDL->setIcon(NULL);
		butpmResD->setIcon(bXb&&bYb ? GV->sprIcons[Icon_DownLeft] : GV->sprIcons[Icon_UpRight]);
		butpmResDR->setIcon(bYb ? GV->sprIcons[Icon_Down] : GV->sprIcons[Icon_Up]);

	}
	else if (anchor == 7) {
		butpmResUL->setIcon(NULL);
		butpmResU->setIcon(NULL);
		butpmResUR->setIcon(NULL);

		butpmResL->setIcon(bYb ? GV->sprIcons[Icon_Up] : GV->sprIcons[Icon_Down]);
		butpmResC->setIcon(bXb&&bYb ? GV->sprIcons[Icon_UpRight] : GV->sprIcons[Icon_DownLeft]);
		butpmResR->setIcon(NULL);

		butpmResDL->setIcon(GV->sprIcons[Icon_Anchor]);
		butpmResD->setIcon(bXb ? GV->sprIcons[Icon_Right] : GV->sprIcons[Icon_Left]);
		butpmResDR->setIcon(NULL);
	}
	else if (anchor == 8) {
		butpmResUL->setIcon(NULL);
		butpmResU->setIcon(NULL);
		butpmResUR->setIcon(NULL);

		butpmResL->setIcon(bXb&&bYb ? GV->sprIcons[Icon_UpLeft] : GV->sprIcons[Icon_DownRight]);
		butpmResC->setIcon(bYb ? GV->sprIcons[Icon_Up] : GV->sprIcons[Icon_Down]);
		butpmResR->setIcon(bXb&&bYb ? GV->sprIcons[Icon_UpRight] : GV->sprIcons[Icon_DownLeft]);

		butpmResDL->setIcon(bXb ? GV->sprIcons[Icon_Left] : GV->sprIcons[Icon_Right]);
		butpmResD->setIcon(GV->sprIcons[Icon_Anchor]);
		butpmResDR->setIcon(bXb ? GV->sprIcons[Icon_Right] : GV->sprIcons[Icon_Left]);
	}
	else if (anchor == 9) {
		butpmResUL->setIcon(NULL);
		butpmResU->setIcon(NULL);
		butpmResUR->setIcon(NULL);

		butpmResL->setIcon(NULL);
		butpmResC->setIcon(bXb&&bYb ? GV->sprIcons[Icon_UpLeft] : GV->sprIcons[Icon_DownRight]);
		butpmResR->setIcon(bYb ? GV->sprIcons[Icon_Up] : GV->sprIcons[Icon_Down]);

		butpmResDL->setIcon(NULL);
		butpmResD->setIcon(bXb ? GV->sprIcons[Icon_Left] : GV->sprIcons[Icon_Right]);
		butpmResDR->setIcon(GV->sprIcons[Icon_Anchor]);
	}
}
