#ifndef H_C_COLORPICKER
#define H_C_COLORPICKER

#include "../shared/gcnWidgets/wWin.h"
#include "../shared/gcnWidgets/wButton.h"
#include "../shared/gcnWidgets/wSlider.h"
#include "../shared/gcnWidgets/wTextField.h"
#include "../shared/gcnWidgets/wLabel.h"
#include "wViewport.h"

class cColorPicker : public WIDG::VpCallback, public gcn::ActionListener {
 private:
  SHR::Win * winPicker;
  bool bUseAlpha;
  SHR::Lab * labR, * labG, * labB, * labA, * labHex;
  SHR::Slider * sliR, * sliG, * sliB, * sliA;
  SHR::TextField * tfR, * tfG, * tfB, * tfA, * tfHex;
  WIDG::Viewport * vp;
  DWORD dwColor;
 public:
  cColorPicker(SHR::Contener * dest, int x, int y, DWORD dwInitColor = 0xFFFFFFFF, bool bWithAlpha = 0);
  ~cColorPicker();
  void SetColor(DWORD n);
  DWORD GetColor(){ return dwColor; };

  virtual void Draw(int iCode);
  void action(const gcn::ActionEvent &actionEvent);
  void SetName(const char * n){ winPicker->setCaption(n); };
};

#endif
