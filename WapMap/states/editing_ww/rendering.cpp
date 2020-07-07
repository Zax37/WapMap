#include "../editing_ww.h"
#include "../loadmap.h"
#include "../../../shared/commonFunc.h"
#include "../../langID.h"
#include "../../cObjectUserData.h"
#include "../dialog.h"
#include <cmath>
#include <hgevector.h>
#include "../../objEdit/editEnemy.h"
#include "../../objEdit/editElevPath.h"
#include "../../objEdit/editWarp.h"
#include "../../objEdit/editCrate.h"
#include "../../objEdit/editStatue.h"
#include "../../objEdit/editSpringboard.h"
#include "../../rendering/cRenderableItem.h"
#include "../../databanks/imageSets.h"
#include "../../databanks/anims.h"
#include "../../databanks/tiles.h"
#include "../../cBrush.h"

extern HGE *hge;

bool State::ObjSortCoordX(WWD::Object *a, WWD::Object *b) {
    if (GetUserDataFromObj(a)->GetX() == GetUserDataFromObj(b)->GetX())
        return (a->GetParam(WWD::Param_ID) < b->GetParam(WWD::Param_ID));
    return (GetUserDataFromObj(a)->GetX() < GetUserDataFromObj(b)->GetX());
}

bool State::ObjSortCoordY(WWD::Object *a, WWD::Object *b) {
    if (GetUserDataFromObj(a)->GetY() == GetUserDataFromObj(b)->GetY())
        return (a->GetParam(WWD::Param_ID) < b->GetParam(WWD::Param_ID));
    return (GetUserDataFromObj(a)->GetY() < GetUserDataFromObj(b)->GetY());
}

bool State::ObjSortCoordZ(WWD::Object *a, WWD::Object *b) {
    int z1 = a->GetZ();
    int z2 = b->GetZ();
    if (z1 == z2)
        return (a->GetParam(WWD::Param_ID) < b->GetParam(WWD::Param_ID));
    return (z1 < z2);
}

#define SWITCH_FOR_TA_COLOR(a) switch (a) { \
    case WWD::Attrib_Climb: \
    q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_CLIMB; \
    break; \
    case WWD::Attrib_Death: \
    q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_DEATH; \
    break; \
    case WWD::Attrib_Ground: \
    q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_GROUND; \
    break; \
    case WWD::Attrib_Solid: \
    q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_SOLID; \
    break; \
    default: \
    q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_UNKNOWN; \
    break; \
}

void State::EditingWW::DrawTileAttributes(WWD::TileAttrib *attrib, float posX, float posY, float widthMod, float heightMod) {
	if (!attrib) return;
    float w = attrib->getWidth() * widthMod;
    float h = attrib->getHeight() * heightMod;

    if (attrib->getType() == WWD::AttribType_Single) {
        auto a = (WWD::SingleTileAttrib*)attrib;
        if (a->getAttrib() == WWD::Attrib_Clear) return;

        SWITCH_FOR_TA_COLOR(a->getAttrib())

        q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;
        q.v[0].x = posX;
        q.v[0].y = posY;
        q.v[1].x = q.v[0].x + w;
        q.v[1].y = q.v[0].y;
        q.v[2].x = q.v[1].x;
        q.v[2].y = q.v[0].y + h;
        q.v[3].x = q.v[0].x;
        q.v[3].y = q.v[2].y;
        hge->Gfx_RenderQuad(&q);
    } else if (attrib->getType() == WWD::AttribType_Double) {
        auto a = (WWD::DoubleTileAttrib*)attrib;
        if (a->getInsideAttrib() == WWD::Attrib_Clear && a->getOutsideAttrib() == WWD::Attrib_Clear) return;

        auto mask = a->getMask();
        q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;

        if (a->getInsideAttrib() != WWD::Attrib_Clear) {
            SWITCH_FOR_TA_COLOR(a->getInsideAttrib())

            q.v[0].x = posX + mask.x1 * widthMod;
            q.v[0].y = posY + mask.y1 * heightMod;
            q.v[1].x = posX + (mask.x2 + 1) * widthMod;
            q.v[1].y = q.v[0].y;
            q.v[2].x = q.v[1].x;
            q.v[2].y = posY + (mask.y2 + 1) * heightMod;
            q.v[3].x = q.v[0].x;
            q.v[3].y = q.v[2].y;
            hge->Gfx_RenderQuad(&q);
        }

        if (a->getOutsideAttrib() != WWD::Attrib_Clear) {
            SWITCH_FOR_TA_COLOR(a->getOutsideAttrib())

            //upper
            q.v[0].x = posX;
            q.v[0].y = posY;
            q.v[1].x = q.v[0].x + w;
            q.v[1].y = q.v[0].y;
            q.v[2].x = q.v[1].x;
            q.v[2].y = posY + mask.y1 * heightMod;
            q.v[3].x = q.v[0].x;
            q.v[3].y = q.v[2].y;
            hge->Gfx_RenderQuad(&q);
            //lower
            q.v[0].x = posX;
            q.v[0].y = posY + (mask.y2 + 1) * heightMod;
            q.v[1].x = q.v[0].x + w;
            q.v[1].y = q.v[0].y;
            q.v[2].x = q.v[1].x;
            q.v[2].y = posY + h;
            q.v[3].x = q.v[0].x;
            q.v[3].y = q.v[2].y;
            hge->Gfx_RenderQuad(&q);
            //left
            q.v[0].x = posX;
            q.v[0].y = posY + mask.y1 * heightMod;
            q.v[1].x = posX + mask.x1 * widthMod;
            q.v[1].y = q.v[0].y;
            q.v[2].x = q.v[1].x;
            q.v[2].y = posY + (mask.y2 + 1) * heightMod;
            q.v[3].x = q.v[0].x;
            q.v[3].y = q.v[2].y;
            hge->Gfx_RenderQuad(&q);
            //right
            q.v[0].x = posX + (mask.x2 + 1) * widthMod;
            q.v[0].y = posY + mask.y1 * heightMod;
            q.v[1].x = posX + w;
            q.v[1].y = q.v[0].y;
            q.v[2].x = q.v[1].x;
            q.v[2].y = posY + (mask.y2 + 1) * heightMod;
            q.v[3].x = q.v[0].x;
            q.v[3].y = q.v[2].y;
            hge->Gfx_RenderQuad(&q);
        }
    } else if (attrib->getType() == WWD::AttribType_Mask) {
        auto a = (WWD::MaskTileAttrib*)attrib;
        auto data = a->getData();
        if (data == nullptr) return;

        int x1 = std::min(itpDDx1, itpDDx2),
            y1 = std::min(itpDDy1, itpDDy2),
            x2 = std::max(itpDDx1, itpDDx2),
            y2 = std::max(itpDDy1, itpDDy2);
        if (!btpDragDropMask) {
            x1 = a->getWidth();
        }

        for (int i = 0, y = 0; y < a->getHeight(); ++y) {
            for (int x = 0; x < a->getWidth(); ++x, ++i) {
                auto attrib = data[i];

                if (x >= x1 && y >= y1 && x <= x2 && y <= y2) {
                    int i = 0;
                    for (; i < 5; ++i) {
                        if (rbtpIn[i]->isSelected()) break;
                    }
                    attrib = (WWD::TILE_ATTRIB)i;
                }

                if (attrib) {
                    SWITCH_FOR_TA_COLOR(attrib);

                    q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;
                    q.v[0].x = posX;
                    q.v[0].y = posY;
                    q.v[1].x = posX + widthMod;
                    q.v[1].y = posY;
                    q.v[2].x = posX + widthMod;
                    q.v[2].y = posY + heightMod;
                    q.v[3].x = posX;
                    q.v[3].y = posY + heightMod;
                    hge->Gfx_RenderQuad(&q);
                }
                posX += widthMod;
            }
            posX -= w;
            posY += heightMod;
        }
    }
}

void State::EditingWW::DrawTileAttributes(int tileId, float posX, float posY, float widthMod, float heightMod) {
    DrawTileAttributes(hParser->GetTileAttribs(tileId), posX, posY, widthMod, heightMod);
}

void State::EditingWW::DrawDB() {
    int dx, dy;
    winDB->getAbsolutePosition(dx, dy);
    int tabindex = tadbTabs->getSelectedTabIndex();

    if (tabindex == 1) {
        //imgset list right separator
        hge->Gfx_RenderLine(dx + 310, dy + 54, dx + 310, dy + 54 + 540, GV->colLineBright);
        hge->Gfx_RenderLine(dx + 309, dy + 53, dx + 309, dy + 54 + 540, GV->colLineDark);
        //imget list upper separator
        hge->Gfx_RenderLine(dx + 9, dy + 184, dx + 308, dy + 184, GV->colLineBright);
        hge->Gfx_RenderLine(dx + 9, dy + 183, dx + 308, dy + 183, GV->colLineDark);

        if (dbAssetsImg->getSelected() != -1 && dbFramesImg->getSelected() != -1) {
            //image list right separator
            hge->Gfx_RenderLine(dx + 494, dy + 54, dx + 494, dy + 260, GV->colLineBright);
            hge->Gfx_RenderLine(dx + 493, dy + 53, dx + 493, dy + 260, GV->colLineDark);
            //image properties separators
            for (int i = 0; i < 8; i++) {
                hge->Gfx_RenderLine(dx + 494, dy + 100 + i * 20, dx + winDB->getWidth() - 5, dy + 100 + i * 20,
                                    GV->colLineBright);
                hge->Gfx_RenderLine(dx + 494, dy + 99 + i * 20, dx + winDB->getWidth() - 5, dy + 99 + i * 20,
                                    GV->colLineDark);
            }
            //image preview flags upper separator
            hge->Gfx_RenderLine(dx + 310, dy + 262, dx + winDB->getWidth() - 5, dy + 262, GV->colLineBright);
            hge->Gfx_RenderLine(dx + 310, dy + 261, dx + winDB->getWidth() - 5, dy + 261, GV->colLineDark);
            //image preview upper separator
            hge->Gfx_RenderLine(dx + 310, dy + 292, dx + winDB->getWidth() - 5, dy + 292, GV->colLineBright);
            hge->Gfx_RenderLine(dx + 310, dy + 291, dx + winDB->getWidth() - 5, dy + 291, GV->colLineDark);
            //flags left separator
            hge->Gfx_RenderLine(dx + 655, dy + 99, dx + 655, dy + 260, GV->colLineBright);
            hge->Gfx_RenderLine(dx + 654, dy + 99, dx + 654, dy + 260, GV->colLineDark);
            //user data vertical separator
            hge->Gfx_RenderLine(dx + 575, dy + 239, dx + 575, dy + 260, GV->colLineBright);
            hge->Gfx_RenderLine(dx + 574, dy + 239, dx + 574, dy + 260, GV->colLineDark);

            cSprBankAssetIMG *img = SprBank->GetAssetByIterator(dbAssetsImg->getSelected())->GetIMGByIterator(
                    dbFramesImg->getSelected());

            GV->fntMyriad16->printf(dx + 650, dy + 99, HGETEXT_RIGHT, "~y~%d", 0, dbFramesImg->getSelected());
            GV->fntMyriad16->printf(dx + 650, dy + 119, HGETEXT_RIGHT, "~y~%d", 0, img->GetID());
            /*GV->fntMyriad16->printf(dx+650, dy+139, HGETEXT_RIGHT, "~y~%d", 0, img->GetFileID());
			char * fl = SHR::FormatSize(img->GetFileSize());
			GV->fntMyriad16->printf(dx+650, dy+159, HGETEXT_RIGHT, "~y~%s", 0, fl);
			delete [] fl;
			GV->fntMyriad16->printf(dx+650, dy+179, HGETEXT_RIGHT, "~y~%d~l~x~y~%d", 0, img->GetFileDimX(), img->GetFileDimY());
			GV->fntMyriad16->printf(dx+650, dy+199, HGETEXT_RIGHT, "~y~%d~l~x~y~%d", 0, img->GetFileOffsetX(), img->GetFileOffsetY());
			GV->fntMyriad16->printf(dx+570, dy+239, HGETEXT_RIGHT, "~y~%d", 0, img->GetFileUserData(0));
			GV->fntMyriad16->printf(dx+650, dy+239, HGETEXT_RIGHT, "~y~%d", 0, img->GetFileUserData(1));
			GV->fntMyriad16->SetColor(0xFF000000);
			GV->fntMyriad16->Render(dx+790, dy+80, HGETEXT_RIGHT, img->GetFileHash().c_str());*/

            /*int flagn = 1;
			for(int i=0;i<8;i++){
			 char ident[32];
			 sprintf(ident, "Flag_%d", i+1);
			 GV->fntMyriad16->Render(dx+676, dy+99+i*20, HGETEXT_LEFT, GETL2S("WinDatabase", ident));
			 hgeSprite * spr = (img->GetFileFlags() & flagn ? GV->gcnParts.sprCBoxOn : GV->gcnParts.sprCBox);
			 spr->Render(dx+658, dy+102+i*20);
			 flagn *= 2;
			}*/

            int vpx = dx + 320, vpy = dy + 302;
            hge->Gfx_SetClipping(vpx, vpy, 465, 282);
            for (int y = 0; y < 3; y++)
                for (int x = 0; x < 4; x++) {
                    GV->sprCheckboard->Render(vpx + 120 * x, vpy + 120 * y);
                }

            hgeSprite *spr = img->GetSprite();
            spr->SetFlip(cbdbisFlipX->isSelected(), cbdbisFlipY->isSelected());
            int sprx = vpx + 233 - idbisMoveX,
                spry = vpy + 141 - idbisMoveY;
            spr->Render(sprx, spry);

            if (cbdbisBorder->isSelected()) {
                float hsx, hsy;
                spr->GetHotSpot(&hsx, &hsy);
                float fsprw = spr->GetWidth() / 2, fsprh = spr->GetHeight() / 2;

                hsx -= fsprw;
                hsy -= fsprh;

                hge->Gfx_RenderLine(sprx - fsprw - hsx, spry - fsprh - hsy, sprx + fsprw - hsx, spry - fsprh - hsy,
                                    0xFFFF0000);
                hge->Gfx_RenderLine(sprx + fsprw - hsx, spry - fsprh - hsy, sprx + fsprw - hsx, spry + fsprh - hsy,
                                    0xFFFF0000);
                hge->Gfx_RenderLine(sprx + fsprw - hsx, spry + fsprh - hsy, sprx - fsprw - hsx, spry + fsprh - hsy,
                                    0xFFFF0000);
                hge->Gfx_RenderLine(sprx - fsprw - hsx, spry + fsprh - hsy, sprx - fsprw - hsx, spry - fsprh - hsy,
                                    0xFFFF0000);
            }
            /*if( cbdbisOffsetBorder->isSelected() && (img->GetFileOffsetX() != 0 || img->GetFileOffsetY() != 0) ){
			 float hsx, hsy;
			 spr->GetHotSpot(&hsx, &hsy);
			 float fsprw = spr->GetWidth()/2, fsprh = spr->GetHeight()/2;

			 hsx -= fsprw+img->GetFileOffsetX();
			 hsy -= fsprh+img->GetFileOffsetY();

			 hge->Gfx_RenderLine(sprx-fsprw-hsx, spry-fsprh-hsy, sprx+fsprw-hsx, spry-fsprh-hsy, 0xFFFF00FF);
			 hge->Gfx_RenderLine(sprx+fsprw-hsx, spry-fsprh-hsy, sprx+fsprw-hsx, spry+fsprh-hsy, 0xFFFFFF00);
			 hge->Gfx_RenderLine(sprx+fsprw-hsx, spry+fsprh-hsy, sprx-fsprw-hsx, spry+fsprh-hsy, 0xFFFFFF00);
			 hge->Gfx_RenderLine(sprx-fsprw-hsx, spry+fsprh-hsy, sprx-fsprw-hsx, spry-fsprh-hsy, 0xFFFFFF00);
			}*/
            hge->Gfx_SetClipping();
        }
    } else if (tabindex == 1) {
        hge->Gfx_RenderLine(dx + 461, dy + 54, dx + 461, dy + 190, GV->colLineBright);
        hge->Gfx_RenderLine(dx + 460, dy + 53, dx + 460, dy + 190, GV->colLineDark);
    }

    if (tabindex == 2) {
        hge->Gfx_RenderLine(dx, dy + 191, dx + winDB->getWidth(), dy + 191, GV->colLineBright);
        hge->Gfx_RenderLine(dx, dy + 190, dx + winDB->getWidth(), dy + 190, GV->colLineDark);
    }

    if (!sadbFramesImg->isVisible() && tabindex == 1 ||
        !sadbFramesAni->isVisible() && tabindex == 2) {
        GV->fntMyriad16->printf(dx + 700, dy + 100, HGETEXT_CENTER, GETL(Lang_SelectResourceFromList), 0);
    }

    if (tabindex == 2 && dbAssetsAni->getSelected() != -1) {
        if (dbFramesAni->getSelected() == -1 ||
            dbFramesAni->getSelected() >= dbFramesAni->getListModel()->getNumberOfElements())
            dbFramesAni->setSelected(0);
        ANI::Animation *ani = hAniBank->GetAssetByIterator(dbAssetsAni->getSelected())->GetAni();
        const char *imgset = tfdbAniImageSet->getText().c_str();

        if (cbdbAniAnimate->isSelected() && ani->GetFramesCount() > 1) {
            fdbAniTimer += hge->Timer_GetDelta() * ((dddbAnimSpeed->getSelected() + 1) * 0.25f * 1000.0f);
            while (fdbAniTimer > ani->GetFrame(dbFramesAni->getSelected())->GetDuration()) {
                fdbAniTimer -= ani->GetFrame(dbFramesAni->getSelected())->GetDuration();
                int nframe = dbFramesAni->getSelected() + 1;
                if (nframe >= ani->GetFramesCount()) {
                    nframe = 0;
                    if (!cbdbAniLoop->isSelected())
                        cbdbAniAnimate->setSelected(0);
                }
                dbFramesAni->setSelected(nframe);
            }
        }

        cSprBankAsset *as = SprBank->GetAssetByID(imgset);
        hgeSprite *spr = GV->sprSmiley;
        if (as != 0 && dbFramesAni->getSelected() >= 0 &&
            dbFramesAni->getSelected() < ani->GetFramesCount() &&
            as->GetIMGByID(ani->GetFrame(dbFramesAni->getSelected())->GetImageID()) != 0)
            spr = as->GetIMGByID(ani->GetFrame(dbFramesAni->getSelected())->GetImageID())->GetSprite();

        GV->fntMyriad16->printf(dx + 780, dy + 220, HGETEXT_RIGHT, "%d/%dms (%d)", 0,
                                int(fdbAniTimer), ani->GetFrame(dbFramesAni->getSelected())->GetDuration(),
                                ani->GetFrame(dbFramesAni->getSelected())->GetImageID());

        if (cbdbFlip->isSelected()) {
            spr->SetFlip(1, 0, 1);
        } else
            spr->SetFlip(0, 0, 0);
        spr->Render(dx + 400, dy + 375);
        if (cbdbBorderAni->isSelected()) {
            float hsx, hsy;
            spr->GetHotSpot(&hsx, &hsy);

            float sprw = spr->GetWidth() / 2, sprh = spr->GetHeight() / 2;

            hsx -= sprw;
            hsy -= sprh;

            hge->Gfx_RenderLine(dx + 400 - sprw - hsx, dy + 375 - sprh - hsy, dx + 400 + sprw - hsx,
                                dy + 375 - sprh - hsy, 0x77FF0000);
            hge->Gfx_RenderLine(dx + 400 + sprw - hsx, dy + 375 - sprh - hsy, dx + 400 + sprw - hsx,
                                dy + 375 + sprh - hsy, 0x77FF0000);
            hge->Gfx_RenderLine(dx + 400 + sprw - hsx, dy + 375 + sprh - hsy, dx + 400 - sprw - hsx,
                                dy + 375 + sprh - hsy, 0x77FF0000);
            hge->Gfx_RenderLine(dx + 400 - sprw - hsx, dy + 375 + sprh - hsy, dx + 400 - sprw - hsx,
                                dy + 375 - sprh - hsy, 0x77FF0000);

            hge->Gfx_RenderLine(dx + 400 - 1, dy + 375 - 1, dx + 400 + 1, dy + 375 - 1, 0x77FF0000);
            hge->Gfx_RenderLine(dx + 400 + 1, dy + 375 - 1, dx + 400 + 1, dy + 375 + 1, 0x77FF0000);
            hge->Gfx_RenderLine(dx + 400 + 1, dy + 375 + 1, dx + 400 - 1, dy + 375 + 1, 0x77FF0000);
            hge->Gfx_RenderLine(dx + 400 - 1, dy + 375 + 1, dx + 400 - 1, dy + 375 - 1, 0x77FF0000);
        }
    }

}

