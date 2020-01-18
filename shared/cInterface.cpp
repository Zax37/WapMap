#include "cInterface.h"

#include "hge.h"

extern HGE * hge;

SHR::Interface::Interface(guiParts * Parts)
{
    hGfx = Parts;
}

void SHR::Interface::Think()
{

}

void SHR::Interface::Render(bool pbHand, DWORD dwCol)
{
    if( bShowMouse ){
     float mx, my;
     hge->Input_GetMousePos(&mx, &my);
     hgeSprite * spr = pbHand ? hGfx->sprCurHand : hGfx->sprCur;
     spr->SetColor(dwCol);
     spr->Render(mx,my);
    }
}

hgeSprite * SHR::Interface::GetCursor()
{
    if( hGfx->sprCurHand->GetWidth() > hGfx->sprCur->GetWidth() )
     return hGfx->sprCurHand;
    else
     return hGfx->sprCur;
}
