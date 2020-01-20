#include "editAmbient.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include "../databanks/sounds.h"

extern HGE * hge;

namespace ObjEdit
{
	cEditObjAmbient::cEditObjAmbient(WWD::Object * obj, State::EditingWW * st) : cObjEdit(obj, st)
	{
		iType = ObjEdit::enAmbient;
		win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Ambient", "WinCaption"));
		win->setDimension(gcn::Rectangle(0, 0, 364, 535));
		win->setClose(1);
		win->addActionListener(hAL);
		win->add(vpAdv);
		win->setShadow(1);
		st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

		char tmp[128];

		labVolume = new SHR::Lab(GETL2S("EditObj_Ambient", "Volume"));
		labVolume->adjustSize();
		win->add(labVolume, 5, 15);

		sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_Damage) != 0 ? hTempObj->GetParam(WWD::Param_Damage) : 100);
		tfVolume = new SHR::TextField(tmp);
		tfVolume->addActionListener(hAL);
		tfVolume->SetNumerical(1, 0);
		tfVolume->setDimension(gcn::Rectangle(0, 0, 100, 20));
		win->add(tfVolume, 125, 15);

		if (!strlen(hTempObj->GetAnim())) {
			hTempObj->SetAnim(hState->hSndBank->getElementAt(0).c_str());
		}

		tddSound = new SHR::TextDropDown("", hState->hSndBank);
		tddSound->setDimension(gcn::Rectangle(0, 0, 350, 20));
		tddSound->SetGfx(&GV->gcnParts);
		tddSound->addActionListener(hAL);
		tddSound->setText(hTempObj->GetAnim());
		win->add(tddSound, 5, 15 + 25);

		hPlayer = new cSoundPlayer(win, 5, 37 + 25,
			hState->hSndBank->GetAssetByID(hTempObj->GetAnim()), GETL2S("EditObj_Ambient", "Preview"));

		labArea = new SHR::Lab(GETL2S("EditObj_Ambient", "AreaType"));
		labArea->adjustSize();
		win->add(labArea, 5, 115 + 25);

		sprintf(tmp, "AREA%p", this);
		rbAreaType[0] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Ambient", "AreaGlobal"), tmp, 0);
		rbAreaType[0]->adjustSize();
		rbAreaType[0]->addActionListener(hAL);
		win->add(rbAreaType[0], 5, 115 + 50);

		rbAreaType[1] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Ambient", "AreaSpecified"), tmp, 0);
		rbAreaType[1]->adjustSize();
		rbAreaType[1]->addActionListener(hAL);
		win->add(rbAreaType[1], 185, 115 + 50);

		if ((!strcmp(hTempObj->GetLogic(), "GlobalAmbientSound") || !strcmp(hTempObj->GetLogic(), "AmbientSound")) &&
			(!hTempObj->GetParam(WWD::Param_MinX) || !hTempObj->GetParam(WWD::Param_MinY) ||
				!hTempObj->GetParam(WWD::Param_MaxX) || !hTempObj->GetParam(WWD::Param_MaxY)) &&
				(!hTempObj->GetAttackRect().x1 || !hTempObj->GetAttackRect().y1 ||
					!hTempObj->GetAttackRect().x2 || !hTempObj->GetAttackRect().y2)) {
			rbAreaType[0]->setSelected(1);
		}
		else {
			rbAreaType[1]->setSelected(1);
		}

		for (int i = 0; i < 2; i++) {
			cbArea[i] = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Ambient", i ? "SecondArea" : "SpecifyArea"));
			cbArea[i]->adjustSize();
			cbArea[i]->addActionListener(hAL);
			win->add(cbArea[i], 5 + i * 177, 115 + 75);

			hPickArea[i] = new cProcPickRect(hTempObj);
			hPickArea[i]->AddWidgets(win, 5 + i * 177 + 26, 115 + 100);
			hPickArea[i]->SetActionListener(hAL);
			hPickArea[i]->SetAllowEmpty(1);
			hPickArea[i]->SetType(i ? PickRect_AttackRect : PickRect_MinMax);

			cbArea[i]->setEnabled(rbAreaType[1]->isSelected());
			hPickArea[i]->Enable(rbAreaType[1]->isSelected());
		}

		cbArea[0]->setSelected(hTempObj->GetParam(WWD::Param_MinX) != 0 &&
			hTempObj->GetParam(WWD::Param_MinY) != 0 &&
			hTempObj->GetParam(WWD::Param_MaxX) != 0 &&
			hTempObj->GetParam(WWD::Param_MaxY) != 0);
		cbArea[1]->setSelected(hTempObj->GetAttackRect().x1 != 0 && hTempObj->GetAttackRect().y1 != 0 &&
			hTempObj->GetAttackRect().x2 != 0 && hTempObj->GetAttackRect().y2 != 0);

		labPlayType = new SHR::Lab(GETL2S("EditObj_Ambient", "PlayPolicy"));
		labPlayType->adjustSize();
		win->add(labPlayType, 5, 350);
		sprintf(tmp, "PLAY%p", this);

		rbPlayPolicy[0] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Ambient", "PlayLoop"), tmp, 0);
		rbPlayPolicy[0]->adjustSize();
		rbPlayPolicy[0]->addActionListener(hAL);
		win->add(rbPlayPolicy[0], 5, 350 + 25);

		rbPlayPolicy[1] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Ambient", "PlayInterval"), tmp, 0);
		rbPlayPolicy[1]->adjustSize();
		rbPlayPolicy[1]->addActionListener(hAL);
		win->add(rbPlayPolicy[1], 182, 350 + 25);

		if (!hTempObj->GetMoveRect().x1 || !hTempObj->GetMoveRect().x2 || !hTempObj->GetMoveRect().y1 || !hTempObj->GetMoveRect().y2) {
			rbPlayPolicy[0]->setSelected(1);
		}
		else {
			rbPlayPolicy[1]->setSelected(1);
		}

		labTurnOnTime = new SHR::Lab(GETL2S("EditObj_Ambient", "TimeOn"));
		labTurnOnTime->adjustSize();
		win->add(labTurnOnTime, 5, 350 + 50);

		labTurnOffTime = new SHR::Lab(GETL2S("EditObj_Ambient", "TimeOff"));
		labTurnOffTime->adjustSize();
		win->add(labTurnOffTime, 182, 350 + 50);

		for (int i = 0; i < 2; i++) {
			hPickArea[i]->Enable(cbArea[i]->isSelected());

			labTimeMin[i] = new SHR::Lab(GETL2S("EditObj_Ambient", i ? "TimeMax" : "TimeMin"));
			labTimeMin[i]->adjustSize();
			win->add(labTimeMin[i], 5, 350 + 75 + 25 * i);

			sprintf(tmp, "%d", i ? hTempObj->GetMoveRect().x2 : hTempObj->GetMoveRect().x1);
			tfTimeOn[i] = new SHR::TextField(tmp);
			tfTimeOn[i]->SetNumerical(1, 0);
			tfTimeOn[i]->setDimension(gcn::Rectangle(0, 0, 75, 20));
			tfTimeOn[i]->addActionListener(hAL);
			win->add(tfTimeOn[i], 172 - 75, 350 + 75 + 25 * i);

			labTimeMax[i] = new SHR::Lab(GETL2S("EditObj_Ambient", i ? "TimeMax" : "TimeMin"));
			labTimeMax[i]->adjustSize();
			win->add(labTimeMax[i], 182, 350 + 75 + 25 * i);

			sprintf(tmp, "%d", i ? hTempObj->GetMoveRect().y2 : hTempObj->GetMoveRect().y1);
			tfTimeOff[i] = new SHR::TextField(tmp);
			tfTimeOff[i]->SetNumerical(1, 0);
			tfTimeOff[i]->addActionListener(hAL);
			tfTimeOff[i]->setDimension(gcn::Rectangle(0, 0, 75, 20));
			win->add(tfTimeOff[i], 182 + 97, 350 + 75 + 25 * i);

			tfTimeOn[i]->setEnabled(rbPlayPolicy[1]->isSelected());
			tfTimeOff[i]->setEnabled(rbPlayPolicy[1]->isSelected());
		}

		win->add(_butAddNext, 157, 500 - 19);
		win->add(_butSave, 257, 500 - 19);
	}

	cEditObjAmbient::~cEditObjAmbient()
	{
		for (int i = 0; i < 2; i++) {
			delete rbAreaType[i];
			delete hPickArea[i];
			delete cbArea[i];
			delete rbPlayPolicy[i];
			delete tfTimeOn[i];
			delete tfTimeOff[i];
			delete labTimeMax[i];
			delete labTimeMin[i];
		}
		delete labTurnOffTime;
		delete labTurnOnTime;
		delete labPlayType;
		delete labArea;
		delete labVolume;
		delete tfVolume;
		delete tddSound;
		delete hPlayer;
		delete win;
		hState->vPort->MarkToRedraw(1);
	}

	void cEditObjAmbient::Save()
	{
		if (rbPlayPolicy[1]->isSelected()) {
			hTempObj->SetMoveRect(WWD::Rect(atoi(tfTimeOn[0]->getText().c_str()),
				atoi(tfTimeOff[0]->getText().c_str()),
				atoi(tfTimeOn[1]->getText().c_str()),
				atoi(tfTimeOff[1]->getText().c_str())));
		}
	}

	void cEditObjAmbient::EnableControlsSync()
	{
		bool bRectPick = hPickArea[0]->IsPicking() || hPickArea[1]->IsPicking();
		for (int i = 0; i < 2; i++) {
			tfTimeOff[i]->setEnabled(rbPlayPolicy[1]->isSelected());
			tfTimeOn[i]->setEnabled(rbPlayPolicy[1]->isSelected());
			hPickArea[i]->Enable(rbAreaType[1]->isSelected() && cbArea[i]->isSelected() && cbArea[0]->isSelected() && !hPickArea[!i]->IsPicking());
			hPickArea[i]->SetAllowEmpty(!hPickArea[i]->IsEnabled());
			rbAreaType[i]->setEnabled(!bRectPick);
		}
		cbArea[0]->setEnabled(rbAreaType[1]->isSelected() && !bRectPick);
		cbArea[1]->setEnabled(rbAreaType[1]->isSelected() && cbArea[0]->isSelected() && !bRectPick);
		_butSave->setEnabled(!bRectPick&&hPickArea[0]->IsValid() && hPickArea[1]->IsValid() && !tddSound->isMarkedInvalid());
		DWORD dwCol = (rbPlayPolicy[1]->isSelected() ? 0xFF000000 : 0xFF222222);
		labTurnOnTime->setColor(dwCol);
		labTurnOffTime->setColor(dwCol);
		labTimeMin[0]->setColor(dwCol);
		labTimeMax[0]->setColor(dwCol);
		labTimeMin[1]->setColor(dwCol);
		labTimeMax[1]->setColor(dwCol);
	}

	void cEditObjAmbient::Action(const gcn::ActionEvent &actionEvent)
	{
		if (actionEvent.getSource() == win) {
			bKill = 1;
			return;
		}
		else if (actionEvent.getSource() == rbAreaType[0]) {
			hTempObj->SetParam(WWD::Param_MinX, 0);
			hTempObj->SetParam(WWD::Param_MinY, 0);
			hTempObj->SetParam(WWD::Param_MaxX, 0);
			hTempObj->SetParam(WWD::Param_MaxY, 0);
			hTempObj->SetAttackRect(WWD::Rect(0, 0, 0, 0));
			EnableControlsSync();
			Action(gcn::ActionEvent(cbArea[0], ""));
			hTempObj->SetLogic("GlobalAmbientSound");
		}
		else if (actionEvent.getSource() == rbAreaType[1]) {
			EnableControlsSync();
			Action(gcn::ActionEvent(cbArea[0], ""));
		}
		else if (actionEvent.getSource() == rbPlayPolicy[0]) {
			hTempObj->SetMoveRect(WWD::Rect(0, 0, 0, 0));
			EnableControlsSync();
		}
		else if (actionEvent.getSource() == rbPlayPolicy[1]) {
			hTempObj->SetMoveRect(WWD::Rect(atoi(tfTimeOn[0]->getText().c_str()),
				atoi(tfTimeOff[0]->getText().c_str()),
				atoi(tfTimeOn[1]->getText().c_str()),
				atoi(tfTimeOff[1]->getText().c_str())));
			EnableControlsSync();
		}
		else if (actionEvent.getSource() == tfVolume) {
			hTempObj->SetParam(WWD::Param_Damage, atoi(tfVolume->getText().c_str()));
		}
		else if (actionEvent.getSource() == cbArea[0]) {
			if (!cbArea[0]->isSelected() || !hPickArea[0]->IsValid()) {
				hTempObj->SetLogic("SpotAmbientSound");
			}
			else
				hTempObj->SetLogic("AmbientSound");
			if (cbArea[0]->isSelected() && cbArea[0]->isEnabled()) {
				hTempObj->SetParam(WWD::Param_MinX, hPickArea[0]->GetValue(0));
				hTempObj->SetParam(WWD::Param_MinY, hPickArea[0]->GetValue(1));
				hTempObj->SetParam(WWD::Param_MaxX, hPickArea[0]->GetValue(2));
				hTempObj->SetParam(WWD::Param_MaxY, hPickArea[0]->GetValue(3));
			}
			else {
				hTempObj->SetParam(WWD::Param_MinX, 0);
				hTempObj->SetParam(WWD::Param_MinY, 0);
				hTempObj->SetParam(WWD::Param_MaxX, 0);
				hTempObj->SetParam(WWD::Param_MaxY, 0);
			}
			EnableControlsSync();
			Action(gcn::ActionEvent(cbArea[1], ""));
		}
		else if (actionEvent.getSource() == cbArea[1]) {
			if (cbArea[1]->isSelected() && cbArea[1]->isEnabled()) {
				hTempObj->SetAttackRect(WWD::Rect(hPickArea[1]->GetValue(0),
					hPickArea[1]->GetValue(1),
					hPickArea[1]->GetValue(2),
					hPickArea[1]->GetValue(3)));
			}
			else {
				hTempObj->SetAttackRect(WWD::Rect(0, 0, 0, 0));
			}
			EnableControlsSync();
		}
		else if (actionEvent.getSource() == hPickArea[0]->GetPickButton()) {
			if (!hPickArea[0]->IsValid() && !hPickArea[0]->IsPicking()) {
				hTempObj->SetLogic("SpotAmbientSound");
			}
			else
				hTempObj->SetLogic("AmbientSound");
			EnableControlsSync();
			return;
		}
		else if (actionEvent.getSource() == hPickArea[1]->GetPickButton()) {

			EnableControlsSync();
			return;
		}
		else if (actionEvent.getSource() == tddSound) {
			tddSound->setMarkedInvalid(hState->hSndBank->GetAssetByID(tddSound->getText().c_str()) == 0);
			if (!tddSound->isMarkedInvalid()) {
				hTempObj->SetAnim(tddSound->getText().c_str());
				hPlayer->SetAsset(hState->hSndBank->GetAssetByID(tddSound->getText().c_str()), 0);
			}
			EnableControlsSync();
		}
		for (int i = 0; i < 2; i++)
			if (actionEvent.getSource() == tfTimeOff[i]) {
				int v1 = atoi(tfTimeOff[0]->getText().c_str()),
					v2 = atoi(tfTimeOff[1]->getText().c_str());
				tfTimeOff[0]->setMarkedInvalid(v1 >= v2);
				tfTimeOff[1]->setMarkedInvalid(v1 >= v2);
			}
			else if (actionEvent.getSource() == tfTimeOn[i]) {
				int v1 = atoi(tfTimeOn[0]->getText().c_str()),
					v2 = atoi(tfTimeOn[1]->getText().c_str());
				tfTimeOn[0]->setMarkedInvalid(v1 >= v2);
				tfTimeOn[1]->setMarkedInvalid(v1 >= v2);
			}
	}

	void cEditObjAmbient::Draw()
	{
		int dx, dy;
		win->getAbsolutePosition(dx, dy);

		hge->Gfx_RenderLine(dx, dy + 24 + 130, dx + win->getWidth(), dy + 24 + 130, GV->colLineDark);
		hge->Gfx_RenderLine(dx, dy + 24 + 131, dx + win->getWidth(), dy + 24 + 131, GV->colLineBright);

		hge->Gfx_RenderLine(dx, dy + 24 + 179, dx + win->getWidth(), dy + 24 + 179, GV->colLineDark);
		hge->Gfx_RenderLine(dx, dy + 24 + 180, dx + win->getWidth(), dy + 24 + 180, GV->colLineBright);

		hge->Gfx_RenderLine(dx, dy + 24 + 337, dx + win->getWidth(), dy + 24 + 337, GV->colLineDark);
		hge->Gfx_RenderLine(dx, dy + 24 + 338, dx + win->getWidth(), dy + 24 + 338, GV->colLineBright);

		hge->Gfx_RenderLine(dx, dy + 24 + 389, dx + win->getWidth(), dy + 24 + 389, GV->colLineDark);
		hge->Gfx_RenderLine(dx, dy + 24 + 390, dx + win->getWidth(), dy + 24 + 390, GV->colLineBright);

		hge->Gfx_RenderLine(dx, dy + 24 + 486 - 19, dx + win->getWidth(), dy + 24 + 486 - 19, GV->colLineDark);
		hge->Gfx_RenderLine(dx, dy + 24 + 487 - 19, dx + win->getWidth(), dy + 24 + 487 - 19, GV->colLineBright);

		hge->Gfx_RenderLine(dx + 178, dy + 24 + 179, dx + 178, dy + 24 + 336, GV->colLineDark);
		hge->Gfx_RenderLine(dx + 179, dy + 24 + 179, dx + 179, dy + 24 + 336, GV->colLineBright);

		hge->Gfx_RenderLine(dx + 178, dy + 24 + 389, dx + 178, dy + 24 + 485 - 19, GV->colLineDark);
		hge->Gfx_RenderLine(dx + 179, dy + 24 + 389, dx + 179, dy + 24 + 485 - 19, GV->colLineBright);

		/*for(int i=0;i<2;i++){
		 GV->fntMyriad13->SetColor(0xFF000000);
		 GV->fntMyriad13->Render(dx+754+137*i, dy+73, HGETEXT_LEFT, "X 1:");
		 GV->fntMyriad13->Render(dx+754+137*i, dy+73+21, HGETEXT_LEFT, "Y 1:");
		 GV->fntMyriad13->Render(dx+754+137*i, dy+73+42, HGETEXT_LEFT, "X 2:");
		 GV->fntMyriad13->Render(dx+754+137*i, dy+73+63, HGETEXT_LEFT, "Y 2:");
		}*/
	}

	void cEditObjAmbient::_Think(bool bMouseConsumed)
	{
		if (!bMouseConsumed) {
			hPickArea[0]->Think();
			hPickArea[1]->Think();
		}
	}
}
