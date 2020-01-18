#ifndef H_C_OE_TOGGLEPEG
#define H_C_OE_TOGGLEPEG

#include "../states/objedit.h"

namespace ObjEdit
{
 class cEditObjTogglePeg : public cObjEdit {
  private:
   SHR::RadBut * rbType[3];
   SHR::Lab * labTimeOn, * labTimeOff, * labOffset, * labnobjDelay, * labIncrement;
   SHR::TextField * tfTimeOn, * tfTimeOff, * tfOffset, * tfnobjDelay;
   SHR::Win * win, * winSeries;
   SHR::CBox * cbAlwaysOn;
   std::vector<std::string> vszDefaultImgsets;
  friend class cObjPropVP;
  public:
   cEditObjTogglePeg(WWD::Object * obj, State::EditingWW * st);
   ~cEditObjTogglePeg();
   virtual void Save();
   virtual void Action(const gcn::ActionEvent &actionEvent);
   virtual void Draw();
   virtual void * GenerateNextObjectData();
   virtual void ApplyDataFromPrevObject(void * ptr);
 };

}

#endif
