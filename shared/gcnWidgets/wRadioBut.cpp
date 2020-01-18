#include "wRadioBut.h"

#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/key.hpp"
#include "guichan/mouseinput.hpp"

#include "../../WapMap/cInterfaceSheet.h"

using namespace gcn;

extern HGE * hge;

namespace SHR
{
    RadBut::GroupMap RadBut::mGroupMap;

    RadBut::RadBut()
    {
        setSelected(false);

        setFocusable(true);
        addMouseListener(this);
        addKeyListener(this);
        bAllowDisselect = 0;
        fTimer = 0;
        bMouseOver = 0;
    }

    RadBut::RadBut(cInterfaceSheet * Parts, const std::string &caption,
                             const std::string &group,
                             bool selected)
    {
        hGfx = Parts;
        setCaption(caption);
        setGroup(group);
        setSelected(selected);

        setFocusable(true);
        addMouseListener(this);
        addKeyListener(this);

        adjustSize();
        bAllowDisselect = 0;
        fTimer = 0;
        bMouseOver = 0;
    }

    RadBut::~RadBut()
    {
        // Remove us from the group list
        setGroup("");
    }

    void RadBut::draw(Graphics* graphics)
    {
        int x, y;
        getAbsolutePosition(x, y);

        int dy = y+getHeight()/2-8;

        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        bool bmo = bMouseOver && mx < x+17 && my > dy && my < dy+17;

        if( bmo && fTimer < 0.1f ){
         fTimer += hge->Timer_GetDelta();
         if( fTimer > 0.1f ) fTimer = 0.1f;
        }else if( !bmo && fTimer > 0.0f ){
         fTimer -= hge->Timer_GetDelta();
         if( fTimer < 0.0f ) fTimer = 0.0f;
        }

        int st = 4;
        if( isEnabled() ){
         st = (fTimer == 0.1f ? 3 : 2);
        }
        hGfx->sprControl[0][st]->Render(x, dy);

        if( isEnabled() && fTimer != 0 && fTimer != 0.1f ){
         hGfx->sprControl[0][3]->SetColor(SETA(0xFFFFFFFF, (unsigned char)(fTimer*10.0f*255.0f)));
         hGfx->sprControl[0][3]->Render(x, dy);
        }

        if( isSelected() )
         hGfx->sprControl[0][!isEnabled()]->Render(x, dy);

        graphics->setFont(getFont());
        graphics->setColor(isEnabled() ? 0xa1a1a1 : 0x222222);

        int h = getHeight() + getHeight() / 2;

        graphics->drawText(getCaption(), h - 4, 0);
    }

    bool RadBut::isSelected() const
    {
        return mSelected;
    }

    void RadBut::setSelected(bool selected)
    {
        if (selected && mGroup != "")
        {
            GroupIterator iter, iterEnd;
            iterEnd = mGroupMap.upper_bound(mGroup);

            for (iter = mGroupMap.lower_bound(mGroup);
                 iter != iterEnd;
                 iter++)
            {
                if (iter->second->isSelected())
                {
                    iter->second->setSelected(false);
                }
            }
        }

        mSelected = selected;
    }

    const std::string &RadBut::getCaption() const
    {
        return mCaption;
    }

    void RadBut::setCaption(const std::string caption)
    {
        mCaption = caption;
    }

    void RadBut::keyPressed(KeyEvent& keyEvent)
    {
        Key key = keyEvent.getKey();

        if (key.getValue() == Key::ENTER ||
            key.getValue() == Key::SPACE)
        {
            setSelected(true);
            distributeActionEvent();
            keyEvent.consume();
        }
    }


    void RadBut::mouseEntered(MouseEvent& mouseEvent)
    {
        bMouseOver = 1;
    }

    void RadBut::mouseExited(MouseEvent& mouseEvent)
    {
        bMouseOver = 0;
    }

    bool RadBut::showHand()
    {
        int x, y;
        getAbsolutePosition(x, y);
        int dy = y+getHeight()/2-8;

        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        return (isEnabled() && !(isSelected() && !bAllowDisselect) && mx < x+17 && my > dy && my < dy+17);
    }

    void RadBut::mouseClicked(MouseEvent& mouseEvent)
    {
        if (mouseEvent.getButton() == MouseEvent::LEFT)
        {
            if( isSelected() && bAllowDisselect )
             setSelected(false);
            else
             setSelected(true);
            distributeActionEvent();
        }
    }

    void RadBut::mouseDragged(MouseEvent& mouseEvent)
    {
        mouseEvent.consume();
    }

    void RadBut::setGroup(const std::string &group)
    {
        if (mGroup != "")
        {
            GroupIterator iter, iterEnd;
            iterEnd = mGroupMap.upper_bound(mGroup);

            for (iter = mGroupMap.lower_bound(mGroup);
                 iter != iterEnd;
                 iter++)
            {
                if (iter->second == this)
                {
                    mGroupMap.erase(iter);
                    break;
                }
            }
        }

        if (group != "")
        {
            mGroupMap.insert(
                std::pair<std::string, RadBut *>(group, this));
        }

        mGroup = group;
    }

    const std::string &RadBut::getGroup() const
    {
        return mGroup;
    }

    void RadBut::adjustSize()
    {
        int height = getFont()->getHeight();
        if( height < 17 ) height = 17;

        setHeight(height);
        setWidth(getFont()->getWidth(getCaption()) + height + height/2);
    }
}
