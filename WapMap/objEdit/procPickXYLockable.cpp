#include "procPickXYLockable.h"
#include "../globals.h"
#include <algorithm>

extern HGE *hge;

cProcPickXYLockable::cProcPickXYLockable(const char *textMain, const char *textX, int valueX, const char *textY,
                                         int valueY, bool lock, const char *lockButTooltip, bool allowNegative) {
    setOpaque(false);

    labMain = new SHR::Lab(textMain);
    labMain->adjustSize();
    SHR::Container::add(labMain, 0, 0);

    labX = new SHR::Lab(textX);
    labX->adjustSize();
    SHR::Container::add(labX, 0, 30);

    labY = new SHR::Lab(textY);
    labY->adjustSize();
    SHR::Container::add(labY, 0, 60);

    int fieldsX = 10 + std::max(labX->getWidth(), labY->getWidth());

    tfX = new SHR::TextField();
    tfX->setDimension(gcn::Rectangle(0, 0, 75, 20));
    tfX->SetNumerical(true, allowNegative);
    tfX->addActionListener(this);
    tfX->setText(std::to_string(valueX));
    tfX->setMaxLength(4);
    SHR::Container::add(tfX, fieldsX, 30);

    tfY = new SHR::TextField();
    tfY->setDimension(gcn::Rectangle(0, 0, 75, 20));
    tfY->SetNumerical(true, allowNegative);
    tfY->addActionListener(this);
    tfY->setText(std::to_string(valueY));
    tfY->setMaxLength(4);
    SHR::Container::add(tfY, fieldsX, 60);

    lockBut = new SHR::But(GV->hGfxInterface, GV->sprIcons16[Icon16_Lock]);
    lockBut->setDimension(gcn::Rectangle(0, 0, 20, 20));
    lockBut->addActionListener(this);
    lockBut->setFocusable(false);
    lockBut->SetTooltip(lockButTooltip);
    SHR::Container::add(lockBut, fieldsX + 81, 46);

    valX = valueX;
    valY = valueY;
    setLocked(lock);
}

cProcPickXYLockable::~cProcPickXYLockable() {
    delete labMain;
    delete labX;
    delete labY;
    delete tfX;
    delete tfY;
    delete lockBut;
}

void cProcPickXYLockable::action(const gcn::ActionEvent &actionEvent) {
    if (actionEvent.getSource() == lockBut) {
        locked = !locked;
        lockBut->setRenderBG(locked);

        if (locked) {
            lockedRatio = std::stof(tfX->getText()) / std::stof(tfY->getText());
        } else if (tfX->getText().empty() || tfY->getText().empty() || tfX->getText()[0] == '0' ||
                   tfY->getText()[0] == '0') {
            allowLocking(false);
        }
    } else if (actionEvent.getSource() == tfX || actionEvent.getSource() == tfY) {
        const std::string &strX = tfX->getText();
        const std::string &strY = tfY->getText();

        bool sourceX = actionEvent.getSource() == tfX;
        if (sourceX) {
            if (strX.empty() || strX[0] == '0') {
                if (!locked) allowLocking(false);
                return;
            }
        } else if (strY.empty() || strY[0] == '0') {
            if (!locked) allowLocking(false);
            return;
        }

        if (locked) {
            float fValX, fValY;
            if (sourceX) {
                fValX = std::stof(tfX->getText());
                fValY = round(fValX / lockedRatio);
                tfY->setText(std::to_string((int) fValY));
            } else {
                fValY = std::stof(tfY->getText());
                fValX = round(fValY * lockedRatio);
                tfX->setText(std::to_string((int) fValX));
            }
            valX = fValX;
            valY = fValY;
            mActionEventId = "CHANGE_X";
            distributeActionEvent();
            mActionEventId = "CHANGE_Y";
            distributeActionEvent();
        } else if (sourceX) {
            valX = std::stoi(tfX->getText());
            mActionEventId = "CHANGE_X";
            distributeActionEvent();
            allowLocking(strY[0] != '0');
        } else {
            valY = std::stoi(tfY->getText());
            mActionEventId = "CHANGE_Y";
            distributeActionEvent();
            allowLocking(strX[0] != '0');
        }
    }
}

void cProcPickXYLockable::allowLocking(bool allow) {
    if (lockBut->isEnabled() == allow) return;

    lockBut->setEnabled(allow);
    if (!allow) {
        locked = false;
        lockBut->setRenderBG(false);
    }
}

void cProcPickXYLockable::draw(Graphics *graphics) {
    int dx, dy;
    getAbsolutePosition(dx, dy);

    drawChildren(graphics);

    if (locked) {
        const gcn::Rectangle &butDim = lockBut->getDimension();
        int butX = butDim.x + butDim.width / 2 + dx + 1;
        int butTop = butDim.y + dy - 1;
        int butBot = butTop + butDim.height;
        int lineW = butDim.width / 2 + 2;
        int lineH = 5;

        hge->Gfx_RenderLine(butX - lineW, butTop - lineH, butX, butTop - lineH, GV->colLineBright);
        hge->Gfx_RenderLine(butX, butTop - lineH, butX, butTop - 1, GV->colLineBright);

        hge->Gfx_RenderLine(butX, butBot + 1, butX, butBot + lineH + 1, GV->colLineBright);
        hge->Gfx_RenderLine(butX - lineW, butBot + lineH + 1, butX, butBot + lineH + 1, GV->colLineBright);
    }
}

void cProcPickXYLockable::setValueX(int val, bool generatesChangeEvent) {
    valX = val;
    tfX->setText(std::to_string(val));
    mActionEventId = "CHANGE_X";
    distributeActionEvent();
}

void cProcPickXYLockable::setValueY(int val, bool generatesChangeEvent) {
    valY = val;
    tfY->setText(std::to_string(val));
    mActionEventId = "CHANGE_Y";
    distributeActionEvent();
}

void cProcPickXYLockable::setEnabled(bool enabled) {
    enabledMaster = enabled;
    labMain->setColor(enabledMaster ? 0xFFa1a1a1 : 0xFF000000);
    tfX->setEnabled(enabledMaster && xEnabled);
    labX->setColor(enabledMaster && xEnabled ? 0xFFa1a1a1 : 0xFF000000);
    tfY->setEnabled(enabledMaster && yEnabled);
    labY->setColor(enabledMaster && yEnabled ? 0xFFa1a1a1 : 0xFF000000);
}

void cProcPickXYLockable::setEnabledX(bool enabled) {
    xEnabled = enabled;
    tfX->setEnabled(enabledMaster && xEnabled);
    labX->setColor(enabledMaster && enabled ? 0xFFa1a1a1 : 0xFF000000);
}

void cProcPickXYLockable::setEnabledY(bool enabled) {
    yEnabled = enabled;
    tfY->setEnabled(enabledMaster && yEnabled);
    labY->setColor(enabledMaster && enabled ? 0xFFa1a1a1 : 0xFF000000);
}
