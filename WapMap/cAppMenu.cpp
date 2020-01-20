#include "cAppMenu.h"
#include "states/editing_ww.h"
#include "../shared/commonFunc.h"
#include "langID.h"
#include "states/error.h"
#include "cNativeController.h"
#include "states/stats.h"
#include "states/loadmap.h"
#include "states/codeEditor.h"
#include "windows/tileBrowser.h"
#include "windows/imgsetBrowser.h"
#include "windows/options.h"
#include "windows/about.h"

#include "databanks/logics.h"

extern HGE * hge;

cAppMenu_Entry::cAppMenu_Entry(std::string lab, Gfx16Icons ico)
{
	strLabel = lab;
	iIcon = ico;
	bEnabled = 1;
	hContext = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
	hContext->hide();
	GV->editState->conMain->add(hContext, 400, 400);
	bFocused = 0;
	fTimer = 0;
}

cAppMenu::cAppMenu()
{
	//bFolded = 1;
	bEnabled = 1;
	iHeight = LAY_VIEWPORT_Y;
	iOpened = -1;
	hRulers = new cRulers();

	hEntries[AppMenu_File] = new cAppMenu_Entry(GETL2S("AppMenu", "Main_File"), Icon16_New);
	hEntries[AppMenu_Edit] = new cAppMenu_Entry(GETL2S("AppMenu", "Main_Edit"), Icon16_Pencil);
	hEntries[AppMenu_Edit]->SetEnabled(0);
	hEntries[AppMenu_Plane] = new cAppMenu_Entry(GETL2S("AppMenu", "Main_Planes"), Icon16_Planes);
	hEntries[AppMenu_Plane]->SetEnabled(0);
	hEntries[AppMenu_View] = new cAppMenu_Entry(GETL2S("AppMenu", "Main_View"), Icon16_View);
	hEntries[AppMenu_View]->SetEnabled(0);
	hEntries[AppMenu_Tools] = new cAppMenu_Entry(GETL2S("AppMenu", "Main_Tools"), Icon16_Tools);
	hEntries[AppMenu_Tools]->SetEnabled(0);
	hEntries[AppMenu_Assets] = new cAppMenu_Entry(GETL2S("AppMenu", "Main_Assets"), Icon16_Database);
	hEntries[AppMenu_Assets]->SetEnabled(0);
	hEntries[AppMenu_WapMap] = new cAppMenu_Entry(GETL2S("AppMenu", "Main_WapMap"), Icon16_Settings);
	//hEntries[AppMenu_Fold] = new cAppMenu_Entry(GETL2S("AppMenu", "Main_Unfold"), Icon16_Down);

	for (int i = 0; i < AppMenu_EntryCount; i++) {
		hEntries[i]->GetContext()->addActionListener(this);
	}

	SHR::Context * workcon = hEntries[AppMenu_File]->GetContext();
	workcon->AddElement(APPMEN_FILE_NEW, GETL2S("AppMenu", "File_New"), GV->sprIcons16[Icon16_New]);
	workcon->AddElement(APPMEN_FILE_OPEN, GETL2S("AppMenu", "File_Open"), GV->sprIcons16[Icon16_Open]);
	workcon->AddElement(APPMEN_FILE_SAVE, GETL2S("AppMenu", "File_Save"), GV->sprIcons16[Icon16_Save]);
	workcon->AddElement(APPMEN_FILE_SAVEAS, GETL2S("AppMenu", "File_SaveAs"), GV->sprIcons16[Icon16_SaveAs]);
	workcon->AddElement(APPMEN_FILE_CLOSE, GETL2S("AppMenu", "File_Close"), GV->sprIcons16[Icon16_XTransparent]);
	workcon->AddElement(APPMEN_FILE_CLOSEALL, GETL2S("AppMenu", "File_CloseAll"), GV->sprIcons16[Icon16_X]);
	workcon->adjustSize();

	conOpen = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
	conOpen->hide();
	GV->editState->conMain->add(conOpen, 400, 400);
	workcon->GetElementByID(APPMEN_FILE_OPEN)->SetCascade(conOpen);
	conOpen->AddElement(APPMEN_FILE_MRU, GETL2S("AppMenu", "File_Open_RecentlyUsed"), NULL);
	conOpen->AddElement(APPMEN_FILE_CLOSED, GETL2S("AppMenu", "File_Open_RecentlyClosed"), NULL);
	conOpen->GetElementByID(APPMEN_FILE_CLOSED)->SetEnabled(0);
	conOpen->addActionListener(this);
	conOpen->adjustSize();

	conOpenMRU = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
	conOpenMRU->hide();
	conOpenMRU->addActionListener(this);
	GV->editState->conMain->add(conOpenMRU, 400, 400);

	SyncDocumentClosed();

	workcon = hEntries[AppMenu_Edit]->GetContext();
	workcon->AddElement(APPMEN_EDIT_WORLD, GETL2S("AppMenu", "Edit_World"), GV->sprIcons16[Icon16_World]);
	workcon->AddElement(APPMEN_EDIT_TILEPROP, GETL2S("AppMenu", "Edit_TileProp"), GV->sprIcons16[Icon16_Properties]);
	workcon->AddElement(APPMEN_EDIT_WORLDSCRIPT, GETL2S("AppMenu", "Edit_GlobalScript"), GV->sprIcons16[Icon16_Code]);
	workcon->adjustSize();

	conPlaneVisibility = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
	conPlaneVisibility->hide();
	conPlaneVisibility->addActionListener(this);
	GV->editState->conMain->add(conPlaneVisibility, 400, 400);

	conPlaneVisibility->AddElement(APPMEN_PLANEVIS_BORDER, GETL2S("AppMenu", "PlaneDraw_Border"), GV->sprIcons16[Icon16_Boundary]);
	conPlaneVisibility->AddElement(APPMEN_PLANEVIS_GRID, GETL2S("AppMenu", "PlaneDraw_Grid"), GV->sprIcons16[Icon16_Grid]);
	conPlaneVisibility->AddElement(APPMEN_PLANEVIS_OBJECTS, GETL2S("AppMenu", "PlaneDraw_Objects"), GV->sprIcons16[Icon16_ModeObject]);
	conPlaneVisibility->ReserveIconSpace(1, 1);
	conPlaneVisibility->adjustSize();

	conPlanesVisibilityList = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
	conPlanesVisibilityList->hide();
	conPlanesVisibilityList->addActionListener(this);
	conPlanesVisibilityList->addSelectionListener(this);
	GV->editState->conMain->add(conPlanesVisibilityList, 400, 400);

	conActivePlane = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
	conActivePlane->hide();
	conActivePlane->addActionListener(this);
	GV->editState->conMain->add(conActivePlane, 400, 400);

	workcon = hEntries[AppMenu_Plane]->GetContext();
	workcon->AddElement(APPMEN_PLANE_MGR, GETL2S("AppMenu", "Plane_Mgr"), GV->sprIcons16[Icon16_Planes]);
	workcon->AddElement(APPMEN_PLANE_ACTIVE, GETL2S("AppMenu", "Plane_Active"), GV->sprIcons16[Icon16_PlaneAction]);
	workcon->GetElementByID(APPMEN_PLANE_ACTIVE)->SetCascade(conActivePlane);
	workcon->adjustSize();

	workcon = hEntries[AppMenu_View]->GetContext();
	workcon->AddElement(APPMEN_VIEW_PLANES, GETL2S("AppMenu", "View_PlaneVisibility"), GV->sprIcons16[Icon16_Planes]);
	workcon->AddElement(APPMEN_VIEW_RULERS, GETL2S("AppMenu", "View_Rulers"), GV->sprIcons16[Icon16_Measure]);
	workcon->AddElement(APPMEN_VIEW_GUIDELINES, GETL2S("AppMenu", "View_GuideLines"), GV->sprIcons16[Icon16_Grid]);
	hEntries[AppMenu_View]->GetContext()->GetElementByID(APPMEN_VIEW_GUIDELINES)->SetIcon(GV->sprIcons16[Icon16_Applied], 1);
	workcon->AddElement(APPMEN_VIEW_TILEPROP, GETL2S("AppMenu", "View_TileProp"), GV->sprIcons16[Icon16_Properties]);
	workcon->GetElementByID(APPMEN_VIEW_PLANES)->SetCascade(conPlanesVisibilityList);
	workcon->adjustSize();

	workcon = hEntries[AppMenu_Tools]->GetContext();
	workcon->AddElement(APPMEN_TOOLS_MAPSHOT, GETL(Lang_MapShot), GV->sprIcons16[Icon16_Mapshot]);
	workcon->AddElement(APPMEN_TOOLS_PLAY, GETL2S("AppMenu", "Tools_Play"), GV->sprIcons16[Icon16_Play]);
	workcon->AddElement(APPMEN_TOOLS_MEASURE, GETL2S("AppMenu", "Tools_Measure"), GV->sprIcons16[Icon16_Measure]);
	workcon->AddElement(APPMEN_TOOLS_STATS, GETL2S("AppMenu", "Tools_Stats"), GV->sprIcons16[Icon16_Stats]);
	workcon->adjustSize();

	workcon = hEntries[AppMenu_Assets]->GetContext();
	workcon->AddElement(APPMEN_ASSETS_TILES, GETL2S("AppMenu", "Assets_Tiles"), GV->sprIcons16[Icon16_ModeTile]);
	workcon->AddElement(APPMEN_ASSETS_IMAGESETS, GETL2S("AppMenu", "Assets_ImgSets"), GV->sprIcons16[Icon16_Flip]);
	workcon->AddElement(APPMEN_ASSETS_ANIMS, GETL2S("AppMenu", "Assets_Anims"), GV->sprIcons16[Icon16_Animation]);
	workcon->AddElement(APPMEN_ASSETS_SOUNDS, GETL2S("AppMenu", "Assets_Sounds"), GV->sprIcons16[Icon16_Sound]);
	workcon->AddElement(APPMEN_ASSETS_LOGICS, GETL2S("AppMenu", "Assets_Logics"), GV->sprIcons16[Icon16_Code]);
	workcon->adjustSize();

	workcon = hEntries[AppMenu_WapMap]->GetContext();
	workcon->AddElement(APPMEN_WM_SETTINGS, GETL2S("AppMenu", "About_Settings"), GV->sprIcons16[Icon16_Settings]);
	workcon->AddElement(APPMEN_WM_ABOUT, GETL2S("AppMenu", "About_About"), GV->sprIcons16[Icon16_Info]);
	workcon->AddElement(APPMEN_WM_UPDATE, GETL2S("AppMenu", "About_Update"), GV->sprIcons16[Icon16_AutoUpdate]);
	workcon->GetElementByID(APPMEN_WM_UPDATE)->SetEnabled(0);
	workcon->AddElement(APPMEN_WM_README, GETL2S("AppMenu", "About_Readme"), GV->sprIcons16[Icon16_Readme]);
	workcon->GetElementByID(APPMEN_WM_README)->SetEnabled(0);
	workcon->AddElement(APPMEN_WM_SITE, GETL2S("AppMenu", "About_Website"), GV->sprIcons16[Icon16_RSS]);
	workcon->GetElementByID(APPMEN_WM_SITE)->SetEnabled(0);
	workcon->adjustSize();

	iOverallWidth = 0;
	for (int i = 0; i < AppMenu_EntryCount; i++)
		iOverallWidth += GV->fntMyriad13->GetStringWidth(hEntries[i]->GetLabel().c_str()) + 26;
}

