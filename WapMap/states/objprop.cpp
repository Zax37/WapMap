#include "objprop.h"

#include "../returncodes.h"
#include "../globals.h"
#include "../langID.h"
#include "../cObjectUserData.h"
#include "../../shared/gcnWidgets/wTab.h"
#include "../databanks/logics.h"
#include "codeEditor.h"
#include <math.h>

#include "editing_ww.h"
#include "../../shared/commonFunc.h"
#include "../databanks/imageSets.h"
#include "../databanks/anims.h"

extern HGE * hge;

namespace State
{

	void cObjectPropAL::action(const gcn::ActionEvent &actionEvent)
	{
		if (actionEvent.getSource() == m_hOwn->butSave) {
			bool logicfound = 0;
			if (m_hOwn->rbLogicStandard->isSelected()) {
				for (size_t i = 0; i < m_hOwn->getNumberOfElements(); i++)
					if (m_hOwn->getElementAt(i).compare(m_hOwn->tddadvLogic->getText()) == 0) {
						logicfound = 1;
						break;
					}
				if (!logicfound) {
					int r = MessageBox(hge->System_GetState(HGE_HWND), GETL2S("ObjectProperties", "NoLogicStandard"), "WapMap", MB_YESNO | MB_ICONWARNING);
					if (r == IDNO) return;
				}
			}
			else {
				for (size_t i = 0; i < GV->editState->hCustomLogics->getNumberOfElements(); i++)
					if (GV->editState->hCustomLogics->getElementAt(i).compare(m_hOwn->tddadvCustomLogic->getText()) == 0) {
						logicfound = 1;
						break;
					}
				if (!logicfound) {
					int r = MessageBox(hge->System_GetState(HGE_HWND), GETL2S("ObjectProperties", "NoLogicCustom"), "WapMap", MB_YESNO | MB_ICONWARNING);
					if (r == IDYES) {
						m_hOwn->bLogicEditQuit = 1;
						GV->StateMgr->Push(new State::CodeEditor(0, 0, m_hOwn->tddadvCustomLogic->getText()));
					}
				}
			}
			if (!m_hOwn->bLogicEditQuit) {
				m_hOwn->Save();
				GetUserDataFromObj(m_hOwn->hObj)->SyncToObj();
				m_hOwn->hState->vPort->MarkToRedraw(1);
				m_hOwn->bKill = 1;
			}
		}
		else if (actionEvent.getSource() == m_hOwn->butCustomLogicEdit) {
			cCustomLogic * handle = GV->editState->hCustomLogics->GetLogicByName(m_hOwn->tddadvCustomLogic->getText().c_str());
			if (handle != 0) {
				GV->StateMgr->Push(new State::CodeEditor(handle, 0, ""));
			}
			else {
				int r = MessageBox(hge->System_GetState(HGE_HWND), GETL2S("ObjectProperties", "NoLogicCustom"), "WapMap", MB_YESNO | MB_ICONWARNING);
				if (r == IDYES) {

					GV->StateMgr->Push(new State::CodeEditor(0, 0, m_hOwn->tddadvCustomLogic->getText()));
				}
			}
			//}else if( actionEvent.getSource() == m_hOwn->butPickMinMaxXY ){
			// m_hOwn->bSwap = 1;
		}
		else if (actionEvent.getSource() == m_hOwn->rbLogicStandard || actionEvent.getSource() == m_hOwn->rbLogicCustom) {
			m_hOwn->tfadvName->setEnabled(m_hOwn->rbLogicStandard->isSelected());
			m_hOwn->tddadvLogic->setVisible(m_hOwn->rbLogicStandard->isSelected());
			m_hOwn->tddadvCustomLogic->setVisible(m_hOwn->rbLogicCustom->isSelected());
			m_hOwn->butCustomLogicEdit->setEnabled(m_hOwn->rbLogicCustom->isSelected());

			if (actionEvent.getSource() == m_hOwn->rbLogicStandard) {
				m_hOwn->tfadvName->setText(m_hOwn->strNameHolder);
				m_hOwn->tddadvLogic->setText(m_hOwn->tddadvCustomLogic->getText());
			}
			else {
				m_hOwn->strNameHolder = m_hOwn->tfadvName->getText();
				m_hOwn->tfadvName->setText(GETL2S("ObjectProperties", "CustomLogicNameNotify"));
				m_hOwn->tddadvCustomLogic->setText(m_hOwn->tddadvLogic->getText());
			}
		}
		else if (actionEvent.getSource() == m_hOwn->guirectMinMax->butPick) {
			m_hOwn->bSwap = 1;
			m_hOwn->iPickType = PickRect_MinMax;
		}
		else if (actionEvent.getSource() == m_hOwn->guirectUser[0]->butPick) {
			m_hOwn->bSwap = 1;
			m_hOwn->iPickType = PickRect_User1;
		}
		else if (actionEvent.getSource() == m_hOwn->guirectUser[1]->butPick) {
			m_hOwn->bSwap = 1;
			m_hOwn->iPickType = PickRect_User2;
		}
		else if (actionEvent.getSource() == m_hOwn->guirectAttack->butPick) {
			m_hOwn->bSwap = 1;
			m_hOwn->iPickType = PickRect_Attack;
		}
		else if (actionEvent.getSource() == m_hOwn->guirectMove->butPick) {
			m_hOwn->bSwap = 1;
			m_hOwn->iPickType = PickRect_Move;
		}
		else if (actionEvent.getSource() == m_hOwn->guirectClip->butPick) {
			m_hOwn->bSwap = 1;
			m_hOwn->iPickType = PickRect_Clip;
		}
		else if (actionEvent.getSource() == m_hOwn->guirectHit->butPick) {
			m_hOwn->bSwap = 1;
			m_hOwn->iPickType = PickRect_Hit;
		}
		else if (actionEvent.getSource() == m_hOwn->butCancel || actionEvent.getSource() == m_hOwn->window) {
			m_hOwn->bKill = 1;
			m_hOwn->bCanceled = 1;
		}
		else if (actionEvent.getSource() == m_hOwn->butSelListImageSet) {
			m_hOwn->window->setVisible(0);
			m_hOwn->winisPick->setVisible(1);
			m_hOwn->winisPick->setPosition(hge->System_GetState(HGE_SCREENWIDTH) / 2 - m_hOwn->winisPick->getWidth() / 2,
				hge->System_GetState(HGE_SCREENHEIGHT) / 2 - m_hOwn->winisPick->getHeight() / 2);
			m_hOwn->asImageSetPick = GV->editState->SprBank->GetAssetByID(m_hOwn->tddadvImageSet->getText().c_str());
			m_hOwn->iisPickIT = 0;
			m_hOwn->asimgImageSetFrame = NULL;
			if (!m_hOwn->asImageSetPick) {
				m_hOwn->asImageSetPick = GV->editState->SprBank->GetAssetByIterator(0);
				m_hOwn->iisFrameIT = 0;
			}
			else {
				for (int i = 0; i < GV->editState->SprBank->GetAssetsCount(); i++)
					if (GV->editState->SprBank->GetAssetByIterator(i) == m_hOwn->asImageSetPick) {
						m_hOwn->iisPickIT = i;
						break;
					}
				m_hOwn->asimgImageSetFrame = m_hOwn->asImageSetPick->GetIMGByNoID(atoi(m_hOwn->tfadvLocI->getText().c_str()));
			}
			if (!m_hOwn->asimgImageSetFrame)
				m_hOwn->asimgImageSetFrame = m_hOwn->asImageSetPick->GetIMGByIterator(0);
			else {
				for (int i = 0; i < m_hOwn->asImageSetPick->GetSpritesCount(); i++)
					if (m_hOwn->asimgImageSetFrame == m_hOwn->asImageSetPick->GetIMGByIterator(i)) {
						m_hOwn->iisFrameIT = i;
						break;
					}
			}
			int scaleend = std::max(0, m_hOwn->asImageSetPick->GetSpritesCount() * 138 - 790);
			m_hOwn->sliisFrame->setScale(0, scaleend);
			m_hOwn->sliisFrame->setEnabled(scaleend != 0);
			m_hOwn->sliisPick->setValue(std::max(0, std::min(int(m_hOwn->sliisPick->getScaleEnd() - 790), int((m_hOwn->iisPickIT) * 138 - 395 + 64))));
			m_hOwn->sliisFrame->setValue(std::max(0, std::min(int(m_hOwn->sliisFrame->getScaleEnd() - 790), int((m_hOwn->iisFrameIT) * 138 - 395 + 64))));
		}
		else if (actionEvent.getSource() == m_hOwn->winisPick) {
			m_hOwn->window->setVisible(1);
			m_hOwn->winisPick->setVisible(0);
			m_hOwn->window->setPosition(hge->System_GetState(HGE_SCREENWIDTH) / 2 - m_hOwn->window->getWidth() / 2,
				hge->System_GetState(HGE_SCREENHEIGHT) / 2 - m_hOwn->window->getHeight() / 2);
		}
		else if (actionEvent.getSource() == m_hOwn->butisOK) {
			char tmp[64];
			sprintf(tmp, "%d", (m_hOwn->iisFrameIT == 0 ? -1 : m_hOwn->asimgImageSetFrame->GetID()));
			m_hOwn->tfadvLocI->setText(tmp);
			m_hOwn->tddadvImageSet->setText(m_hOwn->asImageSetPick->GetID());
			action(gcn::ActionEvent(m_hOwn->winisPick, ""));
		}
	}

