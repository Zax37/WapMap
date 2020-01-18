#ifndef H_WINDOW
#define H_WINDOW
#include "../wViewport.h"
#include "../../shared/gcnWidgets/wWin.h"

class cWindow : public WIDG::VpCallback, public gcn::ActionListener
{
 protected:
  SHR::Win * myWin;
 public:
  virtual ~cWindow(){};

  virtual void Think(){};
  virtual void OnDocumentChange(){};
  virtual void Open(){};
  virtual void Close(){};
  virtual void OnRegistered(){};

  SHR::Win * GetWindow(){ return myWin; };

  virtual void action(const gcn::ActionEvent&){};
};

#endif // H_WINDOW