cAppMenu::~cAppMenu()
{

}

void cAppMenu::SyncDocumentSwitched()
{
	if (GV->editState->hParser == NULL) return;
	SyncPlanes();
	hEntries[AppMenu_Tools]->GetContext()->GetElementByID(APPMEN_TOOLS_PLAY)->SetEnabled(strlen(GV->editState->hParser->GetFilePath()) > 0);
}

void cAppMenu::SyncDocumentClosed()
{
	hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_CLOSE)->SetEnabled(GV->editState->MDI->GetDocsCount() >= 1);
	hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_SAVE)->SetEnabled(GV->editState->MDI->GetDocsCount() >= 1);
	hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_SAVEAS)->SetEnabled(GV->editState->MDI->GetDocsCount() >= 1);
	hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_CLOSEALL)->SetEnabled(GV->editState->MDI->GetDocsCount() > 1);

	if (GV->editState->MDI->GetCachedClosedDocsCount() > 0) {
		conOpen->GetElementByID(APPMEN_FILE_CLOSED)->SetEnabled(1);
		conOpen->GetElementByID(APPMEN_FILE_CLOSED)->SetCascade(GV->editState->MDI->GetClosedContext());
		hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_OPEN)->SetCascade(conOpen);
		conOpen->adjustSize();
		conOpen->hide();
	}

	if (GV->editState->MDI->GetDocsCount() == 0) {
		hEntries[AppMenu_Edit]->SetEnabled(0);
		hEntries[AppMenu_Plane]->SetEnabled(0);
		hEntries[AppMenu_View]->SetEnabled(0);
		hEntries[AppMenu_Tools]->SetEnabled(0);
		hEntries[AppMenu_Assets]->SetEnabled(0);
	}
}

