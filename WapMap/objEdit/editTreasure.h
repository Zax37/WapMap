#ifndef H_C_OE_TREASURE
#define H_C_OE_TREASURE

#include "../states/objedit.h"

namespace ObjEdit
{
 class cEditObjTreasure : public cObjEdit {
  private:
   SHR::RadBut * rbType[31];
   SHR::CBox * cbAddGlitter;
   SHR::Win * win;
  friend class cObjPropVP;
  public:
   cEditObjTreasure(WWD::Object * obj, State::EditingWW * st);
   ~cEditObjTreasure();
   virtual void Action(const gcn::ActionEvent &actionEvent);
   virtual void Draw();
 };

}

#endif
