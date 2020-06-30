#ifndef SHR_TREE_H
#define SHR_TREE_H

#include "../cGUIparts.h"

#include <string>

#include "guichan/focuslistener.hpp"
#include "guichan/graphics.hpp"
#include "guichan/keylistener.hpp"
#include "guichan/mouseevent.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"
#include "guichan/widget.hpp"

using namespace gcn;

namespace SHR {
    class TreeFolder;

    class TreeElement {
    protected:
        hgeSprite *sprIcon;
        char *szName;
        int m_iW, m_iH;
        TreeFolder *m_hParent;
    public:
        TreeElement(const char *pszName, hgeSprite *psprIcon, TreeFolder *phParent);

        const char *GetName() { return (const char *) szName; };

        hgeSprite *GetIcon() { return sprIcon; };

        virtual int Render(int x, int y);

        virtual int GetWidth();

        virtual int GetHeight();

        virtual void GainFocus();

        virtual void LoseFocus();

        virtual bool IsDir() { return 0; };

        virtual void Logic();
    };

    class TreeFolder : public TreeElement {
    protected:
        int m_iFocus;
        std::vector<TreeElement *> m_vElements;
        bool m_bOpened;
        int m_iWex, m_iHex;
        float lmx, lmy;
    public:
        virtual int Render(int x, int y);

        TreeFolder(const char *pszName, hgeSprite *psprIcon, TreeFolder *phParent);

        void AddElement(TreeElement *phEl);

        virtual int GetWidth();

        virtual int GetHeight();

        virtual void GainFocus();

        virtual void LoseFocus();

        virtual bool IsDir() { return 1; };

        virtual void Logic();

        void RecalculateSize();

        void SetOpened(bool bn);

        bool IsOpened() { return m_bOpened; };
    };


    class GCN_CORE_DECLSPEC Tree : public TreeFolder,
                                   public gcn::Widget,
                                   public MouseListener,
                                   public KeyListener,
                                   public FocusListener {
    public:
        Tree(guiParts *Parts, const char *pszName, hgeSprite *psprIcon);

        virtual void logic();

        virtual void draw(Graphics *graphics);

        virtual void focusLost(const FocusEvent &event);

        virtual void mouseMoved(MouseEvent &mouseEvent);

        virtual void mousePressed(MouseEvent &mouseEvent);

        virtual void mouseReleased(MouseEvent &mouseEvent);

        virtual void mouseEntered(MouseEvent &mouseEvent);

        virtual void mouseExited(MouseEvent &mouseEvent);

        virtual void mouseDragged(DragEvent &mouseEvent);

        virtual void keyPressed(KeyEvent &keyEvent);

        virtual void keyReleased(KeyEvent &keyEvent);

        virtual int GetWidth();

        virtual int GetHeight();

        void adjustSize();

    protected:
        guiParts *hGfx;

        bool mHasMouse;
        bool mKeyPressed;
        bool mMousePressed;
    };
}

#endif
