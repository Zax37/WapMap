#include "editSpringboard.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"

extern HGE * hge;

namespace ObjEdit
{
	cEditObjSpringboard::cEditObjSpringboard(WWD::Object * obj, State::EditingWW * st) : cObjEdit(obj, st)
	{
		iType = ObjEdit::enSpringboard;
		win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Springboard", "WinCaption"));
		win->setDimension(gcn::Rectangle(0, 0, 275, 125));
		win->setClose(1);
		win->addActionListener(hAL);
		win->add(vpAdv);
		win->setMovable(0);
		st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

		win->add(_butAddNext, win->getWidth() - 210, win->getHeight() - 60);
		win->add(_butSave, win->getWidth() - 110, win->getHeight() - 60);

		labJumpHeight = new SHR::Lab(GETL2S("EditObj_Springboard", "JumpHeight"));
		labJumpHeight->adjustSize();
		win->add(labJumpHeight, 5, 19);

		char tmp[64];
		sprintf(tmp, "%d", (hTempObj->GetParam(WWD::Param_MaxY) == 0 ? 450 : hTempObj->GetParam(WWD::Param_MaxY)));
		tfJumpHeight = new SHR::TextField(tmp);
		tfJumpHeight->setDimension(gcn::Rectangle(0, 0, 50, 20));
		tfJumpHeight->SetNumerical(1, 0);
		tfJumpHeight->addActionListener(hAL);
		win->add(tfJumpHeight, 100, 19);

		butPick = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Springboard", "Pick"));
		butPick->setDimension(gcn::Rectangle(0, 0, 100, 33));
		butPick->addActionListener(hAL);
		win->add(butPick, 165, 15);

		bPicking = 0;
	}

	cEditObjSpringboard::~cEditObjSpringboard()
	{
		delete labJumpHeight;
		delete tfJumpHeight;
		delete butPick;
		delete win;
		hState->vPort->MarkToRedraw(1);
	}

	void cEditObjSpringboard::Action(const gcn::ActionEvent &actionEvent)
	{
		if (actionEvent.getSource() == win) {
			bKill = 1;
			return;
		}
		else if (actionEvent.getSource() == butPick) {
			bPicking = !bPicking;
			bAllowDragging = !bPicking;
			iPickY = (hTempObj->GetParam(WWD::Param_MaxY) == 0 ? 450 : hTempObj->GetParam(WWD::Param_MaxY));
			tfJumpHeight->setEnabled(!bPicking);
			_butSave->setEnabled(!bPicking);
			butPick->setCaption(GETL2S("EditObj_Springboard", bPicking ? "Cancel" : "Pick"));
		}
		else if (actionEvent.getSource() == tfJumpHeight) {
			int nval = atoi(tfJumpHeight->getText().c_str());
			hTempObj->SetParam(WWD::Param_MaxY, nval == 0 ? 450 : nval);
		}
	}

	void cEditObjSpringboard::_Think(bool bMouseConsumed)
	{
		if (!bPicking) return;
		float mx, my;
		hge->Input_GetMousePos(&mx, &my);

		if (bMouseConsumed || hState->conMain->getWidgetAt(mx, my) != hState->vPort->GetWidget()) {
			iPickY = (hTempObj->GetParam(WWD::Param_MaxY) == 0 ? 450 : hTempObj->GetParam(WWD::Param_MaxY));
			return;
		}

		int newpick = hTempObj->GetParam(WWD::Param_LocationY) - hState->Scr2WrdY(hState->GetActivePlane(), my);

		if (newpick > 0) {
			hState->bShowHand = 1;
			iPickY = newpick;
			if (hge->Input_KeyDown(HGEK_LBUTTON)) {
				char tmp[32];
				sprintf(tmp, "%d", iPickY);
				tfJumpHeight->setText(tmp, 1);
				butPick->simulatePress();
			}
		}
		else {
			iPickY = (hTempObj->GetParam(WWD::Param_MaxY) == 0 ? 450 : hTempObj->GetParam(WWD::Param_MaxY));
			return;
		}
	}
}
