#ifndef H_STATE_ERROR
#define H_STATE_ERROR

#include "../../shared/cStateMgr.h"
#include "../../shared/gcnWidgets/wWin.h"
#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/cWWD.h"
#include "guichan.hpp"

#define ST_ER_ICON_WARNING 1
#define ST_ER_ICON_FATAL 2
#define ST_ER_ICON_INFO 3

#define ST_ER_BUT_OK 1
#define ST_ER_BUT_YESNO 2
#define ST_ER_BUT_OKCANCEL 3

#define RETURN_OK 0
#define RETURN_CANCEL 1
#define RETURN_YES 2
#define RETURN_NO 3

namespace State {
 int ErrorStandalone(const char * pszTitle, const char * pszErrorString, int piIcon, int piButtons, bool pbOpaque);

 class Error : public SHR::cState {
  public:
   Error(const char * pszTitle, const char * pszErrorString, int piIcon, int piButtons, bool pbOpaque, bool pbStandalone = 0);
   ~Error();
   virtual bool Opaque();
   virtual void Init();
   virtual void Destroy();
   virtual bool Think();
   virtual bool Render();
   virtual void GainFocus(int iReturnCode, bool bFlipped);
   int GetReturn(){ return ret; };
  private:
   bool standalone;
   bool stop;
   int ret;
   gcn::Gui * gui;
   SHR::Win * winError;
   int iIcon, iButtons;
   SHR::But * butOK, * butYes, * butNo, * butCancel;
   bool bOpaque;
   char * szErrorString, * szTitle;
  friend class ErrorActionListener;
 };
};

#endif
