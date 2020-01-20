#include "editElevator.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../../shared/commonFunc.h"
#include "../cObjectUserData.h"
#include <cmath>

extern HGE * hge;

namespace ObjEdit
{

	cEditObjElevator::cEditObjElevator(WWD::Object * obj, State::EditingWW * st) : cObjEdit(obj, st)
	{
		iType = ObjEdit::enElevator;

		winType = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Elevator", "WinCaption"));
		winType->setDimension(gcn::Rectangle(0, 0, 300, 260));
		winType->setClose(1);
		winType->addActionListener(hAL);
		winType->setMovable(0);
		winType->setShadow(0);
		st->conMain->add(winType, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - winType->getHeight());

		winProp = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Elevator", "Win2Caption"));
		winProp->setDimension(gcn::Rectangle(0, 0, 600, 200));
		winProp->setClose(1);
		winProp->addActionListener(hAL);
		winProp->add(vpAdv);
		winProp->setMovable(0);
		winProp->setShadow(0);
		st->conMain->add(winProp, st->vPort->GetX() + winType->getWidth(), st->vPort->GetY() + st->vPort->GetHeight() - winProp->getHeight());

		labType = new SHR::Lab(GETL2S("EditObj_Elevator", "Type"));
		labType->adjustSize();
		winType->add(labType, 5, 10);

		char tmp[128];
		sprintf(tmp, "%p", this);
		for (int i = 0; i < 4; i++) {
			char lab[64];
			sprintf(lab, "Type_%d", i + 1);
			rbType[i] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Elevator", lab), tmp);
			rbType[i]->addActionListener(hAL);
			rbType[i]->adjustSize();
			winType->add(rbType[i], 5, 35 + 40 * i);

			sprintf(lab, "Type_%d_desc", i + 1);
			labTypeDesc[i] = new SHR::Lab(GETL2S("EditObj_Elevator", lab));
			labTypeDesc[i]->adjustSize();
			labTypeDesc[i]->setColor(0xFF101010);
			winType->add(labTypeDesc[i], 5, 35 + 40 * i + 20);
		}

		cbOneWay = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Elevator", "OneWay"));
		cbOneWay->adjustSize();
		cbOneWay->addActionListener(hAL);
		winType->add(cbOneWay, 5, 35 + 160);

		labOneWay = new SHR::Lab(GETL2S("EditObj_Elevator", "OneWayDesc"));
		labOneWay->adjustSize();
		winType->add(labOneWay, 5, 35 + 180);

		if (!strcmp(hTempObj->GetLogic(), "Elevator"))
			rbType[0]->setSelected(1);
		else if (!strcmp(hTempObj->GetLogic(), "StartElevator") || !strcmp(hTempObj->GetLogic(), "OneWayStartElevator"))
			rbType[1]->setSelected(1);
		else if (!strcmp(hTempObj->GetLogic(), "StopElevator"))
			rbType[2]->setSelected(1);
		else if (!strcmp(hTempObj->GetLogic(), "TriggerElevator"))
			rbType[3]->setSelected(1);

		rArea.x1 = hTempObj->GetParam(WWD::Param_MinX);
		rArea.y1 = hTempObj->GetParam(WWD::Param_MinY);
		rArea.x2 = hTempObj->GetParam(WWD::Param_MaxX);
		rArea.y2 = hTempObj->GetParam(WWD::Param_MaxY);

		if (!rbType[0]->isSelected()) {
			if (rArea.x1 == 0 && GetUserDataFromObj(obj)->GetX() <= rArea.x2 && rArea.x2 != 0)
				rArea.x1 = GetUserDataFromObj(obj)->GetX();
			if (rArea.x2 == 0 && GetUserDataFromObj(obj)->GetX() >= rArea.x1 && rArea.x1 != 0)
				rArea.x2 = GetUserDataFromObj(obj)->GetX();
			if (rArea.y1 == 0 && GetUserDataFromObj(obj)->GetY() <= rArea.y2 && rArea.y2 != 0)
				rArea.y1 = GetUserDataFromObj(obj)->GetY();
			if (rArea.y2 == 0 && GetUserDataFromObj(obj)->GetY() >= rArea.y1 && rArea.y1 != 0)
				rArea.y2 = GetUserDataFromObj(obj)->GetY();
		}

		cbOneWay->setSelected(rArea.x1 == rArea.x2 && rArea.y1 == rArea.y2 && rArea.x1 != 0 && rArea.y2 != 0 ||
			!strcmp(hTempObj->GetLogic(), "OneWayStartElevator"));

		labSpeedX = new SHR::Lab(GETL2S("EditObj_Elevator", "SpeedX"));
		labSpeedX->adjustSize();
		winProp->add(labSpeedX, 5, 10);

		tfSpeedX = new SHR::TextField("0");
		tfSpeedX->setDimension(gcn::Rectangle(0, 0, 75, 20));
		tfSpeedX->SetNumerical(1, 0);
		tfSpeedX->addActionListener(hAL);
		winProp->add(tfSpeedX, 150, 10);

		labSpeedY = new SHR::Lab(GETL2S("EditObj_Elevator", "SpeedY"));
		labSpeedY->adjustSize();
		winProp->add(labSpeedY, 5, 35);

		tfSpeedY = new SHR::TextField("0");
		tfSpeedY->setDimension(gcn::Rectangle(0, 0, 75, 20));
		tfSpeedY->SetNumerical(1, 0);
		tfSpeedY->addActionListener(hAL);
		winProp->add(tfSpeedY, 150, 35);

		butPickSpeed = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Elevator", "Pick"));
		butPickSpeed->setDimension(gcn::Rectangle(0, 0, 75, 33));
		butPickSpeed->addActionListener(hAL);
		winProp->add(butPickSpeed, 150, 60);

		/*labSpeedLinear = new SHR::Lab(GETL2S("EditObj_Elevator", "LinearSpeed"));
		labSpeedLinear->adjustSize();
		win->add(labSpeedLinear, 5, 295);

		tfSpeedLinear = new SHR::TextField("0");
		tfSpeedLinear->setDimension(gcn::Rectangle(0, 0, 140, 20));
		tfSpeedLinear->SetNumerical(1, 0);
		tfSpeedLinear->addActionListener(hAL);
		win->add(tfSpeedLinear, 150, 295);

		labAngle = new SHR::Lab(GETL2S("EditObj_Elevator", "Angle"));
		labAngle->adjustSize();
		win->add(labAngle, 5, 320);

		tfAngle = new SHR::TextField("0");
		tfAngle->setDimension(gcn::Rectangle(0, 0, 140, 20));
		tfAngle->SetNumerical(1, 0);
		tfAngle->addActionListener(hAL);
		win->add(tfAngle, 150, 320);*/

		labMoveArea = new SHR::Lab(GETL2S("EditObj_Elevator", "MoveArea"));
		labMoveArea->adjustSize();
		winProp->add(labMoveArea, 235, 10);

		hRectPick = new cProcPickRect(hTempObj);
		hRectPick->AddWidgets(winProp, 235, 35);
		hRectPick->SetActionListener(hAL);
		hRectPick->SetType(PickRect_MinMax);

		labDirection = new SHR::Lab(GETL2S("EditObj_Elevator", "Direction"));
		labDirection->adjustSize();
		winProp->add(labDirection, 230 + 150, 10);

		int xoffset = 230 + 150;
		int yoffset = 35;
		butDirection[0] = hState->MakeButton(xoffset + 32, yoffset, Icon_Up, winProp);
		butDirection[1] = hState->MakeButton(xoffset + 64, yoffset, Icon_UpRight, winProp);
		butDirection[2] = hState->MakeButton(xoffset + 64, yoffset + 32, Icon_Right, winProp);
		butDirection[3] = hState->MakeButton(xoffset + 64, yoffset + 64, Icon_DownRight, winProp);
		butDirection[4] = hState->MakeButton(xoffset + 32, yoffset + 64, Icon_Down, winProp);
		butDirection[5] = hState->MakeButton(xoffset, yoffset + 64, Icon_DownLeft, winProp);
		butDirection[6] = hState->MakeButton(xoffset, yoffset + 32, Icon_Left, winProp);
		butDirection[7] = hState->MakeButton(xoffset, yoffset, Icon_UpLeft, winProp);
		for (int i = 0; i < 8; i++) {
			butDirection[i]->removeActionListener(GV->editState->al);
			butDirection[i]->addActionListener(hAL);
		}

		bRectangleArea = (rArea.x1 != 0 &&
			rArea.y1 != 0 &&
			rArea.x2 != 0 &&
			rArea.y2 != 0);
		if (!bRectangleArea)
			bAreaAxis = (rArea.y1 != 0 && rArea.y2 != 0);

		iDirection = hTempObj->GetParam(WWD::Param_Direction);
		if (!rbType[0]->isSelected() && bRectangleArea)
			iDirection = 7;
		if (iDirection < 1 || iDirection > 9) {
			if (bRectangleArea) iDirection = 3;
			else {
				iDirection = (hTempObj->GetParam(WWD::Param_MinX) != 0 && hTempObj->GetParam(WWD::Param_MaxX) != 0
					? 6
					: 2);
			}
		}

		sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedX) == 0 ? 125 : hTempObj->GetParam(WWD::Param_SpeedX));
		tfSpeedX->setText(tmp);
		sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedY) == 0 ? 125 : hTempObj->GetParam(WWD::Param_SpeedY));
		tfSpeedY->setText(tmp);

		RebuildWindow();

		winProp->add(_butAddNext, 450, 75);
		winProp->add(_butSave, 450, 125);
	}

	void cEditObjElevator::RebuildWindow()
	{
		if (!rbType[0]->isSelected() && bRectangleArea)
			iDirection = 7;
		if (bRectangleArea &&
			(iDirection == 8 || iDirection == 2 || iDirection == 4 || iDirection == 6))
			iDirection = 7;
		if (!bRectangleArea &&
			(iDirection == 7 || iDirection == 9 || iDirection == 1 || iDirection == 3))
			iDirection = 2;

		tfSpeedX->setEnabled(bRectangleArea || !bAreaAxis);
		labSpeedX->setColor(tfSpeedX->isEnabled() ? 0xFF000000 : 0xFF222222);
		if (!tfSpeedX->isEnabled())
			tfSpeedX->setText("0");
		else {
			char tmp[64];
			sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedX) == 0 ? 125 : hTempObj->GetParam(WWD::Param_SpeedX));
			tfSpeedX->setText(tmp);
		}
		tfSpeedY->setEnabled(bRectangleArea || bAreaAxis);
		labSpeedY->setColor(tfSpeedY->isEnabled() ? 0xFF000000 : 0xFF222222);
		if (!tfSpeedY->isEnabled())
			tfSpeedY->setText("0");
		else {
			char tmp[64];
			sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedY) == 0 ? 125 : hTempObj->GetParam(WWD::Param_SpeedY));
			tfSpeedY->setText(tmp);
		}

		/*tfSpeedLinear->setEnabled(bRectangleArea);
		tfAngle->setEnabled(bRectangleArea);
		labSpeedLinear->setColor(tfSpeedLinear->isEnabled() ? 0xFF000000 : 0xFF222222);
		labAngle->setColor(tfAngle->isEnabled() ? 0xFF000000 : 0xFF222222);
		if( tfAngle->isEnabled() )
		 tfAngle->setMarkedInvalid(atoi(tfAngle->getText().c_str())>360);
		else
		 tfAngle->setMarkedInvalid(0);*/

		hRectPick->SetVisible(bRectangleArea);

		for (int i = 0; i < 8; i++)
			butDirection[i]->setIconColor(0x55FFFFFF);
		if (iDirection == 8) butDirection[0]->setIconColor(0xFFFFFFFF);
		else if (iDirection == 9) butDirection[1]->setIconColor(0xFFFFFFFF);
		else if (iDirection == 6) butDirection[2]->setIconColor(0xFFFFFFFF);
		else if (iDirection == 3) butDirection[3]->setIconColor(0xFFFFFFFF);
		else if (iDirection == 2) butDirection[4]->setIconColor(0xFFFFFFFF);
		else if (iDirection == 1) butDirection[5]->setIconColor(0xFFFFFFFF);
		else if (iDirection == 4) butDirection[6]->setIconColor(0xFFFFFFFF);
		else if (iDirection == 7) butDirection[7]->setIconColor(0xFFFFFFFF);
		for (int i = 0; i < 7; i++)
			butDirection[i]->setEnabled(rbType[0]->isSelected() || !bRectangleArea);
	}

	cEditObjElevator::~cEditObjElevator()
	{
		delete cbOneWay;
		delete labOneWay;
		for (int i = 0; i < 8; i++)
			delete butDirection[i];
		delete tfSpeedX;
		delete tfSpeedY;
		//delete tfAngle;
		//delete tfSpeedLinear;
		delete hRectPick;
		delete labDirection;
		delete labMoveArea;
		delete labSpeedX;
		delete labSpeedY;
		//delete labSpeedLinear;
		//delete labAngle;
		delete labType;
		for (int i = 0; i < 4; i++) {
			delete rbType[i];
			delete labTypeDesc[i];
		}
		delete winType;
		delete winProp;
		hState->vPort->MarkToRedraw(1);
	}

	void cEditObjElevator::Save()
	{

	}

	void cEditObjElevator::Action(const gcn::ActionEvent &actionEvent)
	{
		if (actionEvent.getSource() == winType || actionEvent.getSource() == winProp) {
			bKill = 1;
			return;
		}
		for (int i = 0; i < 4; i++)
			if (actionEvent.getSource() == rbType[i]) {
				if (i == 0) hTempObj->SetLogic("Elevator");
				else if (i == 1) hTempObj->SetLogic("StartElevator");
				else if (i == 2) hTempObj->SetLogic("StopElevator");
				else if (i == 3) hTempObj->SetLogic("TriggerElevator");
				RebuildWindow();
			}
		for (int i = 0; i < 8; i++)
			if (actionEvent.getSource() == butDirection[i]) {
				int iOldDir = iDirection;
				if (i == 0) iDirection = 8;
				else if (i == 1) iDirection = 9;
				else if (i == 2) iDirection = 6;
				else if (i == 3) iDirection = 3;
				else if (i == 4) iDirection = 2;
				else if (i == 5) iDirection = 1;
				else if (i == 6) iDirection = 4;
				else if (i == 7) iDirection = 7;
				bRectangleArea = (i == 1 || i == 3 || i == 5 || i == 7);
				bAreaAxis = (i == 0 || i == 4);
				if ((iDirection == 2 || iDirection == 8) && iOldDir != 2 && iOldDir != 8)
					hTempObj->SetParam(WWD::Param_SpeedY, 125);
				if ((iDirection == 4 || iDirection == 6) && iOldDir != 4 && iOldDir != 6)
					hTempObj->SetParam(WWD::Param_SpeedX, 125);
				RebuildWindow();
			}
	}

	void cEditObjElevator::_Think(bool bMouseConsumed)
	{
		float mx, my;
		hge->Input_GetMousePos(&mx, &my);
		bool bMouseIn = 1;
		if (mx < hState->vPort->GetX() || my < hState->vPort->GetY() ||
			mx > hState->vPort->GetX() + hState->vPort->GetWidth() || my > hState->vPort->GetY() + hState->vPort->GetHeight())
			bMouseIn = 0;
	}

	void cEditObjElevator::Draw()
	{
		int dx, dy;
		winProp->getAbsolutePosition(dx, dy);
		hge->Gfx_RenderLine(dx, dy + 255, dx + winProp->getWidth(), dy + 255, GV->colLineDark);
		hge->Gfx_RenderLine(dx, dy + 256, dx + winProp->getWidth(), dy + 256, GV->colLineBright);

		hge->Gfx_RenderLine(dx, dy + 363, dx + winProp->getWidth(), dy + 363, GV->colLineDark);
		hge->Gfx_RenderLine(dx, dy + 364, dx + winProp->getWidth(), dy + 364, GV->colLineBright);

		hge->Gfx_RenderLine(dx, dy + 518, dx + winProp->getWidth(), dy + 518, GV->colLineDark);
		hge->Gfx_RenderLine(dx, dy + 519, dx + winProp->getWidth(), dy + 519, GV->colLineBright);
	}
}
