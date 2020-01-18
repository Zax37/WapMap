#include "wViewport.h"

#include "guichan/exception.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "hge.h"

extern HGE * hge;

namespace WIDG
{
    Viewport::Viewport(VpCallback * phCallback, int piCode)
    {
        hCallback = phCallback;
        iCode = piCode;
    }

    void Viewport::draw(Graphics* graphics)
    {
        ClipRectangle rect = graphics->getCurrentClipArea();
        hCallback->Draw(iCode);
        hge->Gfx_SetClipping(rect.x, rect.y, rect.width, rect.height);
    }
}
