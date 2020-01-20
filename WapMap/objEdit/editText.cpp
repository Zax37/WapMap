/*

		  char text[strlen(m_hOwn->tbtextText->getText().c_str())+1];
		  strcpy(text, m_hOwn->tbtextText->getText().c_str());
		  if( strlen(text) == 0 ) return;

		  int beginx = m_hOwn->Scr2WrdX(m_hOwn->GetActivePlane(), m_hOwn->vPort->GetX()+m_hOwn->vPort->GetWidth()/2),
			  beginy = m_hOwn->Scr2WrdY(m_hOwn->GetActivePlane(), m_hOwn->vPort->GetY()+m_hOwn->vPort->GetHeight()/2);

		  int offsetx = 0, offsety = 0;
		  m_hOwn->vObjectsPicked.clear();
		  printf("%s (%d)\n", text, strlen(text));
		  for(int i=0;i<strlen(text);i++){
		   printf("doing '%c'\n", text[i]);
		   int frame = -1, yoffset = 0;
		   if( text[i] > 64 && text[i] < 91 )
			frame = text[i]-64;
		   else if( text[i] > 96 && text[i] < 123 )
			frame = text[i]-96;
		   else if( text[i] > 47 && text[i] < 58 )
			frame = text[i]-21;
		   else if( text[i] == 44 ){
			frame = 37;
			yoffset = 6;
		   }else if( text[i] == 96 || text[i] == 39 )
			frame = 37;
		   else if( text[i] == 46 ){
			frame = 38;
			yoffset = 6;
		   }else if( text[i] == 33 )
			frame = 39;
		   else if( text[i] == 63 )
			frame = 40;
		   else if( text[i] == 61 )
			frame = 41;
		   else if( text[i] == 60 )
			frame = 42;
		   else if( text[i] == 62 )
			frame = 43;
		   else if( text[i] == 40 )
			frame = 44;
		   else if( text[i] == 41 )
			frame = 45;
		   else if( text[i] == 91 )
			frame = 46;
		   else if( text[i] == 93 )
			frame = 47;
		   else if( text[i] == 64 )
			frame = 48;
		   else if( text[i] == '#' )
			frame = 49;
		   else if( text[i] == '$' )
			frame = 50;
		   else if( text[i] == '%' )
			frame = 51;
		   else if( text[i] == '^' )
			frame = 52;
		   else if( text[i] == '&' )
			frame = 53;
		   else if( text[i] == '*' )
			frame = 54;

		   if( text[i] == ' ' ){
			offsetx += 5;
			continue;
		   }else if( text[i] == '\n' ){
			offsetx = 0;
			offsety += 10;
			continue;
		   }else if( frame == -1 )
			continue;

		   WWD::Object * obj = new WWD::Object();
		   obj->SetLogic("DoNothing");
		   obj->SetImageSet("GAME_FONT");

		   obj->SetParam(WWD::Param_LocationI, frame);
		   obj->SetParam(WWD::Param_LocationX, beginx+offsetx);
		   obj->SetParam(WWD::Param_LocationY, beginy+offsety+yoffset);
		   m_hOwn->GetActivePlane()->AddObjectAndCalcID(obj);
		   obj->SetUserData(new cObjUserData(obj));
		   m_hOwn->hPlaneData[m_hOwn->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(obj);

		   offsetx += m_hOwn->SprBank->GetAssetByID("GAME_FONT")->GetIMGByID(frame)->GetSprite()->GetWidth();

		   m_hOwn->vObjectsPicked.push_back(obj);
		  }

		  m_hOwn->objContext->EmulateClickID(OBJMENU_MOVE);
		  m_hOwn->tbtextText->setText("");
		  m_hOwn->winAddText->setVisible(0);

	 */

#include "editText.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include "../databanks/imageSets.h"

extern HGE * hge;

