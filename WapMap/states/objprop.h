#ifndef H_C_OBJECTPROP
#define H_C_OBJECTPROP

#include "../../shared/cStateMgr.h"
#include "../../shared/gcnWidgets/wContainer.h"
#include "../../shared/gcnWidgets/wSlider.h"
#include "../../shared/gcnWidgets/wWin.h"
#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wScrollArea.h"
#include "../../shared/gcnWidgets/wCheckbox.h"
#include "../../shared/gcnWidgets/wDropDown.h"
#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wTextField.h"
#include "../../shared/gcnWidgets/wTabbedArea.h"
#include "../../shared/gcnWidgets/wRadioBut.h"
#include "../../shared/gcnWidgets/wTextDropDown.h"
#include "../../shared/cWWD.h"
#include "../wViewport.h"
#include "../cSoundPlayer.h"
#include "guichan.hpp"

class cBankImageSet;
class cSprBankAsset;
class cSprBankAssetIMG;

namespace State {
 class EditingWW;

#define OBJVP_ADV    0
#define OBJVP_ISPICK 1

class cObjectProp;

enum RectPickType {
 PickRect_MinMax = 0,
 PickRect_Move,
 PickRect_Hit,
 PickRect_Attack,
 PickRect_Clip,
 PickRect_User1,
 PickRect_User2
};

class cObjPropVP: public WIDG::VpCallback {
 private:
  cObjectProp * m_hOwn;
 public:
  virtual void Draw(int iCode);
  cObjPropVP(cObjectProp * owner);
};

 class cObjectPropAL: public gcn::ActionListener {
  private:
   cObjectProp * m_hOwn;
  public:
   void action(const gcn::ActionEvent &actionEvent);
   cObjectPropAL(cObjectProp * owner){ m_hOwn = owner; };
 };

class cRectPropGUI: public gcn::ActionListener {
 public:
  SHR::Lab * labDesc, * labX1, * labY1, * labX2, * labY2;
  SHR::TextField * tfX1, * tfY1, * tfX2, * tfY2;
  SHR::But * butPick, * butClear;

  cRectPropGUI(WWD::Rect * src, SHR::Contener * dest, int dx, int dy, const char * desc);
  ~cRectPropGUI();

  WWD::Rect GetRect();
  void SetRect(WWD::Rect r);
  void action(const gcn::ActionEvent &actionEvent);
};

class ObjProp;

class cObjectProp: public gcn::ListModel {
 protected:
  bool bKill;
  bool bCanceled;
  bool bSwap;
  bool bLogicEditQuit;
  RectPickType iPickType;
  cObjectPropAL * hAL;
  SHR::Win * window;
  WWD::Object * hObj;
  SHR::Lab * labadvName, * labadvLogicType, * labadvLogic, * labadvImageSet, * labadvAnim, * labadvID, * labadvLocX, * labadvLocY, * labadvLocZ, * labadvLocI,
           * labadvDrawFlags, * labadvDynamicFlags, * labadvAddFlags, * labadvHitType, * labadvType, * labadvUserFlags,
           * labadvScore, * labadvPoints, * labadvPowerup, * labadvDamage, * labadvSmarts, * labadvHealth, * labadvUserValues, * labadvUserValue[8],
           * labadvSpeedX, * labadvSpeedY, * labadvTweakX, * labadvTweakY, * labadvCounter,
           * labadvSpeed, * labadvWidth, * labadvHeight, * labadvDirection, * labadvFaceDir, * labadvTimeDelay, * labadvFrameDelay, * labadvMoveResX,
           * labadvMoveResY;
  SHR::TextDropDown * tddadvImageSet, * tddadvAnim, * tddadvLogic, * tddadvCustomLogic;
  SHR::TextField * tfadvName, * tfadvID, * tfadvLocX, * tfadvLocY, * tfadvLocZ, * tfadvLocI,
                 * tfadvScore, * tfadvPoints, * tfadvPowerup, * tfadvDamage, * tfadvSmarts, * tfadvHealth, * tfadvUserValue[8], * tfadvSpeedX, * tfadvSpeedY, * tfadvTweakX, * tfadvTweakY, * tfadvCounter, * tfadvSpeed,
                 * tfadvWidth, * tfadvHeight, * tfadvDirection, * tfadvFaceDir, * tfadvTimeDelay, * tfadvFrameDelay, * tfadvMoveResX, * tfadvMoveResY;
  SHR::But * butSave, * butCancel, * butSelListImageSet, * butCustomLogicAdd, * butCustomLogicEdit, * butCustomLogicDelete;
  SHR::CBox * cbadvDraw_No, * cbadvDraw_Mirror, * cbadvDraw_Invert, * cbadvDraw_Flash, * cbadvDynamic_NoHit, * cbadvDynamic_AlwaysActive,
            * cbadvDynamic_Safe, * cbadvDynamic_AutoHitDmg, * cbadvAdd_Difficult, * cbadvAdd_EyeCandy, * cbadvAdd_HighDetail, * cbadvAdd_Multiplayer,
            * cbadvAdd_ExtraMemory, * cbadvAdd_FastCPU, * cbadvHitType[12], * cbadvUser[12];
  SHR::RadBut * rbLogicStandard, * rbLogicCustom;
  cRectPropGUI * guirectMinMax, * guirectMove, * guirectHit, * guirectAttack, * guirectClip, * guirectUser[2];
  SHR::DropDown * ddAdvType;
  gcn::GenericListModel * lmAdvType;
  cObjPropVP * vpCB;
  WIDG::Viewport * vpAdv, * vpisPick;
  State::EditingWW * hState;

  SHR::Win * winisPick;
  SHR::Slider * sliisPick, * sliisFrame;
  SHR::But * butisOK;
  SHR::Lab * labisFrame, * labisImageSet;
  cSprBankAsset * asImageSetPick;
  cSprBankAssetIMG * asimgImageSetFrame;
  int iisPickIT, iisFrameIT;

  std::string strNameHolder;

  friend class cObjPropVP;
  friend class cObjectPropAL;
  friend class cRectPropGUI;
  friend class ObjProp;
 public:
  ~cObjectProp();
  cObjectProp(WWD::Object * obj, bool vis, State::EditingWW * st, bool addtabs = 1);
  WWD::Object * GetObject(){ return hObj; };
  SHR::Win * GetWindow(){ return window; };
  SHR::Win * GetPickWindow(){ return winisPick; };
  bool Kill(){ return bKill; };
  bool Swap(){ return bSwap; };
  void SwapDone(){ bSwap = 0; };
  bool Canceled(){ return bCanceled; };
  void Save();

  //inherited from listmodel
  std::string getElementAt(int i);
  int getNumberOfElements();
};

 class ObjProp : public SHR::cState {
  public:
   ObjProp(EditingWW * main, WWD::Object * obj, bool bMove = 0);
   virtual bool Opaque(){ return 0; };
   virtual void Init();
   virtual void Destroy();
   virtual bool Think();
   virtual bool Render();
   virtual void GainFocus(int iReturnCode, bool bFlipped);
   //virtual void GainFocus(int iReturnCode);

   bool bRenderHand;
   WWD::Object * hObj;
   gcn::Gui * gui;
   cObjectProp * hProp;
   EditingWW * mainSt;
   SHR::Contener * conMain;
   bool bMoveObject;
  //friend class LoadMapActionListener;
 };

};

#endif
