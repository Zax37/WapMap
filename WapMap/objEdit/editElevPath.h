#ifndef H_C_OE_ELEVPATH
#define H_C_OE_ELEVPATH

#include "../states/objedit.h"
#include "../../shared/gcnWidgets/wListbox.h"
#include "../../shared/gcnWidgets/wTextDropDown.h"

namespace ObjEdit
{
  class cEditObjElevPath;

  class lmImgSetEditObjElevPath: public gcn::ListModel {
   public:
    lmImgSetEditObjElevPath(){};
    std::string getElementAt(int i);
    int getNumberOfElements();
  };

  class lmEditObjElevPath: public gcn::ListModel {
  private:
   cEditObjElevPath * m_hOwn;
  public:
   lmEditObjElevPath(cEditObjElevPath * own){ m_hOwn = own; };
   std::string getElementAt(int i);
   int getNumberOfElements();
 };

 class cEditObjElevPath : public cObjEdit {
  private:
   SHR::Win * win;
   SHR::But * butGenChain, * butCloseChain, * butInvertPath, * butInvX, * butInvY;
   SHR::But * butStepAdd, * butStepDel;
   lmEditObjElevPath * lmSteps;
   lmImgSetEditObjElevPath * lmDefImgSets;
   SHR::ListBox * lbSteps;
   SHR::TextField * tfStepMove, * tfStepWaitTime, * tfSpeed;
   SHR::But * butMoveDir[8];
   SHR::RadBut * rbMove, * rbWait;
   SHR::Lab * labMove, * labWaitTime, * labType, * labSteps, * labSpeed, * labImgSet, * labDir;
   SHR::TextDropDown * tddImageSet;
   int iDragStartX, iDragStartY;
   bool bDragging;
   int iDragStepOrigDir, iDragStepOrigMove;
   int iDragStep;
  friend class cObjPropVP;
  protected:
   virtual void _Think(bool bMouseConsumed);
  public:
   cEditObjElevPath(WWD::Object * obj, State::EditingWW * st);
   ~cEditObjElevPath();
   virtual void Save();
   virtual void Action(const gcn::ActionEvent &actionEvent);
   virtual void Draw();
   std::vector< std::pair<int, int> > vSteps;
   int GetMoveX(int i);
   int GetMoveY(int i);
   int GetMoveDir(int x, int y);
   int GetActualStep(){ return lbSteps->getSelected(); };
   void ChangeActiveStep();
   void SynchronizeObj();
   void CloseChain(bool bChangeQuestioned = 0);
 };

}

#endif
