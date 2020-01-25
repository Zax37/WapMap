#include "options.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../../shared/commonFunc.h"
#include "../cInterfaceSheet.h"
#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wSlider.h"
#include "../../shared/gcnWidgets/wScrollArea.h"
#include "../../shared/gcnWidgets/wDropDown.h"
#include "../cNativeController.h"

extern HGE * hge;

cListModelDisplay::cListModelDisplay(bool bCrazyHook) {
	if (bCrazyHook) {
		const char * cptr = GETL2S("Options", "Default");
		m_vOpt.push_back(std::string(cptr));
	}
	std::vector<SHR::DisplayMode> displays = SHR::GetDisplayModes();
	for (int i = 0; i < displays.size(); i++) {
		if (displays[i].iDepth == 32 && ((bCrazyHook && displays[i].iWidth >= 640 && displays[i].iHeight >= 480) ||
			(!bCrazyHook && displays[i].iWidth >= 1024 && displays[i].iHeight >= 768))) {
			char tmp[256];
			sprintf(tmp, "%dx%d", displays[i].iWidth, displays[i].iHeight);
			bool already = 0;
			for (int x = 0; x < m_vOpt.size(); x++)
				if (!strcmp(tmp, m_vOpt[x].c_str())) {
					already = 1;
					break;
				}
			if (already) continue;
			m_vOpt.push_back(std::string(tmp));
		}
	}
}

cListModelDisplay::~cListModelDisplay() {

}

std::string cListModelDisplay::getElementAt(int i)
{
	if (i >= 0 && i < m_vOpt.size())
		return m_vOpt[i];
	return "";
}

int cListModelDisplay::getNumberOfElements()
{
	return m_vOpt.size();
}