void State::EditingWW::DrawTileProperties() {
    if (!hParser) return;
    int dx, dy;
    winTileProp->getAbsolutePosition(dx, dy);

    hge->Gfx_RenderLine(dx - 2, dy + 58, dx + 302, dy + 58, GV->colLineDark);
    hge->Gfx_RenderLine(dx - 2, dy + 59, dx + 302, dy + 59, GV->colLineBright);

    hge->Gfx_RenderLine(dx - 2, dy + 202, dx + 302, dy + 202, GV->colLineDark);
    hge->Gfx_RenderLine(dx - 2, dy + 203, dx + 302, dy + 203, GV->colLineBright);

    if (rbtpDouble->isSelected()) {
        hge->Gfx_RenderLine(dx - 2, dy + 259, dx + 302, dy + 259, GV->colLineDark);
        hge->Gfx_RenderLine(dx - 2, dy + 260, dx + 302, dy + 260, GV->colLineBright);

        hge->Gfx_RenderLine(dx - 2, dy + 320, dx + 302, dy + 320, GV->colLineDark);
        hge->Gfx_RenderLine(dx - 2, dy + 321, dx + 302, dy + 321, GV->colLineBright);
    }

    hge->Gfx_RenderLine(dx + 8 + !btpZoomTile * 32, dy + 64 + !btpZoomTile * 32, dx + 75 + btpZoomTile * 32 + 32,
                        dy + 64 + !btpZoomTile * 32, GV->colLineDark);
    hge->Gfx_RenderLine(dx + 9 + !btpZoomTile * 32, dy + 63 + !btpZoomTile * 32, dx + 9 + !btpZoomTile * 32,
                        dy + 131 + btpZoomTile * 32 + 32, GV->colLineDark);
    hge->Gfx_RenderLine(dx + 76 + btpZoomTile * 32 + 32, dy + 63 + !btpZoomTile * 32, dx + 76 + btpZoomTile * 32 + 32,
                        dy + 131 + btpZoomTile * 32 + 32, GV->colLineDark);
    hge->Gfx_RenderLine(dx + 8 + !btpZoomTile * 32, dy + 131 + btpZoomTile * 32 + 32, dx + 75 + btpZoomTile * 32 + 32,
                        dy + 131 + btpZoomTile * 32 + 32, GV->colLineDark);

    hge->Gfx_RenderLine(dx + 9 + !btpZoomTile * 32, dy + 65 + !btpZoomTile * 32, dx + 74 + btpZoomTile * 32 + 32,
                        dy + 65 + !btpZoomTile * 32, GV->colLineBright);
    hge->Gfx_RenderLine(dx + 10 + !btpZoomTile * 32, dy + 64 + !btpZoomTile * 32, dx + 10 + !btpZoomTile * 32,
                        dy + 130 + btpZoomTile * 32 + 32, GV->colLineBright);
    hge->Gfx_RenderLine(dx + 75 + btpZoomTile * 32 + 32, dy + 64 + !btpZoomTile * 32, dx + 75 + btpZoomTile * 32 + 32,
                        dy + 130 + btpZoomTile * 32 + 32, GV->colLineBright);
    hge->Gfx_RenderLine(dx + 9 + !btpZoomTile * 32, dy + 130 + btpZoomTile * 32 + 32, dx + 75 + btpZoomTile * 32 + 32,
                        dy + 130 + btpZoomTile * 32 + 32, GV->colLineBright);

    GV->fntMyriad16->Render(dx + 145, dy + 66, HGETEXT_LEFT, (std::string(GETL(Lang_Width)) + ':').c_str(), 0);
    GV->fntMyriad16->Render(dx + 145, dy + 91, HGETEXT_LEFT, (std::string(GETL(Lang_Height)) + ':').c_str(), 0);

    GV->fntMyriad16->Render(dx + 145, dy + 114, HGETEXT_LEFT, (std::string(GETL(Lang_Type)) + ':').c_str(), 0);

    GV->fntMyriad16->Render(dx + 8, dy + 208, HGETEXT_LEFT, (
            std::string(GETL(rbtpMask->isSelected() ? Lang_AttribMaskType : Lang_AttribInside)) + ':').c_str(), 0);

    GV->fntMyriad16->Render(dx + 130, dy + 208, HGETEXT_LEFT, GETL(Lang_PropClear), 0);
    GV->fntMyriad16->Render(dx + 230, dy + 208, HGETEXT_LEFT, GETL(Lang_PropSolid), 0);

    GV->fntMyriad16->Render(dx + 30, dy + 238, HGETEXT_LEFT, GETL(Lang_PropGround), 0);
    GV->fntMyriad16->Render(dx + 130, dy + 238, HGETEXT_LEFT, GETL(Lang_PropClimb), 0);
    GV->fntMyriad16->Render(dx + 230, dy + 238, HGETEXT_LEFT, GETL(Lang_PropDeath), 0);

    if (rbtpDouble->isSelected()) {
        GV->fntMyriad16->Render(dx + 130, dy + 268, HGETEXT_LEFT, GETL(Lang_PropClear), 0);
        GV->fntMyriad16->Render(dx + 230, dy + 268, HGETEXT_LEFT, GETL(Lang_PropSolid), 0);

        GV->fntMyriad16->Render(dx + 30, dy + 298, HGETEXT_LEFT, GETL(Lang_PropGround), 0);
        GV->fntMyriad16->Render(dx + 130, dy + 298, HGETEXT_LEFT, GETL(Lang_PropClimb), 0);
        GV->fntMyriad16->Render(dx + 230, dy + 298, HGETEXT_LEFT, GETL(Lang_PropDeath), 0);

        GV->fntMyriad16->Render(dx + 8, dy + 268, HGETEXT_LEFT, (std::string(GETL(Lang_AttribOutside)) + ':').c_str(), 0);
        GV->fntMyriad16->Render(dx + 105, dy + 327, HGETEXT_RIGHT, GETL(Lang_Mask), 0);
        GV->fntMyriad16->Render(dx + 150, dy + 327, HGETEXT_LEFT, "x", 0);
        GV->fntMyriad16->Render(dx + 202, dy + 325, HGETEXT_LEFT, "-", 0);
        GV->fntMyriad16->Render(dx + 250, dy + 327, HGETEXT_LEFT, "x", 0);
    }

    double widthMod = 64.f / hParser->GetMainPlane()->GetTileWidth(),
           heightMod = 64.f / hParser->GetMainPlane()->GetTileHeight();

    if (btpZoomTile) {
        widthMod *= 2.0f;
        heightMod *= 2.0f;
    } else {
        dx += 32;
        dy += 32;
    }

    cTile *tile = hTileset->GetTile(hParser->GetMainPlane()->GetImageSet(0), itpSelectedTile);
    if (tile != NULL) {
        if (!rbtpSingle->isSelected()) {
            float mx, my;
            hge->Input_GetMousePos(&mx, &my);
            if (btpDragDropMask && !hge->Input_GetKeyState(HGEK_LBUTTON)) {
                if (rbtpMask->isSelected()) {
                    int i = 0;
                    for (; i < 5; ++i) {
                        if (rbtpIn[i]->isSelected()) break;
                    }
                    ((WWD::MaskTileAttrib *) hTempAttrib)->setArea(std::min(itpDDx1, itpDDx2),
                                                                   std::min(itpDDy1, itpDDy2),
                                                                   std::max(itpDDx1, itpDDx2),
                                                                   std::max(itpDDy1, itpDDy2),
                                                                   (WWD::TILE_ATTRIB)i);
                }
                btpDragDropMask = false;
            } else if (!btpDragDropMask && hge->Input_KeyDown(HGEK_LBUTTON) &&
                       mx > dx + 10 && mx < dx + 10 + 64 + btpZoomTile * 64 &&
                       my > dy + 65 && my < dy + 65 + 64 + btpZoomTile * 64) {
                btpDragDropMask = 1;
                itpDDx1 = (mx - dx - 10) / widthMod;
                itpDDy1 = (my - dy - 65) / heightMod;
                itpDDx2 = itpDDx1;
                itpDDy2 = itpDDy1;
            } else if (btpDragDropMask) {
                itpDDx2 = (mx - dx - 10) / widthMod;
                itpDDy2 = (my - dy - 65) / heightMod;

                if (itpDDx2 >= hTempAttrib->getWidth()) itpDDx2 = hTempAttrib->getWidth() - 1;
                if (itpDDy2 >= hTempAttrib->getHeight()) itpDDy2 = hTempAttrib->getHeight() - 1;
                if (itpDDx2 < 0) itpDDx2 = 0;
                if (itpDDy2 < 0) itpDDy2 = 0;

                if (rbtpDouble->isSelected()) {
                    ((WWD::DoubleTileAttrib *) hTempAttrib)->setMask(std::min(itpDDx1, itpDDx2),
                                                                     std::min(itpDDy1, itpDDy2),
                                                                     std::max(itpDDx1, itpDDx2),
                                                                     std::max(itpDDy1, itpDDy2));
                    char tmp[16];
                    sprintf(tmp, "%d", std::min(itpDDx1, itpDDx2));
                    tftpX1->setText(tmp);
                    sprintf(tmp, "%d", std::min(itpDDy1, itpDDy2));
                    tftpY1->setText(tmp);

                    sprintf(tmp, "%d", std::max(itpDDx1, itpDDx2));
                    tftpX2->setText(tmp);
                    sprintf(tmp, "%d", std::max(itpDDy1, itpDDy2));
                    tftpY2->setText(tmp);
                }
            }
        }

        tile->GetImage()->SetColor(0xffffffff);
        tile->GetImage()->SetFlip(0, 0);
        tile->GetImage()->SetHotSpot(0, 0);
        tile->GetImage()->RenderEx(dx + 10, dy + 65, 0, widthMod, heightMod);
        DrawTileAttributes(hTempAttrib, dx + 10, dy + 65, widthMod, heightMod);
    } else {

    }
}

int State::EditingWW::RenderPlane(WWD::Plane *plane, int pl) {
    int rcount = 0;

    int psx = std::max(Wrd2ScrXrb(hParser->GetMainPlane(), 0), 0),
        psy = std::max(Wrd2ScrYrb(hParser->GetMainPlane(), 0), 0);

    hge->Gfx_SetClipping(psx - 1, psy - 1, std::min(Wrd2ScrXrb(hParser->GetMainPlane(), hParser->GetMainPlane()->GetPlaneWidthPx()), vPort->GetWidth()) + 1,
                         std::min(Wrd2ScrYrb(hParser->GetMainPlane(), hParser->GetMainPlane()->GetPlaneHeightPx()), vPort->GetHeight()) + 1);

    float cammx = fCamX * fZoom,
          cammy = fCamY * fZoom;

    if (!plane->GetFlag(WWD::Flag_p_MainPlane)) {
        cammx *= (plane->GetMoveModX() / 100.0f);
        cammy *= (plane->GetMoveModY() / 100.0f);

        if (plane->GetFlag(WWD::Flag_p_XWrapping)) {
            cammx -= vPort->GetWidth() / 2.f * (1.0f - (plane->GetMoveModX() / 100.0f));
        } else if (cammx < 0) {
            cammx = -psx;
        }

        if (plane->GetFlag(WWD::Flag_p_YWrapping)) {
            cammy -= vPort->GetHeight() / 2.f * (1.0f - (plane->GetMoveModY() / 100.0f));
        } else if (cammy < 0) {
            cammy = -psy;
        }
    }

    int sx = floor(cammx / (plane->GetTileWidth() * fZoom)),
        sy = floor(cammy / (plane->GetTileHeight() * fZoom)),
        ex = sx + vPort->GetWidth() / (plane->GetTileWidth() * fZoom) + 2,
        ey = sy + vPort->GetHeight() / (plane->GetTileHeight() * fZoom) + 2;

    if (!plane->GetFlag(WWD::Flag_p_XWrapping)) {
        if (sx < 0) sx = 0;
        if (ex > plane->GetPlaneWidth()) ex = plane->GetPlaneWidth();
    }

    if (!plane->GetFlag(WWD::Flag_p_YWrapping)) {
        if (sy < 0) sy = 0;
        if (ey > plane->GetPlaneHeight()) ey = plane->GetPlaneHeight();
    }

    DWORD col = 0xFFFFFFFF;
    if (GV->bAlphaHigherPlanes && (hParser->GetFlag(WWD::Flag_w_UseZCoords)
        ? plane->GetZCoord() > GetActivePlane()->GetZCoord()
        : pl > GetActivePlaneID())) {
        col = 0x77FFFFFF;
    }

    float endplanex = ex * plane->GetTileWidth() * fZoom - cammx,
          endplaney = ey * plane->GetTileHeight() * fZoom - cammy;

    bool bUsingBuffer = (hPlaneData[pl]->hRB != NULL);
    /*if (bUsingBuffer) {
        rcount += hPlaneData[pl]->hRB->GetEntityCount(sx, sy, ex - 1, ey - 1);
        int startx = (sx - (sx % plane->GetPlaneWidth())) * plane->GetTileWidth() * fZoom - cammx,
            starty = (sy - (sy % plane->GetPlaneHeight())) * plane->GetTileHeight() * fZoom - cammy;
        for (int y = starty;
             (plane->GetFlag(WWD::Flag_p_YWrapping) && y < vPort->GetHeight() ||
              !plane->GetFlag(WWD::Flag_p_YWrapping) && y == starty);
             y += (plane->GetPlaneHeightPx() * fZoom)) {
            for (int x = startx;
                 (plane->GetFlag(WWD::Flag_p_XWrapping) && x < vPort->GetWidth() ||
                  !plane->GetFlag(WWD::Flag_p_XWrapping) && x == startx);
                 x += (plane->GetPlaneWidthPx() * fZoom)) {
                hPlaneData[pl]->hRB->Render(x, y, fZoom);
            }
        }
    }*/

    bool ghosting = false;
    for (auto & i : vTileGhosting)
        if (i.pl == plane) {
            ghosting = true;
            break;
        }

    std::sort(std::begin(vTileGhosting), std::end(vTileGhosting), [](TileGhost& a, TileGhost& b) {
        if (a.y == b.y) return a.x < b.x;
        return a.y < b.y;
    });
    auto tgIterator = vTileGhosting.begin();

    float th = plane->GetTileHeight() * fZoom,
          tw = plane->GetTileWidth() * fZoom,
          posY = floor(sy * th - cammy);

    bool pickerHighlight = false;
    if (!bUsingBuffer || ghosting)
        for (int y = sy; y < ey; y++, posY += th) {
            float posX = floor(sx * tw - cammx);
            for (int x = sx; x < ex; x++, posX += tw) {
                WWD::Tile* tile = plane->GetTile(x, y);
                if (ghosting) {
                    bool ghosted = false;
                    int cx = plane->GetFlag(WWD::Flag_p_XWrapping) ? plane->ClampX(x) : x,
                        cy = plane->GetFlag(WWD::Flag_p_YWrapping) ? plane->ClampY(y) : y;
                    while (tgIterator != vTileGhosting.end() && (cy > tgIterator->y || (cy == tgIterator->y && cx > tgIterator->x)))
                        ++tgIterator;

                    if (tgIterator != vTileGhosting.end()) {
                        if (tgIterator->id == EWW_TILE_PIPETTE) {
                            if (plane == tgIterator->pl && x == tgIterator->x && y == tgIterator->y) {
                                pickerHighlight = true;
                                if (tile->IsInvisible()) {
                                    tgIterator->id = EWW_TILE_ERASE;
                                }
                                else if (tile->IsFilled()) {
                                    tgIterator->id = EWW_TILE_FILL;
                                }
                                else {
                                    tgIterator->id = tile->GetID();
                                }
                            }
                        } else if (cy == tgIterator->y && cx == tgIterator->x && plane == tgIterator->pl) {
                            if (tgIterator->id >= 0 && tgIterator->id != tile->GetID()) {
                                hgeSprite *spr;
                                spr = hTileset->GetSet(plane->GetImageSet(0))->GetTile(tgIterator->id)->GetImage();
                                spr->SetColor(SETA(col, 160));
                                spr->SetHotSpot(0, 0);
                                spr->RenderEx(posX, posY, 0, fZoom);
                                if (bDrawTileProperties && pl == GetActivePlaneID()) {
                                    DrawTileAttributes(tgIterator->id, posX, posY, fZoom, fZoom);
                                }
                                ghosted = true;
                            } else if (tgIterator->id == EWW_TILE_ERASE && !tile->IsInvisible()) {
                                ghosted = true;
                            } else if (tgIterator->id == EWW_TILE_FILL && !tile->IsFilled()) {
                                SHR::SetQuad(&q, SETA(hDataCtrl->GetPalette()->GetColor(plane->GetFillColor()), GETA(col)),
                                             posX, posY, ceil(posX + tw), ceil(posY + th));
                                hge->Gfx_RenderQuad(&q);
                                ghosted = true;
                            }
                        }
                    }
                    
                    if (plane->GetFlags() & WWD::Flag_p_XWrapping && cx == plane->GetPlaneWidth() - 1) {
                        tgIterator = vTileGhosting.begin();
                    }

                    if (ghosted) continue;
                }

                //if (!bUsingBuffer)
                    if (tile->IsFilled()) {
                        SHR::SetQuad(&q, SETA(hDataCtrl->GetPalette()->GetColor(plane->GetFillColor()), GETA(col)),
                                     posX, posY, ceil(posX + tw), ceil(posY + th));
                        hge->Gfx_RenderQuad(&q);
                        rcount++;
                    } else if (!tile->IsInvisible()) {
                        hgeSprite *spr;
                        //uint16_t imgSetIndex = tile->GetID() >> 16;
                        uint16_t id = tile->GetID();
                        cTile *tileImg = hTileset->GetTile(plane->GetImageSet(0), id);
                        if (!tileImg) {
                            GV->sprTile->RenderEx(posX, posY, 0, fZoom);
                            char tmp[32];
                            sprintf(tmp, "ID#%d", plane->GetTile(x, y)->GetID());
                            GV->fntMyriad16->SetColor(0x99FFFFFF);
                            GV->fntMyriad16->Render(posX + plane->GetTileWidth() / 2 * fZoom,
                                                    posY + plane->GetTileHeight() * 0.75 * fZoom - 10,
                                                    HGETEXT_CENTER,
                                                    tmp, 0);
                        } else {
                            spr = tileImg->GetImage();
                            spr->SetColor(col);
                            spr->SetFlip(0, 0);
                            spr->SetHotSpot(0, 0);
                            spr->RenderEx(posX, posY, 0, fZoom + 0.0001);
                        }

                        if (bDrawTileProperties && pl == GetActivePlaneID()) {
                            DrawTileAttributes(tile->GetID(), posX, posY, fZoom + 0.0001, fZoom);
                        }
                        rcount++;
                    }

                //if (bUsingBuffer) continue;

                if (hPlaneData[pl]->bDrawBoundary && x == 0) {
                    hge->Gfx_RenderLine(posX, std::max(0.f, -cammy), posX, endplaney, ARGB(GETA(col), 255, 0, 255));
                }

                if (hPlaneData[pl]->bDrawBoundary && (plane->GetFlag(WWD::Flag_p_XWrapping)
                    ? x % plane->GetPlaneWidth() == plane->GetPlaneWidth() - 1
                    : x == plane->GetPlaneWidth() - 1)) {
                    hge->Gfx_RenderLine(posX + tw, std::max(0.f, -cammy),
                                        posX + tw, endplaney,
                                        ARGB(GETA(col), 255, 0, 255));
                } else if (hPlaneData[pl]->bDrawGrid) {
                    hge->Gfx_RenderLine(posX + tw, std::max(0.f, -cammy),
                                        posX + tw, endplaney,
                                        ARGB(GETA(col), 255, 255, 255));
                }
            }
            //if (bUsingBuffer) continue;

            if (hPlaneData[pl]->bDrawBoundary && y == 0) {
                hge->Gfx_RenderLine(std::max(0.f, -cammx), posY, endplanex, posY, ARGB(GETA(col), 255, 0, 255));
            }

            if (hPlaneData[pl]->bDrawBoundary && (plane->GetFlag(WWD::Flag_p_YWrapping)
                ? y % plane->GetPlaneHeight() == plane->GetPlaneHeight() - 1
                : y == plane->GetPlaneHeight() - 1)) {
                hge->Gfx_RenderLine(std::max(0.f, -cammx), ceil(posY + th), endplanex,
                                    ceil(posY + th), ARGB(GETA(col), 255, 0, 255));
            } else if (hPlaneData[pl]->bDrawGrid)
                hge->Gfx_RenderLine(std::max(0.f, -cammx), ceil(posY + th), endplanex,
                                    ceil(posY + th), ARGB(GETA(col), 255, 255, 255));

            if (ghosting && plane->GetFlags() & WWD::Flag_p_YWrapping && y % plane->GetPlaneHeight() == plane->GetPlaneHeight() - 1) {
                tgIterator = vTileGhosting.begin();
            }
        }

    if (pickerHighlight) {
        for (auto & highlight : vTileGhosting) {
            float posX = highlight.x * tw - cammx, posY = highlight.y * th - cammy;

            hge->Gfx_RenderLine(posX, posY, posX + tw, posY, GV->colLineDark);
            hge->Gfx_RenderLine(posX, posY + th, posX + tw, posY + th, GV->colLineDark);
            hge->Gfx_RenderLine(posX, posY - 1, posX - 1, posY + th, GV->colLineDark);
            hge->Gfx_RenderLine(posX + tw, posY - 1, posX + tw, posY + th, GV->colLineDark);

            int color = (highlight.x == iTileWriteIDx && highlight.y == iTileWriteIDy) ? TILE_PICK_COLOR_SOLID : TILE_HIGHLIGHT_COLOR_SOLID;
            hge->Gfx_RenderLine(posX, posY + 1, posX + tw - 1, posY + 1, color);
            hge->Gfx_RenderLine(posX + 1, posY + th - 1, posX + tw - 1, posY + th - 1, color);
            hge->Gfx_RenderLine(posX + 1, posY + 1, posX + 1, posY + th - 1, color);
            hge->Gfx_RenderLine(posX + tw - 1, posY + 1, posX + tw - 1, posY + th - 1, color);

            char text[32];
            if (highlight.id == EWW_TILE_ERASE)
                sprintf(text, "ID: -");
            else if (highlight.id == EWW_TILE_FILL)
                sprintf(text, "ID: FILL");
            else
                sprintf(text, "ID: %d", highlight.id);
            GV->fntMyriad16->SetColor(0xFF000000);
            GV->fntMyriad16->Render(posX + tw / 2 + 1, posY + th / 2 + 1 - 8, HGETEXT_CENTER, text, 0);
            GV->fntMyriad16->SetColor(0xFFFFFFFF);
            GV->fntMyriad16->Render(posX + tw / 2, posY + th / 2 - 8, HGETEXT_CENTER, text, 0);
        }
    }

    hge->Gfx_SetClipping();

    DrawPlaneOverlay(plane);
    return rcount;
}

void State::EditingWW::RenderToViewportBuffer() {
    if (hParser == NULL) {
        q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;
        SHR::SetQuad(&q, 0xFF0b0b0b, 0, 0, hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT));
        hge->Gfx_RenderQuad(&q);
        return;
    }

    for (int y = 0; y <= vPort->GetHeight() / 120; y++)
        for (int x = 0; x <= vPort->GetWidth() / 120; x++) {
            GV->sprCheckboard->Render(x * 120, y * 120);
        }

    float mx, my;
    hge->Input_GetMousePos(&mx, &my);

    std::vector<cRenderableItem *> vRenderOrders;
    for (int i = 0; i < hParser->GetPlanesCount(); i++) {
        if (!hPlaneData[i]->bDraw) continue;
        vRenderOrders.push_back(new cPlaneRenderWrapper(hParser->GetPlane(i), i));

        if (hPlaneData[i]->bDrawObjects && hPlaneData[i]->ObjectData.hQuadTree != NULL) {
            std::vector<WWD::Object *> list = hPlaneData[i]->ObjectData.hQuadTree->GetObjectsByArea(
                    Scr2WrdX(hParser->GetPlane(i), vPort->GetX()),
                    Scr2WrdY(hParser->GetPlane(i), vPort->GetY()),
                    vPort->GetWidth() / fZoom,
                    vPort->GetHeight() / fZoom);
            for (auto & item : list) {
                if (iActiveTool == EWW_TOOL_EDITOBJ && item == hEditObj->GetOrigObj()) continue;
                vRenderOrders.push_back(new cObjectRenderWrapper(item));
            }
        }
    }

    sort(vRenderOrders.begin(), vRenderOrders.end(), RenderableItemSortZ);

    iTilesOnScreen = 0;
    iObjectsOnScreen = 0;
    while (!vRenderOrders.empty()) {
        int c = vRenderOrders.front()->Render();
        if (vRenderOrders.front()->GetType() == cRenderableItem::PlaneWrapper)
            iTilesOnScreen += c;
        else if (vRenderOrders.front()->GetType() == cRenderableItem::ObjectWrapper)
            iObjectsOnScreen += c;
        delete vRenderOrders.front();
        vRenderOrders.erase(vRenderOrders.begin());
    }
#ifdef DEBUG_DRAW
    hge->Gfx_RenderLine(0, 0, vPort->GetWidth(), 0, 0xFFFF00FF);
	hge->Gfx_RenderLine(0, 0, 0, vPort->GetHeight(), 0xFFFF00FF);
	hge->Gfx_RenderLine(vPort->GetWidth(), vPort->GetHeight(), vPort->GetWidth(), 0, 0xFFFF00FF);
	hge->Gfx_RenderLine(vPort->GetWidth(), vPort->GetHeight(), 0, vPort->GetHeight(), 0xFFFF00FF);
