#include "../editing_ww.h"

#include "../loadmap.h"
#include "../../globals.h"
#include "../../version.h"
#include "../../../shared/commonFunc.h"
#include "../../langID.h"
#include "../../returncodes.h"
#include "../../cObjectUserData.h"
#include "../error.h"
#include <math.h>
#include <hgevector.h>
#include "../../objEdit/editElevPath.h"
#include "../../objEdit/editWarp.h"
#include "../../objEdit/editCrate.h"
#include "../../objEdit/editStatue.h"
#include "../../objEdit/editSpringboard.h"
#include "../../objEdit/editDialog.h"
#include "../../cAppMenu.h"
#include "../../cInterfaceSheet.h"
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
    if (GetUserDataFromObj(a)->GetZ() == GetUserDataFromObj(b)->GetZ())
        return (a->GetParam(WWD::Param_ID) < b->GetParam(WWD::Param_ID));
    return (GetUserDataFromObj(a)->GetZ() < GetUserDataFromObj(b)->GetZ());
}

void State::EditingWW::DrawTileAtrib(WWD::TileAtrib *atrib, float posx, float posy, float width, float height) {
	if (!atrib) return;
    float wid = 64.0f * width;
    float hei = 64.0f * height;
    if (atrib->GetType() == WWD::AtribType_Single) {
        if (atrib->GetAtribInside() == WWD::Atrib_Clear) return;
        hgeQuad q;
        q.blend = BLEND_DEFAULT;
        q.tex = 0;

        if (atrib->GetAtribInside() == WWD::Atrib_Climb)
            q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_CLIMB;
        else if (atrib->GetAtribInside() == WWD::Atrib_Death)
            q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_DEATH;
        else if (atrib->GetAtribInside() == WWD::Atrib_Ground)
            q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_GROUND;
        else if (atrib->GetAtribInside() == WWD::Atrib_Solid)
            q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_SOLID;

        q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;
        q.v[0].x = posx;
        q.v[0].y = posy;
        q.v[1].x = q.v[0].x + wid;
        q.v[1].y = q.v[0].y;
        q.v[2].x = q.v[1].x;
        q.v[2].y = q.v[0].y + hei;
        q.v[3].x = q.v[0].x;
        q.v[3].y = q.v[2].y;
        hge->Gfx_RenderQuad(&q);
    } else {
        if (atrib->GetAtribInside() == WWD::Atrib_Clear && atrib->GetAtribOutside() == WWD::Atrib_Clear) return;

        hgeQuad q;
        q.blend = BLEND_DEFAULT;
        q.tex = 0;
        WWD::Rect mask = atrib->GetMask();
        q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;

        if (atrib->GetAtribInside() != WWD::Atrib_Clear) {
            if (atrib->GetAtribInside() == WWD::Atrib_Climb)
                q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_CLIMB;
            else if (atrib->GetAtribInside() == WWD::Atrib_Death)
                q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_DEATH;
            else if (atrib->GetAtribInside() == WWD::Atrib_Ground)
                q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_GROUND;
            else if (atrib->GetAtribInside() == WWD::Atrib_Solid)
                q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_SOLID;

            q.v[0].x = posx + mask.x1 * width;
            q.v[0].y = posy + mask.y1 * height;
            q.v[1].x = posx + (mask.x2 + 1) * width;
            q.v[1].y = q.v[0].y;
            q.v[2].x = q.v[1].x;
            q.v[2].y = posy + (mask.y2 + 1) * height;
            q.v[3].x = q.v[0].x;
            q.v[3].y = q.v[2].y;
            hge->Gfx_RenderQuad(&q);
        }
        if (atrib->GetAtribOutside() != WWD::Atrib_Clear) {
            if (atrib->GetAtribOutside() == WWD::Atrib_Climb)
                q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_CLIMB;
            else if (atrib->GetAtribOutside() == WWD::Atrib_Death)
                q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_DEATH;
            else if (atrib->GetAtribOutside() == WWD::Atrib_Ground)
                q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_GROUND;
            else if (atrib->GetAtribOutside() == WWD::Atrib_Solid)
                q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = COLOR_SOLID;

            //upper
            q.v[0].x = posx;
            q.v[0].y = posy;
            q.v[1].x = q.v[0].x + wid;
            q.v[1].y = q.v[0].y;
            q.v[2].x = q.v[1].x;
            q.v[2].y = posy + mask.y1 * height;
            q.v[3].x = q.v[0].x;
            q.v[3].y = q.v[2].y;
            hge->Gfx_RenderQuad(&q);
            //lower
            q.v[0].x = posx;
            q.v[0].y = posy + (mask.y2 + 1) * height;
            q.v[1].x = q.v[0].x + wid;
            q.v[1].y = q.v[0].y;
            q.v[2].x = q.v[1].x;
            q.v[2].y = posy + hei;
            q.v[3].x = q.v[0].x;
            q.v[3].y = q.v[2].y;
            hge->Gfx_RenderQuad(&q);
            //left
            q.v[0].x = posx;
            q.v[0].y = posy + mask.y1 * height;
            q.v[1].x = posx + mask.x1 * width;
            q.v[1].y = q.v[0].y;
            q.v[2].x = q.v[1].x;
            q.v[2].y = posy + (mask.y2 + 1) * height;
            q.v[3].x = q.v[0].x;
            q.v[3].y = q.v[2].y;
            hge->Gfx_RenderQuad(&q);
            //right
            q.v[0].x = posx + (mask.x2 + 1) * width;
            q.v[0].y = posy + mask.y1 * height;
            q.v[1].x = posx + wid;
            q.v[1].y = q.v[0].y;
            q.v[2].x = q.v[1].x;
            q.v[2].y = posy + (mask.y2 + 1) * height;
            q.v[3].x = q.v[0].x;
            q.v[3].y = q.v[2].y;
            hge->Gfx_RenderQuad(&q);
        }
    }
}

