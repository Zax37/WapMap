#include "cUndoManager.h"
#include "states/editing_ww.h"
#include "cObjectUserData.h"
#include "globals.h"
#include "cMDI.h"

cUndoManager::cUndoManager() : m_inAction(false) {}

cUndoManager::~cUndoManager() {
    Clear();
}

cUndoManager::Snapshot::~Snapshot() {
    for (auto* obj : objects) {
        delete obj;
    }
    objects.clear();
}

void cUndoManager::FreeAction(Action& action) {
    action.snapshots.clear();
    action.description.clear();
}

void cUndoManager::Clear() {
    for (auto& a : m_undoStack) FreeAction(a);
    for (auto& a : m_redoStack) FreeAction(a);
    m_undoStack.clear();
    m_redoStack.clear();
    if (m_inAction) {
        FreeAction(m_pending);
        m_inAction = false;
    }
    m_snappedTilePlanes.clear();
}

void cUndoManager::BeginAction(const char* description) {
    if (m_inAction) {
        EndAction();
    }
    m_pending.description = description ? description : "Edit";
    m_pending.snapshots.clear();
    m_snappedTilePlanes.clear();
    m_inAction = true;
}

int cUndoManager::FindPlaneIndex(WWD::Plane* plane) {
    if (!GV || !GV->editState || !GV->editState->hParser) return -1;
    for (int i = 0; i < GV->editState->hParser->GetPlanesCount(); i++) {
        if (GV->editState->hParser->GetPlane(i) == plane) return i;
    }
    return -1;
}

void cUndoManager::SnapshotTiles(WWD::Plane* plane, int x1, int y1, int x2, int y2) {
    if (!m_inAction || !plane) return;

    int planeIndex = FindPlaneIndex(plane);
    if (planeIndex < 0) return;

    // Coalesce: if we already snapped tiles for this plane in this action, skip
    if (m_snappedTilePlanes.count(planeIndex)) return;
    m_snappedTilePlanes.insert(planeIndex);

    // Normalize coordinates
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);

    // Clamp to plane bounds
    int pw = plane->GetPlaneWidth();
    int ph = plane->GetPlaneHeight();
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= pw) x2 = pw - 1;
    if (y2 >= ph) y2 = ph - 1;

    if (x1 > x2 || y1 > y2) return;

    Snapshot snap;
    snap.kind = Snap_Tiles;
    snap.planeIndex = planeIndex;
    snap.x1 = x1;
    snap.y1 = y1;
    snap.x2 = x2;
    snap.y2 = y2;

    int w = x2 - x1 + 1;
    int h = y2 - y1 + 1;
    snap.tiles.resize(w * h);
    int idx = 0;
    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++, idx++) {
            WWD::Tile* t = plane->GetTile(x, y);
            if (t) snap.tiles[idx] = *t;
        }
    }

    m_pending.snapshots.push_back(std::move(snap));
}

void cUndoManager::SnapshotObjects(WWD::Plane* plane,
                                    const std::vector<WWD::Object*>& objs,
                                    SnapshotKind kind) {
    if (!m_inAction || !plane || objs.empty()) return;

    int planeIndex = FindPlaneIndex(plane);
    if (planeIndex < 0) return;

    Snapshot snap;
    snap.kind = kind;
    snap.planeIndex = planeIndex;
    for (auto* obj : objs) {
        snap.objects.push_back(new WWD::Object(obj));
        snap.objectIDs.push_back(obj->GetParam(WWD::Param_ID));
    }

    m_pending.snapshots.push_back(std::move(snap));
}

void cUndoManager::SnapshotObject(WWD::Plane* plane, WWD::Object* obj, SnapshotKind kind) {
    std::vector<WWD::Object*> v;
    v.push_back(obj);
    SnapshotObjects(plane, v, kind);
}

void cUndoManager::EndAction() {
    if (!m_inAction) return;
    m_inAction = false;
    m_snappedTilePlanes.clear();

    if (m_pending.snapshots.empty()) {
        m_pending.description.clear();
        return;
    }

    m_undoStack.push_back(std::move(m_pending));
    m_pending = Action();

    // Clear redo stack (new action invalidates redo history)
    for (auto& a : m_redoStack) FreeAction(a);
    m_redoStack.clear();

    // Cap undo stack size
    while ((int)m_undoStack.size() > MAX_UNDO_STEPS) {
        FreeAction(m_undoStack.front());
        m_undoStack.erase(m_undoStack.begin());
    }
}

