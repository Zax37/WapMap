#ifndef SHR_TEXTDROPDOWN_H
#define SHR_TEXTDROPDOWN_H

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
    class GCN_CORE_DECLSPEC TextDropDown :
            public ActionListener,
            public BasicContainer,
            public KeyListener,
            public MouseListener,
            public FocusListener,
            public gcn::SelectionListener {
    public:
        TextDropDown(std::string defText,
                     ListModel *listModel = NULL,
                     ScrollArea *scrollArea = NULL,
                     ListBox *listBox = NULL);

        ~TextDropDown() override;

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

        gcn::Rectangle getChildrenArea() override;

        void focusLost(const Event &event) override;

        void action(const ActionEvent &actionEvent) override;

        void death(const Event &event) override;

        void keyPressed(KeyEvent &keyEvent) override;

        void mousePressed(MouseEvent &mouseEvent) override;

        void mouseReleased(MouseEvent &mouseEvent) override;

        void mouseWheelMovedUp(MouseEvent &mouseEvent) override;

        void mouseWheelMovedDown(MouseEvent &mouseEvent) override;

        void mouseDragged(MouseEvent &mouseEvent) override;

        void mouseEntered(MouseEvent &mouseEvent) override;

        void mouseExited(MouseEvent &mouseEvent) override;

        void valueChanged(const SelectionEvent &event) override;

        void SetGfx(guiParts *n) { hGfx = n; };

        void disableScroll(bool bDisable) { mScrollDisabled = bDisable; };

        void setText(const std::string &text, bool bGenerateActionEvent = 0);

        const std::string &getText() const;

        bool isMarkedInvalid() { return bMarkedInvalid; };

        void setMarkedInvalid(bool b) { bMarkedInvalid = b; };

        virtual bool showHand();

    protected:
        virtual void dropDown();

        virtual void foldUp();

        void distributeValueChangedEvent();

        bool mDroppedDown;
        bool mPushed;
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

        std::string mText;
        int mCaretPosition;
        int mSelectionPosition;
        int mXScroll;

        bool bMarkedInvalid;

        guiParts *hGfx;

        float fFocusTimer;
        float fButtonTimer;
        bool mHasMouse;
        bool bTextFocused;

        void fixScroll();

        void deleteSelection();
    };
}

#endif // end GCN_DROPDOWN_HPP
