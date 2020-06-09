#ifndef SHR_LISTBOX_H
#define SHR_LISTBOX_H

#include <list>

#include "guichan/keylistener.hpp"
#include "guichan/listmodel.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/selectionlistener.hpp"
#include "guichan/platform.hpp"
#include "guichan/widget.hpp"

using namespace gcn;

namespace SHR {
    class gcn::SelectionListener;

    class GCN_CORE_DECLSPEC ListBox :
            public gcn::Widget,
            public MouseListener,
            public KeyListener {
    public:
        ListBox();

        ListBox(ListModel *listModel);

        virtual ~ListBox() {}

        int getSelected() const;

        void setSelected(int selected, bool generatingEvent = true);

        void setListModel(ListModel *listModel);

        ListModel *getListModel();

        void adjustSize();

        bool isWrappingEnabled() const;

        void setWrappingEnabled(bool wrappingEnabled);

        void addSelectionListener(gcn::SelectionListener *selectionListener);

        void removeSelectionListener(gcn::SelectionListener *selectionListener);

        virtual unsigned int getRowHeight() const;

        virtual void draw(Graphics *graphics);

        virtual void drawRow(Graphics *graphics, int i);

        virtual void logic();

        virtual void keyPressed(KeyEvent &keyEvent);

        virtual void mousePressed(MouseEvent &mouseEvent);

        virtual void mouseDragged(MouseEvent &mouseEvent);

        virtual void mouseMoved(MouseEvent &mouseEvent);

        virtual void mouseExited(MouseEvent &mouseEvent);

        virtual int getTextXOffset() const { return 2; }

        int findIndexOf(const std::string& option);

    protected:
        void distributeValueChangedEvent();

        int mSelected, mHighlighted;
        ListModel *mListModel;
        bool mWrappingEnabled;
        typedef std::list<SelectionListener *> SelectionListenerList;
        SelectionListenerList mSelectionListeners;
        typedef SelectionListenerList::iterator SelectionListenerIterator;

        friend class TextDropDown;
    };
}

#endif