void cUndoManager::CancelAction() {
    if (!m_inAction) return;
    m_inAction = false;
    m_snappedTilePlanes.clear();
    FreeAction(m_pending);
    m_pending = Action();
}

bool cUndoManager::CanUndo() const {
    return !m_undoStack.empty();
}

bool cUndoManager::CanRedo() const {
    return !m_redoStack.empty();
}

const char* cUndoManager::GetUndoDescription() const {
    if (m_undoStack.empty()) return "";
    return m_undoStack.back().description.c_str();
}

const char* cUndoManager::GetRedoDescription() const {
    if (m_redoStack.empty()) return "";
    return m_redoStack.back().description.c_str();
}

void cUndoManager::RestoreObjectState(WWD::Object* dst, WWD::Object* src) {
    for (int i = 0; i < OBJ_PARAMS_CNT; i++)
        dst->SetParam((WWD::OBJ_PARAMS)i, src->GetParam((WWD::OBJ_PARAMS)i));
    for (int i = 0; i < 8; i++)
        dst->SetUserValue(i, src->GetUserValue(i));
    dst->SetName(src->GetName());
    dst->SetLogic(src->GetLogic());
    dst->SetImageSet(src->GetImageSet());
    dst->SetAnim(src->GetAnim());
    dst->SetMoveRect(src->GetMoveRect());
    dst->SetHitRect(src->GetHitRect());
    dst->SetAttackRect(src->GetAttackRect());
    dst->SetClipRect(src->GetClipRect());
    dst->SetUserRect(0, src->GetUserRect(0));
    dst->SetUserRect(1, src->GetUserRect(1));
    dst->SetAddFlags(src->GetAddFlags());
    dst->SetDynamicFlags(src->GetDynamicFlags());
    dst->SetDrawFlags(src->GetDrawFlags());
    dst->SetUserFlags(src->GetUserFlags());
    dst->SetTypeFlags(src->GetTypeFlags());
    dst->SetHitTypeFlags(src->GetHitTypeFlags());
}

void cUndoManager::RebuildQuadtreeForPlane(State::EditingWW* editor, int planeIndex) {
    if (!editor || planeIndex < 0 || planeIndex >= (int)editor->hPlaneData.size()) return;
    auto* pd = editor->hPlaneData[planeIndex];
    if (!pd) return;

    // Clear cell references for all objects before rebuilding the quadtree.
    // When the old quadtree is deleted, its cells become invalid. We must clear
    // the old references before building the new quadtree, otherwise objects
    // will have both invalid (old) and valid (new) cell references.
    WWD::Plane* plane = editor->hParser->GetPlane(planeIndex);
    if (plane) {
        for (int i = 0; i < plane->GetObjectsCount(); i++) {
            WWD::Object* obj = plane->GetObjectByIterator(i);
            if (obj && obj->GetUserData()) {
                cObjUserData* ud = (cObjUserData*)obj->GetUserData();
                ud->ClearCellReferences();
            }
        }
    }

    if (pd->ObjectData.hQuadTree) {
        delete pd->ObjectData.hQuadTree;
    }
    pd->ObjectData.hQuadTree = new cObjectQuadTree(plane, editor->SprBank);
}

