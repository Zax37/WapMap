#include "editCandy.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../../shared/commonFunc.h"
#include "../cObjectUserData.h"
#include "../databanks/imageSets.h"
#include "../cInterfaceSheet.h"

#include <SFML/Graphics.hpp>
#include <set>

extern HGE *hge;

#define IMAGE_TILE_WIDTH 64
#define IMAGE_TILE_HEIGHT 64
#define IMAGE_TILES_PER_ROW 5
#define IMAGE_TILE_INTERNAL_PADDING 2
#define CONTAINER_X 0
#define CONTAINER_Y 25
#define CONTAINER_WIDTH IMAGE_TILE_WIDTH * IMAGE_TILES_PER_ROW + 4
#define CONTAINER_HEIGHT 320
#define WINDOW_WIDTH_WITH_SCROLL CONTAINER_WIDTH + 12
#define WINDOW_HEIGHT CONTAINER_HEIGHT + 24 + 240

namespace ObjEdit {

	std::vector<std::string> cEditObjCandy::AnimationsList = {
		"GAME_BACKWARD50",
		"GAME_BACKWARD100",
		"GAME_CYCLE50",
		"GAME_CYCLE100",
		"GAME_CYCLE200",
		"GAME_CYCLE500",
		"GAME_FORWARD50",
		"GAME_FORWARD100",
		"GAME_REVERSECYCLE50",
		"GAME_REVERSECYCLE100",
		"GAME_REVERSECYCLE200",
		"GAME_REVERSECYCLE500",
	};

	cEditObjCandy::cEditObjCandy(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enCandy;
		
/*#ifdef BUILD_DEBUG
		GV->Console->Printf("Listing standard imagesets for level %d:", hState->hParser->GetBaseLevel());
#endif // BUILD_DEBUG*/

		for (size_t i = 0; i < GV->editState->SprBank->GetAssetsCount(); i++) {
			cSprBankAsset* imgSet = GV->editState->SprBank->GetAssetByIterator(i);
			std::string name(imgSet->GetID());
			if (name.starts_with("LEVEL_") && GV->vstrStandardImagesets.contains(name)) {
				standardImgs.push_back(imgSet);
/*#ifdef BUILD_DEBUG
				GV->Console->Print(imgSet->GetID());
#endif // BUILD_DEBUG*/
			}
			else if (name.starts_with("CUSTOM_")) {
				customImgs.push_back(imgSet);
			} else {
				otherImgs.push_back(imgSet);
			}
		}
		int height = ceil(standardImgs.size() / (float)IMAGE_TILES_PER_ROW) * IMAGE_TILE_HEIGHT;

		win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Candy", "WinCaption"));
		win->setDimension(gcn::Rectangle(100, 100, (height <= CONTAINER_HEIGHT ? CONTAINER_WIDTH : WINDOW_WIDTH_WITH_SCROLL), WINDOW_HEIGHT));
		win->setClose(1);
		win->addActionListener(hAL);
		win->addMouseListener(this);
		st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY());

		win->add(_butAddNext, 100, CONTAINER_HEIGHT + 205);
		win->add(_butSave, 212, CONTAINER_HEIGHT + 205);

		imgsCon = new SHR::Container();
		imgsCon->setOpaque(0);
		saImgPick = new SHR::ScrollArea(imgsCon, SHR::ScrollArea::SHOW_NEVER, SHR::ScrollArea::SHOW_AUTO);
		saImgPick->setDimension(gcn::Rectangle(0, 0, WINDOW_WIDTH_WITH_SCROLL, CONTAINER_HEIGHT - 1));
		saImgPick->setOpaque(0);
		//win->add(imgsCon, CONTAINER_X, CONTAINER_Y);
		win->add(saImgPick, -4 + CONTAINER_X, 8 + CONTAINER_Y);

		win->add(vpAdv);

		updateDimensions = [this]() {
			int spritesCount = CountActualImagesToDisplay(CurrentlyDisplayedImgSet());
			int height = ceil(spritesCount / (float)IMAGE_TILES_PER_ROW) * IMAGE_TILE_HEIGHT;
			imgsCon->setDimension(gcn::Rectangle(0, 0, WINDOW_WIDTH_WITH_SCROLL, height));
			tabs->setWidth(height <= CONTAINER_HEIGHT ? CONTAINER_WIDTH : WINDOW_WIDTH_WITH_SCROLL);

			const gcn::Rectangle& dimension = win->getDimension();
			win->setDimension(gcn::Rectangle(dimension.x, dimension.y, (height <= CONTAINER_HEIGHT ? CONTAINER_WIDTH : WINDOW_WIDTH_WITH_SCROLL), dimension.height));
		};

