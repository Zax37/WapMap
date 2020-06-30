#include "wTree.h"

#include "guichan/exception.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/key.hpp"
#include "guichan/mouseevent.hpp"
#include "guichan/mouseinput.hpp"
#include "guichan/mouseinput.hpp"

#ifdef WAP_ARCHIVE
#include "../../WapArchive/globals.h"
#endif
#ifdef WAP_GRAPHIC
#include "../../WapGraphic/globals.h"
#endif
#ifdef WAP_MAP

#include "../../WapMap/globals.h"

#endif

extern HGE *hge;

namespace SHR {
    Tree::Tree(guiParts *Parts, const char *pszName, hgeSprite *psprIcon) : TreeFolder(pszName, psprIcon, NULL) {
        mHasMouse = mKeyPressed = mMousePressed = 0;
        setFocusable(true);

        addMouseListener(this);
        addKeyListener(this);
        addFocusListener(this);
        hGfx = Parts;
        adjustSize();
        /*m_bStandalone = bStandalone;
        szName = new char[strlen(pszName)+1];
        strcpy(szName, pszName);
        sprIcon = psprIcon;*/
    }

    void Tree::logic() {
        if (m_iFocus != -1)
            m_vElements[m_iFocus]->Logic();
    }

    void Tree::draw(Graphics *graphics) {
        ClipRectangle rect = graphics->getCurrentClipArea();
        int x, y;
        getAbsolutePosition(x, y);

        hge->Gfx_SetClipping();
        Render(x, y);
        hge->Gfx_SetClipping(rect.x, rect.y, rect.width, rect.height);
    }