#endif
}

void State::EditingWW::DrawWorldP() {
    int dx, dy;
    byte alpha = winWorld->getAlpha();
    winWorld->getAbsolutePosition(dx, dy);
    hge->Gfx_RenderLine(dx, dy + 110, dx + winWorld->getWidth(), dy + 110, SETA(GV->colLineBright, alpha));
    hge->Gfx_RenderLine(dx, dy + 109, dx + winWorld->getWidth(), dy + 109, SETA(GV->colLineDark, alpha));

    hge->Gfx_RenderLine(dx, dy + 220, dx + winWorld->getWidth(), dy + 220, SETA(GV->colLineBright, alpha));
    hge->Gfx_RenderLine(dx, dy + 219, dx + winWorld->getWidth(), dy + 219, SETA(GV->colLineDark, alpha));

    /*hge->Gfx_RenderLine(dx + 400, dy + 20, dx + 400, dy + winWorld->getHeight(), SETA(GV->colLineDark, alpha));
    hge->Gfx_RenderLine(dx + 401, dy + 20, dx + 401, dy + winWorld->getHeight(), SETA(GV->colLineBright, alpha));*/
}

void State::EditingWW::DrawViewport() {
    int iCamX = fCamX, iCamY = fCamY;
    hge->Gfx_SetClipping();
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);

    vPort->Render();

    if (hParser != NULL) {
        int cammx = int(iCamX * (GetActivePlane()->GetMoveModX() / 100.0f) * fZoom);
        int cammy = int(iCamY * (GetActivePlane()->GetMoveModY() / 100.0f) * fZoom);
        vPort->ClipScreen();

        if (MDI->GetActiveDoc()->vGuides.size() > 0 && bShowGuideLines) {
            WWD::Plane *mainPl;
            for (int i = 0; i < hParser->GetPlanesCount(); i++)
                if (hParser->GetPlane(i)->GetFlag(WWD::Flag_p_MainPlane)) {
                    mainPl = hParser->GetPlane(i);
                    break;
                }

            for (size_t i = 0; i < MDI->GetActiveDoc()->vGuides.size(); i++) {
                stGuideLine gl = MDI->GetActiveDoc()->vGuides[i];
                if (gl.iPos < 0) continue;
                if (gl.bOrient == GUIDE_HORIZONTAL) {
                    int val = Wrd2ScrY(mainPl, gl.iPos);
                    hge->Gfx_RenderLine(vPort->GetX(), val, vPort->GetX() + vPort->GetWidth(), val, 0xFF30DBD9);
                } else {
                    int val = Wrd2ScrX(mainPl, gl.iPos);
                    hge->Gfx_RenderLine(val, vPort->GetY(), val, vPort->GetY() + vPort->GetHeight(), 0xFF30DBD9);
                }
                if (i == iManipulatedGuide && gl.iPos >= 0) {
                    GV->fntMyriad16->SetColor(0xFF000000);
                    GV->fntMyriad16->printf(mx + 14, my + 16, HGETEXT_LEFT,
                                            (gl.bOrient == GUIDE_HORIZONTAL ? "Y: %d" : "X: %d"), 0, gl.iPos);
                    GV->fntMyriad16->SetColor(0xFFFFFFFF);
                    GV->fntMyriad16->printf(mx + 15, my + 15, HGETEXT_LEFT,
                                            (gl.bOrient == GUIDE_HORIZONTAL ? "Y: %d" : "X: %d"), 0, gl.iPos);
                }
            }
        }

        //if (!bDragSelection) {
            for (auto obj : vObjectsHL) {
                WWD::Rect box = SprBank->GetObjectRenderRect(obj);
                box.x2 += box.x1;
                box.y2 += box.y1;
                box.x1 = Wrd2ScrX(GetActivePlane(), box.x1);
                box.y1 = Wrd2ScrY(GetActivePlane(), box.y1);
                box.x2 = Wrd2ScrX(GetActivePlane(), box.x2);
                box.y2 = Wrd2ScrY(GetActivePlane(), box.y2);
                hge->Gfx_RenderLine(box.x1 - 1, box.y1, box.x2, box.y1, GV->colActive);
                hge->Gfx_RenderLine(box.x2, box.y1, box.x2, box.y2, GV->colActive);
                hge->Gfx_RenderLine(box.x2, box.y2, box.x1, box.y2, GV->colActive);
                hge->Gfx_RenderLine(box.x1, box.y2, box.x1, box.y1, GV->colActive);

                int x = Wrd2ScrX(GetActivePlane(), obj->GetX()), y = Wrd2ScrY(GetActivePlane(), obj->GetY());
                hge->Gfx_RenderLine(x - 3, y - 3, x + 3, y + 3, GV->colActive);
                hge->Gfx_RenderLine(x - 3, y + 3, x + 3, y - 3, GV->colActive);
            }
        //}

        if (iMode == EWW_MODE_OBJECT && iActiveTool == EWW_TOOL_NONE && hge->Input_GetKeyState(HGEK_ALT) &&
            !vObjectClipboard.empty()) {
            std::vector<WWD::Object *> rend = vObjectClipboard;

            sort(rend.begin(), rend.end(), ObjSortCoordZ);
            float mx, my;
            hge->Input_GetMousePos(&mx, &my);
            int diffx = Scr2WrdX(GetActivePlane(), mx), diffy = Scr2WrdY(GetActivePlane(), my);
            diffx -= vObjectClipboard[0]->GetParam(WWD::Param_LocationX);
            diffy -= vObjectClipboard[0]->GetParam(WWD::Param_LocationY);
            for (int i = 0; i < rend.size(); i++) {
                //if( (rend[i]->GetDrawFlags() & WWD::Flag_dr_NoDraw) && !cbdoNoDraw->isSelected() ) continue;
                hgeSprite *spr = SprBank->GetObjectSprite(rend[i]);
                spr->SetColor(0x77FFFFFF);
                spr->SetFlip(rend[i]->GetFlipX(), rend[i]->GetFlipY(), 1);
                int rx = Wrd2ScrX(GetActivePlane(), rend[i]->GetX() + diffx),
                    ry = Wrd2ScrY(GetActivePlane(), rend[i]->GetY() + diffy);
                spr->RenderEx(rx, ry, 0, fZoom);
            }
        }

        if (winDuplicate->isVisible() && atoi(tfdTimes->getText().c_str()) > 0) {
            int count = atoi(tfdTimes->getText().c_str());
            int offx = atoi(tfdOffsetX->getText().c_str());
            int offy = atoi(tfdOffsetY->getText().c_str());
            for (int i = 0; i < count; i++) {
                for (auto & obj : vObjectsPicked) {
                    if (obj != hStartingPosObj) {
                        RenderObject(obj,
                                     Wrd2ScrX(GetActivePlane(), obj->GetX() + offx * (i + 1)),
                                     Wrd2ScrY(GetActivePlane(), obj->GetY() + offy * (i + 1)),
                                     0x77FFFFFF);
                    }
                }
            }
        }

        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        for (int i = 0; i < vObjectsPicked.size(); i++) {
            /*if (fnd) {
                if (iActiveTool == EWW_TOOL_EDITOBJ)
                    hEditObj->RenderObjectOverlay();
                continue;
            }*/
            WWD::Object *obj = vObjectsPicked[i];

            hgeSprite *spr = SprBank->GetObjectSprite(obj);
            float hsx, hsy;
            spr->GetHotSpot(&hsx, &hsy);
            float sprW = spr->GetWidth() / 2, sprH = spr->GetHeight() / 2;

            hsx -= sprW;
            hsy -= sprH;

            float posX = (obj->GetX() - hsx) * fZoom,
                  posY = (obj->GetY() - hsy) * fZoom;

            sprW *= fZoom;
            sprH *= fZoom;

            if (vObjectsPicked.size() == 1) {
                LogicInfo logicInfo = GetLogicInfo(obj->GetLogic());

                bool bDrawObjProperties = 1;
                int minX = obj->GetParam(WWD::Param_MinX), minY = obj->GetParam(WWD::Param_MinY),
                    maxX = obj->GetParam(WWD::Param_MaxX), maxY = obj->GetParam(WWD::Param_MaxY);

                if (logicInfo.IsSoundTrigger() ||
                    !strcmp(obj->GetLogic(), "GlobalAmbientSound") || !strcmp(obj->GetLogic(), "AmbientSound")) {
                    WWD::Rect r[3];
                    for (int i = 0; i < 3; i++) r[i] = WWD::Rect(0, 0, 0, 0);

                    if (logicInfo.IsSoundTrigger()) {
                        r[0].x1 = obj->GetX();
                        r[0].y1 = obj->GetY();
                        int w = 0, h = 0;
                        if (strstr(obj->GetLogic(), "Tiny")) { w = 16; }
                        else if (strstr(obj->GetLogic(), "Small")) { w = 32; }
                        else if (strstr(obj->GetLogic(), "Big")) { w = 128; }
                        else if (strstr(obj->GetLogic(), "Huge")) { w = 256; }
                        else if (strstr(obj->GetLogic(), "Wide")) {
                            w = 100;
                            h = 32;
                        }
                        else if (strstr(obj->GetLogic(), "Tall")) {
                            w = 32;
                            h = 100;
                        }
                        else { w = 64; }
                        if (!h) h = w;
                        r[0].x2 = r[0].x1 + w;
                        r[0].y2 = r[0].y1 + h;
                        r[0].x1 = r[0].x1 - w;
                        r[0].y1 = r[0].y1 - h;
                    } else {
                        r[0].x1 = obj->GetParam(WWD::Param_MinX);
                        r[0].y1 = obj->GetParam(WWD::Param_MinY);
                        r[0].x2 = obj->GetParam(WWD::Param_MaxX);
                        r[0].y2 = obj->GetParam(WWD::Param_MaxY);
                        r[1] = obj->GetAttackRect();
                    }

                    char szRectID[3][32];
                    sprintf(szRectID[0], "SoundRect_1");
                    sprintf(szRectID[1], "SoundRect_2");
                    sprintf(szRectID[2], "SoundRect_3");
                    for (int i = 1; i >= 0; i--)
                        if (!r[i].x1 || !r[i].y1 || !r[i].x2 || !r[i].y2) {
                            r[i] = r[i + 1];
                            strcpy(szRectID[i], szRectID[i + 1]);
                            szRectID[i + 1][0] = '\0';
                            r[i + 1] = WWD::Rect(0, 0, 0, 0);
                        }

                    bool bmulticolor = 0;
                    for (int i = 1; i < 3; i++) if (r[i].x1 && r[i].y1 && r[i].x2 && r[i].y2) bmulticolor = 1;

                    DWORD dwCols[3] = {0xAAFF0000, 0xAA00FF00, 0xAA0000FF};
                    for (int i = 0; i < 3; i++) {
                        if (!r[i].x1 || !r[i].y1 || !r[i].x2 || !r[i].y2) continue;
                        r[i].x1 = Wrd2ScrX(GetActivePlane(), r[i].x1);
                        r[i].y1 = Wrd2ScrY(GetActivePlane(), r[i].y1);
                        r[i].x2 = Wrd2ScrX(GetActivePlane(), r[i].x2);
                        r[i].y2 = Wrd2ScrY(GetActivePlane(), r[i].y2);
                        RenderAreaRect(r[i], true, bmulticolor ? dwCols[i] : 0xFFFFFFFF);
                        if (bmulticolor) {
                            int cx = std::max(vPort->GetX(), r[i].x1),
                                cy = std::max(vPort->GetY(), r[i].y1);
                            GV->fntMyriad16->printf(cx + 11, cy + 11, HGETEXT_LEFT, "~l~%s", 0,
                                                    GETL2S("EditObj_Dialog", szRectID[i]));
                            GV->fntMyriad16->SetColor(SETA(dwCols[i], 255));
                            GV->fntMyriad16->printf(cx + 10, cy + 10, HGETEXT_LEFT, "%s", 0,
                                                    GETL2S("EditObj_Dialog", szRectID[i]));
                        }
                    }
                    vPort->ClipScreen();
                    //bDrawObjProperties = iActiveTool != EWW_TOOL_EDITOBJ;
                } else {
                    int iSpringboardValue = 0;
                    bool bFill = true;
                    if (!strcmp(vObjectsPicked[0]->GetLogic(), "SpringBoard") ||
                        !strcmp(vObjectsPicked[0]->GetLogic(), "GroundBlower") ||
                        !strcmp(vObjectsPicked[0]->GetLogic(), "WaterRock")) {
                        if (iActiveTool == EWW_TOOL_EDITOBJ && hEditObj->iType == ObjEdit::enSpringboard &&
                            ((ObjEdit::cEditObjSpringboard *) hEditObj)->bPicking) {
                            minY = ((ObjEdit::cEditObjSpringboard *) hEditObj)->iPickY;
                        } else {
                            if (!maxY) minY = 450;
                            else minY = maxY;
                        }
                        iSpringboardValue = minY;
                        minY = obj->GetY() - minY;
                        maxX = minX = maxY = 0;
                        bFill = false;
                    } else if (!strcmp(vObjectsPicked[0]->GetLogic(), "Shake")) {
                        WWD::Rect r = obj->GetAttackRect();
                        minX = r.x1;
                        minY = r.y1;
                        maxX = r.x2;
                        maxY = r.y2;
                    } else if (!strcmp(vObjectsPicked[0]->GetLogic(), "Stalactite")) {
                        WWD::Rect r = obj->GetUserRect(0);
                        minX = r.x1;
                        minY = r.y1;
                        maxX = r.x2;
                        maxY = r.y2;
                    } else if (!strcmp(vObjectsPicked[0]->GetLogic(), "SpotAmbientSound") ||
                               !strcmp(vObjectsPicked[0]->GetLogic(), "AmbientPosSound")) {
                        minX = obj->GetX();
                        minY = obj->GetY();
                        maxX = std::min(GetActivePlane()->GetPlaneWidthPx(), minX + 640);
                        maxY = std::min(GetActivePlane()->GetPlaneHeightPx(), minY + 640);
                        minX = std::max(0, minX - 640);
                        minY = std::max(0, minY - 640);
                    } else if (strstr(obj->GetLogic(), "Elevator")
                        && strcmp(obj->GetLogic(), "PathElevator") != 0
                        && strcmp(obj->GetLogic(), "SlidingElevator") != 0) {
                        int width = obj->GetParam(WWD::Param_Width);
                        int height = obj->GetParam(WWD::Param_Height);

                        if (!strcmp(obj->GetLogic(), "StandardElevator")) {
                            if (!width && minX > 0) width = minX;
                            if (!height && maxX > 0) height = maxX;

                            if (width <= 0) {
                                width = 128;
                            }
                            minX = obj->GetX() - width / 2;
                            maxX = obj->GetX() + width / 2;

                            if (height <= 0) {
                                height = 0;

                                if (!minY) minY = obj->GetY();
                                if (!maxY) maxY = obj->GetY();
                            } else {
                                minY = obj->GetY() - height / 2;
                                maxY = obj->GetY() + height / 2;
                            }
                        } else if (!strcmp(obj->GetLogic(), "Elevator")) {
                            if (width > 0) {
                                minX = obj->GetX() - width / 2;
                                maxX = obj->GetX() + width / 2;
                            }

                            if (height > 0) {
                                minY = obj->GetY() - height / 2;
                                maxY = obj->GetY() + height / 2;
                            }

                            if (maxX - minX < 1) {
                                minX = 0;
                                maxX = 0;
                            }

                            if (maxY - minY < 1) {
                                minY = 0;
                                maxY = 0;
                            }
                        } else {
                            if (!minX) minX = obj->GetX();
                            if (!maxX) maxX = obj->GetX();
                            if (!minY) minY = obj->GetY();
                            if (!maxY) maxY = obj->GetY();
                        }

                        if (iActiveTool == EWW_TOOL_MOVEOBJECT) {
                            if (!width) {
                                int diffX = obj->GetX() - obj->GetParam(WWD::Param_LocationX);
                                if (minX) minX += diffX;
                                if (maxX) maxX += diffX;
                            }

                            if (!height) {
                                int diffY = obj->GetY() - obj->GetParam(WWD::Param_LocationY);
                                if (minY) minY += diffY;
                                if (maxY) maxY += diffY;
                            }
                        }
                    }

                    if ((minX != 0 || minY != 0 || maxX != 0 || maxY != 0) && (minX != maxX || minY != maxY)) {

                        WWD::Rect rect(minX && minX != maxX ? Wrd2ScrX(hParser->GetMainPlane(), minX) : 0,
                                       minY && minY != maxY ? Wrd2ScrY(hParser->GetMainPlane(), minY) : 0,
                                       maxX && minX != maxX ? Wrd2ScrX(hParser->GetMainPlane(), maxX) : 0,
                                       maxY && minY != maxY ? Wrd2ScrY(hParser->GetMainPlane(), maxY) : 0);

                        RenderAreaRect(rect, bFill);

                        if (iSpringboardValue != 0) {
                            RenderArrow(posX - cammx, Wrd2ScrY(GetActivePlane(), SprBank->GetObjectRenderRect(obj).y1), posX - cammx, rect.y1, true);
                            GV->fntMyriad16->printf(posX - cammx, rect.y1 - 19, HGETEXT_CENTER, "~l~%s: %d", 0,
                                                    GETL2S("EditObj_SpringBoard", "OverlayValue"),
                                                    iSpringboardValue);
                            GV->fntMyriad16->printf(posX - cammx - 1, rect.y1 - 20, HGETEXT_CENTER, "~w~%s: ~y~%d",
                                                    0, GETL2S("EditObj_SpringBoard", "OverlayValue"),
                                                    iSpringboardValue);
                        }
                    }
                }

                if (!strcmp(obj->GetLogic(), "PathElevator")) {
                    bDrawObjProperties = false;
                    bool bEditMode = 0;
                    if (iActiveTool == EWW_TOOL_EDITOBJ && hEditObj->iType == ObjEdit::enPathElevator) bEditMode = 1;
                    int dir[8], dist[8];
                    if (obj->GetParam(WWD::Param_Powerup) == 1) {
                        dir[0] = 6;
                        dir[1] = 7;
                        dir[2] = 6;
                        dir[3] = 1;
                        dist[0] = 100;
                        dist[1] = 100;
                        dist[2] = 100;
                        dist[3] = 100;
                    } else if (obj->GetParam(WWD::Param_Powerup) == 2) {
                        dir[0] = 6;
                        dir[1] = 7;
                        dir[2] = 5;
                        dir[3] = 6;
                        dir[4] = 1;
                        dist[0] = 100;
                        dist[1] = 100;
                        dist[2] = 1000;
                        dist[3] = 100;
                        dist[4] = 100;
                    } else {
                        WWD::Rect tmprect = obj->GetMoveRect();
                        dir[0] = tmprect.x1;
                        dist[0] = tmprect.y1;
                        dir[1] = tmprect.x2;
                        dist[1] = tmprect.y2;
                        tmprect = obj->GetHitRect();
                        dir[2] = tmprect.x1;
                        dist[2] = tmprect.y1;
                        dir[3] = tmprect.x2;
                        dist[3] = tmprect.y2;
                        tmprect = obj->GetAttackRect();
                        dir[4] = tmprect.x1;
                        dist[4] = tmprect.y1;
                        dir[5] = tmprect.x2;
                        dist[5] = tmprect.y2;
                        tmprect = obj->GetClipRect();
                        dir[6] = tmprect.x1;
                        dist[6] = tmprect.y1;
                        dir[7] = tmprect.x2;
                        dist[7] = tmprect.y2;
                    }
                    int origx = Wrd2ScrX(GetActivePlane(), obj->GetX()),
                        origy = Wrd2ScrY(GetActivePlane(), obj->GetY());
                    int stepx = 0, stepy = 0;
                    for (int s = 0; s < 8; s++) {
                        if (dir[s] < 1 || dir[s] > 9) break;
                        if (dir[s] == 5) continue;
                        int modx = 0, mody = 0;

                        if (dir[s] >= 7) mody = -1;
                        else if (dir[s] >= 4) mody = 0;
                        else if (dir[s] >= 1) mody = 1;

                        if (dir[s] == 7 || dir[s] == 4 || dir[s] == 1) modx = -1;
                        else if (dir[s] == 8 || dir[s] == 2) modx = 0;
                        else if (dir[s] == 9 || dir[s] == 6 || dir[s] == 3) modx = 1;

                        int lastx = stepx, lasty = stepy;

                        int stepxvec = dist[s] * modx * fZoom, stepyvec = dist[s] * mody * fZoom;

                        stepx += stepxvec;
                        stepy += stepyvec;

                        if (bEditMode && (((ObjEdit::cEditObjElevPath *) hEditObj)->GetActualStep() == s ||
                                          ((ObjEdit::cEditObjElevPath *) hEditObj)->GetActualStep() == s + 1 ||
                                          (mx > origx + stepx - spr->GetWidth() / 2 &&
                                           mx < origx + stepx + spr->GetWidth() / 2 &&
                                           my > origy + stepy - spr->GetHeight() / 2 &&
                                           my < origy + stepy + spr->GetHeight() / 2)))
                            spr->SetColor(0xFFFFFFFF);
                        else
                            spr->SetColor(0x77FFFFFF);
                        spr->SetFlip(GetUserDataFromObj(obj)->GetFlipX(), GetUserDataFromObj(
                                             obj)->GetFlipY(),/*(list[i]->GetDrawFlags()&WWD::Flag_dr_Mirror), (list[i]->GetDrawFlags()&WWD::Flag_dr_Invert),*/
                                     1);
                        int spacesize = 12;
                        float arrowbodylen = sqrt(stepxvec * stepxvec + stepyvec * stepyvec);
                        arrowbodylen = arrowbodylen - spacesize * 2 - 37;
                        if (bEditMode && ((ObjEdit::cEditObjElevPath *) hEditObj)->GetActualStep() == s) {
                            GV->sprArrowVerticalM->SetColor(COL_ORANGE_ARROW);
                            GV->sprArrowVerticalU->SetColor(COL_ORANGE_ARROW);
                        } else {
                            GV->sprArrowVerticalM->SetColor(0xAAFFFFFF);
                            GV->sprArrowVerticalU->SetColor(0xAAFFFFFF);
                        }
                        RenderArrow(origx + lastx + spacesize * modx * fZoom, origy + lasty + spacesize * mody * fZoom,
                                    origx + lastx + spacesize * modx * fZoom +
                                    (stepxvec - ((fZoom * spacesize * (1 + (mody == 0)) * modx))),
                                    origy + lasty + spacesize * mody * fZoom +
                                    (stepyvec - ((fZoom * spacesize * (1 + (modx == 0)) * mody))), 0, 0);
                        if (arrowbodylen > 0) {
                            GV->sprArrowVerticalM->SetColor(0xFFFFFFFF);
                            GV->sprArrowVerticalU->SetColor(0xFFFFFFFF);
                            GV->fntMyriad16->SetColor(0xFF000000);
                            GV->fntMyriad16->printf(origx + lastx + stepxvec / 2, origy + lasty + stepyvec / 2 + 1 - 10,
                                                    HGETEXT_CENTER, "%s: %d, %d", 0, GETL2S("ObjProp", "Movement"),
                                                    dist[s] * modx, dist[s] * mody);
                            GV->fntMyriad16->SetColor(0xFFFFFFFF);
                            GV->fntMyriad16->printf(origx + lastx + stepxvec / 2, origy + lasty + stepyvec / 2 - 10,
                                                    HGETEXT_CENTER, "%s: ~y~%d~w~, ~y~%d", 0,
                                                    GETL2S("ObjProp", "Movement"), dist[s] * modx, dist[s] * mody);
                        }

                        if (!(stepx == 0 && stepy == 0)) {
                            spr->RenderEx(origx + stepx, origy + stepy, 0, fZoom);
                            GV->fntMyriad20->SetColor(0xFF000000);
                            GV->fntMyriad20->printf(origx + stepx + 2, origy + stepy + 2, HGETEXT_LEFT, "%d", 0, s + 1);
                            GV->fntMyriad20->SetColor(0xFFFFFFFF);
                            GV->fntMyriad20->printf(origx + stepx, origy + stepy, HGETEXT_LEFT, "%d", 0, s + 1);
                        }
                    }
                } else if (strstr(obj->GetLogic(), "Elevator") && strcmp(obj->GetLogic(), "SlidingElevator") != 0) {

                    if (strcmp(obj->GetLogic(), "Elevator") != 0) do { // Elevator_
                        int startX = obj->GetX(), startY = obj->GetY(),
                            dir = obj->GetParam(WWD::Param_Direction),
                            speedX = obj->GetParam(WWD::Param_SpeedX),
                            speedY = obj->GetParam(WWD::Param_SpeedY);

                        bool flip = false, broken = false,
                             returns = !strstr(obj->GetLogic(), "OneWay");

                        if (!speedX) speedX = 125;
                        else if (speedX < 60) {
                            broken = true;
                        }
                        if (dir == 8 || dir == 2) speedX = 0;

                        if (!speedY) speedY = 125;
                        else if (speedY < 60) {
                            broken = true;
                        }
                        if (dir == 6 || dir == 4) speedY = 0;

                        if (dir == 2 || dir == 3 || dir == 6) {
                            flip = returns && startX >= maxX && startY >= maxY;
                        } else {
                            flip = !(returns && startX <= minX && startY <= minY);
                        }

                        startX = Wrd2ScrX(GetActivePlane(), startX);
                        startY = Wrd2ScrY(GetActivePlane(), startY);

                        int x = startX, y = startY,
                            targetX = Wrd2ScrX(GetActivePlane(), flip ? minX : maxX),
                            targetY = Wrd2ScrY(GetActivePlane(), flip ? minY : maxY),
                            color = broken ? 0x77FF7777 : 0x77FFFFFF;

                        spr->SetColor(color);
                        GV->sprArrowVerticalM->SetColor(color);
                        GV->sprArrowVerticalU->SetColor(color);
                        GV->sprArrowVerticalD->SetColor(color);

                        if (!speedY) {
                            if (!speedX || flip ? x < targetX : x > targetX) break;
                            int nextTargetX = Wrd2ScrX(GetActivePlane(), !flip ? minX : maxX);
                            int nextTargetY = Wrd2ScrY(GetActivePlane(), !flip ? minY : maxY);
                            if (y != targetY || y != nextTargetY) {
                                color = 0x77FF7777;
                                spr->SetColor(color);
                                GV->sprArrowVerticalM->SetColor(color);
                                GV->sprArrowVerticalU->SetColor(color);
                                GV->sprArrowVerticalD->SetColor(color);
                                returns = y == targetY;
                            }
                            else if (x != nextTargetX) {
                                returns = false;
                            }

                            if (x != targetX) {
                                spr->RenderEx(targetX, y, 0, fZoom);
                                RenderArrow(x, y, targetX, y, true, false, returns);
                            }

                            if (broken || (!flip ? y < targetY : y > targetY)) break;

                            if (x != nextTargetX) {
                                spr->RenderEx(nextTargetX, y, 0, fZoom);
                                RenderArrow(targetX, y, nextTargetX, y, true, false);
                            }
                        } else if (!speedX) {
                            if (flip ? y < targetY : y > targetY) break;
                            int nextTargetX = Wrd2ScrX(GetActivePlane(), !flip ? minX : maxX);
                            int nextTargetY = Wrd2ScrY(GetActivePlane(), !flip ? minY : maxY);
                            if (x != targetX || x != nextTargetX) {
                                color = 0x77FF7777;
                                spr->SetColor(color);
                                GV->sprArrowVerticalM->SetColor(color);
                                GV->sprArrowVerticalU->SetColor(color);
                                GV->sprArrowVerticalD->SetColor(color);
                                returns = x == targetX;
                            } else if (y != nextTargetY) {
                                returns = false;
                            }

                            if (y != targetY) {
                                spr->RenderEx(x, targetY, 0, fZoom);
                                RenderArrow(x, y, x, targetY, true, false, returns);
                            }

                            if (broken || (!flip ? x < targetX : x > targetX)) break;

                            if (y != nextTargetY) {
                                spr->RenderEx(x, nextTargetY, 0, fZoom);
                                RenderArrow(x, targetY, x, nextTargetY, true, false);
                            }
                        } else {
                            if (speedX > 9999) speedX = 9999;
                            if (speedY > 9999) speedY = 9999;

                            int travelX = targetX - x,
                                travelY = targetY - y;

                            #define SHOW_PATH() \
                                if (flip) { \
                                    if (x < targetX) travelX = 0; \
                                    if (y < targetY) travelY = 0; \
                                } else { \
                                    if (x > targetX) travelX = 0; \
                                    if (y > targetY) travelY = 0; \
                                } \
                                if (!travelX && !travelY) break; \
                                if (travelX && travelY) { \
                                    if (abs(travelX) * 1000 / speedX > abs(travelY) * 1000 / speedY) { \
                                        travelX = (travelX < 0 ^ travelY < 0 ? -travelY : travelY) * speedX / speedY; \
                                    } else { \
                                        travelY = (travelX < 0 ^ travelY < 0 ? -travelX : travelX) * speedY / speedX; \
                                    } \
                                } \
                                x += travelX; \
                                y += travelY; \
                                spr->RenderEx(x, y, 0, fZoom); \
                                if ((x != targetX && travelX != 0) || (y != targetY && travelY != 0)) { \
                                    RenderArrow(x - travelX, y - travelY, x, y, true, false); \
                                    if (abs(targetX - x) > 1 || abs(targetY - y) > 1) { \
                                        spr->RenderEx(targetX, targetY, 0, fZoom); \
                                        RenderArrow(x, y, targetX, targetY, true, false); \
                                        x = targetX; \
                                        y = targetY; \
                                    } \
                                } else { \
                                    int nTX = Wrd2ScrX(GetActivePlane(), !flip ? minX : maxX), \
                                        nTY = Wrd2ScrY(GetActivePlane(), !flip ? minY : maxY); \
                                    if (nTX == x - travelX && nTY == y - travelY) { \
                                        RenderArrow(nTX, nTY, x, y, true, false, true); \
                                        break; \
                                    } else { \
                                        RenderArrow(x - travelX, y - travelY, x, y, true, false); \
                                    } \
                                }

                            #define OTHER_WAY() \
                                flip = !flip; \
                                targetX = Wrd2ScrX(GetActivePlane(), flip ? minX : maxX); \
                                targetY = Wrd2ScrY(GetActivePlane(), flip ? minY : maxY); \
                                travelX = targetX - x; \
                                travelY = targetY - y; \
                                SHOW_PATH();

                            /*if (speedX == speedY && maxX - minX == maxY - minY) {
                                int secondTargetX = Wrd2ScrX(GetActivePlane(), !flip ? minX : maxX),
                                    secondTargetY = Wrd2ScrY(GetActivePlane(), !flip ? minY : maxY);

                                RenderArrow(targetX, targetY, secondTargetX, secondTargetY, true);
                                spr->RenderEx(targetX, targetY, 0, fZoom);

                                if (x != secondTargetX || y != secondTargetY) {
                                    RenderArrow(x, y, targetX, targetY, true);
                                    spr->RenderEx(secondTargetX, secondTargetY, 0, fZoom);
                                }
                            } else {*/
                                SHOW_PATH();
                                if (broken) break;

                                int secondStX = x, secondStY = y;

                                if (returns) {
                                    OTHER_WAY();

                                    if (x != startX || y != startY) {
                                        OTHER_WAY();

                                        if ((x != startX || y != startY) && (x != secondStX || y != secondStY)) {
                                            OTHER_WAY();
                                        }
                                    }
                                }
                            //}
                        }
                    } while (false); // trick to allow breaking
                    else if ((minX != 0 && maxX != 0 && minX <= maxX) || (minY != 0 && maxY != 0 && minY <= maxY)) {
                        if (minX == maxX) { // vertical
                            int x = Wrd2ScrX(GetActivePlane(), obj->GetX()),
                                startY = Wrd2ScrY(GetActivePlane(), minY),
                                endY = Wrd2ScrY(GetActivePlane(), maxY);
                            spr->SetColor(0x77FFFFFF);
                            if (minY != obj->GetY())
                                spr->RenderEx(x, startY, 0, fZoom);
                            if (maxY != obj->GetY())
                                spr->RenderEx(x, endY, 0, fZoom);
                            RenderArrow(x, startY, x, endY, true, true, true);
                        } else if (minY == maxY) { // horizontal
                            int y = Wrd2ScrY(GetActivePlane(), obj->GetY()),
                                startX = Wrd2ScrX(GetActivePlane(), minX),
                                endX = Wrd2ScrX(GetActivePlane(), maxX);
                            spr->SetColor(0x77FFFFFF);
                            if (minX != obj->GetX())
                                spr->RenderEx(startX, y, 0, fZoom);
                            if (maxX != obj->GetX())
                                spr->RenderEx(endX, y, 0, fZoom);
                            RenderArrow(startX, y, endX, y, true, true, true);
                        } else if ((maxX - minX == maxY - minY) // diagonal
                        && (((obj->GetX() <= minX || obj->GetX() >= maxX)
                        && (obj->GetY() <= minY || obj->GetY() >= maxY))
                        || obj->GetX() - minX == obj->GetY() - minY)
                        && obj->GetParam(WWD::Param_SpeedX) == obj->GetParam(WWD::Param_SpeedY)) {
                            int x = obj->GetX(), y = obj->GetY();

                            if (x < minX) x = minX;
                            else if (x > maxX) x = maxX;

                            if (y < minY) y = minY;
                            else if (y > maxY) y = maxY;

                            int dir = obj->GetParam(WWD::Param_Direction);
                            if (dir == 7 || dir == 4 || dir == 1) {
                                if (x == minX) x = maxX;
                                else x = minX;
                            }
                            if (dir == 7 || dir == 8 || dir == 9) {
                                if (y == minY) y = maxY;
                                else y = minY;
                            }

                            int startX = Wrd2ScrX(GetActivePlane(), x), startY = Wrd2ScrY(GetActivePlane(), y);

                            spr->SetColor(0x77FFFFFF);
                            spr->RenderEx(startX, startY, 0, fZoom);

                            #define FLIP() \
                            if (dir != 2 && dir != 8) { \
                                if (x == minX) x = maxX; \
                                else x = minX; \
                            } \
                            if (dir != 4 && dir != 6) { \
                                if (y == minY) y = maxY; \
                                else y = minY; \
                            }
                            FLIP();

                            int endX = Wrd2ScrX(GetActivePlane(), x), endY = Wrd2ScrY(GetActivePlane(), y);
                            spr->RenderEx(endX, endY, 0, fZoom);

                            FLIP();
                            int nextX = Wrd2ScrX(GetActivePlane(), x), nextY = Wrd2ScrY(GetActivePlane(), y);
                            bool onceAgain = (nextX != startX || nextY != startY);

                            RenderArrow(startX, startY, endX, endY, true, true, !onceAgain);

                            if (onceAgain) {
                                RenderArrow(endX, endY, Wrd2ScrX(GetActivePlane(), x), Wrd2ScrY(GetActivePlane(), y), true, true, true);
                            }
                        } else do { // other
                            bool closed = false;
                            std::vector<POINT> travelPoints;

                            int x = obj->GetX(), y = obj->GetY();
                            int width = maxX - minX;
                            int height = maxY - minY;

                            if (width <= 0 && height <= 0) break;

                            int cycleX = 2 * width;
                            int cycleY = 2 * height;

                            int tweakX = std::min(std::max(x - minX, 0), width),
                                tweakY = std::min(std::max(y - minY, 0), height);

                            int dir = obj->GetParam(WWD::Param_Direction);
                            if (dir == 7 || dir == 4 || dir == 1) {
                                tweakX += width;
                            }
                            if (dir == 7 || dir == 8 || dir == 9) {
                                tweakY += height;
                            }

                            std::function<void()> updateXY = [&](){
                                if (cycleX) {
                                    div_t diffX = div(tweakX, cycleX);
                                    diffX.quot = width;
                                    if (diffX.rem <= diffX.quot) {
                                        x = minX + diffX.rem;
                                    } else {
                                        x = maxX + diffX.quot - diffX.rem;
                                    }
                                }
                                if (cycleY) {
                                    div_t diffY = div(tweakY, cycleY);
                                    diffY.quot = height;
                                    if (diffY.rem <= diffY.quot) {
                                        y = minY + diffY.rem;
                                    } else {
                                        y = maxY + diffY.quot - diffY.rem;
                                    }
                                }
                            };

                            updateXY();

                            int scrStartX = Wrd2ScrX(GetActivePlane(), x), scrStartY = Wrd2ScrY(GetActivePlane(), y);
                            travelPoints.emplace_back(POINT{scrStartX, scrStartY});

                            int speedX = obj->GetParam(WWD::Param_SpeedX),
                                speedY = obj->GetParam(WWD::Param_SpeedY);

                            if (width <= 0) speedX = 0;
                            else if (!speedX) speedX = DEFAULT_ELEVATOR_SPEED;
                            if (height <= 0) speedY = 0;
                            else if (!speedY) speedY = DEFAULT_ELEVATOR_SPEED;

                            if (speedX <=0 && speedY <= 0) break;

                            for (int i = 1; i < 32; i++) {
                                if (speedX <= 0) {
                                    tweakY += height - tweakY % height;
                                } else if (speedY <= 0) {
                                    tweakX += width - tweakX % width;
                                } else {
                                    int distX = width - tweakX % width;
                                    int distY = height - tweakY % height;

                                    if (distX * 1000 / speedX < distY * 1000 / speedY) {
                                        tweakX += distX;
                                        tweakY += distX * speedY / speedX;
                                    } else {
                                        tweakX += distY * speedX / speedY;
                                        tweakY += distY;
                                    }
                                }

                                updateXY();
                                int scrX = Wrd2ScrX(GetActivePlane(), x), scrY = Wrd2ScrY(GetActivePlane(), y);
                                /*for (const auto& p : travelPoints) {
                                    if (p.x == scrX && p.y == scrY) {
                                        closed = true;
                                        i = 666;
                                        break;
                                    }
                                }*/
                                travelPoints.emplace_back(POINT{scrX, scrY});
                                if (scrX == travelPoints[0].x && scrY == travelPoints[0].y) break;
                            }

                            const POINT* pp = NULL;
                            spr->SetColor(0x77FFFFFF);
                            for (const auto& p : travelPoints) {
                                spr->RenderEx(p.x, p.y, 0, fZoom);

                                if (pp) {
                                    float sizeDiff = i * 0.1f * fZoom;
                                    fZoom -= sizeDiff;
                                    RenderArrow(pp->x, pp->y, p.x, p.y, true);
                                    fZoom += sizeDiff;
                                }
                                pp = &p;
                            }
                        } while (false); // trick to allow breaking
                    }
                } else if (logicInfo.IsBoss()) {
                    if (iActiveTool != EWW_TOOL_EDITOBJ || !((ObjEdit::cEditObjEnemy *) hEditObj)->isPickingXY() ) {
                        int outX = obj->GetParam(WWD::Param_SpeedX),
                            outY = obj->GetParam(WWD::Param_SpeedY);

                        if (outX && outY) {
                            int wOutX = Wrd2ScrX(GetActivePlane(), outX),
                                wOutY = Wrd2ScrY(GetActivePlane(), outY);
                            hgeSprite *spr2 = SprBank->GetAssetByID("LEVEL_GEM")->GetIMGByIterator(0)->GetSprite();
                            spr2->SetColor(0xBBFFFFFF);
                            spr2->RenderEx(wOutX, wOutY, 0, fZoom);

                            wOutX += (spr2->GetWidth() / 2 + 15) * fZoom;
                            GV->fntMyriad16->printf(wOutX + 1, wOutY + 1, HGETEXT_LEFT, "~l~%s %d, %d", 0,
                                                    GETL2S("EditObj_Enemy", "GemDest"), outX, outY);
                            GV->fntMyriad16->printf(wOutX, wOutY, HGETEXT_LEFT, "~w~%s ~y~%d~w~, ~y~%d", 0,
                                                    GETL2S("EditObj_Enemy", "GemDest"), outX, outY);
                        }
                    }
                } else if (!strcmp(obj->GetLogic(), "SpecialPowerup") &&
                           (!strcmp(obj->GetImageSet(), "GAME_WARP") || !strcmp(obj->GetImageSet(), "GAME_VERTWARP")) ||
                           !strcmp(obj->GetLogic(), "BossWarp") && !strcmp(obj->GetImageSet(), "GAME_BOSSWARP") ||
                           iActiveTool == EWW_TOOL_EDITOBJ &&
                           (hEditObj->iType == ObjEdit::enWarp && ((ObjEdit::cEditObjWarp *) hEditObj)->bPick ||
                            hEditObj->iType == ObjEdit::enCrate &&
                            ((ObjEdit::cEditObjCrate *) hEditObj)->PreviewWarp() ||
                            hEditObj->iType == ObjEdit::enStatue &&
                            ((ObjEdit::cEditObjStatue *) hEditObj)->PreviewWarp())) {
                    int outX, outY;
                    bool bdr = 0;
                    if (iActiveTool == EWW_TOOL_EDITOBJ && conMain->getWidgetAt(mx, my) == vPort->GetWidget() &&
                        (hEditObj->iType == ObjEdit::enWarp && ((ObjEdit::cEditObjWarp *) hEditObj)->bPick ||
                         hEditObj->iType == ObjEdit::enCrate && ((ObjEdit::cEditObjCrate *) hEditObj)->Picking() ||
                         hEditObj->iType == ObjEdit::enStatue && ((ObjEdit::cEditObjStatue *) hEditObj)->Picking())) {
                        outX = mx;
                        outY = my;
                        bdr = 1;
                    } else {
                        outX = Wrd2ScrX(GetActivePlane(), obj->GetParam(WWD::Param_SpeedX));
                        outY = Wrd2ScrY(GetActivePlane(), obj->GetParam(WWD::Param_SpeedY));
                    }
                    hgeSprite *spr2 = SprBank->GetAssetByID("CLAW")->GetIMGByID(401)->GetSprite();
                    spr2->SetColor(0xBBFFFFFF);
                    spr2->RenderEx(outX, outY, 0, fZoom);
                    if (iActiveTool != EWW_TOOL_EDITOBJ || bdr) {
                        int drx, dry;
                        if (bdr) {
                            drx = Scr2WrdX(GetActivePlane(), outX);
                            dry = Scr2WrdY(GetActivePlane(), outY);
                        } else {
                            drx = obj->GetParam(WWD::Param_SpeedX);
                            dry = obj->GetParam(WWD::Param_SpeedY);
                        }
                        GV->fntMyriad16->printf(outX + spr2->GetWidth() / 2 + 1, outY + 1, HGETEXT_LEFT,
                                                "~l~%s: %d, %d", 0, GETL2S("EditObj_Warp", "SpawnPos"), drx, dry);
                        GV->fntMyriad16->printf(outX + spr2->GetWidth() / 2, outY, HGETEXT_LEFT,
                                                "~w~%s: ~y~%d~w~, ~y~%d", 0, GETL2S("EditObj_Warp", "SpawnPos"), drx,
                                                dry);
                    }
                }
                if (iActiveTool == EWW_TOOL_EDITOBJ &&
                    (hEditObj->iType == ObjEdit::enPathElevator || hEditObj->IsMovingObject() ||
                     hEditObj->iType == ObjEdit::enText)
                    || iActiveTool == EWW_TOOL_MOVEOBJECT)
                    bDrawObjProperties = false;

                if (obj == hStartingPosObj) {
                    GV->fntMyriad16->SetColor(0xFF000000);
                    GV->fntMyriad16->printf(posX - cammx + sprW + 5, posY + vPort->GetY() + 1 - cammy - sprH,
                                            HGETEXT_LEFT, "%s", 0, GETL(Lang_StartingPlace));
                    GV->fntMyriad16->SetColor(0xFFFFFFFF);
                    GV->fntMyriad16->printf(posX - cammx + sprW + 4, posY + vPort->GetY() - cammy - sprH, HGETEXT_LEFT,
                                            "%s", 0, GETL(Lang_StartingPlace));

                    GV->fntMyriad16->SetColor(0xFF000000);
                    GV->fntMyriad16->printf(posX - cammx + sprW + 5, posY + vPort->GetY() + 1 - cammy - sprH + 15,
                                            HGETEXT_LEFT, "X: %d Y: %d", 0, obj->GetParam(WWD::Param_LocationX),
                                            obj->GetParam(WWD::Param_LocationY));
                    GV->fntMyriad16->SetColor(0xFFFFFFFF);
                    GV->fntMyriad16->printf(posX - cammx + sprW + 4, posY + vPort->GetY() - cammy - sprH + 15,
                                            HGETEXT_LEFT, "X: ~y~%d ~w~Y: ~y~%d", 0,
                                            obj->GetParam(WWD::Param_LocationX), obj->GetParam(WWD::Param_LocationY));
                } else if (bDrawObjProperties) {
                    int ioff = 15;
                    int iid = obj->GetParam(WWD::Param_ID);
                    if (iActiveTool == EWW_TOOL_EDITOBJ) {
                        iid = hEditObj->GetOrigObj()->GetParam(WWD::Param_ID);
                    }
                    GV->fntMyriad16->printf(posX - cammx + sprW + 5, posY + vPort->GetY() + 1 - cammy - sprH,
                                            HGETEXT_LEFT, "~l~%s: %d", 0, GETL(Lang_ID), iid);
                    GV->fntMyriad16->printf(posX - cammx + sprW + 4, posY + vPort->GetY() - cammy - sprH, HGETEXT_LEFT,
                                            "~w~%s: ~y~%d", 0, GETL(Lang_ID), iid);
                    if (obj->GetName()[0] != '\0' && strcmp(obj->GetLogic(), "CustomLogic") != 0) {
                        GV->fntMyriad16->printf(posX - cammx + sprW + 5, posY + vPort->GetY() + 1 - cammy + ioff - sprH,
                                                HGETEXT_LEFT, "~l~%s %s", 0, GETL(Lang_Name), obj->GetName());
                        GV->fntMyriad16->printf(posX - cammx + sprW + 4, posY + vPort->GetY() - cammy + ioff - sprH,
                                                HGETEXT_LEFT, "~w~%s ~y~%s", 0, GETL(Lang_Name), obj->GetName());
                        ioff += 15;
                    }

                    if (obj->GetLogic()[0] != '\0') {
                        bool customlogic = !strcmp(obj->GetLogic(), "CustomLogic");
                        const char *logicval = (customlogic ? obj->GetName()
                                                            : obj->GetLogic());
                        if (customlogic) {
                            GV->sprIcons16[Icon16_CrazyHook]->SetColor(0xFFFFFFFF);
                            GV->sprIcons16[Icon16_CrazyHook]->Render(
                                    posX - cammx + sprW + 7 + GV->fntMyriad16->GetStringWidth(GETL(Lang_Logic)),
                                    posY + vPort->GetY() - cammy + ioff - sprH);
                        }
                        GV->fntMyriad16->printf(posX - cammx + sprW + 5, posY + vPort->GetY() + 1 - cammy + ioff - sprH,
                                                HGETEXT_LEFT, (customlogic ? "~l~%s:    %s" : "~l~%s: %s"), 0,
                                                GETL(Lang_Logic), logicval);
                        GV->fntMyriad16->printf(posX - cammx + sprW + 4, posY + vPort->GetY() - cammy + ioff - sprH,
                                                HGETEXT_LEFT, (customlogic ? "~w~%s:    ~y~%s" : "~w~%s: ~y~%s"), 0,
                                                GETL(Lang_Logic), logicval);
                        ioff += 15;
                    }

                    if (obj->GetImageSet()[0] != '\0') {
                        GV->fntMyriad16->printf(posX - cammx + sprW + 5, posY + vPort->GetY() + 1 - cammy + ioff - sprH,
                                                HGETEXT_LEFT, "~l~%s: %s", 0, GETL(Lang_Graphic), obj->GetImageSet());
                        GV->fntMyriad16->printf(posX - cammx + sprW + 4, posY + vPort->GetY() - cammy + ioff - sprH,
                                                HGETEXT_LEFT, "~w~%s: ~y~%s", 0, GETL(Lang_Graphic),
                                                obj->GetImageSet());
                        ioff += 15;
                    }

                    if (obj->GetAnim()[0] != '\0') {
                        GV->fntMyriad16->printf(posX - cammx + sprW + 5, posY + vPort->GetY() + 1 - cammy + ioff - sprH,
                                                HGETEXT_LEFT, "~l~%s: %s", 0, GETL(Lang_Anim), obj->GetAnim());
                        GV->fntMyriad16->printf(posX - cammx + sprW + 4, posY + vPort->GetY() - cammy + ioff - sprH,
                                                HGETEXT_LEFT, "~w~%s: ~y~%s", 0, GETL(Lang_Anim), obj->GetAnim());
                        ioff += 15;
                    }

                    GV->fntMyriad16->printf(posX - cammx + sprW + 5, posY + vPort->GetY() + 1 - cammy + ioff - sprH,
                                            HGETEXT_LEFT, "~l~%s: %dx%d, Z: %d", 0,
                                            GETL(Lang_Location), obj->GetParam(WWD::Param_LocationX),
                                            obj->GetParam(WWD::Param_LocationY),
                                            obj->GetParam(WWD::Param_LocationZ));
                    GV->fntMyriad16->printf(posX - cammx + sprW + 4, posY + vPort->GetY() - cammy + ioff - sprH,
                                            HGETEXT_LEFT, "~w~%s: ~y~%d~w~x~y~%d~w~, Z: ~y~%d", 0,
                                            GETL(Lang_Location), obj->GetParam(WWD::Param_LocationX),
                                            obj->GetParam(WWD::Param_LocationY),
                                            obj->GetParam(WWD::Param_LocationZ));
                    ioff += 15;
                    if (obj->GetParam(WWD::Param_MinX) != 0 || obj->GetParam(WWD::Param_MaxX) != 0) {
                        if (obj->GetParam(WWD::Param_MinX) != 0 && obj->GetParam(WWD::Param_MaxX) != 0) {
                            GV->fntMyriad16->printf(posX - cammx + sprW + 5,
                                                    posY + vPort->GetY() + 1 - cammy + ioff - sprH, HGETEXT_LEFT,
                                                    "~l~%s: %d. %s: %d.", 0,
                                                    GETL(Lang_MinX), obj->GetParam(WWD::Param_MinX), GETL(Lang_MaxX),
                                                    obj->GetParam(WWD::Param_MaxX));
                            GV->fntMyriad16->printf(posX - cammx + sprW + 4, posY + vPort->GetY() - cammy + ioff - sprH,
                                                    HGETEXT_LEFT, "~w~%s: ~y~%d~w~. %s: ~y~%d~w~.", 0,
                                                    GETL(Lang_MinX), obj->GetParam(WWD::Param_MinX), GETL(Lang_MaxX),
                                                    obj->GetParam(WWD::Param_MaxX));
                        } else if (obj->GetParam(WWD::Param_MinX) != 0 && obj->GetParam(WWD::Param_MaxX) == 0) {
                            GV->fntMyriad16->printf(posX - cammx + sprW + 5,
                                                    posY + vPort->GetY() + 1 - cammy + ioff - sprH, HGETEXT_LEFT,
                                                    "~l~%s: %d", 0,
                                                    GETL(Lang_MinX), obj->GetParam(WWD::Param_MinX));
                            GV->fntMyriad16->printf(posX - cammx + sprW + 4, posY + vPort->GetY() - cammy + ioff - sprH,
                                                    HGETEXT_LEFT, "~w~%s: ~y~%d", 0,
                                                    GETL(Lang_MinX), obj->GetParam(WWD::Param_MinX));
                        } else if (obj->GetParam(WWD::Param_MinX) == 0 && obj->GetParam(WWD::Param_MaxX) != 0) {
                            GV->fntMyriad16->printf(posX - cammx + sprW + 5,
                                                    posY + vPort->GetY() + 1 - cammy + ioff - sprH, HGETEXT_LEFT,
                                                    "~l~%s: %d", 0,
                                                    GETL(Lang_MaxX), obj->GetParam(WWD::Param_MaxX));
                            GV->fntMyriad16->printf(posX - cammx + sprW + 4, posY + vPort->GetY() - cammy + ioff - sprH,
                                                    HGETEXT_LEFT, "~w~%s: ~y~%d", 0,
                                                    GETL(Lang_MaxX), obj->GetParam(WWD::Param_MaxX));
                        }
                        ioff += 15;
                    }
                    if (obj->GetParam(WWD::Param_MinY) != 0 || obj->GetParam(WWD::Param_MaxY) != 0) {
                        if (obj->GetParam(WWD::Param_MinY) != 0 && obj->GetParam(WWD::Param_MaxY) != 0) {
                            GV->fntMyriad16->printf(posX - cammx + sprW + 5,
                                                    posY + vPort->GetY() + 1 - cammy + ioff - sprH, HGETEXT_LEFT,
                                                    "~l~%s: %d. %s: %d.", 0,
                                                    GETL(Lang_MinY), obj->GetParam(WWD::Param_MinY), GETL(Lang_MaxY),
                                                    obj->GetParam(WWD::Param_MaxY));
                            GV->fntMyriad16->printf(posX - cammx + sprW + 4, posY + vPort->GetY() - cammy + ioff - sprH,
                                                    HGETEXT_LEFT, "~w~%s: ~y~%d~w~. %s: ~y~%d~w~.", 0,
                                                    GETL(Lang_MinY), obj->GetParam(WWD::Param_MinY), GETL(Lang_MaxY),
                                                    obj->GetParam(WWD::Param_MaxY));
                        } else if (obj->GetParam(WWD::Param_MinY) != 0 && obj->GetParam(WWD::Param_MaxY) == 0) {
                            GV->fntMyriad16->printf(posX - cammx + sprW + 5,
                                                    posY + vPort->GetY() + 1 - cammy + ioff - sprH, HGETEXT_LEFT,
                                                    "~l~%s: %d", 0,
                                                    GETL(Lang_MinY), obj->GetParam(WWD::Param_MinY));
                            GV->fntMyriad16->printf(posX - cammx + sprW + 4, posY + vPort->GetY() - cammy + ioff - sprH,
                                                    HGETEXT_LEFT, "~w~%s: ~y~%d", 0,
                                                    GETL(Lang_MinY), obj->GetParam(WWD::Param_MinY));
                        } else if (obj->GetParam(WWD::Param_MinY) == 0 && obj->GetParam(WWD::Param_MaxY) != 0) {
                            GV->fntMyriad16->printf(posX - cammx + sprW + 5,
                                                    posY + vPort->GetY() + 1 - cammy + ioff - sprH, HGETEXT_LEFT,
                                                    "~l~%s: %d", 0,
                                                    GETL(Lang_MaxY), obj->GetParam(WWD::Param_MaxY));
                            GV->fntMyriad16->printf(posX - cammx + sprW + 4, posY + vPort->GetY() - cammy + ioff - sprH,
                                                    HGETEXT_LEFT, "~w~%s: ~y~%d", 0,
                                                    GETL(Lang_MaxY), obj->GetParam(WWD::Param_MaxY));
                        }
                        ioff += 15;
                    }

                    if (logicInfo.CanDropTreasure()) {
                        bool crabNest = !strcmp(obj->GetLogic(), "CrabNest");
                        int crabsNum = crabNest ? (obj->GetUserValue(0) ? obj->GetUserValue(0) : 3) : 0;
                        bool singlecrate = logicInfo.IsSingleCrate();
                        cInventoryItem items[10];
                        for (int i = 0; i < 10; i++)
                            items[i] = cInventoryItem("", -1);
                        int itemoff = 0;
                        if (logicInfo.IsBoss()) {
                            items[0] = hInvCtrl->GetItemByID(31); //end of level powerup
                            itemoff++;
                        }
                        if (obj->GetParam(WWD::Param_Powerup) == 0 && singlecrate) {
                            items[itemoff] = hInvCtrl->GetItemByID(33);
                            itemoff++;
                        } else if (obj->GetParam(WWD::Param_Powerup)) {
                            items[itemoff] = hInvCtrl->GetItemByID(obj->GetParam(WWD::Param_Powerup));
                            if (items[itemoff].second != -1) itemoff++;
                        } else if (crabNest) {
                            singlecrate = true;
                        }
                        if (!singlecrate && strcmp(obj->GetLogic(), "HermitCrab") != 0) {
                            for (int z = 0; z < 2; z++) {
                                WWD::Rect r = obj->GetUserRect(z);
                                if (r.x1 != 0) {
                                    items[itemoff] = hInvCtrl->GetItemByID(r.x1);
                                    if (items[itemoff].second != -1) itemoff++;
                                    else if (crabNest) break;
                                } else if (crabNest) break;
                                if (r.y1 != 0) {
                                    items[itemoff] = hInvCtrl->GetItemByID(r.y1);
                                    if (items[itemoff].second != -1) itemoff++;
                                    else if (crabNest) break;
                                } else if (crabNest) break;
                                if (r.x2 != 0) {
                                    items[itemoff] = hInvCtrl->GetItemByID(r.x2);
                                    if (items[itemoff].second != -1) itemoff++;
                                    else if (crabNest) break;
                                } else if (crabNest) break;
                                if (r.y2 != 0) {
                                    items[itemoff] = hInvCtrl->GetItemByID(r.y2);
                                    if (items[itemoff].second != -1) itemoff++;
                                    else if (crabNest) break;
                                } else if (crabNest) break;
                            }
                        }
                        bool containsWarp = false;
                        for (int y = 0; y < 4; y++)
                            for (int x = 0; x < 3; x++) {
                                int i = y * 3 + x;
                                if (i < crabsNum) {
                                    hgeSprite *crabSpr = GV->editState->SprBank->GetAssetByID("LEVEL_HERMITCRAB")->GetIMGByIterator(0)->GetSprite();
                                    crabSpr->SetColor(0xBBFFFFFF);
                                    crabSpr->SetFlip(0, 0, true);
                                    int grdim = crabSpr->GetWidth();
                                    if (crabSpr->GetHeight() > grdim) grdim = crabSpr->GetHeight();
                                    float fScale = 1.0f;
                                    if (grdim > 48) fScale = 48.0f / float(grdim);
                                    crabSpr->RenderEx(posX - cammx + sprW + 4 + 52 * x + 24,
                                                      posY + vPort->GetY() - cammy + ioff - sprH + y * 54 + 24, 0, fScale);
                                }
                                if (i > 9 || GetInventoryItemID(items[i]) == -1) {
                                    if (i < crabsNum) continue;
                                    break;
                                }
                                if (GetInventoryItemID(items[i]) == 32) {
                                    containsWarp = true;
                                }
                                cSprBankAsset *asset = GV->editState->SprBank->GetAssetByID(
                                        GetInventoryItemImageSet(items[i]));
                                int iframe = GV->editState->hInvCtrl->GetAnimFrame() % asset->GetSpritesCount();
                                hgeSprite *treasureSpr = asset->GetIMGByIterator(iframe)->GetSprite();
                                treasureSpr->SetColor(0xBBFFFFFF);
                                treasureSpr->SetFlip(0, 0, true);
                                int grdim = treasureSpr->GetWidth();
                                if (treasureSpr->GetHeight() > grdim) grdim = treasureSpr->GetHeight();
                                float fScale = 1.0f;
                                if (grdim > 48) fScale = 48.0f / float(grdim);
                                treasureSpr->RenderEx(posX - cammx + sprW + 4 + 52 * x + 24,
                                                      posY + vPort->GetY() - cammy + ioff - sprH + y * 54 + 24, 0, fScale);
                            }

                        if (containsWarp && strcmp(obj->GetLogic(), "CrabNest") != 0) {
                            if (iActiveTool != EWW_TOOL_EDITOBJ || hEditObj->iType != ObjEdit::enEnemy
                                || !((ObjEdit::cEditObjEnemy *) hEditObj)->isPickingXY() ) {
                                int outX = obj->GetParam(WWD::Param_SpeedX),
                                    outY = obj->GetParam(WWD::Param_SpeedY);

                                if (outX && outY) {
                                    int wOutX = Wrd2ScrX(GetActivePlane(), outX),
                                        wOutY = Wrd2ScrY(GetActivePlane(), outY);
                                    hgeSprite *spr2 = GV->editState->SprBank->GetAssetByID("CLAW")->GetIMGByID(401)->GetSprite();
                                    spr2->SetColor(0xBBFFFFFF);
                                    spr2->RenderEx(wOutX, wOutY, 0, fZoom);

                                    if (logicInfo.IsBoss()) { // make sure gem is on top and move second text
                                        spr2 = SprBank->GetAssetByID("LEVEL_GEM")->GetIMGByIterator(0)->GetSprite();
                                        spr2->SetColor(0xBBFFFFFF);
                                        spr2->RenderEx(wOutX, wOutY, 0, fZoom);

                                        wOutY += 15;
                                    }

                                    wOutX += (spr2->GetWidth() / 2 + 15) * fZoom;

                                    GV->fntMyriad16->printf(wOutX + 1, wOutY + 1, HGETEXT_LEFT, "~l~%s %d, %d", 0,
                                                            GETL2S("EditObj_Enemy", "WarpDest"), outX, outY);
                                    GV->fntMyriad16->printf(wOutX, wOutY, HGETEXT_LEFT, "~w~%s ~y~%d~w~, ~y~%d", 0,
                                                            GETL2S("EditObj_Enemy", "WarpDest"), outX, outY);
                                }
                            }
                        }
                    }

                }
                GV->fntMyriad16->SetColor(0xFF000000);
            }

            WWD::Rect box = SprBank->GetObjectRenderRect(obj);
            box.x2 += box.x1;
            box.y2 += box.y1;
            box.x1 = Wrd2ScrX(GetActivePlane(), box.x1);
            box.y1 = Wrd2ScrY(GetActivePlane(), box.y1);
            box.x2 = Wrd2ScrX(GetActivePlane(), box.x2);
            box.y2 = Wrd2ScrY(GetActivePlane(), box.y2);
            DWORD col = (iActiveTool == EWW_TOOL_EDITOBJ ? hEditObj->GetHighlightColor() : GV->colActive);
            hge->Gfx_RenderLine(box.x1 - 1, box.y1, box.x2, box.y1, col);
            hge->Gfx_RenderLine(box.x2, box.y1, box.x2, box.y2, col);
            hge->Gfx_RenderLine(box.x2, box.y2, box.x1, box.y2, col);
            hge->Gfx_RenderLine(box.x1, box.y2, box.x1, box.y1, col);

            if (iActiveTool == EWW_TOOL_EDITOBJ) {
                hEditObj->RenderObjectOverlay();
            } else {
                int x = Wrd2ScrX(GetActivePlane(), obj->GetX()), y = Wrd2ScrY(GetActivePlane(), obj->GetY());
                SHR::SetQuad(&q, GV->colActive, x - 2, y - 3, x + 2, y + 3);
                hge->Gfx_RenderQuad(&q);
                SHR::SetQuad(&q, GV->colActive, x - 3, y - 2, x + 3, y + 2);
                hge->Gfx_RenderQuad(&q);
            }
        }
        hge->Gfx_SetClipping();

        ObjectOverlay();

        vPort->ClipScreen();


#ifdef DEBUG_DRAW
                                                                                                                                if (!hPlaneData[GetActivePlaneID()]->ObjectData.bEmpty) {
			int cellw = hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->GetContainerCellWidth(),
				cellh = hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->GetContainerCellHeight();
			int ioffsetx = Wrd2ScrX(GetActivePlane(), vPort->GetX()) / float(cellw),
				ioffsety = Wrd2ScrY(GetActivePlane(), vPort->GetY()) / float(cellh);
			int startx = ioffsetx * cellw,
				starty = ioffsety * cellh;
			startx = Wrd2ScrX(GetActivePlane(), startx);
			starty = Wrd2ScrY(GetActivePlane(), starty);

			for (int x = startx; x < vPort->GetX() + vPort->GetWidth(); x += cellw * fZoom) {
				if (x < 0) continue;
				hge->Gfx_RenderLine(x, vPort->GetY(), x, vPort->GetY() + vPort->GetHeight(), 0xFF00FF00);
			}
			for (int y = starty; y < vPort->GetY() + vPort->GetHeight(); y += cellh * fZoom) {
				if (y < 0) continue;
				hge->Gfx_RenderLine(vPort->GetX(), y, vPort->GetX() + vPort->GetWidth(), y, 0xFF00FF00);
			}
			GV->fntMyriad16->SetColor(0xFFFFFFFF);
			float gmx, gmy;
			hge->Input_GetMousePos(&gmx, &gmy);
			gmx = Scr2WrdX(GetActivePlane(), gmx);
			gmy = Scr2WrdY(GetActivePlane(), gmy);
			GV->fntMyriad16->printf(vPort->GetX(), vPort->GetY() + 20, HGETEXT_LEFT, "mouse cell 0x~y~%p ~w~(~y~%d~w~x~y~%d~w~)", 0,
				hPlaneData[GetActivePlaneID()]->ObjectData.hQuadTree->GetCellByCoords(gmx, gmy),
				int(gmx / cellw), int(gmy / cellh));
		}
#endif

        float len = 0.0f;
        for (int i = 1; i < m_vMeasurePoints.size(); i++) {
            int x1 = Wrd2ScrX(GetActivePlane(), m_vMeasurePoints[i - 1].first), y1 = Wrd2ScrY(GetActivePlane(),
                                                                                              m_vMeasurePoints[i -
                                                                                                               1].second),
                    x2 = Wrd2ScrX(GetActivePlane(), m_vMeasurePoints[i].first), y2 = Wrd2ScrY(GetActivePlane(),
                                                                                              m_vMeasurePoints[i].second);
            hge->Gfx_RenderLine(x1, y1, x2, y2, 0xFFFFFFFF);
            if (cbmeasAbsoluteDistance->isSelected())
                len += DISTANCE(m_vMeasurePoints[i - 1].first, m_vMeasurePoints[i - 1].second,
                                m_vMeasurePoints[i].first, m_vMeasurePoints[i].second);
            else
                len = DISTANCE(m_vMeasurePoints[i - 1].first, m_vMeasurePoints[i - 1].second, m_vMeasurePoints[i].first,
                               m_vMeasurePoints[i].second);
            if (m_vMeasurePoints[i - 1].first < m_vMeasurePoints[i].first &&
                m_vMeasurePoints[i - 1].second < m_vMeasurePoints[i].second ||
                m_vMeasurePoints[i].first < m_vMeasurePoints[i - 1].first &&
                m_vMeasurePoints[i].second < m_vMeasurePoints[i - 1].second)
                GV->fntMyriad16->printf(x1 + (x2 - x1) / 2, y1 + (y2 - y1) / 2 - 20, HGETEXT_LEFT, "~y~%.0f~w~px", 0,
                                        len);
            else
                GV->fntMyriad16->printf(x1 + (x2 - x1) / 2, y1 + (y2 - y1) / 2, HGETEXT_LEFT, "~y~%.0f~w~px", 0, len);
        }

        if (iActiveTool == EWW_TOOL_MEASURE && m_vMeasurePoints.size() != 0 &&
            conMain->getWidgetAt(mx, my) == vPort->GetWidget()) {
            int x1 = Wrd2ScrX(GetActivePlane(), m_vMeasurePoints[m_vMeasurePoints.size() - 1].first),
                    y1 = Wrd2ScrY(GetActivePlane(), m_vMeasurePoints[m_vMeasurePoints.size() - 1].second);
            int scrx = mx, scry = my;
            if (hge->Input_GetKeyState(HGEK_SHIFT)) {
                int lenx = mx - x1, leny = my - y1;
                float ratio = float(lenx) / float(leny);
                if (ratio >= -0.50f && ratio <= 0.50) { //straight up
                    scrx = x1;
                } else if (ratio > 1.5f || ratio < -1.5f) {
                    scry = y1;
                } else if (ratio >= 0.50f && ratio <= 1.50f) {
                    if (lenx < 0 && leny < 0) {
                        int dist = std::min(abs(lenx), abs(leny));
                        scrx = x1 - dist;
                        scry = y1 - dist;
                    } else {
                        int dist = std::min(abs(lenx), abs(leny));
                        scrx = x1 + dist;
                        scry = y1 + dist;
                    }
                } else if (ratio <= -0.5 && ratio >= -1.5) {
                    if (lenx < 0) {
                        int dist = std::min(lenx, leny);
                        scrx = x1 + dist;
                        scry = y1 - dist;
                    } else {
                        int dist = std::min(lenx, leny);
                        scrx = x1 - dist;
                        scry = y1 + dist;
                    }
                }
            }
            hge->Gfx_RenderLine(x1, y1, scrx, scry, 0xFFFFFFFF);
            if (cbmeasAbsoluteDistance->isSelected())
                len += DISTANCE(m_vMeasurePoints[m_vMeasurePoints.size() - 1].first,
                                m_vMeasurePoints[m_vMeasurePoints.size() - 1].second, Scr2WrdX(GetActivePlane(), scrx),
                                Scr2WrdY(GetActivePlane(), scry));
            else
                len = DISTANCE(m_vMeasurePoints[m_vMeasurePoints.size() - 1].first,
                               m_vMeasurePoints[m_vMeasurePoints.size() - 1].second, Scr2WrdX(GetActivePlane(), scrx),
                               Scr2WrdY(GetActivePlane(), scry));
            if (x1 < scrx && y1 < scry || scrx < x1 && scry < y1)
                GV->fntMyriad16->printf(x1 + (scrx - x1) / 2, y1 + (scry - y1) / 2 - 20, HGETEXT_LEFT, "~y~%.0f~w~px",
                                        0, len);
            else
                GV->fntMyriad16->printf(x1 + (scrx - x1) / 2, y1 + (scry - y1) / 2, HGETEXT_LEFT, "~y~%.0f~w~px", 0,
                                        len);
        }

        /*if (iMode == EWW_MODE_TILE) {
            if (fCamX < 0) {
                hge->Gfx_SetClipping(vPort->GetX(), vPort->GetY(), -fCamX, vPort->GetHeight());
                for (int y = 0; y <= vPort->GetHeight() / 120; y++) {
                    GV->sprCheckboard->Render(vPort->GetX(), vPort->GetY() + y * 120);
                }
            }
            int maxX = std::max(GetActivePlane()->GetPlaneWidthPx() - vPort->GetWidth() / fZoom, -40.0f);
            if (fCamX > maxX && !(GetActivePlane()->GetFlags() & WWD::Flag_p_XWrapping)) {
                hge->Gfx_SetClipping(vPort->GetX() + vPort->GetWidth() - (fCamX - maxX), vPort->GetY(), fCamX - maxX,
                                     vPort->GetHeight());
                for (int y = 0; y <= vPort->GetHeight() / 120; y++) {
                    GV->sprCheckboard->Render(vPort->GetX() + vPort->GetWidth() - (vPort->GetWidth() % 120),
                                              vPort->GetY() + y * 120);
                }
            }
            if (fCamY < 0) {
                hge->Gfx_SetClipping(vPort->GetX(), vPort->GetY(), vPort->GetWidth(), -fCamY);
                for (int x = 0; x <= vPort->GetWidth() / 120; x++) {
                    GV->sprCheckboard->Render(vPort->GetX() + x * 120, vPort->GetY());
                }
            }
            int maxY = std::max(GetActivePlane()->GetPlaneHeightPx() - vPort->GetHeight() / fZoom, -40.0f);
            if (fCamY > maxY && !(GetActivePlane()->GetFlags() & WWD::Flag_p_YWrapping)) {
                hge->Gfx_SetClipping(vPort->GetX(), vPort->GetY() + vPort->GetHeight() - (fCamY - maxY),
                                     vPort->GetWidth(), fCamY - maxY);
                for (int x = 0; x <= vPort->GetWidth() / 120; x++) {
                    GV->sprCheckboard->Render(vPort->GetX() + x * 120,
                                              vPort->GetY() + vPort->GetHeight() - (vPort->GetHeight() % 120));
                }
            }
        }*/
        hge->Gfx_SetClipping();

        if (bDrawTileProperties) {
            GV->fntMyriad16->SetColor(0xFF000000);
            GV->fntMyriad16->printf(vPort->GetX() + 11, vPort->GetY() + vPort->GetHeight() - 99, HGETEXT_LEFT,
                                    GETL(Lang_Legend), 0);
            GV->fntMyriad16->printf(vPort->GetX() + 61, vPort->GetY() + vPort->GetHeight() - 79, HGETEXT_LEFT,
                                    GETL(Lang_PropSolid), 0);
            GV->fntMyriad16->printf(vPort->GetX() + 61, vPort->GetY() + vPort->GetHeight() - 59, HGETEXT_LEFT,
                                    GETL(Lang_PropGround), 0);
            GV->fntMyriad16->printf(vPort->GetX() + 61, vPort->GetY() + vPort->GetHeight() - 39, HGETEXT_LEFT,
                                    GETL(Lang_PropClimb), 0);
            GV->fntMyriad16->printf(vPort->GetX() + 61, vPort->GetY() + vPort->GetHeight() - 19, HGETEXT_LEFT,
                                    GETL(Lang_PropDeath), 0);

            GV->fntMyriad16->SetColor(0xFFFFFFFF);
            GV->fntMyriad16->printf(vPort->GetX() + 10, vPort->GetY() + vPort->GetHeight() - 100, HGETEXT_LEFT,
                                    GETL(Lang_Legend), 0);
            GV->fntMyriad16->printf(vPort->GetX() + 60, vPort->GetY() + vPort->GetHeight() - 80, HGETEXT_LEFT,
                                    GETL(Lang_PropSolid), 0);
            GV->fntMyriad16->printf(vPort->GetX() + 60, vPort->GetY() + vPort->GetHeight() - 60, HGETEXT_LEFT,
                                    GETL(Lang_PropGround), 0);
            GV->fntMyriad16->printf(vPort->GetX() + 60, vPort->GetY() + vPort->GetHeight() - 40, HGETEXT_LEFT,
                                    GETL(Lang_PropClimb), 0);
            GV->fntMyriad16->printf(vPort->GetX() + 60, vPort->GetY() + vPort->GetHeight() - 20, HGETEXT_LEFT,
                                    GETL(Lang_PropDeath), 0);

            SHR::SetQuad(&q, COLOR_SOLID, vPort->GetX() + 10, vPort->GetY() + vPort->GetHeight() - 80,
                         vPort->GetX() + 50, vPort->GetY() + vPort->GetHeight() - 65);
            hge->Gfx_RenderQuad(&q);
            SHR::SetQuad(&q, COLOR_GROUND, vPort->GetX() + 10, vPort->GetY() + vPort->GetHeight() - 60,
                         vPort->GetX() + 50, vPort->GetY() + vPort->GetHeight() - 45);
            hge->Gfx_RenderQuad(&q);
            SHR::SetQuad(&q, COLOR_CLIMB, vPort->GetX() + 10, vPort->GetY() + vPort->GetHeight() - 40,
                         vPort->GetX() + 50, vPort->GetY() + vPort->GetHeight() - 25);
            hge->Gfx_RenderQuad(&q);
            SHR::SetQuad(&q, COLOR_DEATH, vPort->GetX() + 10, vPort->GetY() + vPort->GetHeight() - 20,
                         vPort->GetX() + 50, vPort->GetY() + vPort->GetHeight() - 5);
            hge->Gfx_RenderQuad(&q);
        }
    }

    hge->Gfx_SetClipping();

    ViewportOverlay();

    if (iActiveTool == EWW_TOOL_ALIGNOBJ) {
        GV->fntMyriad16->printf(vPort->GetX() + 14, vPort->GetY() + vPort->GetHeight() - 31, HGETEXT_LEFT,
                                "~l~%s", 0, GETL2S("Various", "AlignObj_Info"));
        GV->fntMyriad16->printf(vPort->GetX() + 15, vPort->GetY() + vPort->GetHeight() - 30, HGETEXT_LEFT,
                                "~w~%s", 0, GETL2S("Various", "AlignObj_Info"));
    } else if (iActiveTool == EWW_TOOL_MOVEOBJECT) {
        int yoffset = vPort->GetY() + vPort->GetHeight();

        GV->fntMyriad16->printf(vPort->GetX() + 14, yoffset - 61, HGETEXT_LEFT,
                                "~l~Shift - %s.", 0, GETL2S("Various", "MoveObj_Info1"));
        GV->fntMyriad16->printf(vPort->GetX() + 15, yoffset - 60, HGETEXT_LEFT,
                                "~y~Shift ~w~- %s.", 0, GETL2S("Various", "MoveObj_Info1"));

        GV->fntMyriad16->printf(vPort->GetX() + 14, yoffset - 46, HGETEXT_LEFT,
                                "~l~Ctrl - %s.", 0, GETL2S("Various", "MoveObj_Info2"));
        GV->fntMyriad16->printf(vPort->GetX() + 15, yoffset - 45, HGETEXT_LEFT,
                                "~y~Ctrl ~w~- %s.", 0, GETL2S("Various", "MoveObj_Info2"));

        GV->fntMyriad16->printf(vPort->GetX() + 14, yoffset - 31, HGETEXT_LEFT, "~l~Esc - %s.", 0,
                                GETL2S("Various", "MoveObj_Info3"));
        GV->fntMyriad16->printf(vPort->GetX() + 15, yoffset - 30, HGETEXT_LEFT, "~y~Esc ~w~- %s.", 0,
                                GETL2S("Various", "MoveObj_Info3"));

    }
}

