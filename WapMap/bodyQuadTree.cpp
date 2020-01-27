#include "bodyQuadTree.h"

#include "globals.h"

cBodyQuadTree::cBodyQuadTree(WWD::Plane *owner) {
    iTypicalCellW = owner->GetPlaneWidthPx();
    iTypicalCellH = owner->GetPlaneHeightPx();
    while (!(iTypicalCellW < QUAD_BODY_CELL_DIM || iTypicalCellH < QUAD_BODY_CELL_DIM)) {
        iTypicalCellW /= 2;
        iTypicalCellH /= 2;
    }

    m_hMainParent = this;
    Init(owner, 0, 0, owner->GetPlaneWidthPx(), owner->GetPlaneHeightPx());
    m_hMainParent = NULL;
}

cBodyQuadTree::cBodyQuadTree(WWD::Plane *owner, int x, int y, int w, int h, cBodyQuadTree *parent) {
    m_hMainParent = parent;
    Init(owner, x, y, w, h);
}

cBodyQuadTree *cBodyQuadTree::GetCellByCoords(int x, int y) {
    if (m_bIsContainer)
        return this;

    if (x < m_iX ||
        x > m_iX + m_iCellW * 2 ||
        y < m_iY ||
        y > m_iY + m_iCellH * 2)
        return NULL;
    int lx = x - m_iX, ly = y - m_iY;
    if (lx < m_iCellW && ly < m_iCellH)
        return m_hCells[0]->GetCellByCoords(x, y);
    else if (lx > m_iCellW && ly < m_iCellH)
        return m_hCells[1]->GetCellByCoords(x, y);
    else if (lx > m_iCellW && ly > m_iCellH)
        return m_hCells[2]->GetCellByCoords(x, y);
    else if (lx < m_iCellW && ly > m_iCellH)
        return m_hCells[3]->GetCellByCoords(x, y);
    return NULL;
}

void cBodyQuadTree::AddBody(cPhysicBody *obj) {
    if (!m_bIsContainer) return;
    m_vObjects.push_back(obj);
    obj->ReferenceCell(this);
}

void cBodyQuadTree::DeleteBody(cPhysicBody *obj) {
    if (!m_bIsContainer) return;
    for (int i = 0; i < m_vObjects.size(); i++)
        if (m_vObjects[i] == obj) {
            m_vObjects.erase(m_vObjects.begin() + i);
            i--;
        }
}

void cBodyQuadTree::UpdateBody(cPhysicBody *obj) {
    if (m_hMainParent != NULL) {
        m_hMainParent->UpdateBody(obj);
        return;
    }

    //float x[4], y[4];
    //GetObjectVertices(&x[0], &y[0]);
    cBodyQuadTree *cells[4];
    GetObjectCells(obj, cells);
    obj->ClearCellReferences();
    for (int i = 0; i < 4; i++)
        if (cells[i] != NULL)
            cells[i]->AddBody(obj);
}

void cBodyQuadTree::GetObjectCells(cPhysicBody *obj, cBodyQuadTree *cells[4]) {
    float x[4], y[4];

    x[0] = obj->GetX();
    y[0] = obj->GetY();
    x[1] = obj->GetX() + obj->GetWidth();
    y[1] = obj->GetY();
    x[2] = obj->GetX() + obj->GetWidth();
    y[2] = obj->GetY() + obj->GetHeight();
    x[3] = obj->GetX();
    y[3] = obj->GetY() + obj->GetHeight();

    //cBodyQuadTree * cells[4];
    for (int z = 0; z < 4; z++) {
        cells[z] = GetCellByCoords(x[z], y[z]);
    }

    for (int z = 0; z < 4; z++)
        for (int y = 0; y < 4; y++)
            if (cells[z] == cells[y] && z != y)
                cells[z] = NULL;
}

void cBodyQuadTree::Init(WWD::Plane *owner, int x, int y, int w, int h) {
    m_iX = x;
    m_iY = y;
    if (w < QUAD_BODY_CELL_DIM || h < QUAD_BODY_CELL_DIM) {
        m_bIsContainer = 1;
        m_iCellW = w;
        m_iCellH = h;
        for (int i = 0; i < 4; i++)
            m_hCells[i] = 0;
    } else {
        m_bIsContainer = 0;
        m_iCellW = w / 2;
        m_iCellH = h / 2;
        m_hCells[0] = new cBodyQuadTree(owner, x, y, m_iCellW, m_iCellH, m_hMainParent);
        m_hCells[1] = new cBodyQuadTree(owner, x + m_iCellW, y, m_iCellW, m_iCellH, m_hMainParent);
        m_hCells[2] = new cBodyQuadTree(owner, x + m_iCellW, y + m_iCellH, m_iCellW, m_iCellH, m_hMainParent);
        m_hCells[3] = new cBodyQuadTree(owner, x, y + m_iCellH, m_iCellW, m_iCellH, m_hMainParent);
    }
}

cBodyQuadTree::~cBodyQuadTree() {
    if (!m_bIsContainer) {
        for (int i = 0; i < 4; i++)
            delete m_hCells[i];
    }
}

