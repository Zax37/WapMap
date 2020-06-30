#ifndef SHR_DROPDOWN_H
#define SHR_DROPDOWN_H

#include "../cGUIparts.h"
#include "cField.h"
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
            public Field,
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

        void setSelected(int selected, bool generatingEvent = true);

        void setListModel(ListModel *listModel);

        ListModel *getListModel();

        void adjustHeight();

        void addSelectionListener(SelectionListener *selectionListener);

        void removeSelectionListener(SelectionListener *selectionListener);

        void draw(Graphics *graphics) override;

        virtual gcn::Rectangle getChildrenArea();

        void focusGained(const FocusEvent &event) override;

        void focusLost(const FocusEvent &event) override;

        void action(const ActionEvent &actionEvent) override;

        void death(const Event &event) override;

        void keyPressed(KeyEvent &keyEvent) override;

        void mousePressed(MouseEvent &mouseEvent) override;

        void mouseReleased(MouseEvent &mouseEvent) override;

        void mouseWheelMovedUp(MouseEvent &mouseEvent) override;

        void mouseWheelMovedDown(MouseEvent &mouseEvent) override;

        void mouseDragged(DragEvent &mouseEvent) override;

        void mouseEntered(MouseEvent &mouseEvent) override;

        void mouseExited(MouseEvent &mouseEvent) override;

        void valueChanged(const SelectionEvent &event) override;

        void SetGfx(guiParts *n) { hGfx = n; };

        void disableScroll(bool bDisable) { mScrollDisabled = bDisable; };

    protected:
        virtual void dropDown();

        virtual void foldUp();

        void distributeValueChangedEvent();

        bool mDroppedDown;
        bool mHasMouse;
        int mFoldedUpHeight;
        int mSelected = 0;
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
        float fFocusTimer;
        bool mKeyboardFocus;

        guiParts *hGfx;
    };
}

#endif // end GCN_DROPDOWN_HPP