void State::EditingWW::RenderArrow(int x, int y, int x2, int y2, bool finished, bool setColors, bool twoSided) {
    float len = sqrt(pow(x2 - x, 2) + pow(y2 - y, 2));

    if (setColors) {
        GV->sprArrowVerticalM->SetColor(0x77FFFFFF);
        GV->sprArrowVerticalU->SetColor(0x77FFFFFF);
        if (twoSided) GV->sprArrowVerticalD->SetColor(0x77FFFFFF);
        if (!finished) {
            GV->sprArrowVerticalU->SetColor(0x00FFFFFF, 0);
            GV->sprArrowVerticalU->SetColor(0x00FFFFFF, 1);
            GV->sprArrowVerticalU->SetColor(0x15FFFFFF, 2);
            GV->sprArrowVerticalU->SetColor(0x15FFFFFF, 3);
            GV->sprArrowVerticalM->SetColor(0xFFFFFFFF);
            GV->sprArrowVerticalM->SetColor(0x15FFFFFF, 0);
            GV->sprArrowVerticalM->SetColor(0x15FFFFFF, 1);
        }
    }
    hgeVector vec(x2 - x, y2 - y);
    vec.Normalize();
    float rot = atan2f(vec.x, vec.y);
    if ((twoSided && len < 80.0f * fZoom) || len < 40.0f * fZoom) {
        if (twoSided) {
            GV->sprArrowVerticalD->RenderEx(x + (x2 - x) / 2, y + (y2 - y) / 2, rot * -1 - M_PI, fZoom);
            GV->sprArrowVerticalU->RenderEx(x + (x2 - x) / 2, y + (y2 - y) / 2, rot * -1 - M_PI, fZoom);
        } else {
            if (x2 > x) x -= 20 * fZoom; else x += 20 * fZoom;
            if (y2 > y) y -= 20 * fZoom; else y += 20 * fZoom;
            GV->sprArrowVerticalU->RenderEx(x + (x2 - x) / 2, y + (y2 - y) / 2, rot * -1 - M_PI, fZoom);
        }
        return;
    }
    len -= 38.0f * fZoom;

    if (twoSided) {
        len -= 38.0f * fZoom;
    }

    vec.x = 0;
    vec.y = len;
    vec.Rotate(rot);
    rot *= -1;
    rot -= M_PI;

    if (twoSided) {
        hgeVector vec2(0, 38.0f * fZoom);
        vec2.Rotate(rot);
        x -= vec2.x;
        y -= vec2.y;
        GV->sprArrowVerticalD->RenderEx(x, y, rot, fZoom);
    }
    GV->sprArrowVerticalM->RenderEx(x, y, rot, fZoom, len / 128.0f);
    GV->sprArrowVerticalU->RenderEx(x - vec.x, y + vec.y, rot, fZoom);
}

