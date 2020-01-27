#ifndef SHR_DROPDOWN_H
#define SHR_DROPDOWN_H

#include "../cGUIparts.h"

#include "guichan/actionlistener.hpp"
#include "guichan/basiccontainer.hpp"
#include "guichan/deathlistener.hpp"
#include "guichan/focushandler.hpp"
#include "guichan/focuslistener.hpp"
#include "guichan/keylistener.hpp"
#include "guichan/listmodel.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"
#include "guichan/selectionlistener.hpp"
#include "wListbox.h"
#include "wScrollArea.h"

using namespace gcn;

namespace SHR {
    class GCN_CORE_DECLSPEC DropDown :
            public ActionListener,
            public BasicContainer,
            public KeyListener,
            public MouseListener,
            public FocusListener,
            public gcn::SelectionListener {
    public:
        DropDown(ListModel *listModel = NULL,
                 ScrollArea *scrollArea = NULL,
                 ListBox *listBox = NULL);

        virtual ~DropDown();

        int getSelected() const;

        void setSelected(int selected);

        void setListModel(ListModel *listModel);

        ListModel *getListModel();

        void adjustHeight();

        void addSelectionListener(SelectionListener *selectionListener);

        void removeSelectionListener(SelectionListener *selectionListener);


        virtual void draw(Graphics *graphics);

        void setBaseColor(const Color &color);

        void setBackgroundColor(const Color &color);

        void setForegroundColor(const Color &color);

        void setFont(Font *font);

        void setSelectionColor(const Color &color);

        virtual gcn::Rectangle getChildrenArea();

        virtual void focusLost(const Event &event);

        virtual void action(const ActionEvent &actionEvent);

        virtual void death(const Event &event);

        virtual void keyPressed(KeyEvent &keyEvent);

        virtual void mousePressed(MouseEvent &mouseEvent);

        virtual void mouseReleased(MouseEvent &mouseEvent);

        virtual void mouseWheelMovedUp(MouseEvent &mouseEvent);

        virtual void mouseWheelMovedDown(MouseEvent &mouseEvent);

        virtual void mouseDragged(MouseEvent &mouseEvent);

        virtual void mouseEntered(MouseEvent &mouseEvent);

        virtual void mouseExited(MouseEvent &mouseEvent);

        virtual bool showHand();

        virtual void valueChanged(const SelectionEvent &event);

        void SetGfx(guiParts *n) { hGfx = n; };

        void disableScroll(bool bDisable) { mScrollDisabled = bDisable; };

    protected:
        virtual void dropDown();

        virtual void foldUp();

        void distributeValueChangedEvent();

        bool mDroppedDown;
        bool mPushed;
        bool mHasMouse;
        int mFoldedUpHeight;
        ScrollArea *mScrollArea;
        ListBox *mListBox;
        FocusHandler mInternalFocusHandler;
        bool mInternalScrollArea;
        bool mInternalListBox;
        bool mIsDragged;
        bool mScrollDisabled;
        typedef std::list<SelectionListener *> SelectionListenerList;
        SelectionListenerList mSelectionListeners;
        typedef SelectionListenerList::iterator SelectionListenerIterator;
        float fButtonTimer;

        guiParts *hGfx;
    };
}

#endif // end GCN_DROPDOWN_HPP
