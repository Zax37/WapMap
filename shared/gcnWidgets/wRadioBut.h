#ifndef SHR_RDBUT_H
#define SHR_RDBUT_H

#include <map>
#include <string>

#include "guichan/keylistener.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"
#include "guichan/widget.hpp"

using namespace gcn;

struct cInterfaceSheet;

namespace SHR
{
    class GCN_CORE_DECLSPEC RadBut :
        public gcn::Widget,
        public MouseListener,
        public KeyListener
    {
    public:
        RadBut();
        RadBut(cInterfaceSheet * Parts,
               const std::string &caption,
               const std::string &group,
               bool selected = false);
        virtual ~RadBut();

        bool isSelected() const;
        void setSelected(bool selected);

        const std::string &getCaption() const;
        void setCaption(const std::string caption);

        void setGroup(const std::string &group);
        const std::string &getGroup() const;

        void adjustSize();
        void allowDisselection(bool b){ bAllowDisselect = b; };


        // Inherited from Widget

        virtual void draw(Graphics* graphics);


        // Inherited from KeyListener

        virtual void keyPressed(KeyEvent& keyEvent);


        // Inherited from MouseListener

        virtual void mouseClicked(MouseEvent& mouseEvent);
        virtual void mouseDragged(MouseEvent& mouseEvent);
        virtual void mouseEntered(MouseEvent& mouseEvent);
        virtual void mouseExited(MouseEvent& mouseEvent);

        virtual bool showHand();

    protected:
        cInterfaceSheet * hGfx;
        bool mSelected;
        std::string mCaption;
        std::string mGroup;
        typedef std::multimap<std::string, RadBut *> GroupMap;
        typedef GroupMap::iterator GroupIterator;
        static GroupMap mGroupMap;
        bool bAllowDisselect;

        float fTimer;
        bool bMouseOver;
    };
}

#endif // end GCN_RadBut_HPP