	void cRectPropGUI::action(const gcn::ActionEvent &actionEvent)
	{
		if (actionEvent.getSource() == butClear) {
			SetRect(WWD::Rect(0, 0, 0, 0));
		}
	}

	cRectPropGUI::cRectPropGUI(WWD::Rect * src, SHR::Contener * dest, int dx, int dy, const char * desc)
	{
		labDesc = new SHR::Lab(desc);
		labDesc->adjustSize();
		dest->add(labDesc, dx, dy);

		labX1 = new SHR::Lab("X 1:");
		labX1->adjustSize();
		dest->add(labX1, dx, dy + 20);

		labY1 = new SHR::Lab("Y 1:");
		labY1->adjustSize();
		dest->add(labY1, dx + 90, dy + 20);

		labX2 = new SHR::Lab("X 2:");
		labX2->adjustSize();
		dest->add(labX2, dx, dy + 45);

		labY2 = new SHR::Lab("Y 2:");
		labY2->adjustSize();
		dest->add(labY2, dx + 90, dy + 45);

		char tmp[32];
		sprintf(tmp, "%d", src->x1);
		tfX1 = new SHR::TextField(tmp);
		tfX1->adjustHeight();
		tfX1->setWidth(45);
		tfX1->SetNumerical(1);
		dest->add(tfX1, dx + 35, dy + 20);

		sprintf(tmp, "%d", src->y1);
		tfY1 = new SHR::TextField(tmp);
		tfY1->adjustHeight();
		tfY1->setWidth(45);
		tfY1->SetNumerical(1);
		dest->add(tfY1, dx + 125, dy + 20);

		sprintf(tmp, "%d", src->x2);
		tfX2 = new SHR::TextField(tmp);
		tfX2->adjustHeight();
		tfX2->setWidth(45);
		tfX2->SetNumerical(1);
		dest->add(tfX2, dx + 35, dy + 45);

		sprintf(tmp, "%d", src->y2);
		tfY2 = new SHR::TextField(tmp);
		tfY2->adjustHeight();
		tfY2->setWidth(45);
		tfY2->SetNumerical(1);
		dest->add(tfY2, dx + 125, dy + 45);

		butPick = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Warp", "Pick"));
		butPick->setDimension(gcn::Rectangle(0, 0, 88, 25));
		dest->add(butPick, dx + 175, dy + 7);

		butClear = new SHR::But(GV->hGfxInterface, GETL2S("Various", "Clear"));
		butClear->setDimension(gcn::Rectangle(0, 0, 88, 25));
		butClear->addActionListener(this);
		dest->add(butClear, dx + 175, dy + 30 + 7);
	}

	cRectPropGUI::~cRectPropGUI()
	{
		delete tfX1; delete tfX2; delete tfY1; delete tfY2; delete labDesc; delete labX1; delete labY1; delete labX2; delete labY2;
		delete butPick; delete butClear;
	}

	void cRectPropGUI::SetRect(WWD::Rect r)
	{
		char tmp[128];
		sprintf(tmp, "%d", r.x1);
		tfX1->setText(tmp);
		sprintf(tmp, "%d", r.y1);
		tfY1->setText(tmp);
		sprintf(tmp, "%d", r.x2);
		tfX2->setText(tmp);
		sprintf(tmp, "%d", r.y2);
		tfY2->setText(tmp);
	}

	WWD::Rect cRectPropGUI::GetRect()
	{
		WWD::Rect ret;
		ret.x1 = atoi(tfX1->getText().c_str());
		ret.y1 = atoi(tfY1->getText().c_str());
		ret.x2 = atoi(tfX2->getText().c_str());
		ret.y2 = atoi(tfY2->getText().c_str());
		return ret;
	}

	void cObjPropVP::Draw(int iCode)
	{
		if (iCode == OBJVP_ADV) {
			int dx, dy;
			m_hOwn->GetWindow()->getAbsolutePosition(dx, dy);
			dy += 24;

			//hge->Gfx_SetClipping();
			hge->Gfx_RenderLine(dx + 5, dy + 5, dx + m_hOwn->window->getWidth(), dy + 5, 0xFF1f1f1f);
			hge->Gfx_RenderLine(dx + 5, dy + 6, dx + m_hOwn->window->getWidth(), dy + 6, 0xFF5c5c5c);

			//position upper separator
			/*hge->Gfx_RenderLine(dx, dy+189, dx+278+95, dy+189, 0xFF1f1f1f);
			hge->Gfx_RenderLine(dx, dy+190, dx+278+95, dy+190, 0xFF5c5c5c);
			//position upper separator
			hge->Gfx_RenderLine(dx, dy+240, dx+278+95, dy+240, 0xFF1f1f1f);
			hge->Gfx_RenderLine(dx, dy+241, dx+278+95, dy+241, 0xFF5c5c5c);*/

			//user values upper separator
			hge->Gfx_RenderLine(dx, dy + 496, dx + 714, dy + 496, 0xFF1f1f1f);
			hge->Gfx_RenderLine(dx, dy + 497, dx + 714, dy + 497, 0xFF5c5c5c);

			//add. flags upper separator
			hge->Gfx_RenderLine(dx + 278 + 95, dy + 108 + 5, dx + 715, dy + 108 + 5, 0xFF1f1f1f);
			hge->Gfx_RenderLine(dx + 278 + 95, dy + 109 + 5, dx + 715, dy + 109 + 5, 0xFF5c5c5c);

			//add.flags lower separator
			hge->Gfx_RenderLine(dx + 278 + 95, dy + 198 + 5, dx + 715, dy + 198 + 5, 0xFF1f1f1f);
			hge->Gfx_RenderLine(dx + 278 + 95, dy + 199 + 5, dx + 715, dy + 199 + 5, 0xFF5c5c5c);

			//user flags upper separator
			//hge->Gfx_RenderLine(dx+278+95, dy+468+5, dx+715, dy+468+5, 0xFF1f1f1f);
			//hge->Gfx_RenderLine(dx+278+95, dy+469+5, dx+715, dy+469+5, 0xFF5c5c5c);

			//rect separators
			for (int i = 0; i < 7; i++) {
				hge->Gfx_RenderLine(dx + 715, dy + 78 + i * 74, dx + m_hOwn->window->getWidth(), dy + 78 + i * 74, 0xFF1f1f1f);
				hge->Gfx_RenderLine(dx + 715, dy + 79 + i * 74, dx + m_hOwn->window->getWidth(), dy + 79 + i * 74, 0xFF5c5c5c);
			}

			//vert flags left separator
			hge->Gfx_RenderLine(dx + 278 + 95, dy, dx + 278 + 95, dy + 315, 0xFF1f1f1f);
			hge->Gfx_RenderLine(dx + 278 + 96, dy, dx + 278 + 96, dy + 315, 0xFF5c5c5c);

			//vert flags right separator
			hge->Gfx_RenderLine(dx + 715, dy, dx + 715, dy + 620, 0xFF1f1f1f);
			hge->Gfx_RenderLine(dx + 716, dy, dx + 716, dy + 620, 0xFF5c5c5c);

			//dynamic and draw flags vertical separator
			hge->Gfx_RenderLine(dx + 563 - 28, dy, dx + 563 - 28, dy + 112, 0xFF1f1f1f);
			hge->Gfx_RenderLine(dx + 564 - 28, dy, dx + 564 - 28, dy + 112, 0xFF5c5c5c);

			//type flags vert sep
			hge->Gfx_RenderLine(dx + 563 - 23, dy + 203, dx + 563 - 23, dy + 496, 0xFF1f1f1f);
			hge->Gfx_RenderLine(dx + 564 - 23, dy + 203, dx + 564 - 23, dy + 496, 0xFF5c5c5c);
		}
		else if (iCode == OBJVP_ISPICK) {
			float mx, my;
			hge->Input_GetMousePos(&mx, &my);
			int dx, dy;
			m_hOwn->GetPickWindow()->getAbsolutePosition(dx, dy);
			dy += 24;
			int w = m_hOwn->GetPickWindow()->getWidth();

			hgeQuad q;
			q.tex = 0;
			q.blend = BLEND_DEFAULT;
			SHR::SetQuad(&q, 0xFF3e3e3e, dx + 5, dy + 25, dx + w - 5, dy + 193 - 5);
			hge->Gfx_RenderQuad(&q);
			SHR::SetQuad(&q, 0xFF3e3e3e, dx + 5, dy + 233, dx + w - 5, dy + 406);
			hge->Gfx_RenderQuad(&q);

			for (int z = 0; z < 2; z++) {
				int move = (z ? m_hOwn->sliisFrame->getValue() : m_hOwn->sliisPick->getValue());
				int istart = move / 138.0f;
				int iend = std::min(istart + 7, (z ? m_hOwn->asImageSetPick->GetSpritesCount()
					: GV->editState->SprBank->GetAssetsCount()));
				hge->Gfx_SetClipping(dx + 5, (z ? dy + 233 : dy + 25), w - 10, 163);
				if (mx > dx + 5 && mx < dx + w - 5 && my >(z ? dy + 233 : dy + 25) && my < (z ? dy + 233 : dy + 25) + 163) {
					if (z) m_hOwn->sliisFrame->setValue(m_hOwn->sliisFrame->getValue() + hge->Input_GetMouseWheel() * 64);
					else    m_hOwn->sliisPick->setValue(m_hOwn->sliisPick->getValue() + hge->Input_GetMouseWheel() * 64);
				}
				for (int i = istart; i < iend; i++) {
					hgeSprite * spr = (z ? m_hOwn->asImageSetPick->GetIMGByIterator(i)->GetSprite()
						: GV->editState->SprBank->GetAssetByIterator(i)->GetIMGByIterator(0)->GetSprite());
					spr->SetColor(0xFFFFFFFF);
					int x = dx + 5 + i * 138 - move + 2;
					int y = (z ? dy + 238
						: dy + 30);
					GV->sprCheckboard->Render4V(x, y, x + 128, y, x + 128, y + 128, x, y + 128);
					if (spr->GetWidth() <= 120 && spr->GetHeight() <= 120)
						spr->Render(x + 64, y + 64);
					else {
						float scale = 1.0f;
						if (spr->GetWidth() > spr->GetHeight())
							scale = 120.0f / spr->GetWidth();
						else
							scale = 120.0f / spr->GetHeight();
						spr->RenderEx(x + 64, y + 64, 0, scale);
					}
					bool bMouseOver = (mx > x && mx < x + 128 && my > y && my < y + 128) &&
						(mx > dx + 5 && mx < dx + w + -5);
					if (bMouseOver) {
						((ObjProp*)GV->StateMgr->GetState())->bRenderHand = 1;
						if (hge->Input_KeyDown(HGEK_LBUTTON)) {
							if (z) {
								m_hOwn->asimgImageSetFrame = m_hOwn->asImageSetPick->GetIMGByIterator(i);
								m_hOwn->iisFrameIT = i;
							}
							else {
								m_hOwn->asImageSetPick = GV->editState->SprBank->GetAssetByIterator(i);
								m_hOwn->iisPickIT = i;
								m_hOwn->asimgImageSetFrame = m_hOwn->asImageSetPick->GetIMGByIterator(0);
								m_hOwn->iisFrameIT = 0;
								m_hOwn->sliisFrame->setValue(0);
							}
						}
					}
					bool condition = (z ? m_hOwn->asImageSetPick->GetIMGByIterator(i) == m_hOwn->asimgImageSetFrame
						: GV->editState->SprBank->GetAssetByIterator(i) == m_hOwn->asImageSetPick);
					if (condition || bMouseOver) {
						hge->Gfx_RenderLine(x, y, x + 128, y, (bMouseOver ? 0xFFFFFF00 : 0xFF00FF00));
						hge->Gfx_RenderLine(x, y + 128, x + 128, y + 128, (bMouseOver ? 0xFFFFFF00 : 0xFF00FF00));
						hge->Gfx_RenderLine(x, y, x, y + 128, (bMouseOver ? 0xFFFFFF00 : 0xFF00FF00));
						hge->Gfx_RenderLine(x + 128, y, x + 128, y + 128, (bMouseOver ? 0xFFFFFF00 : 0xFF00FF00));
					}
					GV->fntMyriad10->SetColor(bMouseOver ? 0xFFFFFF00 : (condition ? 0xFF00FF00 : 0xFF000000));
					GV->fntMyriad10->Render(x + 64, y + 130, HGETEXT_CENTER, (z ? m_hOwn->asImageSetPick->GetIMGByIterator(i)->GetName()
						: GV->editState->SprBank->GetAssetByIterator(i)->GetID()));
				}
			}
			hge->Gfx_SetClipping();

			GV->sprShadeBar->SetFlip(0, 0);
			GV->sprShadeBar->RenderStretch(dx + 5, dy + 25, dx + w - 5, dy + 31);
			GV->sprShadeBar->SetFlip(0, 1);
			GV->sprShadeBar->RenderStretch(dx + 5, dy + 193 - 5 - 6, dx + w - 5, dy + 193 - 5);
			GV->sprShadeBar->SetFlip(0, 0);

			GV->sprShadeBar->SetFlip(0, 0);
			GV->sprShadeBar->RenderStretch(dx + 5, dy + 233, dx + w - 5, dy + 239);
			GV->sprShadeBar->SetFlip(0, 1);
			GV->sprShadeBar->RenderStretch(dx + 5, dy + 406 - 6, dx + w - 5, dy + 406);
			GV->sprShadeBar->SetFlip(0, 0);
		}
	}

