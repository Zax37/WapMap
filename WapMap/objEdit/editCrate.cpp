#include "editCrate.h"
#include "editStatue.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include "winInventoryPickbox.h"
#include "../databanks/imageSets.h"

extern HGE * hge;

namespace ObjEdit
{
	cEditObjCrate::cEditObjCrate(WWD::Object * obj, State::EditingWW * st) : cObjEdit(obj, st)
	{
		if (hState->hParser->GetBaseLevel() == 11 || hState->hParser->GetBaseLevel() == 14)
			bStackable = 0;
		else
			bStackable = 1;

		iType = ObjEdit::enCrate;
		bPick = 0;

		win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Crate", "WinCaption"));
		win->setDimension(gcn::Rectangle(0, 0, 250, 282));
		win->setClose(1);
		win->addActionListener(hAL);
		win->add(vpAdv);
		win->setMovable(0);
		st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

		win->add(_butAddNext, 5, win->getHeight() - 60);
		win->add(_butSave, 125, win->getHeight() - 60);

		char tmp[64];
		sprintf(tmp, "%p", this);

		labAlign = new SHR::Lab(GETL2S("EditObj_Statue", "Align"));
		labAlign->adjustSize();
		win->add(labAlign, 5, 12);

		rbType[0] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Statue", "Behind"), tmp, strstr(hTempObj->GetLogic(), "Behind") || strstr(hTempObj->GetLogic(), "Back"));
		rbType[0]->adjustSize();
		rbType[0]->addActionListener(hAL);
		win->add(rbType[0], 5, 30);
		rbType[1] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Statue", "Front"), tmp, strstr(hTempObj->GetLogic(), "Front"));
		rbType[1]->adjustSize();
		rbType[1]->addActionListener(hAL);
		win->add(rbType[1], 100, 30);

		bAlign = rbType[1]->isSelected();

		labDurability = new SHR::Lab(GETL2S("EditObj_Statue", "Durability"));
		labDurability->setColor(0xFFb2b2b2);
		labDurability->adjustSize();
		win->add(labDurability, 5, 55);

		lmDurability = new lmEditObjStatueDurability();

		ddDurability = new SHR::DropDown();
		ddDurability->setDimension(gcn::Rectangle(0, 0, 150, 20));
		ddDurability->setListModel(lmDurability);
		ddDurability->addActionListener(hAL);
		ddDurability->SetGfx(&GV->gcnParts);
		win->add(ddDurability, labDurability->getX() + labDurability->getWidth() + 5, 55);

		if (hTempObj->GetParam(WWD::Param_Health) <= 1)
			ddDurability->setSelected(0);
		else if (hTempObj->GetParam(WWD::Param_Health) < 999)
			ddDurability->setSelected(1);
		else if (hTempObj->GetParam(WWD::Param_Health) >= 999)
			ddDurability->setSelected(2);

		cbIncludeSpecials = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Statue", "IncludeSpecials"));
		cbIncludeSpecials->adjustSize();
		win->add(cbIncludeSpecials, 5, 5);

		cbRandomCount = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Statue", "RandomItemCount"));
		cbRandomCount->adjustSize();
		cbRandomCount->setSelected(1);
		win->add(cbRandomCount, 5, 5);

		butRandomize = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Statue", "Randomize"));
		butRandomize->setDimension(gcn::Rectangle(0, 0, 100, 33));
		butRandomize->addActionListener(hAL);
		win->add(butRandomize, 125, win->getHeight() - 120);

		labWarpDest = new SHR::Lab(GETL2S("EditObj_Statue", "WarpDest"));
		labWarpDest->adjustSize();
		win->add(labWarpDest, 5, 85);

		sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedX));
		tfWarpX = new SHR::TextField(tmp);
		tfWarpX->addActionListener(hAL);
		tfWarpX->setDimension(gcn::Rectangle(0, 0, 100, 20));
		win->add(tfWarpX, 20, 105);

		sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedY));
		tfWarpY = new SHR::TextField(tmp);
		tfWarpY->addActionListener(hAL);
		tfWarpY->setDimension(gcn::Rectangle(0, 0, 100, 20));
		win->add(tfWarpY, 20, 132);

		butWarpPick = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Warp", "Pick"));
		butWarpPick->setDimension(gcn::Rectangle(0, 0, 110, 33));
		butWarpPick->addActionListener(hAL);
		win->add(butWarpPick, 125, 115);

		labTreasures = new SHR::Lab(GETL2S("EditObj_Statue", "Treasures"));
		labTreasures->adjustSize();
		win->add(labTreasures, 5, 90);

		if (!bStackable) {
			invTabs[0] = new SHR::InvTab(&GV->gcnParts);
			invTabs[0]->adjustSize();
			invTabs[0]->addActionListener(hAL);
			if (hTempObj->GetParam(WWD::Param_Powerup) != 0)
				invTabs[0]->SetItem(hState->hInvCtrl->GetItemByID(hTempObj->GetParam(WWD::Param_Powerup)));
			win->add(invTabs[0], 44 + 54, 115);
			for (int i = 1; i < 9; i++)
				invTabs[i] = NULL;
			iCratesCount = 1;
		}
		else {
			for (int i = 0; i < 9; i++) {
				invTabs[i] = new SHR::InvTab(&GV->gcnParts);
				invTabs[i]->setDimension(gcn::Rectangle(0, 0, 32, 32));
				invTabs[i]->addActionListener(hAL);
				if (i > 0)
					invTabs[i]->setVisible(0);
				win->add(invTabs[i], 44 + 54 + 8 + 5, 115 - 43 * i);
			}
			iCratesCount = 1;
			if (hTempObj->GetParam(WWD::Param_Powerup) != 0)
				invTabs[0]->SetItem(hState->hInvCtrl->GetItemByID(hTempObj->GetParam(WWD::Param_Powerup)));
			for (int i = 0; i < 2; i++) {
				WWD::Rect r = hTempObj->GetUserRect(i);
				if (r.x1 != 0) {
					invTabs[iCratesCount]->setVisible(1);
					invTabs[iCratesCount]->SetItem(hState->hInvCtrl->GetItemByID(r.x1));
					if (invTabs[iCratesCount]->GetItem().second == -1)
						GV->Console->Printf("~y~Warning:~w~ Found invalid item in container id#%d (%d).", hTempObj->GetParam(WWD::Param_ID), r.x1);
					else
						iCratesCount++;
				}
				if (r.y1 != 0) {
					invTabs[iCratesCount]->setVisible(1);
					invTabs[iCratesCount]->SetItem(hState->hInvCtrl->GetItemByID(r.y1));
					if (invTabs[iCratesCount]->GetItem().second == -1)
						GV->Console->Printf("~y~Warning:~w~ Found invalid item in container id#%d (%d).", hTempObj->GetParam(WWD::Param_ID), r.y1);
					else
						iCratesCount++;
				}
				if (r.x2 != 0) {
					invTabs[iCratesCount]->setVisible(1);
					invTabs[iCratesCount]->SetItem(hState->hInvCtrl->GetItemByID(r.x2));
					if (invTabs[iCratesCount]->GetItem().second == -1)
						GV->Console->Printf("~y~Warning:~w~ Found invalid item in container id#%d (%d).", hTempObj->GetParam(WWD::Param_ID), r.x2);
					else
						iCratesCount++;
				}
				if (r.y2 != 0) {
					invTabs[iCratesCount]->setVisible(1);
					invTabs[iCratesCount]->SetItem(hState->hInvCtrl->GetItemByID(r.y2));
					if (invTabs[iCratesCount]->GetItem().second == -1)
						GV->Console->Printf("~y~Warning:~w~ Found invalid item in container id#%d (%d).", hTempObj->GetParam(WWD::Param_ID), r.y2);
					else
						iCratesCount++;
				}
			}
			if (iCratesCount < 9) {
				invTabs[iCratesCount]->setVisible(1);
				iCratesCount++;
			}

			if (iCratesCount == 2 && invTabs[0]->GetItem().second == -1) {
				invTabs[0]->SetItem(hState->hInvCtrl->GetItemByID(33));
			}
		}
		if (iCratesCount > 2) {
			if (!strcmp(hTempObj->GetLogic(), "FrontCrate"))
				hTempObj->SetLogic("FrontStackedCrates");
			else if (!strcmp(hTempObj->GetLogic(), "BehindCrate"))
				hTempObj->SetLogic("BackStackedCrates");
		}
		RebuildWindow();

		hInventory = new cInvPickbox();
		hInventory->SetPosition(hState->vPort->GetX() + hState->vPort->GetWidth() - hInventory->GetWidth(),
			hState->vPort->GetY() + hState->vPort->GetHeight() - hInventory->GetHeight());
		hInventory->SetVisible(1);
	}

	cEditObjCrate::~cEditObjCrate()
	{
		for (int i = 0; i < 2; i++)
			delete rbType[i];
		delete labDurability;
		delete ddDurability;
		delete butRandomize;
		delete lmDurability;
		delete labAlign;
		delete cbRandomCount;
		delete cbIncludeSpecials;
		delete labTreasures;
		delete butWarpPick;
		delete tfWarpX;
		delete tfWarpY;
		delete labWarpDest;
		for (int i = 0; i < 9; i++)
			delete invTabs[i];
		delete win;
		delete hInventory;
		hState->vPort->MarkToRedraw(1);
	}

	void cEditObjCrate::Save()
	{
		if (!ddDurability->isEnabled())
			hTempObj->SetParam(WWD::Param_Health, 0);
	}

	void cEditObjCrate::Action(const gcn::ActionEvent &actionEvent)
	{
		if (actionEvent.getSource() == win) {
			bKill = 1;
			return;
		}
		else if (actionEvent.getSource() == rbType[0]) {
			bAlign = 0;
			if (!bStackable) {
				SetLogic(0, 0);
			}
			else {
				SetLogic(iCratesCount > 2, 0);
			}
			hTempObj->SetParam(WWD::Param_LocationZ, 1010);
		}
		else if (actionEvent.getSource() == rbType[1]) {
			bAlign = 1;
			if (!bStackable) {
				SetLogic(0, 1);
			}
			else {
				SetLogic(iCratesCount > 2, 1);
			}
			hTempObj->SetParam(WWD::Param_LocationZ, 5000);
		}
		else if (actionEvent.getSource() == ddDurability) {
			if (ddDurability->getSelected() == 0)
				hTempObj->SetParam(WWD::Param_Health, 1);
			else if (ddDurability->getSelected() == 1)
				hTempObj->SetParam(WWD::Param_Health, 2);
			else if (ddDurability->getSelected() == 2)
				hTempObj->SetParam(WWD::Param_Health, 999);
		}
		else if (!bStackable && actionEvent.getSource() == invTabs[0]) {
			if (invTabs[0]->GetItem().second == -1)
				invTabs[0]->SetItem(hState->hInvCtrl->GetItemByID(33));
		}
		else if (actionEvent.getSource() == tfWarpX) {
			hTempObj->SetParam(WWD::Param_SpeedX, atoi(tfWarpX->getText().c_str()));
			hState->vPort->MarkToRedraw(1);
		}
		else if (actionEvent.getSource() == tfWarpY) {
			hTempObj->SetParam(WWD::Param_SpeedY, atoi(tfWarpY->getText().c_str()));
			hState->vPort->MarkToRedraw(1);
		}
		else if (actionEvent.getSource() == butWarpPick) {
			bPick = !bPick;
			tfWarpX->setEnabled(!bPick);
			tfWarpY->setEnabled(!bPick);
			for (int i = 0; i < 2; i++) rbType[i]->setEnabled(!bPick);
			for (int i = 0; i < 9; i++) invTabs[i]->setEnabled(!bPick);
			_butSave->setEnabled(!bPick);
			ddDurability->setEnabled(!bPick&&iCratesCount <= 2);
			labDurability->setColor(ddDurability->isEnabled() ? 0xFFa1a1a1 : 0xFFb2b2b2);
			butWarpPick->setCaption(GETL2S("EditObj_Warp", (bPick ? "Unpick" : "Pick")));
		}
		else if (actionEvent.getSource() == butRandomize) {
			int iRandNum = 1;
			if (bStackable) {
				if (cbRandomCount->isSelected()) {
					iRandNum = hge->Random_Int(1, 9);
				}
				else {
					iRandNum = 0;
					for (int i = 0; i < 9; i++)
						if (invTabs[i]->GetItem().second != -1)
							iRandNum++;
					if (iRandNum == 0)
						iRandNum = 1;
				}
			}
			for (int i = 0; i < 9; i++) invTabs[i]->SetItem(cInventoryItem("", -1));
			for (int i = 0; i < iRandNum; i++) {
				int id = hge->Random_Int(0, (cbIncludeSpecials->isSelected() ? InventoryItemsCount - 2
					: 31));
				if (id == 3) id++;
				invTabs[i]->SetItem(GV->editState->hInvCtrl->GetItemByIt(id));
			}
			iCratesCount = iRandNum + (iRandNum < 9 && bStackable);
			RebuildWindow();
			ApplyInventoryToObject();
		}
		if (bStackable) {
			for (int i = 0; i < 9; i++)
				if (actionEvent.getSource() == invTabs[i]) {
					if (invTabs[i]->GetItem().second != -1 && i < 8 && !invTabs[i + 1]->isVisible()) {
						invTabs[i + 1]->setVisible(1);
						iCratesCount++;
						RebuildWindow();
						if (iCratesCount > 2) {
							SetLogic(1, bAlign);
						}
					}
					else if (invTabs[i]->GetItem().second == -1 && i < 8) {
						if (i == 0 && iCratesCount == 2) {
							invTabs[0]->SetItem(hState->hInvCtrl->GetItemByID(33));
						}
						else {
							if (i + 2 == iCratesCount) {
								for (int x = 8; x > 1; x--) {
									if (invTabs[x]->isVisible()) {
										if (invTabs[x]->GetItem().second == -1 && invTabs[x - 1]->GetItem().second == -1) {
											invTabs[x]->setVisible(0);
											iCratesCount--;
										}
										else
											break;
									}
								}
							}
							for (int x = 1; x < iCratesCount - 1; x++) {
								if (invTabs[x]->GetItem().second == -1) {
									for (int y = x; y < iCratesCount - 1; y++)
										invTabs[y]->SetItem(invTabs[y + 1]->GetItem());
									invTabs[iCratesCount - 1]->setVisible(0);
									iCratesCount--;
								}
							}
							if (iCratesCount == 2) {
								SetLogic(0, bAlign);
								if (invTabs[0]->GetItem().second == -1)
									invTabs[0]->SetItem(hState->hInvCtrl->GetItemByID(33));
							}
						}
					}
					RebuildWindow();
					hState->vPort->MarkToRedraw(1);
					ApplyInventoryToObject();
				}
		}
	}

	void cEditObjCrate::Draw()
	{
		hInventory->Think();
		int dx, dy;
		win->getAbsolutePosition(dx, dy);
		hge->Gfx_RenderLine(dx, dy + 101, dx + win->getWidth(), dy + 101, GV->colLineDark);
		hge->Gfx_RenderLine(dx, dy + 102, dx + win->getWidth(), dy + 102, GV->colLineBright);

		hge->Gfx_RenderLine(dx, dy + win->getHeight() - 125, dx + win->getWidth(), dy + win->getHeight() - 125, GV->colLineDark);
		hge->Gfx_RenderLine(dx, dy + win->getHeight() - 124, dx + win->getWidth(), dy + win->getHeight() - 124, GV->colLineBright);

		hge->Gfx_RenderLine(dx, dy + win->getHeight() - 45, dx + win->getWidth(), dy + win->getHeight() - 45, GV->colLineDark);
		hge->Gfx_RenderLine(dx, dy + win->getHeight() - 44, dx + win->getWidth(), dy + win->getHeight() - 44, GV->colLineBright);

		if (bPick) {
			float mx, my;
			hge->Input_GetMousePos(&mx, &my);
			if (hge->Input_KeyDown(HGEK_LBUTTON) && hState->conMain->getWidgetAt(mx, my) == hState->vPort->GetWidget()) {
				char tmp[64];
				sprintf(tmp, "%d", hState->Scr2WrdX(hState->GetActivePlane(), mx));
				tfWarpX->setText(tmp, 1);
				sprintf(tmp, "%d", hState->Scr2WrdY(hState->GetActivePlane(), my));
				tfWarpY->setText(tmp, 1);
				butWarpPick->simulatePress();
			}
		}

		if (bShowWarpOptions) {
			hge->Gfx_RenderLine(dx, dy + 101 + 75, dx + win->getWidth(), dy + 101 + 75, GV->colLineDark);
			hge->Gfx_RenderLine(dx, dy + 102 + 75, dx + win->getWidth(), dy + 102 + 75, GV->colLineBright);
			GV->fntMyriad13->SetColor(0xFFa1a1a1);
			GV->fntMyriad13->Render(dx + 7, dy + 122, HGETEXT_LEFT, "X:", 0);
			GV->fntMyriad13->Render(dx + 7, dy + 149, HGETEXT_LEFT, "Y:", 0);
		}

		if (bStackable) {
			for (int i = 8; i >= 0; i--)
				if (invTabs[i]->isVisible()) {
					hState->SprBank->GetObjectSprite(hTempObj)->Render(dx + 117, dy + 150 + i * 43 + 75 * bShowWarpOptions);
				}
		}
	}

	void cEditObjCrate::SetLogic(bool bStacked, bool bf)
	{
		if (!bStacked && invTabs[0]->GetItem().second == 32)
			bStacked = 1;
		if (!bStacked && !bf) {
			hTempObj->SetLogic("BehindCrate");
			ddDurability->setEnabled(!bPick);
		}
		else if (!bStacked && bf) {
			hTempObj->SetLogic("FrontCrate");
			ddDurability->setEnabled(!bPick);
		}
		else if (bStacked && !bf) {
			hTempObj->SetLogic("BackStackedCrates");
			ddDurability->setEnabled(0);
		}
		else if (bStacked && bf) {
			hTempObj->SetLogic("FrontStackedCrates");
			ddDurability->setEnabled(0);
		}
		labDurability->setColor(ddDurability->isEnabled() ? 0xFFa1a1a1 : 0xFFb2b2b2);
	}

	void cEditObjCrate::RebuildWindow()
	{
		if (bStackable)
			for (int i = 0; i < 9; i++)
				invTabs[i]->setVisible(i < iCratesCount);

		bool bwarpf = 0;
		if (bStackable) {
			for (int i = 0; i < 9; i++)
				if (invTabs[i]->isVisible() && invTabs[i]->GetItem().second == 32) //warp
					bwarpf = 1;
		}
		else
			bwarpf = invTabs[0]->GetItem().second == 32;
		bShowWarpOptions = bwarpf;

		tfWarpX->setVisible(bShowWarpOptions);
		tfWarpY->setVisible(bShowWarpOptions);
		butWarpPick->setVisible(bShowWarpOptions);
		labWarpDest->setVisible(bShowWarpOptions);

		labTreasures->setY(90 + 75 * bShowWarpOptions);

		for (int i = 0; i < iCratesCount; i++)
			invTabs[i]->setY(121 + (iCratesCount - i - 1) * 43 + 75 * bShowWarpOptions);

		win->setHeight(305 + (iCratesCount - 1) * 43 + 75 * bShowWarpOptions);
		win->setY(hState->vPort->GetY() + hState->vPort->GetHeight() - win->getHeight());
		_butSave->setY(win->getHeight() - 55);
		_butAddNext->setY(win->getHeight() - 55);
		butRandomize->setY(win->getHeight() - 100);
		cbIncludeSpecials->setY(win->getHeight() - 140);
		cbRandomCount->setY(win->getHeight() - 120);
		hState->vPort->MarkToRedraw(1);

		ddDurability->setEnabled(iCratesCount <= 2);
		if (!ddDurability->isEnabled())
			ddDurability->setSelected(0);
		labDurability->setColor(ddDurability->isEnabled() ? 0xFFa1a1a1 : 0xFFb2b2b2);
	}

	void cEditObjCrate::ApplyInventoryToObject()
	{
		if (invTabs[0]->GetItem().second == -1)
			hTempObj->SetParam(WWD::Param_Powerup, 0);
		else
			hTempObj->SetParam(WWD::Param_Powerup, invTabs[0]->GetItem().second);
		if (bStackable) {
			for (int i = 0; i < 2; i++) {
				WWD::Rect r;
				if (invTabs[i * 4 + 1]->GetItem().second == -1) r.x1 = 0;
				else r.x1 = invTabs[i * 4 + 1]->GetItem().second;
				if (invTabs[i * 4 + 2]->GetItem().second == -1) r.y1 = 0;
				else r.y1 = invTabs[i * 4 + 2]->GetItem().second;
				if (invTabs[i * 4 + 3]->GetItem().second == -1) r.x2 = 0;
				else r.x2 = invTabs[i * 4 + 3]->GetItem().second;
				if (invTabs[i * 4 + 4]->GetItem().second == -1) r.y2 = 0;
				else r.y2 = invTabs[i * 4 + 4]->GetItem().second;
				hTempObj->SetUserRect(i, r);
			}
		}
		SetLogic(bStackable&&invTabs[2]->isVisible(), rbType[1]->isSelected());
	}
}