void State::EditingWW::DrawTileAtrib(int tileid, float posx, float posy, float width, float height) {
    DrawTileAtrib(hParser->GetTileAtribs(tileid), posx, posy, width, height);
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

            GV->fntMyriad13->printf(dx + 650, dy + 99, HGETEXT_RIGHT, "~y~%d", 0, dbFramesImg->getSelected());
            GV->fntMyriad13->printf(dx + 650, dy + 119, HGETEXT_RIGHT, "~y~%d", 0, img->GetID());
            /*GV->fntMyriad13->printf(dx+650, dy+139, HGETEXT_RIGHT, "~y~%d", 0, img->GetFileID());
			char * fl = SHR::FormatSize(img->GetFileSize());
			GV->fntMyriad13->printf(dx+650, dy+159, HGETEXT_RIGHT, "~y~%s", 0, fl);
			delete [] fl;
			GV->fntMyriad13->printf(dx+650, dy+179, HGETEXT_RIGHT, "~y~%d~l~x~y~%d", 0, img->GetFileDimX(), img->GetFileDimY());
			GV->fntMyriad13->printf(dx+650, dy+199, HGETEXT_RIGHT, "~y~%d~l~x~y~%d", 0, img->GetFileOffsetX(), img->GetFileOffsetY());
			GV->fntMyriad13->printf(dx+570, dy+239, HGETEXT_RIGHT, "~y~%d", 0, img->GetFileUserData(0));
			GV->fntMyriad13->printf(dx+650, dy+239, HGETEXT_RIGHT, "~y~%d", 0, img->GetFileUserData(1));
			GV->fntMyriad13->SetColor(0xFF000000);
			GV->fntMyriad13->Render(dx+790, dy+80, HGETEXT_RIGHT, img->GetFileHash().c_str());*/

            /*int flagn = 1;
			for(int i=0;i<8;i++){
			 char ident[32];
			 sprintf(ident, "Flag_%d", i+1);
			 GV->fntMyriad13->Render(dx+676, dy+99+i*20, HGETEXT_LEFT, GETL2S("WinDatabase", ident));
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
        GV->fntMyriad13->printf(dx + 700, dy + 100, HGETEXT_CENTER, GETL(Lang_SelectResourceFromList), 0);
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

        GV->fntMyriad13->printf(dx + 780, dy + 220, HGETEXT_RIGHT, "%d/%dms (%d)", 0,
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

    GV->fntMyriad13->Render(dx + 145, dy + 68, HGETEXT_LEFT, GETL(Lang_Width), 0);
    GV->fntMyriad13->Render(dx + 145, dy + 94, HGETEXT_LEFT, GETL(Lang_Height), 0);

    GV->fntMyriad13->Render(dx + 145, dy + 110, HGETEXT_LEFT, GETL(Lang_Type), 0);
    GV->fntMyriad13->Render(dx + 165, dy + 136, HGETEXT_LEFT, GETL(Lang_AtribSingle), 0);
    GV->fntMyriad13->Render(dx + 165, dy + 163, HGETEXT_LEFT, GETL(Lang_AtribDouble), 0);

    GV->fntMyriad13->Render(dx + 5, dy + 205, HGETEXT_LEFT, GETL(Lang_AtribInside), 0);

    GV->fntMyriad13->Render(dx + 125, dy + 205, HGETEXT_LEFT, GETL(Lang_PropClear), 0);
    GV->fntMyriad13->Render(dx + 225, dy + 205, HGETEXT_LEFT, GETL(Lang_PropSolid), 0);

    GV->fntMyriad13->Render(dx + 25, dy + 235, HGETEXT_LEFT, GETL(Lang_PropGround), 0);
    GV->fntMyriad13->Render(dx + 125, dy + 235, HGETEXT_LEFT, GETL(Lang_PropClimb), 0);
    GV->fntMyriad13->Render(dx + 225, dy + 235, HGETEXT_LEFT, GETL(Lang_PropDeath), 0);

    if (rbtpDouble->isSelected()) {
        GV->fntMyriad13->Render(dx + 125, dy + 265, HGETEXT_LEFT, GETL(Lang_PropClear), 0);
        GV->fntMyriad13->Render(dx + 225, dy + 265, HGETEXT_LEFT, GETL(Lang_PropSolid), 0);

        GV->fntMyriad13->Render(dx + 25, dy + 295, HGETEXT_LEFT, GETL(Lang_PropGround), 0);
        GV->fntMyriad13->Render(dx + 125, dy + 295, HGETEXT_LEFT, GETL(Lang_PropClimb), 0);
        GV->fntMyriad13->Render(dx + 225, dy + 295, HGETEXT_LEFT, GETL(Lang_PropDeath), 0);

        GV->fntMyriad13->Render(dx + 5, dy + 265, HGETEXT_LEFT, GETL(Lang_AtribOutside), 0);
        GV->fntMyriad13->Render(dx + 110, dy + 327, HGETEXT_RIGHT, GETL(Lang_Mask), 0);
        GV->fntMyriad13->Render(dx + 150, dy + 327, HGETEXT_LEFT, "x", 0);
        GV->fntMyriad13->Render(dx + 202, dy + 325, HGETEXT_LEFT, "-", 0);
        GV->fntMyriad13->Render(dx + 250, dy + 327, HGETEXT_LEFT, "x", 0);
    }

    cTile *tile = hTileset->GetTile("ACTION", itpSelectedTile);
    if (tile != NULL) {
        if (rbtpDouble->isSelected()) {
            float mx, my;
            hge->Input_GetMousePos(&mx, &my);
            if (btpDragDropMask && !hge->Input_GetKeyState(HGEK_LBUTTON)) {
                btpDragDropMask = 0;
            } else if (!btpDragDropMask && hge->Input_KeyDown(HGEK_LBUTTON) &&
                       mx > dx + 10 + (!btpZoomTile * 32) && mx < dx + 10 + 64 + 32 + btpZoomTile * 32 &&
                       my > dy + 65 + (!btpZoomTile * 32) && my < dy + 65 + 64 + 32 + btpZoomTile * 32) {
                btpDragDropMask = 1;
                if (btpZoomTile) {
                    itpDDx1 = (mx - dx - 10) / 2;
                    itpDDy1 = (my - dy - 65) / 2;
                } else {
                    itpDDx1 = mx - dx - 10 - 32;
                    itpDDy1 = my - dy - 65 - 32;
                }
                char tmp[16];
                sprintf(tmp, "%d", itpDDx1);
                tftpX1->setText(tmp);
                sprintf(tmp, "%d", itpDDy1);
                tftpY1->setText(tmp);
            } else if (btpDragDropMask) {
                int nx, ny;
                if (btpZoomTile) {
                    nx = (mx - dx - 10) / 2;
                    ny = (my - dy - 65) / 2;
                } else {
                    nx = mx - dx - 10 - 32;
                    ny = my - dy - 65 - 32;
                }
                if (nx >= 64) nx = 63;
                if (ny >= 64) ny = 63;
                if (nx < 0) nx = 0;
                if (ny < 0) ny = 0;
                htpWorkingAtrib->SetMask(std::min(itpDDx1, nx), std::min(itpDDy1, ny), std::max(itpDDx1, nx),
                                         std::max(itpDDy1, ny));
                char tmp[16];
                sprintf(tmp, "%d", std::min(itpDDx1, nx));
                tftpX1->setText(tmp);
                sprintf(tmp, "%d", std::min(itpDDy1, ny));
                tftpY1->setText(tmp);

                sprintf(tmp, "%d", std::max(itpDDx1, nx));
                tftpX2->setText(tmp);
                sprintf(tmp, "%d", std::max(itpDDy1, ny));
                tftpY2->setText(tmp);
            }
        }

        tile->GetImage()->SetColor(0xffffffff);
        tile->GetImage()->SetFlip(0, 0);
        tile->GetImage()->SetHotSpot(0, 0);
        if (btpZoomTile) {
            tile->GetImage()->RenderEx(dx + 10, dy + 65, 0, 2.0f);
            DrawTileAtrib(htpWorkingAtrib, dx + 10, dy + 65, 2.0f, 2.0f);
        } else {
            tile->GetImage()->Render(dx + 10 + 32, dy + 65 + 32);
            DrawTileAtrib(htpWorkingAtrib, dx + 10 + 32, dy + 65 + 32, 1.0f, 1.0f);
        }
    } else {

    }
}

int State::EditingWW::RenderPlane(WWD::Plane *plane, int pl) {
    int rcount = 0;
    int iCamX = fCamX, iCamY = fCamY;
    int sx = iCamX * (plane->GetMoveModX() / 100.0f);
    int sy = iCamY * (plane->GetMoveModY() / 100.0f);
    sx = sx * fZoom / (plane->GetTileWidth() * fZoom);
    sy = sy * fZoom / (plane->GetTileHeight() * fZoom);
    int ex = sx + vPort->GetWidth() / (plane->GetTileWidth() * fZoom) + 2;
    int ey = sy + vPort->GetHeight() / (plane->GetTileHeight() * fZoom) + 2;
    int enwx = ex, enwy = ey;

    if (sx < 0) sx = 0;
    if (sy < 0) sy = 0;

    if (ex > plane->GetPlaneWidth() && (plane->GetFlags() & WWD::Flag_p_XWrapping) == 0) {
        ex = plane->GetPlaneWidth();
    }

    if (ey > plane->GetPlaneHeight() && (plane->GetFlags() & WWD::Flag_p_YWrapping) == 0) {
        ey = plane->GetPlaneHeight();
    }

    DWORD col = 0xFFFFFFFF;
    if (GV->bAlphaHigherPlanes && plane->GetZCoord() > GetActivePlane()->GetZCoord() ||
        GV->bAlphaHigherPlanes && pl > GetActivePlaneID() && !(hParser->GetFlags() & WWD::Flag_w_UseZCoords)) {
        col = 0x77FFFFFF;
    }

    int cammx = int(iCamX * (plane->GetMoveModX() / 100.0f) * fZoom);
    int cammy = int(iCamY * (plane->GetMoveModY() / 100.0f) * fZoom);
    int cammendx = cammx + vPort->GetWidth() / fZoom;
    int cammendy = cammy + vPort->GetHeight() / fZoom;

    int endplanex = ex * plane->GetTileWidth() * fZoom - cammx;
    int endplaney = ey * plane->GetTileHeight() * fZoom - cammy;

    bool bUsingBuffer = (hPlaneData[pl]->hRB != NULL);
    if (bUsingBuffer) {
        rcount += hPlaneData[pl]->hRB->GetEntityCount(sx, sy, enwx - 1, enwy - 1);
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
    }
    bool ghosting = 0;
    for (size_t i = 0; i < vTileGhosting.size(); i++)
        if (vTileGhosting[i].pl == plane) {
            ghosting = 1;
            break;
        }
    if (!bUsingBuffer || ghosting)
        for (int y = sy; y < ey; y++) {
            float posy = y * plane->GetTileHeight() * fZoom - cammy;
            for (int x = sx; x < ex; x++) {
                float posx = x * plane->GetTileWidth() * fZoom - cammx;
                bool highlight = 0;
                if (!vTileGhosting.empty()) {
                    bool ghosted = 0;
                    for (int i = 0; i < vTileGhosting.size(); i++) {
                        if (x == vTileGhosting[i].x && y == vTileGhosting[i].y && plane == vTileGhosting[i].pl) {
                            if (vTileGhosting[i].id >= 0 && vTileGhosting[i].id != plane->GetTile(x, y)->GetID()) {
                                hgeSprite *spr;
                                spr = hTileset->GetSet(plane->GetImageSet(0))->GetTile(vTileGhosting[i].id)->GetImage();
                                spr->SetColor(SETA(col, 160));
                                spr->SetHotSpot(0, 0);
                                spr->RenderEx(posx, posy, 0, fZoom);
                                ghosted = 1;
                                if (bDrawTileProperties && pl == GetActivePlaneID()) {
                                    DrawTileAtrib(vTileGhosting[i].id, posx, posy,
                                                  1.0f / (64.0f / (plane->GetTileWidth() * fZoom)),
                                                  1.0f / (64.0f / (plane->GetTileHeight() * fZoom)));
                                }
                                break;
                            } else if (vTileGhosting[i].id == EWW_TILE_ERASE && !plane->GetTile(x, y)->IsInvisible()) {
                                ghosted = 1;
                                break;
                            } else if (vTileGhosting[i].id == EWW_TILE_FILL && !plane->GetTile(x, y)->IsFilled()) {
                                hgeQuad q;
                                q.blend = BLEND_DEFAULT;
                                q.tex = 0;
                                q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = SETA(
                                        hDataCtrl->GetPalette()->GetColor(plane->GetFillColor()), GETA(col));
                                q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;
                                q.v[0].x = posx;
                                q.v[0].y = posy;
                                q.v[1].x = q.v[0].x + plane->GetTileWidth() * fZoom;
                                q.v[1].y = q.v[0].y;
                                q.v[2].x = q.v[1].x;
                                q.v[2].y = q.v[0].y + plane->GetTileHeight() * fZoom;
                                q.v[3].x = q.v[0].x;
                                q.v[3].y = q.v[2].y;
                                hge->Gfx_RenderQuad(&q);
                                ghosted = 1;
                                break;
                            } else if (vTileGhosting[i].id == EWW_TILE_PIPETTE)
                                highlight = 1;
                        }
                    }
                    if (ghosted)
                        continue;
                }

                if (!bUsingBuffer)
                    if (plane->GetTile(x, y)->IsInvisible()) {

                    } else if (plane->GetTile(x, y)->IsFilled()) {
                        hgeQuad q;
                        q.blend = BLEND_DEFAULT;
                        q.tex = 0;
                        q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = SETA(
                                hDataCtrl->GetPalette()->GetColor(plane->GetFillColor()), GETA(col));
                        q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;
                        q.v[0].x = posx;
                        q.v[0].y = posy;
                        q.v[1].x = q.v[0].x + plane->GetTileWidth() * fZoom;
                        q.v[1].y = q.v[0].y;
                        q.v[2].x = q.v[1].x;
                        q.v[2].y = q.v[0].y + plane->GetTileHeight() * fZoom;
                        q.v[3].x = q.v[0].x;
                        q.v[3].y = q.v[2].y;
                        hge->Gfx_RenderQuad(&q);
                        rcount++;
                    } else {
                        hgeSprite *spr;
                        cTile *tile = hTileset->GetTile(plane->GetImageSet(0), plane->GetTile(x, y)->GetID());
                        if (!tile) {
                            GV->sprTile->RenderEx(posx, posy, 0, fZoom);
                            char tmp[32];
                            sprintf(tmp, "ID#%d", plane->GetTile(x, y)->GetID());
                            GV->fntMyriad13->SetColor(0x99FFFFFF);
                            GV->fntMyriad13->Render(posx + plane->GetTileWidth() / 2 * fZoom,
                                                    posy + plane->GetTileHeight() * 0.75 * fZoom - 10,
                                                    HGETEXT_CENTER,
                                                    tmp, 0);
                        } else {
                            spr = tile->GetImage();
                            spr->SetColor(col);
                            spr->SetFlip(0, 0);
                            spr->SetHotSpot(0, 0);
                            spr->RenderEx(posx, posy, 0, fZoom);
                        }

                        if (bDrawTileProperties && pl == GetActivePlaneID()) {
                            DrawTileAtrib(plane->GetTile(x, y)->GetID(), posx, posy,
                                          1.0f / (64.0f / (plane->GetTileWidth() * fZoom)),
                                          1.0f / (64.0f / (plane->GetTileHeight() * fZoom)));
                        }
                        rcount++;
                    }

                if (highlight) {
                    int tw = plane->GetTileWidth() * fZoom,
                            th = plane->GetTileHeight() * fZoom;
                    hge->Gfx_RenderLine(posx, posy, posx + tw, posy, GV->colLineDark);
                    hge->Gfx_RenderLine(posx, posy + th, posx + tw, posy + th, GV->colLineDark);
                    hge->Gfx_RenderLine(posx, posy - 1, posx - 1, posy + th, GV->colLineDark);
                    hge->Gfx_RenderLine(posx + tw, posy - 1, posx + tw, posy + th, GV->colLineDark);

                    hge->Gfx_RenderLine(posx, posy + 1, posx + tw - 1, posy, TILE_HIGHLIGHT_COLOR_SOLID);
                    hge->Gfx_RenderLine(posx + 1, posy + th - 1, posx + tw - 1, posy + th - 1,
                                        TILE_HIGHLIGHT_COLOR_SOLID);
                    hge->Gfx_RenderLine(posx + 1, posy + 1, posx + 1, posy + th - 1, TILE_HIGHLIGHT_COLOR_SOLID);
                    hge->Gfx_RenderLine(posx + tw - 1, posy + 1, posx + tw - 1, posy + th - 1,
                                        TILE_HIGHLIGHT_COLOR_SOLID);

                    char text[32];
                    if (plane->GetTile(x, y)->IsInvisible())
                        sprintf(text, "ID: -");
                    else if (plane->GetTile(x, y)->IsFilled())
                        sprintf(text, "ID: FILL");
                    else
                        sprintf(text, "ID: %d", plane->GetTile(x, y)->GetID());
                    GV->fntMyriad13->SetColor(0xFF000000);
                    GV->fntMyriad13->Render(posx + tw / 2 + 1, posy + tw * 0.75 + 1 - 20, HGETEXT_CENTER, text, 0);
                    GV->fntMyriad13->SetColor(0xFFFFFFFF);
                    GV->fntMyriad13->Render(posx + tw / 2, posy + tw * 0.75 - 20, HGETEXT_CENTER, text, 0);
                }

                if (bUsingBuffer) continue;

                if (hPlaneData[pl]->bDrawBoundary &&
                    ((x % plane->GetPlaneWidth()) == plane->GetPlaneWidth() - 1 &&
                     (plane->GetFlags() & WWD::Flag_p_YWrapping) != 0 ||
                     (x % plane->GetPlaneWidth()) == plane->GetPlaneWidth() - 2 &&
                     (plane->GetFlags() & WWD::Flag_p_YWrapping) == 0))
                    hge->Gfx_RenderLine(int(posx + plane->GetTileWidth() * fZoom), 0,
                                        int(posx + plane->GetTileWidth() * fZoom), endplaney,
                                        ARGB(GETA(col), 255, 0, 255));
                else if (hPlaneData[pl]->bDrawGrid)
                    hge->Gfx_RenderLine(int(posx + plane->GetTileWidth() * fZoom), 0,
                                        int(posx + plane->GetTileWidth() * fZoom), endplaney,
                                        ARGB(GETA(col), 255, 255, 255));
            }
            if (bUsingBuffer) continue;
            if (hPlaneData[pl]->bDrawBoundary &&
                ((y % plane->GetPlaneHeight()) == plane->GetPlaneHeight() - 1 &&
                 (plane->GetFlags() & WWD::Flag_p_YWrapping) != 0 ||
                 (y % plane->GetPlaneHeight()) == plane->GetPlaneHeight() - 2 &&
                 (plane->GetFlags() & WWD::Flag_p_YWrapping) == 0))
                hge->Gfx_RenderLine(0, int(posy + plane->GetTileHeight() * fZoom), endplanex,
                                    int(posy + plane->GetTileHeight() * fZoom), ARGB(GETA(col), 255, 0, 255));
            else if (hPlaneData[pl]->bDrawGrid)
                hge->Gfx_RenderLine(0, int(posy + plane->GetTileHeight() * fZoom), endplanex,
                                    int(posy + plane->GetTileHeight() * fZoom), ARGB(GETA(col), 255, 255, 255));
        }

    DrawPlaneOverlay(plane);
    return rcount;
}

void State::EditingWW::RenderToViewportBuffer() {
    //3e3e3e
    if (hParser == NULL) {
        int ox = vPort->GetX() % 128, oy = vPort->GetY() % 128;
        for (int y = 0; y <= vPort->GetHeight() / 128 + 1; y++)
            for (int x = 0; x <= vPort->GetWidth() / 128 + 1; x++) {
                GV->hGfxInterface->sprMainBackground->Render(x * 128 - ox, y * 128 - oy);
            }
        return;
    } else {
        for (int y = 0; y <= vPort->GetHeight() / 120; y++)
            for (int x = 0; x <= vPort->GetWidth() / 120; x++) {
                GV->sprCheckboard->Render(x * 120, y * 120);
            }
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
            for (size_t y = 0; y < list.size(); y++) {
                if (iActiveTool == EWW_TOOL_EDITOBJ && list[y] == hEditObj->GetOrigObj()) continue;
                vRenderOrders.push_back(new cObjectRenderWrapper(list[y]));
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
    hge->Gfx_RenderLine(dx, dy + 105, dx + winWorld->getWidth(), dy + 105, SETA(GV->colLineBright, alpha));
    hge->Gfx_RenderLine(dx, dy + 104, dx + winWorld->getWidth(), dy + 104, SETA(GV->colLineDark, alpha));

    hge->Gfx_RenderLine(dx, dy + 215, dx + winWorld->getWidth(), dy + 215, SETA(GV->colLineBright, alpha));
    hge->Gfx_RenderLine(dx, dy + 214, dx + winWorld->getWidth(), dy + 214, SETA(GV->colLineDark, alpha));

    hge->Gfx_RenderLine(dx + 400, dy + 20, dx + 400, dy + winWorld->getHeight(), SETA(GV->colLineDark, alpha));
    hge->Gfx_RenderLine(dx + 401, dy + 20, dx + 401, dy + winWorld->getHeight(), SETA(GV->colLineBright, alpha));
}

void State::EditingWW::DrawViewport() {
    int iCamX = fCamX, iCamY = fCamY;
    hge->Gfx_SetClipping();
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);

    GV->hGfxInterface->sprMainFrameLU->Render(0, 0);
    hge->Gfx_SetClipping(0, 0, hge->System_GetState(HGE_SCREENWIDTH) - 10, 50);
    for (int x = 10; x < hge->System_GetState(HGE_SCREENWIDTH) - 10; x += 96)
        GV->hGfxInterface->sprMainFrameU->Render(x, 0);
    hge->Gfx_SetClipping();
    GV->hGfxInterface->sprMainFrameRU->Render(hge->System_GetState(HGE_SCREENWIDTH) - 10, 0);

    hge->Gfx_SetClipping(hge->System_GetState(HGE_SCREENWIDTH) - 10, 10, 0,
                         hge->System_GetState(HGE_SCREENHEIGHT) - 26);
    for (int y = 26; y < hge->System_GetState(HGE_SCREENHEIGHT) - 26; y += 26)
        GV->hGfxInterface->sprMainFrameR->Render(hge->System_GetState(HGE_SCREENWIDTH) - 3, y);
    hge->Gfx_SetClipping();

    GV->hGfxInterface->sprMainFrameRD->Render(hge->System_GetState(HGE_SCREENWIDTH) - 26,
                                              hge->System_GetState(HGE_SCREENHEIGHT) - 26);
    GV->hGfxInterface->sprMainFrameLD->Render(0, hge->System_GetState(HGE_SCREENHEIGHT) - 5);
    GV->hGfxInterface->sprMainFrameD->RenderStretch(15, hge->System_GetState(HGE_SCREENHEIGHT) - 3,
                                                    hge->System_GetState(HGE_SCREENWIDTH) - 26,
                                                    hge->System_GetState(HGE_SCREENHEIGHT));

    /*GV->gcnParts.sprGcnWinBarL->RenderStretch(0, 0, 2, 24);
	GV->gcnParts.sprGcnWinBarM->RenderStretch(2, 0, hge->System_GetState(HGE_SCREENWIDTH)-2, 24);
	GV->gcnParts.sprGcnWinBarR->RenderStretch(hge->System_GetState(HGE_SCREENWIDTH)-2, 0, hge->System_GetState(HGE_SCREENWIDTH), 24);*/

    hgeQuad q;
    q.blend = BLEND_DEFAULT;
    q.tex = 0;
    q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;
    q.v[0].x = 1;
    q.v[0].y = LAY_APPMENU_Y;
    q.v[1].x = hge->System_GetState(HGE_SCREENWIDTH) - 4;
    q.v[1].y = LAY_APPMENU_Y;
    q.v[2].x = hge->System_GetState(HGE_SCREENWIDTH) - 4;
    q.v[2].y = LAY_APPMENU_Y + LAY_APPMENU_H + LAY_MODEBAR_H + 2;
    q.v[3].x = 0;
    q.v[3].y = LAY_APPMENU_Y + LAY_APPMENU_H + LAY_MODEBAR_H + 3;
    q.v[0].col = q.v[1].col = 0xFF2b2b2b;
    q.v[2].col = q.v[3].col = 0xFF141414;
    hge->Gfx_RenderQuad(&q);

    GV->sprIconCaption->Render(4, 2);
    GV->hGfxInterface->sprMiniLogo->Render(25, 3);

    for (int i = 0; i < 2; i++) {
        int x = hge->System_GetState(HGE_SCREENWIDTH) - 5 - 20 + (-22 * (i == 0));
        bool mouseover = (mx > x && mx < x + 20 && my > 4 && my < 4 + 19 && conMain->getWidgetAt(mx, my) == NULL);

        if (mouseover && fAppBarTimers[i] < 0.2f) {
            fAppBarTimers[i] += hge->Timer_GetDelta();
            if (fAppBarTimers[i] > 0.2f) fAppBarTimers[i] = 0.2f;
        } else if (!mouseover && fAppBarTimers[i] > 0.0f) {
            fAppBarTimers[i] -= hge->Timer_GetDelta();
            if (fAppBarTimers[i] < 0.0f) fAppBarTimers[i] = 0.0f;
        }

        GV->hGfxInterface->sprAppBar[i]->SetColor(
                SETA(0xFFFFFFFF, (unsigned char) (125.0f + 130.0f * fAppBarTimers[i] * 5.0f)));
        GV->hGfxInterface->sprAppBar[i]->Render(x, 4);
    }

    hge->Gfx_SetClipping(0, 110, hge->System_GetState(HGE_SCREENWIDTH) - 3,
                         hge->System_GetState(HGE_SCREENHEIGHT) - 113);
    for (int y = 0; y <= hge->System_GetState(HGE_SCREENHEIGHT) / 128; y++)
        for (int x = 0; x <= hge->System_GetState(HGE_SCREENWIDTH) / 128; x++) {
            GV->hGfxInterface->sprMainBackground->Render(x * 128, y * 128);
        }
    hge->Gfx_SetClipping();

    MDI->Render();
    hAppMenu->Render();

    //if( !hAppMenu->IsFolded() ){
    //mode bar underlay
    hge->Gfx_RenderLine(1, 50, hge->System_GetState(HGE_SCREENWIDTH) - 4, 50, 0xFF0e0e0e);
    hge->Gfx_RenderLine(1, 51, hge->System_GetState(HGE_SCREENWIDTH) - 4, 51, 0xFF2b2b2b);

    //hge->Gfx_RenderLine(0, LAY_MDI_Y-1, hge->System_GetState(HGE_SCREENWIDTH)-4, LAY_MDI_Y-1, 0xFF1d1d1d);
    //hge->Gfx_RenderLine(0, 41+24+24, hge->System_GetState(HGE_SCREENWIDTH), 41+24+24, GV->colLineBright);

    if (hmbActive != 0)
        for (int i = 0; i < hmbActive->vSeparators.size(); i++) {
            int x = hmbActive->vSeparators[i];
            hge->Gfx_RenderLine(x, 3 + 24 + 23,
                                x, 4 + 24 + 23 + 32,
                                GV->colLineDark);
            hge->Gfx_RenderLine(x + 1, 3 + 23 + 24,
                                x + 1, 4 + 23 + 24 + 32,
                                GV->colLineBright);
        }

    /*hge->Gfx_RenderLine(800+34, 24+24, 800+34, 40+24+24, GV->colLineDark);
	hge->Gfx_RenderLine(801+34, 24+24, 801+34, 40+23+24, GV->colLineBright);*/
    //}

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
                    GV->fntMyriad13->SetColor(0xFF000000);
                    GV->fntMyriad13->printf(mx + 14, my + 16, HGETEXT_LEFT,
                                            (gl.bOrient == GUIDE_HORIZONTAL ? "Y: %d" : "X: %d"), 0, gl.iPos);
                    GV->fntMyriad13->SetColor(0xFFFFFFFF);
                    GV->fntMyriad13->printf(mx + 15, my + 15, HGETEXT_LEFT,
                                            (gl.bOrient == GUIDE_HORIZONTAL ? "Y: %d" : "X: %d"), 0, gl.iPos);
                }
            }
        }

        for (size_t i = 0; i < vObjectsHL.size(); i++) {
            bool fnd = 0;
            for (size_t j = 0; j < vObjectsForbidHL.size(); j++)
                if (vObjectsHL[i] == vObjectsForbidHL[j]) {
                    fnd = 1;
                    break;
                }
            if (fnd) continue;
            WWD::Rect box = SprBank->GetObjectRenderRect(vObjectsHL[i]);
            box.x2 += box.x1;
            box.y2 += box.y1;
            box.x1 = Wrd2ScrX(GetActivePlane(), box.x1);
            box.y1 = Wrd2ScrY(GetActivePlane(), box.y1);
            box.x2 = Wrd2ScrX(GetActivePlane(), box.x2);
            box.y2 = Wrd2ScrY(GetActivePlane(), box.y2);
            hge->Gfx_RenderLine(box.x1, box.y1, box.x2, box.y1, 0xFFFF0000);
            hge->Gfx_RenderLine(box.x2, box.y1, box.x2, box.y2, 0xFFFF0000);
            hge->Gfx_RenderLine(box.x2, box.y2, box.x1, box.y2, 0xFFFF0000);
            hge->Gfx_RenderLine(box.x1, box.y2, box.x1, box.y1, 0xFFFF0000);
        }

        if (iMode == EWW_MODE_OBJECT && iActiveTool == EWW_TOOL_NONE && hge->Input_GetKeyState(HGEK_TAB) &&
            vObjectClipboard.size() != 0) {
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
                spr->SetFlip(GetUserDataFromObj(rend[i])->GetFlipX(), GetUserDataFromObj(rend[i])->GetFlipY(), 1);
                int rx = Wrd2ScrX(GetActivePlane(), GetUserDataFromObj(rend[i])->GetX() + diffx),
                        ry = Wrd2ScrY(GetActivePlane(), GetUserDataFromObj(rend[i])->GetY() + diffy);
                spr->RenderEx(rx, ry, 0, fZoom);
            }
        }

        if (winDuplicate->isVisible() && atoi(tfdTimes->getText().c_str()) > 0) {
            int count = atoi(tfdTimes->getText().c_str());
            int offx = atoi(tfdOffsetX->getText().c_str());
            int offy = atoi(tfdOffsetY->getText().c_str());
            for (int i = 0; i < count; i++) {
                if (objdSource != NULL) {
                    RenderObject(objdSource,
                                 Wrd2ScrX(GetActivePlane(), GetUserDataFromObj(objdSource)->GetX() + offx * (i + 1)),
                                 Wrd2ScrY(GetActivePlane(), GetUserDataFromObj(objdSource)->GetY() + offy * (i + 1)),
                                 0x77FFFFFF);
                } else {
                    for (int y = 0; y < vObjectsPicked.size(); y++) {
                        RenderObject(vObjectsPicked[y], Wrd2ScrX(GetActivePlane(),
                                                                 GetUserDataFromObj(vObjectsPicked[y])->GetX() +
                                                                 offx * (i + 1)),
                                     Wrd2ScrY(GetActivePlane(),
                                              GetUserDataFromObj(vObjectsPicked[y])->GetY() + offy * (i + 1)),
                                     0x77FFFFFF);
                    }
                }
            }
        }

        /*GV->sprThumbtack->SetColor(0xFFFF0000);
		GV->sprThumbtack->Render(vPort->GetX()+vPort->GetWidth()/2-150,
								 vPort->GetY()+vPort->GetHeight()/2);
		GV->sprThumbtack->SetColor(0xFF00FF00);
		GV->sprThumbtack->Render(vPort->GetX()+vPort->GetWidth()/2,
								 vPort->GetY()+vPort->GetHeight()/2);
		GV->sprThumbtack->SetColor(0xFF0000FF);
		GV->sprThumbtack->Render(vPort->GetX()+vPort->GetWidth()/2+150,
								 vPort->GetY()+vPort->GetHeight()/2);*/

        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        for (int i = 0; i < vObjectsPicked.size(); i++) {
            bool fnd = 0;
            for (int j = 0; j < vObjectsForbidHL.size(); j++)
                if (vObjectsPicked[i] == vObjectsForbidHL[j]) {
                    fnd = 1;
                    break;
                }
            if (fnd) {
                if (iActiveTool == EWW_TOOL_EDITOBJ)
                    hEditObj->RenderObjectOverlay();
                continue;
            }
            WWD::Object *obj = vObjectsPicked[i];

            hgeSprite *spr = SprBank->GetObjectSprite(obj);
            float hsx, hsy;
            spr->GetHotSpot(&hsx, &hsy);
            float sprw = spr->GetWidth() / 2, sprh = spr->GetHeight() / 2;

            hsx -= sprw;
            hsy -= sprh;

            float posx = (GetUserDataFromObj(obj)->GetX() - hsx) * fZoom, posy =
                    (GetUserDataFromObj(obj)->GetY() - hsy) * fZoom;

            sprw *= fZoom;
            sprh *= fZoom;

            WWD::Rect box = SprBank->GetObjectRenderRect(obj);
            box.x2 += box.x1;
            box.y2 += box.y1;
            box.x1 = Wrd2ScrX(GetActivePlane(), box.x1);
            box.y1 = Wrd2ScrY(GetActivePlane(), box.y1);
            box.x2 = Wrd2ScrX(GetActivePlane(), box.x2);
            box.y2 = Wrd2ScrY(GetActivePlane(), box.y2);
            DWORD col = (iActiveTool == EWW_TOOL_EDITOBJ ? hEditObj->GetHighlightColor() : 0xFFFF0000);
            hge->Gfx_RenderLine(box.x1, box.y1, box.x2, box.y1, col);
            hge->Gfx_RenderLine(box.x2, box.y1, box.x2, box.y2, col);
            hge->Gfx_RenderLine(box.x2, box.y2, box.x1, box.y2, col);
            hge->Gfx_RenderLine(box.x1, box.y2, box.x1, box.y1, col);


            if (vObjectsPicked.size() == 1) {

                bool bDrawObjProperties = 1;

                if (strstr(obj->GetLogic(), "SoundTrigger") ||
                    !strcmp(obj->GetLogic(), "GlobalAmbientSound") || !strcmp(obj->GetLogic(), "AmbientSound")) {
                    WWD::Rect r[3];
                    for (int i = 0; i < 3; i++) r[i] = WWD::Rect(0, 0, 0, 0);

                    if (strstr(obj->GetLogic(), "SoundTrigger")) {
                        r[0].x1 = GetUserDataFromObj(obj)->GetX();
                        r[0].y1 = GetUserDataFromObj(obj)->GetY();
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
                    DWORD dwfCols[3] = {0x20FF0000, 0x2000FF00, 0x200000FF};
                    for (int i = 0; i < 3; i++) {
                        if (!r[i].x1 || !r[i].y1 || !r[i].x2 || !r[i].y2) continue;
                        r[i].x1 = Wrd2ScrX(GetActivePlane(), r[i].x1);
                        r[i].y1 = Wrd2ScrY(GetActivePlane(), r[i].y1);
                        r[i].x2 = Wrd2ScrX(GetActivePlane(), r[i].x2);
                        r[i].y2 = Wrd2ScrY(GetActivePlane(), r[i].y2);
                        int cx = std::max(vPort->GetX(), r[i].x1),
                                cy = std::max(vPort->GetY(), r[i].y1),
                                cw = std::min(vPort->GetX() + vPort->GetWidth(), r[i].x2),
                                ch = std::min(vPort->GetY() + vPort->GetHeight(), r[i].y2);
                        hge->Gfx_SetClipping(cx, cy, cw - cx, ch - cy);
                        RenderAreaRect(r[i], WWD::Rect(1, 1, 1, 1), 0, bmulticolor ? dwCols[i] : 0xAAFFFFFF, 1,
                                       bmulticolor ? dwfCols[i] : 0x20FFFFFF);
                        if (bmulticolor) {
                            GV->fntMyriad13->printf(cx + 11, cy + 11, HGETEXT_LEFT, "~l~%s", 0,
                                                    GETL2S("EditObj_Dialog", szRectID[i]));
                            GV->fntMyriad13->SetColor(SETA(dwCols[i], 255));
                            GV->fntMyriad13->printf(cx + 10, cy + 10, HGETEXT_LEFT, "%s", 0,
                                                    GETL2S("EditObj_Dialog", szRectID[i]));
                        }
                    }
                    vPort->ClipScreen();
                    //bDrawObjProperties = iActiveTool != EWW_TOOL_EDITOBJ;
                } else if ( /*strstr(obj->GetLogic(), "Elevator") == 0*/1) {
                    int minx = obj->GetParam(WWD::Param_MinX), miny = obj->GetParam(WWD::Param_MinY),
                            maxx = obj->GetParam(WWD::Param_MaxX), maxy = obj->GetParam(WWD::Param_MaxY);
                    int iSpringboardValue = 0;
                    bool bFill = 1;
                    if (!strcmp(vObjectsPicked[0]->GetLogic(), "SpringBoard") ||
                        !strcmp(vObjectsPicked[0]->GetLogic(), "GroundBlower")) {
                        if (iActiveTool == EWW_TOOL_EDITOBJ && hEditObj->iType == ObjEdit::enSpringboard &&
                            ((ObjEdit::cEditObjSpringboard *) hEditObj)->bPicking) {
                            miny = ((ObjEdit::cEditObjSpringboard *) hEditObj)->iPickY;
                        } else {
                            if (!maxy) miny = 450;
                            else miny = maxy;
                        }
                        iSpringboardValue = miny;
                        miny = GetUserDataFromObj(obj)->GetY() - miny;
                        maxx = minx = maxy = 0;
                        bFill = 0;
                    } else if (!strcmp(vObjectsPicked[0]->GetLogic(), "Shake")) {
                        WWD::Rect r = obj->GetAttackRect();
                        minx = r.x1;
                        miny = r.y1;
                        maxx = r.x2;
                        maxy = r.y2;
                    } else if (!strcmp(vObjectsPicked[0]->GetLogic(), "Stalactite")) {
                        WWD::Rect r = obj->GetUserRect(0);
                        minx = r.x1;
                        miny = r.y1;
                        maxx = r.x2;
                        maxy = r.y2;
                    } else if (!strcmp(vObjectsPicked[0]->GetLogic(), "SpotAmbientSound") ||
                               !strcmp(vObjectsPicked[0]->GetLogic(), "AmbientPosSound")) {
                        minx = GetUserDataFromObj(obj)->GetX();
                        miny = GetUserDataFromObj(obj)->GetY();
                        maxx = std::min(GetActivePlane()->GetPlaneWidthPx(), minx + 640);
                        maxy = std::min(GetActivePlane()->GetPlaneHeightPx(), miny + 640);
                        minx = std::max(0, minx - 640);
                        miny = std::max(0, miny - 640);
                    }

                    if ((minx != 0 || miny != 0 || maxx != 0 || maxy != 0) &&
                        !(minx == maxx && miny == maxy) &&
                        strcmp(obj->GetLogic(), "OneWayStartElevator") != 0) {

                        if (strstr(obj->GetLogic(), "Elevator") &&
                            strcmp(obj->GetLogic(), "Elevator") != 0 && strcmp(obj->GetLogic(), "PathElevator") != 0) {
                            if (minx == 0 && GetUserDataFromObj(obj)->GetX() <= maxx && maxx != 0)
                                minx = GetUserDataFromObj(obj)->GetX();
                            if (maxx == 0 && GetUserDataFromObj(obj)->GetX() >= minx && minx != 0)
                                maxx = GetUserDataFromObj(obj)->GetX();
                            if (miny == 0 && GetUserDataFromObj(obj)->GetY() <= maxy && maxy != 0)
                                miny = GetUserDataFromObj(obj)->GetY();
                            if (maxy == 0 && GetUserDataFromObj(obj)->GetY() >= miny && miny != 0)
                                maxy = GetUserDataFromObj(obj)->GetY();
                        }

                        if (minx != 0) minx = minx * fZoom - cammx;
                        if (miny != 0) miny = miny * fZoom - cammy;
                        if (maxx != 0) maxx = maxx * fZoom - cammx;
                        if (maxy != 0) maxy = maxy * fZoom - cammy;

                        if (minx < vPort->GetX() + vPort->GetWidth() ||
                            miny < vPort->GetY() + vPort->GetHeight() ||
                            maxx > vPort->GetX() ||
                            maxy > vPort->GetY()) {
                            int mmd_xmin = ((minx == 0) ? vPort->GetX() : minx + vPort->GetX());
                            int mmd_xmax = ((maxx == 0) ? vPort->GetX() + vPort->GetWidth() : maxx + vPort->GetX());
                            int mmd_ymin = ((miny == 0) ? vPort->GetY() : miny + vPort->GetY());
                            int mmd_ymax = ((maxy == 0) ? vPort->GetY() + vPort->GetHeight() : maxy + vPort->GetY());

                            int clampx = std::max(vPort->GetX(), mmd_xmin), clampy = std::max(vPort->GetY(), mmd_ymin);

                            hge->Gfx_SetClipping(clampx, clampy,
                                                 std::min(vPort->GetX() + vPort->GetWidth(), mmd_xmax) - clampx,
                                                 std::min(vPort->GetY() + vPort->GetHeight(), mmd_ymax) - clampy);

                            RenderAreaRect(WWD::Rect(mmd_xmin, mmd_ymin, mmd_xmax, mmd_ymax),
                                           WWD::Rect(minx != 0 && minx > vPort->GetX() - 5 &&
                                                     minx < vPort->GetX() + vPort->GetWidth() + 5,
                                                     miny != 0 && mmd_ymin >= vPort->GetY() &&
                                                     mmd_ymin <= vPort->GetY() + vPort->GetHeight() + 5,
                                                     maxx != 0 && maxx > vPort->GetX() - 5 &&
                                                     maxx < vPort->GetX() + vPort->GetWidth() + 5,
                                                     maxy != 0 && mmd_ymax > vPort->GetY() - 5 &&
                                                     mmd_ymax < vPort->GetY() + vPort->GetHeight() + 5),
                                           0, 0xAAFFFFFF, bFill, 0x20FFFFFF);

                            vPort->ClipScreen();
                            if (iSpringboardValue != 0) {
                                GV->fntMyriad13->printf(posx - cammx, mmd_ymin - 13, HGETEXT_CENTER, "~l~%s: %d", 0,
                                                        GETL2S("EditObj_SpringBoard", "OverlayValue"),
                                                        iSpringboardValue);
                                GV->fntMyriad13->printf(posx - cammx - 1, mmd_ymin - 14, HGETEXT_CENTER, "~w~%s: ~y~%d",
                                                        0, GETL2S("EditObj_SpringBoard", "OverlayValue"),
                                                        iSpringboardValue);
                            }
                        }
                    }
                }

                if (!strcmp(obj->GetLogic(), "PathElevator")) {
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
                    int origx = Wrd2ScrX(GetActivePlane(), GetUserDataFromObj(obj)->GetX()),
                            origy = Wrd2ScrY(GetActivePlane(), GetUserDataFromObj(obj)->GetY());
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

                        hgeSprite *spr = SprBank->GetObjectSprite(obj);

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
                        int spacesize = (spr->GetWidth() / 2 + spr->GetHeight() / 2) / 2;
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
                            GV->fntMyriad13->SetColor(0xFF000000);
                            GV->fntMyriad13->printf(origx + lastx + stepxvec / 2, origy + lasty + stepyvec / 2 + 1 - 10,
                                                    HGETEXT_CENTER, "%s: %d, %d", 0, GETL2S("ObjProp", "Movement"),
                                                    stepxvec, stepyvec);
                            GV->fntMyriad13->SetColor(0xFFFFFFFF);
                            GV->fntMyriad13->printf(origx + lastx + stepxvec / 2, origy + lasty + stepyvec / 2 - 10,
                                                    HGETEXT_CENTER, "%s: ~y~%d~w~, ~y~%d", 0,
                                                    GETL2S("ObjProp", "Movement"), stepxvec, stepyvec);
                        }

                        if (!(stepx == 0 && stepy == 0)) {
                            spr->RenderEx(origx + stepx, origy + stepy, 0, fZoom);
                            GV->fntMyriad20->SetColor(0xFF000000);
                            GV->fntMyriad20->printf(origx + stepx + 2, origy + stepy + 2, HGETEXT_LEFT, "%d", 0, s + 1);
                            GV->fntMyriad20->SetColor(0xFFFFFFFF);
                            GV->fntMyriad20->printf(origx + stepx, origy + stepy, HGETEXT_LEFT, "%d", 0, s + 1);
                        }
                    }
                } else if (strstr(obj->GetLogic(), "Elevator") && strcmp(obj->GetLogic(), "StandardElevator") != 0) {
                    int minx = obj->GetParam(WWD::Param_MinX),
                            miny = obj->GetParam(WWD::Param_MinY),
                            maxx = obj->GetParam(WWD::Param_MaxX),
                            maxy = obj->GetParam(WWD::Param_MaxY);
                    bool bStandard = !strcmp(obj->GetLogic(), "Elevator");
                    if (!bStandard) {
                        if (minx == 0 && GetUserDataFromObj(obj)->GetX() <= maxx && maxx != 0)
                            minx = GetUserDataFromObj(obj)->GetX();
                        if (maxx == 0 && GetUserDataFromObj(obj)->GetX() >= minx && minx != 0)
                            maxx = GetUserDataFromObj(obj)->GetX();
                        if (miny == 0 && GetUserDataFromObj(obj)->GetY() <= maxy && maxy != 0)
                            miny = GetUserDataFromObj(obj)->GetY();
                        if (maxy == 0 && GetUserDataFromObj(obj)->GetY() >= miny && miny != 0)
                            maxy = GetUserDataFromObj(obj)->GetY();
                    }
                    if (minx != 0 && maxx != 0 && minx <= maxx || miny != 0 && maxy != 0 && miny <= maxy) {
                        hgeSprite *spr = SprBank->GetObjectSprite(obj);
                        if (minx == maxx) { //vertical
                            int starty = Wrd2ScrY(GetActivePlane(), miny),
                                    endy = Wrd2ScrY(GetActivePlane(), maxy);
                            float xpad = 22 * fZoom;
                            int x = Wrd2ScrX(GetActivePlane(), GetUserDataFromObj(obj)->GetX());
                            spr->SetColor(0x77FFFFFF);
                            if (miny != GetUserDataFromObj(obj)->GetY())
                                spr->RenderEx(x, starty, 0, fZoom);
                            if (maxy != GetUserDataFromObj(obj)->GetY())
                                spr->RenderEx(x, endy, 0, fZoom);
                            float lpad = 48 * fZoom, rpad = 48 * fZoom;
                            GV->sprArrowVerticalU->SetColor(0x77FFFFFF);
                            GV->sprArrowVerticalM->SetColor(0x77FFFFFF);
                            GV->sprArrowVerticalD->SetColor(0x77FFFFFF);
                            GV->sprArrowVerticalU->RenderEx(x, starty + 37 * fZoom, 0, fZoom);
                            GV->sprArrowVerticalM->RenderEx(x, endy - rpad, 0, fZoom,
                                                            float(endy - starty - lpad - 37 * fZoom) / 128.0f);
                            GV->sprArrowVerticalD->RenderEx(x - xpad, endy - rpad, 0, fZoom);
                        } else if (miny == maxy) { //horizontal
                            int startx = Wrd2ScrX(GetActivePlane(), minx),
                                    endx = Wrd2ScrX(GetActivePlane(), maxx);
                            float ypad = 22 * fZoom;
                            int y = Wrd2ScrY(GetActivePlane(), GetUserDataFromObj(obj)->GetY()) - ypad;
                            spr->SetColor(0x77FFFFFF);
                            if (minx != GetUserDataFromObj(obj)->GetX())
                                spr->RenderEx(startx, y + ypad, 0, fZoom);
                            if (maxx != GetUserDataFromObj(obj)->GetX())
                                spr->RenderEx(endx, y + ypad, 0, fZoom);
                            float lpad = 35 * fZoom, rpad = 39 * fZoom;
                            GV->sprArrowHorizontalL->SetColor(0x77FFFFFF);
                            GV->sprArrowHorizontalC->SetColor(0x77FFFFFF);
                            GV->sprArrowHorizontalR->SetColor(0x77FFFFFF);
                            GV->sprArrowHorizontalL->RenderEx(startx, y, 0, fZoom);
                            GV->sprArrowHorizontalC->RenderEx(startx + lpad, y, 0,
                                                              float(endx - startx - lpad - rpad) / 8.0f, fZoom);
                            GV->sprArrowHorizontalR->RenderEx(endx - rpad, y, 0, fZoom);
                        } else { //4-way
                            int speedx = obj->GetParam(WWD::Param_SpeedX),
                                    speedy = obj->GetParam(WWD::Param_SpeedY);
                            if (speedx <= 0) speedx = SIM_ELEVATOR_DEFAULT_SPEED;
                            if (speedy <= 0) speedy = SIM_ELEVATOR_DEFAULT_SPEED;
                            int origspeedx = speedx,
                                    origspeedy = speedy;
                            int initx = GetUserDataFromObj(obj)->GetX(), inity = GetUserDataFromObj(obj)->GetY();
                            bool bDrawInit = (initx < minx || initx > maxx || inity < miny || inity > maxy);
                            if (initx < minx) initx = minx;
                            if (initx > maxx) initx = maxx;
                            if (inity < miny) inity = miny;
                            if (inity > maxy) inity = maxy;
                            spr->SetColor(0x77FFFFFF);
                            if (bDrawInit) {
                                spr->RenderEx(Wrd2ScrX(GetActivePlane(), initx), Wrd2ScrY(GetActivePlane(), inity), 0,
                                              fZoom);
                            }
                            int posx = initx - minx, posy = inity - miny;
                            int rectw = maxx - minx, recth = maxy - miny;
                            int dir = (bStandard ? obj->GetParam(WWD::Param_Direction) : 7);
                            bool bDirVer = (dir <= 3 || dir == 5 || dir > 9), bDirHor = (dir <= 0 || dir == 3 ||
                                                                                         dir == 6 || dir >= 9);
                            bool bPathClosed = 0;
                            std::vector<std::pair<int, int> > vPrevPos;
                            vPrevPos.push_back(std::pair<int, int>(posx, posy));
                            std::vector<int> vDirs;
                            int iStepLooped = -1;
                            for (int i = 0; i < 8; i++) {
                                int lastx = posx, lasty = posy;
                                if (bDirHor && bDirVer) dir = 3;
                                else if (!bDirHor && !bDirVer) dir = 7;
                                else if (bDirHor && !bDirVer) dir = 9;
                                else if (!bDirHor && bDirVer) dir = 1;
                                int wayx = (bDirHor ? rectw - posx : posx),
                                        wayy = (bDirVer ? recth - posy : posy);
                                float timex = (speedx > 0 ? float(wayx) / float(speedx) : -1),
                                        timey = (speedy > 0 ? float(wayy) / float(speedy) : -1);
                                if (timex == timey || timex == -1 || timey == -1) { //we go to the rect corner
                                    posx = (bDirHor ? rectw : 0);
                                    posy = (bDirVer ? recth : 0);
                                    bDirHor = !bDirHor;
                                    bDirVer = !bDirVer;
                                    if (!bStandard) {
                                        if (speedx == 0)
                                            dir = (bDirVer ? 2 : 8);
                                        else if (speedy == 0)
                                            dir = (bDirHor ? 6 : 4);
                                        if (iStepLooped == -1 && strcmp(obj->GetLogic(), "OneWayStartElevator") != 0)
                                            iStepLooped = vPrevPos.size();
                                        if (!strcmp(obj->GetLogic(), "OneWayStartElevator")) {
                                            bPathClosed = 1;
                                        } else {
                                            speedx = origspeedx;
                                            speedy = origspeedy;
                                        }
                                    }
                                } else if (timey > timex) { //bounce off the | border
                                    posx = (bDirHor ? rectw : 0);
                                    posy += (bDirVer ? speedy * timex : -speedy * timex);
                                    if (!bStandard) {
                                        speedx = 0;
                                    } else bDirHor = !bDirHor;
                                } else if (timex > timey) { //bounce off the - border
                                    posx += (bDirHor ? speedx * timey : -speedx * timey);
                                    posy = (bDirVer ? recth : 0);
                                    if (!bStandard) {
                                        speedy = 0;
                                    } else bDirVer = !bDirVer;
                                }
                                vDirs.push_back(dir);
                                for (int i = 0; i < vPrevPos.size(); i++) {
                                    if (posx == vPrevPos[i].first && posy == vPrevPos[i].second) {
                                        bPathClosed = 1;
                                        break;
                                    }
                                }
                                vPrevPos.push_back(std::pair<int, int>(posx, posy));
                                if (bPathClosed) break;
                            }
                            if (bPathClosed) {
                                for (int i = 1; i < vPrevPos.size(); i++) {
                                    int arroffx = spr->GetWidth() / 2, arroffy = spr->GetWidth() / 2;
                                    if (!(vDirs[i - 1] == 7 || vDirs[i - 1] == 1 || vDirs[i - 1] == 6)) arroffx *= -1;
                                    if (!(vDirs[i - 1] == 7 || vDirs[i - 1] == 9 || vDirs[i - 1] == 2)) arroffy *= -1;
                                    if (vDirs[i - 1] == 6 || vDirs[i - 1] == 4) arroffy = 0;
                                    if (vDirs[i - 1] == 2 || vDirs[i - 1] == 8) arroffx = 0;
                                    if (i > iStepLooped) {
                                        GV->sprArrowVerticalM->SetColor(0x77FFFFFF);
                                        GV->sprArrowVerticalU->SetColor(0x77FFFFFF);
                                    } else {
                                        GV->sprArrowVerticalM->SetColor(0x7733FF33);
                                        GV->sprArrowVerticalU->SetColor(0x7733FF33);
                                    }
                                    spr->RenderEx(Wrd2ScrX(GetActivePlane(), vPrevPos[i].first + minx),
                                                  Wrd2ScrY(GetActivePlane(), vPrevPos[i].second + miny), 0, fZoom);
                                    RenderArrow(Wrd2ScrX(GetActivePlane(), vPrevPos[i - 1].first + minx - arroffx / 2),
                                                Wrd2ScrY(GetActivePlane(), vPrevPos[i - 1].second + miny - arroffy / 2),
                                                Wrd2ScrX(GetActivePlane(), vPrevPos[i].first + minx + arroffx / 2),
                                                Wrd2ScrY(GetActivePlane(), vPrevPos[i].second + miny + arroffy / 2),
                                                1, 0);
                                }
                            } else {
                                int arroffx = spr->GetWidth() / 2, arroffy = spr->GetWidth() / 2;
                                if (!(vDirs[0] == 7 || vDirs[0] == 4 || vDirs[0] == 1)) arroffx *= -1;
                                if (!(vDirs[0] == 7 || vDirs[0] == 8 || vDirs[0] == 9)) arroffy *= -1;
                                hgeVector vec(speedx, speedy);
                                if (speedx < 150 || speedy < 150) {
                                    vec.x *= 5;
                                    vec.y *= 5;
                                }
                                vec.Clamp(150);
                                RenderArrow(Wrd2ScrX(GetActivePlane(), vPrevPos[0].first + minx),
                                            Wrd2ScrY(GetActivePlane(), vPrevPos[0].second + miny),
                                            Wrd2ScrX(GetActivePlane(),
                                                     vPrevPos[0].first + minx + (arroffx < 0 ? vec.x : -vec.x) * fZoom),
                                            Wrd2ScrY(GetActivePlane(), vPrevPos[0].second + miny +
                                                                       (arroffy < 0 ? vec.y : -vec.y) * fZoom),
                                            0);
                            }
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
                    int outx, outy;
                    bool bdr = 0;
                    if (iActiveTool == EWW_TOOL_EDITOBJ && conMain->getWidgetAt(mx, my) == vPort->GetWidget() &&
                        (hEditObj->iType == ObjEdit::enWarp && ((ObjEdit::cEditObjWarp *) hEditObj)->bPick ||
                         hEditObj->iType == ObjEdit::enCrate && ((ObjEdit::cEditObjCrate *) hEditObj)->Picking() ||
                         hEditObj->iType == ObjEdit::enStatue && ((ObjEdit::cEditObjStatue *) hEditObj)->Picking())) {
                        outx = mx;
                        outy = my;
                        bdr = 1;
                    } else {
                        outx = Wrd2ScrX(GetActivePlane(), obj->GetParam(WWD::Param_SpeedX)),
                                outy = Wrd2ScrY(GetActivePlane(), obj->GetParam(WWD::Param_SpeedY));
                    }
                    hgeSprite *spr2 = SprBank->GetAssetByID("CLAW")->GetIMGByID(401)->GetSprite();
                    spr2->SetColor(0xBBFFFFFF);
                    spr2->RenderEx(outx, outy, 0, fZoom);
                    if (iActiveTool != EWW_TOOL_EDITOBJ || bdr) {
                        int drx, dry;
                        if (bdr) {
                            drx = Scr2WrdX(GetActivePlane(), outx);
                            dry = Scr2WrdY(GetActivePlane(), outy);
                        } else {
                            drx = obj->GetParam(WWD::Param_SpeedX);
                            dry = obj->GetParam(WWD::Param_SpeedY);
                        }
                        GV->fntMyriad13->printf(outx + spr2->GetWidth() / 2 + 1, outy + 1, HGETEXT_LEFT,
                                                "~l~%s: %d, %d", 0, GETL2S("EditObj_Warp", "SpawnPos"), drx, dry);
                        GV->fntMyriad13->printf(outx + spr2->GetWidth() / 2, outy, HGETEXT_LEFT,
                                                "~w~%s: ~y~%d~w~, ~y~%d", 0, GETL2S("EditObj_Warp", "SpawnPos"), drx,
                                                dry);
                    }
                }
                if (iActiveTool == EWW_TOOL_EDITOBJ &&
                    (hEditObj->iType == ObjEdit::enPathElevator || hEditObj->IsMovingObject() ||
                     hEditObj->iType == ObjEdit::enText)
                    || iActiveTool == EWW_TOOL_MOVEOBJECT)
                    bDrawObjProperties = 0;

                if (obj == hStartingPosObj) {
                    GV->fntMyriad13->printf(posx - cammx + sprw + 5, posy + vPort->GetY() + 1 - cammy - sprh,
                                            HGETEXT_LEFT, "~l~%s", 0, GETL(Lang_StartingPlace));
                    GV->fntMyriad13->printf(posx - cammx + sprw + 4, posy + vPort->GetY() - cammy - sprh, HGETEXT_LEFT,
                                            "~w~%s~l~", 0, GETL(Lang_StartingPlace));

                    GV->fntMyriad13->printf(posx - cammx + sprw + 5, posy + vPort->GetY() + 1 - cammy - sprh + 15,
                                            HGETEXT_LEFT, "~l~X: %d Y: %d", 0, obj->GetParam(WWD::Param_LocationX),
                                            obj->GetParam(WWD::Param_LocationY));
                    GV->fntMyriad13->printf(posx - cammx + sprw + 4, posy + vPort->GetY() - cammy - sprh + 15,
                                            HGETEXT_LEFT, "~w~X: ~y~%d ~w~Y: ~y~%d~l~", 0,
                                            obj->GetParam(WWD::Param_LocationX), obj->GetParam(WWD::Param_LocationY));
                } else if (bDrawObjProperties) {
                    int ioff = 15;
                    int iid = obj->GetParam(WWD::Param_ID);
                    if (iActiveTool == EWW_TOOL_EDITOBJ) {
                        iid = hEditObj->GetOrigObj()->GetParam(WWD::Param_ID);
                    }
                    GV->fntMyriad13->printf(posx - cammx + sprw + 5, posy + vPort->GetY() + 1 - cammy - sprh,
                                            HGETEXT_LEFT, "~l~%s: %d", 0, GETL(Lang_ID), iid);
                    GV->fntMyriad13->printf(posx - cammx + sprw + 4, posy + vPort->GetY() - cammy - sprh, HGETEXT_LEFT,
                                            "~w~%s: ~y~%d", 0, GETL(Lang_ID), iid);
                    if (obj->GetName()[0] != '\0' && strcmp(obj->GetLogic(), "CustomLogic") != 0) {
                        GV->fntMyriad13->printf(posx - cammx + sprw + 5, posy + vPort->GetY() + 1 - cammy + ioff - sprh,
                                                HGETEXT_LEFT, "~l~%s: %s", 0, GETL(Lang_Name), obj->GetName());
                        GV->fntMyriad13->printf(posx - cammx + sprw + 4, posy + vPort->GetY() - cammy + ioff - sprh,
                                                HGETEXT_LEFT, "~w~%s: ~y~%s", 0, GETL(Lang_Name), obj->GetName());
                        ioff += 15;
                    }

                    if (obj->GetLogic()[0] != '\0') {
                        bool customlogic = !strcmp(obj->GetLogic(), "CustomLogic");
                        const char *logicval = (customlogic ? obj->GetName()
                                                            : obj->GetLogic());
                        if (customlogic) {
                            GV->sprIcons16[Icon16_CrazyHook]->SetColor(0xFFFFFFFF);
                            GV->sprIcons16[Icon16_CrazyHook]->Render(
                                    posx - cammx + sprw + 7 + GV->fntMyriad13->GetStringWidth(GETL(Lang_Logic)),
                                    posy + vPort->GetY() - cammy + ioff - sprh);
                        }
                        GV->fntMyriad13->printf(posx - cammx + sprw + 5, posy + vPort->GetY() + 1 - cammy + ioff - sprh,
                                                HGETEXT_LEFT, (customlogic ? "~l~%s:    %s" : "~l~%s: %s"), 0,
                                                GETL(Lang_Logic), logicval);
                        GV->fntMyriad13->printf(posx - cammx + sprw + 4, posy + vPort->GetY() - cammy + ioff - sprh,
                                                HGETEXT_LEFT, (customlogic ? "~w~%s:    ~y~%s" : "~w~%s: ~y~%s"), 0,
                                                GETL(Lang_Logic), logicval);
                        ioff += 15;
                    }

                    if (obj->GetImageSet()[0] != '\0') {
                        GV->fntMyriad13->printf(posx - cammx + sprw + 5, posy + vPort->GetY() + 1 - cammy + ioff - sprh,
                                                HGETEXT_LEFT, "~l~%s: %s", 0, GETL(Lang_Graphic), obj->GetImageSet());
                        GV->fntMyriad13->printf(posx - cammx + sprw + 4, posy + vPort->GetY() - cammy + ioff - sprh,
                                                HGETEXT_LEFT, "~w~%s: ~y~%s", 0, GETL(Lang_Graphic),
                                                obj->GetImageSet());
                        ioff += 15;
                    }

                    if (obj->GetAnim()[0] != '\0') {
                        GV->fntMyriad13->printf(posx - cammx + sprw + 5, posy + vPort->GetY() + 1 - cammy + ioff - sprh,
                                                HGETEXT_LEFT, "~l~%s: %s", 0, GETL(Lang_Anim), obj->GetAnim());
                        GV->fntMyriad13->printf(posx - cammx + sprw + 4, posy + vPort->GetY() - cammy + ioff - sprh,
                                                HGETEXT_LEFT, "~w~%s: ~y~%s", 0, GETL(Lang_Anim), obj->GetAnim());
                        ioff += 15;
                    }

                    GV->fntMyriad13->printf(posx - cammx + sprw + 5, posy + vPort->GetY() + 1 - cammy + ioff - sprh,
                                            HGETEXT_LEFT, "~l~%s: %dx%d, Z: %d", 0,
                                            GETL(Lang_Location), obj->GetParam(WWD::Param_LocationX),
                                            obj->GetParam(WWD::Param_LocationY),
                                            obj->GetParam(WWD::Param_LocationZ));
                    GV->fntMyriad13->printf(posx - cammx + sprw + 4, posy + vPort->GetY() - cammy + ioff - sprh,
                                            HGETEXT_LEFT, "~w~%s: ~y~%d~w~x~y~%d~w~, Z: ~y~%d", 0,
                                            GETL(Lang_Location), obj->GetParam(WWD::Param_LocationX),
                                            obj->GetParam(WWD::Param_LocationY),
                                            obj->GetParam(WWD::Param_LocationZ));
                    ioff += 15;
                    if (obj->GetParam(WWD::Param_MinX) != 0 || obj->GetParam(WWD::Param_MaxX) != 0) {
                        if (obj->GetParam(WWD::Param_MinX) != 0 && obj->GetParam(WWD::Param_MaxX) != 0) {
                            GV->fntMyriad13->printf(posx - cammx + sprw + 5,
                                                    posy + vPort->GetY() + 1 - cammy + ioff - sprh, HGETEXT_LEFT,
                                                    "~l~%s: %d. %s: %d.", 0,
                                                    GETL(Lang_MinX), obj->GetParam(WWD::Param_MinX), GETL(Lang_MaxX),
                                                    obj->GetParam(WWD::Param_MaxX));
                            GV->fntMyriad13->printf(posx - cammx + sprw + 4, posy + vPort->GetY() - cammy + ioff - sprh,
                                                    HGETEXT_LEFT, "~w~%s: ~y~%d~w~. %s: ~y~%d~w~.", 0,
                                                    GETL(Lang_MinX), obj->GetParam(WWD::Param_MinX), GETL(Lang_MaxX),
                                                    obj->GetParam(WWD::Param_MaxX));
                        } else if (obj->GetParam(WWD::Param_MinX) != 0 && obj->GetParam(WWD::Param_MaxX) == 0) {
                            GV->fntMyriad13->printf(posx - cammx + sprw + 5,
                                                    posy + vPort->GetY() + 1 - cammy + ioff - sprh, HGETEXT_LEFT,
                                                    "~l~%s: %d", 0,
                                                    GETL(Lang_MinX), obj->GetParam(WWD::Param_MinX));
                            GV->fntMyriad13->printf(posx - cammx + sprw + 4, posy + vPort->GetY() - cammy + ioff - sprh,
                                                    HGETEXT_LEFT, "~w~%s: ~y~%d", 0,
                                                    GETL(Lang_MinX), obj->GetParam(WWD::Param_MinX));
                        } else if (obj->GetParam(WWD::Param_MinX) == 0 && obj->GetParam(WWD::Param_MaxX) != 0) {
                            GV->fntMyriad13->printf(posx - cammx + sprw + 5,
                                                    posy + vPort->GetY() + 1 - cammy + ioff - sprh, HGETEXT_LEFT,
                                                    "~l~%s: %d", 0,
                                                    GETL(Lang_MaxX), obj->GetParam(WWD::Param_MaxX));
                            GV->fntMyriad13->printf(posx - cammx + sprw + 4, posy + vPort->GetY() - cammy + ioff - sprh,
                                                    HGETEXT_LEFT, "~w~%s: ~y~%d", 0,
                                                    GETL(Lang_MaxX), obj->GetParam(WWD::Param_MaxX));
                        }
                        ioff += 15;
                    }
                    if (obj->GetParam(WWD::Param_MinY) != 0 || obj->GetParam(WWD::Param_MaxY) != 0) {
                        if (obj->GetParam(WWD::Param_MinY) != 0 && obj->GetParam(WWD::Param_MaxY) != 0) {
                            GV->fntMyriad13->printf(posx - cammx + sprw + 5,
                                                    posy + vPort->GetY() + 1 - cammy + ioff - sprh, HGETEXT_LEFT,
                                                    "~l~%s: %d. %s: %d.", 0,
                                                    GETL(Lang_MinY), obj->GetParam(WWD::Param_MinY), GETL(Lang_MaxY),
                                                    obj->GetParam(WWD::Param_MaxY));
                            GV->fntMyriad13->printf(posx - cammx + sprw + 4, posy + vPort->GetY() - cammy + ioff - sprh,
                                                    HGETEXT_LEFT, "~w~%s: ~y~%d~w~. %s: ~y~%d~w~.", 0,
                                                    GETL(Lang_MinY), obj->GetParam(WWD::Param_MinY), GETL(Lang_MaxY),
                                                    obj->GetParam(WWD::Param_MaxY));
                        } else if (obj->GetParam(WWD::Param_MinY) != 0 && obj->GetParam(WWD::Param_MaxY) == 0) {
                            GV->fntMyriad13->printf(posx - cammx + sprw + 5,
                                                    posy + vPort->GetY() + 1 - cammy + ioff - sprh, HGETEXT_LEFT,
                                                    "~l~%s: %d", 0,
                                                    GETL(Lang_MinY), obj->GetParam(WWD::Param_MinY));
                            GV->fntMyriad13->printf(posx - cammx + sprw + 4, posy + vPort->GetY() - cammy + ioff - sprh,
                                                    HGETEXT_LEFT, "~w~%s: ~y~%d", 0,
                                                    GETL(Lang_MinY), obj->GetParam(WWD::Param_MinY));
                        } else if (obj->GetParam(WWD::Param_MinY) == 0 && obj->GetParam(WWD::Param_MaxY) != 0) {
                            GV->fntMyriad13->printf(posx - cammx + sprw + 5,
                                                    posy + vPort->GetY() + 1 - cammy + ioff - sprh, HGETEXT_LEFT,
                                                    "~l~%s: %d", 0,
                                                    GETL(Lang_MaxY), obj->GetParam(WWD::Param_MaxY));
                            GV->fntMyriad13->printf(posx - cammx + sprw + 4, posy + vPort->GetY() - cammy + ioff - sprh,
                                                    HGETEXT_LEFT, "~w~%s: ~y~%d", 0,
                                                    GETL(Lang_MaxY), obj->GetParam(WWD::Param_MaxY));
                        }
                        ioff += 15;
                    }

                    if (!strcmp(obj->GetLogic(), "FrontCrate") || !strcmp(obj->GetLogic(), "Statue") ||
                        !strcmp(obj->GetLogic(), "BehindCrate") || !strcmp(obj->GetLogic(), "BackStackedCrates") ||
                        !strcmp(obj->GetLogic(), "FrontStackedCrates") || IsObjectEnemy(obj) &&
                                                                          !(!strcmp(obj->GetLogic(), "Rat") ||
                                                                            !strcmp(obj->GetLogic(), "Fish"))) {
                        bool singlecrate =
                                !strcmp(obj->GetLogic(), "FrontCrate") || !strcmp(obj->GetLogic(), "BehindCrate");
                        cInventoryItem items[10];
                        for (int i = 0; i < 10; i++)
                            items[i] = cInventoryItem("", -1);
                        int itemoff = 0;
                        if (!strcmp(obj->GetLogic(), "Raux") ||
                            !strcmp(obj->GetLogic(), "Katherine") ||
                            !strcmp(obj->GetLogic(), "Wolvington") ||
                            !strcmp(obj->GetLogic(), "Gabriel") ||
                            !strcmp(obj->GetLogic(), "Marrow") ||
                            !strcmp(obj->GetLogic(), "Aquatis") ||
                            !strcmp(obj->GetLogic(), "RedTail") ||
                            !strcmp(obj->GetLogic(), "Omar")) {
                            items[0] = hInvCtrl->GetItemByID(31); //end of level powerup
                            itemoff++;
                        }
                        if (obj->GetParam(WWD::Param_Powerup) == 0 && singlecrate) {
                            items[itemoff] = hInvCtrl->GetItemByID(33);
                            itemoff++;
                        } else if (obj->GetParam(WWD::Param_Powerup)) {
                            items[itemoff] = hInvCtrl->GetItemByID(obj->GetParam(WWD::Param_Powerup));
                            if (items[itemoff].second != -1) itemoff++;
                        }
                        if (!singlecrate) {
                            for (int z = 0; z < 2; z++) {
                                WWD::Rect r = obj->GetUserRect(z);
                                if (r.x1 != 0) {
                                    items[itemoff] = hInvCtrl->GetItemByID(r.x1);
                                    if (items[itemoff].second != -1) itemoff++;
                                }
                                if (r.y1 != 0) {
                                    items[itemoff] = hInvCtrl->GetItemByID(r.y1);
                                    if (items[itemoff].second != -1) itemoff++;
                                }
                                if (r.x2 != 0) {
                                    items[itemoff] = hInvCtrl->GetItemByID(r.x2);
                                    if (items[itemoff].second != -1) itemoff++;
                                }
                                if (r.y2 != 0) {
                                    items[itemoff] = hInvCtrl->GetItemByID(r.y2);
                                    if (items[itemoff].second != -1) itemoff++;
                                }
                            }
                        }
                        for (int y = 0; y < 4; y++)
                            for (int x = 0; x < 3; x++) {
                                if (y * 3 + x > 9 || items[y * 3 + x].second == -1) break;
                                cSprBankAsset *asset = GV->editState->SprBank->GetAssetByID(
                                        items[y * 3 + x].first.c_str());
                                int iframe = GV->editState->hInvCtrl->GetAnimFrame() % asset->GetSpritesCount();
                                hgeSprite *spr = asset->GetIMGByIterator(iframe)->GetSprite();
                                spr->SetColor(0xBBFFFFFF);
                                spr->SetFlip(0, 0);
                                int grdim = spr->GetWidth();
                                if (spr->GetHeight() > grdim) grdim = spr->GetHeight();
                                float fScale = 1.0f;
                                if (grdim > 48) fScale = 48.0f / float(grdim);
                                spr->RenderEx(posx - cammx + sprw + 4 + 52 * x + 24,
                                              posy + vPort->GetY() - cammy + ioff - sprh + y * 54 + 24, 0, fScale);
                            }
                    }

                }
                GV->fntMyriad13->SetColor(0xFF000000);
            }
            if (iActiveTool == EWW_TOOL_EDITOBJ) {
                hEditObj->RenderObjectOverlay();
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
			GV->fntMyriad13->SetColor(0xFFFFFFFF);
			float gmx, gmy;
			hge->Input_GetMousePos(&gmx, &gmy);
			gmx = Scr2WrdX(GetActivePlane(), gmx);
			gmy = Scr2WrdY(GetActivePlane(), gmy);
			GV->fntMyriad13->printf(vPort->GetX(), vPort->GetY() + 20, HGETEXT_LEFT, "mouse cell 0x~y~%p ~w~(~y~%d~w~x~y~%d~w~)", 0,
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
                GV->fntMyriad13->printf(x1 + (x2 - x1) / 2, y1 + (y2 - y1) / 2 - 20, HGETEXT_LEFT, "~y~%.0f~w~px", 0,
                                        len);
            else
                GV->fntMyriad13->printf(x1 + (x2 - x1) / 2, y1 + (y2 - y1) / 2, HGETEXT_LEFT, "~y~%.0f~w~px", 0, len);
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
                GV->fntMyriad13->printf(x1 + (scrx - x1) / 2, y1 + (scry - y1) / 2 - 20, HGETEXT_LEFT, "~y~%.0f~w~px",
                                        0, len);
            else
                GV->fntMyriad13->printf(x1 + (scrx - x1) / 2, y1 + (scry - y1) / 2, HGETEXT_LEFT, "~y~%.0f~w~px", 0,
                                        len);
        }

        if (iMode == EWW_MODE_TILE) {
            if (fCamX < 0) {
                hge->Gfx_SetClipping(vPort->GetX(), vPort->GetY(), -fCamX, vPort->GetHeight());
                for (int y = 0; y <= vPort->GetHeight() / 120; y++) {
                    GV->sprCheckboard->Render(vPort->GetX(), vPort->GetY() + y * 120);
                }
            }
            int maxx = std::max(GetActivePlane()->GetPlaneWidthPx() - vPort->GetWidth() / fZoom, -40.0f);
            if (fCamX > maxx && !(GetActivePlane()->GetFlags() & WWD::Flag_p_XWrapping)) {
                hge->Gfx_SetClipping(vPort->GetX() + vPort->GetWidth() - (fCamX - maxx), vPort->GetY(), fCamX - maxx,
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
            int maxy = std::max(GetActivePlane()->GetPlaneHeightPx() - vPort->GetHeight() / fZoom, -40.0f);
            if (fCamY > maxy && !(GetActivePlane()->GetFlags() & WWD::Flag_p_YWrapping)) {
                hge->Gfx_SetClipping(vPort->GetX(), vPort->GetY() + vPort->GetHeight() - (fCamY - maxy),
                                     vPort->GetWidth(), fCamY - maxy);
                for (int x = 0; x <= vPort->GetWidth() / 120; x++) {
                    GV->sprCheckboard->Render(vPort->GetX() + x * 120,
                                              vPort->GetY() + vPort->GetHeight() - (vPort->GetHeight() % 120));
                }
            }
        }
        hge->Gfx_SetClipping();

        if (bDrawTileProperties) {
            GV->fntMyriad13->SetColor(0xFF000000);
            GV->fntMyriad13->printf(vPort->GetX() + 11, vPort->GetY() + vPort->GetHeight() - 99, HGETEXT_LEFT,
                                    GETL(Lang_Legend), 0);
            GV->fntMyriad13->printf(vPort->GetX() + 61, vPort->GetY() + vPort->GetHeight() - 79, HGETEXT_LEFT,
                                    GETL(Lang_PropSolid), 0);
            GV->fntMyriad13->printf(vPort->GetX() + 61, vPort->GetY() + vPort->GetHeight() - 59, HGETEXT_LEFT,
                                    GETL(Lang_PropGround), 0);
            GV->fntMyriad13->printf(vPort->GetX() + 61, vPort->GetY() + vPort->GetHeight() - 39, HGETEXT_LEFT,
                                    GETL(Lang_PropClimb), 0);
            GV->fntMyriad13->printf(vPort->GetX() + 61, vPort->GetY() + vPort->GetHeight() - 19, HGETEXT_LEFT,
                                    GETL(Lang_PropDeath), 0);

            GV->fntMyriad13->SetColor(0xFFFFFFFF);
            GV->fntMyriad13->printf(vPort->GetX() + 10, vPort->GetY() + vPort->GetHeight() - 100, HGETEXT_LEFT,
                                    GETL(Lang_Legend), 0);
            GV->fntMyriad13->printf(vPort->GetX() + 60, vPort->GetY() + vPort->GetHeight() - 80, HGETEXT_LEFT,
                                    GETL(Lang_PropSolid), 0);
            GV->fntMyriad13->printf(vPort->GetX() + 60, vPort->GetY() + vPort->GetHeight() - 60, HGETEXT_LEFT,
                                    GETL(Lang_PropGround), 0);
            GV->fntMyriad13->printf(vPort->GetX() + 60, vPort->GetY() + vPort->GetHeight() - 40, HGETEXT_LEFT,
                                    GETL(Lang_PropClimb), 0);
            GV->fntMyriad13->printf(vPort->GetX() + 60, vPort->GetY() + vPort->GetHeight() - 20, HGETEXT_LEFT,
                                    GETL(Lang_PropDeath), 0);

            hgeQuad q;
            q.blend = BLEND_DEFAULT;
            q.tex = 0;

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

    GV->hGfxInterface->sprMainShadeBar->RenderStretch(vPort->GetX(), vPort->GetY(),
                                                      hge->System_GetState(HGE_SCREENWIDTH) - 3, vPort->GetY() + 9);
    //GV->sprShadeBar->RenderStretch(vPort->GetX(), vPort->GetY(), vPort->GetX()+vPort->GetWidth(), vPort->GetY()+6);

    /*if( cScrollOrientation == 0 ){
	 GV->sprMicroVert->Render(hge->System_GetState(HGE_SCREENWIDTH)-8, hge->System_GetState(HGE_SCREENHEIGHT)-31);
	}else if( cScrollOrientation == 1 ){
	 GV->sprMicroHor->Render(hge->System_GetState(HGE_SCREENWIDTH)-10, hge->System_GetState(HGE_SCREENHEIGHT)-28);
	}else if( cScrollOrientation == 2 ){
	 GV->sprMicroZoom->Render(hge->System_GetState(HGE_SCREENWIDTH)-11, hge->System_GetState(HGE_SCREENHEIGHT)-31);
	}*/

    if (fHintTime >= 0) {
        fHintTime += hge->Timer_GetDelta();
        if (fHintTime > HINT_TIME)
            fHintTime = -1;
        GV->fntMyriad10->printf(45, hge->System_GetState(HGE_SCREENHEIGHT) - 18, HGETEXT_LEFT, szHint, 0);
    }

    if (iActiveTool == EWW_TOOL_ALIGNOBJ) {
        GV->fntMyriad13->printf(vPort->GetX() + 14, vPort->GetY() + vPort->GetHeight() - 31, HGETEXT_LEFT,
                                "~l~%s", 0, GETL2S("Various", "AlignObj_Info"));
        GV->fntMyriad13->printf(vPort->GetX() + 15, vPort->GetY() + vPort->GetHeight() - 30, HGETEXT_LEFT,
                                "~w~%s", 0, GETL2S("Various", "AlignObj_Info"));
    } else if (iActiveTool == EWW_TOOL_MOVEOBJECT) {
        int yoffset = vPort->GetY() + vPort->GetHeight();

        GV->fntMyriad13->printf(vPort->GetX() + 14, yoffset - 61, HGETEXT_LEFT,
                                "~l~Shift - %s.", 0, GETL2S("Various", "MoveObj_Info1"));
        GV->fntMyriad13->printf(vPort->GetX() + 15, yoffset - 60, HGETEXT_LEFT,
                                "~y~Shift ~w~- %s.", 0, GETL2S("Various", "MoveObj_Info1"));

        GV->fntMyriad13->printf(vPort->GetX() + 14, yoffset - 46, HGETEXT_LEFT,
                                "~l~Ctrl - %s.", 0, GETL2S("Various", "MoveObj_Info2"));
        GV->fntMyriad13->printf(vPort->GetX() + 15, yoffset - 45, HGETEXT_LEFT,
                                "~y~Ctrl ~w~- %s.", 0, GETL2S("Various", "MoveObj_Info2"));

        GV->fntMyriad13->printf(vPort->GetX() + 14, yoffset - 31, HGETEXT_LEFT, "~l~Esc - %s.", 0,
                                GETL2S("Various", "MoveObj_Info3"));
        GV->fntMyriad13->printf(vPort->GetX() + 15, yoffset - 30, HGETEXT_LEFT, "~y~Esc ~w~- %s.", 0,
                                GETL2S("Various", "MoveObj_Info3"));

    }

    //lower bar
    hge->Gfx_RenderLine(1, hge->System_GetState(HGE_SCREENHEIGHT) - 29, hge->System_GetState(HGE_SCREENWIDTH) - 2,
                        hge->System_GetState(HGE_SCREENHEIGHT) - 29, 0xFF333333);
    hge->Gfx_RenderLine(1, hge->System_GetState(HGE_SCREENHEIGHT) - 3, hge->System_GetState(HGE_SCREENWIDTH) - 2,
                        hge->System_GetState(HGE_SCREENHEIGHT) - 3, 0xFF333333);

    q.v[0].x = 1;
    q.v[0].y = hge->System_GetState(HGE_SCREENHEIGHT) - 29;
    q.v[1].x = hge->System_GetState(HGE_SCREENWIDTH) - 2;
    q.v[1].y = hge->System_GetState(HGE_SCREENHEIGHT) - 29;
    q.v[2].x = hge->System_GetState(HGE_SCREENWIDTH) - 2;
    q.v[2].y = hge->System_GetState(HGE_SCREENHEIGHT) - 4;
    q.v[3].x = 1;
    q.v[3].y = hge->System_GetState(HGE_SCREENHEIGHT) - 4;
    q.v[0].col = q.v[1].col = 0xFF2a2a2a;
    q.v[2].col = q.v[3].col = 0xFF151515;
    hge->Gfx_RenderQuad(&q);

    if (hParser != NULL) {
        int wx = -1, wy = -1, tx = -1, ty = -1;
        if (conMain->getWidgetAt(mx, my) == vPort->GetWidget()) {
            wx = Scr2WrdX(GetActivePlane(), mx);
            wy = Scr2WrdY(GetActivePlane(), my);
            tx = (wx / GetActivePlane()->GetTileWidth()) % GetActivePlane()->GetPlaneWidth();
            ty = (wy / GetActivePlane()->GetTileHeight()) % GetActivePlane()->GetPlaneHeight();
        }
        int x = butMicroObjectCB->getX() + 10 + butMicroObjectCB->getWidth();
        GV->fntMyriad13->SetColor(0xFFa1a1a1);
        const char *text = GETL2S("Various", "TilesOnScreen");
        GV->fntMyriad13->Render(x, hge->System_GetState(HGE_SCREENHEIGHT) - 26, HGETEXT_LEFT, text, 0);
        x += GV->fntMyriad13->GetStringWidth(text);
        GV->fntMyriad13->SetColor(0xFFeeeeee);
        GV->fntMyriad13->printf(x + 50, hge->System_GetState(HGE_SCREENHEIGHT) - 26, HGETEXT_RIGHT, "%d", 0,
                                iTilesOnScreen);
        x += 55;
        hge->Gfx_RenderLine(x - 3, hge->System_GetState(HGE_SCREENHEIGHT) - 30, x - 3,
                            hge->System_GetState(HGE_SCREENHEIGHT) - 3, GV->colLineDark);
        hge->Gfx_RenderLine(x - 2, hge->System_GetState(HGE_SCREENHEIGHT) - 30, x - 2,
                            hge->System_GetState(HGE_SCREENHEIGHT) - 3, GV->colLineBright);

        text = GETL2S("Various", "ObjectsOnScreen");
        GV->fntMyriad13->SetColor(0xFFa1a1a1);
        GV->fntMyriad13->Render(x, hge->System_GetState(HGE_SCREENHEIGHT) - 26, HGETEXT_LEFT, text, 0);
        x += GV->fntMyriad13->GetStringWidth(text);
        GV->fntMyriad13->SetColor(0xFFeeeeee);
        GV->fntMyriad13->printf(x + 50, hge->System_GetState(HGE_SCREENHEIGHT) - 26, HGETEXT_RIGHT, "%d", 0,
                                iObjectsOnScreen);
        x += 55;
        hge->Gfx_RenderLine(x - 3, hge->System_GetState(HGE_SCREENHEIGHT) - 30, x - 3,
                            hge->System_GetState(HGE_SCREENHEIGHT) - 3, GV->colLineDark);
        hge->Gfx_RenderLine(x - 2, hge->System_GetState(HGE_SCREENHEIGHT) - 30, x - 2,
                            hge->System_GetState(HGE_SCREENHEIGHT) - 3, GV->colLineBright);

        text = GETL2S("Various", "WorldMousePosition");
        GV->fntMyriad13->SetColor(0xFFa1a1a1);
        GV->fntMyriad13->Render(x, hge->System_GetState(HGE_SCREENHEIGHT) - 26, HGETEXT_LEFT, text, 0);
        x += GV->fntMyriad13->GetStringWidth(text);
        GV->fntMyriad13->SetColor(0xFFeeeeee);
        GV->fntMyriad13->printf(x + 85, hge->System_GetState(HGE_SCREENHEIGHT) - 26, HGETEXT_RIGHT, "%dx%d", 0, wx, wy);
        x += 90;
        hge->Gfx_RenderLine(x - 3, hge->System_GetState(HGE_SCREENHEIGHT) - 30, x - 3,
                            hge->System_GetState(HGE_SCREENHEIGHT) - 3, GV->colLineDark);
        hge->Gfx_RenderLine(x - 2, hge->System_GetState(HGE_SCREENHEIGHT) - 30, x - 2,
                            hge->System_GetState(HGE_SCREENHEIGHT) - 3, GV->colLineBright);

        text = GETL2S("Various", "TileMousePosition");
        GV->fntMyriad13->SetColor(0xFFa1a1a1);
        GV->fntMyriad13->Render(x, hge->System_GetState(HGE_SCREENHEIGHT) - 26, HGETEXT_LEFT, text, 0);
        x += GV->fntMyriad13->GetStringWidth(text);
        GV->fntMyriad13->SetColor(0xFFeeeeee);
        GV->fntMyriad13->printf(x + 65, hge->System_GetState(HGE_SCREENHEIGHT) - 26, HGETEXT_RIGHT, "%dx%d", 0, tx, ty);
        x += 70;
        hge->Gfx_RenderLine(x - 3, hge->System_GetState(HGE_SCREENHEIGHT) - 30, x - 3,
                            hge->System_GetState(HGE_SCREENHEIGHT) - 3, GV->colLineDark);
        hge->Gfx_RenderLine(x - 2, hge->System_GetState(HGE_SCREENHEIGHT) - 30, x - 2,
                            hge->System_GetState(HGE_SCREENHEIGHT) - 3, GV->colLineBright);

        GV->fntMyriad13->Render(x, hge->System_GetState(HGE_SCREENHEIGHT) - 26, HGETEXT_LEFT, "FPS:", 0);
        GV->fntMyriad13->SetColor(0xFFa1a1a1);
        x += GV->fntMyriad13->GetStringWidth("FPS:", 0);
        GV->fntMyriad13->SetColor(0xFFeeeeee);
        GV->fntMyriad13->printf(x + 30, hge->System_GetState(HGE_SCREENHEIGHT) - 26, HGETEXT_RIGHT, "%d", 0,
                                hge->Timer_GetFPS());
        x += 35;

        GV->fntMyriad13->SetColor(0xFFa1a1a1);
        GV->fntMyriad13->Render(hge->System_GetState(HGE_SCREENWIDTH) - 195,
                                hge->System_GetState(HGE_SCREENHEIGHT) - 26, HGETEXT_RIGHT, GETL2S("Various", "Zoom"),
                                0);
        GV->fntMyriad13->SetColor(0xFFeeeeee);
        GV->fntMyriad13->printf(hge->System_GetState(HGE_SCREENWIDTH) - 167,
                                hge->System_GetState(HGE_SCREENHEIGHT) - 26, HGETEXT_RIGHT, "%.1fx", 0, fZoom);

        /*GV->fntMyriad10->printf(hge->System_GetState(HGE_SCREENWIDTH)-350, hge->System_GetState(HGE_SCREENHEIGHT)-18, HGETEXT_RIGHT,
								GETL(Lang_InfoBar), 0,
								fZoom, wx, wy, tx, ty, hge->Timer_GetFPS());*/
    }
#ifdef SHOWMEMUSAGE
                                                                                                                            GV->UpdateMemUsage();
	EnterCriticalSection(&GV->csMemUsage);
	//GV->fntMyriad13->printf(840, 25, HGETEXT_LEFT, "~l~Mem: ~y~%s~l~MB", 0, GV->szMemUsage);
	LeaveCriticalSection(&GV->csMemUsage);
#endif

    hge->Gfx_RenderLine(1, 0, 1, hge->System_GetState(HGE_SCREENHEIGHT), GV->colOutline);
    hge->Gfx_RenderLine(hge->System_GetState(HGE_SCREENWIDTH), 0, hge->System_GetState(HGE_SCREENWIDTH),
                        hge->System_GetState(HGE_SCREENHEIGHT), GV->colOutline);

    hge->Gfx_RenderLine(0, hge->System_GetState(HGE_SCREENHEIGHT), hge->System_GetState(HGE_SCREENWIDTH),
                        hge->System_GetState(HGE_SCREENHEIGHT), GV->colOutline);
}

void State::EditingWW::RenderArrow(int x, int y, int x2, int y2, bool finished, bool setcolors) {
    float len = sqrt(pow(x2 - x, 2) + pow(y2 - y, 2));

    if (setcolors) {
        GV->sprArrowVerticalM->SetColor(0x77FFFFFF);
        GV->sprArrowVerticalU->SetColor(0x77FFFFFF);
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
    if (len < 40.0f * fZoom) {
        GV->sprArrowVerticalU->RenderEx(x, y, rot, fZoom);
        return;
    }
    len -= 37.0f * fZoom;
    vec.x = 0;
    vec.y = len;
    vec.Rotate(rot);
    rot *= -1;
    rot -= M_PI;

    GV->sprArrowVerticalM->RenderEx(x, y, rot, fZoom, len / 128.0f);
    GV->sprArrowVerticalU->RenderEx(x - vec.x, y + vec.y, rot, fZoom);
}

bool State::EditingWW::Render() {
    try {
        gui->draw();
    }
    catch (gcn::Exception &exc) {
        GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(),
                            exc.getLine());
    }

    if (_isOnTop()) {
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        if (conMain->getWidgetAt(mx, my) == NULL) {
            if (my >= 3 && my < 24 && (mx > hge->System_GetState(HGE_SCREENWIDTH) - 5 - 20 - 22 &&
                                       mx < hge->System_GetState(HGE_SCREENWIDTH) - 5 - 20 - 2 ||
                                       mx > hge->System_GetState(HGE_SCREENWIDTH) - 5 - 20 &&
                                       mx < hge->System_GetState(HGE_SCREENWIDTH) - 5)) //caption control icons
                bShowHand = 1;
        } else {
            gcn::Widget *w = conMain, *last = NULL;
            while (w != NULL) {
                last = w;
                int x, y;
                w->getAbsolutePosition(x, y);
                w = w->getWidgetAt(mx - x, my - y);
            }
            if (last != NULL)
                if (last->showHand()) bShowHand = 1;
        }

        if (butMicroTileCB->mouseOver() || bForceTileClipbPreview)
            RenderTileClipboardPreview();
        else if (butMicroObjectCB->mouseOver() || bForceObjectClipbPreview)
            RenderObjectClipboardPreview();

        hInvCtrl->DrawDraggedObject();
        if (GV->StateMgr->IsAppFocused())
            GV->IF->Render(bShowHand, dwCursorColor);
        bShowHand = 0;
        dwCursorColor = 0xFFFFFFFF;
        GV->Console->Render();
    } else {
        hgeQuad q;
        q.tex = 0;
        q.blend = BLEND_DEFAULT;
        q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;
        q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = 0x77000000;

        q.v[0].x = 0;
        q.v[0].y = 0;
        q.v[1].x = hge->System_GetState(HGE_SCREENWIDTH);
        q.v[1].y = 0;
        q.v[2].x = q.v[1].x;
        q.v[2].y = hge->System_GetState(HGE_SCREENHEIGHT);
        q.v[3].x = 0;
        q.v[3].y = q.v[2].y;

        hge->Gfx_RenderQuad(&q);
    }

    if (fade_iAction < 2) {
        hgeQuad q;
        q.tex = 0;
        q.blend = BLEND_DEFAULT;
        q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;
        q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = 0xFF000000;
        q.v[0].x = 0;
        q.v[0].y = 24;
        q.v[1].x = hge->System_GetState(HGE_SCREENWIDTH);
        q.v[1].y = 24;
        q.v[2].x = hge->System_GetState(HGE_SCREENWIDTH);
        q.v[2].y = hge->System_GetState(HGE_SCREENHEIGHT);
        q.v[3].x = 0;
        q.v[3].y = hge->System_GetState(HGE_SCREENHEIGHT);
        hge->Gfx_RenderQuad(&q);

        GV->sprLogoBig->Render(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 151,
                               hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 38);
        GV->fntMyriad10->Render(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 35,
                                hge->System_GetState(HGE_SCREENHEIGHT) / 2 + 21, HGETEXT_RIGHT, WA_VERSTRING, 0);

        if (fade_iAction == 0) {
            hgeQuad q;
            q.tex = 0;
            q.blend = BLEND_DEFAULT;
            q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;
            q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = ARGB(int(fade_fAlpha), 0, 0, 0);
            q.v[0].x = 0;
            q.v[0].y = 24;
            q.v[1].x = hge->System_GetState(HGE_SCREENWIDTH);
            q.v[1].y = 24;
            q.v[2].x = hge->System_GetState(HGE_SCREENWIDTH);
            q.v[2].y = hge->System_GetState(HGE_SCREENHEIGHT);
            q.v[3].x = 0;
            q.v[3].y = hge->System_GetState(HGE_SCREENHEIGHT);

            hge->Gfx_RenderQuad(&q);
        }
        return 0;
    } else if (fade_iAction == 2) {
        //hge->Gfx_Clear(0xFF000000);

        if (hge->System_GetState(HGE_SCREENWIDTH) > 1102) {
            GV->sprLogoBig->SetColor(ARGB(255 - int(fade_fAlpha), 255, 255, 255));
            GV->fntMyriad10->SetColor(ARGB(255 - int(fade_fAlpha), 255, 255, 255));
            GV->sprLogoBig->Render(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 151,
                                   hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 38);
            GV->fntMyriad10->Render(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 35,
                                    hge->System_GetState(HGE_SCREENHEIGHT) / 2 + 21, HGETEXT_RIGHT, WA_VERSTRING, 0);
            GV->sprLogoBig->SetColor(0xFFFFFFFF);
            GV->fntMyriad10->SetColor(0xFFFFFFFF);
        }

        hgeQuad q;
        q.tex = 0;
        q.blend = BLEND_DEFAULT;
        q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;
        q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = ARGB(255 - int(fade_fAlpha), 0, 0, 0);
        q.v[0].x = 0;
        q.v[0].y = 24;
        q.v[1].x = hge->System_GetState(HGE_SCREENWIDTH);
        q.v[1].y = 24;
        q.v[2].x = hge->System_GetState(HGE_SCREENWIDTH);
        q.v[2].y = hge->System_GetState(HGE_SCREENHEIGHT);
        q.v[3].x = 0;
        q.v[3].y = hge->System_GetState(HGE_SCREENHEIGHT);
        hge->Gfx_RenderQuad(&q);

        if (hge->System_GetState(HGE_SCREENWIDTH) < 1102) {
            GV->sprLogoBig->Render(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 151,
                                   hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 38);
            GV->fntMyriad10->Render(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 35,
                                    hge->System_GetState(HGE_SCREENHEIGHT) / 2 + 21, HGETEXT_RIGHT, WA_VERSTRING, 0);
        }
    }
#ifdef CONF_WATERMARK
                                                                                                                            GV->fntMyriad80->SetColor(ARGB(64, 255, 255, 255));
	GV->fntMyriad80->Render(vPort->GetX() + vPort->GetWidth() - 5, vPort->GetY() + vPort->GetHeight() - 80, HGETEXT_RIGHT, CONF_WATERMARK);
#endif
    return 0;
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
        GV->fntMyriad13->printf(fx + 135, fy + i * 140 - fSliVal, HGETEXT_LEFT, "%s: ~y~%d ~l~(#~y~%i~l~)", 0,
                                GETL2S("ObjectSearch", "ID"), obj->GetParam(WWD::Param_ID), i + 1);
        int yoffset = 20;
        if (strlen(obj->GetName()) > 0) {
            GV->fntMyriad13->printf(fx + 135, fy + i * 140 - fSliVal + yoffset, HGETEXT_LEFT, "%s: ~y~%s~l~", 0,
                                    GETL2S("ObjectSearch", "Name"), obj->GetName());
            yoffset += 20;
        }
        if (strlen(obj->GetLogic()) > 0) {
            GV->fntMyriad13->printf(fx + 135, fy + i * 140 - fSliVal + yoffset, HGETEXT_LEFT, "%s: ~y~%s~l~", 0,
                                    GETL2S("ObjectSearch", "Logic"), obj->GetLogic());
            yoffset += 20;
        }
        if (strlen(obj->GetImageSet()) > 0) {
            GV->fntMyriad13->printf(fx + 135, fy + i * 140 - fSliVal + yoffset, HGETEXT_LEFT, "%s: ~y~%s~l~", 0,
                                    GETL2S("ObjectSearch", "ImageSet"), obj->GetImageSet());
            yoffset += 20;
        }
        if (strlen(obj->GetAnim()) > 0) {
            GV->fntMyriad13->printf(fx + 135, fy + i * 140 - fSliVal + yoffset, HGETEXT_LEFT, "%s: ~y~%s~l~", 0,
                                    GETL2S("ObjectSearch", "Anim"), obj->GetAnim());
            yoffset += 20;
        }
        GV->fntMyriad13->printf(fx + 135, fy + i * 140 - fSliVal + yoffset, HGETEXT_LEFT, "%s: ~y~%d~l~x~y~%d~l~", 0,
                                GETL2S("ObjectSearch", "Pos"), obj->GetParam(WWD::Param_LocationX),
                                obj->GetParam(WWD::Param_LocationY));
        yoffset += 20;

        int butx = fx + 135, buty = fy + i * 140 - fSliVal + 97, butw =
                GV->fntMyriad13->GetStringWidth(GETL2S("ObjectSearch", "GoToObject")) + 8;
        if (bHasMouse && mx > butx && mx < butx + butw && my > buty && my < buty + 33 && my > fy && my < fy + 386) {
            if (hge->Input_GetKeyState(HGEK_LBUTTON)) {
                GV->gcnParts.sprButBarP[0][0]->RenderStretch(butx, buty, butx + 4, buty + 33);
                GV->gcnParts.sprButBarP[0][1]->RenderStretch(butx + 4, buty, butx + butw - 4, buty + 33);
                GV->gcnParts.sprButBarP[0][2]->RenderStretch(butx + butw - 4, buty, butx + butw, buty + 33);
            } else {
                GV->gcnParts.sprButBarH[0][0]->RenderStretch(butx, buty, butx + 4, buty + 33);
                GV->gcnParts.sprButBarH[0][1]->RenderStretch(butx + 4, buty, butx + butw - 4, buty + 33);
                GV->gcnParts.sprButBarH[0][2]->RenderStretch(butx + butw - 4, buty, butx + butw, buty + 33);
            }
            if (hge->Input_KeyUp(HGEK_LBUTTON)) {
                WWD::Object *obj = GetActivePlane()->GetObjectByIterator(vObjSearchResults[i].first);
                vObjectsPicked.clear();
                vObjectsPicked.push_back(obj);
                fCamX = GetUserDataFromObj(obj)->GetX() - vPort->GetWidth() / 2 * fZoom;
                fCamY = GetUserDataFromObj(obj)->GetY() - vPort->GetHeight() / 2 * fZoom;
                vPort->MarkToRedraw(1);
            }
        } else {
            GV->gcnParts.sprButBar[0][0]->RenderStretch(butx, buty, butx + 4, buty + 33);
            GV->gcnParts.sprButBar[0][1]->RenderStretch(butx + 4, buty, butx + butw - 4, buty + 33);
            GV->gcnParts.sprButBar[0][2]->RenderStretch(butx + butw - 4, buty, butx + butw, buty + 33);
        }
        GV->fntMyriad13->Render(butx + butw / 2, buty + 8, HGETEXT_CENTER, GETL2S("ObjectSearch", "GoToObject"), 0);

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
    spr->SetFlip(GetUserDataFromObj(hObj)->GetFlipX(), GetUserDataFromObj(
                         hObj)->GetFlipY(),/*(list[i]->GetDrawFlags()&WWD::Flag_dr_Mirror), (list[i]->GetDrawFlags()&WWD::Flag_dr_Invert),*/
                 1);
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
            if (hPl->GetTile(x, y)->IsInvisible()) {
                continue;
            } else if (hPl->GetTile(x, y)->IsFilled()) {
                hgeQuad q;
                q.blend = BLEND_DEFAULT;
                q.tex = 0;
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
            } else {
                hgeSprite *spr;
                cTile *p = hTileset->GetTile(hPl->GetImageSet(0), hPl->GetTile(x, y)->GetID());
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
                 (x % hPl->GetPlaneWidth()) == hPl->GetPlaneWidth() - 2 &&
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
             (y % hPl->GetPlaneHeight()) == hPl->GetPlaneHeight() - 2 &&
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
            vPort->MarkToRedraw(1);
        }
    }
    hgeQuad q;
    q.blend = BLEND_DEFAULT;
    q.tex = 0;
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
        GV->fntMyriad13->printf(baseX + 5, baseY + 229, HGETEXT_LEFT, "~l~%s: #~y~%d", 0, GETL(Lang_ActualFillColor),
                                GetActivePlane()->GetFillColor());
        previewcol = GetActivePlane()->GetFillColor();
    } else {
        GV->fntMyriad13->printf(baseX + 5, baseY + 229, HGETEXT_LEFT, "~l~%s: #~y~%d", 0, GETL(Lang_SelectedFillColor),
                                sely * 16 + selx);
        previewcol = sely * 16 + selx;
    }
    SHR::SetQuad(&q, hDataCtrl->GetPalette()->GetColor(previewcol), baseX + 161, baseY + 229, baseX + 196, baseY + 249);
    hge->Gfx_RenderQuad(&q);
}

void State::EditingWW::DrawPlaneOverlay(WWD::Plane *hPl) {
    if (hPl == GetActivePlane() && iMode == EWW_MODE_TILE &&
        (bObjDragSelection || iTileSelectX1 != -1 && iTileSelectY1 != -1)) {
        int x1, y1, x2, y2;
        if (bObjDragSelection) {
            float mx, my;
            hge->Input_GetMousePos(&mx, &my);
            mx = Scr2WrdX(hPl, mx);
            my = Scr2WrdY(hPl, my);
            if (iObjDragOrigX == mx && iObjDragOrigY == my) return;
            x1 = int(iObjDragOrigX / GetActivePlane()->GetTileWidth()) * GetActivePlane()->GetTileWidth();
            y1 = int(iObjDragOrigY / GetActivePlane()->GetTileHeight()) * GetActivePlane()->GetTileHeight();
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
        hgeQuad q;
        q.blend = BLEND_DEFAULT;
        q.tex = 0;
        q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 0;
        q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = 0x77ffcc00;
        //vPort->ClipScreen();
        q.v[0].x = x1;
        q.v[0].y = y1;
        q.v[1].x = x2;
        q.v[1].y = y1;
        q.v[2].x = x2;
        q.v[2].y = y2;
        q.v[3].x = x1;
        q.v[3].y = y2;
        hge->Gfx_RenderQuad(&q);
        hge->Gfx_RenderLine(q.v[0].x, q.v[0].y, q.v[1].x, q.v[1].y, 0xFFFF0000);
        hge->Gfx_RenderLine(q.v[1].x, q.v[1].y, q.v[2].x, q.v[2].y, 0xFFFF0000);
        hge->Gfx_RenderLine(q.v[2].x, q.v[2].y, q.v[3].x, q.v[3].y, 0xFFFF0000);
        hge->Gfx_RenderLine(q.v[3].x, q.v[3].y, q.v[0].x, q.v[0].y, 0xFFFF0000);
    }
}

void State::EditingWW::ViewportOverlay() {
#ifdef BUILD_DEBUG
                                                                                                                            if (bShowConsole) {
		for (int i = 1; i < (int(vPort->GetHeight() / 15) + 1); i++) {
			const char * line = GV->Console->GetLine(i);
			if (line == NULL) break;
			GV->fntSystem17->SetColor(0xFFFFFFFF);
			GV->fntSystem17->Render(5, vPort->GetY() + vPort->GetHeight() - i * 15, HGETEXT_LEFT, line, 0);
		}
	}
#endif
}

void State::EditingWW::RenderCloudTip(int x, int y, int w, int h, int ax, int ay) {
    hgeQuad q;
    q.blend = BLEND_DEFAULT;
    q.tex = 0;
    SHR::SetQuad(&q, GV->colBase, x + 10, y, x + w - 10, y + h);
    hge->Gfx_RenderQuad(&q);
    SHR::SetQuad(&q, GV->colBase, x, y + 10, x + 10, y + h - 10);
    hge->Gfx_RenderQuad(&q);
    SHR::SetQuad(&q, GV->colBase, x + w - 10, y + 10, x + w, y + h - 10);
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
    hge->Gfx_RenderLine(x + 10, y + h, x + 25, y + h, 0xFF000000);
    hge->Gfx_RenderLine(x + 45, y + h, x + w - 10, y + h, 0xFF000000);
    hge->Gfx_RenderLine(x + 10, y, x + w - 10, y, 0xFF000000);
    hge->Gfx_RenderLine(x, y + 10, x, y + h - 10, 0xFF000000);
    hge->Gfx_RenderLine(x + w, y + 10, x + w, y + h - 10, 0xFF000000);
    GV->sprCloudTip[0]->Render(x, y);
    GV->sprCloudTip[1]->Render(x + w - 10, y);
    GV->sprCloudTip[2]->Render(x, y + h - 10);
    GV->sprCloudTip[3]->Render(x + w - 10, y + h - 10);
}

void State::EditingWW::RenderTileClipboardPreview() {
    if (MDI->GetActiveDoc() == NULL) return;
    if (MDI->GetActiveDoc()->hTileClipboard == NULL) {
        int len = GV->fntMyriad13->GetStringWidth(GETL2S("ClipboardPreview", "TileClipboardEmpty"));
        RenderCloudTip(10, hge->System_GetState(HGE_SCREENHEIGHT) - 70, len + 20, 20, butMicroTileCB->getX() + 9,
                       butMicroTileCB->getY() - 3);
        GV->fntMyriad13->Render(20, hge->System_GetState(HGE_SCREENHEIGHT) - 70, HGETEXT_LEFT,
                                GETL2S("ClipboardPreview", "TileClipboardEmpty"), 0);
        return;
    }
    int cbw = MDI->GetActiveDoc()->iTileCBw,
            cbh = MDI->GetActiveDoc()->iTileCBh;
    int tilew = 64, tileh = 64;
    if (tilew * cbw > 400 || tilew * cbh > 400) {
        tilew = 400 / (cbw > cbh ? cbw : cbh);
        tileh = 400 / (cbw > cbh ? cbw : cbh);
    }
    int px = 20, py = hge->System_GetState(HGE_SCREENHEIGHT) - (tileh * cbh + 20 + 50) + 10;
    RenderCloudTip(px - 10, py - 10, tilew * cbw + 20, tileh * cbh + 20, butMicroTileCB->getX() + 9,
                   butMicroTileCB->getY() - 3);
    for (int y = 0; y < cbh; y++)
        for (int x = 0; x < cbw; x++) {
            if (MDI->GetActiveDoc()->hTileClipboard[x][y].IsInvisible()) continue;
            else if (MDI->GetActiveDoc()->hTileClipboard[x][y].IsFilled()) {

            } else {
                hgeSprite *spr = hTileset->GetTile(MDI->GetActiveDoc()->hTileClipboardImageSet,
                                                   MDI->GetActiveDoc()->hTileClipboard[x][y].GetID())->GetImage();
                spr->SetColor(0xFFFFFFFF);
                spr->SetFlip(0, 0);
                spr->SetHotSpot(0, 0);
                spr->RenderStretch(px + tilew * x, py + tileh * y, px + tilew * (x + 1), py + tileh * (y + 1));
            }
        }
}

/*}else if( plane->GetTile(x,y)->IsFilled() ){
 hgeQuad q;
 q.blend = BLEND_DEFAULT;
 q.tex = 0;
 q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = SETA(hPalette->GetColor(plane->GetFillColor()), GETA(col));
 q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;
 q.v[0].x = posx; q.v[0].y = posy;
 q.v[1].x = q.v[0].x+plane->GetTileWidth()*fZoom; q.v[1].y = q.v[0].y;
 q.v[2].x = q.v[1].x; q.v[2].y = q.v[0].y+plane->GetTileHeight()*fZoom;
 q.v[3].x = q.v[0].x; q.v[3].y = q.v[2].y;
 hge->Gfx_RenderQuad(&q);*/

void State::EditingWW::RenderObjectClipboardPreview() {
    if (MDI->GetActiveDoc() == NULL) return;
    if (vObjectClipboard.size() == 0) {
        int len = GV->fntMyriad13->GetStringWidth(GETL2S("ClipboardPreview", "ObjectClipboardEmpty"));
        RenderCloudTip(10, hge->System_GetState(HGE_SCREENHEIGHT) - 70, len + 20, 20, butMicroObjectCB->getX() + 9,
                       butMicroObjectCB->getY() - 3);
        GV->fntMyriad13->Render(20, hge->System_GetState(HGE_SCREENHEIGHT) - 70, HGETEXT_LEFT,
                                GETL2S("ClipboardPreview", "ObjectClipboardEmpty"), 0);
        return;
    }
    int sizeX = 0, sizeY = 0;
    WWD::Object *horBase = NULL, *verBase = NULL;
    {
        int minx = -1, miny = -1, maxx = -1, maxy = -1;
        for (int i = 0; i < vObjectClipboard.size(); i++) {
            hgeSprite *spr = SprBank->GetObjectSprite(vObjectClipboard[i]);
            int minvalx = vObjectClipboard[i]->GetParam(WWD::Param_LocationX) - spr->GetWidth() / 2,
                    minvaly = vObjectClipboard[i]->GetParam(WWD::Param_LocationY) - spr->GetHeight() / 2;
            if (minvalx < minx || minx == -1) horBase = vObjectClipboard[i];
            if (minvaly < miny || miny == -1) verBase = vObjectClipboard[i];
            minx = minx == -1 ? minvalx : std::min(minx, minvalx);
            miny = miny == -1 ? minvaly : std::min(miny, minvaly);
            maxx = maxx == -1 ? vObjectClipboard[i]->GetParam(WWD::Param_LocationX) + spr->GetWidth() / 2 : std::max(
                    maxx, int(vObjectClipboard[i]->GetParam(WWD::Param_LocationX) + spr->GetWidth() / 2));
            maxy = maxy == -1 ? vObjectClipboard[i]->GetParam(WWD::Param_LocationY) + spr->GetHeight() / 2 : std::max(
                    maxy, int(vObjectClipboard[i]->GetParam(WWD::Param_LocationY) + spr->GetHeight() / 2));
        }
        sizeX = maxx - minx;
        sizeY = maxy - miny;
    }
    float fScale = 1.0f;
    if (sizeX > 400 || sizeY > 400) {
        fScale = 400.0f / float(sizeX > sizeY ? sizeX : sizeY);
        fScale = 400.0f / float(sizeX > sizeY ? sizeX : sizeY);
    }
    sizeX *= fScale;
    sizeY *= fScale;
    int cx = 20, cy = hge->System_GetState(HGE_SCREENHEIGHT) - sizeY - 50 + 10;
    RenderCloudTip(10, cy - 10, sizeX + 20, sizeY + 20, butMicroObjectCB->getX() + 9, butMicroObjectCB->getY() - 3);
    int baseX = horBase->GetParam(WWD::Param_LocationX) - SprBank->GetObjectSprite(horBase)->GetWidth() / 2,
            baseY = verBase->GetParam(WWD::Param_LocationY) - SprBank->GetObjectSprite(verBase)->GetHeight() / 2;
    for (int i = 0; i < vObjectClipboard.size(); i++) {
        hgeSprite *spr = SprBank->GetObjectSprite(vObjectClipboard[i]);
        int offx = vObjectClipboard[i]->GetParam(WWD::Param_LocationX) - baseX, offy =
                vObjectClipboard[i]->GetParam(WWD::Param_LocationY) - baseY;
        spr->SetColor(0xFFFFFFFF);
        spr->SetFlip(GetUserDataFromObj(vObjectClipboard[i])->GetFlipX(),
                     GetUserDataFromObj(vObjectClipboard[i])->GetFlipY());
        spr->RenderEx(cx + offx * fScale, cy + offy * fScale, 0, fScale);
    }
}

void State::EditingWW::DrawWelcomeScreen() {
    int dx, dy;
    winWelcome->getAbsolutePosition(dx, dy);
    hge->Gfx_SetClipping();

    for (int i = 0; i < 4; i++) {
        GV->sprIcons128[i]->Render(dx + i * 150 + 11, dy);
        /*if( i != 0 ){
		 hge->Gfx_RenderLine(dx+i*150, dy+24, dx+i*150, dy+219, GV->colLineDark);
		 hge->Gfx_RenderLine(dx+i*150+1, dy+24, dx+i*150+1, dy+219, GV->colLineBright);
		}*/
    }

    hge->Gfx_RenderLine(dx + 1, dy + 180, dx + winWelcome->getWidth() - 1, dy + 180, GV->colLineDark);
    hge->Gfx_RenderLine(dx + 1, dy + 181, dx + winWelcome->getWidth() - 1, dy + 181, GV->colLineBright);

    /*if( conRecentFiles->isVisible() ){
	 int rfx, rfy;
	 conRecentFiles->getAbsolutePosition(rfx, rfy);

	}*/
}

void State::EditingWW::RenderAreaRect(WWD::Rect r, WWD::Rect dr, bool bClip, DWORD hwCol, bool bFill, DWORD hwFillCol) {
    if (bClip)
        hge->Gfx_SetClipping(r.x1, r.y1, r.x2 - r.x1, r.y2 - r.y1);
    GV->sprDottedLineHorizontal->SetColor(hwCol);
    GV->sprDottedLineVertical->SetColor(hwCol);

    if (bFill) {
        hgeQuad q;
        q.tex = 0;
        q.blend = BLEND_DEFAULT;
        SHR::SetQuad(&q, hwFillCol, r.x1, r.y1, r.x2, r.y2);
        hge->Gfx_RenderQuad(&q);
    }

    if (dr.x1 || dr.x2)
        for (int y = r.y1; y < r.y2; y += 120) {
            if (dr.x1) GV->sprDottedLineVertical->Render(r.x1, y);
            if (dr.x2) GV->sprDottedLineVertical->Render(r.x2 - 5, y);
        }

    if (dr.y1 || dr.y2)
        for (int x = r.x1; x < r.x2; x += 120) {
            if (dr.y1) GV->sprDottedLineHorizontal->Render(x, r.y1);
            if (dr.y2) GV->sprDottedLineHorizontal->Render(x, r.y2 - 5);
        }
}

void State::EditingWW::DrawCrashRetrieve() {
    int dx, dy;
    conCrashRetrieve->getAbsolutePosition(dx, dy);
    GV->fntMyriad13->SetColor(0xFFa1a1a1);
    GV->fntMyriad13->Render(dx + 5, dy + 3, HGETEXT_LEFT, GETL2S("WinCrashRetrieve", "Label"), 0);
    int c = 0;
    for (int i = 0; i < 10; i++) {
        if (szCrashRetrieve[i] == NULL) break;
        if (iCrashRetrieveIcon[i] < 50)
            GV->sprGamesSmall[iCrashRetrieveIcon[i]]->Render(dx + 2, dy + 25 + 25 * i);
        else
            GV->sprLevelsMicro16[iCrashRetrieveIcon[i] - 51]->Render(dx + 2, dy + 25 + 25 * i);
        GV->fntMyriad13->Render(dx + 20, dy + 25 + 25 * i, HGETEXT_LEFT, szCrashRetrieve[i], 0);
        c++;
    }
    if (szCrashRetrieveMore != NULL)
        GV->fntMyriad13->Render(dx + 5, dy + 25 + 25 * c, HGETEXT_LEFT, szCrashRetrieveMore, 0);
}

void State::EditingWW::DrawTilePicker() {
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    bool hasmouse = conMain->getWidgetAt(mx, my) == winTilePicker;
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
        /*hgeQuad q;
		q.blend = BLEND_DEFAULT;
		q.tex = 0;
		SHR::SetQuad(&q, 0xFF3e3e3e, itpx, itpy, itpx+230, itpy+slitpiPicker->getHeight());
		hge->Gfx_RenderQuad(&q);*/
        hge->Gfx_SetClipping(itpx, itpy, 230, slitpiPicker->getHeight());

        if (mx > itpx && mx < itpx + 230 && my > itpy && my < itpy + slitpiPicker->getHeight()) {
            slitpiPicker->setValue(slitpiPicker->getValue() + 100 * hge->Input_GetMouseWheel());
        }

        int scroll = slitpiPicker->getScaleEnd() - slitpiPicker->getValue();
        if (iActiveTool == EWW_TOOL_BRUSH) {
            int ioldsel = iTilePicked;
            int loopst = scroll / 57, loopen = slitpiPicker->getHeight() / 57 + 2;
            if (set->GetBrushesCount() == 0) {
                GV->fntMyriad13->SetColor(0xFF000000);
                GV->fntMyriad13->printf(itpx + 115, itpy + slitpiPicker->getHeight() / 2, HGETEXT_CENTER,
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
                if (hasmouse &&
                    (mx > drx && my > dry && mx < drx + 48 && my < dry + 48 ||
                     mx > drx + 55 && mx < drx + 55 + GV->fntMyriad13->GetStringWidth(brush->GetName()) &&
                     my > dry + 14 && my < dry + 34)
                    && my > itpy && my < itpy + slitpiPicker->getHeight()) {
                    colBorder = TILE_HIGHLIGHT_COLOR;
                    bShowHand = 1;
                    if (hge->Input_KeyDown(HGEK_LBUTTON))
                        iTilePicked = i;
                } else if (i == iTilePicked) {
                    colBorder = TILE_PICK_COLOR;
                }
                hge->Gfx_RenderLine(drx - 1, dry, drx + 49, dry, colBorder);
                hge->Gfx_RenderLine(drx, dry + 49, drx + 49, dry + 49, colBorder);
                hge->Gfx_RenderLine(drx, dry, drx, dry + 49, colBorder);
                hge->Gfx_RenderLine(drx + 49, dry, drx + 49, dry + 49, colBorder);

                GV->fntMyriad13->SetColor(colBorder == GV->colLineBright ? 0xFF000000 : colBorder);
                GV->fntMyriad13->Render(drx + 55, dry + 14, HGETEXT_LEFT, brush->GetName(), 0);
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
                    hgeQuad q;
                    q.tex = 0;
                    q.blend = BLEND_DEFAULT;
                    SHR::SetQuad(&q, hDataCtrl->GetPalette()->GetColor(GetActivePlane()->GetFillColor()), drx, dry,
                                 drx + 48, dry + 48);
                    hge->Gfx_RenderQuad(&q);
                } else {
                    cTile *tile = set->GetTileByIterator(i);
                    tile->GetImage()->SetHotSpot(0, 0);
                    tile->GetImage()->SetFlip(0, 0);
                    tile->GetImage()->SetColor(0xFFFFFFFF);
                    tile->GetImage()->RenderEx(drx, dry, 0, 0.75f);
                    if (cbtpiShowProperties->isSelected()) {
                        DrawTileAtrib(tile->GetID(), drx, dry, 0.75f, 0.75f);
                    }
                    if (cbtpiShowTileID->isSelected()) {
                        GV->fntMyriad13->SetColor(0xFF000000);
                        GV->fntMyriad13->printf(drx + 24 + 1, dry + 1 + 32, HGETEXT_CENTER, "%d", 0, tile->GetID());
                        GV->fntMyriad13->SetColor(0xFFFFFFFF);
                        GV->fntMyriad13->printf(drx + 24, dry + 32, HGETEXT_CENTER, "%d", 0, tile->GetID());
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
                if (mx > drx && my > dry && mx < drx + 48 && my < dry + 48 && hasmouse && my > itpy &&
                    my < itpy + slitpiPicker->getHeight()) {
                    colBorder = TILE_HIGHLIGHT_COLOR;
                    bShowHand = 1;
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
        GV->sprShadeBar->RenderStretch(itpx, itpy, itpx + 230, itpy + 6);
        GV->sprShadeBar->SetFlip(0, 1);
        GV->sprShadeBar->RenderStretch(itpx, itpy + slitpiPicker->getHeight() - 6, itpx + 230,
                                       itpy + slitpiPicker->getHeight());
        GV->sprShadeBar->SetFlip(0, 0);
    }

}