winOptions::winOptions()
{
	myWin = new SHR::Win(&GV->gcnParts, GETL(Lang_Options));
	myWin->setDimension(gcn::Rectangle(0, 0, 600, 320));
	myWin->setClose(1);
	myWin->setVisible(0);
	myWin->addActionListener(this);

	vp = new WIDG::Viewport(this, 0);
	myWin->add(vp, 0, 0);

	labscpDesc = new SHR::Lab(GETL2S("Options", "ClawInstallDir"));
	labscpDesc->adjustSize();
	myWin->add(labscpDesc, 5, 22);
	tfscpPath = new SHR::TextField(GV->szClawPath == 0 ? "" : GV->szClawPath);
	tfscpPath->adjustSize();
	tfscpPath->addActionListener(this);
	tfscpPath->setWidth(600 - 50 - labscpDesc->getWidth());
	myWin->add(tfscpPath, labscpDesc->getWidth() + 10, 22);
	butscpPath = GV->editState->MakeButton(600 - 40, 17, Icon_Open, myWin, true, true, this);

	labscpVersion = new SHR::Lab("");
	labscpVersion->adjustSize();
	myWin->add(labscpVersion, tfscpPath->getX(), 45);

	//editor options

	laboptEditorSettings = new SHR::Lab(GETL2S("Options", "Editor"));
	laboptEditorSettings->adjustSize();
	myWin->add(laboptEditorSettings, 35, 80);

	laboptLang = new SHR::Lab(GETL(Lang_Language));
	laboptLang->adjustSize();
	myWin->add(laboptLang, 5, 115);

	lmLang = new cDynamicListModel();

	ddoptLang = new SHR::DropDown();
	ddoptLang->setListModel(lmLang);
	ddoptLang->setDimension(gcn::Rectangle(0, 0, 150, 20));
	ddoptLang->addActionListener(this);
	ddoptLang->SetGfx(&GV->gcnParts);
	ddoptLang->adjustHeight();
	myWin->add(ddoptLang, 140, 115);

	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA fdata;
	hFind = FindFirstFile("lang/*", &fdata);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (fdata.cFileName[0] != '.') {
				std::string filen = fdata.cFileName;
				filen = filen.substr(0, filen.rfind('.'));
				lmLang->Add(filen);
			}
		} while (FindNextFile(hFind, &fdata) != 0);
	}

	for (int i = 0; i < lmLang->getNumberOfElements(); i++) {
		if (!strcmp(lmLang->getElementAt(i).c_str(), GV->Lang->GetName())) {
			ddoptLang->setSelected(i);
			break;
		}
	}

	laboptChangesLang = new SHR::Lab(GETL(Lang_OptionChangesAfterRestart));
	laboptChangesLang->adjustSize();
	laboptChangesLang->setColor(0x5e5e5e);
	laboptChangesLang->setVisible(0);
	myWin->add(laboptChangesLang, 5, 135);

	laboptRes = new SHR::Lab(GETL(Lang_DisplayResolution));
	laboptRes->adjustSize();
	myWin->add(laboptRes, 5, 155);

	ddoptRes = new SHR::DropDown();
	ddoptRes->setListModel(new cListModelDisplay());
	ddoptRes->setDimension(gcn::Rectangle(0, 0, 150, 20));
	ddoptRes->addActionListener(this);
	ddoptRes->SetGfx(&GV->gcnParts);
	ddoptRes->adjustHeight();
	myWin->add(ddoptRes, 140, 155);

	{
		char actres[64];
		sprintf(actres, "%dx%d", GV->iScreenW, GV->iScreenH);
		for (int i = 0; i < ddoptRes->getListModel()->getNumberOfElements(); i++) {
			if (!strcmp(ddoptRes->getListModel()->getElementAt(i).c_str(), actres)) {
				ddoptRes->setSelected(i);
				break;
			}
		}
	}

	laboptChangesRes = new SHR::Lab(GETL(Lang_OptionChangesAfterRestart));
	laboptChangesRes->adjustSize();
	laboptChangesRes->setVisible(0);
	laboptChangesRes->setColor(0x5e5e5e);
	myWin->add(laboptChangesRes, 5, 175);

	cbOptionsAlfaHigherPlanes = new SHR::CBox(GV->hGfxInterface, GETL(Lang_AlphaOverlapping));
	cbOptionsAlfaHigherPlanes->adjustSize();
	//cbOptionsAlfaHigherPlanes->addActionListener(al);
	cbOptionsAlfaHigherPlanes->setSelected(GV->bAlphaHigherPlanes);
	myWin->add(cbOptionsAlfaHigherPlanes, 5, 235);

	cboptSmoothZooming = new SHR::CBox(GV->hGfxInterface, GETL(Lang_UseSmoothZooming));
	cboptSmoothZooming->adjustSize();
	cboptSmoothZooming->setSelected(GV->bSmoothZoom);
	myWin->add(cboptSmoothZooming, 5, 195);
	cboptAutoUpdate = new SHR::CBox(GV->hGfxInterface, GETL(Lang_AutoUpdate));
	cboptAutoUpdate->adjustSize();
	cboptAutoUpdate->setSelected(GV->bAutoUpdate);
	myWin->add(cboptAutoUpdate, 5, 215);

	//crazyhook options

	laboptCrazyHookSettings = new SHR::Lab("");
	laboptCrazyHookSettings->adjustSize();
	myWin->add(laboptCrazyHookSettings, 335, 80);

	laboptGameRes = new SHR::Lab(GETL2S("Options", "GameRes"));
	laboptGameRes->adjustSize();
	myWin->add(laboptGameRes, 300, 115);

	ddoptGameRes = new SHR::DropDown();
	ddoptGameRes->setListModel(new cListModelDisplay(1));
	ddoptGameRes->setDimension(gcn::Rectangle(0, 0, 150, 20));
	ddoptGameRes->addActionListener(this);
	ddoptGameRes->SetGfx(&GV->gcnParts);
	ddoptGameRes->adjustHeight();
	myWin->add(ddoptGameRes, 440, 115);

	{
		char actres[64];
		int w, h;
		GV->editState->hNativeController->GetDisplayResolution(&w, &h);
		if (w == -1 && h == -1) {
			ddoptGameRes->setSelected(0);
		}
		else {
			sprintf(actres, "%dx%d", w, h);
			for (int i = 0; i < ddoptGameRes->getListModel()->getNumberOfElements(); i++) {
				if (!strcmp(ddoptGameRes->getListModel()->getElementAt(i).c_str(), actres)) {
					ddoptGameRes->setSelected(i);
					break;
				}
			}
		}
	}

	cboptCrazyHookDebugInfo = new SHR::CBox(GV->hGfxInterface, GETL2S("Options", "DebugInf"));
	cboptCrazyHookDebugInfo->adjustSize();
	cboptCrazyHookDebugInfo->setSelected(GV->editState->hNativeController->IsDebugInfoOn());
	myWin->add(cboptCrazyHookDebugInfo, 300, 170);
	cboptCrazyHookGodMode = new SHR::CBox(GV->hGfxInterface, GETL2S("Options", "GodMode"));
	cboptCrazyHookGodMode->adjustSize();
	cboptCrazyHookGodMode->setSelected(GV->editState->hNativeController->IsGodModeOn());
	myWin->add(cboptCrazyHookGodMode, 300, 190);
	cboptCrazyHookArmor = new SHR::CBox(GV->hGfxInterface, GETL2S("Options", "ArmorMode"));
	cboptCrazyHookArmor->adjustSize();
	cboptCrazyHookArmor->setSelected(GV->editState->hNativeController->IsArmorModeOn());
	myWin->add(cboptCrazyHookArmor, 300, 210);

	butoptSave = new SHR::But(GV->hGfxInterface, GETL(Lang_Save));
	butoptSave->setDimension(gcn::Rectangle(0, 0, 100, 33));
	butoptSave->addActionListener(this);
	myWin->add(butoptSave, 15, 260);
}