		tabs = new SHR::TabbedArea();
		tabs->setDimension(gcn::Rectangle(0, 0, CONTAINER_WIDTH, 24));
		tabs->setSelectionChangeCallback(updateDimensions);
		tabs->addTab(GETL2S("EditObj_Candy", "Tab_Standard"));
		tabs->addTab(GETL2S("EditObj_Candy", "Tab_Custom"));
		tabs->addTab(GETL2S("EditObj_Candy", "Tab_Other"));
		win->add(tabs, 0, 8);

		highDetail = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Candy", "HighDetail"));
		highDetail->adjustSize();
		highDetail->addActionListener(hAL);
		win->add(highDetail, 8, CONTAINER_HEIGHT + 42);

		labZPos = new SHR::Lab(GETL2S("EditObj_Candy", "ZCoord"));
		labZPos->adjustSize();
		win->add(labZPos, 8, CONTAINER_HEIGHT + 67);

		int z = GetUserDataFromObj(obj)->GetZ();
		if (strstr(obj->GetLogic(), "Behind")) {
			highDetail->setSelected(true);
			if (!z) {
				z = 990;
			}
		}
		else if (strstr(obj->GetLogic(), "Front")) {
			highDetail->setSelected(true);
			if (!z) {
				z = 5100;
			}
		}

#define EOC_GROUP_Y CONTAINER_HEIGHT + 90
		std::string group = std::to_string((int)this);
        rbType[0] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Candy", "Z_Behind"), group, z == 990);
        rbType[0]->adjustSize();
        rbType[0]->addActionListener(hAL);
        win->add(rbType[0], 8, EOC_GROUP_Y);

        rbType[1] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Candy", "Z_Front"), group, z == 5100);
        rbType[1]->adjustSize();
        rbType[1]->addActionListener(hAL);
		win->add(rbType[1], 8, EOC_GROUP_Y + 22);

		bool customZ = z != 990 && z != 5100;
		rbType[2] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Candy", "Z_Custom"), group, customZ);
		rbType[2]->adjustSize();
		rbType[2]->addActionListener(hAL);
		win->add(rbType[2], 8, EOC_GROUP_Y + 44);

        zCoord = new SHR::TextField(customZ ? std::to_string(z) : "1000");
		zCoord->SetNumerical(1);
		zCoord->setDimension(gcn::Rectangle(0, 0, 80, 20));
		zCoord->setEnabled(customZ);
		zCoord->addActionListener(hAL);
		win->add(zCoord, 12 + rbType[2]->getWidth(), EOC_GROUP_Y + 42);

		animated = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Candy", "Animated"));
		animated->setSelected(strstr(obj->GetLogic(), "Ani"));
		animated->adjustSize();
		animated->addActionListener(hAL);
		win->add(animated, 8, CONTAINER_HEIGHT + 163);

