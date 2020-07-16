#include "wTree.h"
#include "guichan/exception.hpp"
#include "guichan/key.hpp"
#include "guichan/mouseevent.hpp"
#include "../../WapMap/globals.h"
#include "../shared/commonFunc.h"

extern HGE *hge;

namespace SHR {
    Tree::Tree() : TreeFolder(NULL, NULL) {
        mHasMouse = mKeyPressed = mMousePressed = false;
        setFocusable(true);

        addMouseListener(this);
        addKeyListener(this);
        addFocusListener(this);

        m_iW = m_iH = m_iWex = m_iHex = 2;
        m_bOpened = true;
        m_hSelected = NULL;
    }

    void Tree::draw(Graphics *graphics) {
        int x, y;
        getAbsolutePosition(x, y);
        Render(x, y);
    }

    void TreeFolder::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() != MouseEvent::LEFT) return;
        if (mouseEvent.getY() < m_iH) {
            //if (mouseEvent.getX() < 0 && mouseEvent.getX() > -20) {
                SetOpened(!IsOpened());
            /*} else {
                TreeElement::mousePressed(mouseEvent);
            }*/
        } else {
            mouseEvent.offsetX(-20);
            mouseEvent.offsetY(-m_iH);
            for (auto &m_vElement : m_vElements) {
                if (mouseEvent.getY() < m_vElement->GetHeight()) {
                    m_vElement->mousePressed(mouseEvent);
                    return;
                }
                mouseEvent.offsetY(-m_vElement->GetHeight());
            }
        }
    }

    void TreeFolder::mouseMoved(MouseEvent &mouseEvent) {
        if (mouseEvent.getY() < m_iH) {
            //TreeElement::mouseMoved(mouseEvent);
            return;
        }
        LoseFocus();
        mouseEvent.offsetX(-20);
        mouseEvent.offsetY(-m_iH);
        for (int i = 0; i < m_vElements.size(); i++) {
            if (mouseEvent.getY() < m_vElements[i]->GetHeight()) {
                if (m_iFocus == i) {
                    m_vElements[i]->mouseMoved(mouseEvent);
                    return;
                }
                if (m_iFocus != -1) {
                    m_vElements[m_iFocus]->LoseFocus();
                }
                m_iFocus = i;
                m_vElements[i]->GainFocus();
                m_vElements[i]->mouseMoved(mouseEvent);
                return;
            }
            mouseEvent.offsetY(-m_vElements[i]->GetHeight());
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
        /*Key key = keyEvent.getKey();

        if (key.getValue() == Key::ENTER
            || key.getValue() == Key::SPACE) {
            mKeyPressed = true;
            keyEvent.consume();
        }*/
    }

    void Tree::keyReleased(KeyEvent &keyEvent) {
        /*Key key = keyEvent.getKey();

        if ((key.getValue() == Key::ENTER
             || key.getValue() == Key::SPACE)
            && mKeyPressed) {
            mKeyPressed = false;
            distributeActionEvent();
            keyEvent.consume();
        }*/
    }

    void Tree::focusLost(const FocusEvent &event) {
        mMousePressed = false;
        mKeyPressed = false;
    }

    TreeElement::TreeElement(const char *pszName, hgeSprite *psprIcon) {
        if (pszName) {
            szName = new char[strlen(pszName) + 1];
            strcpy(szName, pszName);
        } else {
            szName = new char[1];
            szName[0] = 0;
        }
        sprIcon = psprIcon;
        m_iW = GV->fntMyriad16->GetStringWidth(szName) + 24;
        m_iH = 20;
        m_hParent = NULL;
        m_bFocused = m_bSelected = false;
    }

    TreeFolder::TreeFolder(const char *pszName, hgeSprite *psprIcon) : TreeElement(pszName,
                                                                                                         psprIcon) {
        m_bOpened = false;
        m_iWex = m_iW;
        m_iHex = m_iH;
        m_iFocus = -1;
    }

    int Tree::Render(int x, int y) {
        int oldY = y;
        x += 24;
        y += 2;
        for (auto & m_vElement : m_vElements) {
            y += m_vElement->Render(x, y);
        }
        return y - oldY;
    }

    int TreeElement::Render(int x, int y) {
        if (m_bSelected) {
            SHR::SetQuad(&q, GV->colActive, x - 2, y, x + m_iW, y + m_iH);
            hge->Gfx_RenderQuad(&q);
        } else if (m_bFocused) {
            SHR::SetQuad(&q, 0x22FFFFFF, x - 2, y, x + m_iW, y + m_iH);
            hge->Gfx_RenderQuad(&q);
        }
        if (sprIcon != NULL) {
            sprIcon->SetColor(0xFFFFFFFF);
            sprIcon->Render(x, y + 2);
        }
        GV->fntMyriad16->Render(x + 20, y + 3, HGETEXT_LEFT, szName, 0);
        return m_iH;
    }

    int TreeFolder::Render(int x, int y) {
        int oldY = y;
        TreeElement::Render(x, y);

        int quarter = m_iH / 4;
        SHR::SetQuad(&q, GV->colFontWhite, x - 12 - quarter, y + quarter, x - 12 + quarter, y + quarter * 3);
        hge->Gfx_RenderQuad(&q, false);
        hge->Gfx_RenderLine(x - 12 - quarter + 1, y + quarter * 2, x - 12 + quarter - 2, y + quarter * 2, GV->colFontWhite);
        if (m_bOpened) {
            hge->Gfx_RenderLine(x - 12, y + quarter * 3, x - 12, y + m_iH, GV->colFontWhite);
        } else {
            hge->Gfx_RenderLine(x - 12, y + quarter + 1, x - 12, y + quarter * 3 - 2, GV->colFontWhite);
        }

        y += m_iH;
        if (m_bOpened && !m_vElements.empty()) {
            hge->Gfx_RenderLine(x - 12, y, x - 12, y + m_iH * (m_vElements.size() - 0.5f), GV->colFontWhite);
            x += 20;
            for (auto & m_vElement : m_vElements) {
                hge->Gfx_RenderLine(x - 32, y + m_iH / 2, x - 4, y + m_iH / 2, GV->colFontWhite);
                y += m_vElement->Render(x, y);
            }
        }
        return y - oldY;
    }

    int TreeElement::GetWidth() const {
        return m_iW;
    }

    int TreeElement::GetHeight() const {
        return m_iH;
    }

    int TreeFolder::GetWidth() const {
        if (m_bOpened)
            return m_iWex;
        else
            return m_iW;
    }

    int TreeFolder::GetHeight() const {
        if (m_bOpened)
            return m_iHex;
        else
            return m_iH;
    }

    int Tree::GetWidth() const {
        return getWidth();
    }

    int Tree::GetHeight() const {
        return getHeight();
    }

    void TreeElement::GainFocus() {
        m_bFocused = true;
    }

    void TreeElement::LoseFocus() {
        m_bFocused = false;
    }

    void TreeElement::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.getX() > 0) {
            GetRoot()->SetSelectedElement(this);
        }
    }

    void TreeFolder::GainFocus() {
        m_bFocused = true;
    }

    void TreeFolder::LoseFocus() {
        m_bFocused = false;

        if (m_iFocus != -1) {
            m_vElements[m_iFocus]->LoseFocus();
            m_iFocus = -1;
        }
    }

    void TreeFolder::AddElement(const std::shared_ptr<TreeElement>& phEl) {
        ((TreeFolder*)&*phEl)->m_hParent = this;
        m_iHex += phEl->GetHeight();
        m_iWex = std::max(m_iWex, phEl->GetWidth() + 20);
        m_vElements.emplace_back(phEl);
    }

    void Tree::adjustSize() {
        setWidth(m_iWex + 24);
        setHeight(m_iHex);
    }

    void Tree::SetSelectedElement(TreeElement *element) {
        if (m_hSelected != NULL) {
            m_hSelected->LoseSelection();
        }
        m_hSelected = element;
        if (element) {
            element->GainSelection();
        }
        distributeActionEvent();
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
        for (auto & m_vElement : m_vElements) {
            m_iHex += m_vElement->GetHeight();
            m_iWex = std::max(m_iWex, m_vElement->GetWidth());
        }
        if (m_hParent != NULL)
            m_hParent->RecalculateSize();
        else
            ((Tree *) this)->adjustSize();
    }

    TreeElement* TreeFolder::GetFocused() {
        if (m_iFocus == -1) return this;
        if (m_vElements[m_iFocus]->IsDir()) {
            return ((TreeFolder*)(&*m_vElements[m_iFocus]))->GetFocused();
        }
        return &*m_vElements[m_iFocus];
    }

    class Tree *TreeElement::GetRoot() {
        return m_hParent ? m_hParent->GetRoot() : (Tree*)this;
    }

    void TreeElement::mouseMoved(MouseEvent &mouseEvent) {
        if (m_bFocused) {
            m_bFocused = mouseEvent.getX() > 0;
        } else if (mouseEvent.getX() > 0) {
            if (IsDir()) {
                LoseFocus();
            }
            GainFocus();
        }
    }

    SHR::TreeFolder* TreeElement::GetParent() {
        return m_hParent;
    }

    std::shared_ptr<TreeElement> TreeFolder::GetElement(int i) {
        return m_vElements[i];
    }
}
