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

    class TreeElement : public MouseListener {
    protected:
        hgeSprite *sprIcon;
        char *szName;
        int m_iW, m_iH;
        TreeFolder *m_hParent;
        bool m_bFocused, m_bSelected;
    public:
        TreeElement(const char *pszName, hgeSprite *psprIcon);

        const char *GetName() { return (const char *) szName; };

        hgeSprite *GetIcon() { return sprIcon; };

        virtual int Render(int x, int y);

        virtual int GetWidth() const;

        virtual int GetHeight() const;

        virtual void GainFocus();

        virtual void LoseFocus();

        void GainSelection() { m_bSelected = true; };

        void LoseSelection() { m_bSelected = false; };

        virtual bool IsDir() { return false; };

        class Tree* GetRoot();

        void mousePressed(MouseEvent &mouseEvent) override;

        void mouseMoved(MouseEvent &mouseEvent) override;

        SHR::TreeFolder* GetParent();
    };

    class TreeFolder : public TreeElement {
    protected:
        int m_iFocus;
        std::vector<std::shared_ptr<TreeElement>> m_vElements;
        bool m_bOpened;
        int m_iWex, m_iHex;
    public:
        int Render(int x, int y) override;

        TreeFolder(const char *pszName, hgeSprite *psprIcon);

        void AddElement(const std::shared_ptr<TreeElement>& phEl);

        std::shared_ptr<TreeElement> GetElement(int i);

        int GetElementsCount() { return m_vElements.size(); }

        int GetIndexOf(TreeElement* ptr) {
            for (int i = 0; i < m_vElements.size(); ++i) {
                if (&*m_vElements[i] == ptr) return i;
            }
            return -1;
        }

        int GetWidth() const override;

        int GetHeight() const override;

        void GainFocus() override;

        void LoseFocus() override;

        bool IsDir() override { return true; };

        void RecalculateSize();

        void SetOpened(bool bn);

        bool IsOpened() { return m_bOpened; };

        void mousePressed(MouseEvent &mouseEvent) override;

        void mouseMoved(MouseEvent &mouseEvent) override;

        TreeElement* GetFocused();
    };


    class GCN_CORE_DECLSPEC Tree : public TreeFolder,
                                   public gcn::Widget,
                                   public KeyListener,
                                   public FocusListener {
    public:
        Tree();

        void draw(Graphics *graphics) override;

        void focusLost(const FocusEvent &event) override;

        void mouseReleased(MouseEvent &mouseEvent) override;

        void mouseEntered(MouseEvent &mouseEvent) override;

        void mouseExited(MouseEvent &mouseEvent) override;

        void mouseDragged(DragEvent &mouseEvent) override;

        void keyPressed(KeyEvent &keyEvent) override;

        void keyReleased(KeyEvent &keyEvent) override;

        int GetWidth() const override;

        int GetHeight() const override;

        int Render(int x, int y) override;

        void adjustSize();

        TreeElement* GetSelectedElement() { return m_hSelected; }

        void SetSelectedElement(TreeElement* element);

    protected:
        bool mHasMouse;
        bool mKeyPressed;
        bool mMousePressed;

        TreeElement* m_hSelected;
    };
}

#endif