	cObjPropVP::cObjPropVP(cObjectProp * owner)
	{
		m_hOwn = owner;
	}


	void cObjectProp::Save()
	{
		int valtmp = 0;
		bool change = 0;

		valtmp = atoi(tfadvID->getText().c_str());
		if (valtmp != hObj->GetParam(WWD::Param_ID)) {
			hObj->SetParam(WWD::Param_ID, valtmp);
			change = 1;
		}

		if (rbLogicStandard->isSelected()) {
			if (strcmp(hObj->GetLogic(), tddadvLogic->getText().c_str()) != 0) {
				hObj->SetLogic(tddadvLogic->getText().c_str());
				change = 1;
			}
			if (strcmp(hObj->GetName(), tfadvName->getText().c_str()) != 0) {
				hObj->SetName(tfadvName->getText().c_str());
				change = 1;
			}
		}
		else {
			if (strcmp(hObj->GetLogic(), "CustomLogic") != 0) {
				hObj->SetLogic("CustomLogic");
				change = 1;
			}
			if (strcmp(hObj->GetName(), tddadvCustomLogic->getText().c_str()) != 0) {
				hObj->SetName(tddadvCustomLogic->getText().c_str());
				change = 1;
			}
		}

		if (strcmp(hObj->GetImageSet(), tddadvImageSet->getText().c_str()) != 0) {
			hObj->SetImageSet(tddadvImageSet->getText().c_str());
			change = 1;
		}
		if (strcmp(hObj->GetAnim(), tddadvAnim->getText().c_str()) != 0) {
			hObj->SetAnim(tddadvAnim->getText().c_str());
			change = 1;
		}
		valtmp = atoi(tfadvLocX->getText().c_str());
		if (hObj->GetParam(WWD::Param_LocationX) != valtmp) {
			hObj->SetParam(WWD::Param_LocationX, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvLocY->getText().c_str());
		if (hObj->GetParam(WWD::Param_LocationY) != valtmp) {
			hObj->SetParam(WWD::Param_LocationY, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvLocZ->getText().c_str());
		if (hObj->GetParam(WWD::Param_LocationZ) != valtmp) {
			hObj->SetParam(WWD::Param_LocationZ, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvLocI->getText().c_str());
		if (hObj->GetParam(WWD::Param_LocationI) != valtmp) {
			hObj->SetParam(WWD::Param_LocationI, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvScore->getText().c_str());
		if (hObj->GetParam(WWD::Param_Score) != valtmp) {
			hObj->SetParam(WWD::Param_Score, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvPowerup->getText().c_str());
		if (hObj->GetParam(WWD::Param_Powerup) != valtmp) {
			hObj->SetParam(WWD::Param_Powerup, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvSmarts->getText().c_str());
		if (hObj->GetParam(WWD::Param_Smarts) != valtmp) {
			hObj->SetParam(WWD::Param_Smarts, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvPoints->getText().c_str());
		if (hObj->GetParam(WWD::Param_Points) != valtmp) {
			hObj->SetParam(WWD::Param_Points, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvHealth->getText().c_str());
		if (hObj->GetParam(WWD::Param_Health) != valtmp) {
			hObj->SetParam(WWD::Param_Health, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvDamage->getText().c_str());
		if (hObj->GetParam(WWD::Param_Damage) != valtmp) {
			hObj->SetParam(WWD::Param_Damage, valtmp);
			change = 1;
		}
		for (int i = 0; i < 8; i++) {
			valtmp = atoi(tfadvUserValue[i]->getText().c_str());
			if (hObj->GetUserValue(i) != valtmp) {
				hObj->SetUserValue(i, valtmp);
				change = 1;
			}
		}

		if (guirectMove->GetRect() != hObj->GetMoveRect()) {
			hObj->SetMoveRect(guirectMove->GetRect());
			change = 1;
		}
		if (guirectHit->GetRect() != hObj->GetHitRect()) {
			hObj->SetHitRect(guirectHit->GetRect());
			change = 1;
		}
		if (guirectAttack->GetRect() != hObj->GetAttackRect()) {
			hObj->SetAttackRect(guirectAttack->GetRect());
			change = 1;
		}
		if (guirectClip->GetRect() != hObj->GetClipRect()) {
			hObj->SetClipRect(guirectClip->GetRect());
			change = 1;
		}
		for (int i = 0; i < 2; i++) {
			if (guirectUser[i]->GetRect() != hObj->GetUserRect(i)) {
				hObj->SetUserRect(i, guirectUser[i]->GetRect());
				change = 1;
			}
		}

		valtmp = guirectMinMax->GetRect().x1;
		if (hObj->GetParam(WWD::Param_MinX) != valtmp) {
			hObj->SetParam(WWD::Param_MinX, valtmp);
			change = 1;
		}
		valtmp = guirectMinMax->GetRect().y1;
		if (hObj->GetParam(WWD::Param_MinY) != valtmp) {
			hObj->SetParam(WWD::Param_MinY, valtmp);
			change = 1;
		}
		valtmp = guirectMinMax->GetRect().x2;
		if (hObj->GetParam(WWD::Param_MaxX) != valtmp) {
			hObj->SetParam(WWD::Param_MaxX, valtmp);
			change = 1;
		}
		valtmp = guirectMinMax->GetRect().y2;
		if (hObj->GetParam(WWD::Param_MaxY) != valtmp) {
			hObj->SetParam(WWD::Param_MaxY, valtmp);
			change = 1;
		}

		valtmp = atoi(tfadvSpeedX->getText().c_str());
		if (hObj->GetParam(WWD::Param_SpeedX) != valtmp) {
			hObj->SetParam(WWD::Param_SpeedX, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvSpeedY->getText().c_str());
		if (hObj->GetParam(WWD::Param_SpeedY) != valtmp) {
			hObj->SetParam(WWD::Param_SpeedY, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvTweakX->getText().c_str());
		if (hObj->GetParam(WWD::Param_TweakX) != valtmp) {
			hObj->SetParam(WWD::Param_TweakX, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvTweakY->getText().c_str());
		if (hObj->GetParam(WWD::Param_TweakY) != valtmp) {
			hObj->SetParam(WWD::Param_TweakY, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvCounter->getText().c_str());
		if (hObj->GetParam(WWD::Param_Counter) != valtmp) {
			hObj->SetParam(WWD::Param_Counter, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvSpeed->getText().c_str());
		if (hObj->GetParam(WWD::Param_Speed) != valtmp) {
			hObj->SetParam(WWD::Param_Speed, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvWidth->getText().c_str());
		if (hObj->GetParam(WWD::Param_Width) != valtmp) {
			hObj->SetParam(WWD::Param_Width, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvHeight->getText().c_str());
		if (hObj->GetParam(WWD::Param_Height) != valtmp) {
			hObj->SetParam(WWD::Param_Height, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvDirection->getText().c_str());
		if (hObj->GetParam(WWD::Param_Direction) != valtmp) {
			hObj->SetParam(WWD::Param_Direction, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvFaceDir->getText().c_str());
		if (hObj->GetParam(WWD::Param_FaceDir) != valtmp) {
			hObj->SetParam(WWD::Param_FaceDir, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvTimeDelay->getText().c_str());
		if (hObj->GetParam(WWD::Param_TimeDelay) != valtmp) {
			hObj->SetParam(WWD::Param_TimeDelay, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvFrameDelay->getText().c_str());
		if (hObj->GetParam(WWD::Param_FrameDelay) != valtmp) {
			hObj->SetParam(WWD::Param_FrameDelay, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvMoveResX->getText().c_str());
		if (hObj->GetParam(WWD::Param_MoveResX) != valtmp) {
			hObj->SetParam(WWD::Param_MoveResX, valtmp);
			change = 1;
		}
		valtmp = atoi(tfadvMoveResY->getText().c_str());
		if (hObj->GetParam(WWD::Param_MoveResY) != valtmp) {
			hObj->SetParam(WWD::Param_MoveResY, valtmp);
			change = 1;
		}

		int flag = 0;
		if (cbadvDraw_No->isSelected())
			flag += WWD::Flag_dr_NoDraw;
		if (cbadvDraw_Mirror->isSelected())
			flag += WWD::Flag_dr_Mirror;
		if (cbadvDraw_Invert->isSelected())
			flag += WWD::Flag_dr_Invert;
		if (cbadvDraw_Flash->isSelected())
			flag += WWD::Flag_dr_Flash;
		if (hObj->GetDrawFlags() != (WWD::OBJ_DRAW_FLAGS)flag) {
			hObj->SetDrawFlags((WWD::OBJ_DRAW_FLAGS)flag);
			change = 1;
		}

		flag = 0;
		if (cbadvDynamic_NoHit->isSelected())
			flag += WWD::Flag_dy_NoHit;
		if (cbadvDynamic_AlwaysActive->isSelected())
			flag += WWD::Flag_dy_AlwaysActive;
		if (cbadvDynamic_Safe->isSelected())
			flag += WWD::Flag_dy_Safe;
		if (cbadvDynamic_AutoHitDmg->isSelected())
			flag += WWD::Flag_dy_AutoHitDamage;
		if (hObj->GetDynamicFlags() != (WWD::OBJ_DYNAMIC_FLAGS)flag) {
			hObj->SetDynamicFlags((WWD::OBJ_DYNAMIC_FLAGS)flag);
			change = 1;
		}

		flag = 0;
		if (cbadvAdd_Difficult->isSelected())
			flag += WWD::Flag_a_Difficult;
		if (cbadvAdd_EyeCandy->isSelected())
			flag += WWD::Flag_a_EyeCandy;
		if (cbadvAdd_HighDetail->isSelected())
			flag += WWD::Flag_a_HighDetail;
		if (cbadvAdd_Multiplayer->isSelected())
			flag += WWD::Flag_a_Multiplayer;
		if (cbadvAdd_ExtraMemory->isSelected())
			flag += WWD::Flag_a_ExtraMemory;
		if (cbadvAdd_FastCPU->isSelected())
			flag += WWD::Flag_a_FastCPU;
		if (hObj->GetAddFlags() != (WWD::OBJ_ADD_FLAGS)flag) {
			hObj->SetAddFlags((WWD::OBJ_ADD_FLAGS)flag);
			change = 1;
		}

		flag = 0;
		for (int i = 0; i < 12; i++) {
			if (cbadvHitType[i]->isSelected())
				flag += pow(2, i);
		}
		if (hObj->GetHitTypeFlags() != (WWD::OBJ_TYPE_FLAGS)flag) {
			hObj->SetHitTypeFlags((WWD::OBJ_TYPE_FLAGS)flag);
			change = 1;
		}

		{
			int flagval = ddAdvType->getSelected() == 0 ? 0
				: pow(2, ddAdvType->getSelected() - 1);
			if (hObj->GetTypeFlags() != (WWD::OBJ_TYPE_FLAGS)flagval) {
				change = 1;
				hObj->SetTypeFlags((WWD::OBJ_TYPE_FLAGS)flagval);
			}
		}

		flag = 0;
		for (int i = 0; i < 12; i++) {
			if (cbadvUser[i]->isSelected())
				flag += pow(2, i);
		}
		if (hObj->GetUserFlags() != (WWD::OBJ_USER_FLAGS)flag) {
			hObj->SetUserFlags((WWD::OBJ_USER_FLAGS)flag);
			change = 1;
		}

		if (change)
			hState->MarkUnsaved();
	}

	cObjectProp::cObjectProp(WWD::Object * obj, bool vis, State::EditingWW * st, bool addtabs)
	{
		bKill = bCanceled = bSwap = bLogicEditQuit = 0;
		hAL = new cObjectPropAL(this);
		hObj = obj;
		hState = st;

		vpCB = new cObjPropVP(this);

		char tmp[256];
		if (obj->GetName()[0] != '\0')
			sprintf(tmp, "%s (#%d %s)", GETL(Lang_ObjectProperties), obj->GetParam(WWD::Param_ID), obj->GetName());
		else
			sprintf(tmp, "%s (#%d)", GETL(Lang_ObjectProperties), obj->GetParam(WWD::Param_ID));
		window = new SHR::Win(&GV->gcnParts, tmp);
		window->setDimension(gcn::Rectangle(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 997 / 2, hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 590 / 2, 997, 590));
		//window->setMovable(0);
		if (vis)
			window->setVisible(0);
		window->setVisible(vis);
		window->setClose(1);
		window->addActionListener(hAL);

		vpAdv = new WIDG::Viewport(vpCB, OBJVP_ADV);
		window->add(vpAdv, 0, 0);


		//-----------------------------MAIN---------------------------------
		labadvID = new SHR::Lab(GETL(Lang_ID));
		labadvID->adjustSize();
		window->add(labadvID, 5, 20);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_ID));
		tfadvID = new SHR::TextField(tmp);
		tfadvID->SetNumerical(1);
		tfadvID->setDimension(gcn::Rectangle(0, 0, 240, 20));
		window->add(tfadvID, 125, 20);

		labadvName = new SHR::Lab(GETL(Lang_Name));
		labadvName->adjustSize();
		window->add(labadvName, 5, 45);
		tfadvName = new SHR::TextField(obj->GetName());
		tfadvName->setDimension(gcn::Rectangle(0, 0, 240, 20));
		window->add(tfadvName, 125, 45);

		labadvLogicType = new SHR::Lab(GETL2S("ObjectProperties", "LogicType"));
		labadvLogicType->adjustSize();
		window->add(labadvLogicType, 5, 70);

		rbLogicStandard = new SHR::RadBut(GV->hGfxInterface, GETL2S("ObjectProperties", "LogicStandard"), "logicType");
		rbLogicStandard->adjustSize();
		rbLogicStandard->addActionListener(hAL);
		rbLogicStandard->setSelected(strcmp(obj->GetLogic(), "CustomLogic") != 0);
		window->add(rbLogicStandard, 125, 70);
		rbLogicCustom = new SHR::RadBut(GV->hGfxInterface, GETL2S("ObjectProperties", "LogicCustom"), "logicType");
		rbLogicCustom->adjustSize();
		rbLogicCustom->addActionListener(hAL);
		rbLogicCustom->setSelected(strcmp(obj->GetLogic(), "CustomLogic") == 0);
		window->add(rbLogicCustom, 245, 70);
		tfadvName->setEnabled(rbLogicStandard->isSelected());
		if (rbLogicCustom->isSelected())
			tfadvName->setText(GETL2S("ObjectProperties", "CustomLogicNameNotify"));

		labadvLogic = new SHR::Lab(GETL(Lang_Logic));
		labadvLogic->adjustSize();
		window->add(labadvLogic, 5, 95);

		tddadvLogic = new SHR::TextDropDown("", this);
		tddadvLogic->setDimension(gcn::Rectangle(0, 0, 240, 20));
		tddadvLogic->SetGfx(&GV->gcnParts);
		tddadvLogic->addActionListener(hAL);
		tddadvLogic->setText(obj->GetLogic());
		window->add(tddadvLogic, 125, 95);
		tddadvLogic->setVisible(rbLogicStandard->isSelected());

		tddadvCustomLogic = new SHR::TextDropDown("", GV->editState->hCustomLogics);
		tddadvCustomLogic->setDimension(gcn::Rectangle(0, 0, 240, 20));
		tddadvCustomLogic->SetGfx(&GV->gcnParts);
		tddadvCustomLogic->addActionListener(hAL);
		tddadvCustomLogic->setText(rbLogicCustom->isSelected() ? obj->GetName()
			: "");
		window->add(tddadvCustomLogic, 125, 95);
		tddadvCustomLogic->setVisible(rbLogicCustom->isSelected());

		butCustomLogicEdit = new SHR::But(GV->hGfxInterface, GETL2S("ObjectProperties", "CustomLogicEdit"));
		butCustomLogicEdit->setIcon(GV->sprIcons16[Icon16_Pencil]);
		butCustomLogicEdit->setDimension(gcn::Rectangle(0, 0, 115, 25));
		butCustomLogicEdit->addActionListener(hAL);
		window->add(butCustomLogicEdit, 125, 120);
		butCustomLogicEdit->setEnabled(rbLogicCustom->isSelected());

		labadvImageSet = new SHR::Lab(GETL(Lang_Graphic));
		labadvImageSet->adjustSize();
		window->add(labadvImageSet, 5, 150);

		tddadvImageSet = new SHR::TextDropDown("", hState->SprBank);
		tddadvImageSet->setDimension(gcn::Rectangle(0, 0, 240, 20));
		tddadvImageSet->SetGfx(&GV->gcnParts);
		tddadvImageSet->addActionListener(hAL);
		tddadvImageSet->setText(obj->GetImageSet());
		window->add(tddadvImageSet, 125, 150);

		butSelListImageSet = new SHR::But(GV->hGfxInterface, GETL(Lang_SelFromList));
		butSelListImageSet->setDimension(gcn::Rectangle(0, 0, 240, 33));
		butSelListImageSet->addActionListener(hAL);
		window->add(butSelListImageSet, 125, 183);

		labadvAnim = new SHR::Lab(GETL(Lang_Anim));
		labadvAnim->adjustSize();
		window->add(labadvAnim, 5, 225);

		tddadvAnim = new SHR::TextDropDown("", hState->hAniBank);
		tddadvAnim->setDimension(gcn::Rectangle(0, 0, 240, 20));
		tddadvAnim->SetGfx(&GV->gcnParts);
		tddadvAnim->addActionListener(hAL);
		tddadvAnim->setText(obj->GetAnim());
		window->add(tddadvAnim, 125, 225);

		//----------------LOCATION-----------------------------------------------------
		int yoffset = 220,
			xoffset = 371;

		sprintf(tmp, "%s X:", GETL(Lang_Location));
		labadvLocX = new SHR::Lab(tmp);
		labadvLocX->adjustSize();
		window->add(labadvLocX, xoffset + 5, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_LocationX));
		tfadvLocX = new SHR::TextField(tmp);
		tfadvLocX->SetNumerical(1);
		tfadvLocX->setDimension(gcn::Rectangle(0, 0, 80, 20));
		window->add(tfadvLocX, xoffset + 84, yoffset);

		yoffset += 25;

		sprintf(tmp, "%s Y:", GETL(Lang_Location));
		labadvLocY = new SHR::Lab(tmp);
		labadvLocY->adjustSize();
		window->add(labadvLocY, xoffset + 5, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_LocationY));
		tfadvLocY = new SHR::TextField(tmp);
		tfadvLocY->SetNumerical(1);
		tfadvLocY->setDimension(gcn::Rectangle(0, 0, 80, 20));
		window->add(tfadvLocY, xoffset + 84, yoffset);

		yoffset += 25;

		sprintf(tmp, "%s Z:", GETL(Lang_Location));
		labadvLocZ = new SHR::Lab(tmp);
		labadvLocZ->adjustSize();
		window->add(labadvLocZ, xoffset + 5, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_LocationZ));
		tfadvLocZ = new SHR::TextField(tmp);
		tfadvLocZ->SetNumerical(1);
		tfadvLocZ->setDimension(gcn::Rectangle(0, 0, 80, 20));
		window->add(tfadvLocZ, xoffset + 84, yoffset);

		yoffset += 25;

		labadvLocI = new SHR::Lab(GETL2S("ObjectProperties", "FrameI"));
		labadvLocI->adjustSize();
		window->add(labadvLocI, xoffset + 5, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_LocationI));
		tfadvLocI = new SHR::TextField(tmp);
		tfadvLocI->SetNumerical(1);
		tfadvLocI->setDimension(gcn::Rectangle(0, 0, 80, 20));
		window->add(tfadvLocI, xoffset + 84, yoffset);


		//----------------------------------------------------PROPERTIES

		xoffset = 5;
		yoffset = 325;

		labadvSpeedX = new SHR::Lab(GETL(Lang_SpeedX));
		labadvSpeedX->adjustSize();
		window->add(labadvSpeedX, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_SpeedX));
		tfadvSpeedX = new SHR::TextField(tmp);
		tfadvSpeedX->SetNumerical(1);
		tfadvSpeedX->adjustSize();
		tfadvSpeedX->setWidth(80);
		window->add(tfadvSpeedX, xoffset + 80, yoffset);

		xoffset += 185;

		labadvSpeedY = new SHR::Lab(GETL(Lang_SpeedY));
		labadvSpeedY->adjustSize();
		window->add(labadvSpeedY, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_SpeedY));
		tfadvSpeedY = new SHR::TextField(tmp);
		tfadvSpeedY->SetNumerical(1);
		tfadvSpeedY->adjustSize();
		tfadvSpeedY->setWidth(80);
		window->add(tfadvSpeedY, xoffset + 80, yoffset);

		xoffset += 185;

		labadvTweakX = new SHR::Lab(GETL(Lang_TweakX));
		labadvTweakX->adjustSize();
		window->add(labadvTweakX, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_TweakX));
		tfadvTweakX = new SHR::TextField(tmp);
		tfadvTweakX->SetNumerical(1);
		tfadvTweakX->adjustSize();
		tfadvTweakX->setWidth(80);
		window->add(tfadvTweakX, xoffset + 80, yoffset);

		xoffset = 5;
		yoffset += 25;

		labadvTweakY = new SHR::Lab(GETL(Lang_TweakY));
		labadvTweakY->adjustSize();
		window->add(labadvTweakY, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_TweakY));
		tfadvTweakY = new SHR::TextField(tmp);
		tfadvTweakY->SetNumerical(1);
		tfadvTweakY->adjustSize();
		tfadvTweakY->setWidth(80);
		window->add(tfadvTweakY, xoffset + 80, yoffset);

		xoffset += 185;

		labadvCounter = new SHR::Lab(GETL(Lang_Counter));
		labadvCounter->adjustSize();
		window->add(labadvCounter, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_Counter));
		tfadvCounter = new SHR::TextField(tmp);
		tfadvCounter->SetNumerical(1);
		tfadvCounter->adjustSize();
		tfadvCounter->setWidth(80);
		window->add(tfadvCounter, xoffset + 80, yoffset);

		xoffset += 185;

		labadvSpeed = new SHR::Lab(GETL(Lang_Speed));
		labadvSpeed->adjustSize();
		window->add(labadvSpeed, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_Speed));
		tfadvSpeed = new SHR::TextField(tmp);
		tfadvSpeed->SetNumerical(1);
		tfadvSpeed->adjustSize();
		tfadvSpeed->setWidth(80);
		window->add(tfadvSpeed, xoffset + 80, yoffset);

		xoffset = 5;
		yoffset += 25;

		labadvWidth = new SHR::Lab(GETL(Lang_Width));
		labadvWidth->adjustSize();
		window->add(labadvWidth, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_Width));
		tfadvWidth = new SHR::TextField(tmp);
		tfadvWidth->SetNumerical(1);
		tfadvWidth->adjustSize();
		tfadvWidth->setWidth(80);
		window->add(tfadvWidth, xoffset + 80, yoffset);

		xoffset += 185;

		labadvHeight = new SHR::Lab(GETL(Lang_Height));
		labadvHeight->adjustSize();
		window->add(labadvHeight, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_Height));
		tfadvHeight = new SHR::TextField(tmp);
		tfadvHeight->SetNumerical(1);
		tfadvHeight->adjustSize();
		tfadvHeight->
			setWidth(80);
		window->add(tfadvHeight, xoffset + 80, yoffset);

		xoffset += 185;

		labadvDirection = new SHR::Lab(GETL(Lang_Direction));
		labadvDirection->adjustSize();
		window->add(labadvDirection, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_Direction));
		tfadvDirection = new SHR::TextField(tmp);
		tfadvDirection->SetNumerical(1);
		tfadvDirection->adjustSize();
		tfadvDirection->setWidth(80);
		window->add(tfadvDirection, xoffset + 80, yoffset);

		xoffset = 5;
		yoffset += 25;

		labadvFaceDir = new SHR::Lab(GETL(Lang_FacingDirection));
		labadvFaceDir->adjustSize();
		window->add(labadvFaceDir, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_FaceDir));
		tfadvFaceDir = new SHR::TextField(tmp);
		tfadvFaceDir->SetNumerical(1);
		tfadvFaceDir->adjustSize();
		tfadvFaceDir->setWidth(80);
		window->add(tfadvFaceDir, xoffset + 80, yoffset);

		xoffset += 185;

		labadvTimeDelay = new SHR::Lab(GETL(Lang_TimeDelay));
		labadvTimeDelay->adjustSize();
		window->add(labadvTimeDelay, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_TimeDelay));
		tfadvTimeDelay = new SHR::TextField(tmp);
		tfadvTimeDelay->SetNumerical(1);
		tfadvTimeDelay->adjustSize();
		tfadvTimeDelay->setWidth(80);
		window->add(tfadvTimeDelay, xoffset + 80, yoffset);

		xoffset += 185;

		labadvFrameDelay = new SHR::Lab(GETL(Lang_FrameDelay));
		labadvFrameDelay->adjustSize();
		window->add(labadvFrameDelay, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_FrameDelay));
		tfadvFrameDelay = new SHR::TextField(tmp);
		tfadvFrameDelay->SetNumerical(1);
		tfadvFrameDelay->adjustSize();
		tfadvFrameDelay->setWidth(80);
		window->add(tfadvFrameDelay, xoffset + 80, yoffset);

		xoffset = 5;
		yoffset += 25;

		labadvMoveResX = new SHR::Lab(GETL(Lang_MoveResX));
		labadvMoveResX->adjustSize();
		window->add(labadvMoveResX, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_MoveResX));
		tfadvMoveResX = new SHR::TextField(tmp);
		tfadvMoveResX->SetNumerical(1);
		tfadvMoveResX->adjustSize();
		tfadvMoveResX->setWidth(80);
		window->add(tfadvMoveResX, xoffset + 80, yoffset);

		xoffset += 185;

		labadvMoveResY = new SHR::Lab(GETL(Lang_MoveResY));
		labadvMoveResY->adjustSize();
		window->add(labadvMoveResY, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_MoveResY));
		tfadvMoveResY = new SHR::TextField(tmp);
		tfadvMoveResY->SetNumerical(1);
		tfadvMoveResY->adjustSize();
		tfadvMoveResY->setWidth(80);
		window->add(tfadvMoveResY, xoffset + 80, yoffset);

		xoffset += 185;

		//----------------------------------------------PROPERTIES
		labadvScore = new SHR::Lab(GETL(Lang_Score));
		labadvScore->adjustSize();
		window->add(labadvScore, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_Score));
		tfadvScore = new SHR::TextField(tmp);
		tfadvScore->SetNumerical(1);
		tfadvScore->adjustSize();
		tfadvScore->setWidth(80);
		window->add(tfadvScore, xoffset + 80, yoffset);

		xoffset = 5;
		yoffset += 25;

		labadvPoints = new SHR::Lab(GETL(Lang_Points));
		labadvPoints->adjustSize();
		window->add(labadvPoints, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_Points));
		tfadvPoints = new SHR::TextField(tmp);
		tfadvPoints->SetNumerical(1);
		tfadvPoints->adjustSize();
		tfadvPoints->setWidth(80);
		window->add(tfadvPoints, xoffset + 80, yoffset);

