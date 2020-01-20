#ifndef H_C_OE_SHAKE
#define H_C_OE_SHAKE

#include "../states/objedit.h"
#include "procPickRect.h"

namespace ObjEdit
{
 class cEditObjShake : public cObjEdit {
  private:
   SHR::Lab * labActivate;
   SHR::Win * win;
   cProcPickRect * hRectPick;
  friend class cObjPropVP;
  protected:
   virtual void _Think(bool bMouseConsumed);
  public:
   cEditObjShake(WWD::Object * obj, State::EditingWW * st);
   ~cEditObjShake();
   virtual void Action(const gcn::ActionEvent &actionEvent);
   virtual void RenderObjectOverlay();
 };

}

#endif
