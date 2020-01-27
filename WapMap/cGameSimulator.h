#ifndef H_C_GAMESIM
#define H_C_GAMESIM

#include "../shared/cWWD.h"
#include "../shared/cANI.h"
#include "cAnimBank.h"
#include "cPhysics.h"
#include <map>

#include "version.h"
//#include "states/editing_ww.h"

#define SIM_ROPE_DEFAULT_SPEED 3.000f //in seconds
#define SIM_TOGGLEPEG_DEFAULT_ON 1.500f //in secs
#define SIM_TOGGLEPEG_DEFAULT_OFF 1.500f //in secs
#define SIM_TOGGLEPEG_ADDITIONAL_DELAY 0.750f //in secs
#define SIM_EYECANDY_DELAY 0.100f //in seconds
#define SIM_ELEVATOR_DEFAULT_SPEED 125 //in pixels per second
//rope speedx = speed
//     speed = offset
//togglepeg speedx = peg on time
//          speedy = peg off time
//          speed = offset time
//          smarts = don't toggle
//togglepeg2 750 delay

namespace State {
    class EditingWW;
}

namespace GameSim {
    enum eSimEntity {
        SE_Unknown = 0,
        SE_Elevator,
        SE_Treasure,
        SE_Crumble,
        SE_PowderKeg,
        SE_Checkpoint,
        SE_Enemy,
        SE_Warp
    };

    struct stObjUserDataSimEntity {
        eSimEntity enType;
        int *hPtr;
    };

    enum eElevatorType {
        ELEVATOR_NORMAL,
        ELEVATOR_START,
        ELEVATOR_TRIGGER,
        ELEVATOR_PATH
    };
    struct cRope {
        float fFrameDuration, fCounter;
        unsigned short iFrame;
        WWD::Object *hObj;
    };
    struct cCrumblingPeg {
        float fAnimTimer;
        bool bRun;
        WWD::Object *hObj;
    };
    struct cPowderKeg {
        float fAnimTimer;
        bool bExplode;
        WWD::Object *hObj;
    };
    struct cWarp {
        WWD::Object *hObj;
    };
    struct cTogglePeg {
        float fDelay;
        float fTimeOn, fTimeOff;
        float fCounter, fFrameCounter;
        float fFrameDuration;
        unsigned short iFramesCount;
        unsigned short iFrame;
        WWD::Object *hObj;
        bool bOn;
#ifdef REAL_SIM
        cPhysicBody * bodyMe;
#endif
    };
    struct cEyeCandy {
        float fCounter;
        unsigned short iFrame;
        WWD::Object *hObj;
    };
    struct cElevator {
        float fPosX, fPosY;
        float fSpeedVer, fSpeedHor;
        int iMinX, iMinY, iMaxX, iMaxY;
        bool bDirVer, bDirHor;
        WWD::Object *hObj;
        eElevatorType enType;
        bool bSleep;
        bool bOneWay;
#ifdef REAL_SIM
        cPhysicBody * bodyMe;
#endif
    };
    struct cPathElevator {
        WWD::Object *hObj;
        float fPosX, fPosY;
        float fSpeed;
        std::vector<std::pair<int, int> > vPaths;
        float fOrigX, fOrigY;
        int iDirHor, iDirVer;
        int iStep;
        float fSleep;
    };
    struct cCheckpoint {
        bool bWave;
        int iFrame;
        float fTimer;
        WWD::Object *hObj;
    };
#ifdef REAL_SIM
    struct cCheckpoint
    {
        bool bWave;
        int iFrame;
        float fTimer;
        cPhysicBody * bodyMe;
        WWD::Object * hObj;
    };
    struct cTreasure
    {
        cPhysicBody * bodyMe;
        WWD::Object * hObj;
    };
#endif
    /*struct cEnemy
    {
        std::vector<ANI::Frame*> vFrames;
        float fTimer;
        float fEndTimer;
        int iFrame;
        bool bDirection;
        bool bInEndTimer;
        WWD::Object * hObj;
    };*/
    struct cEnemy {
        bool bDirection;
        float fMoveSpeed;
        int iMinX, iMaxX;
        float fAniTimer;
        float fWaitTimer;
        int iAniFrame;
        WWD::Object *hObj;
        bool bMoving;
        ANI::Animation *hAni;
        std::vector<std::string> vIdleAnims;
        std::string strWalkAnim;
        bool bStopping;
        bool bNoWalk;
        bool bHit;
        int iHitFrame;
        float fHitTimer;
        ANI::Animation *hAniHit;
    };
    struct cSoundTrigger {
        cPhysicBody *bodyListener[2];
        WWD::Object *hObj;
        bool bDialogTrigger;
    };
    struct cAmbientSound {
        WWD::Object *hObj;
        float fTimer, fTime;
        bool bOn;
        bool bPlaying;
        HCHANNEL hChan;
    };

