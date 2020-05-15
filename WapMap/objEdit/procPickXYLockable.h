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

    void setEnabled(bool enabled) {
        enabledMaster = enabled;
        tfX->setEnabled(enabledMaster && xEnabled);
        tfY->setEnabled(enabledMaster && yEnabled);
    }

    bool isEnabledX() { return xEnabled; }

    void setEnabledX(bool enabled) { xEnabled = enabled; tfX->setEnabled(enabledMaster && xEnabled); }

    bool isEnabledY() { return yEnabled; }

    void setEnabledY(bool enabled) { yEnabled = enabled; tfY->setEnabled(enabledMaster && yEnabled); }

    void setAllowNegative(bool allowNegative) { tfX->SetNumerical(true, allowNegative); tfY->SetNumerical(true, allowNegative); }
};


#endif //WAPMAP_PROCPICKXYLOCKABLE_H
