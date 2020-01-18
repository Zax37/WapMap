#include "wLink.h"

#include "guichan/exception.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"

#include <hgeFont.h>
#include "guichan/hge/hgeimagefont.hpp"
#include <hge.h>
#include <hgeSprite.h>
extern HGE * hge;

namespace SHR
{
    Link::Link()
    {
        mAlignment = Graphics::LEFT;
        bMouseOver = mMousePressed = 0;
        sprIcon = 0;
        fFocusTimer = 0;
    }

    Link::Link(const std::string& caption, hgeSprite * spr)
    {
        mCaption = caption;
        mAlignment = Graphics::LEFT;

       sprIcon = spr;

       adjustSize();
        bMouseOver = mMousePressed = 0;
       addMouseListener(this);
       fFocusTimer = 0;
    }

    bool Link::showHand()
    {
        return isEnabled();
    }

    const std::string &Link::getCaption() const
    {
        return mCaption;
    }

    void Link::setCaption(const std::string& caption)
    {
        mCaption = caption;
    }

    void Link::setAlignment(Graphics::Alignment alignment)
    {
        mAlignment = alignment;
    }

    Graphics::Alignment Link::getAlignment() const
    {
        return mAlignment;
    }

    void Link::draw(Graphics* graphics)
    {
        if( bMouseOver && fFocusTimer < 0.2f ){
         fFocusTimer += hge->Timer_GetDelta();
         if( fFocusTimer > 0.2f ) fFocusTimer = 0.2f;
        }else if( !bMouseOver && fFocusTimer > 0.0f ){
         fFocusTimer -= hge->Timer_GetDelta();
         if( fFocusTimer < 0.0f ) fFocusTimer = 0.0f;
        }

        int textX;
        int textY = getHeight() / 2 - getFont()->getHeight() / 2;

        switch (getAlignment())
        {
          case Graphics::LEFT:
              textX = 0;
              break;
          case Graphics::CENTER:
              textX = getWidth() / 2;
              break;
          case Graphics::RIGHT:
              textX = getWidth();
              break;
          default:
              throw GCN_EXCEPTION("Unknown alignment.");
        }

        int dx, dy;
        getAbsolutePosition(dx,dy);

        hgeFont * fnt = ((HGEImageFont*)getFont())->getHandleHGE();

        //0xFFa1a1a1

        unsigned char p = 161.0f+(fFocusTimer*5.0f*94.0f);

        fnt->SetColor(ARGB(255, p, p, p));

        int textx = (sprIcon == 0 ? 0 : sprIcon->GetWidth()+2);

        if( sprIcon != 0 )
        sprIcon->Render(dx, dy);

        fnt->Render(dx+textX+textx, dy+textY, HGETEXT_LEFT, getCaption().c_str());
    }

    void Link::adjustSize()
    {
        setWidth(getFont()->getWidth(getCaption()) + (sprIcon != 0 ? sprIcon->GetWidth()+2 : 0));
        setHeight(getFont()->getHeight());
        if( sprIcon != 0 ){
         if( sprIcon->GetHeight() > getHeight() )
          setHeight(sprIcon->GetHeight());
        }
    }

    void Link::mouseExited(MouseEvent& mouseEvent)
    {
        bMouseOver = 0;
    }

    void Link::mouseEntered(MouseEvent& mouseEvent)
    {
        bMouseOver = 1;
    }

    void Link::mousePressed(MouseEvent& mouseEvent)
    {
        if (mouseEvent.getButton() == MouseEvent::LEFT)
        {
            mMousePressed = true;
            mouseEvent.consume();
        }
    }

    void Link::mouseReleased(MouseEvent& mouseEvent)
    {
        if (mouseEvent.getButton() == MouseEvent::LEFT
            && mMousePressed
            && bMouseOver)
        {
            mMousePressed = false;
            distributeActionEvent();
            mouseEvent.consume();
        }
        else if (mouseEvent.getButton() == MouseEvent::LEFT)
        {
            mMousePressed = false;
            mouseEvent.consume();
        }
    }

    void Link::mouseDragged(MouseEvent& mouseEvent)
    {
        mouseEvent.consume();
    }
}
