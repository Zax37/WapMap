#include "procPickRect.h"
#include "../langID.h"
#include "../globals.h"
#include "../states/editing_ww.h"

extern HGE * hge;

namespace ObjEdit
{
	cProcPickRect::cProcPickRect(WWD::Object * phObj)
	{
		iType = PickRect_MinMax;
		hEvList = 0;
		bDrag = 0;
		bAllowEmpty = 0;
		bAllowEmptyAxis = 0;
		hObj = phObj;
		for (int i = 0; i < 4; i++) {
			lab[i] = new SHR::Lab();
			tf[i] = new SHR::TextField("");
			char tmp[50];
			if (i == 0) {
				lab[i]->setCaption("X1:");
				sprintf(tmp, "%d", hObj->GetParam(WWD::Param_MinX));
			}
			else if (i == 1) {
				lab[i]->setCaption("Y1:");
				sprintf(tmp, "%d", hObj->GetParam(WWD::Param_MinY));
			}
			else if (i == 2) {
				lab[i]->setCaption("X2:");
				sprintf(tmp, "%d", hObj->GetParam(WWD::Param_MaxX));
			}
			else if (i == 3) {
				lab[i]->setCaption("Y2:");
				sprintf(tmp, "%d", hObj->GetParam(WWD::Param_MaxY));
			}
			tf[i]->setText(tmp);
			lab[i]->adjustSize();
			tf[i]->setDimension(gcn::Rectangle(0, 0, 100, 20));
			tf[i]->addActionListener(this);
			tf[i]->SetNumerical(1, 0);
		}
		butPick = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Projectile", "Pick"));
		butPick->setDimension(gcn::Rectangle(0, 0, 125, 33));
		butPick->addActionListener(this);

		bPicking = 0;
	}

	cProcPickRect::~cProcPickRect()
	{
		for (int i = 0; i < 4; i++) {
			delete lab[i];
			delete tf[i];
		}
		delete butPick;
	}

	void cProcPickRect::Validate()
	{
		int v[4];
		for (int i = 0; i < 4; i++)
			v[i] = atoi(tf[i]->getText().c_str());
		bool allownull = v[0] == 0 && v[1] == 0 && v[2] == 0 && v[3] == 0 && bAllowEmpty;
		tf[0]->setMarkedInvalid(v[0] == 0 && !allownull && !(bAllowEmptyAxis && v[2] == 0) || v[0] > v[2]);
		tf[1]->setMarkedInvalid(v[1] == 0 && !allownull && !(bAllowEmptyAxis && v[3] == 0) || v[1] > v[3]);
		tf[2]->setMarkedInvalid(v[2] == 0 && !allownull && !(bAllowEmptyAxis && v[0] == 0) || v[0] > v[2]);
		tf[3]->setMarkedInvalid(v[3] == 0 && !allownull && !(bAllowEmptyAxis && v[1] == 0) || v[1] > v[3]);
	}

	void cProcPickRect::SetAllowEmpty(bool b)
	{
		bAllowEmpty = b;
		Validate();
	}

	void cProcPickRect::SetAllowEmptyAxis(bool b)
	{
		bAllowEmptyAxis = b;
		Validate();
	}

	void cProcPickRect::Enable(bool b)
	{
		if (bPicking) return;
		for (int i = 0; i < 4; i++) {
			tf[i]->setEnabled(b);
			lab[i]->setColor(b ? 0xFF000000 : 0xFF222222);
		}
		butPick->setEnabled(b);
	}

	void cProcPickRect::Think()
	{
		if (!bPicking) return;

		if (bDrag && !hge->Input_GetKeyState(HGEK_LBUTTON)) {
			bDrag = 0;
			butPick->simulatePress();
			return;
		}

		float mx, my;
		hge->Input_GetMousePos(&mx, &my);

		if (GV->editState->conMain->getWidgetAt(mx, my) != GV->editState->vPort->GetWidget())
			return;

		if (!bDrag && hge->Input_KeyDown(HGEK_LBUTTON)) {
			bDrag = 1;
			iDragX = GV->editState->Scr2WrdX(GV->editState->GetActivePlane(), mx);
			iDragY = GV->editState->Scr2WrdY(GV->editState->GetActivePlane(), my);
			SetValues(iDragX, iDragY, iDragX, iDragY);
			iDragmX = iDragX;
			iDragmY = iDragY;
			return;
		}

		if (!bDrag)
			return;

		int ax = GV->editState->Scr2WrdX(GV->editState->GetActivePlane(), mx),
			ay = GV->editState->Scr2WrdY(GV->editState->GetActivePlane(), my);

		if (ax < iDragX) {
			SetValues(ax, -1, iDragX);
		}
		else {
			SetValues(-1, -1, ax);
		}
		if (ay < iDragY) {
			SetValues(-1, ay, -1, iDragY);
		}
		else {
			SetValues(-1, -1, -1, ay);
		}
	}

