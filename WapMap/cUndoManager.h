#ifndef H_C_UNDO_MANAGER
#define H_C_UNDO_MANAGER

#include "../shared/cWWD.h"
#include <vector>
#include <string>
#include <set>

namespace State {
    class EditingWW;
}

class cUndoManager {
public:
    static const int MAX_UNDO_STEPS = 50;

    enum SnapshotKind {
        Snap_Tiles,
        Snap_ObjectsAdded,
        Snap_ObjectsDeleted,
        Snap_ObjectsModified
    };

    cUndoManager();
    ~cUndoManager();

    // --- Capture API (called BEFORE mutations) ---

    // Begin an atomic action. All snapshots until EndAction() become one undo step.
    void BeginAction(const char* description);

    // Snapshot tiles in a plane. Coalesced per-plane within one action:
    // the first call per plane captures the given rect; subsequent calls for
    // the same plane in the same action are ignored.
    // For continuous drawing, pass the full plane rect to cover the entire stroke.
    void SnapshotTiles(WWD::Plane* plane, int x1, int y1, int x2, int y2);

    // Snapshot objects (deep copies via WWD::Object copy ctor).
    void SnapshotObjects(WWD::Plane* plane,
                         const std::vector<WWD::Object*>& objs,
                         SnapshotKind kind);

    // Convenience: snapshot a single object.
    void SnapshotObject(WWD::Plane* plane, WWD::Object* obj, SnapshotKind kind);

    // Commit the action. Pushes onto undo stack, clears redo stack.
    void EndAction();

    // Discard pending action (e.g., right-click cancel).
    void CancelAction();

    bool IsInAction() const { return m_inAction; }

    // --- Replay API ---
    bool CanUndo() const;
    bool CanRedo() const;
    const char* GetUndoDescription() const;
    const char* GetRedoDescription() const;
    void Undo(State::EditingWW* editor);
    void Redo(State::EditingWW* editor);

    void Clear();

private:
    struct Snapshot {
        SnapshotKind kind;
        int planeIndex;
        // For Tiles:
        int x1, y1, x2, y2;
        std::vector<WWD::Tile> tiles;
        // For Objects:
        std::vector<WWD::Object*> objects;
        std::vector<int> objectIDs;

        Snapshot() : kind(Snap_Tiles), planeIndex(-1), x1(0), y1(0), x2(0), y2(0) {}
        ~Snapshot();
    };

    struct Action {
        std::string description;
        std::vector<Snapshot> snapshots;
    };

    std::vector<Action> m_undoStack;
    std::vector<Action> m_redoStack;
    Action m_pending;
    bool m_inAction;
    // Track which planes have had tiles snapped in the current action (for coalescing)
    std::set<int> m_snappedTilePlanes;

    int FindPlaneIndex(WWD::Plane* plane);
    void ApplyUndoSnapshot(Snapshot& snap, State::EditingWW* editor, Action& redoAction);
    void RestoreObjectState(WWD::Object* dst, WWD::Object* src);
    void FreeAction(Action& action);
    void RebuildQuadtreeForPlane(State::EditingWW* editor, int planeIndex);
};

#endif