    void Tree::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT) {
            mMousePressed = true;
            mouseEvent.consume();
        }
    }

    void Tree::mouseMoved(MouseEvent &mouseEvent) {
        int x, y;
        getAbsolutePosition(x, y);
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        //mx -= x;
        //my -= y;
        x += 16;
        y += 20;

        for (int i = 0; i < m_vElements.size(); i++) {
            if (mx > x &&
                mx < x + m_vElements[i]->GetWidth() &&
                my > y &&
                my < y + m_vElements[i]->GetHeight()) {
                if (m_iFocus == i) return;
                if (m_iFocus != -1) {
                    m_vElements[m_iFocus]->LoseFocus();
                }
                m_iFocus = i;
                m_vElements[i]->GainFocus();
                return;
            }
            y += m_vElements[i]->GetHeight();
        }
        if (m_iFocus != -1) {
            m_vElements[m_iFocus]->LoseFocus();
            m_iFocus = -1;
        }
    }

    void Tree::mouseExited(MouseEvent &mouseEvent) {
        mHasMouse = false;
        if (m_iFocus != -1) {
            m_vElements[m_iFocus]->LoseFocus();
            m_iFocus = -1;
        }
    }

    void Tree::mouseEntered(MouseEvent &mouseEvent) {
        mHasMouse = true;
    }

    void Tree::mouseReleased(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT
            && mMousePressed
            && mHasMouse) {
            mMousePressed = false;
            distributeActionEvent();
            mouseEvent.consume();
        } else if (mouseEvent.getButton() == MouseEvent::LEFT) {
            mMousePressed = false;
            mouseEvent.consume();
        }
    }

    void Tree::mouseDragged(DragEvent &mouseEvent) {
        mouseEvent.consume();
    }

    void Tree::keyPressed(KeyEvent &keyEvent) {
        Key key = keyEvent.getKey();

        if (key.getValue() == Key::ENTER
            || key.getValue() == Key::SPACE) {
            mKeyPressed = true;
            keyEvent.consume();
        }
    }

    void Tree::keyReleased(KeyEvent &keyEvent) {
        Key key = keyEvent.getKey();

        if ((key.getValue() == Key::ENTER
             || key.getValue() == Key::SPACE)
            && mKeyPressed) {
            mKeyPressed = false;
            distributeActionEvent();
            keyEvent.consume();
        }
    }

    void Tree::focusLost(const FocusEvent &event) {
        mMousePressed = false;
        mKeyPressed = false;
    }

    TreeElement::TreeElement(const char *pszName, hgeSprite *psprIcon, TreeFolder *phParent) {
        szName = new char[strlen(pszName) + 1];
        strcpy(szName, pszName);
        sprIcon = psprIcon;
        m_iW = GV->fntMyriad16->GetStringWidth(szName) + 16;
        m_iH = 20;
        m_hParent = phParent;
    }

    TreeFolder::TreeFolder(const char *pszName, hgeSprite *psprIcon, TreeFolder *phParent) : TreeElement(pszName,
                                                                                                         psprIcon,
                                                                                                         phParent) {
        m_bOpened = 0;
        m_iWex = m_iW;
        m_iHex = m_iH;
        m_iFocus = -1;
    }

    int TreeElement::Render(int x, int y) {
        if (sprIcon != NULL)
            sprIcon->Render(x, y);
        GV->fntMyriad16->Render(x + 20, y, HGETEXT_LEFT, szName, 0);
        return 20;
    }

    int TreeFolder::Render(int x, int y) {
        int oldy = y;
        if (sprIcon != NULL)
            sprIcon->Render(x, y);
        GV->fntMyriad16->Render(x + 16, y, HGETEXT_LEFT, szName, 0);
        y += 20;
        if (m_bOpened) {
            x += 20;
            for (int i = 0; i < m_vElements.size(); i++) {
                y += m_vElements[i]->Render(x, y);
            }
        }
        return y - oldy;
    }

    int TreeElement::GetWidth() {
        return m_iW;
    }

    int TreeElement::GetHeight() {
        return m_iH;
    }

    int TreeFolder::GetWidth() {
        if (m_bOpened)
            return m_iWex;
        else
            return m_iW;
    }

    int TreeFolder::GetHeight() {
        if (m_bOpened)
            return m_iHex;
        else
            return m_iH;
    }

    int Tree::GetWidth() {
        return getWidth();
    }

    int Tree::GetHeight() {
        return getHeight();
    }

    void TreeElement::GainFocus() {

    }

    void TreeElement::LoseFocus() {

    }

    void TreeFolder::GainFocus() {
        printf("%s: gained\n", szName);
    }

    void TreeFolder::LoseFocus() {
        printf("%s: lost\n", szName);
    }

    void TreeFolder::AddElement(TreeElement *phEl) {
        //phEl->GetHeight()+GetHeight();
        m_iHex += phEl->GetHeight();
        m_iWex = std::max(m_iWex, phEl->GetWidth() + 20);
        m_vElements.push_back(phEl);
    }

    void Tree::adjustSize() {
        if (m_bOpened) {
            setWidth(m_iWex);
            setHeight(m_iHex);
        } else {
            setWidth(m_iW);
            setHeight(m_iH);
        }
    }

    void TreeElement::Logic() {

    }

    void TreeFolder::Logic() {
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        if (mx != lmx || my != lmy) {

        }
        lmx = mx;
        lmy = my;
        if (hge->Input_KeyDown(HGEK_LBUTTON)) {
            printf("%s focus %d\n", szName, m_iFocus);
            if (m_iFocus == -1) {
                SetOpened(!IsOpened());
            }
        }
    }

    void TreeFolder::SetOpened(bool bn) {
        if (bn == m_bOpened)
            return;
        m_bOpened = bn;
        if (m_hParent != NULL)
            m_hParent->RecalculateSize();
        else
            ((Tree *) this)->adjustSize();
    }

    void TreeFolder::RecalculateSize() {
        m_iWex = m_iW;
        m_iHex = m_iH;
        for (int i = 0; i < m_vElements.size(); i++) {
            m_iHex += m_vElements[i]->GetHeight();
            m_iWex = std::max(m_iWex, m_vElements[i]->GetWidth());
        }
        if (m_hParent != NULL)
            m_hParent->RecalculateSize();
        else
            ((Tree *) this)->adjustSize();
    }
}