void cAppMenu::SyncDocumentOpened()
{
	hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_CLOSE)->SetEnabled(1);
	hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_CLOSEALL)->SetEnabled(GV->editState->MDI->GetDocsCount() > 1);
	hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_SAVE)->SetEnabled(1);
	hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_SAVEAS)->SetEnabled(1);
	SyncMRU();
	hEntries[AppMenu_Edit]->SetEnabled(1);
	hEntries[AppMenu_Plane]->SetEnabled(1);
	hEntries[AppMenu_View]->SetEnabled(1);
	hEntries[AppMenu_Tools]->SetEnabled(1);
	hEntries[AppMenu_Assets]->SetEnabled(1);
}

void cAppMenu::SyncMRU()
{
	if (GV->editState->hMruList->IsValid() && GV->editState->hMruList->GetFilesCount() > 0) {
		conOpen->GetElementByID(APPMEN_FILE_MRU)->SetEnabled(1);
		conOpen->GetElementByID(APPMEN_FILE_MRU)->SetCascade(conOpenMRU);
		conOpenMRU->ClearElements();
		for (int i = 0; i < GV->editState->hMruList->GetFilesCount(); i++) {
			char * lab = SHR::GetFile(GV->editState->hMruList->GetRecentlyUsedFile(i));
			hgeSprite * ico;
			if (GV->editState->gamesLastOpened[i] < 50)
				ico = GV->sprGamesSmall[GV->editState->gamesLastOpened[i]];
			else
				ico = GV->sprLevelsMicro16[GV->editState->gamesLastOpened[i] - 51];
			conOpenMRU->AddElement(i, lab, ico);
			delete[] lab;
		}
		conOpenMRU->adjustSize();
	}
	else {
		conOpen->GetElementByID(APPMEN_FILE_MRU)->SetEnabled(0);
		conOpen->GetElementByID(APPMEN_FILE_MRU)->SetCascade(NULL);
		hEntries[AppMenu_File]->GetContext()->GetElementByID(APPMEN_FILE_OPEN)->SetCascade(GV->editState->MDI->GetCachedClosedDocsCount() > 0
			? conOpen
			: 0);
	}
	conOpen->adjustSize();
}

