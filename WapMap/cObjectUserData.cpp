#include "cObjectUserData.h"

cObjUserData::cObjUserData(WWD::Object *obj) {
    hObj = obj;
    iPosX = iPosY = iPosZ = iPosI = 0;
    iSpecialData = 0;
    bVisible = 0;
    SyncToObj();
}

void cObjUserData::SetPos(int x, int y) {
    bool bUpdatePos = 0;
    if (iPosX != x || iPosY != y)
        bUpdatePos = 1;
    iPosX = x;
    iPosY = y;

    if (bUpdatePos)
        if (m_vhCells.size() != 0) {
            cObjectQuadTree *cell = m_vhCells[0];
            cell->UpdateObject(hObj);
        }
}

void cObjUserData::SetX(int x) {
    if (iPosX == x) return;
    iPosX = x;
    if (m_vhCells.size() != 0) {
        cObjectQuadTree *cell = m_vhCells[0];
        cell->UpdateObject(hObj);
    }
}

void cObjUserData::SetY(int y) {
    if (iPosY == y) return;
    iPosY = y;
    if (m_vhCells.size() != 0) {
        cObjectQuadTree *cell = m_vhCells[0];
        cell->UpdateObject(hObj);
    }
}


void cObjUserData::SetI(int i) {
    bool bUpdatePos = 0;
    if (iPosI != i)
        bUpdatePos = 1;
    iPosI = i;

    if (bUpdatePos)
        if (m_vhCells.size() != 0) {
            cObjectQuadTree *cell = m_vhCells[0];
            cell->UpdateObject(hObj);
        }
}

void cObjUserData::SyncToObj() {
    bool bUpdatePos = 0;
    if (iPosX != hObj->GetParam(WWD::Param_LocationX))
        bUpdatePos = 1;
    iPosX = hObj->GetParam(WWD::Param_LocationX);

    if (iPosY != hObj->GetParam(WWD::Param_LocationY))
        bUpdatePos = 1;
    iPosY = hObj->GetParam(WWD::Param_LocationY);

    if (iPosZ != hObj->GetParam(WWD::Param_LocationZ))
        bUpdatePos = 1;
    iPosZ = hObj->GetParam(WWD::Param_LocationZ);

    if (iPosI != hObj->GetParam(WWD::Param_LocationI))
        bUpdatePos = 1;
    iPosI = hObj->GetParam(WWD::Param_LocationI);

    bMirror = hObj->GetDrawFlags() & WWD::Flag_dr_Mirror;
    bInvert = hObj->GetDrawFlags() & WWD::Flag_dr_Invert;
    bVisible = !(hObj->GetDrawFlags() & WWD::Flag_dr_NoDraw);

    if (bUpdatePos) {
        if (m_vhCells.size() != 0) {
            cObjectQuadTree *cell = m_vhCells[0];
            cell->UpdateObject(hObj);
        }
    }
}

void cObjUserData::ClearCellReferences() {
    for (int i = 0; i < m_vhCells.size(); i++)
        m_vhCells[i]->DeleteObject(hObj);
    m_vhCells.clear();
}
