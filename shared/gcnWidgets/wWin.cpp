#include "wWin.h"

#include "guichan/exception.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/mouseinput.hpp"

#include "hge.h"

#ifdef WAP_MAP
 #include "../../WapMap/globals.h"
#endif

#ifdef WAP_GRAPHIC
 #include "../../WapGraphic/globals.h"
#endif

#include "../../WapMap/cInterfaceSheet.h"

using namespace gcn;

extern HGE * hge;
extern cInterfaceSheet * _ghGfxInterface;

namespace SHR
{
    Win::Win(guiParts * Parts)
            :mMoved(false)
    {
        setFrameSize(0);
        setPadding(2);
        setTitleBarHeight(16);
        setAlignment(Graphics::CENTER);
        addMouseListener(this);
        setMovable(true);
        setOpaque(0);
        hGfx = Parts;
        bClose = 0;
        bDrawShadow = 1;
        fCloseTimer = 0;
        mMouseOver = 0;
        bHide = 0;
        mAlphaMod = 1.0f;
    }

    Win::Win(guiParts * Parts, const std::string& caption)
            :mMoved(false)
    {
        setCaption(caption);
        setFrameSize(0);
        setPadding(2);
        setTitleBarHeight(16);
        setAlignment(Graphics::CENTER);
        addMouseListener(this);
        setMovable(true);
        setOpaque(0);
        hGfx = Parts;
        bClose = 0;
        bDrawShadow = 1;
        fCloseTimer = 0;
        mMouseOver = 0;
        bHide = 0;
        mAlphaMod = 1.0f;
    }

    Win::~Win()
    {
    }

    void Win::setPadding(unsigned int padding)
    {
        mPadding = padding;
    }

    unsigned int Win::getPadding() const
    {
        return mPadding;
    }

    void Win::setTitleBarHeight(unsigned int height)
    {
        mTitleBarHeight = height;
    }

    unsigned int Win::getTitleBarHeight()
    {
        return mTitleBarHeight;
    }

    void Win::setCaption(const std::string& caption)
    {
        mCaption = caption;
    }

    const std::string& Win::getCaption() const
    {
        return mCaption;
    }

    void Win::setAlignment(Graphics::Alignment alignment)
    {
        mAlignment = alignment;
    }

    Graphics::Alignment Win::getAlignment() const
    {
        return mAlignment;
    }