		const char* anim = obj->GetAnim();
		if (!anim || !*anim) anim = "GAME_CYCLE100";
		animation = new SHR::TextDropDown(anim, this);
		animation->setWidth(CONTAINER_WIDTH - animated->getWidth() - 28);
		animation->setEnabled(animated->isSelected());
		animation->setText(anim);
		animation->addActionListener(hAL);
		win->add(animation, 16 + animated->getWidth(), CONTAINER_HEIGHT + 162);
    }

    cEditObjCandy::~cEditObjCandy() {
        delete highDetail;
        delete animated;
        delete labZPos;

        for (auto & i : rbType)
            delete i;

        delete zCoord;
        delete animation;

        delete tabs;
        delete imgsCon;
        delete saImgPick;

        delete win;
        hState->vPort->MarkToRedraw(1);
    }

    void cEditObjCandy::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
		} else if (actionEvent.getSource() == animation) {
			const std::string& anim = animation->getText();
			hTempObj->SetAnim(anim.c_str());
		} else if (actionEvent.getSource() == highDetail || actionEvent.getSource() == animated) {
			animation->setEnabled(animated->isSelected());
			if (!animated->isSelected()) {
				hTempObj->SetAnim("");
			}
			UpdateLogic();
			if (actionEvent.getSource() == animated && asImageSetPick) {
				if (asFramePick) {
					asFramePick = NULL;
					GV->vstrNANIImagesets.erase(asImageSetPick->GetID());
				} else {
					asFramePick = asImageSetPick->GetIMGByIterator(0);
					GV->vstrNANIImagesets.insert(asImageSetPick->GetID());
				}
				updateDimensions();
			}
		} else if (actionEvent.getSource() == zCoord) {
			const std::string& str = zCoord->getText();
			int z = 0;
			if (!str.empty()) {
				z = std::stoi(str);
			}
			GetUserDataFromObj(hTempObj)->SetZ(z);
			hTempObj->SetParam(WWD::OBJ_PARAMS::Param_LocationZ, z);
			UpdateLogic();
			hState->vPort->MarkToRedraw(1);
		} else for (int i = 0; i < 3; ++i) if (actionEvent.getSource() == rbType[i]) {
			int z = 0;
			switch (i) {
			case 0:
				z = 990;
				break;
			case 1:
				z = 5100;
				break;
			case 2:
				const std::string& str = zCoord->getText();
				if (str.empty()) {
					z = 0;
				} else {
					z = std::stoi(str);
				}
				break;
			}
			GetUserDataFromObj(hTempObj)->SetZ(z);
			hTempObj->SetParam(WWD::OBJ_PARAMS::Param_LocationZ, z);
			zCoord->setEnabled(i == 2);
			UpdateLogic();
			hState->vPort->MarkToRedraw(1);
			break;
		}
    }

	void cEditObjCandy::Draw()
	{
		float mx, my;
		hge->Input_GetMousePos(&mx, &my);
		int dx, dy;
		win->getAbsolutePosition(dx, dy);
		dx += 2 + CONTAINER_X;
		dy += 24 + CONTAINER_Y;
		int w = win->getWidth();

		hge->Gfx_SetClipping(dx, dy, CONTAINER_WIDTH - 1, CONTAINER_HEIGHT - 1);

		std::vector<cSprBankAsset*>& assets = CurrentlyDisplayedImgSet();

		cSprBankAsset* lastHoveredImg = asImageSetHover;
		cSprBankAssetIMG* lastHoveredFrame = asFrameHover;
		asImageSetHover = NULL;
		asFrameHover = NULL;
		float time = hge->Timer_GetTime();
		int frame = time * 5;

		GV->hGfxInterface->sprMainBackground->RenderStretch(dx, dy, dx + CONTAINER_WIDTH - 4, dy + CONTAINER_HEIGHT);

		float dyo = dy - saImgPick->getVerticalScrollAmount();

		int count = assets.size();
		int i_off = 0, i_off_sub = 0;
		for (size_t i = 0; i < count; i++) {
			cSprBankAsset* imgSet = assets[i];
			bool nani = false;
			int add = 0;
			if (GV->vstrNANIImagesets.contains(imgSet->GetID())) {
				nani = true;
				add = imgSet->GetSpritesCount() - 1;
				i_off_sub = i_off;
				i_off += add;
			}

			do {
				int gridX = ((i + i_off - add) % IMAGE_TILES_PER_ROW),
					gridY = ((i + i_off - add) / IMAGE_TILES_PER_ROW);
				int drawX = dx + gridX * IMAGE_TILE_WIDTH,
					drawY = dyo + gridY * IMAGE_TILE_HEIGHT;

				int it = nani ? i_off - i_off_sub - add : frame % imgSet->GetSpritesCount();
				cSprBankAssetIMG* frame = imgSet->GetIMGByIterator(it);
				hgeSprite *spr = frame->GetSprite();
				spr->SetFlip(false, false, true);

				DWORD colBorder = GV->colLineDark;

				if (mx > drawX && my > drawY && mx < drawX + IMAGE_TILE_WIDTH && my < drawY + IMAGE_TILE_HEIGHT) {
					if (my < dy + CONTAINER_HEIGHT) {
						asImageSetHover = imgSet;
						asFrameHover = nani ? frame : NULL;
						colBorder = TILE_HIGHLIGHT_COLOR;

						if (lastHoveredImg != asImageSetHover || lastHoveredFrame != asFrameHover) {
							imgsCon->UpdateTooltip(false);
							imgsCon->SetTooltip(nani ? (std::string(imgSet->GetID()) + " (" + std::to_string(frame->GetID()) + ')').c_str() : imgSet->GetID());
						}
						imgsCon->UpdateTooltip(true);
					}
				}

				if (imgSet == asImageSetPick && (!nani || asFramePick == frame)) {
					colBorder = 0xFF1585e2;
				}

				hge->Gfx_RenderLine(drawX + 1, drawY + 1, drawX + IMAGE_TILE_WIDTH - 2, drawY + 1, colBorder);
				hge->Gfx_RenderLine(drawX + 1, drawY + 1, drawX + 1, drawY + IMAGE_TILE_HEIGHT - 2, colBorder);
				hge->Gfx_RenderLine(drawX + IMAGE_TILE_WIDTH - 1, drawY + 1, drawX + IMAGE_TILE_WIDTH - 1, drawY + IMAGE_TILE_HEIGHT - 2, colBorder);
				hge->Gfx_RenderLine(drawX + 1, drawY + IMAGE_TILE_HEIGHT - 1, drawX + IMAGE_TILE_WIDTH - 2, drawY + IMAGE_TILE_HEIGHT - 1, colBorder);

				GV->sprCheckboard->RenderStretch(drawX + 1, drawY + 1, drawX + IMAGE_TILE_WIDTH - 2, drawY + IMAGE_TILE_HEIGHT - 2);

				int boxSide = IMAGE_TILE_WIDTH - IMAGE_TILE_INTERNAL_PADDING * 2;
				int offset = IMAGE_TILE_WIDTH * 0.5;
				int x = drawX + offset;
				int y = drawY + offset;

				//float ox, oy;
				//spr->GetHotSpot(&ox, &oy);
				float width = nani ? spr->GetWidth() : imgSet->GetMaxIMGWidth();
				float height = nani ? spr->GetHeight() : imgSet->GetMaxIMGHeight();

				if (width < boxSide && height < boxSide) {
					/*x += ((ox - spr->GetWidth() * 0.5));
					y += ((oy - spr->GetHeight() * 0.5));*/
					spr->Render(x, y);
				}
				else {
					float scale = 1.0f;
					if (width > height)
						scale = boxSide / width;
					else
						scale = boxSide / height;

					/*x += ((ox - spr->GetWidth() * 0.5) * scale);
					y += ((oy - spr->GetHeight() * 0.5) * scale);*/
					spr->RenderEx(x, y, 0, scale);
				}
			} while (nani && add-- > 0);

			//GV->fntMyriad13->SetColor((colBorder != GV->colLineBright ? 0xFFFFFFFF : 0xFFa1a1a1));
			//GV->fntMyriad13->printf(drawX + IMAGE_TILE_WIDTH * 0.5, drawY + IMAGE_TILE_HEIGHT - 13, HGETEXT_CENTER, "%s", 0, imgSet->GetID());
		}

		hge->Gfx_SetClipping();
		hge->Gfx_RenderLine(dx, dy + CONTAINER_HEIGHT, dx + CONTAINER_WIDTH - 4, dy + CONTAINER_HEIGHT, GV->colLineBright);

		if (!asImageSetHover) {
			imgsCon->UpdateTooltip(false);
		}
	}

	void cEditObjCandy::UpdateLogic()
	{
		if (highDetail->isSelected()) {
			int z = GetUserDataFromObj(hTempObj)->GetZ();
			std::string logic(z < 4000 ? "Behind" : "Front");
			if (animated->isSelected()) {
				logic += "Ani";
			}
			logic += "Candy";
			hTempObj->SetLogic(logic.c_str());
		} else if (animated->isSelected()) {
			hTempObj->SetLogic("AniCycle");
		} else {
			hTempObj->SetLogic("DoNothing");
		}
	}

	std::vector<cSprBankAsset*>& cEditObjCandy::CurrentlyDisplayedImgSet()
	{
		switch (tabs->getSelectedTabIndex()) {
		case TAB_STANDARD:
			return standardImgs;
		case TAB_CUSTOM:
			return customImgs;
		default:
		case TAB_OTHER:
			return otherImgs;
		}
	}

	int cEditObjCandy::CountActualImagesToDisplay(std::vector<cSprBankAsset*>& assets)
	{
		int count = 0;
		for (auto const& asset : assets) {
			if (GV->vstrNANIImagesets.contains(asset->GetID())) {
				count += asset->GetSpritesCount();
			} else {
				++count;
			}
		}
		return count;
	}

	void cEditObjCandy::mouseClicked(MouseEvent & mouseEvent)
	{
		if (asImageSetHover) {
			asImageSetPick = asImageSetHover;
			asFramePick = asFrameHover;

			if (asFramePick || asImageSetPick->GetSpritesCount() == 1) {
				animated->setSelected(false);
				int frameI = asFramePick ? asFramePick->GetID() : -1;
				GetUserDataFromObj(hTempObj)->SetI(frameI);
				hTempObj->SetParam(WWD::OBJ_PARAMS::Param_LocationI, frameI);
				animation->setEnabled(false);
			}
			else {
				animated->setSelected(true);
				auto it = GV->vstrStandardImagesetAnimations.find(asImageSetPick->GetID());
				if (it != GV->vstrStandardImagesetAnimations.end()) {
					animation->setText(it->second);
				}
				else {
					animation->setText("GAME_CYCLE100");
				}
				animation->setEnabled(true);
			}
			UpdateLogic();
			hTempObj->SetImageSet(asImageSetPick->GetID());
			hState->vPort->MarkToRedraw(1);
		}
	}
}
