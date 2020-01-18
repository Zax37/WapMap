#include "cWindowBar.h"

namespace SHR
{
 cWinBar::cWinBar()
 {

 }

 cWinBar::~cWinBar()
 {

 }

 void cWinBar::Think()
 {

 }

 void cWinBar::Render()
 {
    GV->gcnParts.sprGcnWinBarL->RenderStretch(0, 0, 2, 24);
    GV->gcnParts.sprGcnWinBarM->RenderStretch(2, 0, hge->System_GetState(HGE_SCREENWIDTH)-2, 24);
    GV->gcnParts.sprGcnWinBarR->RenderStretch(hge->System_GetState(HGE_SCREENWIDTH)-2, 0, hge->System_GetState(HGE_SCREENWIDTH), 24);
    hge->Gfx_RenderLine(0,0,
                        0,hge->System_GetState(HGE_SCREENHEIGHT),
                        0xFF000000);
    hge->Gfx_RenderLine(hge->System_GetState(HGE_SCREENWIDTH),0,
                        hge->System_GetState(HGE_SCREENWIDTH),hge->System_GetState(HGE_SCREENHEIGHT),
                        0xFF000000);
    hge->Gfx_RenderLine(0, hge->System_GetState(HGE_SCREENHEIGHT),
                        hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT),
                        0xFF000000);
 }
};
