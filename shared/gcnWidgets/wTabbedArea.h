#ifndef SHR_TABAREA_H
#define SHR_TABAREA_H

#include <map>
#include <string>
#include <vector>

#include "guichan/actionlistener.hpp"
#include "guichan/basiccontainer.hpp"
#include "guichan/keylistener.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"

using namespace gcn;

namespace SHR {
    class Contener;

    class Tab;

    class GCN_CORE_DECLSPEC TabbedArea :
            public ActionListener,
            public BasicContainer,
            public KeyListener,
            public MouseListener {
        friend class Tab;

    public:
        TabbedArea();

        virtual ~TabbedArea();

        void setOpaque(bool opaque);

        bool isOpaque() const;

        virtual void addTab(const std::string &caption, Widget *widget);

        virtual void addTab(Tab *tab, Widget *widget);

        virtual void removeTabWithIndex(unsigned int index);

        virtual void removeTab(Tab *tab);

        virtual bool isTabSelected(unsigned int index) const;

        virtual bool isTabSelected(Tab *tab);

        virtual void setSelectedTab(unsigned int index);

        virtual void setSelectedTab(Tab *tab);

        virtual int getSelectedTabIndex() const;

        Tab *getSelectedTab();


        // Inherited from Widget

        virtual void draw(Graphics *graphics);

        virtual void logic();

        void setWidth(int width);

        void setHeight(int height);

        void setSize(int width, int height);

        void setDimension(const gcn::Rectangle &dimension);


        // Inherited from ActionListener

        void action(const ActionEvent &actionEvent);


        // Inherited from DeathListener

        virtual void death(const Event &event);


        // Inherited from KeyListener

        virtual void keyPressed(KeyEvent &keyEvent);


        // Inherited from MouseListener

        virtual void mousePressed(MouseEvent &mouseEvent);


    protected:
        void adjustSize();

        void adjustTabPositions();

        Tab *mSelectedTab;
        Contener *mTabContainer;
        Contener *mWidgetContainer;
        std::vector<Tab *> mTabsToDelete;
        std::vector<std::pair<Tab *, Widget *> > mTabs;
        bool mOpaque;
    };
}

#endif // end GCN_TABBEDAREA_HPP