		xoffset += 185;

		labadvPowerup = new SHR::Lab(GETL(Lang_Powerup));
		labadvPowerup->adjustSize();
		window->add(labadvPowerup, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_Powerup));
		tfadvPowerup = new SHR::TextField(tmp);
		tfadvPowerup->SetNumerical(1);
		tfadvPowerup->adjustSize();
		tfadvPowerup->setWidth(80);
		window->add(tfadvPowerup, xoffset + 80, yoffset);

		xoffset += 185;

		labadvDamage = new SHR::Lab(GETL(Lang_Damage));
		labadvDamage->adjustSize();
		window->add(labadvDamage, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_Damage));
		tfadvDamage = new SHR::TextField(tmp);
		tfadvDamage->SetNumerical(1);
		tfadvDamage->adjustSize();
		tfadvDamage->setWidth(80);
		window->add(tfadvDamage, xoffset + 80, yoffset);

		xoffset = 5;
		yoffset += 25;

		labadvSmarts = new SHR::Lab(GETL(Lang_Smarts));
		labadvSmarts->adjustSize();
		window->add(labadvSmarts, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_Smarts));
		tfadvSmarts = new SHR::TextField(tmp);
		tfadvSmarts->SetNumerical(1);
		tfadvSmarts->adjustSize();
		tfadvSmarts->setWidth(80);
		window->add(tfadvSmarts, xoffset + 80, yoffset);

		xoffset += 185;

		labadvHealth = new SHR::Lab(GETL(Lang_Health));
		labadvHealth->adjustSize();
		window->add(labadvHealth, xoffset, yoffset + 2);

		sprintf(tmp, "%d", obj->GetParam(WWD::Param_Health));
		tfadvHealth = new SHR::TextField(tmp);
		tfadvHealth->SetNumerical(1);
		tfadvHealth->adjustSize();
		tfadvHealth->setWidth(80);
		window->add(tfadvHealth, xoffset + 80, yoffset);

		xoffset = 5;
		yoffset += 30;

		//---------------------------------------------------USER NUMERICAL VALUES
		labadvUserValues = new SHR::Lab(GETL(Lang_UserValues));
		labadvUserValues->adjustSize();
		window->add(labadvUserValues, xoffset, yoffset);

		yoffset += 20;

		for (int i = 0; i < 8; i++) {
			sprintf(tmp, "#%d", i + 1);
			//sprintf(tmp, "%s #%d", GETL(Lang_UserValue), i+1);
			labadvUserValue[i] = new SHR::Lab(tmp);
			labadvUserValue[i]->adjustSize();
			window->add(labadvUserValue[i], xoffset + (i % 4) * 110, yoffset + 2 + (i >= 4) * 25);
			sprintf(tmp, "%d", obj->GetUserValue(i));
			tfadvUserValue[i] = new SHR::TextField(tmp);
			tfadvUserValue[i]->SetNumerical(1);
			tfadvUserValue[i]->adjustSize();
			tfadvUserValue[i]->setWidth(75);
			window->add(tfadvUserValue[i], xoffset + 25 + (i % 4) * 110, yoffset + (i >= 4) * 25);
		}

		yoffset = 15;

		//--------------------------------------------------RECTS
		sprintf(tmp, "%s #%d", GETL(Lang_UserRect), 1);
		WWD::Rect testrect = WWD::Rect(obj->GetParam(WWD::Param_MinX),
			obj->GetParam(WWD::Param_MinY),
			obj->GetParam(WWD::Param_MaxX),
			obj->GetParam(WWD::Param_MaxY));
		guirectMinMax = new cRectPropGUI(&testrect, window, 725, yoffset, GETL2S("Various", "MinMaxRect"));
		guirectMinMax->butPick->addActionListener(hAL);
		yoffset += 74;
		testrect = obj->GetUserRect(0);
		guirectUser[0] = new cRectPropGUI(&testrect, window, 725, yoffset, tmp);
		guirectUser[0]->butPick->addActionListener(hAL);
		yoffset += 74;
		sprintf(tmp, "%s #%d", GETL(Lang_UserRect), 2);
		testrect = obj->GetUserRect(1);
		guirectUser[1] = new cRectPropGUI(&testrect, window, 725, yoffset, tmp);
		guirectUser[1]->butPick->addActionListener(hAL);
		yoffset += 74;
		testrect = obj->GetMoveRect();
		guirectMove = new cRectPropGUI(&testrect, window, 725, yoffset, GETL(Lang_MoveRect));
		guirectMove->butPick->addActionListener(hAL);
		yoffset += 74;
		testrect = obj->GetHitRect();
		guirectHit = new cRectPropGUI(&testrect, window, 725, yoffset, GETL(Lang_HitRect));
		guirectHit->butPick->addActionListener(hAL);
		yoffset += 74;
		testrect = obj->GetAttackRect();
		guirectAttack = new cRectPropGUI(&testrect, window, 725, yoffset, GETL(Lang_AttackRect));
		guirectAttack->butPick->addActionListener(hAL);
		yoffset += 74;
		testrect = obj->GetClipRect();
		guirectClip = new cRectPropGUI(&testrect, window, 725, yoffset, GETL(Lang_ClipRect));
		guirectClip->butPick->addActionListener(hAL);
		yoffset += 74;

		yoffset = 15;
		xoffset = 380;

		//------------------------------DRAW FLAGS
		labadvDrawFlags = new SHR::Lab(GETL(Lang_DrawingFlags));
		labadvDrawFlags->adjustSize();
		window->add(labadvDrawFlags, xoffset, yoffset);

		yoffset += 20;

		cbadvDraw_No = new SHR::CBox(GV->hGfxInterface, GETL(Lang_NoDraw));
		cbadvDraw_No->setSelected((obj->GetDrawFlags() & WWD::Flag_dr_NoDraw));
		cbadvDraw_No->adjustSize();
		window->add(cbadvDraw_No, xoffset, yoffset);
		yoffset += 20;
		cbadvDraw_Mirror = new SHR::CBox(GV->hGfxInterface, GETL(Lang_Mirror));
		cbadvDraw_Mirror->setSelected((obj->GetDrawFlags() & WWD::Flag_dr_Mirror));
		cbadvDraw_Mirror->adjustSize();
		window->add(cbadvDraw_Mirror, xoffset, yoffset);
		yoffset += 20;
		cbadvDraw_Invert = new SHR::CBox(GV->hGfxInterface, GETL(Lang_Invert));
		cbadvDraw_Invert->setSelected((obj->GetDrawFlags() & WWD::Flag_dr_Invert));
		cbadvDraw_Invert->adjustSize();
		window->add(cbadvDraw_Invert, xoffset, yoffset);
		yoffset += 20;
		cbadvDraw_Flash = new SHR::CBox(GV->hGfxInterface, GETL(Lang_Flash));
		cbadvDraw_Flash->setSelected((obj->GetDrawFlags() & WWD::Flag_dr_Flash));
		cbadvDraw_Flash->adjustSize();
		window->add(cbadvDraw_Flash, xoffset, yoffset);
		yoffset -= 80;

		xoffset -= 28;
		//-------------------------------DYNAMIC FLAGS
		labadvDynamicFlags = new SHR::Lab(GETL(Lang_DynamicFlags));
		labadvDynamicFlags->adjustSize();
		window->add(labadvDynamicFlags, xoffset + 190, yoffset);
		yoffset += 20;

		cbadvDynamic_NoHit = new SHR::CBox(GV->hGfxInterface, GETL(Lang_ObjFlag_NoHit));
		cbadvDynamic_NoHit->setSelected((obj->GetDynamicFlags() & WWD::Flag_dy_NoHit));
		cbadvDynamic_NoHit->adjustSize();
		window->add(cbadvDynamic_NoHit, xoffset + 190, yoffset);
		yoffset += 20;
		cbadvDynamic_AlwaysActive = new SHR::CBox(GV->hGfxInterface, GETL(Lang_ObjFlag_AlwaysActive));
		cbadvDynamic_AlwaysActive->setSelected((obj->GetDynamicFlags() & WWD::Flag_dy_AlwaysActive));
		cbadvDynamic_AlwaysActive->adjustSize();
		window->add(cbadvDynamic_AlwaysActive, xoffset + 190, yoffset);
		yoffset += 20;
		cbadvDynamic_Safe = new SHR::CBox(GV->hGfxInterface, GETL(Lang_ObjFlag_Safe));
		cbadvDynamic_Safe->setSelected((obj->GetDynamicFlags() & WWD::Flag_dy_Safe));
		cbadvDynamic_Safe->adjustSize();
		window->add(cbadvDynamic_Safe, xoffset + 190, yoffset);
		yoffset += 20;
		cbadvDynamic_AutoHitDmg = new SHR::CBox(GV->hGfxInterface, GETL(Lang_ObjFlag_AutoHitDmg));
		cbadvDynamic_AutoHitDmg->setSelected((obj->GetDynamicFlags() & WWD::Flag_dy_AutoHitDamage));
		cbadvDynamic_AutoHitDmg->adjustSize();
		window->add(cbadvDynamic_AutoHitDmg, xoffset + 190, yoffset);
		yoffset += 30;
		//--------------------------------------------ADD FLAGS
		xoffset += 28;
		labadvAddFlags = new SHR::Lab(GETL(Lang_AddFlags));
		labadvAddFlags->adjustSize();
		window->add(labadvAddFlags, xoffset, yoffset);
		yoffset += 20;

		cbadvAdd_Difficult = new SHR::CBox(GV->hGfxInterface, GETL(Lang_ObjFlag_Difficult));
		cbadvAdd_Difficult->setSelected((obj->GetAddFlags() & WWD::Flag_a_Difficult));
		cbadvAdd_Difficult->adjustSize();
		window->add(cbadvAdd_Difficult, xoffset, yoffset);
		yoffset += 20;
		cbadvAdd_EyeCandy = new SHR::CBox(GV->hGfxInterface, GETL(Lang_ObjFlag_EyeCandy));
		cbadvAdd_EyeCandy->setSelected((obj->GetAddFlags() & WWD::Flag_a_EyeCandy));
		cbadvAdd_EyeCandy->adjustSize();
		window->add(cbadvAdd_EyeCandy, xoffset, yoffset);
		yoffset += 20;
		cbadvAdd_HighDetail = new SHR::CBox(GV->hGfxInterface, GETL(Lang_ObjFlag_HighDetail));
		cbadvAdd_HighDetail->setSelected((obj->GetAddFlags() & WWD::Flag_a_HighDetail));
		cbadvAdd_HighDetail->adjustSize();
		window->add(cbadvAdd_HighDetail, xoffset, yoffset);
		yoffset -= 40;

		xoffset -= 28;
		cbadvAdd_Multiplayer = new SHR::CBox(GV->hGfxInterface, GETL(Lang_ObjFlag_Multiplayer));
		cbadvAdd_Multiplayer->setSelected((obj->GetAddFlags() & WWD::Flag_a_Multiplayer));
		cbadvAdd_Multiplayer->adjustSize();
		window->add(cbadvAdd_Multiplayer, xoffset + 190, yoffset);
		yoffset += 20;
		cbadvAdd_ExtraMemory = new SHR::CBox(GV->hGfxInterface, GETL(Lang_ObjFlag_ExtraMemory));
		cbadvAdd_ExtraMemory->setSelected((obj->GetAddFlags() & WWD::Flag_a_ExtraMemory));
		cbadvAdd_ExtraMemory->adjustSize();
		window->add(cbadvAdd_ExtraMemory, xoffset + 190, yoffset);
		yoffset += 20;
		cbadvAdd_FastCPU = new SHR::CBox(GV->hGfxInterface, GETL(Lang_ObjFlag_FastCPU));
		cbadvAdd_FastCPU->setSelected((obj->GetAddFlags() & WWD::Flag_a_FastCPU));
		cbadvAdd_FastCPU->adjustSize();
		window->add(cbadvAdd_FastCPU, xoffset + 190, yoffset);
		yoffset += 30;

		//----------------------------------------------TYPE
		xoffset += 33;
		labadvType = new SHR::Lab(GETL(Lang_TypeFlag));
		labadvType->adjustSize();
		window->add(labadvType, xoffset + 190 - 28, yoffset);
		//sprintf(tmp, "%d_type_%d", obj->GetParam(WWD::Param_ID), hge->Random_Int(0,1000));

		yoffset += 20;

		lmAdvType = new gcn::GenericListModel();
		lmAdvType->addElement(std::string(GETL2S("ObjectProperties", "Default")));
		for (int i = 0; i < 12; i++) {
			lmAdvType->addElement(std::string(GETL(Lang_ObjFlag_HitType1 + i)));
		}

		ddAdvType = new SHR::DropDown();
		ddAdvType->setDimension(gcn::Rectangle(0, 0, 165, 20));
		ddAdvType->setListModel(lmAdvType);
		ddAdvType->addActionListener(hAL);
		ddAdvType->SetGfx(&GV->gcnParts);
		window->add(ddAdvType, xoffset + 190 - 28, yoffset);
		if (obj->GetTypeFlags() != 0) {
			for (int i = 0; i < 12; i++)
				if (pow(2, i) == obj->GetTypeFlags()) {
					ddAdvType->setSelected(i + 1);
					break;
				}
		}

		//yoffset -= 20;
		yoffset += 40;

		//----------------------------------------------HITTYPE
		xoffset -= 28;
		labadvHitType = new SHR::Lab(GETL(Lang_HitTypeFlags));
		labadvHitType->adjustSize();
		window->add(labadvHitType, xoffset + 190, yoffset);

		yoffset += 20;

		for (int i = 0; i < 12; i++) {
			cbadvHitType[i] = new SHR::CBox(GV->hGfxInterface, GETL(Lang_ObjFlag_HitType1 + i));
			cbadvHitType[i]->adjustSize();
			if (i > 7)
				window->add(cbadvHitType[i], xoffset + 190 + ((i - 8) % 2) * 80, yoffset + 160 + ((i - 8) / 2) * 20);
			else
				window->add(cbadvHitType[i], xoffset + 190, yoffset + i * 20);
			if (obj->GetHitTypeFlags() & (WWD::OBJ_TYPE_FLAGS)(1 << i)) // 2^i
				cbadvHitType[i]->setSelected(1);
		}

		yoffset += 210;

		//----------------------------------------------USERFLAGS
		xoffset += 28;
		labadvUserFlags = new SHR::Lab(GETL(Lang_UserFlags));
		labadvUserFlags->adjustSize();
		window->add(labadvUserFlags, xoffset + 90 - 28, yoffset);

		yoffset += 20;

		for (int i = 0; i < 12; i++) {
			sprintf(tmp, "#%d", i + 1);
			//sprintf(tmp, "%s #%d", GETL(Lang_UserFlag), i+1);
			cbadvUser[i] = new SHR::CBox(GV->hGfxInterface, tmp);
			cbadvUser[i]->adjustSize();
			window->add(cbadvUser[i], xoffset + 90 - 28 + (i % 6) * 40, yoffset + 2 + (i / 6) * 25);
			if (obj->GetUserFlags() & (WWD::OBJ_USER_FLAGS)(1 << i))  // 2^i
				cbadvUser[i]->setSelected(1);
		}

		//-------------------MAIN BUTTONS---------------------------------------
		butSave = new SHR::But(GV->hGfxInterface, GETL(Lang_Save));
		butSave->addActionListener(hAL);
		butSave->setDimension(gcn::Rectangle(0, 0, 100, 32));
		window->add(butSave, 720 + 33, 538);

		butCancel = new SHR::But(GV->hGfxInterface, GETL(Lang_Cancel));
		butCancel->addActionListener(hAL);
		butCancel->setDimension(gcn::Rectangle(0, 0, 100, 32));
		window->add(butCancel, 825 + 33, 538);

		winisPick = new SHR::Win(&GV->gcnParts, GETL2S("WinObjectProperties", "PickIS_WinCaption"));
		winisPick->setDimension(gcn::Rectangle(0, 0, 800, 500));
		winisPick->setVisible(0);
		winisPick->setClose(1);
		winisPick->addActionListener(hAL);

		vpisPick = new WIDG::Viewport(vpCB, OBJVP_ISPICK);
		window->add(vpisPick, 0, 0);
		winisPick->add(vpisPick, 0, 0);

		sliisPick = new SHR::Slider(0, GV->editState->SprBank->GetAssetsCount() * 138 - 790);
		sliisPick->setDimension(gcn::Rectangle(0, 0, 790, 11));
		sliisPick->setOrientation(SHR::Slider::HORIZONTAL);
		sliisPick->setMarkerLength(40);
		winisPick->add(sliisPick, 5, 200);

		labisImageSet = new SHR::Lab(GETL2S("WinObjectProperties", "PickIS_ImageSet"));
		labisImageSet->adjustSize();
		winisPick->add(labisImageSet, 5, 10);

		labisFrame = new SHR::Lab(GETL2S("WinObjectProperties", "PickIS_Frame"));
		labisFrame->adjustSize();
		winisPick->add(labisFrame, 5, 215);

		sliisFrame = new SHR::Slider(0, 100);
		sliisFrame->setDimension(gcn::Rectangle(0, 0, 790, 11));
		sliisFrame->setOrientation(SHR::Slider::HORIZONTAL);
		sliisFrame->setMarkerLength(40);
		winisPick->add(sliisFrame, 5, 418);

		butisOK = new SHR::But(GV->hGfxInterface, GETL(Lang_OK));
		butisOK->setDimension(gcn::Rectangle(0, 0, 100, 33));
		butisOK->addActionListener(hAL);
		winisPick->add(butisOK, 400 - 50, 440);
	}

	cObjectProp::~cObjectProp()
	{
		//Delete();
		delete labisFrame;
		delete labisImageSet;
		delete sliisFrame;
		delete sliisPick;
		delete butisOK;
		delete hAL;
		delete vpCB;
		delete vpAdv;
		delete vpisPick;
		delete winisPick;
		for (int i = 0; i < 12; i++) {
			delete cbadvHitType[i];
			delete cbadvUser[i];
		}
		delete ddAdvType;
		delete lmAdvType;
		for (int i = 0; i < 8; i++) {
			delete labadvUserValue[i];
			delete tfadvUserValue[i];
		}
		delete butCustomLogicEdit;
		delete tddadvCustomLogic;
		delete guirectUser[0]; delete guirectUser[1]; delete guirectClip; delete guirectAttack; delete guirectHit; delete guirectMove;
		delete butSave;     delete butCancel;
		delete labadvLogicType; delete rbLogicStandard; delete rbLogicCustom;
		delete labadvName;  delete labadvLogic; delete labadvImageSet; delete labadvAnim; delete labadvID; delete labadvLocX; delete labadvLocY; delete labadvLocZ; delete labadvLocI;
		delete labadvDrawFlags; delete labadvDynamicFlags; delete labadvAddFlags; delete labadvHitType; delete labadvType; delete labadvUserFlags;
		delete labadvScore; delete labadvPoints; delete labadvPowerup; delete labadvDamage; delete labadvSmarts; delete labadvHealth; delete labadvUserValues;
		delete labadvSpeedX; delete labadvSpeedY; delete labadvTweakX; delete labadvTweakY; delete labadvCounter;
		delete labadvSpeed; delete labadvWidth; delete labadvHeight; delete labadvDirection; delete labadvFaceDir; delete labadvTimeDelay; delete labadvFrameDelay; delete labadvMoveResX;
		delete labadvMoveResY;
		delete tfadvName; delete tddadvLogic; delete tfadvID; delete tfadvLocX; delete tfadvLocY; delete tfadvLocZ; delete tfadvLocI;
		delete tfadvScore; delete tfadvPoints; delete tfadvPowerup; delete tfadvDamage; delete tfadvSmarts; delete tfadvHealth;
		delete tfadvSpeedX; delete tfadvSpeedY; delete tfadvTweakX; delete tfadvTweakY; delete tfadvCounter; delete tfadvSpeed;
		delete tfadvWidth; delete tfadvHeight; delete tfadvDirection; delete tfadvFaceDir; delete tfadvTimeDelay; delete tfadvFrameDelay; delete tfadvMoveResX; delete tfadvMoveResY;
		delete cbadvDraw_No; delete cbadvDraw_Mirror; delete cbadvDraw_Invert; delete cbadvDraw_Flash; delete cbadvDynamic_NoHit; delete cbadvDynamic_AlwaysActive;
		delete cbadvDynamic_Safe; delete cbadvDynamic_AutoHitDmg; delete cbadvAdd_Difficult; delete cbadvAdd_EyeCandy; delete cbadvAdd_HighDetail; delete cbadvAdd_Multiplayer;
		delete cbadvAdd_ExtraMemory; delete cbadvAdd_FastCPU;
		delete tddadvImageSet; delete tddadvAnim;
		delete window;
	}

};

