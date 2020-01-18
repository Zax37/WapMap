#ifndef H_C_OBJQTREE
#define H_C_OBJQTREE

#include "../shared/cWWD.h"

class cBankImageSet;

#define QUAD_CELL_DIM 500

class cObjectQuadTree {
 private:
  cObjectQuadTree * m_hMainParent;
  cBankImageSet * m_hBank;
  int iTypicalCellW, iTypicalCellH;
  bool m_bIsContainer;
  std::vector<WWD::Object*> m_vObjects;
  cObjectQuadTree * m_hCells[4];
  int m_iCellW, m_iCellH;
  int m_iX, m_iY;
  cObjectQuadTree(WWD::Plane * owner, int x, int y, int w, int h, cObjectQuadTree * parent);
  void Init(WWD::Plane * parent, int x, int y, int w, int h);

  void Fill(WWD::Plane * owner);
  void AddObject(WWD::Object * obj);

  void GetObjectCells(WWD::Object * obj, cObjectQuadTree * cells[4]);
  bool RectsCollideOrOverlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);

  cBankImageSet * GetBank(){ if( m_hMainParent == NULL ) return m_hBank; else return m_hMainParent->m_hBank; };
 public:
  cObjectQuadTree(WWD::Plane * parent, cBankImageSet * bank);
  ~cObjectQuadTree();
  WWD::Object * GetObjectByWorldPosition(int x, int y);
  std::vector<WWD::Object*> GetObjectsByArea(int x, int y, int w, int h);
  void UpdateObject(WWD::Object * obj);
  void DeleteObject(WWD::Object * obj);
  int GetContainerCellWidth();
  int GetContainerCellHeight();
  int GetObjectsCount(){ return m_vObjects.size(); };
  cObjectQuadTree * GetCellByCoords(int x, int y);
};

#endif
