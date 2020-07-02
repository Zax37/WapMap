#include "objQuadTree.h"
#include "globals.h"
#include "cObjectUserData.h"
#include "databanks/imageSets.h"
#include "states/editing_ww.h"

cObjectQuadTree::cObjectQuadTree(WWD::Plane *owner, cBankImageSet *bank) {
    m_hBank = bank;
    iTypicalCellW = owner->GetPlaneWidthPx() + 4 * WORKSPACE_MARGINS_SIZE;
    iTypicalCellH = owner->GetPlaneHeightPx() + 4 * WORKSPACE_MARGINS_SIZE;
    while (!(iTypicalCellW < QUAD_CELL_DIM || iTypicalCellH < QUAD_CELL_DIM)) {
        iTypicalCellW /= 2;
        iTypicalCellH /= 2;
    }

    m_hMainParent = this;
    Init(owner, -2 * WORKSPACE_MARGINS_SIZE, -2 * WORKSPACE_MARGINS_SIZE,
         owner->GetPlaneWidthPx() + 4 * WORKSPACE_MARGINS_SIZE,
         owner->GetPlaneHeightPx() + 4 * WORKSPACE_MARGINS_SIZE);
    m_hMainParent = NULL;
    Fill(owner);
}

cObjectQuadTree::cObjectQuadTree(WWD::Plane *owner, int x, int y, int w, int h, cObjectQuadTree *parent) {
    m_hMainParent = parent;
    Init(owner, x, y, w, h);
}