State::ObjProp::ObjProp(EditingWW * main, WWD::Object * obj, bool bMove)
{
	bMoveObject = bMove;
	gui = new gcn::Gui();
	gui->setGraphics(GV->gcnGraphics);
	gui->setInput(GV->gcnInput);
	conMain = new SHR::Contener();
	conMain->setDimension(gcn::Rectangle(0, 0, hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT)));
	conMain->setOpaque(0);
	gui->setTop(conMain);

	hObj = obj;
	mainSt = main;
}

void State::ObjProp::Init()
{
	hProp = new cObjectProp(hObj, 1, mainSt);
	conMain->add(hProp->GetWindow());
	conMain->add(hProp->GetPickWindow());
}

void State::ObjProp::Destroy()
{
	delete conMain;
	delete gui;
	delete hProp;
}

bool State::ObjProp::Think()
{
	GV->Console->Think();
	try {
		gui->logic();
	}
	catch (gcn::Exception& exc) {
		GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(), exc.getLine());
	}
	if (hProp->Kill() || hge->Input_KeyDown(HGEK_ESCAPE)) {
		returnCode * rc = new returnCode;
		rc->Ptr = bMoveObject;
		if (bMoveObject && hProp->Canceled())
			rc->Ptr = 2;
		rc->Type = RC_ObjectProp;
		_popMe((int)rc);
	}
	else if (hProp->Swap()) {
		returnCode * rc = new returnCode;
		rc->Type = RC_ObjectProp;
		cRectPropGUI * source;
		if (hProp->iPickType == PickRect_MinMax) source = hProp->guirectMinMax;
		else if (hProp->iPickType == PickRect_User1) source = hProp->guirectUser[0];
		else if (hProp->iPickType == PickRect_User2) source = hProp->guirectUser[1];
		else if (hProp->iPickType == PickRect_Move) source = hProp->guirectMove;
		else if (hProp->iPickType == PickRect_Clip) source = hProp->guirectClip;
		else if (hProp->iPickType == PickRect_Hit) source = hProp->guirectHit;
		else if (hProp->iPickType == PickRect_Attack) source = hProp->guirectAttack;
		int * ptr = new int[4];
		ptr[0] = source->GetRect().x1;
		ptr[1] = source->GetRect().y1;
		ptr[2] = source->GetRect().x2;
		ptr[3] = source->GetRect().y2;
		rc->Ptr = (int)(void*)ptr;
		_flipMe((int)rc);
		hProp->SwapDone();
	}
	return 0;
}