bool State::EditingWW::Render() {
    SHR::SetQuad(&q, 0xFF2f2f2f, 0, 0, hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT));
    hge->Gfx_RenderQuad(&q);

    GV->sprLogoCaption->Render(8, 5);

    float mx, my;
    hge->Input_GetMousePos(&mx, &my);

    {
        // MINIMIZE AND CLOSE BUTTONS
        int x = hge->System_GetState(HGE_SCREENWIDTH) - LAY_APP_BUTTONS_COUNT * LAY_APP_BUTTON_W - bMaximized * 2;
        for (int i = 0; i < LAY_APP_BUTTONS_COUNT; i++) {
            if (mx > x && mx < x + LAY_APP_BUTTON_W && my > 0 && my < LAY_APPMENU_H) {
                if (fAppBarTimers[i] < 0.2f) {
                    fAppBarTimers[i] += hge->Timer_GetDelta();
                    if (fAppBarTimers[i] > 0.2f) fAppBarTimers[i] = 0.2f;
                }
            } else if (fAppBarTimers[i] > 0.0f) {
                fAppBarTimers[i] -= hge->Timer_GetDelta();
                if (fAppBarTimers[i] < 0.0f) fAppBarTimers[i] = 0.0f;
            }

            SHR::SetQuad(&q, SETA(0xFFFFFF, fAppBarTimers[i] * 5.0f * 20.f), x, 0, x + LAY_APP_BUTTON_W, LAY_APPMENU_H);
            hge->Gfx_RenderQuad(&q);

            int j = (bMaximized && i == 1) ? 3 : i;
            GV->hGfxInterface->sprAppBar[j]->SetColor(
                    SETA(0xFFFFFF, (unsigned char) (127.f + fAppBarTimers[i] * 5.0f * 128.f)));
            GV->hGfxInterface->sprAppBar[j]->Render(x + 10, 5);
            x += LAY_APP_BUTTON_W;
        }
    }

    if (hParser != NULL) {
        SHR::SetQuad(&q, 0xFF0b0b0b,
                     0, vPort->GetY() + vPort->GetHeight(), hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT) - LAY_STATUS_H);
        hge->Gfx_RenderQuad(&q);
        SHR::SetQuad(&q, 0xFF0b0b0b,
                     vPort->GetWidth(), vPort->GetY(), hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT) - LAY_STATUS_H);
        hge->Gfx_RenderQuad(&q);

        MDI->Render();

        //mode bar underlay
        hge->Gfx_RenderLine(1, LAY_APPMENU_Y + LAY_APPMENU_H, hge->System_GetState(HGE_SCREENWIDTH) - 1,
                            LAY_APPMENU_Y + LAY_APPMENU_H, GV->colOutline);

        if (hmbActive != 0)
            for (int x : hmbActive->vSeparators) {
                hge->Gfx_RenderLine(x, LAY_MODEBAR_Y + 2,
                                    x, LAY_MODEBAR_Y + LAY_MODEBAR_H - 3,
                                    GV->colBaseDark);
            }

        int wx = -1, wy = -1, tx = -1, ty = -1;
        if (conMain->getWidgetAt(mx, my) == vPort->GetWidget()) {
            wx = Scr2WrdX(GetActivePlane(), mx);
            wy = Scr2WrdY(GetActivePlane(), my);
            tx = (wx / GetActivePlane()->GetTileWidth()) % GetActivePlane()->GetPlaneWidth();
            ty = (wy / GetActivePlane()->GetTileHeight()) % GetActivePlane()->GetPlaneHeight();
        }
        int x = butMicroObjectCB->getX() + butMicroObjectCB->getWidth() + 10,
            y = hge->System_GetState(HGE_SCREENHEIGHT) - (LAY_STATUS_H - GV->fntMyriad16->GetHeight() / 2) - 1;
        std::string text = GETL2S("Various", "TilesOnScreen");
