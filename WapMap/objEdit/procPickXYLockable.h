#ifndef WAPMAP_PROCPICKXYLOCKABLE_H
#define WAPMAP_PROCPICKXYLOCKABLE_H

#include "../../shared/gcnWidgets/wContainer.h"
#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wTextField.h"
#include <guichan/actionlistener.hpp>

class cProcPickXYLockable : public gcn::ActionListener, public SHR::Container {
    int valX, valY;
    float lockedRatio;
    bool locked, enabledMaster = true, xEnabled = true, yEnabled = true;
    SHR::Lab *labMain, *labX, *labY;
    SHR::TextField *tfX, *tfY;
    SHR::But *lockBut;

    void allowLocking(bool allow);

public:
    cProcPickXYLockable(const char *textMain, const char *textX, int valueX,
                        const char *textY, int valueY, bool lock, const char *lockButTooltip, bool allowNegative = false);

    ~cProcPickXYLockable();

    void action(const gcn::ActionEvent &actionEvent) override;

    void draw(Graphics *graphics) override;

    int getValueX() { return valX; }

    int getValueY() { return valY; }

    void setValueX(int val, bool generatesChangeEvent = true);

    void setValueY(int val, bool generatesChangeEvent = true);

    bool isEnabled() { return enabledMaster; }

    void setEnabled(bool enabled);

    bool isEnabledX() { return xEnabled; }

    void setEnabledX(bool enabled);

    bool isEnabledY() { return yEnabled; }

    void setEnabledY(bool enabled);

    void setAllowNegative(bool allowNegative) { tfX->SetNumerical(true, allowNegative); tfY->SetNumerical(true, allowNegative); }

    void setLocked(bool lock) {
        locked = lock;
        lockBut->setRenderBG(lock);

        if (locked) {
            lockedRatio = (float) valX / (float) valY;
            lockBut->setEnabled(true);
        }
    }
};


#endif //WAPMAP_PROCPICKXYLOCKABLE_H