    struct cProjectileSpawner;

    struct cProjectile {
        WWD::Object *hObj;
        bool bPhase;
        float fAnimTimer;
    };

    struct cProjectileSpawner {
        WWD::Object *hObj;
        std::vector<cProjectile *> vhChildren;
        float fDelayTimer;
        float fDelayTime;
        float fSpeedX, fSpeedY;
        int iDistance;
    };

    enum enAnimMode {
        AniLOOP = 0,
        AniHOLD,
        AniONCE
    };

#define GAMESIM_WEAP_NUM 3
    enum enWeapons {
        WeapPISTOL = 0,
        WeapMAGIC,
        WeapDYNAMITE
    };
}

class cGameSimulator {
private:
    State::EditingWW *mO;
    WWD::Parser *hPar;
    WWD::Plane *hMainPlane;
    std::vector<GameSim::cRope> vRopes;
    std::vector<GameSim::cTogglePeg> vTogglePegs;
    std::vector<GameSim::cEyeCandy> vEyeCandies;
    std::vector<GameSim::cElevator *> vElevators;
    std::vector<GameSim::cPathElevator> vPElevators;
    std::vector<GameSim::cEnemy *> vEnemies;
    std::vector<GameSim::cAmbientSound> vAmbients;
    std::vector<GameSim::cProjectileSpawner> vProjSpawner;
    std::vector<GameSim::cCrumblingPeg *> vCrumbPeg;
    std::vector<GameSim::cPowderKeg *> vPowderKeg;
    std::vector<GameSim::cCheckpoint *> vCheckpoint;
    std::vector<GameSim::cWarp *> vWarp;
    int iRopeFramesCount;
    float fRopeCounter;

    bool bInited;

    void Init();

    cAniBankAsset *hClawAnim, *hClawAnimPlay;
    float fPlayerAniTimer;
    int iPlayerAniFrame, iPlayerAniPlayFrame;
    GameSim::enAnimMode iPlayerAniMode, iPlayerAniPlayMode;
    bool bPlayerAniHold;
    int iLives;
    int iScore;
    int iHealth;
    int iAmmo[GAMESIM_WEAP_NUM];

    float fWeaponTimerHUD;
    int iWeaponFrameHUD;
    float fChestTimer, fHeartTimer;
    int iChestFrame, iHeartFrame;
    GameSim::enWeapons iSelectedWeapon;

    void ChangeWeapon();

    void PlayerPlayAnim(cAniBankAsset *hAni);

    cPhysicWorld *hPW;
    bool bClimbing;
    bool bStopAnim;
    cPhysicBody *hLadder;
public:
    bool bPlay;

    cGameSimulator(State::EditingWW *owner) {
        mO = owner;
        bInited = 0;
    };

    ~cGameSimulator();

    void Think();

    void Render();

    void AlterScore(int mod) { iScore += mod; };

    int GetTileColX(WWD::TileAtrib *atr, bool bRight);

    int GetTileColY(WWD::TileAtrib *atr);

    cPhysicBody *bodyClaw;
    std::vector<GameSim::cSoundTrigger> vSndTrig;
#ifdef REAL_SIM
    std::vector<GameSim::cCheckpoint> vCheckpoints;
    std::vector<GameSim::cTreasure> vTreasures;
#endif
    HCHANNEL clawDialog;

    void PlayAnimSound(ANI::Frame *fr);

    ANI::Animation *aniFlagRise, *aniFlagWave, *aniPowderExpl;
    int iGlitterFrame;
    float fGlitterTimer;

    //springboards
    int iSpringBoardFrame;
    float fSpringBoardTimer;
    ANI::Animation *hSpringBoardAnim;
};

#endif