cPhysicBody *cBodyQuadTree::GetBodyByWorldPosition(int x, int y) {
    if (m_bIsContainer) {
        if (x < m_iX ||
            x > m_iX + m_iCellW ||
            y < m_iY ||
            y > m_iY + m_iCellH)
            return NULL;

        for (int i = 0; i < m_vObjects.size(); i++) {
            cPhysicBody *obj = m_vObjects[i];

            if (x > (obj->GetX()) &&
                x < (obj->GetX() + obj->GetWidth()) &&
                y > (obj->GetY()) &&
                y < (obj->GetY() + obj->GetHeight()))
                return obj;
        }
        return NULL;
    } else {
        if (x < m_iX ||
            x > m_iX + m_iCellW * 2 ||
            y < m_iY ||
            y > m_iY + m_iCellH * 2)
            return NULL;
        int lx = x - m_iX, ly = y - m_iY;
        if (lx < m_iCellW && ly < m_iCellH)
            return m_hCells[0]->GetBodyByWorldPosition(x, y);
        else if (lx > m_iCellW && ly < m_iCellH)
            return m_hCells[1]->GetBodyByWorldPosition(x, y);
        else if (lx > m_iCellW && ly > m_iCellH)
            return m_hCells[2]->GetBodyByWorldPosition(x, y);
        else if (lx < m_iCellW && ly > m_iCellH)
            return m_hCells[3]->GetBodyByWorldPosition(x, y);
    }
}

bool cBodyQuadTree::RectsCollideOrOverlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    bool collide = 0, overlap = 0;
    if (x1 > x2 && x1 < x2 + w2 && y1 > y2 && y1 < y2 + h2 ||
        x1 + w1 > x2 && x1 + w1 < x2 + w2 && y1 > y2 && y1 < y2 + h2 ||
        x1 + w1 > x2 && x1 + w1 < x2 + w2 && y1 + h1 > y2 && y1 + h1 < y2 + h2 ||
        x1 > x2 && x1 < x2 + w2 && y1 + h1 > y2 && y1 + h1 < y2 + h2)
        collide = 1;

    int minx, miny, maxx, maxy;
    if (x1 < x2) {
        minx = x1;
        maxx = x2 + w2;
    } else {
        minx = x2;
        maxx = x1 + w1;
    }
    if (y1 < y2) {
        miny = y1;
        maxy = y2 + h2;
    } else {
        miny = y2;
        maxy = y1 + h1;
    }

    if (!((maxx - minx) > (w1 + w2) || (maxy - miny) > (h1 + h2)))
        overlap = 1;

    if (overlap || collide) return 1;
    else return 0;
}

std::vector<cPhysicBody *> cBodyQuadTree::GetBodiesByArea(int x, int y, int w, int h, bool withhid) {
    if (m_bIsContainer) {
        /*if( x < m_iX ||
            x > m_iX+m_iCellW ||
            y < m_iY ||
            y > m_iY+m_iCellH ) return NULL;*/
        std::vector<cPhysicBody *> ret;

        for (int i = 0; i < m_vObjects.size(); i++) {
            cPhysicBody *obj = m_vObjects[i];

            if (RectsCollideOrOverlap(x, y, w, h, obj->GetX(), obj->GetY(), obj->GetWidth(), obj->GetHeight())
                && (!obj->IsHidden() || obj->IsHidden() && withhid))
                ret.push_back(obj);
        }
        return ret;
    } else {
        std::vector<cPhysicBody *> ret[3], mainret;
        if (RectsCollideOrOverlap(x, y, w, h, m_iX, m_iY, m_iCellW, m_iCellH))
            mainret = m_hCells[0]->GetBodiesByArea(x, y, w, h, withhid);
        if (RectsCollideOrOverlap(x, y, w, h, m_iX + m_iCellW, m_iY, m_iCellW, m_iCellH))
            ret[0] = m_hCells[1]->GetBodiesByArea(x, y, w, h, withhid);
        if (RectsCollideOrOverlap(x, y, w, h, m_iX + m_iCellW, m_iY + m_iCellH, m_iCellW, m_iCellH))
            ret[1] = m_hCells[2]->GetBodiesByArea(x, y, w, h, withhid);
        if (RectsCollideOrOverlap(x, y, w, h, m_iX, m_iY + m_iCellH, m_iCellW, m_iCellH))
            ret[2] = m_hCells[3]->GetBodiesByArea(x, y, w, h, withhid);
        for (int z = 0; z < 3; z++) {
            for (int i = 0; i < ret[z].size(); i++) {
                mainret.push_back(ret[z][i]);
            }
        }
        for (int a = 0; a < mainret.size(); a++) {
            for (int b = 0; b < mainret.size(); b++) {
                if (mainret[a] == mainret[b] && a != b) {
                    mainret.erase(mainret.begin() + b);
                }
            }
        }
        return mainret;
    }
}

int cBodyQuadTree::GetContainerCellWidth() {
    if (m_hMainParent == NULL)
        return iTypicalCellW;
    return m_hMainParent->GetContainerCellWidth();
}

int cBodyQuadTree::GetContainerCellHeight() {
    if (m_hMainParent == NULL)
        return iTypicalCellH;
    return m_hMainParent->GetContainerCellHeight();
}