void cAppMenu::SyncPlaneSelectedVisibility()
{
	int sel = conPlanesVisibilityList->GetSelectedID();
	if (sel == -1) return;
	bool objs = GV->editState->hParser->GetPlane(sel)->GetFlags() & WWD::Flag_p_MainPlane;
	conPlaneVisibility->GetElementByID(APPMEN_PLANEVIS_OBJECTS)->SetEnabled(objs);
	if (objs)
		conPlaneVisibility->GetElementByID(APPMEN_PLANEVIS_OBJECTS)->SetIcon((GV->editState->hPlaneData[sel]->bDrawObjects ? GV->sprIcons16[Icon16_Applied] : NULL), 1);
	else
		conPlaneVisibility->GetElementByID(APPMEN_PLANEVIS_OBJECTS)->SetIcon(NULL, 1);
	conPlaneVisibility->GetElementByID(APPMEN_PLANEVIS_BORDER)->SetIcon((GV->editState->hPlaneData[sel]->bDrawBoundary ? GV->sprIcons16[Icon16_Applied] : NULL), 1);
	conPlaneVisibility->GetElementByID(APPMEN_PLANEVIS_GRID)->SetIcon((GV->editState->hPlaneData[sel]->bDrawGrid ? GV->sprIcons16[Icon16_Applied] : NULL), 1);
}

void cAppMenu::SyncPlaneVisibility()
{
	for (int i = 0; i < GV->editState->hParser->GetPlanesCount(); i++) {
		bool draw = GV->editState->hPlaneData[i]->bDraw;
		conPlanesVisibilityList->GetElementByID(i)->SetCascade(draw ? conPlaneVisibility : NULL);
		conPlanesVisibilityList->GetElementByID(i)->SetIcon(GV->sprIcons16[(draw ? Icon16_View : Icon16_ViewNo)]);
	}
}

void cAppMenu::SyncPlaneSwitched()
{
	if (conActivePlane->GetElementsCount() != GV->editState->hParser->GetPlanesCount())
		return;
	for (int i = 0; i < GV->editState->hParser->GetPlanesCount(); i++) {
		conActivePlane->GetElementByID(i)->SetIcon(GV->editState->GetActivePlaneID() == i ? GV->sprIcons16[Icon16_Applied] : NULL);
	}
}

void cAppMenu::SyncModeSwitched()
{
	//hEntries[AppMenu_Mode]->SetIcon(GV->editState->GetActiveMode() == EWW_MODE_OBJECT ? Icon16_ModeObject : Icon16_ModeTile);
}

void cAppMenu::SyncPlanes()
{
	conActivePlane->ClearElements();
	conPlanesVisibilityList->ClearElements();
	for (int i = 0; i < GV->editState->hParser->GetPlanesCount(); i++) {
		conActivePlane->AddElement(i, GV->editState->hParser->GetPlane(i)->GetName(),
			(GV->editState->GetActivePlaneID() == i ? GV->sprIcons16[Icon16_Applied] : NULL));
		conPlanesVisibilityList->AddElement(i, GV->editState->hParser->GetPlane(i)->GetName(),
			GV->sprIcons16[(GV->editState->hPlaneData[i]->bDraw ? Icon16_View : Icon16_ViewNo)]);
		conPlanesVisibilityList->GetElementByID(i)->SetCascade(conPlaneVisibility);
	}
	conPlanesVisibilityList->adjustSize();
	conActivePlane->adjustSize();
}

void cAppMenu::Think(bool bConsumed)
{
	hRulers->Think();
	if (!bEnabled) return;
	float mx, my;
	hge->Input_GetMousePos(&mx, &my);
	if (iOpened != -1) {
		SHR::Context * con = hEntries[iOpened]->GetContext();
		bool close = 0;
		while (1) {
			bool mover = (mx > con->getX() - 3 && my > con->getY() - 3 && mx < con->getX() + con->getWidth() + 3 && my < con->getY() + con->getHeight() + 3);
			if (mover) break;
			if (con->GetSelectedID() == -1) {
				if (mx <= 0 || mx >= iOverallWidth || my < 24 || my > 24 + 28)
					close = 1;
				break;
			}
			if (con->GetElementByID(con->GetSelectedID())->GetCascade() != NULL) {
				con = con->GetElementByID(con->GetSelectedID())->GetCascade();
			}
			else {
				close = 1;
				break;
			}
		}
		if (close) {
			hEntries[iOpened]->GetContext()->SetSelectedID(-1);
			hEntries[iOpened]->GetContext()->setVisible(0);
			iOpened = -1;
		}
	}
	int xoff = 0;
	if (!bConsumed)
		for (int i = 0; i < AppMenu_EntryCount; i++) {
			int w = hEntries[i]->GetWidth();
			bool mouseover = (mx > xoff && mx < xoff + w && my > 24 && my < 24 + 24);
			if (mouseover && hEntries[i]->IsEnabled() && (hge->Input_KeyDown(HGEK_LBUTTON) || iOpened != -1)) {
				if (iOpened != i) {
					if (iOpened != -1)
						hEntries[iOpened]->GetContext()->setVisible(0);
					hEntries[i]->GetContext()->setVisible(1);
				}
				/*if( i == AppMenu_Fold ){
				 SetFolded(!IsFolded());
				 iOpened = -1;
				}else{*/
				hEntries[i]->GetContext()->setPosition(xoff, 24 + 24);
				GV->editState->conMain->moveToTop(hEntries[i]->GetContext());
				iOpened = i;
				//}
				break;
			}
			xoff += w;
		}

}

