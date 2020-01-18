#ifndef H_C_OBJUSERDATA
#define H_C_OBJUSERDATA

#include "../shared/cWWD.h"
#include "objQuadTree.h"

#define GetUserDataFromObj(x) ((cObjUserData*)(x->GetUserData()))

class cObjUserData {
 private:
  WWD::Object * hObj;
  bool bInvert, bMirror, bVisible;
  int iPosX, iPosY, iPosZ, iPosI;
  int * iSpecialData;
  std::vector<cObjectQuadTree*> m_vhCells;
 public:
  cObjUserData(WWD::Object * obj);
  void SyncToObj();
  int GetX(){ return iPosX; };
  int GetY(){ return iPosY; };
  int GetZ(){ return iPosZ; };
  int GetI(){ return iPosI; };
  bool IsVisible(){ return bVisible; };
  void SetVisible(bool b){ bVisible = b; };
  void ReferenceCell(cObjectQuadTree * ptr){ m_vhCells.push_back(ptr); };
  void ClearCellReferences();
  int  GetReferencedCellsCount(){ return m_vhCells.size(); };
  cObjectQuadTree * GetReferencedCell(int i){ return m_vhCells[i]; };
  void SetX(int x);
  void SetY(int y);
  void SetPos(int x, int y);
  void SetI(int i);
  void SetZ(int z){ iPosZ = z; };
  void SetFlip(bool f1, bool f2){ bInvert = f2; bMirror = f1; }
  bool GetFlipX(){ return bMirror; };
  bool GetFlipY(){ return bInvert; };
  void SetSpecialData(int * n){ iSpecialData = n; };
  int * GetSpecialData(){ return iSpecialData; };
};

#endif
