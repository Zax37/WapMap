#ifndef H_C_OE_DIALOG
#define H_C_OE_DIALOG

#include "../states/objedit.h"
#include "../../shared/gcnWidgets/wTextDropDown.h"

namespace ObjEdit
{
  class lmEditObjDialogSizes: public gcn::ListModel {
  public:
   std::string getElementAt(int i);
   int getNumberOfElements();
 };

 class cEditObjDialog : public cObjEdit, gcn::ListModel {
  private:
   SHR::RadBut * rbStandard, * rbOther;
   SHR::DropDown * ddStandardDialogs, * ddActivationArea;
   SHR::TextDropDown * tddOtherDialogs;
   SHR::Win * win;
   SHR::Lab * labActivateMode, * labActivationArea;
   SHR::RadBut * rbActivate[3];
   SHR::TextField * tfRespawnTimes;
   SHR::CBox * cbDialog;
   cSoundPlayer * hPlayer;
   lmEditObjDialogSizes * lmSizes;
   std::vector< std::pair<std::string,std::string> > vsStandardSounds; //first - ANIMATION, second - text
  friend class cObjPropVP;
   void UpdateName();
  public:
   cEditObjDialog(WWD::Object * obj, State::EditingWW * st);
   ~cEditObjDialog();
   virtual void Save();
   virtual void Action(const gcn::ActionEvent &actionEvent);
   virtual void Draw();

   //inherited
   std::string getElementAt(int i);
   int getNumberOfElements();
 };

}

#endif