	void cProcPickRect::action(const gcn::ActionEvent &actionEvent)
	{
		for (int i = 0; i < 4; i++)
			if (actionEvent.getSource() == tf[i]) {
				SetValues();
				Validate();
			}
		if (actionEvent.getSource() == butPick) {
			bPicking = !bPicking;
			butPick->setCaption(GETL2S("EditObj_Projectile", bPicking ? "Cancel" : "Pick"));
			for (int i = 0; i < 4; i++)
				tf[i]->setEnabled(!bPicking);
		}

		if (hEvList != 0)
			hEvList->action(actionEvent);
	}

	void cProcPickRect::AddWidgets(SHR::Contener * dest, int x, int y)
	{
		for (int i = 0; i < 4; i++) {
			dest->add(lab[i], x, y + 22 * i);
			dest->add(tf[i], x + 25, y + 22 * i);
		}
		dest->add(butPick, x, y + 22 * 4);
	}

	bool cProcPickRect::IsValid()
	{
		return !tf[0]->isMarkedInvalid() &&
			!tf[1]->isMarkedInvalid() &&
			!tf[2]->isMarkedInvalid() &&
			!tf[3]->isMarkedInvalid();
	}

	void cProcPickRect::SetValues(int x1, int y1, int x2, int y2)
	{
		if (x1 != -1) {
			char tmp[50];
			sprintf(tmp, "%d", x1);
			tf[0]->setText(tmp);
		}
		if (y1 != -1) {
			char tmp[50];
			sprintf(tmp, "%d", y1);
			tf[1]->setText(tmp);
		}
		if (x2 != -1) {
			char tmp[50];
			sprintf(tmp, "%d", x2);
			tf[2]->setText(tmp);
		}
		if (y2 != -1) {
			char tmp[50];
			sprintf(tmp, "%d", y2);
			tf[3]->setText(tmp);
		}
		if (iType == PickRect_MinMax) {
			hObj->SetParam(WWD::Param_MinX, x1 == -1 ? GetValue(0) : x1);
			hObj->SetParam(WWD::Param_MinY, y1 == -1 ? GetValue(1) : y1);
			hObj->SetParam(WWD::Param_MaxX, x2 == -1 ? GetValue(2) : x2);
			hObj->SetParam(WWD::Param_MaxY, y2 == -1 ? GetValue(3) : y2);
		}
		else {
			WWD::Rect r;
			r.x1 = GetValue(0);
			r.y1 = GetValue(1);
			r.x2 = GetValue(2);
			r.y2 = GetValue(3);
			if (iType == PickRect_AttackRect) {
				hObj->SetAttackRect(r);
			}
			else if (iType == PickRect_UserRect1) {
				hObj->SetUserRect(0, r);
			}
		}
		Validate();
	}

	void cProcPickRect::SetVisible(bool b)
	{
		for (int i = 0; i < 4; i++) {
			tf[i]->setVisible(b);
			lab[i]->setVisible(b);
		}
		butPick->setVisible(b);
	}

	void cProcPickRect::SetType(enProcPickRectType type)
	{
		iType = type;
		char tmp[50];
		if (iType == PickRect_MinMax) {
			SetValues(hObj->GetParam(WWD::Param_MinX), hObj->GetParam(WWD::Param_MinY),
				hObj->GetParam(WWD::Param_MaxX), hObj->GetParam(WWD::Param_MaxY));
		}
		else {
			WWD::Rect r;
			if (iType == PickRect_AttackRect) {
				r = hObj->GetAttackRect();
			}
			else if (iType == PickRect_UserRect1) {
				r = hObj->GetUserRect(0);
			}
			SetValues(r.x1, r.y1, r.x2, r.y2);
		}
		Validate();
	}
}
