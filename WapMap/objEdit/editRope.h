#ifndef H_C_OE_ROPE
#define H_C_OE_ROPE

#include "../states/objedit.h"

namespace ObjEdit
{
 class cEditObjRope : public cObjEdit {
  private:
   SHR::CBox * cbAnimated;
   SHR::Lab * labSpeed, * labOffset;
   SHR::TextField * tfSpeed, * tfOffset;
   SHR::Win * win;
  friend class cObjPropVP;
  public:
   cEditObjRope(WWD::Object * obj, State::EditingWW * st);
   ~cEditObjRope();
   virtual void Save();
   virtual void Action(const gcn::ActionEvent &actionEvent);
 };

}

#endif
