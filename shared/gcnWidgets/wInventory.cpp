#include "wInventory.h"

#include "guichan/exception.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "guichan/key.hpp"
#include "guichan/mouseevent.hpp"
#include "guichan/mouseinput.hpp"

#include "../../WapMap/globals.h"
#include "../../WapMap/states/editing_ww.h"

#include "../../WapMap/databanks/imageSets.h"

extern HGE *hge;

namespace SHR {
    InvTab::InvTab(guiParts *Parts)
            : mHasMouse(false),
              mKeyPressed(false),
              mMousePressed(false) {
        setFocusable(true);
        adjustSize();
        setFrameSize(0);

        addMouseListener(this);
        hGfx = Parts;

        mItem = cInventoryItem("", -1);
        setId("INVTAB");
    }

    void InvTab::draw(Graphics *graphics) {
        ClipRectangle rect = graphics->getCurrentClipArea();
        int x, y;
        getAbsolutePosition(x, y);

        graphics->setColor(GV->colBaseGCN - 0x0d0d0d);
        graphics->drawLine(0, 0, getWidth(), 0);
        graphics->drawLine(0, 0, 0, getHeight());

        graphics->drawLine(getWidth() - 1, getHeight() - 1, getWidth() - 1, 0);
        graphics->drawLine(getWidth(), getHeight() - 1, 0, getHeight() - 1);

        graphics->setColor(GV->colBaseGCN - 0x383838);
        graphics->drawLine(1, 1, getWidth() - 2, 1);
        graphics->drawLine(1, 1, 1, getHeight() - 2);

        graphics->drawLine(getWidth() - 2, getHeight() - 2, getWidth() - 2, 1);
        graphics->drawLine(getWidth() - 1, getHeight() - 2, 1, getHeight() - 2);

        graphics->pushClipArea(gcn::Rectangle(2, 2, getWidth() - 4, getHeight() - 4));
        if (isEnabled())
            graphics->setColor(GV->colBaseGCN - 0x111111);
        else
            graphics->setColor(GV->colBaseGCN - 0x232323);
        graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
        graphics->popClipArea();

        if (mItem.second != -1) {
            cSprBankAsset *asset = GV->editState->SprBank->GetAssetByID(mItem.first.c_str());
            int iframe = GV->editState->hInvCtrl->GetAnimFrame() % asset->GetSpritesCount();
            hgeSprite *spr = asset->GetIMGByIterator(iframe)->GetSprite();
            if (isEnabled())
                spr->SetColor(0xFFFFFFFF);
            else
                spr->SetColor(0x77FFFFFF);
            int grdim = spr->GetWidth();
            if (spr->GetHeight() > grdim) grdim = spr->GetHeight();
            float fScale = 1.0f;
            if (grdim > (getWidth() - 4)) fScale = float(getWidth() - 4) / float(grdim);
            spr->RenderEx(x + getWidth() / 2, y + getHeight() / 2, 0, fScale);
        }
    }

    void InvTab::adjustSize() {
        setWidth(50);
        setHeight(50);
    }

    void InvTab::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT) {
            float mx, my;
            hge->Input_GetMousePos(&mx, &my);
            int x, y;
            getAbsolutePosition(x, y);
            GV->editState->hInvCtrl->SetItemInClipboard(mItem, mx - (x + getWidth() / 2), my - (y + getHeight() / 2));
            mItem = cInventoryItem("", -1);
            mMousePressed = true;
            mouseEvent.consume();
            distributeActionEvent();
        }
    }

    void InvTab::mouseExited(MouseEvent &mouseEvent) {
        mHasMouse = false;
    }

    void InvTab::mouseEntered(MouseEvent &mouseEvent) {
        mHasMouse = true;
        requestFocus();
    }

    void InvTab::mouseReleased(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == MouseEvent::LEFT) {
            mMousePressed = false;
            mouseEvent.consume();
        }
    }

    void InvTab::ObjectDropped() {
        mItem = GV->editState->hInvCtrl->GetItemInClipboard();
        GV->editState->hInvCtrl->SetClipboardEmpty();
        distributeActionEvent();
    }

    void InvTab::mouseDragged(MouseEvent &mouseEvent) {
        mouseEvent.consume();
    }
}
