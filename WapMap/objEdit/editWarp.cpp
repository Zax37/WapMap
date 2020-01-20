#include "editWarp.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include "../databanks/imageSets.h"

extern HGE * hge;

namespace ObjEdit
{
	cEditObjWarp::cEditObjWarp(WWD::Object * obj, State::EditingWW * st) : cObjEdit(obj, st)
	{
		iType = ObjEdit::enWarp;
		win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Warp", "WinCaption"));
		win->setDimension(gcn::Rectangle(0, 0, 200, 375));
		win->setClose(1);
		win->addActionListener(hAL);
		win->add(vpAdv);
		win->setShadow(0);
		win->setMovable(0);
		st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

		win->add(_butAddNext, 0, 325);
		win->add(_butSave, 100, 325);

		fCpAnimTimer = fScpAnimTimer = 0;
		iCpAnimFrame = 0;
		iScpAnimFrame = 5;

		char tmp[24];
		sprintf(tmp, "%p", this);

		rbType[0] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Warp", "Hor"), tmp, !strcmp(hTempObj->GetImageSet(), "GAME_WARP"));
		rbType[0]->adjustSize();
		rbType[0]->addActionListener(hAL);
		win->add(rbType[0], 5, 10);
		rbType[1] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Warp", "Ver"), tmp, !strcmp(hTempObj->GetImageSet(), "GAME_VERTWARP"));
		rbType[1]->adjustSize();
		rbType[1]->addActionListener(hAL);
		win->add(rbType[1], 5, 80);
		rbType[2] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Warp", "Boss"), tmp, !strcmp(hTempObj->GetImageSet(), "GAME_BOSSWARP"));
		rbType[2]->adjustSize();
		rbType[2]->addActionListener(hAL);
		win->add(rbType[2], 5, 175);

		if (!rbType[0]->isSelected() && !rbType[1]->isSelected() && !rbType[2]->isSelected()) {
			rbType[0]->setSelected(1);
			hTempObj->SetImageSet("GAME_WARP");
			hState->vPort->MarkToRedraw(1);
		}

		char tmpstr[256];
		sprintf(tmpstr, "%d", hTempObj->GetParam(WWD::Param_SpeedX));
		tfX = new SHR::TextField(tmpstr);
		tfX->SetNumerical(1, 0);
		tfX->addActionListener(hAL);
		tfX->setDimension(gcn::Rectangle(0, 0, 75, 20));
		win->add(tfX, 20, 275);

		sprintf(tmpstr, "%d", hTempObj->GetParam(WWD::Param_SpeedY));
		tfY = new SHR::TextField(tmpstr);
		tfY->SetNumerical(1, 0);
		tfY->addActionListener(hAL);
		tfY->setDimension(gcn::Rectangle(0, 0, 75, 20));
		win->add(tfY, 20, 300);

		butPick = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Warp", "Pick"));
		butPick->setDimension(gcn::Rectangle(0, 0, 95, 33));
		butPick->addActionListener(hAL);
		win->add(butPick, 100, 282);

		cbOneTime = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Warp", "OneTime"));
		cbOneTime->adjustSize();
		cbOneTime->setSelected(hTempObj->GetParam(WWD::Param_Smarts) == 0);
		win->add(cbOneTime, 5, 325);

		bPick = 0;

		RebuildWindow();
	}

	cEditObjWarp::~cEditObjWarp()
	{
		for (int i = 0; i < 3; i++)
			delete rbType[i];
		delete tfX;
		delete tfY;
		delete cbOneTime;
		delete butPick;
		delete win;
		hState->vPort->MarkToRedraw(1);
	}

	void cEditObjWarp::RebuildWindow()
	{
		win->setHeight(378 + (!rbType[2]->isSelected() * 22));
		win->setY(hState->vPort->GetY() + hState->vPort->GetHeight() - win->getHeight());
		_butSave->setY(328 + (!rbType[2]->isSelected() * 22));
		_butAddNext->setY(328 + (!rbType[2]->isSelected() * 22));
		cbOneTime->setVisible(!rbType[2]->isSelected());
	}

	void cEditObjWarp::Save()
	{
		if (!rbType[2]->isSelected())
			hTempObj->SetParam(WWD::Param_Smarts, cbOneTime->isSelected() ? 0 : 1);
	}