#define RENDER_STATUS_VALUE(valName) text += " ~w~"; text += valName; \
        GV->fntMyriad16->SetColor(0xAAe1e1e1); \
        GV->fntMyriad16->Render(x, y, HGETEXT_LEFT, text.c_str(), 0); \
        x += GV->fntMyriad16->GetStringWidth(text.c_str()) + 20; \
        hge->Gfx_RenderLine(x - 10, hge->System_GetState(HGE_SCREENHEIGHT) - LAY_STATUS_H + 2, x - 10, \
                            hge->System_GetState(HGE_SCREENHEIGHT) - 3, GV->colLineBright);
        RENDER_STATUS_VALUE(std::to_string(iTilesOnScreen));
        text = GETL2S("Various", "ObjectsOnScreen");
        RENDER_STATUS_VALUE(std::to_string(iObjectsOnScreen));
        text = GETL2S("Various", "WorldMousePosition");
        RENDER_STATUS_VALUE(std::to_string(wx) + 'x' + std::to_string(wy));
        text = GETL2S("Various", "TileMousePosition");
        RENDER_STATUS_VALUE(std::to_string(tx) + 'x' + std::to_string(ty));
        text = "FPS:";
        RENDER_STATUS_VALUE(std::to_string(hge->Timer_GetFPS()));
        GV->fntMyriad16->Render(hge->System_GetState(HGE_SCREENWIDTH) - 200,
                                y, HGETEXT_RIGHT, GETL2S("Various", "Zoom"),
                                0);
        GV->fntMyriad16->printf(hge->System_GetState(HGE_SCREENWIDTH) - 170,
                                y, HGETEXT_RIGHT, "%.1fx", 0, fZoom);
    }
#ifdef SHOWMEMUSAGE
    GV->UpdateMemUsage();
	EnterCriticalSection(&GV->csMemUsage);
	//GV->fntMyriad16->printf(840, 25, HGETEXT_LEFT, "~l~Mem: ~y~%s~l~MB", 0, GV->szMemUsage);
	LeaveCriticalSection(&GV->csMemUsage);
#endif

    //lower bar
    hge->Gfx_RenderLine(0, hge->System_GetState(HGE_SCREENHEIGHT) - LAY_STATUS_H, hge->System_GetState(HGE_SCREENWIDTH),
                        hge->System_GetState(HGE_SCREENHEIGHT) - LAY_STATUS_H, GV->colOutline);

    try {
        gui->draw();
    }
    catch (gcn::Exception &exc) {
        GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(),
                            exc.getLine());
    }

    if (!bMaximized) {
        hge->Gfx_RenderLine(1, 1, hge->System_GetState(HGE_SCREENWIDTH) - 1, 1, GV->colOutline);
        hge->Gfx_RenderLine(1, 1, 1, hge->System_GetState(HGE_SCREENHEIGHT) - 1, GV->colOutline);
        hge->Gfx_RenderLine(1, hge->System_GetState(HGE_SCREENHEIGHT), hge->System_GetState(HGE_SCREENWIDTH) - 1, hge->System_GetState(HGE_SCREENHEIGHT), GV->colOutline);
        hge->Gfx_RenderLine(hge->System_GetState(HGE_SCREENWIDTH), 1, hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT) - 1, GV->colOutline);
    }

    if (hParser == NULL) {
        /*if (MDI->GetDocsCount()) {
            int x = vPort->GetX() + 8, y = vPort->GetY() + 8;

            float mx, my;
            hge->Input_GetMousePos(&mx, &my);
            if (mx > x && my > y && mx < x + 32 && my < y + 32) {
                if (hge->Input_GetKeyState(HGEK_LBUTTON)) {
                    MDI->BackToLastActive();
                }

                fHomeBackButTimer += hge->Timer_GetDelta();
                if (fHomeBackButTimer > 0.2) {
                    fHomeBackButTimer = 0.2;
                }
            } else if (fHomeBackButTimer > 0.0) {
                fHomeBackButTimer -= hge->Timer_GetDelta();
                if (fHomeBackButTimer < 0.0) {
                    fHomeBackButTimer = 0.0;
                }
            }

            GV->sprHomepageBackButton->SetColor(SETA(0xFFFFFF, 0x77 + 0x77 * fHomeBackButTimer * 5.f));
            GV->sprHomepageBackButton->Render(x, y);
        }*/
    } else {
        if (_isOnTop()) {
            if (bForceTileClipbPreview)
                RenderTileClipboardPreview();
            else if (bForceObjectClipbPreview)
                RenderObjectClipboardPreview();

            hInvCtrl->DrawDraggedObject();
        } else {
            q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = 0x77000000;

            q.v[0].x = q.v[3].x = 0;
            q.v[0].y = q.v[1].y = 0;
            q.v[1].x = q.v[2].x = hge->System_GetState(HGE_SCREENWIDTH);
            q.v[2].y = q.v[3].y = hge->System_GetState(HGE_SCREENHEIGHT);

            hge->Gfx_RenderQuad(&q);
        }
    }

    if (fade_iAction < 2) {
        q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = 0xFF000000;
        q.v[0].x = q.v[3].x = 0;
        q.v[0].y = q.v[1].y = LAY_MODEBAR_Y;
        q.v[1].x = q.v[2].x = hge->System_GetState(HGE_SCREENWIDTH);
        q.v[2].y = q.v[3].y = hge->System_GetState(HGE_SCREENHEIGHT);
        hge->Gfx_RenderQuad(&q);

        GV->RenderLogoWithVersion(hge->System_GetState(HGE_SCREENWIDTH) / 2, hge->System_GetState(HGE_SCREENHEIGHT) / 2);

        if (fade_iAction == 0) {
            q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = ARGB(int(fade_fAlpha), 0, 0, 0);

            hge->Gfx_RenderQuad(&q);
        }
    } else if (fade_iAction == 2) {
        q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = ARGB(255 - int(fade_fAlpha), 0, 0, 0);
        q.v[0].x = q.v[3].x = 0;
        q.v[0].y = q.v[1].y = LAY_MODEBAR_Y;
        q.v[1].x = q.v[2].x = hge->System_GetState(HGE_SCREENWIDTH);
        q.v[2].y = q.v[3].y = hge->System_GetState(HGE_SCREENHEIGHT);
        hge->Gfx_RenderQuad(&q);

        GV->RenderLogoWithVersion(hge->System_GetState(HGE_SCREENWIDTH) / 2, hge->System_GetState(HGE_SCREENHEIGHT) / 2, 255 - int(fade_fAlpha));
    }

    if (draggedFilesIn) {
        SHR::SetQuad(&q, 0xEE000000,
                     0, LAY_VIEWPORT_Y, hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT));
        hge->Gfx_RenderQuad(&q);

        auto& files = hge->System_GetDraggedFiles();

        int x = hge->System_GetState(HGE_SCREENWIDTH) / 2, y = hge->System_GetState(HGE_SCREENHEIGHT) / 2 + 18;

        if (files.size()) {
            y -= std::min(files.size(), 10u) * 9;
            GV->fntMyriad16->Render(x, y, HGETEXT_CENTER, GETL(Lang_FilesDragged_MapsToOpen), 0);

            int maxW = 0;
            for (auto filepath : files) {
                int w = GV->fntMyriad16->GetStringWidth(filepath.c_str(), false);
                if (w > maxW) {
                    maxW = w;
                }
            }

            if (maxW > 500) {
                maxW = 500;
            }

            x -= maxW / 2 + 10;
            y += 18 * 2;

            int i = 0;
            for (auto filepath : files) {
                byte icon = iDraggedFileIcon[i++];
                if (icon < 50)
                    GV->sprGamesSmall[icon]->Render(x, y);
                else
                    GV->sprLevelsMicro16[icon - 51]->Render(x, y);

                if (maxW == 500 && GV->fntMyriad16->GetStringWidth(filepath.c_str()) > 500) {
                    std::string temp(filepath);
                    do {
                        temp.erase(0, 5);
                    } while (GV->fntMyriad16->GetStringWidth(temp.c_str()) > 490);
                    temp.insert(0, "(...)");
                    GV->fntMyriad16->Render(x + 20, y, HGETEXT_LEFT, temp.c_str(), 0);
                } else {
                    GV->fntMyriad16->Render(x + 20, y, HGETEXT_LEFT, filepath.c_str(), 0);
                }

                y += 18;

                if (i == 10) {
                    GV->fntMyriad16->printf(x + maxW / 2, y, HGETEXT_CENTER,
                                            GETL(Lang_FilesDragged_AndMore), 0, files.size() - i);
                    break;
                }
            }
        } else {
            GV->fntMyriad16->Render(x, y, HGETEXT_CENTER, GETL(Lang_FilesDragged_Unsupported), 0);
        }
    }

    if (GV->bWinter) {
        if (fade_iAction == 2) {
            GV->sprSnowflake->SetColor(ARGB(255 - int(fade_fAlpha), 255, 255, 255));
        }

        float dt = hge->Timer_GetTime() * 37.f;
#define SNOWFLAKES_COUNT 12
        int w = hge->System_GetState(HGE_SCREENWIDTH) / SNOWFLAKES_COUNT,
            h = hge->System_GetState(HGE_SCREENHEIGHT) + 32;
        for (int i = 0; i < SNOWFLAKES_COUNT; ++i) {
            dt += i * 240;
            float x = (0.5f + i + 0.4f * sin(dt * 0.01f)) * w,
                  y = -16.f + int((dt * (i % 2 ? 2.5f : 4.f) + abs(sin(dt * 0.3f))) + i * (h * 0.66f)) % h;
            GV->sprSnowflake->RenderEx(x, y, sin(dt * 0.0001f) * 90, (i % 2 ? 0.5f : 0.3f));
        }
    }