void cAppMenu::Render()
{
	/*hgeQuad q;
	q.tex = 0;
	q.blend = BLEND_DEFAULT;
	SHR::SetQuad(&q, 0xFF3e3e3e, 0, 24, hge->System_GetState(HGE_SCREENWIDTH), 24+28);
	hge->Gfx_RenderQuad(&q);*/
	float mx, my;
	hge->Input_GetMousePos(&mx, &my);
	int xoff = 0;
	for (int i = 0; i < AppMenu_EntryCount; i++) {
		int w = hEntries[i]->GetWidth();
		bool mouseover = (mx > xoff && mx < xoff + w && my > LAY_APPMENU_Y && my < LAY_APPMENU_Y + LAY_APPMENU_H &&
			GV->editState->conMain->getWidgetAt(mx, my) == 0);
		xoff += hEntries[i]->Render(xoff, LAY_APPMENU_Y, mouseover);
		if (mouseover && hEntries[i]->IsEnabled() && bEnabled)
			GV->editState->bShowHand = 1;
	}
	hRulers->Render();
}

int cAppMenu_Entry::GetWidth()
{
	return GV->fntMyriad13->GetStringWidth(GetLabel().c_str()) + 28;
}

int cAppMenu_Entry::Render(int x, int y, bool bFocused)
{
	int w = GV->fntMyriad13->GetStringWidth(GetLabel().c_str()) + 28;
	bool unfolded = (hContext != 0 && hContext->isVisible());
	if (unfolded && fTimer < 0.4f) {
		fTimer += hge->Timer_GetDelta();
		if (fTimer > 0.4f) fTimer = 0.4f;
	}
	else if (!unfolded && fTimer > 0.2f) {
		fTimer -= hge->Timer_GetDelta();
		if (fTimer < 0.2f) fTimer = 0.2f;
	}
	else if (bFocused && fTimer < 0.2f) {
		fTimer += hge->Timer_GetDelta();
		if (fTimer > 0.2f) fTimer = 0.2f;
	}
	else if (!bFocused && fTimer > 0.0f) {
		fTimer -= hge->Timer_GetDelta();
		if (fTimer < 0.0f) fTimer = 0.0f;
	}

	if (fTimer > 0.2f) {
		hgeQuad q;
		q.tex = 0;
		q.blend = BLEND_DEFAULT;
		unsigned char p = (fTimer - 0.2f)*5.0f*255.0f;
		SHR::SetQuad(&q, 0xFFFFFFFF, x, y, x + w - 3, y + LAY_APPMENU_H);
		DWORD colTop = SETA(0xFF151515, p),
			colBottom = SETA(0xFF2b2b2b, p);
		q.v[0].col = q.v[1].col = colTop;
		q.v[2].col = q.v[3].col = colBottom;
		hge->Gfx_RenderQuad(&q);
	}

	hge->Gfx_RenderLine(x + w - 2, y + 1, x + w - 2, y + LAY_APPMENU_H, 0xFF333233);
	GV->sprIcons16[GetIcon()]->Render(x + 2, y + 4);
	if (fTimer > 0.0f && bEnabled) {
		GV->sprIcons16[GetIcon()]->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
		GV->sprIcons16[GetIcon()]->SetColor(SETA(0xFFFFFFFF, (unsigned char)((fTimer > 0.2f ? 0.2f : fTimer)*5.0f*255.0f)));
		GV->sprIcons16[GetIcon()]->Render(x + 2, y + 4);
		GV->sprIcons16[GetIcon()]->SetBlendMode(BLEND_DEFAULT);
	}
	GV->sprIcons16[GetIcon()]->SetColor(0xFFFFFFFF);
	DWORD dwText = 0xFFFFFFFF;
	if (!IsEnabled()) {
		dwText = 0xFF484848;
	}
	else if (fTimer < 0.2f) {
		unsigned char p = (161.0f + 94.0f*fTimer*5.0f);
		dwText = ARGB(255, p, p, p);
	}
	GV->fntMyriad13->SetColor(dwText);
	GV->fntMyriad13->Render(x + 20, y + 4, HGETEXT_LEFT, GetLabel().c_str());

	hge->Gfx_RenderLine(x + w - 1, y, x + w - 1, y + LAY_APPMENU_H, 0xFF111111);
	return w;
}