    void Win::draw(Graphics* graphics)
    {
        gcn::Rectangle d = getChildrenArea();

        if( !bHide && mAlphaMod < 1.0f ){
         mAlphaMod += hge->Timer_GetDelta()*5.0f;
         if( mAlphaMod > 1.0f ) mAlphaMod = 1.0f;
        }else if( bHide && (mAlphaMod > 0.0f || isVisible()) ){
         mAlphaMod -= hge->Timer_GetDelta()*5.0f;
         if( mAlphaMod <= 0.0f ){
          mAlphaMod = 0.0f;
          setVisible(0);
         }
        }

        ClipRectangle rect = graphics->getCurrentClipArea();
        int x, y;
        getAbsolutePosition(x, y);

        if( bClose ){
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        bool bExitFocus = (mMouseOver && mx > x+getWidth()-18 && my > y+5 && mx < x+getWidth()-2 && my < y+21);
        if( bExitFocus && fCloseTimer < 0.2f ){
         fCloseTimer += hge->Timer_GetDelta();
         if( fCloseTimer > 0.2f ) fCloseTimer = 0.2f;
        }else if( !bExitFocus && fCloseTimer > 0.0f ){
         fCloseTimer -= hge->Timer_GetDelta();
         if( fCloseTimer < 0.0f ) fCloseTimer = 0;
        }
        }
        //DrawWindow(x, y, getWidth(), getHeight(), rect.x, rect.y, rect.width, rect.height);

        hge->Gfx_SetClipping();

        //hGfx->sprWindowBG->RenderStretch(x, y, x+getWidth(), y+getHeight());
        hgeQuad quad;
        quad.tex = 0;
        quad.blend = BLEND_DEFAULT;
        quad.v[0].z = quad.v[1].z = quad.v[2].z = quad.v[3].z = 1.0f;
        //quad.v[0].col = quad.v[1].col = quad.v[2].col = quad.v[3].col = GV->colBase;
        quad.v[0].col = quad.v[1].col = SETA(0xFF2f2f2f, getAlpha());
        quad.v[2].col = quad.v[3].col = SETA(0xFF1a1a1a, getAlpha());
        quad.v[0].x = x;            quad.v[0].y = y+4;
        quad.v[1].x = x+getWidth(); quad.v[1].y = y+4;
        quad.v[2].x = x+getWidth(); quad.v[2].y = y+getHeight()-4;
        quad.v[3].x = x;            quad.v[3].y = y+getHeight()-4;
        hge->Gfx_RenderQuad(&quad);

        DWORD defcol = SETA(0xFFFFFFFF, getAlpha());

        for(int i=0;i<4;i++)
         _ghGfxInterface->sprWindow[i]->SetColor(defcol);

        _ghGfxInterface->sprWindow[0]->Render(x, y);
        _ghGfxInterface->sprWindow[1]->Render(x+getWidth()-5, y);

        _ghGfxInterface->sprWindow[2]->Render(x, y+getHeight()-5);
        _ghGfxInterface->sprWindow[3]->Render(x+getWidth()-5, y+getHeight()-5);

        unsigned char galph = getAlpha();


        //upper bar

        quad.v[0].y = quad.v[1].y = y+5;
        quad.v[2].y = quad.v[3].y = y+22;
        quad.v[0].col = quad.v[1].col = SETA(0xFF373737, galph);
        quad.v[2].col = quad.v[3].col = SETA(0xFF282828, galph);
        hge->Gfx_RenderQuad(&quad);

        quad.v[0].col = quad.v[1].col = quad.v[2].col = quad.v[3].col = SETA(0xFF393939, galph);
        quad.v[0].y = quad.v[1].y = y+1;
        quad.v[2].y = quad.v[3].y = y+5;
        quad.v[0].x = quad.v[3].x = x+5;
        quad.v[1].x = quad.v[2].x = x+getWidth()-5;
        hge->Gfx_RenderQuad(&quad);

        quad.v[0].col = quad.v[1].col = quad.v[2].col = quad.v[3].col = SETA(0xFF1a1a1a, galph);
        quad.v[0].y = quad.v[1].y = y+getHeight()-5;
        quad.v[2].y = quad.v[3].y = y+getHeight();
        hge->Gfx_RenderQuad(&quad);

        hge->Gfx_RenderLine(x+5, y+1, x+getWidth()-5, y+1, SETA(0xFF585858, galph));
        hge->Gfx_RenderLine(x+5, y+getHeight(), x+getWidth()-5, y+getHeight(), SETA(0xFF2a2a2a, galph));

        hge->Gfx_RenderLine(x, y+23, x+getWidth(), y+23, SETA(0xFF2e2e2e, galph));
        hge->Gfx_RenderLine(x, y+24, x+getWidth(), y+24, SETA(0xFF181818, galph));
        hge->Gfx_RenderLine(x, y+25, x+getWidth(), y+25, SETA(0xFF272727, galph));


        //hge->Gfx_RenderLine(x, y+1, x+getWidth(), y+1, GV->colOutline);
        hge->Gfx_RenderLine(x+1, y+5, x+1, y+getHeight()-4, SETA(0xFF565656, galph));
        //hge->Gfx_RenderLine(x+1, y+getHeight(), x+getWidth(), y+getHeight(), GV->colOutline);
        hge->Gfx_RenderLine(x+getWidth(), y+5, x+getWidth(), y+getHeight()-4, SETA(0xFF565656, galph));

        if( bDrawShadow )
         GV->RenderDropShadow(x, y, getWidth(), getHeight(), galph);

        if( bClose ){
         unsigned char p = fCloseTimer*5.0f*125.0f+130.0f * getAlphaModifier();
         GV->sprIcons16[Icon16_X]->SetColor(SETA(0xFFFFFFFF, p));
         GV->sprIcons16[Icon16_X]->Render(x+getWidth()-18, y+5);
         GV->sprIcons16[Icon16_X]->SetColor(0xFFFFFFFF);
         //hGfx->sprIconClose->Render(x+getWidth()-22, y+2);
        }

        int textX;
        int textY;

        textY = ((int)getTitleBarHeight() - getFont()->getHeight()) / 2+3;

        switch (getAlignment())
        {
          case Graphics::LEFT:
              textX = 4;
              break;
          case Graphics::CENTER:
              textX = getWidth() / 2;
              break;
          case Graphics::RIGHT:
              textX = getWidth() - 4;
              break;
          default:
              throw GCN_EXCEPTION("Unknown alignment.");
        }

        graphics->setColor(0xFFFFFFFF);
        graphics->setFont(getFont());
        //graphics->pushClipArea(gcn::Rectangle(0, 0, getWidth(), getTitleBarHeight() - 1));
        graphics->drawText(getCaption(), textX, textY, getAlignment());
        //graphics->popClipArea();

        hge->Gfx_SetClipping(rect.x, rect.y, rect.width, rect.height);
        drawChildren(graphics);

    }

