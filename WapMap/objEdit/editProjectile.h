#ifndef H_C_OE_PROJECTILE
#define H_C_OE_PROJECTILE

#include "../states/objedit.h"

namespace ObjEdit
{
 class cEditObjProjectile : public cObjEdit {
  private:
   SHR::Lab * labSpeed, * labDelay, * labDamage;
   SHR::TextField * tfSpeed, * tfDelay, * tfDamage;
   SHR::Lab * labDir;
   SHR::But * butDir[4];
   SHR::Win * win;

   SHR::Lab * labActivRect;
   SHR::Lab * labX1, * labY1, * labX2, * labY2;
   SHR::TextField * tfX1, * tfY1, * tfX2, * tfY2;
   SHR::But * butPick;
   bool bPick;
   bool bDrag;
   int iDragX, iDragY;
   int iDragmX, iDragmY;
   void UpdateRectMarks(bool updv = 0);
   void EnableSave();
   void SetDirection(int dir);
  friend class cObjPropVP;
  protected:
   virtual void _Think(bool bMouseConsumed);
  public:
   cEditObjProjectile(WWD::Object * obj, State::EditingWW * st);
   ~cEditObjProjectile();
   virtual void Save();
   virtual void Action(const gcn::ActionEvent &actionEvent);
   virtual void Draw();
 };

}

#endif
