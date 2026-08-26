#include "../editing_ww.h"
#include "../../cObjectUserData.h"

int State::EditingWW::GetTileClipboardSize() {
    if (iCurTileCbE == CLIPBOARD_IS_EMPTY)
        return 0;
    for (int i = 0; i < TILE_CLIPBOARD_CAPACITY; i++) {
        if (arTileClipboard[i] == NULL)
            return i;
    }
    return TILE_CLIPBOARD_CAPACITY;
}

void State::EditingWW::CopyTiles() {
    auto* plane = GetActivePlane();

    if (iTileSelectX2 >= plane->GetPlaneWidth() && !plane->GetFlag(WWD::Flag_p_XWrapping)) {
        iTileSelectX2 = plane->GetPlaneWidth() - 1;
    }
    if (iTileSelectY2 >= plane->GetPlaneHeight() && !plane->GetFlag(WWD::Flag_p_YWrapping)) {
        iTileSelectY2 = plane->GetPlaneHeight() - 1;
    }

    int w = std::abs(iTileSelectX2 - iTileSelectX1) + 1;
    int h = std::abs(iTileSelectY2 - iTileSelectY1) + 1;

    int last = TILE_CLIPBOARD_CAPACITY - 1;

    if (arTileClipboard[last] != NULL) {
        delete arTileClipboard[last];
    }

    for (int i = last-1; i >= 0; i--)
        arTileClipboard[i+1] = arTileClipboard[i];
 
    iCurTileCbE = 0;

    arTileClipboard[0] = new cTileClipboardEntry(w, h, plane->GetImageSet(0));

    for (int i = 0, y = iTileSelectY1; y <= iTileSelectY2; y++)
        for (int x = iTileSelectX1; x <= iTileSelectX2; x++, i++)
            arTileClipboard[0]->tiles[i] = *plane->GetTile(x, y);
}

void State::EditingWW::CutTiles() {
    CopyTiles();
    auto* plane = GetActivePlane();
    UndoBegin("Cut Tiles");
    UndoSnapshotTiles(plane, iTileSelectX1, iTileSelectY1, iTileSelectX2, iTileSelectY2);
    bool bChanges = false;
    for (int x = iTileSelectX1; x <= iTileSelectX2; x++)
        for (int y = iTileSelectY1; y <= iTileSelectY2; y++) {
            WWD::Tile *tile = plane->GetTile(x, y);
            if (!tile->IsInvisible()) {
                bChanges = true;
                tile->SetInvisible();
            }
        }
    if (bChanges) {
        vPort->MarkToRedraw();
        MarkUnsaved();
    }
    UndoEnd();
}

void State::EditingWW::PasteTiles() {
    if (iCurTileCbE == CLIPBOARD_IS_EMPTY)
        return;
    bool bChanges = false;
    auto* plane = GetActivePlane();
    int tx = Scr2WrdX(plane, contextX) / plane->GetTileWidth(),
        ty = Scr2WrdY(plane, contextY) / plane->GetTileHeight();

    auto cbEntry = arTileClipboard[iCurTileCbE];

    if (cbEntry == NULL) {
        if (iCurTileCbE == 0)
            iCurTileCbE = CLIPBOARD_IS_EMPTY;
        else
            iCurTileCbE = 0;
        return;
    }

    UndoBegin("Paste Tiles");
    UndoSnapshotTiles(plane, tx, ty, tx + cbEntry->width - 1, ty + cbEntry->height - 1);

    for (int i = 0, y = ty; y < ty + cbEntry->height; ++y) {
        for (int x = tx; x < tx + cbEntry->width; ++x, ++i) {
            WWD::Tile *tile = plane->GetTile(x, y);
            if (tile && *tile != cbEntry->tiles[i]) {
                bChanges = true;
                *tile = cbEntry->tiles[i];
            }
        }
    }
    if (bChanges) {
        vPort->MarkToRedraw();
        MarkUnsaved();
    }
    UndoEnd();
}

int State::EditingWW::GetObjClipboardSize() {
    if (iCurObjCbE == CLIPBOARD_IS_EMPTY)
        return 0;
    for (int i = 0; i < OBJ_CLIPBOARD_CAPACITY; i++) {
        if (arvObjectClipboard[i] == NULL)
            return i;
    }
    return OBJ_CLIPBOARD_CAPACITY;
}

void State::EditingWW::CopyObjects() {
    if (vObjectsPicked.empty())
        return;
    int last = OBJ_CLIPBOARD_CAPACITY - 1;

    if (arvObjectClipboard[last] != NULL) {
        for (auto object : *(arvObjectClipboard[last])) {
            delete object;
        }
        delete arvObjectClipboard[last];
    }

    for (int i = last-1; i >= 0; i--)
        arvObjectClipboard[i+1] = arvObjectClipboard[i];
 
    iCurObjCbE = 0;
    
    arvObjectClipboard[0] = new std::vector<WWD::Object*>;
    arvObjectClipboard[0]->reserve(100); //arbitrary value
    for (auto object : vObjectsPicked) {
        if (object != hStartingPosObj) {
            arvObjectClipboard[0]->push_back(new WWD::Object(object));
        }
    }
}

void State::EditingWW::CutObjects() {
    CopyObjects();
    auto plane = GetActivePlane();
    UndoBegin("Cut Objects");
    // Filter out starting position object
    std::vector<WWD::Object*> toDelete;
    for (auto &obj : vObjectsPicked) {
        if (obj != hStartingPosObj)
            toDelete.push_back(obj);
    }
    UndoSnapshotObjects(plane, toDelete, cUndoManager::Snap_ObjectsDeleted);
    for (auto &obj : toDelete) {
        plane->DeleteObject(obj);
    }
    vObjectsPicked.clear();
    vPort->MarkToRedraw();
    MarkUnsaved();
    UndoEnd();
}

void State::EditingWW::PasteObjects() {
    if (iCurObjCbE == CLIPBOARD_IS_EMPTY)
        return;
    auto *cbEntry = arvObjectClipboard[iCurObjCbE];
    if (cbEntry == NULL) {
        iCurObjCbE = 0;
        return;
    }
    vObjectsPicked.clear();
    float x = int(fCamX * (GetActivePlane()->GetMoveModX() / 100.0f) * fZoom) + contextX - vPort->GetX();
    float y = int(fCamY * (GetActivePlane()->GetMoveModY() / 100.0f) * fZoom) + contextY - vPort->GetY();

    x = x / fZoom;
    y = y / fZoom;

    for (const auto &clipboardObject : *cbEntry) {
        float diffX = clipboardObject->GetX() - (*cbEntry)[0]->GetX();
        float diffY = clipboardObject->GetY() - (*cbEntry)[0]->GetY();

        auto *object = new WWD::Object(clipboardObject);
        GetActivePlane()->AddObjectAndCalcID(object);
        object->SetUserData(new cObjUserData(object));
        vObjectsPicked.push_back(object);
        GetUserDataFromObj(object)->SetPos(x + diffX, y + diffY);
    }

    vPort->MarkToRedraw();
    if (UpdateMovedObjectWithRects(vObjectsPicked)) {
        UndoBegin("Paste Objects");
        UndoSnapshotObjects(GetActivePlane(), vObjectsPicked, cUndoManager::Snap_ObjectsAdded);
        UndoEnd();
        MarkUnsaved();
        vPort->MarkToRedraw();
    } else {
        std::vector<WWD::Object *> tmp = vObjectsPicked;
        for (auto &i : tmp) {
            GetActivePlane()->DeleteObject(i);
        }
    }
}