void cUndoManager::ApplyUndoSnapshot(Snapshot& snap, State::EditingWW* editor, Action& redoAction) {
    WWD::Plane* plane = editor->hParser->GetPlane(snap.planeIndex);
    if (!plane) return;

    switch (snap.kind) {
    case Snap_Tiles: {
        Snapshot redoSnap;
        redoSnap.kind = Snap_Tiles;
        redoSnap.planeIndex = snap.planeIndex;
        redoSnap.x1 = snap.x1;
        redoSnap.y1 = snap.y1;
        redoSnap.x2 = snap.x2;
        redoSnap.y2 = snap.y2;

        int w = snap.x2 - snap.x1 + 1;
        int h = snap.y2 - snap.y1 + 1;
        redoSnap.tiles.resize(w * h);

        int idx = 0;
        for (int y = snap.y1; y <= snap.y2; y++) {
            for (int x = snap.x1; x <= snap.x2; x++, idx++) {
                WWD::Tile* cur = plane->GetTile(x, y);
                if (cur) {
                    redoSnap.tiles[idx] = *cur;
                    *cur = snap.tiles[idx];
                }
            }
        }

        if (snap.planeIndex >= 0 && snap.planeIndex < (int)editor->hPlaneData.size()) {
            editor->hPlaneData[snap.planeIndex]->bUpdateBuffer = true;
        }

        redoAction.snapshots.push_back(std::move(redoSnap));
        break;
    }
    case Snap_ObjectsAdded: {
        // Undo an add = delete the objects. Snapshot current state for redo.
        Snapshot redoSnap;
        redoSnap.kind = Snap_ObjectsDeleted;  // Redo will re-add these objects
        redoSnap.planeIndex = snap.planeIndex;

        for (WWD::Object* savedObj : snap.objects) {
            int objID = savedObj->GetParam(WWD::Param_ID);
            WWD::Object* live = plane->GetObjectByObjectID(objID);
            if (live) {
                redoSnap.objects.push_back(new WWD::Object(live));
                redoSnap.objectIDs.push_back(objID);
                plane->DeleteObject(live);
            }
        }

        redoAction.snapshots.push_back(std::move(redoSnap));
        break;
    }
    case Snap_ObjectsDeleted: {
        // Undo a delete = re-add the objects. Snapshot for redo.
        Snapshot redoSnap;
        redoSnap.kind = Snap_ObjectsAdded;  // Redo will delete these objects
        redoSnap.planeIndex = snap.planeIndex;

        for (WWD::Object* savedObj : snap.objects) {
            WWD::Object* restored = new WWD::Object(savedObj);
            plane->AddObjectAndCalcID(restored);
            restored->SetUserData(new cObjUserData(restored));
            redoSnap.objects.push_back(new WWD::Object(restored));
            redoSnap.objectIDs.push_back(restored->GetParam(WWD::Param_ID));
        }

        redoAction.snapshots.push_back(std::move(redoSnap));
        break;
    }
    case Snap_ObjectsModified: {
        // Undo a modify = restore old params. Snapshot current for redo.
        Snapshot redoSnap;
        redoSnap.kind = Snap_ObjectsModified;
        redoSnap.planeIndex = snap.planeIndex;

        for (size_t i = 0; i < snap.objects.size(); i++) {
            WWD::Object* savedObj = snap.objects[i];
            int objID = snap.objectIDs[i];
            WWD::Object* live = plane->GetObjectByObjectID(objID);
            if (live) {
                redoSnap.objects.push_back(new WWD::Object(live));
                redoSnap.objectIDs.push_back(objID);

                RestoreObjectState(live, savedObj);

                cObjUserData* ud = (cObjUserData*)live->GetUserData();
                if (ud) ud->SyncToObj();
            }
        }

        redoAction.snapshots.push_back(std::move(redoSnap));
        break;
    }
    }
}

void cUndoManager::Undo(State::EditingWW* editor) {
    if (m_undoStack.empty() || !editor || !editor->hParser) return;

    if (m_inAction) CancelAction();

    Action action = std::move(m_undoStack.back());
    m_undoStack.pop_back();

    Action redoAction;
    redoAction.description = action.description;

    // Apply snapshots in reverse order
    for (auto it = action.snapshots.rbegin(); it != action.snapshots.rend(); ++it) {
        ApplyUndoSnapshot(*it, editor, redoAction);
    }

    m_redoStack.push_back(std::move(redoAction));

    // Rebuild quadtree for planes that had object changes
    std::set<int> affectedPlanes;
    for (auto& snap : action.snapshots) {
        if (snap.kind != Snap_Tiles) {
            affectedPlanes.insert(snap.planeIndex);
        }
    }
    for (int pi : affectedPlanes) {
        RebuildQuadtreeForPlane(editor, pi);
    }

    editor->vPort->MarkToRedraw();
}

void cUndoManager::Redo(State::EditingWW* editor) {
    if (m_redoStack.empty() || !editor || !editor->hParser) return;

    if (m_inAction) CancelAction();

    Action action = std::move(m_redoStack.back());
    m_redoStack.pop_back();

    Action undoAction;
    undoAction.description = action.description;

    for (auto it = action.snapshots.rbegin(); it != action.snapshots.rend(); ++it) {
        ApplyUndoSnapshot(*it, editor, undoAction);
    }

    m_undoStack.push_back(std::move(undoAction));

    std::set<int> affectedPlanes;
    for (auto& snap : action.snapshots) {
        if (snap.kind != Snap_Tiles) {
            affectedPlanes.insert(snap.planeIndex);
        }
    }
    for (int pi : affectedPlanes) {
        RebuildQuadtreeForPlane(editor, pi);
    }

    editor->vPort->MarkToRedraw();
}