	void cEditObjWarp::Action(const gcn::ActionEvent &actionEvent)
	{
		if (actionEvent.getSource() == win) {
			bKill = 1;
			return;
		}
		else if (actionEvent.getSource() == rbType[0]) {
			hTempObj->SetLogic("SpecialPowerup");
			hTempObj->SetImageSet("GAME_WARP");
			hState->vPort->MarkToRedraw(1);
			RebuildWindow();
		}
		else if (actionEvent.getSource() == rbType[1]) {
			hTempObj->SetLogic("SpecialPowerup");
			hTempObj->SetImageSet("GAME_VERTWARP");
			hState->vPort->MarkToRedraw(1);
			RebuildWindow();
		}
		else if (actionEvent.getSource() == rbType[2]) {
			hTempObj->SetLogic("BossWarp");
			hTempObj->SetImageSet("GAME_BOSSWARP");
			hState->vPort->MarkToRedraw(1);
			RebuildWindow();
		}
		else if (actionEvent.getSource() == butPick) {
			if (!bPick) {
				bPick = 1;
				bAllowDragging = 0;
				tfX->setEnabled(0);
				tfY->setEnabled(0);
				rbType[0]->setEnabled(0);
				rbType[1]->setEnabled(0);
				_butSave->setEnabled(0);
				butPick->setCaption(GETL2S("EditObj_Warp", "Unpick"));
			}
			else {
				bPick = 0;
				bAllowDragging = 1;
				tfX->setEnabled(1);
				tfY->setEnabled(1);
				rbType[0]->setEnabled(1);
				rbType[1]->setEnabled(1);
				_butSave->setEnabled(1);
				butPick->setCaption(GETL2S("EditObj_Warp", "Pick"));
			}
		}
		else if (actionEvent.getSource() == tfX) {
			hTempObj->SetParam(WWD::Param_SpeedX, atoi(tfX->getText().c_str()));
			hState->vPort->MarkToRedraw(1);
		}
		else if (actionEvent.getSource() == tfY) {
			hTempObj->SetParam(WWD::Param_SpeedY, atoi(tfY->getText().c_str()));
			hState->vPort->MarkToRedraw(1);
		}
	}

	void cEditObjWarp::Draw()
	{
		int dx, dy;
		win->getAbsolutePosition(dx, dy);
		fCpAnimTimer += hge->Timer_GetDelta();
		while (fCpAnimTimer > 0.15) {
			fCpAnimTimer -= 0.1;
			iCpAnimFrame++;
			if (iCpAnimFrame > 9)
				iCpAnimFrame = 0;
		}
		fScpAnimTimer += hge->Timer_GetDelta();
		while (fScpAnimTimer > 0.15) {
			fScpAnimTimer -= 0.1;
			iScpAnimFrame++;
			if (iScpAnimFrame > 9)
				iScpAnimFrame = 0;
		}

		hState->SprBank->GetAssetByID("GAME_WARP")->GetIMGByIterator(iCpAnimFrame)->GetSprite()->Render(dx + 100, dy + 70);
		hState->SprBank->GetAssetByID("GAME_VERTWARP")->GetIMGByIterator(iScpAnimFrame)->GetSprite()->Render(dx + 100, dy + 155);
		hState->SprBank->GetAssetByID("GAME_BOSSWARP")->GetIMGByIterator(iScpAnimFrame)->GetSprite()->Render(dx + 100, dy + 240);
		GV->fntMyriad13->SetColor(0xFF000000);
		GV->fntMyriad13->Render(dx + 5, dy + 295 - 25, HGETEXT_LEFT, GETL2S("EditObj_Warp", "TeleportDestination"));
		GV->fntMyriad13->Render(dx + 5, dy + 318 - 25, HGETEXT_LEFT, "X:");
		GV->fntMyriad13->Render(dx + 5, dy + 342 - 25, HGETEXT_LEFT, "Y:");

		hge->Gfx_RenderLine(dx, dy + 295 - 27, dx + win->getWidth(), dy + 295 - 27, GV->colLineDark);
		hge->Gfx_RenderLine(dx, dy + 295 - 26, dx + win->getWidth(), dy + 295 - 26, GV->colLineBright);

		hge->Gfx_RenderLine(dx, dy + 295 + 67 - (rbType[2]->isSelected() * 22), dx + win->getWidth(), dy + 295 + 67 - (rbType[2]->isSelected() * 22), GV->colLineDark);
		hge->Gfx_RenderLine(dx, dy + 295 + 68 - (rbType[2]->isSelected() * 22), dx + win->getWidth(), dy + 295 + 68 - (rbType[2]->isSelected() * 22), GV->colLineBright);

		if (bPick) {
			float mx, my;
			hge->Input_GetMousePos(&mx, &my);
			if (hge->Input_KeyDown(HGEK_LBUTTON) && hState->conMain->getWidgetAt(mx, my) == hState->vPort->GetWidget()) {
				bPick = 0;
				bAllowDragging = 1;
				tfX->setEnabled(1);
				tfY->setEnabled(1);
				rbType[0]->setEnabled(1);
				rbType[1]->setEnabled(1);
				_butSave->setEnabled(1);
				butPick->setCaption(GETL2S("EditObj_Warp", "Pick"));
				char tmp[64];
				sprintf(tmp, "%d", hState->Scr2WrdX(hState->GetActivePlane(), mx));
				tfX->setText(tmp, 1);
				sprintf(tmp, "%d", hState->Scr2WrdY(hState->GetActivePlane(), my));
				tfY->setText(tmp, 1);
			}
		}
	}
}