namespace ObjEdit
{
	cEditObjText::cEditObjText(WWD::Object * obj, State::EditingWW * st) : cObjEdit(obj, st)
	{
		bAllowDragging = 0;
		iType = ObjEdit::enText;
		win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Text", "WinCaption"));
		win->setDimension(gcn::Rectangle(0, 0, 400, 270));
		win->setClose(1);
		win->addActionListener(hAL);
		win->add(vpAdv);
		st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

		hTempObj->SetLogic("DoNothing");
		hTempObj->SetImageSet("GAME_FONT");

		areaX = hTempObj->GetParam(WWD::Param_LocationX);
		areaY = hTempObj->GetParam(WWD::Param_LocationY);

		tbText = new SHR::TextBox(GETL2S("EditObj_Text", "Sample"));
		tbText->setFont(GV->gcnParts.gcnfntMyriad13);
		tbText->setDimension(gcn::Rectangle(0, 0, 100, 20));
		tbText->setMinimalSize(300, 100);
		tbText->addActionListener(hAL);


		saText = new SHR::ScrollArea();
		saText->setContent(tbText);
		saText->setBackgroundColor(0x3c3c3c);
		saText->setDimension(gcn::Rectangle(0, 0, 388, 150));
		win->add(saText, 4, 13);

		labAlign = new SHR::Lab(GETL2S("EditObj_Text", "Align"));
		labAlign->adjustSize();
		win->add(labAlign, 5, 176);

		butApply = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Text", "Apply"));
		butApply->setDimension(gcn::Rectangle(0, 0, 100, 33));
		butApply->addActionListener(hAL);
		butApply->setEnabled(0);
		win->add(butApply, 290, 170);

		butAlign[0] = GV->editState->MakeButton(15 + labAlign->getWidth(), 170, Icon_TextAlign_Left, win);
		butAlign[1] = GV->editState->MakeButton(15 + labAlign->getWidth() + 32, 170, Icon_TextAlign_Justify, win);
		butAlign[2] = GV->editState->MakeButton(15 + labAlign->getWidth() + 64, 170, Icon_TextAlign_Right, win);
		butAlign[1]->setIconColor(0x55FFFFFF);
		butAlign[2]->setIconColor(0x55FFFFFF);
		for (int i = 0; i < 3; i++) {
			butAlign[i]->removeActionListener(GV->editState->al);
			butAlign[i]->addActionListener(hAL);
		}
		iAlign = 0;

		GenerateText();

		GV->editState->vObjectsForbidHL.push_back(hTempObj);

		win->add(_butSave, 150, 220);
	}

	cEditObjText::~cEditObjText()
	{
		GV->editState->vObjectsForbidHL.clear();
		if (!ObjectSaved()) {
			for (int i = 1; i < hObjects.size(); i++) {
				GV->editState->GetActivePlane()->DeleteObject(hObjects[i]);
			}
			hObjects.clear();
			hObjects.push_back(hTempObj);
		}
		for (int i = 0; i < 3; i++)
			delete butAlign[i];
		delete labAlign;
		delete butApply;
		delete saText;
		delete tbText;
		delete win;
		hState->vPort->MarkToRedraw(1);
	}

	int cEditObjText::GetCharFrame(char c)
	{
		if (c > 64 && c < 91)
			return c - 64;
		else if (c > 96 && c < 123)
			return c - 96;
		else if (c > 47 && c < 58)
			return c - 21;
		else if (c == 44)
			return 37;
		else if (c == 96 || c == 39)
			return 37;
		else if (c == 46)
			return 38;
		else if (c == 33)
			return 39;
		else if (c == 63)
			return 40;
		else if (c == 61)
			return 41;
		else if (c == 60)
			return 42;
		else if (c == 62)
			return 43;
		else if (c == 40)
			return 44;
		else if (c == 41)
			return 45;
		else if (c == 91)
			return 46;
		else if (c == 93)
			return 47;
		else if (c == 64)
			return 48;
		else if (c == '#')
			return 49;
		else if (c == '$')
			return 50;
		else if (c == '%')
			return 51;
		else if (c == '^')
			return 52;
		else if (c == '&')
			return 53;
		else if (c == '*')
			return 54;
		else
			return -1;
	}

