#ifndef H_C_BODYQTREE
#define H_C_BODYQTREE

#include "cPhysics.h"

#define QUAD_BODY_CELL_DIM 500

class cBodyQuadTree {
private:
    cBodyQuadTree *m_hMainParent;
    int iTypicalCellW, iTypicalCellH;
    bool m_bIsContainer;
    std::vector<cPhysicBody *> m_vObjects;
    cBodyQuadTree *m_hCells[4];
    int m_iCellW, m_iCellH;
    int m_iX, m_iY;

    cBodyQuadTree(WWD::Plane *owner, int x, int y, int w, int h, cBodyQuadTree *parent);

    void Init(WWD::Plane *parent, int x, int y, int w, int h);

    void AddBody(cPhysicBody *obj);

    void GetObjectCells(cPhysicBody *obj, cBodyQuadTree *cells[4]);

    cBodyQuadTree *GetCellByCoords(int x, int y);

    bool RectsCollideOrOverlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);

    friend class cPhysicWorld;

public:
    cBodyQuadTree(WWD::Plane *parent);

    ~cBodyQuadTree();

    cPhysicBody *GetBodyByWorldPosition(int x, int y);

    std::vector<cPhysicBody *> GetBodiesByArea(int x, int y, int w, int h, bool withhid = 1);

    void UpdateBody(cPhysicBody *obj);

    void DeleteBody(cPhysicBody *obj);

    int GetContainerCellWidth();

    int GetContainerCellHeight();
};

#endif