winOptions::~winOptions()
{
	delete laboptChangesRes; delete laboptChangesLang; delete laboptRes; delete laboptLang; delete laboptGameRes;
	delete laboptEditorSettings; delete laboptCrazyHookSettings; delete labscpDesc; delete labscpVersion;
	delete butscpPath; delete butoptSave;
	delete ddoptLang; delete ddoptRes; delete ddoptGameRes;
	delete lmLang;
	delete cbOptionsAlfaHigherPlanes; delete cboptCrazyHookGodMode; delete cboptCrazyHookArmor; delete cboptCrazyHookDebugInfo;
	delete cboptAutoUpdate; delete cboptSmoothZooming;
	delete tfscpPath;
	delete vp;
	delete myWin;
}

void winOptions::Think()
{

}

void winOptions::Draw(int piCode)
{
	int dx, dy;
	myWin->getAbsolutePosition(dx, dy);

	unsigned char alpha = myWin->getAlpha();
	DWORD col = SETA(0xFFFFFFFF, alpha),
		colt = SETA(0xFFFFFFFF, (unsigned char)(alpha*0.5f));

	GV->sprIcons[Icon_WapMap]->SetColor(col);
	GV->sprIcons[Icon_WapMap]->Render(dx + 5, dy + 90);

	GV->sprIcons[Icon_CrazyHook]->SetColor(GV->editState->hNativeController->IsCrazyHookAvailable() ? col : colt);
	GV->sprIcons[Icon_CrazyHook]->Render(dx + 300, dy + 90);

	//upper
	hge->Gfx_RenderLine(dx, dy + 80, dx + myWin->getWidth(), dy + 80, GV->colLineDark);
	hge->Gfx_RenderLine(dx, dy + 81, dx + myWin->getWidth(), dy + 81, GV->colLineBright);

	//bottom
	hge->Gfx_RenderLine(dx, dy + 270, dx + myWin->getWidth(), dy + 270, GV->colLineDark);
	hge->Gfx_RenderLine(dx, dy + 271, dx + myWin->getWidth(), dy + 271, GV->colLineBright);

	//middle
	hge->Gfx_RenderLine(dx + 296, dy + 81, dx + 296, dy + 270, GV->colLineDark);
	hge->Gfx_RenderLine(dx + 297, dy + 81, dx + 297, dy + 270, GV->colLineBright);
}

void winOptions::Open()
{
	myWin->setVisible(1);
	myWin->getParent()->moveToTop(myWin);
}

void winOptions::Close()
{
	myWin->setVisible(0);
}

void winOptions::SyncWithExe()
{
	action(gcn::ActionEvent(tfscpPath, ""));
}