	void cEditObjText::GenerateText()
	{
		char* text = new char[strlen(tbText->getText().c_str()) + 1];
		strcpy(text, tbText->getText().c_str());
		if (strlen(text) == 0) {
			delete[] text;
			return;
		}

		butApply->setEnabled(0);

		for (int i = 1; i < hObjects.size(); i++) {
			GV->editState->GetActivePlane()->DeleteObject(hObjects[i]);
		}
		hObjects.clear();
		hObjects.push_back(hTempObj);

		areaW = 0;
		areaH = 10;

		int offsetx = 10, offsety = 10;
		for (int i = 0; i < strlen(text); i++) {
			int frame = GetCharFrame(text[i]), yoffset = 0;
			if (text[i] == 46 || text[i] == 44) {
				yoffset = 6;
			}

			if (text[i] == ' ') {
				offsetx += 5;
				continue;
			}
			else if (text[i] == '\n') {
				offsetx = 10;
				offsety += 10;
				areaH += 10;
				continue;
			}
			else if (frame == -1)
				continue;

			if (i == 0) {
				hTempObj->SetParam(WWD::Param_LocationI, frame);
				hTempObj->SetParam(WWD::Param_LocationX, areaX + offsetx);
				hTempObj->SetParam(WWD::Param_LocationY, areaY + offsety + yoffset);
				GetUserDataFromObj(hTempObj)->SyncToObj();
			}
			else {
				WWD::Object * obj = new WWD::Object();
				obj->SetLogic("DoNothing");
				obj->SetImageSet("GAME_FONT");
				obj->SetParam(WWD::Param_LocationI, frame);
				obj->SetParam(WWD::Param_LocationX, areaX + offsetx);
				obj->SetParam(WWD::Param_LocationY, areaY + offsety + yoffset);
				GV->editState->GetActivePlane()->AddObjectAndCalcID(obj);
				obj->SetUserData(new cObjUserData(obj));
				GV->editState->hPlaneData[GV->editState->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(obj);
				hObjects.push_back(obj);
			}
			offsetx += GV->editState->SprBank->GetAssetByID("GAME_FONT")->GetIMGByID(frame)->GetSprite()->GetWidth();
			if (offsetx > areaW)
				areaW = offsetx;
		}
		areaW += 5;
		areaH += 10;
		ApplyAlign();
		delete[] text;
	}

	void cEditObjText::ApplyAlign()
	{
		char* text = new char[strlen(tbText->getText().c_str()) + 1];
		strcpy(text, tbText->getText().c_str());
		if (strlen(text) == 0) {
			delete[] text;
			return;
		}

		int linew = 0;
		int chridx = 0;
		int objit = 0;
		for (int i = 0; i <= strlen(text); i++) {
			if (i == strlen(text) || text[i] == '\n') {
				int offset = 10;
				for (int z = chridx; z < i; z++) {
					if (text[z] == ' ') {
						offset += 5;
						continue;
					}
					else {
						int fr = GetCharFrame(text[z]);
						if (fr != -1) {
							int frmod = 0;
							if (fr == 9)
								frmod = -2;
							if (iAlign == 0)
								hObjects[objit]->SetParam(WWD::Param_LocationX, areaX + offset + frmod);
							else if (iAlign == 1)
								hObjects[objit]->SetParam(WWD::Param_LocationX, areaX + (areaW / 2) - (linew / 2) + offset - 10 + frmod);
							else if (iAlign == 2)
								hObjects[objit]->SetParam(WWD::Param_LocationX, areaX + areaW - linew + offset - 10 + frmod);
							offset += GV->editState->SprBank->GetAssetByID("GAME_FONT")->GetIMGByID(fr)->GetSprite()->GetWidth();
							GetUserDataFromObj(hObjects[objit])->SyncToObj();
							objit++;
						}
					}
				}
				linew = 0;
				chridx = i + 1;
				continue;
			}
			else if (text[i] == ' ') {
				linew += 5;
				continue;
			}
			else {
				int frame = GetCharFrame(text[i]);
				if (frame == -1) continue;
				linew += GV->editState->SprBank->GetAssetByID("GAME_FONT")->GetIMGByID(frame)->GetSprite()->GetWidth();
			}
		}

		/*if( iAlign == 0 ){
		 int offset = 10;
		 int objit = 0;
		 for(int i=0;i<strlen(text);i++){
		  int frame = GetCharFrame(text[i]);
		  if( text[i] == ' ' ){
		   offset += 5;
		   continue;
		  }else if( text[i] == '\n' ){
		   offset = 0;
		   continue;
		  }else if( frame == -1 ){
		   continue;
		  }else{
		   hObjects[objit]->SetParam(WWD::Param_LocationX, areaX+offset);
		   GetUserDataFromObj(hObjects[objit])->SyncToObj();
		   objit++;
		   offset += GV->editState->SprBank->GetAssetByID("GAME_FONT")->GetIMGByID(frame)->GetSprite()->GetWidth();
		  }
		 }
		}else if( iAlign == 1 ){
		 int center =
		}else if( iAlign == 2 ){

		}*/
		GV->editState->vPort->MarkToRedraw(1);
		delete[] text;
	}

	void cEditObjText::Action(const gcn::ActionEvent &actionEvent)
	{
		if (actionEvent.getSource() == win) {
			bKill = 1;
			return;
		}
		else if (actionEvent.getSource() == tbText) {
			butApply->setEnabled(strlen(tbText->getText().c_str()) > 0);
		}
		else if (actionEvent.getSource() == butApply) {
			GenerateText();
			return;
		}
		for (int i = 0; i < 3; i++)
			if (actionEvent.getSource() == butAlign[i]) {
				iAlign = i;
				for (int y = 0; y < 3; y++)
					butAlign[y]->setIconColor(y == i ? 0xFFFFFFFF : 0x55FFFFFF);
				if (butApply->isEnabled())
					GenerateText();
				else
					ApplyAlign();
				return;
			}
	}

	void cEditObjText::_Think(bool bMouseConsumed)
	{
		if (_bDragging) {
			if (iDragX != hTempObj->GetParam(WWD::Param_LocationX) ||
				iDragY != hTempObj->GetParam(WWD::Param_LocationY)) {
				int diffx = hTempObj->GetParam(WWD::Param_LocationX) - iDragX,
					diffy = hTempObj->GetParam(WWD::Param_LocationY) - iDragY;
				for (int i = 1; i < hObjects.size(); i++) {
					hObjects[i]->SetParam(WWD::Param_LocationX, hObjects[i]->GetParam(WWD::Param_LocationX) + diffx);
					hObjects[i]->SetParam(WWD::Param_LocationY, hObjects[i]->GetParam(WWD::Param_LocationY) + diffy);
					GetUserDataFromObj(hObjects[i])->SyncToObj();
				}
				iDragX = hTempObj->GetParam(WWD::Param_LocationX);
				iDragY = hTempObj->GetParam(WWD::Param_LocationY);
				areaX += diffx;
				areaY += diffy;
				GV->editState->vPort->MarkToRedraw(1);
			}
		}
		if (!bMouseConsumed) {
			float mx, my;
			hge->Input_GetMousePos(&mx, &my);
			int wmx = GV->editState->Scr2WrdX(GV->editState->GetActivePlane(), mx),
				wmy = GV->editState->Scr2WrdY(GV->editState->GetActivePlane(), my);
			bool mouseOver = (wmx > areaX && wmy > areaY && wmx < areaX + areaW && wmy < areaY + areaH);
			if (mouseOver) {
				GV->editState->bShowHand = 1;
				if (hge->Input_KeyDown(HGEK_LBUTTON)) {
					_bDragging = 1;
					_iMoveInitX = wmx;
					_iMoveInitY = wmy;
					_iDragOffX = wmx - hTempObj->GetParam(WWD::Param_LocationX);
					_iDragOffY = wmy - hTempObj->GetParam(WWD::Param_LocationY);
					iDragX = hTempObj->GetParam(WWD::Param_LocationX);
					iDragY = hTempObj->GetParam(WWD::Param_LocationY);
				}
			}
		}
	}

	void cEditObjText::Save()
	{
		if (butApply->isEnabled())
			GenerateText();
	}

	void cEditObjText::RenderObjectOverlay()
	{
		int ax1 = GV->editState->Wrd2ScrX(GV->editState->GetActivePlane(), areaX),
			ay1 = GV->editState->Wrd2ScrY(GV->editState->GetActivePlane(), areaY),
			ax2 = GV->editState->Wrd2ScrX(GV->editState->GetActivePlane(), areaX + areaW),
			ay2 = GV->editState->Wrd2ScrY(GV->editState->GetActivePlane(), areaY + areaH);
		hge->Gfx_RenderLine(ax1, ay1, ax2, ay1, 0xFFFF0000);
		hge->Gfx_RenderLine(ax1, ay2, ax2, ay2, 0xFFFF0000);
		hge->Gfx_RenderLine(ax1, ay1, ax1, ay2, 0xFFFF0000);
		hge->Gfx_RenderLine(ax2, ay1, ax2, ay2, 0xFFFF0000);
	}

	void cEditObjText::Draw()
	{
		saText->setBackgroundColor(tbText->isFocused() ? 0x434343 : 0x3c3c3c);
		int dx, dy;
		win->getAbsolutePosition(dx, dy);

		hge->Gfx_RenderLine(dx + 5, dy + 28, dx + 395, dy + 28, GV->colLineDark);
		hge->Gfx_RenderLine(dx + 6, dy + 29, dx + 394, dy + 29, GV->colLineBright);
		hge->Gfx_RenderLine(dx + 6, dy + 28 + 152, dx + 394, dy + 28 + 152, GV->colLineBright);
		hge->Gfx_RenderLine(dx + 5, dy + 29 + 152, dx + 395, dy + 29 + 152, GV->colLineDark);

		hge->Gfx_RenderLine(dx + 5, dy + 27, dx + 5, dy + 29 + 152, GV->colLineDark);
		hge->Gfx_RenderLine(dx + 6, dy + 28, dx + 6, dy + 28 + 152, GV->colLineBright);

		hge->Gfx_RenderLine(dx + 5 + 390, dy + 28, dx + 5 + 390, dy + 28 + 152, GV->colLineBright);
		hge->Gfx_RenderLine(dx + 6 + 390, dy + 27, dx + 6 + 390, dy + 29 + 152, GV->colLineDark);

		hge->Gfx_RenderLine(dx, dy + 225, dx + win->getWidth(), dy + 225, GV->colLineDark);
		hge->Gfx_RenderLine(dx, dy + 226, dx + win->getWidth(), dy + 226, GV->colLineBright);
	}
}