#ifdef CONF_WATERMARK
    GV->fntMyriad80->SetColor(ARGB(64, 255, 255, 255));
	GV->fntMyriad80->Render(vPort->GetX() + vPort->GetWidth() - 5, vPort->GetY() + vPort->GetHeight() - 80, HGETEXT_RIGHT, CONF_WATERMARK);
#endif
    return false;
}

void State::EditingWW::DrawObjSearch() {
    if (strlen(winobjseaName->getText().c_str()) != 0) {
        //if( !(!strcmp(szObjSearchBuffer, winobjseaName->getText().c_str())) ){
        //}
        if (szObjSearchBuffer == NULL || !(!strcmp(szObjSearchBuffer, winobjseaName->getText().c_str()))) {
            if (szObjSearchBuffer != NULL)
                delete[] szObjSearchBuffer;
            szObjSearchBuffer = new char[strlen(winobjseaName->getText().c_str()) + 1];
            strcpy(szObjSearchBuffer, winobjseaName->getText().c_str());
            UpdateSearchResults();
        }
    } else {
        if (szObjSearchBuffer != NULL) {
            delete[] szObjSearchBuffer;
            szObjSearchBuffer = NULL;
            UpdateSearchResults();
        }
        return;
    }
    int fx, fy;
    winSearchObj->getAbsolutePosition(fx, fy);
    hge->Gfx_SetClipping(fx + 8, fy + 122, 424, 386);
    fx += 8;
    fy += 122;
    float fSliVal = sliSearchObj->getScaleEnd() - sliSearchObj->getValue();
    int startindex = (sliSearchObj->isVisible() ? fSliVal : 0) / 140;

    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    bool bHasMouse = conMain->getWidgetAt(mx, my) == winSearchObj;

    if (bHasMouse && sliSearchObj->isVisible() && mx > fx && my > fy && mx < fx + 416 && my < fy + 386 - 122) {
        sliSearchObj->setValue(sliSearchObj->getValue() + hge->Input_GetMouseWheel() * 140);
    }

    for (int i = startindex; i < startindex + 387 / 140 + 2; i++) {
        if (i < 0) continue;
        else if (i >= vObjSearchResults.size()) break;

        WWD::Object *obj = GetActivePlane()->GetObjectByIterator(vObjSearchResults[i].first);
        hgeSprite *spr = SprBank->GetObjectSprite(obj);
        spr->SetColor(0xFFFFFFFF);
		spr->SetFlip(GetUserDataFromObj(obj)->GetFlipX(), GetUserDataFromObj(obj)->GetFlipY(), true);
        float fScale = 1;
        if (spr->GetWidth() > 130 || spr->GetHeight() > 130)
            fScale = 130 / (spr->GetWidth() > spr->GetHeight() ? spr->GetWidth() : spr->GetHeight());
        GV->sprCheckboard->RenderStretch(fx, fy + i * 140 - fSliVal, fx + 130, fy + 130 + i * 140 - fSliVal);
        spr->RenderEx(fx + 65, fy + i * 140 - fSliVal + 65, 0, fScale);
        GV->fntMyriad16->printf(fx + 135, fy + i * 140 - fSliVal, HGETEXT_LEFT, "~w~%s: ~y~%d ~w~(#~y~%i~w~)", 0,
                                GETL2S("ObjectSearch", "ID"), obj->GetParam(WWD::Param_ID), i + 1);
        int yoffset = 20;
        if (strlen(obj->GetName()) > 0) {
            GV->fntMyriad16->printf(fx + 135, fy + i * 140 - fSliVal + yoffset, HGETEXT_LEFT, "%s: ~y~%s~w~", 0,
                                    GETL2S("ObjectSearch", "Name"), obj->GetName());
            yoffset += 20;
        }
        if (strlen(obj->GetLogic()) > 0) {
            GV->fntMyriad16->printf(fx + 135, fy + i * 140 - fSliVal + yoffset, HGETEXT_LEFT, "%s: ~y~%s~w~", 0,
                                    GETL2S("ObjectSearch", "Logic"), obj->GetLogic());
            yoffset += 20;
        }
        if (strlen(obj->GetImageSet()) > 0) {
            GV->fntMyriad16->printf(fx + 135, fy + i * 140 - fSliVal + yoffset, HGETEXT_LEFT, "%s: ~y~%s~w~", 0,
                                    GETL2S("ObjectSearch", "ImageSet"), obj->GetImageSet());
            yoffset += 20;
        }
        if (strlen(obj->GetAnim()) > 0) {
            GV->fntMyriad16->printf(fx + 135, fy + i * 140 - fSliVal + yoffset, HGETEXT_LEFT, "%s: ~y~%s~w~", 0,
                                    GETL2S("ObjectSearch", "Anim"), obj->GetAnim());
            yoffset += 20;
        }
        GV->fntMyriad16->printf(fx + 135, fy + i * 140 - fSliVal + yoffset, HGETEXT_LEFT, "%s: ~y~%d~w~x~y~%d~w~", 0,
                                GETL2S("ObjectSearch", "Pos"), obj->GetParam(WWD::Param_LocationX),
                                obj->GetParam(WWD::Param_LocationY));
        yoffset += 20;

        int butx = fx + 135, buty = fy + i * 140 - fSliVal + 97, butw = butObjSearchSelect->getWidth(), buth = 33;
        if (bHasMouse && mx > butx && mx < butx + butw && my > buty && my < buty + 33 && my > fy && my < fy + 386) {
            if (hge->Input_GetKeyState(HGEK_LBUTTON)) {
                butObjSearchSelect->drawButton(GV->hGfxInterface, 3, butx, buty, butw, buth, 0xFFFFFFFF);
            } else {
                butObjSearchSelect->drawButton(GV->hGfxInterface, 3, butx, buty, butw, buth, 0x77FFFFFF);
            }
            if (hge->Input_KeyUp(HGEK_LBUTTON)) {
                WWD::Object *obj = GetActivePlane()->GetObjectByIterator(vObjSearchResults[i].first);
                vObjectsPicked.clear();
                vObjectsPicked.push_back(obj);
                fCamX = obj->GetX() - vPort->GetWidth() / 2 * fZoom;
                fCamY = obj->GetY() - vPort->GetHeight() / 2 * fZoom;
                vPort->MarkToRedraw();
            }
        } else {
            butObjSearchSelect->drawButton(GV->hGfxInterface, 2, butx, buty, butw, buth, 0xFFFFFFFF);
        }
        GV->fntMyriad16->Render(butx + butw / 2, buty + 8, HGETEXT_CENTER, GETL2S("ObjectSearch", "GoToObject"), 0);

        hge->Gfx_RenderLine(fx, fy + i * 140 - fSliVal + 135, fx + 424, fy + i * 140 - fSliVal + 135, GV->colLineDark);
        hge->Gfx_RenderLine(fx, fy + i * 140 - fSliVal + 136, fx + 424, fy + i * 140 - fSliVal + 136,
                            GV->colLineBright);

    }
    hge->Gfx_SetClipping();
    if (sliSearchObj->isVisible()) {
        hge->Gfx_RenderLine(fx, fy, fx + 424, fy, 0xFF000000);
        hge->Gfx_RenderLine(fx, fy + 386, fx + 424, fy + 386, 0xFF000000);
        hge->Gfx_RenderLine(fx, fy, fx, fy + 386, 0xFF000000);
    }
}

int State::EditingWW::RenderObject(WWD::Object *hObj, int x, int y, DWORD col) {
    hgeSprite *spr = SprBank->GetObjectSprite(hObj);
    spr->SetColor(col);
    spr->SetFlip(hObj->GetFlipX(), hObj->GetFlipY(), true);
    if (!strcmp(hObj->GetLogic(), "PunkRat")) {
        hgeSprite *cannonspr = SprBank->GetAssetByID("LEVEL_CANNON")->GetIMGByIterator(0)->GetSprite();
        cannonspr->SetColor(col);
        cannonspr->SetFlip(GetUserDataFromObj(hObj)->GetFlipX(), GetUserDataFromObj(hObj)->GetFlipY());
        cannonspr->RenderEx(x + (5 * GetUserDataFromObj(hObj)->GetFlipX()) * fZoom, y + 15 * fZoom, 0, fZoom);
    }
    spr->RenderEx(x, y, 0, fZoom);
    if (!strcmp(hObj->GetLogic(), "BreakPlank")) {
        float fmod = 0;
        for (int z = 0; z < hObj->GetParam(WWD::Param_Width) - 1; z++) {
            fmod += 64.0f * fZoom;
            spr->RenderEx(x + fmod, y, 0, fZoom);
        }
        return hObj->GetParam(WWD::Param_Width);
    } else if (!strcmp(hObj->GetLogic(), "FrontStackedCrates") || !strcmp(hObj->GetLogic(), "BackStackedCrates")) {
        int irepeatnum = 0;
        for (int z = 0; z < 2; z++) {
            WWD::Rect tmprect = hObj->GetUserRect(z);
            if (tmprect.x1 != 0) irepeatnum++;
            if (tmprect.y1 != 0) irepeatnum++;
            if (tmprect.x2 != 0) irepeatnum++;
            if (tmprect.y2 != 0) irepeatnum++;
        }
        if (!hObj->GetParam(WWD::Param_Powerup))
            irepeatnum--;
        float fmod = 0;
        for (int z = 0; z < irepeatnum; z++) {
            fmod -= 43.0f * fZoom;
            spr->RenderEx(x, y + fmod, 0, fZoom);
        }
        return irepeatnum + 1;
    }
    return 1;
}

void State::EditingWW::PreRender() {
    if (hParser != NULL)
        for (int i = 0; i < hParser->GetPlanesCount(); i++) {
            if (hPlaneData[i]->hRB != NULL)
                hPlaneData[i]->hRB->Update();
        }
    vPort->Update();
}

int State::EditingWW::RenderLayer(WWD::Plane *hPl, bool bDefaultZoom) {
    int pl = 0;
    for (int i = 0; i < hParser->GetPlanesCount(); i++)
        if (hParser->GetPlane(i) == hPl) {
            pl = i;
            break;
        }
    int rcount = 0;
    float fDrawZoom = (bDefaultZoom ? 1.0f : fZoom);
    int endplanex = hPl->GetPlaneWidth() * hPl->GetTileWidth() * fDrawZoom - 1,
            endplaney = hPl->GetPlaneHeight() * hPl->GetTileHeight() * fDrawZoom - 1;
    for (int y = 0; y < hPl->GetPlaneHeight(); y++) {
        float posy = y * hPl->GetTileHeight();
        for (int x = 0; x < hPl->GetPlaneWidth(); x++) {
            float posx = x * hPl->GetTileWidth();
            WWD::Tile* tile = hPl->GetTile(x, y);
            if (tile->IsFilled()) {
                q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = hDataCtrl->GetPalette()->GetColor(
                        hPl->GetFillColor());
                q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;
                q.v[0].x = posx;
                q.v[0].y = posy;
                q.v[1].x = q.v[0].x + hPl->GetTileWidth() * fDrawZoom;
                q.v[1].y = q.v[0].y;
                q.v[2].x = q.v[1].x;
                q.v[2].y = q.v[0].y + hPl->GetTileHeight() * fDrawZoom;
                q.v[3].x = q.v[0].x;
                q.v[3].y = q.v[2].y;
                hge->Gfx_RenderQuad(&q);
                rcount++;
            } else if (!tile->IsInvisible()) {
                hgeSprite *spr;
                //uint16_t imgSetIndex = tile->GetID() >> 16;
                uint16_t id = tile->GetID();
                cTile *p = hTileset->GetTile(hPl->GetImageSet(0), id);
                spr = p ? p->GetImage() : GV->sprBlank;
                spr->SetColor(0xFFFFFFFF);
                spr->SetFlip(0, 0);
                spr->SetHotSpot(0, 0);
                spr->RenderEx(posx, posy, 0, fDrawZoom);
                rcount++;
            }

            if (hPlaneData[pl]->bDrawBoundary &&
                ((x % hPl->GetPlaneWidth()) == hPl->GetPlaneWidth() - 1 &&
                 (hPl->GetFlags() & WWD::Flag_p_YWrapping) != 0 ||
                 (x % hPl->GetPlaneWidth()) == hPl->GetPlaneWidth() - 1 &&
                 (hPl->GetFlags() & WWD::Flag_p_YWrapping) == 0))
                hge->Gfx_RenderLine(int(posx + hPl->GetTileWidth() * fDrawZoom), 0,
                                    int(posx + hPl->GetTileWidth() * fDrawZoom), endplaney, ARGB(255, 255, 0, 255));
            else if (hPlaneData[pl]->bDrawGrid)
                hge->Gfx_RenderLine(int(posx + hPl->GetTileWidth() * fDrawZoom), 0,
                                    int(posx + hPl->GetTileWidth() * fDrawZoom), endplaney, ARGB(255, 255, 255, 255));
        }
        if (hPlaneData[pl]->bDrawBoundary &&
            ((y % hPl->GetPlaneHeight()) == hPl->GetPlaneHeight() - 1 &&
             (hPl->GetFlags() & WWD::Flag_p_YWrapping) != 0 ||
             (y % hPl->GetPlaneHeight()) == hPl->GetPlaneHeight() - 1 &&
             (hPl->GetFlags() & WWD::Flag_p_YWrapping) == 0))
            hge->Gfx_RenderLine(0, int(posy + hPl->GetTileHeight() * fDrawZoom), endplanex,
                                int(posy + hPl->GetTileHeight() * fDrawZoom), ARGB(255, 255, 0, 255));
        else if (hPlaneData[pl]->bDrawGrid)
            hge->Gfx_RenderLine(0, int(posy + hPl->GetTileHeight() * fDrawZoom), endplanex,
                                int(posy + hPl->GetTileHeight() * fDrawZoom), ARGB(255, 255, 255, 255));
    }
    return rcount;
}

void State::EditingWW::DrawSelectFillColor() {
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    int selx = -1, sely = -1;
    int baseX, baseY;
    wintpFillColor->getAbsolutePosition(baseX, baseY);
    if (mx > baseX + 6 && mx < baseX + 196 && my > baseY + 29 &&
        my < baseY + 219 /*&& ( int(my-baseY+10)%12 > 2 || int(my-baseY+10)%12 == 0 )*/) {
        selx = (mx - (baseX + 6)) / 12;
        sely = (my - (baseY + 29)) / 12;
        if (hge->Input_KeyDown(HGEK_LBUTTON) && sely * 16 + selx != GetActivePlane()->GetFillColor()) {
            GetActivePlane()->SetFillColor(sely * 16 + selx);
            vPort->MarkToRedraw();
        }
    }
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 16; x++) {
            SHR::SetQuad(&q, hDataCtrl->GetPalette()->GetColor(y * 16 + x), baseX + 6 + x * 12, baseY + 29 + y * 12,
                         baseX + 6 + x * 12 + 10, baseY + 29 + y * 12 + 10);
            hge->Gfx_RenderQuad(&q);
            if (x == selx && y == sely) {
                hge->Gfx_RenderLine(baseX + 5 + x * 12, baseY + 29 + y * 12, baseX + 6 + x * 12 + 10,
                                    baseY + 29 + y * 12, TILE_HIGHLIGHT_COLOR_SOLID);
                hge->Gfx_RenderLine(baseX + 6 + x * 12, baseY + 29 + y * 12, baseX + 6 + x * 12,
                                    baseY + 29 + y * 12 + 10, TILE_HIGHLIGHT_COLOR_SOLID);
                hge->Gfx_RenderLine(baseX + 6 + x * 12 + 11, baseY + 28 + y * 12, baseX + 6 + x * 12 + 11,
                                    baseY + 30 + y * 12 + 10, TILE_HIGHLIGHT_COLOR_SOLID);
                hge->Gfx_RenderLine(baseX + 5 + x * 12, baseY + 30 + y * 12 + 10, baseX + 6 + x * 12 + 10,
                                    baseY + 29 + y * 12 + 10, TILE_HIGHLIGHT_COLOR_SOLID);
            } else if (y * 16 + x == GetActivePlane()->GetFillColor()) {
                hge->Gfx_RenderLine(baseX + 5 + x * 12, baseY + 29 + y * 12, baseX + 6 + x * 12 + 10,
                                    baseY + 29 + y * 12, TILE_PICK_COLOR_SOLID);
                hge->Gfx_RenderLine(baseX + 6 + x * 12, baseY + 29 + y * 12, baseX + 6 + x * 12,
                                    baseY + 29 + y * 12 + 10, TILE_PICK_COLOR_SOLID);
                hge->Gfx_RenderLine(baseX + 6 + x * 12 + 11, baseY + 28 + y * 12, baseX + 6 + x * 12 + 11,
                                    baseY + 30 + y * 12 + 10, TILE_PICK_COLOR_SOLID);
                hge->Gfx_RenderLine(baseX + 5 + x * 12, baseY + 30 + y * 12 + 10, baseX + 6 + x * 12 + 10,
                                    baseY + 29 + y * 12 + 10, TILE_PICK_COLOR_SOLID);
            }
        }
    int previewcol;
    if (selx == -1 && sely == -1) {
        GV->fntMyriad16->printf(baseX + 5, baseY + 229, HGETEXT_LEFT, "~l~%s: #~y~%d", 0, GETL(Lang_ActualFillColor),
                                GetActivePlane()->GetFillColor());
        previewcol = GetActivePlane()->GetFillColor();
    } else {
        GV->fntMyriad16->printf(baseX + 5, baseY + 229, HGETEXT_LEFT, "~l~%s: #~y~%d", 0, GETL(Lang_SelectedFillColor),
                                sely * 16 + selx);
        previewcol = sely * 16 + selx;
    }
    SHR::SetQuad(&q, hDataCtrl->GetPalette()->GetColor(previewcol), baseX + 161, baseY + 229, baseX + 196, baseY + 249);
    hge->Gfx_RenderQuad(&q);
}

void State::EditingWW::DrawPlaneOverlay(WWD::Plane *hPl) {
    if (hPl == GetActivePlane() && iMode == EWW_MODE_TILE &&
        (bDragSelection || (iTileSelectX1 != -1 && iTileSelectY1 != -1))) {
        int x1, y1, x2, y2;
        if (bDragSelection) {
            float mx, my;
            hge->Input_GetMousePos(&mx, &my);
            mx = Scr2WrdX(hPl, mx);
            my = Scr2WrdY(hPl, my);
            if (iDragSelectionOrigX == mx && iDragSelectionOrigY == my) return;
            x1 = int(iDragSelectionOrigX / GetActivePlane()->GetTileWidth()) * GetActivePlane()->GetTileWidth();
            y1 = int(iDragSelectionOrigY / GetActivePlane()->GetTileHeight()) * GetActivePlane()->GetTileHeight();
            x2 = int(mx / GetActivePlane()->GetTileWidth()) * GetActivePlane()->GetTileWidth();
            y2 = int(my / GetActivePlane()->GetTileHeight()) * GetActivePlane()->GetTileHeight();
            if (x1 > x2) x1 += GetActivePlane()->GetTileWidth(); else x2 += GetActivePlane()->GetTileWidth();
            if (y1 > y2) y1 += GetActivePlane()->GetTileHeight(); else y2 += GetActivePlane()->GetTileHeight();
            x1 = Wrd2ScrXrb(hPl, x1);
            y1 = Wrd2ScrYrb(hPl, y1);
            x2 = Wrd2ScrXrb(hPl, x2);
            y2 = Wrd2ScrYrb(hPl, y2);
        } else {
            x1 = Wrd2ScrXrb(hPl, iTileSelectX1 * GetActivePlane()->GetTileWidth());
            y1 = Wrd2ScrYrb(hPl, iTileSelectY1 * GetActivePlane()->GetTileHeight());
            x2 = Wrd2ScrXrb(hPl, (iTileSelectX2 + 1) * GetActivePlane()->GetTileWidth());
            y2 = Wrd2ScrYrb(hPl, (iTileSelectY2 + 1) * GetActivePlane()->GetTileHeight());
        }

        RenderAreaRect(WWD::Rect(x1, y1, x2, y2), true);
    }
}

void State::EditingWW::ViewportOverlay() {
    hRulers->Render();

    hge->Gfx_SetClipping(vPort->GetX(), vPort->GetY(), vPort->GetWidth() - (hParser == NULL) * 2, vPort->GetHeight());
    GV->Console->Render();
    hge->Gfx_SetClipping();

    hge->Gfx_RenderLine(vPort->GetX() + 1, vPort->GetY(), vPort->GetX() + 1, vPort->GetY() + vPort->GetHeight(), SETA(0, 0xFF));
    hge->Gfx_RenderLine(vPort->GetX() + 2, vPort->GetY(), vPort->GetX() + 2, vPort->GetY() + vPort->GetHeight(), SETA(0, 0x77));

    hge->Gfx_RenderLine(vPort->GetX() + vPort->GetWidth(), vPort->GetY(), vPort->GetX() + vPort->GetWidth(), vPort->GetY() + vPort->GetHeight(), SETA(0, 0x77));

    hge->Gfx_RenderLine(vPort->GetX(), vPort->GetY() + 1, vPort->GetX() + vPort->GetWidth(), vPort->GetY() + 1, SETA(0, 0xFF));
    hge->Gfx_RenderLine(vPort->GetX(), vPort->GetY() + 2, vPort->GetX() + vPort->GetWidth(), vPort->GetY() + 2, SETA(0, 0x77));

    hge->Gfx_RenderLine(vPort->GetX(), vPort->GetY() + vPort->GetHeight(), vPort->GetX() + vPort->GetWidth(), vPort->GetY() + vPort->GetHeight(), SETA(0, 0x77));
}

void State::EditingWW::RenderCloudTip(int x, int y, int w, int h, int ax, int ay) {
    SHR::SetQuad(&q, GV->colBase, x, y, x + w, y + h);
    hge->Gfx_RenderQuad(&q);
    hgeTriple tr;
    tr.tex = 0;
    tr.blend = BLEND_DEFAULT;
    tr.v[0].col = tr.v[1].col = tr.v[2].col = GV->colBase;
    tr.v[0].z = tr.v[1].z = tr.v[2].z = 1.0f;
    tr.v[0].x = x + 25;
    tr.v[0].y = y + h;
    tr.v[1].x = x + 45;
    tr.v[1].y = y + h;
    tr.v[2].x = ax;
    tr.v[2].y = ay;
    hge->Gfx_RenderTriple(&tr);
    hge->Gfx_RenderLine(x + 25, y + h, ax, ay, 0xFF000000);
    hge->Gfx_RenderLine(x + 45, y + h, ax, ay, 0xFF000000);
    hge->Gfx_RenderLine(x, y + h, x + 25, y + h, 0xFF000000);
    hge->Gfx_RenderLine(x + 45, y + h, x + w, y + h, 0xFF000000);
    hge->Gfx_RenderLine(x, y, x + w, y, 0xFF000000);
    hge->Gfx_RenderLine(x, y, x, y + h, 0xFF000000);
    hge->Gfx_RenderLine(x + w, y, x + w, y + h, 0xFF000000);
}