void cAppMenu::valueChanged(const SelectionEvent& event)
{
	if (event.getSource() == conPlanesVisibilityList) {
		SyncPlaneSelectedVisibility();
	}
}

void cAppMenu::action(const gcn::ActionEvent &actionEvent)
{
	if (actionEvent.getSource() == hEntries[AppMenu_File]->GetContext()) {
		int id = hEntries[AppMenu_File]->GetContext()->GetSelectedID();
		if (id == APPMEN_FILE_NEW) {
			if (GV->editState->NewMap_data == 0)
				GV->editState->NewMap_Open();
		}
		else if (id == APPMEN_FILE_OPEN) {
			char szFileopen[512] = "\0";
			if (GV->editState->PromptForDocument(szFileopen) != 0) {
				char * path = SHR::GetDir(szFileopen);
				GV->SetLastOpenPath(path);
				delete[] path;
				GV->StateMgr->Push(new State::LoadMap(szFileopen));
			}
		}
		else if (id == APPMEN_FILE_SAVEAS || (id == APPMEN_FILE_SAVE && strlen(GV->editState->hParser->GetFilePath()) == 0)) {
			OPENFILENAME ofn;
			char szFileopen[512] = "\0";
			ZeroMemory((&ofn), sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hge->System_GetState(HGE_HWND);
			ofn.lpstrFilter = "WapWorld Document (*.wwd)\0*.wwd\0Wszystkie pliki (*.*)\0*.*\0\0";
			ofn.lpstrFile = szFileopen;
			ofn.nMaxFile = sizeof(szFileopen);
			//((Menu*)Herculo.GameState)->ofn->Flags = OFN_FILEMUSTEXIST; // | OFN_NOCHANGEDIR
			ofn.lpstrDefExt = "wwd";
			ofn.lpstrInitialDir = GV->szLastSavePath;
			if (GetSaveFileName(&ofn)) {
				GV->editState->hParser->SetFilePath((const char*)szFileopen);
				bool ok = 0;
				try {
					GV->editState->MDI->PrepareDocToSave(GV->editState->MDI->GetActiveDocIt());
					GV->editState->hParser->CompileToFile(szFileopen, 1);
					GV->editState->hDataCtrl->RelocateDocument(szFileopen);
					ok = 1;
				}
				catch (WWD::Exception& exc) {
#ifdef BUILD_DEBUG
					GV->Console->Printf("~r~WWD exception: ~y~%d ~w~(~y~%s~w~:~y~%d~w~)", exc.iErrorCode, exc.szFile, exc.iLine);
#else
					GV->Console->Printf("~r~WWD exception ~y~%d", exc.iErrorCode);
#endif
				}
				if (ok) {
					char * fl = SHR::GetFile(GV->editState->hParser->GetFilePath());
					delete[] GV->editState->MDI->GetActiveDoc()->szFileName;
					GV->editState->MDI->GetActiveDoc()->szFileName = new char[strlen(fl) + 1];
					strcpy(GV->editState->MDI->GetActiveDoc()->szFileName, fl);
					GV->editState->MDI->GetActiveDoc()->bSaved = 1;
					GV->editState->SetHint("%s: %s", GETL2S("Hints", "FileSavedAs"), fl);
					delete[] fl;
					GV->editState->MDI->UpdateCrashList();
				}
				char * lastsave = SHR::GetDir(szFileopen);
				GV->SetLastSavePath(lastsave);
				delete[] lastsave;
			}
			//}else if( actionEvent.getSource() == m_hOwn->cbOptionsAlfaHigherPlanes ){
			// GV->bAlphaHigherPlanes = GV->cbOptionsAlfaHigherPlanes->isSelected();
		}
		else if (id == APPMEN_FILE_SAVE) {
			if (!GV->editState->MDI->GetActiveDoc()->bSaved) {
				try {
					GV->editState->MDI->PrepareDocToSave(GV->editState->MDI->GetActiveDocIt());
					GV->editState->hParser->CompileToFile(GV->editState->hParser->GetFilePath());
					GV->editState->MDI->GetActiveDoc()->bSaved = 1;
				}
				catch (WWD::Exception& exc) {
#ifdef BUILD_DEBUG
					GV->Console->Printf("~r~WWD exception: ~y~%d ~w~(~y~%s~w~:~y~%d~w~)", exc.iErrorCode, exc.szFile, exc.iLine);
#else
					GV->Console->Printf("~r~WWD exception ~y~%d", exc.iErrorCode);
#endif
				}
				//char * fl = SHR::GetFile(GV->editState->hParser->GetFilePath());
				//GV->editState->SetHint("%s: %s", GETL2S("Hints", "FileSaved"), fl);
				//delete [] fl;
			}
			else {
				GV->editState->SetHint(GETL2S("Hints", "NothingToSave"));
			}
		}
		else if (id == APPMEN_FILE_CLOSE) {
			GV->editState->MDI->CloseDocByIt(GV->editState->MDI->GetActiveDocIt());
		}
		else if (id == APPMEN_FILE_CLOSEALL) {
			GV->editState->MDI->CloseAllDocs();
		}
		hEntries[AppMenu_File]->GetContext()->setVisible(0);
	}
	else if (actionEvent.getSource() == hEntries[AppMenu_Edit]->GetContext()) {
		int id = hEntries[AppMenu_Edit]->GetContext()->GetSelectedID();
		if (id == APPMEN_EDIT_WORLD) {
			if (GV->editState->winWorld->isVisible()) GV->editState->SyncWorldOptionsWithParser();
			GV->editState->winWorld->setVisible(1);
			GV->editState->conMain->moveToTop(GV->editState->winWorld);
		}
		else if (id == APPMEN_EDIT_TILEPROP) {
			GV->editState->winTileProp->setVisible(1);
			GV->editState->conMain->moveToTop(GV->editState->winTileProp);
		}
		else if (id == APPMEN_EDIT_WORLDSCRIPT) {
			if (strlen(GV->editState->hParser->GetFilePath()) == 0) {
				MessageBox(hge->System_GetState(HGE_HWND), GETL2S("Win_LogicBrowser", "GlobalScriptDocumentSave"), "WapMap", MB_OK | MB_ICONERROR);
				return;
			}
			if (GV->editState->hCustomLogics->GetGlobalScript() == 0) {
				if (MessageBox(hge->System_GetState(HGE_HWND), GETL2S("Win_LogicBrowser", "NoGlobalScript"), "WapMap", MB_YESNO | MB_ICONWARNING) == IDYES) {
					GV->editState->hDataCtrl->FixCustomDir();
					GV->StateMgr->Push(new State::CodeEditor(0, 0, ""));
				}
			}
			else {
				GV->StateMgr->Push(new State::CodeEditor(GV->editState->hCustomLogics->GetGlobalScript(), 0, ""));
			}
		}
		//}else if( actionEvent.getSource() == conOpen ){
	}
	else if (actionEvent.getSource() == conActivePlane) {
		GV->editState->ddActivePlane->setSelected(conActivePlane->GetSelectedID());
		GV->editState->SwitchPlane();
		hEntries[AppMenu_Plane]->GetContext()->setVisible(0);
	}
	else if (actionEvent.getSource() == conOpenMRU) {
		hEntries[AppMenu_File]->GetContext()->setVisible(0);
		conOpenMRU->setVisible(0);
		GV->StateMgr->Push(new State::LoadMap(GV->editState->hMruList->GetRecentlyUsedFile(conOpenMRU->GetSelectedID())));
	}
	else if (actionEvent.getSource() == hEntries[AppMenu_WapMap]->GetContext()) {
		int id = hEntries[AppMenu_WapMap]->GetContext()->GetSelectedID();
		if (id == APPMEN_WM_SETTINGS) {
			GV->editState->hwinOptions->Open();
		}
		else if (id == APPMEN_WM_README) {

		}
		else if (id == APPMEN_WM_ABOUT) {
			GV->editState->hwinAbout->Open();
		}
		else if (id == APPMEN_WM_UPDATE) {

		}
		else if (id == APPMEN_WM_SITE) {

		}
		hEntries[AppMenu_WapMap]->GetContext()->setVisible(0);
	}
	else if (actionEvent.getSource() == hEntries[AppMenu_View]->GetContext()) {
		int id = hEntries[AppMenu_View]->GetContext()->GetSelectedID();
		if (id == APPMEN_VIEW_RULERS) {
			hRulers->SetVisible(!hRulers->IsVisible());
		}
		else if (id == APPMEN_VIEW_GUIDELINES) {
			GV->editState->bShowGuideLines = !GV->editState->bShowGuideLines;
			hEntries[AppMenu_View]->GetContext()->GetElementByID(APPMEN_VIEW_GUIDELINES)->SetIcon(GV->editState->bShowGuideLines ? GV->sprIcons16[Icon16_Applied] : NULL, 1);
		}
		else if (id == APPMEN_VIEW_TILEPROP) {
			GV->editState->bDrawTileProperties = !GV->editState->bDrawTileProperties;
			GV->editState->vPort->MarkToRedraw(1);
			hEntries[AppMenu_View]->GetContext()->GetElementByID(APPMEN_VIEW_TILEPROP)->SetIcon(GV->editState->bDrawTileProperties ? GV->sprIcons16[Icon16_Applied] : NULL, 1);
		}
	}
	else if (actionEvent.getSource() == hEntries[AppMenu_Plane]->GetContext()) {
		int id = hEntries[AppMenu_Plane]->GetContext()->GetSelectedID();
		if (id == APPMEN_PLANE_MGR) {
			GV->editState->winpmMain->setVisible(1);
			GV->editState->conMain->moveToTop(GV->editState->winpmMain);
			GV->editState->SyncPlaneProperties();
		}
	}
	else if (actionEvent.getSource() == conPlanesVisibilityList) {
		int id = conPlanesVisibilityList->GetSelectedID();
		GV->editState->hPlaneData[id]->bDraw = !GV->editState->hPlaneData[id]->bDraw;
		GV->editState->vPort->MarkToRedraw(1);
		SyncPlaneVisibility();
	}
	else if (actionEvent.getSource() == conPlaneVisibility) {
		int id = conPlaneVisibility->GetSelectedID(),
			pl = conPlanesVisibilityList->GetSelectedID();
		if (id == APPMEN_PLANEVIS_BORDER) {
			GV->editState->hPlaneData[pl]->bDrawBoundary = !GV->editState->hPlaneData[pl]->bDrawBoundary;
		}
		else if (id == APPMEN_PLANEVIS_GRID) {
			GV->editState->hPlaneData[pl]->bDrawGrid = !GV->editState->hPlaneData[pl]->bDrawGrid;
		}
		else if (id == APPMEN_PLANEVIS_OBJECTS) {
			GV->editState->hPlaneData[pl]->bDrawObjects = !GV->editState->hPlaneData[pl]->bDrawObjects;
		}
		if (GV->editState->hPlaneData[pl]->hRB != 0)
			GV->editState->hPlaneData[pl]->hRB->Redraw();
		GV->editState->vPort->MarkToRedraw(1);
		SyncPlaneSelectedVisibility();
	}
	else if (actionEvent.getSource() == hEntries[AppMenu_Tools]->GetContext()) {
		int id = hEntries[AppMenu_Tools]->GetContext()->GetSelectedID();
		if (id == APPMEN_TOOLS_PLAY) {
			GV->editState->hNativeController->RunGame(GV->editState->MDI->GetActiveDoc()->hParser->GetFilePath());
		}
		else if (id == APPMEN_TOOLS_MEASURE)
			GV->editState->SetTool(EWW_TOOL_MEASURE);
		else if (id == APPMEN_TOOLS_MAPSHOT) {
			GV->editState->winMapShot->setVisible(1);
			GV->editState->conMain->moveToTop(GV->editState->winMapShot);
		}
		else if (id == APPMEN_TOOLS_STATS) {
			GV->StateMgr->Push(new State::MapStats(GV->editState->hParser));
		}
		hEntries[AppMenu_Tools]->GetContext()->setVisible(0);
	}
	else if (actionEvent.getSource() == hEntries[AppMenu_Assets]->GetContext()) {
		int id = hEntries[AppMenu_Assets]->GetContext()->GetSelectedID();
		if (id == APPMEN_ASSETS_TILES) {
			GV->editState->hwinTileBrowser->Open();
		}
		else if (id == APPMEN_ASSETS_IMAGESETS) {
			GV->editState->hwinImageSetBrowser->Open();
		}
		else if (id == APPMEN_ASSETS_LOGICS) {
			GV->editState->winLogicBrowser->setVisible(1);
			GV->editState->conMain->moveToTop(GV->editState->winLogicBrowser);
		}
		else {
			GV->editState->winDB->setVisible(1);
			GV->editState->conMain->moveToTop(GV->editState->winDB);
		}
	}
}

void cAppMenu::FixInterfacePositions()
{
	GV->editState->MDI->SetY(LAY_MDI_Y);
	GV->editState->vPort->SetPos(hRulers->IsVisible() * 19, LAY_VIEWPORT_Y + hRulers->IsVisible() * 19);
	GV->editState->vPort->Resize(hge->System_GetState(HGE_SCREENWIDTH) - 20 - (hRulers->IsVisible() * 19), hge->System_GetState(HGE_SCREENHEIGHT) - 30 - 18 - LAY_VIEWPORT_Y - hRulers->IsVisible() * 19);
	GV->editState->sliVer->setY(LAY_VIEWPORT_Y + 2);
	GV->editState->sliVer->setHeight(hge->System_GetState(HGE_SCREENHEIGHT) - 46 - LAY_VIEWPORT_Y - 5);
	//GV->editState->SetIconBarVisible(!bFolded);
}

void cAppMenu::SetToolSpecificEnabled(bool b)
{
	hEntries[AppMenu_File]->SetEnabled(b);
	hEntries[AppMenu_Edit]->SetEnabled(b);
	hEntries[AppMenu_Plane]->SetEnabled(b);
	hEntries[AppMenu_Tools]->SetEnabled(b);
	hEntries[AppMenu_Assets]->SetEnabled(b);
}

/*void cAppMenu::SetFolded(bool b)
{
	//if( bFolded == b ) return;
	iHeight = (b ? 29 : 106);
	bFolded = b;

	if( hRulers->IsVisible() )
	 iHeight += 19;

	hEntries[AppMenu_Fold]->SetIcon(IsFolded() ? Icon16_Down : Icon16_Up);
	hEntries[AppMenu_Fold]->SetLabel(GETL2S("AppMenu", IsFolded() ? "Main_Unfold" : "Main_Fold"));

	FixInterfacePositions();
}*/

void cAppMenu::NotifyRulersSwitch()
{
	//SetFolded(IsFolded());
	FixInterfacePositions();
	hEntries[AppMenu_View]->GetContext()->GetElementByID(APPMEN_VIEW_RULERS)->SetIcon(hRulers->IsVisible() ? GV->sprIcons16[Icon16_Applied] : NULL, 1);
}