cObjectQuadTree *cObjectQuadTree::GetCellByCoords(int x, int y) {
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

void cObjectQuadTree::AddObject(WWD::Object *obj) {
    if (!m_bIsContainer) return;
    m_vObjects.push_back(obj);
    GetUserDataFromObj(obj)->ReferenceCell(this);
}

void cObjectQuadTree::DeleteObject(WWD::Object *obj) {
    if (!m_bIsContainer) return;
    for (int i = 0; i < m_vObjects.size(); i++)
        if (m_vObjects[i] == obj) {
            m_vObjects.erase(m_vObjects.begin() + i);
            i--;
        }
}

void cObjectQuadTree::UpdateObject(WWD::Object *obj) {
    if (m_hMainParent != NULL) {
        m_hMainParent->UpdateObject(obj);
        return;
    }

    cObjectQuadTree *cells[4];
    GetObjectCells(obj, cells);
    cObjUserData *ud = GetUserDataFromObj(obj);
    ud->ClearCellReferences();
    for (auto & cell : cells)
        if (cell != NULL)
            cell->AddObject(obj);
}

void cObjectQuadTree::GetObjectCells(WWD::Object *obj, cObjectQuadTree *cells[4]) {
    float posX = obj->GetX(), posY = obj->GetY();

    cSprBankAsset *asset = GetBank()->GetAssetByID(obj->GetImageSet());
    int iw, ih;
    float ihx, ihy;

    if (asset == NULL || asset->GetSpritesCount() == 0) {
        iw = GV->sprSmiley->GetWidth();
        ih = GV->sprSmiley->GetHeight();
        GV->sprSmiley->GetHotSpot(&ihx, &ihy);
    } else {
        int count = asset->GetSpritesCount();
        iw = 0;
        ih = 0;
        for (int z = 0; z < count; z++) {
            hgeSprite* spr = asset->GetIMGByIterator(z)->GetSprite();
            if (!spr) {
                spr = GV->sprSmiley;
            }

            if (spr->GetWidth() > iw) {
                iw = spr->GetWidth();
                float tmp;
                spr->GetHotSpot(&ihx, &tmp);
            }

            if (spr->GetHeight() > ih) {
                ih = spr->GetHeight();
                float tmp;
                spr->GetHotSpot(&tmp, &ihy);
            }
        }
    }

    float x[4], y[4];
    float hsx, hsy;
    hsx = ihx;
    hsy = ihy;
    float sprw = iw / 2, sprh = ih / 2;
    hsx -= sprw;
    hsy -= sprh;
    x[0] = posX - sprw - hsx;
    y[0] = posY - sprw - hsx;
    x[1] = posX + sprw - hsx;
    y[1] = posY - sprw - hsx;
    x[2] = posX + sprw - hsy;
    y[2] = posY + sprw - hsy;
    x[3] = posX - sprw - hsx;
    y[3] = posY + sprw - hsx;

    for (int z = 0; z < 4; z++) {
        cells[z] = GetCellByCoords(x[z], y[z]);
    }

    for (int z = 0; z < 4; z++)
        for (int y = 0; y < 4; y++)
            if (cells[z] == cells[y] && z != y)
                cells[z] = NULL;
}

void cObjectQuadTree::Fill(WWD::Plane *owner) {
    for (int i = 0; i < owner->GetObjectsCount(); i++) {
        WWD::Object *obj = owner->GetObjectByIterator(i);
        //float x[4], y[4];
        //GetObjectVertices(obj, &x[0], &y[0]);

        cObjectQuadTree *cells[4];
        GetObjectCells(obj, cells);
        /*for(int z=0;z<4;z++){
         cells[z] = GetCellByCoords(x[z], y[z]);
        }

        for(int z=0;z<4;z++)
         for(int y=0;y<4;y++)
          if( cells[z] == cells[y] && z != y )
           cells[z] = NULL;*/

        for (auto & cell : cells)
            if (cell != NULL)
                cell->AddObject(obj);
        //cells[z]->m_vObjects.push_back(obj);

        /*if( (x1 > m_iX && x1 < m_iX+m_iCellW && y1 > m_iY && y1 < m_iY+m_iCellH) ||
            (x2 > m_iX && x2 < m_iX+m_iCellW && y1 > m_iY && y1 < m_iY+m_iCellH) ||
            (x2 > m_iX && x2 < m_iX+m_iCellW && y2 > m_iY && y2 < m_iY+m_iCellH) ||
            (x1 > m_iX && x1 < m_iX+m_iCellW && y2 > m_iY && y2 < m_iY+m_iCellH) ){
         m_vObjects.push_back(obj);*/
    }
}

void cObjectQuadTree::Init(WWD::Plane *owner, int x, int y, int w, int h) {
    m_iX = x;
    m_iY = y;
    if (w < QUAD_CELL_DIM || h < QUAD_CELL_DIM) {
        m_bIsContainer = true;
        m_iCellW = w;
        m_iCellH = h;
        for (auto & m_hCell : m_hCells)
            m_hCell = 0;
    } else {
        m_bIsContainer = false;
        m_iCellW = w / 2;
        m_iCellH = h / 2;
        m_hCells[0] = new cObjectQuadTree(owner, x, y, m_iCellW, m_iCellH, m_hMainParent);
        m_hCells[1] = new cObjectQuadTree(owner, x + m_iCellW, y, m_iCellW, m_iCellH, m_hMainParent);
        m_hCells[2] = new cObjectQuadTree(owner, x + m_iCellW, y + m_iCellH, m_iCellW, m_iCellH, m_hMainParent);
        m_hCells[3] = new cObjectQuadTree(owner, x, y + m_iCellH, m_iCellW, m_iCellH, m_hMainParent);
    }
}

cObjectQuadTree::~cObjectQuadTree() {
    if (!m_bIsContainer) {
        for (auto & m_hCell : m_hCells)
            delete m_hCell;
    }
}

WWD::Object *cObjectQuadTree::GetObjectByWorldPosition(int x, int y) {
    if (m_bIsContainer) {
        if (x < m_iX ||
            x > m_iX + m_iCellW ||
            y < m_iY ||
            y > m_iY + m_iCellH)
            return NULL;

        for (auto & m_vObject : m_vObjects) {
            WWD::Rect box = GetBank()->GetObjectRenderRect(m_vObject);
            if (x > box.x1 &&
                x < box.x1 + box.x2 &&
                y > box.y1 &&
                y < box.y1 + box.y2)
                return m_vObject;
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
            return m_hCells[0]->GetObjectByWorldPosition(x, y);
        else if (lx > m_iCellW && ly < m_iCellH)
            return m_hCells[1]->GetObjectByWorldPosition(x, y);
        else if (lx > m_iCellW && ly > m_iCellH)
            return m_hCells[2]->GetObjectByWorldPosition(x, y);
        else if (lx < m_iCellW && ly > m_iCellH)
            return m_hCells[3]->GetObjectByWorldPosition(x, y);
    }
}

bool cObjectQuadTree::RectsCollideOrOverlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    bool collide = false, overlap = false;
    if ((x1 > x2 && x1 < x2 + w2 && y1 > y2 && y1 < y2 + h2) ||
        (x1 + w1 > x2 && x1 + w1 < x2 + w2 && y1 > y2 && y1 < y2 + h2) ||
        (x1 + w1 > x2 && x1 + w1 < x2 + w2 && y1 + h1 > y2 && y1 + h1 < y2 + h2) ||
        (x1 > x2 && x1 < x2 + w2 && y1 + h1 > y2 && y1 + h1 < y2 + h2))
        collide = true;

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
        overlap = true;

    return overlap || collide;
}

std::vector<WWD::Object *> cObjectQuadTree::GetObjectsByArea(int x, int y, int w, int h) {
    if (m_bIsContainer) {
        /*if( x < m_iX ||
            x > m_iX+m_iCellW ||
            y < m_iY ||
            y > m_iY+m_iCellH ) return NULL;*/
        std::vector<WWD::Object *> ret;

        for (auto & m_vObject : m_vObjects) {
            WWD::Rect box = GetBank()->GetObjectRenderRect(m_vObject);

            if (RectsCollideOrOverlap(x, y, w, h, box.x1, box.y1, box.x2, box.y2))
                ret.push_back(m_vObject);
        }
        return ret;
    } else {
        std::vector<WWD::Object *> ret[3], mainret;
        if (RectsCollideOrOverlap(x, y, w, h, m_iX, m_iY, m_iCellW, m_iCellH))
            mainret = m_hCells[0]->GetObjectsByArea(x, y, w, h);
        if (RectsCollideOrOverlap(x, y, w, h, m_iX + m_iCellW, m_iY, m_iCellW, m_iCellH))
            ret[0] = m_hCells[1]->GetObjectsByArea(x, y, w, h);
        if (RectsCollideOrOverlap(x, y, w, h, m_iX + m_iCellW, m_iY + m_iCellH, m_iCellW, m_iCellH))
            ret[1] = m_hCells[2]->GetObjectsByArea(x, y, w, h);
        if (RectsCollideOrOverlap(x, y, w, h, m_iX, m_iY + m_iCellH, m_iCellW, m_iCellH))
            ret[2] = m_hCells[3]->GetObjectsByArea(x, y, w, h);
        for (auto & z : ret) {
            for (int i = 0; i < z.size(); i++) {
                mainret.push_back(z[i]);
            }
        }
        for (int a = 0; a < mainret.size(); a++) {
            for (int b = 0; b < mainret.size(); b++) {
                if (mainret[a] == mainret[b] && a != b) {
                    mainret.erase(mainret.begin() + b);
                    if (a >= b) a--;
                    b--;
                }
            }
        }
        return mainret;
    }
}

int cObjectQuadTree::GetContainerCellWidth() {
    if (m_hMainParent == NULL)
        return iTypicalCellW;
    return m_hMainParent->GetContainerCellWidth();
}

int cObjectQuadTree::GetContainerCellHeight() {
    if (m_hMainParent == NULL)
        return iTypicalCellH;
    return m_hMainParent->GetContainerCellHeight();
}
