#include "cObjectUserData.h"

cObjUserData::cObjUserData(WWD::Object *obj) {
    hObj = obj;
    iPosX = iPosY = iPosZ = iPosI = 0;
    iSpecialData = 0;
    bVisible = false;
    SyncToObj();
}

void cObjUserData::SetPos(int x, int y) {
    bool bUpdatePos = false;
    if (iPosX != x || iPosY != y)
        bUpdatePos = true;
    iPosX = x;
    iPosY = y;

    if (bUpdatePos)
        if (!m_vhCells.empty()) {
            cObjectQuadTree *cell = m_vhCells[0];
            cell->UpdateObject(hObj);
        }
}

void cObjUserData::SetX(int x) {
    if (iPosX == x) return;
    iPosX = x;
    if (!m_vhCells.empty()) {
        cObjectQuadTree *cell = m_vhCells[0];
        cell->UpdateObject(hObj);
    }
}

void cObjUserData::SetY(int y) {
    if (iPosY == y) return;
    iPosY = y;
    if (!m_vhCells.empty()) {
        cObjectQuadTree *cell = m_vhCells[0];
        cell->UpdateObject(hObj);
    }
}


void cObjUserData::SetI(int i) {
    bool bUpdatePos = false;
    if (iPosI != i)
        bUpdatePos = true;
    iPosI = i;

    if (bUpdatePos)
        if (!m_vhCells.empty()) {
            cObjectQuadTree *cell = m_vhCells[0];
            cell->UpdateObject(hObj);
        }
}

void cObjUserData::SyncToObj() {
    bool bUpdatePos = false;
    if (iPosX != hObj->GetParam(WWD::Param_LocationX))
        bUpdatePos = true;
    iPosX = hObj->GetParam(WWD::Param_LocationX);

    if (iPosY != hObj->GetParam(WWD::Param_LocationY))
        bUpdatePos = true;
    iPosY = hObj->GetParam(WWD::Param_LocationY);

    if (iPosZ != hObj->GetParam(WWD::Param_LocationZ))
        bUpdatePos = true;
    iPosZ = hObj->GetParam(WWD::Param_LocationZ);

    if (iPosI != hObj->GetParam(WWD::Param_LocationI))
        bUpdatePos = true;
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
    for (auto & m_vhCell : m_vhCells)
        m_vhCell->DeleteObject(hObj);
    m_vhCells.clear();
}