bool State::ObjProp::Render()
{
	try {
		gui->draw();
	}
	catch (gcn::Exception& exc) {
		GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(), exc.getLine());
	}
	GV->IF->Render(bRenderHand);
	GV->Console->Render();
	bRenderHand = 0;
	return 0;
}

void State::ObjProp::GainFocus(int iReturnCode, bool bFlipped)
{
	if (bFlipped) {
		if (((returnCode*)iReturnCode)->Type == RC_ObjPropSelectedValues) {
			if (((returnCode*)iReturnCode)->Ptr != 0) {
				int * ret = (int*)(((returnCode*)iReturnCode)->Ptr);
				cRectPropGUI * dest;
				if (hProp->iPickType == PickRect_MinMax) dest = hProp->guirectMinMax;
				else if (hProp->iPickType == PickRect_User1) dest = hProp->guirectUser[0];
				else if (hProp->iPickType == PickRect_User2) dest = hProp->guirectUser[1];
				else if (hProp->iPickType == PickRect_Move) dest = hProp->guirectMove;
				else if (hProp->iPickType == PickRect_Clip) dest = hProp->guirectClip;
				else if (hProp->iPickType == PickRect_Hit) dest = hProp->guirectHit;
				else if (hProp->iPickType == PickRect_Attack) dest = hProp->guirectAttack;
				dest->SetRect(WWD::Rect(ret[0], ret[1], ret[2], ret[3]));
				delete[] ret;
			}
		}
	}
	if (((returnCode*)iReturnCode)->Type == RC_CodeEditor) {
		stCodeEditorRC * rc = (stCodeEditorRC*)(((returnCode*)iReturnCode)->Ptr);
		if (rc->hLogic != 0) {
			if (rc->bSaved) {
				GV->editState->hDataCtrl->FixCustomDir();
				rc->hLogic->Save();
				if (rc->bNewLogic) {
					GV->editState->hCustomLogics->RegisterLogic(rc->hLogic);
				}
				if (hProp->bLogicEditQuit) {
					hProp->Save();
					GetUserDataFromObj(hObj)->SyncToObj();
					hProp->hState->vPort->MarkToRedraw(1);
					hProp->bKill = 1;
				}
			}
		}
		delete rc;
	}
	delete (returnCode*)iReturnCode;
}

std::string State::cObjectProp::getElementAt(int i)
{
	if (i < 0 || i >= GV->vstrClawLogics.size()) return "";
	return GV->vstrClawLogics[i];
}

int State::cObjectProp::getNumberOfElements()
{
	return GV->vstrClawLogics.size();
}