void State::EditingWW::RenderTileClipboardPreview() {
    if (MDI->GetActiveDoc() == NULL) return;
    if (MDI->GetActiveDoc()->hTileClipboard == NULL) {
        int len = GV->fntMyriad16->GetStringWidth(GETL2S("ClipboardPreview", "TileClipboardEmpty"));
        RenderCloudTip(10, hge->System_GetState(HGE_SCREENHEIGHT) - 70, len + 20, 20, butMicroTileCB->getX() + 12,
                       butMicroTileCB->getY() - 3);
        GV->fntMyriad16->Render(20, hge->System_GetState(HGE_SCREENHEIGHT) - 70, HGETEXT_LEFT,
                                GETL2S("ClipboardPreview", "TileClipboardEmpty"), 0);
        return;
    }
    int cbw = MDI->GetActiveDoc()->iTileCBw,
        cbh = MDI->GetActiveDoc()->iTileCBh;
    int tileW = 64, tileH = 64;
    if (tileW * cbw > 400 || tileW * cbh > 400) {
        tileW = 400 / (cbw > cbh ? cbw : cbh);
        tileH = 400 / (cbw > cbh ? cbw : cbh);
    }
    int px = 20, py = hge->System_GetState(HGE_SCREENHEIGHT) - (tileH * cbh + 20 + 50) + 10;
    RenderCloudTip(px - 10, py - 10, tileW * cbw + 20, tileH * cbh + 20, butMicroTileCB->getX() + 12,
                   butMicroTileCB->getY() - 3);
    for (int i = 0, y = 0; y < cbh; y++)
        for (int x = 0; x < cbw; x++, i++) {
            WWD::Tile* tile = &MDI->GetActiveDoc()->hTileClipboard[i];
            if (!tile->IsInvisible() && !tile->IsFilled()) {
                //uint16_t imgSetIndex = tile->GetID() >> 16;
                uint16_t id = tile->GetID();
                hgeSprite *spr = hTileset->GetTile(MDI->GetActiveDoc()->hTileClipboardImageSet,
                                                   id)->GetImage();
                spr->SetColor(0xFFFFFFFF);
                spr->SetFlip(0, 0);
                spr->SetHotSpot(0, 0);
                spr->RenderStretch(px + tileW * x, py + tileH * y, px + tileW * (x + 1), py + tileH * (y + 1));
            }
        }
}

void State::EditingWW::RenderObjectClipboardPreview() {
    if (MDI->GetActiveDoc() == NULL) return;
    if (vObjectClipboard.empty()) {
        int len = GV->fntMyriad16->GetStringWidth(GETL2S("ClipboardPreview", "ObjectClipboardEmpty"));
        RenderCloudTip(10, hge->System_GetState(HGE_SCREENHEIGHT) - 70, len + 20, 20, butMicroObjectCB->getX() + 12,
                       butMicroObjectCB->getY() - 3);
        GV->fntMyriad16->Render(20, hge->System_GetState(HGE_SCREENHEIGHT) - 70, HGETEXT_LEFT,
                                GETL2S("ClipboardPreview", "ObjectClipboardEmpty"), 0);
        return;
    }
    if (vObjectClipboard.size() == 1) {
        hgeSprite *spr = SprBank->GetObjectSprite(vObjectClipboard[0]);
        spr->SetColor(0xFFFFFFFF);
        spr->SetFlip(vObjectClipboard[0]->GetFlipX(), vObjectClipboard[0]->GetFlipY(), true);
        int sizeX = spr->GetWidth();
        int sizeY = spr->GetHeight();
        int cx = 20, cy = hge->System_GetState(HGE_SCREENHEIGHT) - sizeY - 70;
        RenderCloudTip(10, cy - 10, sizeX + 20, sizeY + 20, butMicroObjectCB->getX() + 12, butMicroObjectCB->getY() - 3);
        float hX = 0, hY = 0;
        SprBank->GetObjectSprite(vObjectClipboard[0])->GetHotSpot(&hX, &hY);
        spr->Render(cx + hX, cy + hY);

        return;
    }
    int sizeX = 0, sizeY = 0;
    WWD::Object *horBase = NULL, *verBase = NULL;
    {
        int minX = -1, minY = -1, maxX = -1, maxY = -1;
        for (auto & object : vObjectClipboard) {
            hgeSprite *spr = SprBank->GetObjectSprite(object);
            int minValX = object->GetParam(WWD::Param_LocationX) - spr->GetWidth() / 2,
                minValY = object->GetParam(WWD::Param_LocationY) - spr->GetHeight() / 2;
            if (minValX < minX || minX == -1) horBase = object;
            if (minValY < minY || minY == -1) verBase = object;
            minX = minX == -1 ? minValX : std::min(minX, minValX);
            minY = minY == -1 ? minValY : std::min(minY, minValY);
            maxX = maxX == -1 ? object->GetParam(WWD::Param_LocationX) + spr->GetWidth() / 2 : std::max(
                    maxX, int(object->GetParam(WWD::Param_LocationX) + spr->GetWidth() / 2));
            maxY = maxY == -1 ? object->GetParam(WWD::Param_LocationY) + spr->GetHeight() / 2: std::max(
                    maxY, int(object->GetParam(WWD::Param_LocationY) + spr->GetHeight() / 2));
        }
        sizeX = maxX - minX;
        sizeY = maxY - minY;
    }
    float fScale = 1.0f;
    if (sizeX > 400 || sizeY > 400) {
        fScale = 400.0f / float(sizeX > sizeY ? sizeX : sizeY);
        fScale = 400.0f / float(sizeX > sizeY ? sizeX : sizeY);
    }
    sizeX *= fScale;
    sizeY *= fScale;
    int cx = 20, cy = hge->System_GetState(HGE_SCREENHEIGHT) - sizeY - 70;
    RenderCloudTip(10, cy - 10, sizeX + 20, sizeY + 20, butMicroObjectCB->getX() + 12, butMicroObjectCB->getY() - 3);
    int baseX = horBase->GetParam(WWD::Param_LocationX) - SprBank->GetObjectSprite(horBase)->GetWidth() / 2,
        baseY = verBase->GetParam(WWD::Param_LocationY) - SprBank->GetObjectSprite(verBase)->GetHeight() / 2;
    for (auto & object : vObjectClipboard) {
        hgeSprite *spr = SprBank->GetObjectSprite(object);
        int offX = object->GetParam(WWD::Param_LocationX) - baseX,
            offY = object->GetParam(WWD::Param_LocationY) - baseY;
        spr->SetColor(0xFFFFFFFF);
        spr->SetFlip(object->GetFlipX(), object->GetFlipY(), true);
        spr->RenderEx(cx + offX * fScale, cy + offY * fScale, 0, fScale);
    }
}

void State::EditingWW::DrawWelcomeScreen() {
    int dx, dy;
    winWelcome->getAbsolutePosition(dx, dy);
    hge->Gfx_SetClipping();



    static const char *welcome = GETL2S("HomeScreen", "Welcome"),
                      *whatAreWeDoing = GETL2S("HomeScreen", "WhatAreWeDoing");

    GV->fntMyriad20->Render(dx, dy, HGETEXT_LEFT, welcome, true);
    dy += 30;
    GV->fntMyriad16->Render(dx, dy, HGETEXT_LEFT, whatAreWeDoing, true);
    dy += 40;
    hge->Gfx_RenderLine(dx, dy, dx + winWelcome->getWidth() - 1, dy, GV->colLineBright);
    dy += 120;
    hge->Gfx_RenderLine(dx, dy, dx + winWelcome->getWidth() - 1, dy, GV->colLineBright);
}

void State::EditingWW::RenderAreaRect(WWD::Rect r, bool bFill, DWORD hwCol) {
    GV->sprDottedLineCorner->SetColor(hwCol);
    GV->sprDottedLineHorizontal->SetColor(hwCol);
    GV->sprDottedLineVertical->SetColor(hwCol);

    int ox, oy, ow, oh;
    hge->Gfx_GetClipping(&ox, &oy, &ow, &oh);

    if (r.x1 > r.x2) std::swap(r.x1, r.x2);
    if (r.y1 > r.y2) std::swap(r.y1, r.y2);

    int cx = std::max(ox, r.x1),
        cy = std::max(oy, r.y1),
        cx2 = r.x2 ? std::min(ox + ow, r.x2) : ox + ow,
        cy2 = r.y2 ? std::min(oy + oh, r.y2) : oy + oh;

    if (cx2 - cx < 4) {
        cx2 = cx + 4;
    }
    if (cy2 - cy < 4) {
        cy2 = cy + 4;
    }

    bool tl = true, tr = true, bl = true, br = true;

    int increment = 16;

    if (!r.x1) {
        r.x1 -= int(fCamX * fZoom) % increment;
        tl = bl = false;
    }

    if (!r.y1) {
        r.y1 -= int(fCamY * fZoom) % increment;
        tl = tr = false;
    }

    if (!r.x2) {
        r.x2 = cx2 + increment;
        tr = br = false;
    }

    if (!r.y2) {
        r.y2 = cy2 + increment;
        bl = br = false;
    }

    if (bFill) {
        SHR::SetQuad(&q, SETA(hwCol, 0x20), r.x1, r.y1, r.x2, r.y2);
        hge->Gfx_RenderQuad(&q);
    }

    int x = r.x1, y = r.y1, ex = r.x2 - (tr || br) * increment, ey = r.y2 - (bl || br) * increment;

    hge->Gfx_SetClipping(cx, cy, cx2 - cx, cy2 - cy);

    if (tl) {
        GV->sprDottedLineCorner->SetFlip(false, false);
        GV->sprDottedLineCorner->Render(x, y);
        x += 14;
        y += 14;
    }

    if (tr) {
        hge->Gfx_SetClipping(x - 6, cy, std::min(ex - cx + 16, cx2 - cx), cy2 - cy);
        GV->sprDottedLineCorner->SetFlip(true, false);
        GV->sprDottedLineCorner->Render(ex + 4, r.y1);
    }

    if (bl) {
        hge->Gfx_SetClipping(cx, y - 6, cx2 - cx, std::max(ey - cy + 16, cy2 - cy));
        GV->sprDottedLineCorner->SetFlip(false, true);
        GV->sprDottedLineCorner->Render(r.x1, ey + 4);
    }

    if (br) {
        hge->Gfx_SetClipping(x - 6, y - 6, std::min(ex - cx + 16, cx2 - cx), std::max(ey - cy + 16, cy2 - cy));
        GV->sprDottedLineCorner->SetFlip(true, true);
        GV->sprDottedLineCorner->Render(ex + 4, ey + 4);
    }

    hge->Gfx_SetClipping(cx, cy, cx2 - cx, std::min(ey - cy + 6, cy2 - cy));
    ey += 4;

    for (; y < ey; y += increment) {
        GV->sprDottedLineVertical->Render(r.x1, y);
        GV->sprDottedLineVertical->Render(r.x2 - 4, y);
    }

    hge->Gfx_SetClipping(cx, cy, std::min(ex - cx + 6, cx2 - cx), cy2 - cy);
    ex += 4;

    for (; x < ex; x += increment) {
        GV->sprDottedLineHorizontal->Render(x, r.y1);
        GV->sprDottedLineHorizontal->Render(x, r.y2 - 4);
    }

    hge->Gfx_SetClipping(ox, oy, ow, oh);
}

void State::EditingWW::DrawCrashRetrieve() {
    int dx, dy;
    conCrashRetrieve->getAbsolutePosition(dx, dy);
    GV->fntMyriad16->SetColor(0xFFe1e1e1);
    GV->fntMyriad16->Render(dx, dy, HGETEXT_LEFT, GETL2S("WinCrashRetrieve", "Label"), 0);
    int c = 0;
    for (int i = 0; i < 10; i++) {
        if (szCrashRetrieve[i] == NULL) break;
        if (iCrashRetrieveIcon[i] < 50)
            GV->sprGamesSmall[iCrashRetrieveIcon[i]]->Render(dx, dy + 30 + 25 * i);
        else
            GV->sprLevelsMicro16[iCrashRetrieveIcon[i] - 51]->Render(dx, dy + 30 + 25 * i);
        GV->fntMyriad16->Render(dx + 22, dy + 30 + 25 * i, HGETEXT_LEFT, szCrashRetrieve[i], 0);
        c++;
    }
    if (szCrashRetrieveMore != NULL)
        GV->fntMyriad16->Render(dx, dy + 30 + 25 * c, HGETEXT_LEFT, szCrashRetrieveMore, 0);
}

void State::EditingWW::DrawTilePicker() {
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    bool hasMouse = conMain->getWidgetAt(mx, my) == winTilePicker;
    int dx, dy;
    winTilePicker->getAbsolutePosition(dx, dy);

    hge->Gfx_RenderLine(dx, dy + 68, dx + winTilePicker->getWidth(), dy + 68, GV->colLineDark);
    hge->Gfx_RenderLine(dx, dy + 69, dx + winTilePicker->getWidth(), dy + 69, GV->colLineBright);

    hge->Gfx_RenderLine(dx, dy + iTilePickerOffUp + 13, dx + winTilePicker->getWidth(), dy + iTilePickerOffUp + 13,
                        GV->colLineDark);
    hge->Gfx_RenderLine(dx, dy + iTilePickerOffUp + 14, dx + winTilePicker->getWidth(), dy + iTilePickerOffUp + 14,
                        GV->colLineBright);

    hge->Gfx_RenderLine(dx, dy + winTilePicker->getHeight() - iTilePickerOffDown + 20,
                        dx + winTilePicker->getWidth(), dy + winTilePicker->getHeight() - iTilePickerOffDown + 20,
                        GV->colLineDark);
    hge->Gfx_RenderLine(dx, dy + winTilePicker->getHeight() - iTilePickerOffDown + 21,
                        dx + winTilePicker->getWidth(), dy + winTilePicker->getHeight() - iTilePickerOffDown + 21,
                        GV->colLineBright);

    cTileImageSet *set = hTileset->GetSet(GetActivePlane()->GetImageSet(0));
    if (set) {
        if (iActiveTool == EWW_TOOL_BRUSH && iTilePicked >= 0 && set->GetBrushByIterator(iTilePicked)->HasSettings()) {
            hge->Gfx_RenderLine(dx, dy + winTilePicker->getHeight() - 40,
                                dx + winTilePicker->getWidth(), dy + winTilePicker->getHeight() - 40,
                                GV->colLineDark);
            hge->Gfx_RenderLine(dx, dy + winTilePicker->getHeight() - 39,
                                dx + winTilePicker->getWidth(), dy + winTilePicker->getHeight() - 39,
                                GV->colLineBright);
        }

        int itpx = dx + 4, itpy = dy + iTilePickerOffUp + 16;
        hge->Gfx_SetClipping(itpx, itpy, 230, slitpiPicker->getHeight());

        if (mx > itpx && mx < itpx + 230 && my > itpy && my < itpy + slitpiPicker->getHeight() && slitpiPicker->isEnabled()) {
            slitpiPicker->setValue(slitpiPicker->getValue() + 100 * hge->Input_GetMouseWheel());
        }

        int scroll = slitpiPicker->getScaleEnd() - slitpiPicker->getValue();
        if (iActiveTool == EWW_TOOL_BRUSH) {
            int ioldsel = iTilePicked;
            int loopst = scroll / 57, loopen = slitpiPicker->getHeight() / 57 + 2;
            if (set->GetBrushesCount() == 0) {
                GV->fntMyriad16->SetColor(0xFFe1e1e1);
                GV->fntMyriad16->printf(itpx + 115, itpy + slitpiPicker->getHeight() / 2, HGETEXT_CENTER,
                                        GETL(Lang_NoBrushes), 0);
            }
            for (int i = 0; i < set->GetBrushesCount() && i < (loopst + loopen); i++) {
                int drx = itpx + 2 + 12, dry = itpy + 4 + i * 57 - scroll;
                cBrush *brush = set->GetBrushByIterator(i);
                hgeSprite *spr = NULL;
                if (brush->GetIcon() == BRUSHICON_INVALID) {
                    spr = GV->gcnParts.sprIconError;
                } else {
                    cTile *tile = set->GetTile(brush->GetIcon());
                    if (tile == NULL) {
                        spr = GV->sprSmiley;
                    } else {
                        spr = tile->GetImage();
                        spr->SetColor(0xFFFFFFFF);
                        spr->SetFlip(0, 0);
                        spr->SetHotSpot(0, 0);
                    }
                }
                spr->RenderStretch(drx, dry, drx + 48, dry + 48);
                hge->Gfx_RenderLine(drx - 2, dry - 1, drx + 50, dry - 1, GV->colLineDark);
                hge->Gfx_RenderLine(drx - 1, dry + 50, drx + 50, dry + 50, GV->colLineDark);
                hge->Gfx_RenderLine(drx - 1, dry - 1, drx - 1, dry + 50, GV->colLineDark);
                hge->Gfx_RenderLine(drx + 50, dry - 1, drx + 50, dry + 50, GV->colLineDark);
                DWORD colBorder = GV->colLineBright;
                if (hasMouse &&
                    (mx > drx && my > dry && mx < drx + 48 && my < dry + 48 ||
                     mx > drx + 55 && mx < drx + 55 + GV->fntMyriad16->GetStringWidth(brush->GetName()) &&
                     my > dry + 14 && my < dry + 34)
                    && my > itpy && my < itpy + slitpiPicker->getHeight()) {
                    colBorder = TILE_HIGHLIGHT_COLOR;
                    if (hge->Input_KeyDown(HGEK_LBUTTON))
                        iTilePicked = i;
                } else if (i == iTilePicked) {
                    colBorder = TILE_PICK_COLOR;
                }
                hge->Gfx_RenderLine(drx - 1, dry, drx + 49, dry, colBorder);
                hge->Gfx_RenderLine(drx, dry + 49, drx + 49, dry + 49, colBorder);
                hge->Gfx_RenderLine(drx, dry, drx, dry + 49, colBorder);
                hge->Gfx_RenderLine(drx + 49, dry, drx + 49, dry + 49, colBorder);

                GV->fntMyriad16->SetColor(colBorder == GV->colLineBright ? 0xFFe1e1e1 : colBorder);
                GV->fntMyriad16->Render(drx + 55, dry + 14, HGETEXT_LEFT, brush->GetName(), 0);
            }
            if (ioldsel != iTilePicked) {
                HandleBrushSwitch(ioldsel, iTilePicked);
            }
        } else {
            int loopst = scroll / 57 * 4, loopen = slitpiPicker->getHeight() / 57 * 4 + 8;
            if (loopst < 0) loopst = 0;
            for (int i = -3 + loopst; i < set->GetTilesCount() && i < (-2 + loopst + loopen); i++) {
                int idx = (i + 3) % 4, idy = (i + 3) / 4;
                int drx = itpx + 2 + idx * 57, dry = itpy + 4 + idy * 57 - scroll + 10;
                if (i == -3) {
                    GV->sprIcons[Icon_Pipette]->Render(drx + 8, dry + 8);
                } else if (i == -2) {
                    GV->sprIcons[Icon_Erase]->Render(drx + 8, dry + 8);
                } else if (i == -1) {
                    SHR::SetQuad(&q, hDataCtrl->GetPalette()->GetColor(GetActivePlane()->GetFillColor()), drx, dry,
                                 drx + 48, dry + 48);
                    hge->Gfx_RenderQuad(&q);
                } else {
                    cTile *tile = set->GetTileByIterator(i);
                    tile->GetImage()->SetHotSpot(0, 0);
                    tile->GetImage()->SetFlip(0, 0);
                    tile->GetImage()->SetColor(0xFFFFFFFF);
                    float scaleX = 48.f / GV->editState->GetActivePlane()->GetTileWidth(),
                          scaleY = 48.f / GV->editState->GetActivePlane()->GetTileHeight();
                    tile->GetImage()->RenderEx(drx, dry, 0, scaleX, scaleY);
                    if (cbtpiShowProperties->isSelected()) {
                        DrawTileAttributes(tile->GetID(), drx, dry, scaleX, scaleY);
                    }
                    if (cbtpiShowTileID->isSelected()) {
                        GV->fntMyriad16->SetColor(0xFF000000);
                        GV->fntMyriad16->printf(drx + 24 + 1, dry + 1 + 32, HGETEXT_CENTER, "%d", 0, tile->GetID());
                        GV->fntMyriad16->SetColor(0xFFFFFFFF);
                        GV->fntMyriad16->printf(drx + 24, dry + 32, HGETEXT_CENTER, "%d", 0, tile->GetID());
                    }
                }
                hge->Gfx_RenderLine(drx - 2, dry - 1, drx + 50, dry - 1, GV->colLineDark);
                hge->Gfx_RenderLine(drx - 1, dry + 50, drx + 50, dry + 50, GV->colLineDark);
                hge->Gfx_RenderLine(drx - 1, dry - 1, drx - 1, dry + 50, GV->colLineDark);
                hge->Gfx_RenderLine(drx + 50, dry - 1, drx + 50, dry + 50, GV->colLineDark);
                DWORD colBorder = GV->colLineBright;
                if (iTilePicked == EWW_TILE_PIPETTE && iPipetteTileHL != EWW_TILE_NONE)
                    if (iPipetteTileHL == EWW_TILE_ERASE && i == -2) colBorder = TILE_HIGHLIGHT_COLOR;
                    else if (iPipetteTileHL == EWW_TILE_FILL && i == -1) colBorder = TILE_HIGHLIGHT_COLOR;
                    else if (iPipetteTileHL == i) colBorder = TILE_HIGHLIGHT_COLOR;
                if (mx > drx && my > dry && mx < drx + 48 && my < dry + 48 && hasMouse && my > itpy &&
                    my < itpy + slitpiPicker->getHeight()) {
                    colBorder = TILE_HIGHLIGHT_COLOR;
                    if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                        iTilePicked = (i < 0 ? i - 1 : i);
                        if (i == -1) {
                            wintpFillColor->setVisible(1);
                        }
                    }
                } else if (i < 0 && i - 1 == iTilePicked || i >= 0 && i == iTilePicked ||
                           iTilePicked == EWW_TILE_PIPETTE && i == -3) {
                    colBorder = TILE_PICK_COLOR;
                }
                hge->Gfx_RenderLine(drx - 1, dry, drx + 49, dry, colBorder);
                hge->Gfx_RenderLine(drx, dry + 49, drx + 49, dry + 49, colBorder);
                hge->Gfx_RenderLine(drx, dry, drx, dry + 49, colBorder);
                hge->Gfx_RenderLine(drx + 49, dry, drx + 49, dry + 49, colBorder);
            }
        }
    }

}