void winOptions::PickAndSetClawLocation()
{
    OPENFILENAME ofn;
    char szFileopen[512] = "\0";
    ZeroMemory((&ofn), sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hge->System_GetState(HGE_HWND);
    ofn.lpstrFilter = "Kapitan Pazur (CLAW.EXE)\0claw.exe\0Wszystkie pliki (*.*)\0*.*\0\0";
    ofn.lpstrFile = szFileopen;
    ofn.nMaxFile = sizeof(szFileopen);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = "exe";
    ofn.lpstrInitialDir = GV->szLastOpenPath;
    if (GetOpenFileName(&ofn) != 0) {
        char * path = SHR::GetDir(szFileopen);
        tfscpPath->setText(std::string(path), 1);
        GV->SetLastOpenPath(path);
        delete[] path;
    }
}

void winOptions::action(const ActionEvent& actionEvent)
{
	if (actionEvent.getSource() == ddoptLang) {
		laboptChangesLang->setVisible(!(!strcmp(ddoptLang->getListModel()->getElementAt(ddoptLang->getSelected()).c_str(), GV->Lang->GetName())));
	}
	else if (actionEvent.getSource() == ddoptRes) {
		int x, y;
		sscanf(ddoptRes->getListModel()->getElementAt(ddoptRes->getSelected()).c_str(), "%dx%d", &x, &y);
		laboptChangesRes->setVisible((x != hge->System_GetState(HGE_SCREENWIDTH) || y != hge->System_GetState(HGE_SCREENHEIGHT)));
	}
	else if (actionEvent.getSource() == butscpPath) {
        PickAndSetClawLocation();
	}
	else if (actionEvent.getSource() == tfscpPath) {
		if (!GV->editState->hNativeController->IsValid() ||
			GV->editState->hNativeController->GetPath() != tfscpPath->getText())
			GV->editState->hNativeController->SetPath(tfscpPath->getText());
		bool b = GV->editState->hNativeController->IsValid() && GV->editState->hNativeController->IsCrazyHookAvailable();
		cboptCrazyHookDebugInfo->setEnabled(b);
		cboptCrazyHookGodMode->setEnabled(b);
		cboptCrazyHookArmor->setEnabled(b);
		laboptCrazyHookSettings->setCaption(GETL2S("Options", (b ? "CrazyHook" : "CrazyHookNotFound")));
		laboptCrazyHookSettings->adjustSize();
		ddoptGameRes->setEnabled(b);

		laboptGameRes->setColor(b ? 0xa1a1a1 : 0x5e5e5e);
		laboptCrazyHookSettings->setColor(b ? 0xa1a1a1 : 0x5e5e5e);

		if (GV->editState->hNativeController->IsValid()) {
			labscpVersion->setCaption(std::string(GETL2S("Options", "DetectedVer")) + " " + GV->editState->hNativeController->GetVersionStr());
			labscpVersion->adjustSize();
		}
		labscpVersion->setVisible(GV->editState->hNativeController->IsValid());
	}
	else if (actionEvent.getSource() == butoptSave) {

		delete[] GV->szClawPath;
		GV->szClawPath = new char[strlen(tfscpPath->getText().c_str()) + 1];
		strcpy(GV->szClawPath, tfscpPath->getText().c_str());
		GV->ini->SetValue("Paths", "Claw", GV->szClawPath);

		GV->ini->SetValue("WapMap", "Language", ddoptLang->getListModel()->getElementAt(ddoptLang->getSelected()).c_str());
		char tmp[8];
		sprintf(tmp, "%d", cbOptionsAlfaHigherPlanes->isSelected());
		GV->ini->SetValue("WapMap", "AlphaOverlapping", tmp);
		GV->bAlphaHigherPlanes = cbOptionsAlfaHigherPlanes->isSelected();

		sprintf(tmp, "%d", cboptAutoUpdate->isSelected());
		GV->ini->SetValue("WapMap", "AutoUpdate", tmp);
		GV->bAutoUpdate = cboptAutoUpdate->isSelected();

		sprintf(tmp, "%d", cboptSmoothZooming->isSelected());
		GV->ini->SetValue("WapMap", "SmoothZoom", tmp);
		if (GV->bSmoothZoom != cboptSmoothZooming->isSelected())
			GV->editState->fZoom = GV->editState->fDestZoom;
		GV->bSmoothZoom = cboptSmoothZooming->isSelected();

		int w, h;
		sscanf(ddoptRes->getListModel()->getElementAt(ddoptRes->getSelected()).c_str(), "%dx%d", &w, &h);
		sprintf(tmp, "%d", w);
		GV->ini->SetValue("WapMap", "DisplayWidth", tmp);
		sprintf(tmp, "%d", h);
		GV->ini->SetValue("WapMap", "DisplayHeight", tmp);
		GV->iScreenW = w;
		GV->iScreenH = h;

		if (ddoptGameRes->getSelected() == 0) {
			w = h = -1;
			sprintf(tmp, "%d", w);
			GV->ini->SetValue("CrazyHook", "DisplayWidth", tmp);
			GV->ini->SetValue("CrazyHook", "DisplayHeight", tmp);
		}
		else {
			sscanf(ddoptGameRes->getListModel()->getElementAt(ddoptGameRes->getSelected()).c_str(), "%dx%d", &w, &h);
			sprintf(tmp, "%d", w);
			GV->ini->SetValue("CrazyHook", "DisplayWidth", tmp);
			sprintf(tmp, "%d", h);
			GV->ini->SetValue("CrazyHook", "DisplayHeight", tmp);
		}
		GV->editState->hNativeController->SetDisplayResolution(w, h);

		sprintf(tmp, "%d", cboptCrazyHookDebugInfo->isSelected());
		GV->ini->SetValue("CrazyHook", "DebugInfo", tmp);
		GV->editState->hNativeController->SetDebugInfo(cboptCrazyHookDebugInfo->isSelected());

		sprintf(tmp, "%d", cboptCrazyHookGodMode->isSelected());
		GV->ini->SetValue("CrazyHook", "GodMode", tmp);
		GV->editState->hNativeController->SetGodMode(cboptCrazyHookGodMode->isSelected());

		sprintf(tmp, "%d", cboptCrazyHookArmor->isSelected());
		GV->ini->SetValue("CrazyHook", "ArmorMode", tmp);
		GV->editState->hNativeController->SetArmorMode(cboptCrazyHookArmor->isSelected());

		GV->ini->SaveFile("settings.cfg");
		Close();
	}
}