    void Win::mouseEntered(MouseEvent& mouseEvent)
    {
        mMouseOver = 1;
    }

    void Win::mouseExited(MouseEvent& mouseEvent)
    {
        mMouseOver = 0;
    }

    bool Win::showHand()
    {
        if( !bClose ) return 0;
        int x,y;
        float mx, my;
        getAbsolutePosition(x, y);
        hge->Input_GetMousePos(&mx, &my);
        return (mMouseOver && mx > x+getWidth()-18 && my > y+5 && mx < x+getWidth()-2 && my < y+21);
    }

    void Win::mousePressed(MouseEvent& mouseEvent)
    {
        if (mouseEvent.getSource() != this)
        {
            return;
        }

        if (getParent() != NULL)
        {
            getParent()->moveToTop(this);
        }

        if( bClose ){
         if( mouseEvent.getX() > getWidth()-18 &&
             mouseEvent.getX() < getWidth()-2 &&
             mouseEvent.getY() > 5 &&
             mouseEvent.getY() < 21 ){
          setShow(0);
          distributeActionEvent();
         }
        }

        mDragOffsetX = mouseEvent.getX();
        mDragOffsetY = mouseEvent.getY();

        mMoved = mouseEvent.getY() <= (int)22;
    }

    void Win::mouseReleased(MouseEvent& mouseEvent)
    {
        mMoved = false;
    }

    void Win::mouseDragged(MouseEvent& mouseEvent)
    {
        if (mouseEvent.isConsumed() || mouseEvent.getSource() != this)
        {
            return;
        }

        if (isMovable() && mMoved)
        {
            setPosition(mouseEvent.getX() - mDragOffsetX + getX(),
                        mouseEvent.getY() - mDragOffsetY + getY());
        }

        mouseEvent.consume();
    }

    gcn::Rectangle Win::getChildrenArea()
    {
        return gcn::Rectangle(getPadding(),
                         getTitleBarHeight(),
                         getWidth() - getPadding() * 2,
                         getHeight() - getPadding() - getTitleBarHeight());
    }

    void Win::setMovable(bool movable)
    {
        mMovable = movable;
    }

    bool Win::isMovable() const
    {
        return mMovable;
    }

    void Win::setOpaque(bool opaque)
    {
        mOpaque = opaque;
    }

    bool Win::isOpaque()
    {
        return mOpaque;
    }

    void Win::resizeToContent()
    {
        WidgetListIterator it;

        int w = 0, h = 0;
        for (it = mWidgets.begin(); it != mWidgets.end(); it++)
        {
            if ((*it)->getX() + (*it)->getWidth() > w)
            {
                w = (*it)->getX() + (*it)->getWidth();
            }

            if ((*it)->getY() + (*it)->getHeight() > h)
            {
                h = (*it)->getY() + (*it)->getHeight();
            }
        }

        setSize(w + 2* getPadding(), h + getPadding() + getTitleBarHeight());
    }
}
