#ifndef H_C_PHYSICS
#define H_C_PHYSICS

#include "../shared/cWWD.h"

#define PHYS_GRAVITY 1250.0f //px/s2

class cBodyQuadTree;

class cPhysicBody;

enum enBodyType {
    BODY_TRIGGER = 0,
    BODY_DYNAMIC,
    BODY_TILE,
    BODY_PLAYER,
    BODY_ELEVATOR,
    BODY_TREASURE
};

class cPhysicWorld {
private:
    std::vector<cPhysicBody *> vhActiveBodies;
    cBodyQuadTree *hQT;
    WWD::Plane *hPl;

    friend class cPhysicBody;

    void RegisterBody(cPhysicBody *body);

    void NotifyDeletion(cPhysicBody *body);

    void BodyWakeUpNotify(cPhysicBody *body);

public:
    cPhysicWorld(WWD::Plane *hpPl);

    ~cPhysicWorld();

    void CompileTiles();

    void Update();

    cBodyQuadTree *GetTree() { return hQT; };
};

class cPhysicBody {
private:
    float fX, fY, fW, fH;
    cPhysicWorld *hOwner;
    std::vector<cBodyQuadTree *> m_vhCells;
    WWD::TILE_ATRIB iAtrib;
    bool bSleep;
    WWD::Object *hObj;
    bool bGlueObject;
    int iObjOffX, iObjOffY;
    float fSpeedX, fSpeedY;
    bool bFly;
    std::vector<cPhysicBody *> vhCols;
    bool bEnableGravity;
    enBodyType iType;
    void *hUD;

    void (*colCB)(void *hUserData, cPhysicBody *trigger, cPhysicBody *body);

    bool bHidden;
    bool bNoSleep;
public:
    cPhysicBody(cPhysicWorld *w, float x1, float y1, float width, float height,
                WWD::TILE_ATRIB atr = WWD::Atrib_Clear); //static
    cPhysicBody(cPhysicWorld *w, float x1, float y1, float width, float height, WWD::Object *obj,
                enBodyType it); //dynamic
    cPhysicBody(cPhysicWorld *w, float x1, float y1, float width, float height,
                void (*pcolCB)(void *hUserData, cPhysicBody *trigger, cPhysicBody *body), void *phUD,
                WWD::Object *obj = NULL); //trigger
    ~cPhysicBody();

    void SetX(float x);

    void SetY(float y);

    void SetPos(float x, float y);

    float GetX() { return fX; };

    float GetY() { return fY; };

    float GetWidth() { return fW; };

    float GetHeight() { return fH; };

    int GetCollisionsCount() { return vhCols.size(); };

    cPhysicBody *GetCollisionBody(int i) {
        if (i >= 0 && i < vhCols.size()) return vhCols[i];
        return NULL;
    };

    void ReferenceCell(cBodyQuadTree *ptr) { m_vhCells.push_back(ptr); };

    void ClearCellReferences();

    WWD::TILE_ATRIB GetAtrib() { return iAtrib; };

    bool IsSleeping() { return bSleep; };

    void WakeUp();

    void Sleep() { if (!bNoSleep) bSleep = 1; };

    void GlueObject(bool g) { bGlueObject = g; };

    void Update();

    void BanSleeping(bool b) { bNoSleep = b; };

    bool SleepBanned() { return bNoSleep; };

    void SetObjectOffset(int x, int y) {
        iObjOffX = x;
        iObjOffY = y;
    };

    void SetSpeed(float x, float y) {
        fSpeedX = x;
        fSpeedY = y;
        if (fSpeedX != 0 || fSpeedY != 0) WakeUp();
    };

    void SetSpeedX(float x) {
        fSpeedX = x;
        if (fSpeedX != 0) WakeUp();
    };

    void SetSpeedY(float y) {
        fSpeedY = y;
        if (fSpeedY != 0) WakeUp();
    };

    float GetSpeedX() { return fSpeedX; };

    float GetSpeedY() { return fSpeedY; };

    bool IsInAir() { return bFly; };

    void EnableGravity(bool b) { bEnableGravity = b; };

    enBodyType GetType() { return iType; };

    void Triggered(cPhysicBody *b);

    void Hide(bool b) { bHidden = b; };

    bool IsHidden() { return bHidden; };
};

#endif
