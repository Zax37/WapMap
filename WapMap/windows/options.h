#ifndef H_WIN_OPTIONS
#define H_WIN_OPTIONS

#include "window.h"

namespace SHR
{
 class Lab;
 class But;
 class DropDown;
 class CBox;
 class TextField;
}

class cDynamicListModel;

 class cListModelDisplay: public gcn::ListModel {
  private:
   std::vector<std::string> m_vOpt;
  public:
   cListModelDisplay(bool bCrazyHook = 0);
   ~cListModelDisplay();

   std::string getElementAt(int i);
   int getNumberOfElements();
 };

class winOptions : public cWindow
{
 private:
  SHR::Lab * laboptChangesRes, * laboptChangesLang, * laboptRes, * laboptLang, * laboptGameRes,
           * laboptEditorSettings, * laboptCrazyHookSettings, * labscpDesc, * labscpVersion;
  SHR::But * butscpPath, * butoptSave;
  SHR::DropDown * ddoptLang, * ddoptRes, * ddoptGameRes;
  SHR::CBox * cbOptionsAlfaHigherPlanes, * cboptCrazyHookGodMode, * cboptCrazyHookArmor, * cboptCrazyHookDebugInfo,
            * cboptAutoUpdate, * cboptSmoothZooming;
  SHR::TextField * tfscpPath;
  WIDG::Viewport * vp;
  cDynamicListModel * lmLang;
 public:
  winOptions();
  ~winOptions();

  virtual void Draw(int piCode);
  virtual void Think();
  virtual void Open();
  virtual void Close();
  virtual void action(const ActionEvent& actionEvent);

  void SyncWithExe();
  void PickAndSetClawLocation();
};

#endif // H_WIN_TILEBROWSER
