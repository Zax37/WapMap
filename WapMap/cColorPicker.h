#ifndef H_C_COLORPICKER
#define H_C_COLORPICKER

#include "../shared/gcnwidgets/wWin.h"
#include "../shared/gcnwidgets/wButton.h"
#include "../shared/gcnwidgets/wSlider.h"
#include "../shared/gcnwidgets/wTextField.h"
#include "../shared/gcnwidgets/wLabel.h"
#include "wViewport.h"

class cColorPicker : public WIDG::VpCallback, public gcn::ActionListener {
private:
    SHR::Win *winPicker;
    bool bUseAlpha;
    SHR::Lab *labR, *labG, *labB, *labA, *labHex;
    SHR::Slider *sliR, *sliG, *sliB, *sliA;
    SHR::TextField *tfR, *tfG, *tfB, *tfA, *tfHex;
    WIDG::Viewport *vp;
    DWORD dwColor;
public:
    cColorPicker(SHR::Container *dest, int x, int y, DWORD dwInitColor = 0xFFFFFFFF, bool bWithAlpha = 0);

    ~cColorPicker();

    void SetColor(DWORD n);

    DWORD GetColor() { return dwColor; };

    virtual void Draw(int iCode);

    void action(const gcn::ActionEvent &actionEvent);

    void SetName(const char *n) { winPicker->setCaption(n); };
};

#endif
