#ifndef SHR_CONTEXT_H
#define SHR_CONTEXT_H

#include "../cGUIparts.h"

#include <string>

#include "guichan/focuslistener.hpp"
#include "guichan/graphics.hpp"
#include "guichan/keylistener.hpp"
#include "guichan/mouseevent.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"
#include "guichan/widget.hpp"
#include "guichan/selectionlistener.hpp"

#define CON_MOUSEOVER(me) ( me.getX() > x && me.getX() < x+w && me.getY() > y && me.getY() < y+h )

using namespace gcn;

namespace SHR {
    class gcn::SelectionListener;

    class Context;

    class ContextEl {
    private:
        int iID;
        hgeSprite *sprIcon[2];
        char *szCaption;
        int iAlign;
        Context *hCascade;
        bool bFocused;
        Context *hParent;
        bool bEnabled;
    public:
        ContextEl(int id, const char *cap, hgeSprite *spr = NULL);

        ~ContextEl() {};

        int GetID() { return iID; };

        hgeSprite *GetIcon(bool id = 0) { return sprIcon[id]; };

        const char *GetCaption() { return szCaption; };

        int GetAlign() { return iAlign; };

        Context *GetCascade() { return hCascade; };

        bool IsAnyInputFocused() { return bFocused; };

        Context *GetParent() { return hParent; };

        bool IsEnabled() { return bEnabled; };

        void SetEnabled(bool b) { bEnabled = b; };

        void SetIcon(hgeSprite *i, bool id = 0) { sprIcon[id] = i; }

        void SetID(int i) { iID = i; };

        void SetCaption(const char *cap);

        void SetAlign(int i) { iAlign = i; };

        void SetCascade(Context *con);

        void SetFocused(bool b, bool switchcascade = 1);

        void SetParent(Context *h) { hParent = h; };

        float fTimer;
    };

    class ContextModel {
    private:
        std::vector<ContextEl *> vElements;

        friend class Context;

    public:
        ContextModel() {};

        ~ContextModel();

        void AddElement(int id, const char *cap, hgeSprite *spr = NULL);

        void AddElement(ContextEl *el) { vElements.push_back(el); };

        ContextEl *GetElementByID(int id);

        void ClearElements();
    };

    class GCN_CORE_DECLSPEC Context : public gcn::Widget,
                                      public MouseListener,
                                      public FocusListener,
                                      public WidgetListener {
    public:
        Context(guiParts *Parts, hgeFont *font);

        void adjustSize();

        virtual void draw(Graphics *graphics);

        virtual void focusLost(const FocusEvent &event);

        void mousePressed(MouseEvent &mouseEvent) override;

        void mouseReleased(MouseEvent &mouseEvent) override;

        void mouseEntered(MouseEvent &mouseEvent) override;

        void mouseExited(MouseEvent &mouseEvent) override;

        void mouseDragged(MouseEvent &mouseEvent) override;

        void mouseMoved(MouseEvent &mouseEvent) override;

        void AddElement(ContextEl *el);

        void AddElement(int id, const char *cap, hgeSprite *spr = NULL, ContextEl *after = NULL);

        void AddSeparator();

        int GetSelectedID() { return iSelectedID; };

        int GetElementsCount() { return vElements.size(); };

        int GetSelectedIt() { return iSelected; };

        void SetSelectedIt(int i) { iSelected = i; iSelectedID = i == -1 ? -1 : vElements[i]->GetID(); };

        void SelectPrev();

        void SelectNext();

        void RemoveElement(int i);

        void ClearElements();

        void SetModel(ContextModel *m);

        void EmulateClickID(int id);

        ContextEl *GetElementByID(int id);

        ContextEl *GetSelectedElement();

        void setTopLineOffset(int offX) {
            topLineXOffset = offX;
        }

        unsigned char getAlpha() override {
            return fShowTimer * 5.0f * 255.0f;
        }

        void OpenSubContext(int i);

        bool MouseOver() { return mHasMouse; };

        void ReserveIconSpace(bool icon, bool reserve) { mReserveIconSpace[icon] = reserve; };

        bool IsReservedIconSpace(bool icon) { return mReserveIconSpace[icon]; };

        virtual void widgetShown(const Event &event);

        virtual void widgetHidden(const Event &event);

        void hide();

        void addSelectionListener(gcn::SelectionListener *selectionListener);

        void removeSelectionListener(gcn::SelectionListener *selectionListener);

        void distributeValueChangedEvent();

    protected:
        float fShowTimer;
        bool bHide;
        guiParts *hGfx;
        int iSelectedID, iSelected;
        int iRowHeight;
        bool mHasMouse;
        bool mKeyPressed;
        bool mMousePressed;
        bool mReserveIconSpace[2];
        std::vector<ContextEl *> vElements;
        std::vector<int> vSeparators;
        hgeFont *hFont;
        int topLineXOffset;

        typedef std::list<SelectionListener *> SelectionListenerList;
        SelectionListenerList mSelectionListeners;
        typedef SelectionListenerList::iterator SelectionListenerIterator;
    };
}

#endif
