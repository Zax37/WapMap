#ifndef H_C_OE_CRABNEST
#define H_C_OE_CRABNEST

#include "../states/objedit.h"
#include "procPickRect.h"

namespace ObjEdit
{
 class cEditObjCrabNest : public cObjEdit {
  private:
   SHR::Lab * labNumber, * labActivate;
   SHR::TextField * tfNumber;
   SHR::Win * win;
   cProcPickRect * hRectPick;
  friend class cObjPropVP;
  protected:
   virtual void _Think(bool bMouseConsumed);
  public:
   cEditObjCrabNest(WWD::Object * obj, State::EditingWW * st);
   ~cEditObjCrabNest();
   virtual void Save();
   virtual void Action(const gcn::ActionEvent &actionEvent);
 };

}

#endif
