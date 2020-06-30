#include "cGameSimulator.h"
#include "states/editing_ww.h"
#include "cObjectUserData.h"
#include "../shared/commonFunc.h"
#include "bodyQuadTree.h"
#include "version.h"

extern HGE *hge;
using namespace GameSim;

#ifdef REAL_SIM

static const char * WEAPON_ANIMS[] = {"GAME_INTERFACE_PISTOL",
                                      "GAME_INTERFACE_MAGIC",
                                      "GAME_INTERFACE_DYNAMITE"};
static const char * WEAPON_IMGSET[] = {"GAME_INTERFACE_WEAPONS_PISTOL",
                                       "GAME_INTERFACE_WEAPONS_MAGIC",
                                       "GAME_INTERFACE_WEAPONS_DYNAMITE"};

void CheckpointTrigCb(void * user, cPhysicBody * trigger, cPhysicBody * body)
{
    if( body->GetType() != BODY_PLAYER ) return;
    GV->Console->Printf("Checkpoint #~y~%d~w~ reached.", (int)user);
    delete GV->editState->hSim->vCheckpoints[(int)user].bodyMe;
    GV->editState->hSim->vCheckpoints[(int)user].bodyMe = NULL;
    GV->editState->hSim->vCheckpoints[(int)user].fTimer = 0;
    GV->editState->hSim->PlayAnimSound(GV->editState->hSim->aniFlagRise->GetFrame(0));
}

void TreasureTrigCb(void * user, cPhysicBody * trigger, cPhysicBody * body)
{
    if( body->GetType() != BODY_PLAYER ) return;
    delete GV->editState->hSim->vTreasures[(int)user].bodyMe;
    GetUserDataFromObj(GV->editState->hSim->vTreasures[(int)user].hObj)->SetVisible(0);
    //GV->editState->hSim->vTreasures.erase(GV->editState->hSim->vTreasures.begin()+(int)user);
    GV->editState->hSim->AlterScore(100);
}

void SndTrigCb(void * user, cPhysicBody * trigger, cPhysicBody * body)
{
    if( body != GV->editState->hSim->bodyClaw ) return;

    GV->Console->Printf("Snd. trig. #~y~%d~w~ launched [~y~%s~w~].", (int)user, GV->editState->hSim->vSndTrig[(int)user].hObj->GetAnim());
    cSndBankAsset * as = GV->editState->hSndBank->GetAssetByID(GV->editState->hSim->vSndTrig[(int)user].hObj->GetAnim());
    if( as == NULL )
     GV->Console->Printf("~r~Asset '~y~%s~r~' invalid!", GV->editState->hSim->vSndTrig[(int)user].hObj->GetAnim());
    else{
     if( GV->editState->hSim->vSndTrig[(int)user].bDialogTrigger )
      GV->editState->hSim->clawDialog = hge->Effect_Play(as->GetSound());
     else
      hge->Effect_Play(as->GetSound());
    }
    if( GV->editState->hSim->vSndTrig[(int)user].hObj->GetParam(WWD::Param_Smarts) != -1 ){
     if( trigger == GV->editState->hSim->vSndTrig[(int)user].bodyListener[0] ){
      delete GV->editState->hSim->vSndTrig[(int)user].bodyListener[0];
      GV->editState->hSim->vSndTrig[(int)user].bodyListener[0] = NULL;
     }else{
      delete GV->editState->hSim->vSndTrig[(int)user].bodyListener[1];
      GV->editState->hSim->vSndTrig[(int)user].bodyListener[1] = NULL;
     }
    }
}
#endif

void cGameSimulator::Init() {
    if (bInited) return;

    bPlay = GV->bRealSim;

    bInited = 1;

    if (mO->SprBank->GetAssetByID("LEVEL_ROPE") != NULL)
        iRopeFramesCount = mO->SprBank->GetAssetByID("LEVEL_ROPE")->GetSpritesCount();
    else
        iRopeFramesCount = -1;

    hPar = mO->hParser;

    iGlitterFrame = 0;
    fGlitterTimer = 0;

    iSpringBoardFrame = 0;
    fSpringBoardTimer = 0;
    if (GV->editState->hParser->GetBaseLevel() == 4 || GV->editState->hParser->GetBaseLevel() == 13 ||
        GV->editState->hParser->GetBaseLevel() == 6)
        hSpringBoardAnim = GV->editState->hAniBank->GetAssetByID("LEVEL_SPRINGYFERN_IDLE")->GetAni();
    else if (GV->editState->hParser->GetBaseLevel() == 7 || GV->editState->hParser->GetBaseLevel() == 12)
        hSpringBoardAnim = GV->editState->hAniBank->GetAssetByID("LEVEL_ROCKSPRING_IDLE")->GetAni();
    else if (GV->editState->hParser->GetBaseLevel() == 9)
        hSpringBoardAnim = GV->editState->hAniBank->GetAssetByID("LEVEL_SPRINGBOX_IDLE")->GetAni();
    else
        hSpringBoardAnim = NULL;

#ifdef REAL_SIM
    if( bPlay ){
     GV->editState->bDisableGUI = 1;
     bStopAnim = 0;
     bClimbing = 0;
     hPW = NULL;
     for(int p=0;p<hPar->GetPlanesCount();p++){
      WWD::Plane * pl = hPar->GetPlane(p);
      if( pl->GetObjectsCount() == 0 ) continue;
      hPW = new cPhysicWorld(pl);
      hMainPlane = pl;
      hPW->CompileTiles();
      break;
     }
     bodyClaw = new cPhysicBody(hPW, GetUserDataFromObj(GV->editState->hStartingPosObj)->GetX()-28, GetUserDataFromObj(GV->editState->hStartingPosObj)->GetY()-54,
                          44, 115, GV->editState->hStartingPosObj, BODY_PLAYER);
     bodyClaw->GlueObject(1);
     bodyClaw->SetObjectOffset(5, -5);
     bodyClaw->BanSleeping(1);

     hClawAnimPlay = NULL;
     iPlayerAniPlayFrame = 0;
     iLives = 1;
     iWeaponFrameHUD = 0;
     fWeaponTimerHUD = 0;
     iAmmo[WeapPISTOL] = 10;
     iAmmo[WeapMAGIC] = 5;
     iAmmo[WeapDYNAMITE] = 3;
     iHealth = 100;
     fChestTimer = fHeartTimer = 0;
     iChestFrame = iHeartFrame = 0;
     iSelectedWeapon = WeapPISTOL;
     iScore = 0;
     GV->editState->butsimExit->setVisible(0);
     GV->editState->vPort->SetPos(0, 0);
     GV->editState->vPort->Resize(hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT));
     bPlay = 1;
     hClawAnim = GV->editState->hAniBank->GetAssetByID("CLAW_STAND");
     GetUserDataFromObj(GV->editState->hStartingPosObj)->SetI(GV->editState->SprBank->GetAssetByID("CLAW")->GetIMGByNoID(hClawAnim->GetAni()->GetFrame(0)->GetImageID())->GetIt()+1);
     fPlayerAniTimer = 0;
     iPlayerAniFrame = 0;
     iPlayerAniMode = AniLOOP;
     bPlayerAniHold = 0;
     clawDialog = 0;

    }
#endif

    if (GV->editState->hAniBank->GetAssetByID("LEVEL_POWDERKEG_EXPLODE") != 0)
        aniPowderExpl = GV->editState->hAniBank->GetAssetByID("LEVEL_POWDERKEG_EXPLODE")->GetAni();
    else
        aniPowderExpl = 0;

    aniFlagRise = GV->editState->hAniBank->GetAssetByID("GAME_CHECKPOINT_RISE")->GetAni();
    aniFlagWave = GV->editState->hAniBank->GetAssetByID("GAME_CHECKPOINT_WAVE")->GetAni();

    for (int p = 0; p < hPar->GetPlanesCount(); p++) {
        WWD::Plane *pl = hPar->GetPlane(p);
        if (pl->GetObjectsCount() == 0) continue;
        for (int o = 0; o < pl->GetObjectsCount(); o++) {
            WWD::Object *obj = pl->GetObjectByIterator(o);
            if (!strcmp(obj->GetLogic(), "TProjectile")) {
                cProjectileSpawner n;
                n.hObj = obj;
                n.fDelayTimer = 0;
                n.fDelayTime =
                        obj->GetParam(WWD::Param_Speed) > 0 ? float(obj->GetParam(WWD::Param_Speed)) / 1000.0f : 500;
                n.fSpeedX = obj->GetParam(WWD::Param_SpeedX);
                n.fSpeedY = obj->GetParam(WWD::Param_SpeedY);
                GetUserDataFromObj(obj)->SetVisible(0);
                int dir = obj->GetParam(WWD::Param_Direction);
                if ((dir == 8 || dir == 2) && n.fSpeedY == 0)
                    n.fSpeedY = 100;
                if ((dir == 4 || dir == 6) && n.fSpeedX == 0)
                    n.fSpeedX = 100;

                if (dir == 4)
                    n.fSpeedX *= -1;
                if (dir == 8)
                    n.fSpeedY *= -1;
                if (dir == 8 || dir == 2) n.fSpeedX = 0;
                if (dir == 4 || dir == 6) n.fSpeedY = 0;
                //calculate projectile distance
                n.iDistance = 5;
                int tileW = pl->GetTileWidth(),
                        tileH = pl->GetTileHeight();
                WWD::TileAtrib emptyatr(tileW, tileH, WWD::AtribType_Single, WWD::Atrib_Clear);
                int tileX = obj->GetParam(WWD::Param_LocationX) / tileW, tileY =
                        obj->GetParam(WWD::Param_LocationY) / tileH;
                WWD::Rect testrect = GV->editState->SprBank->GetObjectRenderRect(obj);
                testrect.x2 += testrect.x1;
                testrect.y2 += testrect.y1;
                if (n.fSpeedX != 0) {
                    testrect.x1 = 0;
                    testrect.x2 = pl->GetPlaneWidthPx();
                } else if (n.fSpeedY != 0) {
                    testrect.y1 = 0;
                    testrect.y2 = pl->GetPlaneHeightPx();
                }
                while (1) {
                    if (n.fSpeedX > 0) tileX++;
                    else if (n.fSpeedX < 0) tileX--;
                    if (n.fSpeedY > 0) tileY++;
                    else if (n.fSpeedY < 0) tileY--;
                    n.iDistance += (n.fSpeedX != 0 ? tileW : tileH);
                    if (tileX < 0 || tileY < 0 || tileX >= pl->GetPlaneWidth() || tileY >= pl->GetPlaneHeight()) break;
                    WWD::Tile *t = GV->editState->GetActivePlane()->GetTile(tileX, tileY);
                    if (!t) break;
                    WWD::TileAtrib *atr = NULL;
                    if (t->IsFilled() || t->IsInvisible())
                        atr = &emptyatr;
                    else
                        atr = GV->editState->hParser->GetTileAtribs(t->GetID());
                    if (!atr) break;
                    std::vector<WWD::CollisionRect> rects = atr->GetColRects();
                    int offx = tileX * tileW, offy = tileY * tileH;
                    int col = -1;
                    for (int i = 0; i < rects.size(); i++) {
                        rects[i].WWD_CR_RECT.x1 += offx;
                        rects[i].WWD_CR_RECT.y1 += offy;
                        rects[i].WWD_CR_RECT.x2 += offx;
                        rects[i].WWD_CR_RECT.y2 += offy;
                        if (rects[i].WWD_CR_TYPE != WWD::Atrib_Climb && testrect.Collide(rects[i].WWD_CR_RECT)) {
                            col = i;
                            break;
                        }
                    }
                    if (col != -1) {
                        if (n.fSpeedX != 0) {
                            if (n.fSpeedX > 0) {
                                n.iDistance = abs(rects[col].WWD_CR_RECT.x1 - obj->GetParam(WWD::Param_LocationX));
                            } else {
                                n.iDistance = abs(rects[col].WWD_CR_RECT.x2 - obj->GetParam(WWD::Param_LocationX));
                            }
                        } else {
                            if (n.fSpeedY > 0) {
                                n.iDistance = abs(rects[col].WWD_CR_RECT.y1 - obj->GetParam(WWD::Param_LocationY));
                            } else {
                                n.iDistance = abs(rects[col].WWD_CR_RECT.y2 - obj->GetParam(WWD::Param_LocationY));
                            }
                        }
                        break;
                    }
                }
                if (n.iDistance > 1000)
                    n.iDistance = 1000;
                vProjSpawner.push_back(n);
            }
            if (!strcmp(obj->GetLogic(), "AniRope") && iRopeFramesCount != -1) {
                cRope n;
                n.hObj = obj;
                if (obj->GetParam(WWD::Param_SpeedX) == 0)
                    n.fFrameDuration = float(SIM_ROPE_DEFAULT_SPEED) / float(iRopeFramesCount);
                else
                    n.fFrameDuration =
                            float(float(obj->GetParam(WWD::Param_SpeedX)) / 1000.0f) / float(iRopeFramesCount);
                if (obj->GetParam(WWD::Param_Speed) == 0)
                    n.iFrame = 0;
                else
                    n.iFrame = int((float(obj->GetParam(WWD::Param_Speed)) / 1000.0f) / n.fFrameDuration) %
                               iRopeFramesCount;
                //printf("rope speed %f start %d (speedx %d, speed %d) time %f\n", n.fFrameDuration, n.iFrame, obj->GetParam(WWD::Param_SpeedX), obj->GetParam(WWD::Param_Speed));
                vRopes.push_back(n);
            } else if (!strncmp(obj->GetLogic(), "TogglePeg", 9) ||
                       !strncmp(obj->GetLogic(), "FloorSpike", 10) ||
                       !strncmp(obj->GetLogic(), "SawBlade", 8)) {
                if (obj->GetParam(WWD::Param_Smarts)) continue;
                cTogglePeg n;
                n.hObj = obj;
                n.fFrameDuration = 0.050f;
                n.iFrame = 0;

                if (obj->GetParam(WWD::Param_SpeedX) != 0)
                    n.fTimeOn = float(obj->GetParam(WWD::Param_SpeedX)) / 1000.0f;
                else
                    n.fTimeOn = SIM_TOGGLEPEG_DEFAULT_ON;

                if (obj->GetParam(WWD::Param_SpeedY) != 0)
                    n.fTimeOff = float(obj->GetParam(WWD::Param_SpeedY)) / 1000.0f;
                else
                    n.fTimeOff = SIM_TOGGLEPEG_DEFAULT_OFF;

                int iOffset = 0;
                float fOffReal = float(obj->GetParam(WWD::Param_Speed)) / 1000.0f;

                if (!strncmp(obj->GetLogic(), "TogglePeg", 9) && strlen(obj->GetLogic()) > 9)
                    iOffset = obj->GetLogic()[9] - 49;
                else if (!strncmp(obj->GetLogic(), "FloorSpike", 10) && strlen(obj->GetLogic()) > 10)
                    iOffset = obj->GetLogic()[10] - 49;
                else if (!strncmp(obj->GetLogic(), "SawBlade", 8) && strlen(obj->GetLogic()) > 8)
                    iOffset = obj->GetLogic()[8] - 49;

                if (iOffset >= 1 && iOffset < 4) {
                    fOffReal += float(iOffset) * SIM_TOGGLEPEG_ADDITIONAL_DELAY;
                }

#ifdef REAL_SIM
                if( !strncmp(obj->GetLogic(), "TogglePeg", 8) && bPlay ){
                 n.bodyMe = new cPhysicBody(hPW, obj->GetParam(WWD::Param_LocationX)-22, obj->GetParam(WWD::Param_LocationY)-8, 45, 40, WWD::Atrib_Ground);
                }
#endif

                n.fCounter = 0;
                n.fDelay = fOffReal;

                n.iFramesCount = mO->SprBank->GetAssetByID(obj->GetImageSet())->GetSpritesCount();
                n.bOn = 1;
                n.fFrameCounter = 0;

                vTogglePegs.push_back(n);
            } else if (strstr(obj->GetLogic(), "AmbientSound") != NULL) {
                if (obj->GetHitRect().x1 != 0 || obj->GetHitRect().y1 != 0 || obj->GetHitRect().x2 != 0 ||
                    obj->GetHitRect().y2 != 0 ||
                    obj->GetParam(WWD::Param_MinX) != 0 || obj->GetParam(WWD::Param_MinY) != 0 ||
                    obj->GetParam(WWD::Param_MaxX) != 0 || obj->GetParam(WWD::Param_MaxY) != 0)
                    continue;
                cAmbientSound n;
                n.hObj = obj;
                n.fTimer = 0;
                n.fTime = 0.001;
                n.bOn = 0;
                n.hChan = NULL;
                vAmbients.push_back(n);
            } else if (!strcmp(obj->GetLogic(), "AniCycle") ||
                       !strcmp(obj->GetLogic(), "BehindAniCandy") ||
                       !strcmp(obj->GetImageSet(), "GAME_VERTWARP") ||
                       !strcmp(obj->GetLogic(), "FrontAniCandy") ||
                       !strcmp(obj->GetImageSet(), "GAME_WARP") ||
                       !strncmp(obj->GetImageSet(), "GAME_CURSES", 11) ||
                       !strcmp(obj->GetLogic(), "ConveyorBelt")) {
                if (mO->SprBank->GetAssetByID(obj->GetImageSet()) == NULL) continue;
                cEyeCandy n;
                n.hObj = obj;
                n.fCounter = 0;
                n.iFrame = 0;
                vEyeCandies.push_back(n);
            } else if (!strcmp(obj->GetLogic(), "PathElevator")) {
                cPathElevator n;
                n.hObj = obj;
                if (obj->GetParam(WWD::Param_Speed) != 0)
                    n.fSpeed = obj->GetParam(WWD::Param_Speed);
                else
                    n.fSpeed = SIM_ELEVATOR_DEFAULT_SPEED;
                n.fOrigX = obj->GetParam(WWD::Param_LocationX);
                n.fOrigY = obj->GetParam(WWD::Param_LocationY);
                n.fPosX = n.fOrigX;
                n.fPosY = n.fOrigY;
                if (obj->GetParam(WWD::Param_Powerup) == 1) {
                    n.vPaths.push_back(std::pair<int, int>(6, 100));
                    n.vPaths.push_back(std::pair<int, int>(7, 100));
                    n.vPaths.push_back(std::pair<int, int>(6, 100));
                    n.vPaths.push_back(std::pair<int, int>(1, 100));
                } else if (obj->GetParam(WWD::Param_Powerup) == 2) {
                    n.vPaths.push_back(std::pair<int, int>(6, 100));
                    n.vPaths.push_back(std::pair<int, int>(7, 100));
                    n.vPaths.push_back(std::pair<int, int>(5, 1000));
                    n.vPaths.push_back(std::pair<int, int>(6, 100));
                    n.vPaths.push_back(std::pair<int, int>(1, 100));
                } else {
                    WWD::Rect tmpr = obj->GetMoveRect();
                    if (tmpr.x1 != 0)
                        n.vPaths.push_back(std::pair<int, int>(tmpr.x1, tmpr.y1));
                    if (tmpr.x2 != 0)
                        n.vPaths.push_back(std::pair<int, int>(tmpr.x2, tmpr.y2));
                    tmpr = obj->GetHitRect();
                    if (tmpr.x1 != 0)
                        n.vPaths.push_back(std::pair<int, int>(tmpr.x1, tmpr.y1));
                    if (tmpr.x2 != 0)
                        n.vPaths.push_back(std::pair<int, int>(tmpr.x2, tmpr.y2));
                    tmpr = obj->GetAttackRect();
                    if (tmpr.x1 != 0)
                        n.vPaths.push_back(std::pair<int, int>(tmpr.x1, tmpr.y1));
                    if (tmpr.x2 != 0)
                        n.vPaths.push_back(std::pair<int, int>(tmpr.x2, tmpr.y2));
                    tmpr = obj->GetClipRect();
                    if (tmpr.x1 != 0)
                        n.vPaths.push_back(std::pair<int, int>(tmpr.x1, tmpr.y1));
                    if (tmpr.x2 != 0)
                        n.vPaths.push_back(std::pair<int, int>(tmpr.x2, tmpr.y2));
                }

                n.iStep = 0;

                if (n.vPaths[0].first == 5) {
                    n.iDirHor = n.iDirVer = 0;
                } else {
                    if (n.vPaths[0].first >= 7) n.iDirVer = -1;
                    else if (n.vPaths[0].first >= 4) n.iDirVer = 0;
                    else if (n.vPaths[0].first >= 1) n.iDirVer = 1;

                    if (n.vPaths[0].first == 7 || n.vPaths[0].first == 4 || n.vPaths[0].first == 1) n.iDirHor = -1;
                    else if (n.vPaths[0].first == 8 || n.vPaths[0].first == 2) n.iDirHor = 0;
                    else if (n.vPaths[0].first == 9 || n.vPaths[0].first == 6 || n.vPaths[0].first == 3) n.iDirHor = 1;
                }
                n.fSleep = 0;

                vPElevators.push_back(n);
            } else if (!strcmp(obj->GetLogic(), "Elevator") || !strcmp(obj->GetLogic(), "StartElevator") ||
                       !strcmp(obj->GetLogic(), "TriggerElevator") || !strcmp(obj->GetLogic(), "StopElevator") ||
                       !strcmp(obj->GetLogic(), "OneWayStartElevator") ||
                       !strcmp(obj->GetLogic(), "OneWayTriggerElevator") ||
                       !strcmp(obj->GetLogic(), "StandardElevator")) {
                cElevator *n = new cElevator;
                n->hObj = obj;
                if (strstr(obj->GetLogic(), "Standard") != 0) {
                    n->iMinX = obj->GetParam(WWD::Param_LocationX) - 600;
                    n->iMinY = obj->GetParam(WWD::Param_LocationY) - 600;
                    n->iMaxX = obj->GetParam(WWD::Param_LocationX) + 600;
                    n->iMaxY = obj->GetParam(WWD::Param_LocationX) + 600;
                } else {
                    n->iMinX = obj->GetParam(WWD::Param_MinX);
                    n->iMinY = obj->GetParam(WWD::Param_MinY);
                    n->iMaxX = obj->GetParam(WWD::Param_MaxX);
                    n->iMaxY = obj->GetParam(WWD::Param_MaxY);
                }
                n->bOneWay = (strstr(obj->GetLogic(), "OneWay") != NULL);
                n->bSleep = !strcmp(obj->GetLogic(), "StartElevator") || n->bOneWay;
                n->enType = ELEVATOR_NORMAL;
                if (strstr(obj->GetLogic(), "Start") != 0)
                    n->enType = ELEVATOR_START;
                else if (strstr(obj->GetLogic(), "Trigger") != 0)
                    n->enType = ELEVATOR_TRIGGER;

                if (n->enType != ELEVATOR_NORMAL) {
                    stObjUserDataSimEntity *se = new stObjUserDataSimEntity();
                    se->enType = SE_Elevator;
                    se->hPtr = (int *) n;
                    GetUserDataFromObj(obj)->SetSpecialData((int *) se);
                }

                if (n->iMinX == n->iMaxX && n->iMinY == n->iMaxY) continue;
                int dir = obj->GetParam(WWD::Param_Direction);
                if (n->iMaxX - n->iMinX == n->iMaxY - n->iMaxX) {
                    if (dir == 6 || dir == 2) dir = 3;
                    else if (dir == 4) dir = 1;
                    else if (dir == 8) dir = 9;
                }
                if (dir == 0 || dir == 5 || dir > 9) {
                    if (n->iMaxX == obj->GetParam(WWD::Param_LocationX))
                        n->bDirHor = 0;
                    else
                        n->bDirHor = 1;
                    if (n->iMaxY == obj->GetParam(WWD::Param_LocationY))
                        n->bDirVer = 0;
                    else
                        n->bDirVer = 1;
                } else {
                    n->bDirVer = (dir <= 3);
                    n->bDirHor = (dir == 3 || dir == 6 || dir == 9);
                }
                if (obj->GetParam(WWD::Param_SpeedX) == 0)
                    n->fSpeedHor = SIM_ELEVATOR_DEFAULT_SPEED;
                else
                    n->fSpeedHor = obj->GetParam(WWD::Param_SpeedX);

                if (obj->GetParam(WWD::Param_SpeedY) == 0)
                    n->fSpeedVer = SIM_ELEVATOR_DEFAULT_SPEED;
                else
                    n->fSpeedVer = obj->GetParam(WWD::Param_SpeedY);

                n->fPosX = obj->GetParam(WWD::Param_LocationX);
                n->fPosY = obj->GetParam(WWD::Param_LocationY);

#ifdef REAL_SIM
                if( bPlay ){
                 int x = n->fPosX, y = n->fPosY, w = 63, h = 15;
                 n->bodyMe = new cPhysicBody(hPW, x, y, w, h, obj, BODY_ELEVATOR);
                 n->bodyMe->GlueObject(1);
                 n->bodyMe->EnableGravity(0);
                 n->bodyMe->SetObjectOffset(-5, 1);
                }
#endif

                vElevators.push_back(n);
#ifdef REAL_SIM
                }else if( bPlay && (strstr(obj->GetLogic(), "GlitterlessPowerup") || strstr(obj->GetLogic(), "TreasurePowerup")) ){
                 cTreasure n;
                 n.hObj = obj;
                 hgeSprite * spr = GV->editState->SprBank->GetAssetByID(obj->GetImageSet())->GetIMGByNoID(0)->GetSprite();
                 n.bodyMe = new cPhysicBody(hPW, obj->GetParam(WWD::Param_LocationX)-spr->GetWidth()/2,
                                            obj->GetParam(WWD::Param_LocationY)-spr->GetHeight()/2, spr->GetWidth(), spr->GetHeight(),
                                            TreasureTrigCb, (void*)vTreasures.size(), obj);
                 vTreasures.push_back(n);
                }else if( bPlay && strstr(obj->GetLogic(), "Checkpoint") != NULL ){
                 cCheckpoint n;
                 n.hObj = obj;
                 n.bWave = 0;
                 n.iFrame = 0;
                 n.fTimer = -1;
                 n.bodyMe = new cPhysicBody(hPW, obj->GetParam(WWD::Param_LocationX)-28, obj->GetParam(WWD::Param_LocationY)-48, 25, 112,
                                            CheckpointTrigCb, (void*)vCheckpoints.size(), obj);
                 vCheckpoints.push_back(n);
                }else if( bPlay && strstr(obj->GetLogic(), "SoundTrigger") != NULL ){
                 cSoundTrigger n;
                 n.hObj = obj;
                 n.bDialogTrigger = !strncmp(obj->GetLogic(), "ClawDialog", 10);
                 int x, y, w, h;

                 if( obj->GetHitRect().x1 != 0 || obj->GetHitRect().y1 != 0 || obj->GetHitRect().x2 != 0 || obj->GetHitRect().y2 != 0 ){
                  n.bodyListener[1] = new cPhysicBody(hPW, obj->GetHitRect().x1, obj->GetHitRect().y1,
                                                      obj->GetHitRect().x2-obj->GetHitRect().x1, obj->GetHitRect().y2-obj->GetHitRect().y1,
                                                      SndTrigCb, (void*)vSndTrig.size(), obj);
                 }else
                  n.bodyListener[1] = NULL;
                 if( strstr(obj->GetLogic(), "Small") != NULL ){
                  x = obj->GetParam(WWD::Param_LocationX)-32;
                  y = obj->GetParam(WWD::Param_LocationY)-32;
                  w = h = 64;
                 }else if( strstr(obj->GetLogic(), "Big") != NULL ){
                  x = obj->GetParam(WWD::Param_LocationX)-128;
                  y = obj->GetParam(WWD::Param_LocationY)-128;
                  w = h = 256;
                 }else if( strstr(obj->GetLogic(), "Huge") != NULL ){
                  x = obj->GetParam(WWD::Param_LocationX)-256;
                  y = obj->GetParam(WWD::Param_LocationY)-256;
                  w = h = 512;
                 }else if( strstr(obj->GetLogic(), "Tall") != NULL ){
                  x = obj->GetParam(WWD::Param_LocationX)-32;
                  y = obj->GetParam(WWD::Param_LocationY)-100;
                  w = 64;
                  h = 200;
                 }else if( strstr(obj->GetLogic(), "Wide") != NULL ){
                  x = obj->GetParam(WWD::Param_LocationX)-100;
                  y = obj->GetParam(WWD::Param_LocationY)-32;
                  w = 200;
                  h = 32;
                 }else if( strstr(obj->GetLogic(), "Tiny") != NULL ){
                  x = obj->GetParam(WWD::Param_LocationX)-16;
                  y = obj->GetParam(WWD::Param_LocationY)-16;
                  w = h = 32;
                 }else{
                  x = obj->GetParam(WWD::Param_LocationX)-64;
                  y = obj->GetParam(WWD::Param_LocationY)-64;
                  w = h = 128;
                 }
                 n.bodyListener[0] = new cPhysicBody(hPW, x, y, w, h, SndTrigCb, (void*)vSndTrig.size(), obj);
                 vSndTrig.push_back(n);
#endif
            } else if (strstr(obj->GetLogic(), "Checkpoint") != NULL) {
                cCheckpoint *n = new cCheckpoint;
                n->hObj = obj;
                n->bWave = 0;
                n->iFrame = 0;
                n->fTimer = -1;
                vCheckpoint.push_back(n);
                stObjUserDataSimEntity *se = new stObjUserDataSimEntity();
                se->enType = SE_Checkpoint;
                se->hPtr = (int *) n;
                GetUserDataFromObj(obj)->SetSpecialData((int *) se);
            } else if (!strcmp(obj->GetLogic(), "CrumblingPeg")) {
                cCrumblingPeg *n = new cCrumblingPeg;
                n->hObj = obj;
                n->bRun = 0;
                n->fAnimTimer = 0;
                vCrumbPeg.push_back(n);
                stObjUserDataSimEntity *se = new stObjUserDataSimEntity();
                se->enType = SE_Crumble;
                se->hPtr = (int *) n;
                GetUserDataFromObj(obj)->SetSpecialData((int *) se);
            } else if (!strcmp(obj->GetLogic(), "SpecialPowerup") &&
                       (!strcmp(obj->GetImageSet(), "GAME_WARP") || !strcmp(obj->GetImageSet(), "GAME_VERTWARP"))) {
                cWarp *w = new cWarp;
                w->hObj = obj;
                vWarp.push_back(w);
                stObjUserDataSimEntity *se = new stObjUserDataSimEntity();
                se->enType = SE_Warp;
                se->hPtr = (int *) w;
                GetUserDataFromObj(obj)->SetSpecialData((int *) se);
            } else if (!strcmp(obj->GetLogic(), "PowderKeg")) {
                cPowderKeg *n = new cPowderKeg;
                n->hObj = obj;
                n->bExplode = 0;
                n->fAnimTimer = 0;
                vPowderKeg.push_back(n);
                stObjUserDataSimEntity *se = new stObjUserDataSimEntity();
                se->enType = SE_PowderKeg;
                se->hPtr = (int *) n;
                GetUserDataFromObj(obj)->SetSpecialData((int *) se);
            } else if (!strcmp(obj->GetLogic(), "Soldier") || !strcmp(obj->GetLogic(), "Officer") ||
                       !strcmp(obj->GetLogic(), "Rat") ||
                       !strcmp(obj->GetLogic(), "RobberThief") || !strcmp(obj->GetLogic(), "CutThroat") ||
                       !strcmp(obj->GetLogic(), "Seagull") ||
                       !strcmp(obj->GetLogic(), "TownGuard1") || !strcmp(obj->GetLogic(), "TownGuard2") ||
                       !strcmp(obj->GetLogic(), "RedTailPirate") || !strcmp(obj->GetLogic(), "BearSailor") ||
                       !strcmp(obj->GetLogic(), "HermitCrab") || !strcmp(obj->GetLogic(), "PegLeg") ||
                       !strcmp(obj->GetLogic(), "CrazyHook") ||
                       !strcmp(obj->GetLogic(), "Siren") || !strcmp(obj->GetLogic(), "Mercat") ||
                       !strcmp(obj->GetLogic(), "Fish") ||
                       !strcmp(obj->GetLogic(), "TigerGuard") || !strcmp(obj->GetLogic(), "Chameleon")) {
                if (GetUserDataFromObj(obj)->GetZ() == 0) GetUserDataFromObj(obj)->SetZ(4000);
                cEnemy *n = new cEnemy;
                n->hObj = obj;
                n->bDirection = 1;
                n->fMoveSpeed = 120;
                n->fAniTimer = 0;
                n->iAniFrame = 0;
                n->iMinX = obj->GetParam(WWD::Param_MinX);
                n->iMaxX = obj->GetParam(WWD::Param_MaxX);
                n->bMoving = 1;
                n->bHit = 0;
                GetUserDataFromObj(n->hObj)->SetFlip(n->bDirection, 0);
                bool bAlignPos = 1;
                n->bNoWalk = 0;
                if (!strcmp(obj->GetLogic(), "Officer")) {
                    n->vIdleAnims.push_back("STAND1");
                    n->vIdleAnims.push_back("STAND2");
                    n->vIdleAnims.push_back("STAND3");
                    n->vIdleAnims.push_back("STAND4");
                    n->vIdleAnims.push_back("STAND5");
                    n->strWalkAnim = "FASTADVANCE";
                    n->bStopping = 1;
                } else if (!strcmp(obj->GetLogic(), "Soldier")) {
                    n->vIdleAnims.push_back("STAND");
                    n->vIdleAnims.push_back("STAND1");
                    n->vIdleAnims.push_back("STAND2");
                    n->strWalkAnim = "FASTADVANCE";
                    n->bStopping = 1;
                } else if (!strcmp(obj->GetLogic(), "Rat")) {
                    n->vIdleAnims.push_back("IDLE");
                    n->strWalkAnim = "WALK";
                    n->bStopping = 0;
                    n->fMoveSpeed = 60;
                } else if (!strcmp(obj->GetLogic(), "CutThroat")) {
                    n->vIdleAnims.push_back("STAND1");
                    n->vIdleAnims.push_back("STAND2");
                    n->vIdleAnims.push_back("STAND3");
                    n->vIdleAnims.push_back("STAND4");
                    n->strWalkAnim = "FASTADVANCE";
                    n->bStopping = 1;
                } else if (!strcmp(obj->GetLogic(), "RobberThief")) {
                    n->vIdleAnims.push_back("IDLE");
                    n->vIdleAnims.push_back("IDLE1");
                    n->vIdleAnims.push_back("IDLE2");
                    n->vIdleAnims.push_back("IDLE3");
                    n->vIdleAnims.push_back("IDLE4");
                    n->vIdleAnims.push_back("IDLE5");
                    n->strWalkAnim = "FASTADVANCE";
                    n->bStopping = 1;
                } else if (!strcmp(obj->GetLogic(), "TownGuard1") || !strcmp(obj->GetLogic(), "TownGuard2")) {
                    n->vIdleAnims.push_back("IDLE1");
                    n->vIdleAnims.push_back("IDLE2");
                    n->strWalkAnim = "FASTADVANCE";
                    n->bStopping = 1;
                } else if (!strcmp(obj->GetLogic(), "Seagull")) {
                    n->vIdleAnims.push_back("HOME");
                    n->strWalkAnim = "FLYING";
                    n->bStopping = 0;
                    bAlignPos = 0;
                } else if (!strcmp(obj->GetLogic(), "RedTailPirate")) {
                    n->vIdleAnims.push_back("IDLE");
                    n->vIdleAnims.push_back("HOME");
                    n->strWalkAnim = "FASTADVANCE";
                    n->bStopping = 1;
                } else if (!strcmp(obj->GetLogic(), "HermitCrab")) {
                    n->vIdleAnims.push_back("IDLE");
                    n->strWalkAnim = "FASTADVANCE";
                    n->bStopping = 1;
                    n->bNoWalk = 1;
                    n->bMoving = 0;
                } else if (!strcmp(obj->GetLogic(), "BearSailor")) {
                    n->vIdleAnims.push_back("IDLE");
                    n->vIdleAnims.push_back("HOME");
                    n->strWalkAnim = "FASTADVANCE";
                    n->bStopping = 1;
                } else if (!strcmp(obj->GetLogic(), "CrazyHook")) {
                    n->vIdleAnims.push_back("IDLE1");
                    n->strWalkAnim = "FASTADVANCE";
                    n->bStopping = 1;
                } else if (!strcmp(obj->GetLogic(), "PegLeg")) {
                    n->vIdleAnims.push_back("IDLE");
                    n->vIdleAnims.push_back("IDLE1");
                    n->vIdleAnims.push_back("IDLE2");
                    n->vIdleAnims.push_back("IDLE3");
                    n->vIdleAnims.push_back("IDLE4");
                    n->strWalkAnim = "FASTADVANCE";
                    n->bStopping = 1;
                } else if (!strcmp(obj->GetLogic(), "Siren")) {
                    n->vIdleAnims.push_back("IDLE");
                    n->strWalkAnim = "FASTADVANCE";
                    n->bStopping = 1;
                } else if (!strcmp(obj->GetLogic(), "Mercat")) {
                    n->vIdleAnims.push_back("HOME");
                    n->vIdleAnims.push_back("IDLE1");
                    n->vIdleAnims.push_back("IDLE2");
                    n->vIdleAnims.push_back("IDLE3");
                    n->vIdleAnims.push_back("IDLE4");
                    n->strWalkAnim = "FASTADVANCE";
                    n->bStopping = 1;
                } else if (!strcmp(obj->GetLogic(), "Fish")) {
                    n->vIdleAnims.push_back("IDLE1");
                    n->strWalkAnim = "SWIM";
                    n->bStopping = 0;
                    bAlignPos = 0;
                    n->fMoveSpeed = 90;
                } else if (!strcmp(obj->GetLogic(), "TigerGuard")) {
                    n->vIdleAnims.push_back("HOME");
                    n->vIdleAnims.push_back("IDLE");
                    n->strWalkAnim = "FASTADVANCE";
                    n->bStopping = 1;
                } else if (!strcmp(obj->GetLogic(), "Chameleon")) {
                    n->vIdleAnims.push_back("IDLE");
                    n->strWalkAnim = "FASTADVANCE";
                    n->bStopping = 1;
                }
                char anim[256];
                sprintf(anim, "%s_%s", obj->GetImageSet(), n->strWalkAnim.c_str());
                n->hAni = GV->editState->hAniBank->GetAssetByID(anim)->GetAni();
                hgeSprite *spr = GV->editState->SprBank->GetObjectSprite(n->hObj);
                sprintf(anim, "%s_HITLOW", obj->GetImageSet());
                if (GV->editState->hAniBank->GetAssetByID(anim) != 0) {
                    n->hAniHit = GV->editState->hAniBank->GetAssetByID(anim)->GetAni();
                } else {
                    n->hAniHit = 0;
                }

                if (bAlignPos) {
                    int atilex =
                            n->hObj->GetParam(WWD::Param_LocationX) / GV->editState->GetActivePlane()->GetTileWidth(),
                            atiley =
                            n->hObj->GetParam(WWD::Param_LocationY) / GV->editState->GetActivePlane()->GetTileHeight();
                    WWD::Plane *pl = GV->editState->GetActivePlane();
                    int endtile = -1, endoff = 0;
                    for (int y = atiley; y < pl->GetPlaneHeight(); y++) {
                        endoff = GetTileColY(GV->editState->hParser->GetTileAtribs(pl->GetTile(atilex, y)->GetID()));
                        if (endoff == -1) {
                            endoff = 0;
                        } else {
                            endtile = y;
                            break;
                        }
                    }
                    if (endtile == -1) {
                        GV->Console->Printf("~r~Warning: ~w~NPC id ~y~%d ~w~is in air, unable to emulate. Skipping.");
                        continue;
                    }
                    float hsx, hsy;
                    spr->GetHotSpot(&hsx, &hsy);
                    int newy = endtile * GV->editState->GetActivePlane()->GetTileHeight() - spr->GetHeight() / 2 +
                               (hsy - spr->GetHeight() / 2) + endoff;
                    GetUserDataFromObj(n->hObj)->SetY(newy);

                    if (n->iMaxX == 0)
                        n->iMaxX = pl->GetPlaneWidthPx();

                    int limitmin = 0, limitmax = pl->GetPlaneWidthPx();
                    for (int x = atilex; x < pl->GetPlaneWidth(); x++) {
                        WWD::TileAtrib *atr = GV->editState->hParser->GetTileAtribs(pl->GetTile(x, endtile)->GetID());
                        if (GetTileColY(atr) != endoff) {
                            limitmax = x * GV->editState->GetActivePlane()->GetTileWidth() - spr->GetWidth() * 0.25;
                            break;
                        }
                        if (GetTileColX(atr, 1) != 0) {
                            limitmax =
                                    (x + 1) * GV->editState->GetActivePlane()->GetTileWidth() - spr->GetWidth() * 0.25 -
                                    GetTileColX(atr, 1);
                            break;
                        }
                    }
                    for (int x = atilex; x >= 0; x--) {
                        WWD::TileAtrib *atr = GV->editState->hParser->GetTileAtribs(pl->GetTile(x, endtile)->GetID());
                        if (GetTileColY(atr) != endoff) {
                            limitmin =
                                    (x + 1) * GV->editState->GetActivePlane()->GetTileWidth() + spr->GetWidth() * 0.25;
                            break;
                        }
                        if (GetTileColX(atr, 0) != 0) {
                            limitmin = x * GV->editState->GetActivePlane()->GetTileWidth() + spr->GetWidth() * 0.25 +
                                       GetTileColX(atr, 0);
                            break;
                        }
                    }
                    if (limitmin > n->iMinX) n->iMinX = limitmin;
                    if (limitmax < n->iMaxX) n->iMaxX = limitmax;
                }

                vEnemies.push_back(n);
                if (n->hAniHit != 0) {
                    stObjUserDataSimEntity *se = new stObjUserDataSimEntity();
                    se->enType = SE_Enemy;
                    se->hPtr = (int *) n;
                    GetUserDataFromObj(obj)->SetSpecialData((int *) se);
                }
            }
            /*}else if( !strcmp(obj->GetLogic(), "Rat") ||
                      !strcmp(obj->GetLogic(), "Soldier") ||
                      !strcmp(obj->GetLogic(), "Officer") ||
                      !strcmp(obj->GetLogic(), "Punkrat") ||
                      !strcmp(obj->GetLogic(), "Raux") ||
                      !strcmp(obj->GetLogic(), "RobberThief") ||
                      !strcmp(obj->GetLogic(), "CutThroat") ||
                      !strcmp(obj->GetLogic(), "Katherine") ||
                      !strcmp(obj->GetLogic(), "Seagull") ||
                      !strcmp(obj->GetLogic(), "Townguard") ||
                      !strcmp(obj->GetLogic(), "Townguard2") ||
                      !strcmp(obj->GetLogic(), "Wolvington") ||
                      !strcmp(obj->GetLogic(), "HermitCrab") ||
                      !strcmp(obj->GetLogic(), "BearSailor") ||
                      !strcmp(obj->GetLogic(), "Redtailmen") ||
                      !strcmp(obj->GetLogic(), "Gabriel") ||
                      !strcmp(obj->GetLogic(), "Pegleg") || //yup, pirate with guns
                      !strcmp(obj->GetLogic(), "Hook") ||
                      !strcmp(obj->GetLogic(), "Marrow") ||
                      !strcmp(obj->GetLogic(), "Fish") ||
                      !strcmp(obj->GetLogic(), "Mercat") ||
                      !strcmp(obj->GetLogic(), "Mermaid") ||
                      !strcmp(obj->GetLogic(), "Aquatis") ||
                      !strcmp(obj->GetLogic(), "Chameleon") ||
                      !strcmp(obj->GetLogic(), "RedTail") ||
                      !strcmp(obj->GetLogic(), "Tiger") ||
                      !strcmp(obj->GetLogic(), "Omar") ){
             char * upname = SHR::ToUpper(obj->GetLogic());
             char anim[256];
             sprintf(anim, "LEVEL_%s_STAND", upname);
             int istart = 0;
             if( mO->hAniBank->GetAssetByID(anim) == NULL ){
              istart++;
              sprintf(anim, "LEVEL_%s_STAND1", upname);
              if( mO->hAniBank->GetAssetByID(anim) == NULL ){
               delete [] upname;
               continue;
              }
             }
             cEnemy n;
             for(int i=istart;1;i++){
              if( i == 0 )
               sprintf(anim, "LEVEL_%s_STAND", upname);
              else
               sprintf(anim, "LEVEL_%s_STAND%d", upname, i);
              if( mO->hAniBank->GetAssetByID(anim) == NULL ) break;
              ANI::Animation * ani = mO->hAniBank->GetAssetByID(anim)->GetAni();
              for(int y=0;y<ani->GetFramesCount();y++){
               n.vFrames.push_back(ani->GetFrame(y));
              }
             }
             n.fTimer = 0;
             n.fEndTimer = 2.0f;
             n.iFrame = 0;
             n.hObj = obj;
             n.bDirection = hge->Random_Int(0,1);
             n.bInEndTimer = hge->Random_Int(0,1);
             vEnemies.push_back(n);
             delete [] upname;
            }*/
        }
    }
    GV->Console->Printf("~w~Found %d/%d active objects.",
                        vRopes.size() + vTogglePegs.size() + vEyeCandies.size() + vElevators.size() + vEnemies.size() +
                        vSndTrig.size() + vPElevators.size() + vAmbients.size() + vProjSpawner.size() +
                        vCrumbPeg.size() +
                        vPowderKeg.size() + vCheckpoint.size() + vEnemies.size(),
                        GV->editState->GetActivePlane()->GetObjectsCount());
}

cGameSimulator::~cGameSimulator() {
    GV->Console->Printf("~w~Releasing simulation objects...");
    for (int i = 0; i < vElevators.size(); i++)
        delete vElevators[i];
    for (int i = 0; i < vCrumbPeg.size(); i++)
        delete vCrumbPeg[i];
    for (int i = 0; i < vPowderKeg.size(); i++)
        delete vPowderKeg[i];
    for (int i = 0; i < vCheckpoint.size(); i++)
        delete vCheckpoint[i];
    for (int i = 0; i < vEnemies.size(); i++)
        delete vEnemies[i];
    for (int i = 0; i < vWarp.size(); i++)
        delete vWarp[i];
    for (int i = 0; i < vProjSpawner.size(); i++) {
        for (int p = 0; p < vProjSpawner[i].vhChildren.size(); p++) {
            GV->editState->GetActivePlane()->DeleteObject(vProjSpawner[i].vhChildren[p]->hObj);
            vProjSpawner[i].vhChildren.erase(vProjSpawner[i].vhChildren.begin() + p);
            p--;
        }
    }

    for (int p = 0; p < hPar->GetPlanesCount(); p++) {
        WWD::Plane *pl = hPar->GetPlane(p);
        if (pl->GetObjectsCount() == 0) continue;
        for (int o = 0; o < pl->GetObjectsCount(); o++) {
            delete (stObjUserDataSimEntity *) GetUserDataFromObj(pl->GetObjectByIterator(o))->GetSpecialData();
            GetUserDataFromObj(pl->GetObjectByIterator(o))->SetSpecialData(0);
            GetUserDataFromObj(pl->GetObjectByIterator(o))->SyncToObj();
        }
    }
    mO->vPort->MarkToRedraw();
    GV->Console->Printf("~g~Simulation data released.");
}

void cGameSimulator::ChangeWeapon() {
    iWeaponFrameHUD = 0;
    fWeaponTimerHUD = 0;
}

#ifdef REAL_SIM
void cGameSimulator::Render()
{
    if( !bPlay ) return;

    if( clawDialog != 0 ){
     if( !hge->Channel_IsPlaying(clawDialog) ){
      hge->Channel_Stop(clawDialog);
      clawDialog = 0;
     }else{
      int x = GV->editState->Wrd2ScrX(hMainPlane, GetUserDataFromObj(GV->editState->hStartingPosObj)->GetX()),
          y = GV->editState->Wrd2ScrY(hMainPlane, GetUserDataFromObj(GV->editState->hStartingPosObj)->GetY()-75);
      GV->editState->SprBank->GetAssetByID("GAME_EXCLAMATION")->GetIMGByIterator(0)->GetSprite()->Render(x, y);
     }
    }

    if( hPW != NULL ){
     std::vector<cPhysicBody*> bodies = hPW->GetTree()->GetBodiesByArea(GV->editState->fCamX, GV->editState->fCamY, hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT));
     for(int i=0;i<bodies.size();i++){
      int x1 = GV->editState->Wrd2ScrX(hMainPlane, bodies[i]->GetX()),
          y1 = GV->editState->Wrd2ScrY(hMainPlane, bodies[i]->GetY()),
          x2 = GV->editState->Wrd2ScrX(hMainPlane, bodies[i]->GetX()+bodies[i]->GetWidth()),
          y2 = GV->editState->Wrd2ScrY(hMainPlane, bodies[i]->GetY()+bodies[i]->GetHeight());
      DWORD dwCol;
      if( bodies[i]->IsHidden() )
       dwCol = 0x77777777;
      else if( bodies[i]->GetAtrib() == WWD::Atrib_Clear )
       dwCol = 0xFFFF00FF;
      else if( bodies[i]->GetAtrib() == WWD::Atrib_Climb )
       dwCol = COLOR_CLIMB;
      else if( bodies[i]->GetAtrib() == WWD::Atrib_Death )
       dwCol = COLOR_DEATH;
      else if( bodies[i]->GetAtrib() == WWD::Atrib_Ground )
       dwCol = COLOR_GROUND;
      else if( bodies[i]->GetAtrib() == WWD::Atrib_Solid )
       dwCol = COLOR_SOLID;
      hge->Gfx_RenderLine(x1, y1, x2, y1, dwCol);
      hge->Gfx_RenderLine(x2, y1, x2, y2, dwCol);
      hge->Gfx_RenderLine(x1, y1, x1, y2, dwCol);
      hge->Gfx_RenderLine(x1, y2, x2, y2, dwCol);
      hge->Gfx_RenderLine(x1, y1, x2, y2, dwCol);
      hge->Gfx_RenderLine(x1, y2, x2, y1, dwCol);
      for(int y=0;y<bodies[i]->GetCollisionsCount();y++){
       hge->Gfx_RenderLine(x1+bodies[i]->GetWidth()/2*GV->editState->fZoom, y1+bodies[i]->GetHeight()/2*GV->editState->fZoom,
                           GV->editState->Wrd2ScrX(hMainPlane, bodies[i]->GetCollisionBody(y)->GetX()+bodies[i]->GetCollisionBody(y)->GetWidth()/2),
                           GV->editState->Wrd2ScrY(hMainPlane, bodies[i]->GetCollisionBody(y)->GetY()+bodies[i]->GetCollisionBody(y)->GetHeight()/2),
                           0xFFFFCC00);
      }
     }
    }

    //anim game_interface_chest
    cSprBankAssetIMG * img = GV->editState->SprBank->GetAssetByID("GAME_INTERFACE_TREASURECHEST")->GetIMGByNoID(GV->editState->hAniBank->GetAssetByID("GAME_INTERFACE_CHEST")->GetAni()->GetFrame(iChestFrame)->GetImageID());
    if( img == NULL )
     img = GV->editState->SprBank->GetAssetByID("GAME_INTERFACE_TREASURECHEST")->GetIMGByIterator(0);
    img->GetSprite()->Render(14, 15);
    char score[11];
    sprintf(score, "%010d", iScore);
    cSprBankAsset * as = GV->editState->SprBank->GetAssetByID("GAME_INTERFACE_SCORENUMBERS");
    for(int i=0;i<10;i++){
     as->GetIMGByID(score[i]-48)->GetSprite()->Render(36+i*11, 15);
    }

    GV->editState->SprBank->GetAssetByID("GAME_INTERFACE_HEALTHHEART")->GetIMGByIterator(iHeartFrame)->GetSprite()->Render(hge->System_GetState(HGE_SCREENWIDTH)-32, 15);
    sprintf(score, "%03d", iHealth);
    as = GV->editState->SprBank->GetAssetByID("GAME_INTERFACE_HEALTHNUMBERS");
    for(int i=0;i<3;i++){
     as->GetIMGByID(score[i]-48)->GetSprite()->Render(hge->System_GetState(HGE_SCREENWIDTH)-53+i*11, 14);
    }

    img = GV->editState->SprBank->GetAssetByID(WEAPON_IMGSET[iSelectedWeapon])->GetIMGByIterator(GV->editState->hAniBank->GetAssetByID(WEAPON_ANIMS[iSelectedWeapon])->GetAni()->GetFrame(iWeaponFrameHUD)->GetImageID()-1);
    if( img == NULL )
     img = GV->editState->SprBank->GetAssetByID(WEAPON_IMGSET[iSelectedWeapon])->GetIMGByIterator(0);
    img->GetSprite()->Render(hge->System_GetState(HGE_SCREENWIDTH)-26, 46);
    sprintf(score, "%02d", iAmmo[iSelectedWeapon]);
    as = GV->editState->SprBank->GetAssetByID("GAME_INTERFACE_SMALLNUMBERS");
    for(int i=0;i<2;i++){
     as->GetIMGByID(score[i]-48)->GetSprite()->Render(hge->System_GetState(HGE_SCREENWIDTH)-40+i*8, 46);
    }

    GV->editState->SprBank->GetAssetByID("GAME_INTERFACE_LIVESHEAD")->GetIMGByIterator(0)->GetSprite()->Render(hge->System_GetState(HGE_SCREENWIDTH)-18, 72);
    sprintf(score, "%01d", iLives);;
    as->GetIMGByID(score[0]-48)->GetSprite()->Render(hge->System_GetState(HGE_SCREENWIDTH)-33, 73);
}
#endif

void cGameSimulator::PlayerPlayAnim(cAniBankAsset *hAni) {
    hClawAnimPlay = hAni;
    fPlayerAniTimer = 0;
    iPlayerAniPlayFrame = 0;
}

void cGameSimulator::PlayAnimSound(ANI::Frame *fr) {
    if (fr->GetKeyFrameName() != NULL) {
        cSndBankAsset *as = GV->editState->hSndBank->GetAssetByID(fr->GetKeyFrameName());
        if (as != NULL) {
            hge->Effect_Play(as->GetSound());
        }
    }
}

void cGameSimulator::Think() {
    if (!bInited)
        Init();
#ifdef REAL_SIM
    if( bPlay ){

     fHeartTimer += hge->Timer_GetDelta()*1000.0f;
     while(fHeartTimer > 150){
      fHeartTimer -= 150;
      iHeartFrame++;
      if( iHeartFrame >= 3 )
       iHeartFrame = 0;
     }

     fChestTimer += hge->Timer_GetDelta()*1000.0f;
     while( fChestTimer > GV->editState->hAniBank->GetAssetByID("GAME_INTERFACE_CHEST")->GetAni()->GetFrame(iChestFrame)->GetDuration() ){
      fChestTimer -= GV->editState->hAniBank->GetAssetByID("GAME_INTERFACE_CHEST")->GetAni()->GetFrame(iChestFrame)->GetDuration();
      iChestFrame++;
      if( iChestFrame >= GV->editState->hAniBank->GetAssetByID("GAME_INTERFACE_CHEST")->GetAni()->GetFramesCount() )
       iChestFrame = 0;
     }

     fWeaponTimerHUD += hge->Timer_GetDelta()*1000.0f;
     while( fWeaponTimerHUD > GV->editState->hAniBank->GetAssetByID(WEAPON_ANIMS[iSelectedWeapon])->GetAni()->GetFrame(iWeaponFrameHUD)->GetDuration() ){
      fWeaponTimerHUD -= GV->editState->hAniBank->GetAssetByID(WEAPON_ANIMS[iSelectedWeapon])->GetAni()->GetFrame(iWeaponFrameHUD)->GetDuration();
      iWeaponFrameHUD++;
      if( iWeaponFrameHUD >= GV->editState->hAniBank->GetAssetByID(WEAPON_ANIMS[iSelectedWeapon])->GetAni()->GetFramesCount() )
       iWeaponFrameHUD = 0;
     }

     GV->editState->fCamX = GetUserDataFromObj(GV->editState->hStartingPosObj)->GetX()-(GV->editState->vPort->GetWidth()/2/GV->editState->fZoom);
     GV->editState->fCamY = GetUserDataFromObj(GV->editState->hStartingPosObj)->GetY()-(GV->editState->vPort->GetHeight()/2/GV->editState->fZoom);
     cAniBankAsset * curAni = hClawAnim;

     if( hge->Input_KeyDown(HGEK_1) ){
      iSelectedWeapon = WeapPISTOL;
      ChangeWeapon();
     }else if( hge->Input_KeyDown(HGEK_2) ){
      iSelectedWeapon = WeapMAGIC;
      ChangeWeapon();
     }else if( hge->Input_KeyDown(HGEK_3) ){
      iSelectedWeapon = WeapDYNAMITE;
      ChangeWeapon();
     }else if( hge->Input_KeyDown(HGEK_SHIFT) ){
      iSelectedWeapon = (GameSim::enWeapons)(int(iSelectedWeapon)+1);
      if( iSelectedWeapon >= GAMESIM_WEAP_NUM )
       iSelectedWeapon = WeapPISTOL;
      ChangeWeapon();
     }

     bool bMoving = 0;

     if( !bClimbing )
     if( hge->Input_GetKeyState(HGEK_RIGHT) ){
      GetUserDataFromObj(GV->editState->hStartingPosObj)->SetFlip(0, 0);
      if( !hge->Input_GetKeyState(HGEK_DOWN) && !hge->Input_GetKeyState(HGEK_UP) ){
       hClawAnim = GV->editState->hAniBank->GetAssetByID("CLAW_WALK");
       iPlayerAniMode = AniLOOP;
       bPlayerAniHold = 0;
       bodyClaw->SetSpeedX(250);
       bMoving = 1;
       //GetUserDataFromObj(GV->editState->hStartingPosObj)->SetX(GetUserDataFromObj(GV->editState->hStartingPosObj)->GetX()+250*hge->Timer_GetDelta());
      }

     }else if( hge->Input_GetKeyState(HGEK_LEFT) ){
      GetUserDataFromObj(GV->editState->hStartingPosObj)->SetFlip(1, 0);
      if( !hge->Input_GetKeyState(HGEK_DOWN) && !hge->Input_GetKeyState(HGEK_UP) ){
       iPlayerAniMode = AniLOOP;
       bPlayerAniHold = 0;
       hClawAnim = GV->editState->hAniBank->GetAssetByID("CLAW_WALK");
       bodyClaw->SetSpeedX(-250);
       bMoving = 1;
       //GetUserDataFromObj(GV->editState->hStartingPosObj)->SetX(GetUserDataFromObj(GV->editState->hStartingPosObj)->GetX()-250*hge->Timer_GetDelta());
      }

     }

     if( bClimbing ){
      bool bf = 0;
      for(int i=0;i<bodyClaw->GetCollisionsCount();i++)
       if( bodyClaw->GetCollisionBody(i) == hLadder ){
        bf = 1;
        break;
       }
      if( !bf ){
       bClimbing = 0;
       hLadder = 0;
       hClawAnimPlay = 0;
       iPlayerAniPlayFrame = 0;
       fPlayerAniTimer = 0;
       bStopAnim = 0;
       bodyClaw->EnableGravity(1);
      }
     }

     if( bClimbing ){
      if( hge->Input_GetKeyState(HGEK_UP) ){
       bStopAnim = 0;
       bodyClaw->SetSpeedY(-250);
      }else if( hge->Input_GetKeyState(HGEK_DOWN) ){
       bStopAnim = 0;
       bodyClaw->SetSpeedY(250);
      }else{
       bStopAnim = 1;
       bodyClaw->SetSpeedY(0);
      }
     }

     if( !bodyClaw->IsInAir() && hClawAnimPlay == GV->editState->hAniBank->GetAssetByID("CLAW_JUMP") )
      hClawAnimPlay = 0;

     if( hge->Input_KeyDown(HGEK_UP) && !bClimbing && bodyClaw->IsInAir() && bodyClaw->GetCollisionsCount() != 0 ){
      bool bLadder = 0;
      for(int i=0;i<bodyClaw->GetCollisionsCount();i++)
       if( bodyClaw->GetCollisionBody(i)->GetAtrib() == WWD::Atrib_Climb ){
        bLadder = 1;
        hLadder = bodyClaw->GetCollisionBody(i);
        break;
       }
      if( bLadder ){
       bodyClaw->EnableGravity(0);
       bClimbing = 1;
       hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_CLIMB");
       iPlayerAniPlayMode = AniLOOP;
       bPlayerAniHold = 0;
       bodyClaw->SetSpeed(0,0);
       bStopAnim = 1;
      }
     }else if( hge->Input_GetKeyState(HGEK_UP) && !bodyClaw->IsInAir() && !bClimbing ){
      hClawAnim = GV->editState->hAniBank->GetAssetByID("CLAW_LOOKUP");
      iPlayerAniMode = AniHOLD;
      bPlayerAniHold = 0;

     }else if( hge->Input_GetKeyState(HGEK_DOWN) && !bodyClaw->IsInAir() && !bClimbing ){
      hClawAnim = GV->editState->hAniBank->GetAssetByID("CLAW_DUCK");
      GetUserDataFromObj(GV->editState->hStartingPosObj)->SetI(GV->editState->SprBank->GetAssetByID("CLAW")->GetIMGByNoID(hClawAnim->GetAni()->GetFrame(0)->GetImageID())->GetIt()+1);
      bPlayerAniHold = 0;
     }else if( hge->Input_KeyDown(HGEK_SPACE) && !bodyClaw->IsInAir() ){
      bodyClaw->EnableGravity(1);
      hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_JUMP");
      iPlayerAniPlayMode = AniONCE;
      bStopAnim = 0;
      bPlayerAniHold = 0;
      iPlayerAniPlayFrame = 0;
      fPlayerAniTimer = 0;
      bodyClaw->SetSpeed(250*( GetUserDataFromObj(GV->editState->hStartingPosObj)->GetFlipX() ? -1 : 1 ), -500 );
     }else if( hClawAnim != GV->editState->hAniBank->GetAssetByID("CLAW_STAND") && !bMoving && !bClimbing && hClawAnimPlay == 0 ){
      hClawAnim = GV->editState->hAniBank->GetAssetByID("CLAW_STAND");
      iPlayerAniMode = AniLOOP;
      bPlayerAniHold = 0;
     }
     if( hge->Input_KeyDown(HGEK_CTRL) ){
      if( bodyClaw->IsInAir() )
       hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_JUMPPOWERUPSWIPE");
      else if( hge->Input_GetKeyState(HGEK_DOWN) )
       hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_DUCKSWIPE");
      else{
       int anim = hge->Random_Int(0, 3);
       if( anim == 0 )
        hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_POWERUPSWIPE");
       else if( anim == 1 )
        hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_UPPERCUT");
       else if( anim == 2 )
        hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_PUNCH");
       else if( anim == 3 )
        hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_KICK");
      }
      iPlayerAniPlayMode = AniONCE;
      bPlayerAniHold = 0;
      iPlayerAniPlayFrame = 0;
      fPlayerAniTimer = 0;
      PlayAnimSound(hClawAnimPlay->GetAni()->GetFrame(0));
     }else if( hge->Input_KeyDown(HGEK_ALT) ){
      if( iSelectedWeapon == WeapDYNAMITE ){
       if( bodyClaw->IsInAir() )
        hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_EMPTYJUMPDYNAMITE");
       else if( hge->Input_GetKeyState(HGEK_DOWN) )
        hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_DUCKEMPTYDYNAMITE");
       else
        hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_EMPTYDYNAMITE");
      }else if( iSelectedWeapon == WeapMAGIC ){
       if( bodyClaw->IsInAir() )
        hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_EMPTYJUMPMAGIC");
       else if( hge->Input_GetKeyState(HGEK_DOWN) )
        hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_DUCKEMPTYMAGIC");
       else
        hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_EMPTYMAGIC");
      }else if( iSelectedWeapon == WeapPISTOL ){
       if( bodyClaw->IsInAir() )
        hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_EMPTYJUMPPISTOL");
       else if( hge->Input_GetKeyState(HGEK_DOWN) )
        hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_DUCKEMPTYPISTOL");
       else
        hClawAnimPlay = GV->editState->hAniBank->GetAssetByID("CLAW_EMPTYPISTOL");
      }
      iPlayerAniPlayMode = AniONCE;
      bPlayerAniHold = 0;
      iPlayerAniPlayFrame = 0;
      fPlayerAniTimer = 0;
      PlayAnimSound(hClawAnimPlay->GetAni()->GetFrame(0));
     }
     if( bodyClaw->IsInAir() && !bClimbing )
      hClawAnim = GV->editState->hAniBank->GetAssetByID("CLAW_FALL");
     if( !bMoving )
      bodyClaw->SetSpeedX(0);
     if( curAni != hClawAnim ){
      iPlayerAniFrame = 0;
     }

     if( hClawAnimPlay != NULL && !bStopAnim ){
      fPlayerAniTimer += hge->Timer_GetDelta()*1000.0f;
      int fr = iPlayerAniPlayFrame;
      while( fPlayerAniTimer > hClawAnimPlay->GetAni()->GetFrame(iPlayerAniPlayFrame)->GetDuration() ){
       fPlayerAniTimer -= hClawAnimPlay->GetAni()->GetFrame(iPlayerAniPlayFrame)->GetDuration();
       iPlayerAniPlayFrame++;
       if( iPlayerAniPlayFrame >= hClawAnimPlay->GetAni()->GetFramesCount() )
        if( iPlayerAniPlayMode == AniLOOP ){
         iPlayerAniPlayFrame = 0;
        }else if( iPlayerAniPlayMode == AniONCE ){
         iPlayerAniPlayFrame = 0;
         fPlayerAniTimer = 0;
         hClawAnimPlay = 0;
         break;
        }else{
         iPlayerAniPlayFrame--;
         fPlayerAniTimer = 0;
         bPlayerAniHold = 1;
        }
       PlayAnimSound(hClawAnimPlay->GetAni()->GetFrame(iPlayerAniPlayFrame));
      }
      if( hClawAnimPlay != 0 )
       GetUserDataFromObj(GV->editState->hStartingPosObj)->SetI(GV->editState->SprBank->GetAssetByID("CLAW")->GetIMGByNoID(hClawAnimPlay->GetAni()->GetFrame(iPlayerAniPlayFrame)->GetImageID())->GetIt()+1);
      //if( fr != iPlayerAniPlayFrame && hClawAnimPlay != 0 )
      // GV->Console->Printf("animp %s/%d", hClawAnimPlay->GetID(), iPlayerAniPlayFrame);
     }else if( hClawAnim->GetAni()->GetFramesCount() > 1 && !bPlayerAniHold && !bStopAnim ){
      fPlayerAniTimer += hge->Timer_GetDelta()*1000.0f;
      int fr = iPlayerAniFrame;
      while( fPlayerAniTimer > hClawAnim->GetAni()->GetFrame(iPlayerAniFrame)->GetDuration() ){
       fPlayerAniTimer -= hClawAnim->GetAni()->GetFrame(iPlayerAniFrame)->GetDuration();
       iPlayerAniFrame++;
       if( iPlayerAniFrame >= hClawAnim->GetAni()->GetFramesCount() )
        if( iPlayerAniMode == AniLOOP )
         iPlayerAniFrame = 0;
        else{
         bPlayerAniHold = 1;
         iPlayerAniFrame--;
         break;
        }
       PlayAnimSound(hClawAnim->GetAni()->GetFrame(iPlayerAniFrame));
      }
      GetUserDataFromObj(GV->editState->hStartingPosObj)->SetI(GV->editState->SprBank->GetAssetByID("CLAW")->GetIMGByNoID(hClawAnim->GetAni()->GetFrame(iPlayerAniFrame)->GetImageID())->GetIt()+1);
      //if( fr != iPlayerAniFrame )
      // GV->Console->Printf("anim %s/%d", hClawAnim->GetID(), iPlayerAniFrame);
     }
     hPW->Update();
    }
#endif

    if (hSpringBoardAnim != 0) {
        fSpringBoardTimer += hge->Timer_GetDelta();
        if (fSpringBoardTimer * 1000 > hSpringBoardAnim->GetFrame(iSpringBoardFrame)->GetDuration()) {
            fSpringBoardTimer -= float(hSpringBoardAnim->GetFrame(iSpringBoardFrame)->GetDuration()) / 1000.0f;
            iSpringBoardFrame++;
            if (iSpringBoardFrame >= hSpringBoardAnim->GetFramesCount()) {
                iSpringBoardFrame = 0;
            }
        }
    }

    fGlitterTimer += hge->Timer_GetDelta();
    while (fGlitterTimer > 0.1) {
        fGlitterTimer -= 0.1;
        iGlitterFrame++;
        if (iGlitterFrame > 16) iGlitterFrame = 0;
    }

    if (GV->editState->vPort->GetWidget()->isMouseOver()) {
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        int wx = GV->editState->Scr2WrdX(GV->editState->GetActivePlane(), mx),
                wy = GV->editState->Scr2WrdY(GV->editState->GetActivePlane(), my);
        std::vector<WWD::Object *> objv = GV->editState->hPlaneData[GV->editState->GetActivePlaneID()]->ObjectData.hQuadTree->GetObjectsByArea(
                wx, wy, 2, 2);
        sort(objv.begin(), objv.end(), State::ObjSortCoordZ);
        WWD::Object *obj = 0;
        for (int i = 0; i < objv.size(); i++)
            if (GetUserDataFromObj(objv[i])->GetSpecialData() != NULL) {
                obj = objv[i];
                break;
            }
        if (obj != NULL) {
            if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                eSimEntity type = ((stObjUserDataSimEntity *) GetUserDataFromObj(obj)->GetSpecialData())->enType;
                if (type == SE_Elevator) {
                    cElevator *el = (cElevator *) (((stObjUserDataSimEntity *) GetUserDataFromObj(
                            obj)->GetSpecialData())->hPtr);
                    if (el->bSleep) el->bSleep = 0;
                    if (el->bOneWay || el->enType == ELEVATOR_START) {
                        delete ((stObjUserDataSimEntity *) GetUserDataFromObj(obj)->GetSpecialData());
                        GetUserDataFromObj(obj)->SetSpecialData(0);
                    }
                } else if (type == SE_Crumble) {
                    cCrumblingPeg *peg = (cCrumblingPeg *) (((stObjUserDataSimEntity *) GetUserDataFromObj(
                            obj)->GetSpecialData())->hPtr);
                    peg->bRun = 1;
                    cSndBankAsset *as = GV->editState->hSndBank->GetAssetByID("LEVEL_PEGCRUMBLE");
                    if (as)
                        hge->Effect_PlayEx(as->GetSound(), 100);
                    delete ((stObjUserDataSimEntity *) GetUserDataFromObj(obj)->GetSpecialData());
                    GetUserDataFromObj(obj)->SetSpecialData(0);
                } else if (type == SE_PowderKeg) {
                    cPowderKeg *keg = (cPowderKeg *) (((stObjUserDataSimEntity *) GetUserDataFromObj(
                            obj)->GetSpecialData())->hPtr);
                    keg->bExplode = 1;
                    hge->Effect_PlayEx(GV->editState->hSndBank->GetAssetByID("GAME_KEGBOOM")->GetSound(), 100);
                    GetUserDataFromObj(obj)->SetI(0);
                    delete ((stObjUserDataSimEntity *) GetUserDataFromObj(obj)->GetSpecialData());
                    GetUserDataFromObj(obj)->SetSpecialData(0);
                } else if (type == SE_Checkpoint) {
                    cCheckpoint *cp = (cCheckpoint *) (((stObjUserDataSimEntity *) GetUserDataFromObj(
                            obj)->GetSpecialData())->hPtr);
                    cp->fTimer = 0;
                    PlayAnimSound(aniFlagRise->GetFrame(0));
                    delete ((stObjUserDataSimEntity *) GetUserDataFromObj(obj)->GetSpecialData());
                    GetUserDataFromObj(obj)->SetSpecialData(0);
                } else if (type == SE_Enemy) {
                    cEnemy *n = (cEnemy *) (((stObjUserDataSimEntity *) GetUserDataFromObj(
                            obj)->GetSpecialData())->hPtr);
                    n->bHit = 1;
                    n->iHitFrame = 0;
                    GetUserDataFromObj(n->hObj)->SetI(n->hAniHit->GetFrame(0)->GetImageID());
                    n->fHitTimer = 0;
                    char snd[25];
                    sprintf(snd, "GAME_HIT%d", hge->Random_Int(1, 4));
                    hge->Effect_PlayEx(GV->editState->hSndBank->GetAssetByID(snd)->GetSound());
                } else if (type == SE_Warp) {
                    cWarp *n = (cWarp *) (((stObjUserDataSimEntity *) GetUserDataFromObj(obj)->GetSpecialData())->hPtr);
                    GV->editState->fCamX =
                            (n->hObj->GetParam(WWD::Param_SpeedX) - GV->editState->vPort->GetWidth() / 2) *
                            GV->editState->fZoom;
                    GV->editState->fCamY =
                            (n->hObj->GetParam(WWD::Param_SpeedY) - GV->editState->vPort->GetHeight() / 2) *
                            GV->editState->fZoom;
                    if (n->hObj->GetParam(WWD::Param_Smarts) == 0) {
                        delete ((stObjUserDataSimEntity *) GetUserDataFromObj(obj)->GetSpecialData());
                        GetUserDataFromObj(obj)->SetSpecialData(0);
                    }
                }
            }
        }
    }
    //bool bChange = 0;
    for (int i = 0; i < vRopes.size(); i++) {
        vRopes[i].fCounter += hge->Timer_GetDelta();
        while (vRopes[i].fCounter > vRopes[i].fFrameDuration) {
            vRopes[i].fCounter -= vRopes[i].fFrameDuration;
            vRopes[i].iFrame++;
            if (vRopes[i].iFrame > iRopeFramesCount)
                vRopes[i].iFrame = 0;
            GetUserDataFromObj(vRopes[i].hObj)->SetI(vRopes[i].iFrame);
            //bChange = 1;
        }
    }
    for (int i = 0; i < vTogglePegs.size(); i++) {
        if (vTogglePegs[i].fDelay != 0) {
            vTogglePegs[i].fDelay -= hge->Timer_GetDelta();
            if (vTogglePegs[i].fDelay < 0)
                vTogglePegs[i].fDelay = 0;
            continue;
        }
        vTogglePegs[i].fCounter += hge->Timer_GetDelta();
        if (vTogglePegs[i].bOn) {
            if (vTogglePegs[i].fCounter > vTogglePegs[i].fTimeOn) {
                vTogglePegs[i].fCounter -= vTogglePegs[i].fTimeOn;
                vTogglePegs[i].bOn = 0;
#ifdef REAL_SIM
                if( bPlay )
                vTogglePegs[i].bodyMe->Hide(1);
#endif
                //GetUserDataFromObj(vTogglePegs[i].hObj)->SetI(vTogglePegs[i].iFramesCount);
                //bChange = 1;
            }
        } else {
            int before = vTogglePegs[i].iFrame;
            if (vTogglePegs[i].fCounter < vTogglePegs[i].iFramesCount * vTogglePegs[i].fFrameDuration) {
                vTogglePegs[i].iFrame = vTogglePegs[i].fCounter / vTogglePegs[i].fFrameDuration;
            } else if (vTogglePegs[i].fCounter >
                       vTogglePegs[i].fTimeOff - vTogglePegs[i].iFramesCount * vTogglePegs[i].fFrameDuration) {
                vTogglePegs[i].iFrame = vTogglePegs[i].iFramesCount - (vTogglePegs[i].fCounter -
                                                                       (vTogglePegs[i].fTimeOff -
                                                                        vTogglePegs[i].iFramesCount *
                                                                        vTogglePegs[i].fFrameDuration)) /
                                                                      vTogglePegs[i].fFrameDuration;
            } else if (vTogglePegs[i].iFrame != vTogglePegs[i].iFramesCount) {
                vTogglePegs[i].iFrame = vTogglePegs[i].iFramesCount;
            }
            if (before != vTogglePegs[i].iFrame) {
                GetUserDataFromObj(vTogglePegs[i].hObj)->SetI(vTogglePegs[i].iFrame);
                //bChange = 1;
            }
            if (vTogglePegs[i].fCounter > vTogglePegs[i].fTimeOff) {
                vTogglePegs[i].fCounter -= vTogglePegs[i].fTimeOff;
                vTogglePegs[i].bOn = 1;
#ifdef REAL_SIM
                if( bPlay )
                 vTogglePegs[i].bodyMe->Hide(0);
#endif
                GetUserDataFromObj(vTogglePegs[i].hObj)->SetI(0);
                //bChange = 1;
            }
        }
    }
#ifdef REAL_SIM
    if( bPlay )
    for(int i=0;i<vCheckpoints.size();i++){
     if( vCheckpoints[i]->fTimer < 0 ) continue;
     vCheckpoints[i]->fTimer += hge->Timer_GetDelta()*1000.0f;
     int fr = vCheckpoints[i]->iFrame;
     ANI::Animation * ani = (vCheckpoints[i]->bWave ? aniFlagWave : aniFlagRise );
     while( vCheckpoints[i]->fTimer > ani->GetFrame(vCheckpoints[i]->iFrame)->GetDuration() ){
      vCheckpoints[i]->fTimer -= ani->GetFrame(vCheckpoints[i]->iFrame)->GetDuration();
      vCheckpoints[i]->iFrame++;
      if( vCheckpoints[i]->iFrame >= ani->GetFramesCount() ){
       vCheckpoints[i]->iFrame = 0;
       if( !vCheckpoints[i]->bWave ){
        vCheckpoints[i]->bWave = 1;
        ani = aniFlagWave;
       }
      }
      PlayAnimSound(ani->GetFrame(vCheckpoints[i]->iFrame));
     }
     if( fr != vCheckpoints[i]->iFrame )
      GetUserDataFromObj(vCheckpoints[i]->hObj)->SetI(GV->editState->SprBank->GetAssetByID(vCheckpoints[i]->hObj->GetImageSet())->GetIMGByID(ani->GetFrame(vCheckpoints[i]->iFrame)->GetImageID())->GetIt()+1);
    }
#endif
    int scrx1 = GV->editState->Scr2WrdX(GV->editState->GetActivePlane(), GV->editState->vPort->GetX()),
            scry1 = GV->editState->Scr2WrdY(GV->editState->GetActivePlane(), GV->editState->vPort->GetY()),
            scrx2 = GV->editState->Scr2WrdX(GV->editState->GetActivePlane(),
                                            GV->editState->vPort->GetX() + GV->editState->vPort->GetWidth()),
            scry2 = GV->editState->Scr2WrdY(GV->editState->GetActivePlane(),
                                            GV->editState->vPort->GetY() + GV->editState->vPort->GetHeight());
    for (int i = 0; i < vCheckpoint.size(); i++) {
        if (vCheckpoint[i]->fTimer < 0) continue;
        vCheckpoint[i]->fTimer += hge->Timer_GetDelta() * 1000.0f;
        int fr = vCheckpoint[i]->iFrame;
        ANI::Animation *ani = (vCheckpoint[i]->bWave ? aniFlagWave : aniFlagRise);
        while (vCheckpoint[i]->fTimer > ani->GetFrame(vCheckpoint[i]->iFrame)->GetDuration()) {
            vCheckpoint[i]->fTimer -= ani->GetFrame(vCheckpoint[i]->iFrame)->GetDuration();
            vCheckpoint[i]->iFrame++;
            if (vCheckpoint[i]->iFrame >= ani->GetFramesCount()) {
                vCheckpoint[i]->iFrame = 0;
                if (!vCheckpoint[i]->bWave) {
                    vCheckpoint[i]->bWave = 1;
                    ani = aniFlagWave;
                }
            }
            PlayAnimSound(ani->GetFrame(vCheckpoint[i]->iFrame));
        }
        if (fr != vCheckpoint[i]->iFrame)
            GetUserDataFromObj(vCheckpoint[i]->hObj)->SetI(
                    GV->editState->SprBank->GetAssetByID(vCheckpoint[i]->hObj->GetImageSet())->GetIMGByID(
                            ani->GetFrame(vCheckpoint[i]->iFrame)->GetImageID())->GetIt() + 1);
    }
    for (int i = 0; i < vPowderKeg.size(); i++) {
        if (vPowderKeg[i]->bExplode) {
            vPowderKeg[i]->fAnimTimer += hge->Timer_GetDelta();
            cObjUserData *ud = GetUserDataFromObj(vPowderKeg[i]->hObj);
            if (vPowderKeg[i]->fAnimTimer * 1000.0f > aniPowderExpl->GetFrame(ud->GetI())->GetDuration()) {
                vPowderKeg[i]->fAnimTimer -= float(aniPowderExpl->GetFrame(ud->GetI())->GetDuration()) / 1000.0f;
                int newi = ud->GetI();
                newi++;
                if (newi == 2) {
                    std::vector<WWD::Object *> objv = GV->editState->hPlaneData[GV->editState->GetActivePlaneID()]->ObjectData.hQuadTree->GetObjectsByArea(
                            vPowderKeg[i]->hObj->GetParam(WWD::Param_LocationX) - 75,
                            vPowderKeg[i]->hObj->GetParam(WWD::Param_LocationY) - 75,
                            150, 150);
                    for (int z = 0; z < objv.size(); z++) {
                        if (GetUserDataFromObj(objv[z])->GetSpecialData() != 0 &&
                            ((stObjUserDataSimEntity *) GetUserDataFromObj(objv[z])->GetSpecialData())->enType ==
                            SE_PowderKeg) {
                            cPowderKeg *keg = (cPowderKeg *) (((stObjUserDataSimEntity *) GetUserDataFromObj(
                                    objv[z])->GetSpecialData())->hPtr);
                            keg->bExplode = 1;
                            hge->Effect_PlayEx(GV->editState->hSndBank->GetAssetByID("GAME_KEGBOOM")->GetSound(), 100);
                            GetUserDataFromObj(objv[z])->SetI(0);
                            delete ((stObjUserDataSimEntity *) GetUserDataFromObj(objv[z])->GetSpecialData());
                            GetUserDataFromObj(objv[z])->SetSpecialData(0);
                        }
                    }
                }
                if (newi >= aniPowderExpl->GetFramesCount()) {
                    ud->SetVisible(0);
                    delete vPowderKeg[i];
                    vPowderKeg.erase(vPowderKeg.begin() + i);
                    i--;
                } else
                    ud->SetI(newi);
            }
        }
    }
    for (int i = 0; i < vCrumbPeg.size(); i++) {
        if (vCrumbPeg[i]->bRun) {
            vCrumbPeg[i]->fAnimTimer += hge->Timer_GetDelta();
            if (vCrumbPeg[i]->fAnimTimer > 0.05f) {
                vCrumbPeg[i]->fAnimTimer -= 0.05f;
                cObjUserData *ud = GetUserDataFromObj(vCrumbPeg[i]->hObj);
                int newi = ud->GetI();
                newi++;
                if (newi >=
                    GV->editState->SprBank->GetAssetByID(vCrumbPeg[i]->hObj->GetImageSet())->GetSpritesCount()) {
                    ud->SetVisible(0);
                    delete vCrumbPeg[i];
                    vCrumbPeg.erase(vCrumbPeg.begin() + i);
                    i--;
                } else
                    ud->SetI(newi);
            }
        }
    }
    for (int i = 0; i < vProjSpawner.size(); i++) {
        for (int p = 0; p < vProjSpawner[i].vhChildren.size(); p++) {
            cObjUserData *ud = GetUserDataFromObj(vProjSpawner[i].vhChildren[p]->hObj);
            if (vProjSpawner[i].vhChildren[p]->bPhase) { //flying
                ud->SetX(ud->GetX() + vProjSpawner[i].fSpeedX * hge->Timer_GetDelta());
                ud->SetY(ud->GetY() + vProjSpawner[i].fSpeedY * hge->Timer_GetDelta());
                if (GV->editState->hParser->GetBaseLevel() == 14) {
                    vProjSpawner[i].vhChildren[p]->fAnimTimer += hge->Timer_GetDelta();
                    if (vProjSpawner[i].vhChildren[p]->fAnimTimer > 0.075) {
                        vProjSpawner[i].vhChildren[p]->fAnimTimer -= 0.075;
                        int newi = ud->GetI();
                        newi++;
                        if (newi >= GV->editState->SprBank->GetAssetByID(
                                vProjSpawner[i].vhChildren[p]->hObj->GetImageSet())->GetSpritesCount())
                            newi = 0;
                        ud->SetI(newi);
                    }
                }
                if (abs(ud->GetX() - vProjSpawner[i].hObj->GetParam(WWD::Param_LocationX)) >
                    vProjSpawner[i].iDistance ||
                    abs(ud->GetY() - vProjSpawner[i].hObj->GetParam(WWD::Param_LocationY)) >
                    vProjSpawner[i].iDistance) {
                    GV->editState->GetActivePlane()->DeleteObject(vProjSpawner[i].vhChildren[p]->hObj);
                    vProjSpawner[i].vhChildren.erase(vProjSpawner[i].vhChildren.begin() + p);
                    p--;
                }
            } else { //animating
                ANI::Animation *ani = GV->editState->hAniBank->GetAssetByID(vProjSpawner[i].hObj->GetAnim())->GetAni();
                vProjSpawner[i].vhChildren[p]->fAnimTimer += hge->Timer_GetDelta();
                if (vProjSpawner[i].vhChildren[p]->fAnimTimer * 1000 > ani->GetFrame(ud->GetI())->GetDuration()) {
                    vProjSpawner[i].vhChildren[p]->fAnimTimer -= ani->GetFrame(ud->GetI())->GetDuration() / 1000.0f;
                    if (ani->GetFramesCount() > ud->GetI() + 1) {
                        ud->SetI(ud->GetI() + 1);
                    } else {
                        vProjSpawner[i].vhChildren[p]->bPhase = 1;
                        if (GV->editState->hParser->GetBaseLevel() == 14) {
                            vProjSpawner[i].vhChildren[p]->hObj->SetParam(WWD::Param_LocationI, 0);
                            vProjSpawner[i].vhChildren[p]->fAnimTimer = 0;
                            vProjSpawner[i].vhChildren[p]->hObj->SetImageSet(
                                    vProjSpawner[i].fSpeedX < 0 ? "LEVEL_PROJECTILES_1" : "LEVEL_PROJECTILES_2");
                        }
                    }
                }
            }
        }
        vProjSpawner[i].fDelayTimer += hge->Timer_GetDelta();
        int scrposx = vProjSpawner[i].hObj->GetParam(WWD::Param_LocationX),
                scrposy = vProjSpawner[i].hObj->GetParam(WWD::Param_LocationY);
        if (scrposx < scrx1 || scrposx > scrx2 || scrposy < scry1 || scrposy > scry2) continue;
        if (vProjSpawner[i].fDelayTimer > vProjSpawner[i].fDelayTime) {
            while (vProjSpawner[i].fDelayTimer > vProjSpawner[i].fDelayTime)
                vProjSpawner[i].fDelayTimer -= vProjSpawner[i].fDelayTime;
            //spawn new projectile
            WWD::Object *proj = new WWD::Object();
            proj->SetImageSet(vProjSpawner[i].hObj->GetImageSet());
            proj->SetLogic("WM_PROJ");
            proj->SetParam(WWD::Param_LocationX, vProjSpawner[i].hObj->GetParam(WWD::Param_LocationX));
            proj->SetParam(WWD::Param_LocationY, vProjSpawner[i].hObj->GetParam(WWD::Param_LocationY));
            proj->SetParam(WWD::Param_LocationZ, 4050);
            GV->editState->GetActivePlane()->AddObjectAndCalcID(proj);
            proj->SetUserData(new cObjUserData(proj));
            GV->editState->hPlaneData[GV->editState->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(proj);
            cProjectile *n = new cProjectile;
            n->hObj = proj;
            n->fAnimTimer = 0;
            cAniBankAsset *ani = GV->editState->hAniBank->GetAssetByID(vProjSpawner[i].hObj->GetAnim());
            n->bPhase = ani == NULL;
            n->fAnimTimer = 0;
            vProjSpawner[i].vhChildren.push_back(n);
        }
    }
    for (int i = 0; i < vEyeCandies.size(); i++) {
        vEyeCandies[i].fCounter += hge->Timer_GetDelta();
        while (vEyeCandies[i].fCounter > SIM_EYECANDY_DELAY) {
            vEyeCandies[i].fCounter -= SIM_EYECANDY_DELAY;
            vEyeCandies[i].iFrame++;
            if (vEyeCandies[i].iFrame >=
                mO->SprBank->GetAssetByID(vEyeCandies[i].hObj->GetImageSet())->GetSpritesCount())
                vEyeCandies[i].iFrame -= mO->SprBank->GetAssetByID(
                        vEyeCandies[i].hObj->GetImageSet())->GetSpritesCount();
            GetUserDataFromObj(vEyeCandies[i].hObj)->SetI(vEyeCandies[i].iFrame + 1);
            //bChange = 1;
        }
    }
    for (int i = 0; i < vAmbients.size(); i++) {
        vAmbients[i].bPlaying = (vAmbients[i].hChan == NULL ? 0 : hge->Channel_IsPlaying(vAmbients[i].hChan));
        if (!vAmbients[i].bPlaying && vAmbients[i].hChan != NULL) {
            hge->Channel_Stop(vAmbients[i].hChan);
            vAmbients[i].hChan = NULL;
        }

        vAmbients[i].fTimer += hge->Timer_GetDelta();
        if (vAmbients[i].fTimer > vAmbients[i].fTime && vAmbients[i].fTime > 0) {
            if (vAmbients[i].fTime == 0)
                vAmbients[i].fTimer = 0;
            else
                vAmbients[i].fTimer -= vAmbients[i].fTime;
            vAmbients[i].bOn = !vAmbients[i].bOn;
            int newtimer = 0;
            if (vAmbients[i].bOn) {
                GV->Console->Printf("Ambient #~y~%d on ~w~[~y~%s~w~]", i, vAmbients[i].hObj->GetAnim());
                newtimer = hge->Random_Int(vAmbients[i].hObj->GetMoveRect().x1, vAmbients[i].hObj->GetMoveRect().y1);
            } else {
                GV->Console->Printf("Ambient #~y~%d off ~w~[~y~%s~w~]", i, vAmbients[i].hObj->GetAnim());
                newtimer = hge->Random_Int(vAmbients[i].hObj->GetMoveRect().x2, vAmbients[i].hObj->GetMoveRect().y2);
            }
            vAmbients[i].fTime = float(newtimer) / 1000.0f;
        }

        if (!vAmbients[i].bPlaying && vAmbients[i].bOn) {
            cSndBankAsset *as = GV->editState->hSndBank->GetAssetByID(vAmbients[i].hObj->GetAnim());
            if (as != NULL)
                vAmbients[i].hChan = hge->Effect_PlayEx(as->GetSound(),
                                                        (vAmbients[i].hObj->GetParam(WWD::Param_Damage) != 0
                                                         ? vAmbients[i].hObj->GetParam(WWD::Param_Damage) : 100));
        } else if (vAmbients[i].bPlaying && !vAmbients[i].bOn) {
            hge->Channel_Stop(vAmbients[i].hChan);
            vAmbients[i].hChan = NULL;
        }
    }
    for (int i = 0; i < vEnemies.size(); i++) {
        cEnemy *el = vEnemies[i];

        if (el->bHit) {
            el->fHitTimer += hge->Timer_GetDelta();
            if (el->fHitTimer * 1000 > el->hAniHit->GetFrame(el->iHitFrame)->GetDuration()) {
                el->fHitTimer -= float(el->hAniHit->GetFrame(el->iHitFrame)->GetDuration()) / 1000.0f;
                el->iHitFrame++;
                if (el->iHitFrame >= el->hAniHit->GetFramesCount()) {
                    el->bHit = 0;
                } else {
                    GetUserDataFromObj(el->hObj)->SetI(el->hAniHit->GetFrame(el->iHitFrame)->GetImageID());
                }
            }
            continue;
        }

        int posx = GetUserDataFromObj(el->hObj)->GetX();
        if (el->bMoving) {
            int movx = el->fMoveSpeed * hge->Timer_GetDelta() * (el->bDirection == 0 ? -1 : 1);
            posx += movx;
            bool bBorder = 0;
            if (el->bDirection && posx > el->iMaxX) {
                bBorder = 1;
                posx = el->iMaxX;
            } else if (!el->bDirection && posx < el->iMinX) {
                bBorder = 1;
                posx = el->iMinX;
            }
            GetUserDataFromObj(el->hObj)->SetX(posx);
            if (bBorder) {
                bool idle = hge->Random_Int(0, 2);
                if (idle == 0 || !el->bStopping) {
                    el->bDirection = !el->bDirection;
                    GetUserDataFromObj(el->hObj)->SetFlip(el->bDirection, 0);
                } else {
                    el->bMoving = 0;
                    el->fWaitTimer = 0;
                    el->iAniFrame = 0;
                    char anim[256];
                    sprintf(anim, "%s_%s", el->hObj->GetImageSet(),
                            el->vIdleAnims[hge->Random_Int(0, el->vIdleAnims.size() - 1)].c_str());
                    el->hAni = GV->editState->hAniBank->GetAssetByID(anim)->GetAni();
                }
            }
        } else if (!el->bNoWalk) {
            el->fWaitTimer += hge->Timer_GetDelta();
            if (el->fWaitTimer > 5) {
                el->bMoving = 1;
                el->fAniTimer = 0;
                el->bDirection = !el->bDirection;
                GetUserDataFromObj(el->hObj)->SetFlip(el->bDirection, 0);
                char anim[256];
                sprintf(anim, "%s_%s", el->hObj->GetImageSet(), el->strWalkAnim.c_str());
                el->hAni = GV->editState->hAniBank->GetAssetByID(anim)->GetAni();
                el->iAniFrame = 0;
            }
        }

        el->fAniTimer += hge->Timer_GetDelta() * 1000.0f;
        if (el->bMoving || el->hAni->GetFramesCount() > 1) {
            while (el->fAniTimer > el->hAni->GetFrame(el->iAniFrame)->GetDuration()) {
                el->fAniTimer -= el->hAni->GetFrame(el->iAniFrame)->GetDuration();
                el->iAniFrame++;
                if (el->iAniFrame == el->hAni->GetFramesCount()) {
                    if (el->bMoving) {
                        el->iAniFrame = 0;
                    } else {
                        char anim[256];
                        sprintf(anim, "%s_%s", el->hObj->GetImageSet(),
                                el->vIdleAnims[hge->Random_Int(0, el->vIdleAnims.size() - 1)].c_str());
                        el->hAni = GV->editState->hAniBank->GetAssetByID(anim)->GetAni();
                        el->iAniFrame = 0;
                    }
                }
                //bChange = 1;
            }
        } else {
            if (el->fAniTimer > 1000) {
                char anim[256];
                sprintf(anim, "%s_%s", el->hObj->GetImageSet(),
                        el->vIdleAnims[hge->Random_Int(0, el->vIdleAnims.size() - 1)].c_str());
                el->hAni = GV->editState->hAniBank->GetAssetByID(anim)->GetAni();
                el->iAniFrame = 0;
                el->fAniTimer -= 1000;
            }
        }
        GetUserDataFromObj(el->hObj)->SetI(el->hAni->GetFrame(el->iAniFrame)->GetImageID());
    }
    for (int i = 0; i < vPElevators.size(); i++) {
        cPathElevator *n = &vPElevators[i];
        if (n->vPaths[n->iStep].first == 5) {
            n->fSleep += hge->Timer_GetDelta() * 1000;
        } else {
            n->fPosX += n->fSpeed * hge->Timer_GetDelta() * n->iDirHor;
            n->fPosY += n->fSpeed * hge->Timer_GetDelta() * n->iDirVer;
        }

        if (n->vPaths[n->iStep].first == 5 && n->fSleep >= n->vPaths[n->iStep].second ||
            abs(n->fPosX - n->fOrigX) > n->vPaths[n->iStep].second ||
            abs(n->fPosY - n->fOrigY) > n->vPaths[n->iStep].second) {
            //int olddist = n->vPaths[n->iStep].second;
            //int postx = abs(n->fPosX-n->fOrigX)-olddist, posty = abs(n->fPosY-n->fOrigY)-olddist;
            //int postx = 0, posty = 0;
            n->fOrigX += n->vPaths[n->iStep].second * n->iDirHor;
            n->fOrigY += n->vPaths[n->iStep].second * n->iDirVer;

            n->iStep++;
            if (n->iStep >= n->vPaths.size()) {
                n->iStep = 0;
                n->fOrigX = n->hObj->GetParam(WWD::Param_LocationX);
                n->fOrigY = n->hObj->GetParam(WWD::Param_LocationY);
                n->fPosX = n->fOrigX;
                n->fPosY = n->fOrigY;
            }

            if (n->vPaths[n->iStep].first == 5) {
                n->iDirHor = n->iDirVer = 0;
            } else {
                if (n->vPaths[n->iStep].first >= 7) n->iDirVer = -1;
                else if (n->vPaths[n->iStep].first >= 4) n->iDirVer = 0;
                else if (n->vPaths[n->iStep].first >= 1) n->iDirVer = 1;

                if (n->vPaths[n->iStep].first == 7 || n->vPaths[n->iStep].first == 4 ||
                    n->vPaths[n->iStep].first == 1)
                    n->iDirHor = -1;
                else if (n->vPaths[n->iStep].first == 8 || n->vPaths[n->iStep].first == 2) n->iDirHor = 0;
                else if (n->vPaths[n->iStep].first == 9 || n->vPaths[n->iStep].first == 6 ||
                         n->vPaths[n->iStep].first == 3)
                    n->iDirHor = 1;
            }
            n->fSleep = 0;
            //n->fPosX = n->fOrigX+postx*n->iDirHor;
            //n->fPosY = n->fOrigY+posty*n->iDirVer;
        }
        GetUserDataFromObj(n->hObj)->SetPos(n->fPosX, n->fPosY);
    }
    for (int i = 0; i < vElevators.size(); i++) {
        cElevator *el = vElevators[i];
#ifdef REAL_SIM
        if( bPlay ){
        if( el->bodyMe->GetX() > GV->editState->fCamX-500 && el->bodyMe->GetX() < GV->editState->fCamX+GV->editState->vPort->GetWidth()+500 &&
            el->bodyMe->GetY() > GV->editState->fCamY-500 && el->bodyMe->GetY() < GV->editState->fCamY+GV->editState->vPort->GetHeight()+500 ){
        if( el->iMinX != 0 && el->iMaxX != 0 )
         if( el->bDirHor ){
          el->bodyMe->SetSpeedX(el->fSpeedHor);
          if( el->bodyMe->GetX() > el->iMaxX ){
           el->bDirHor = 0;
           el->bodyMe->SetX(el->iMaxX-el->bodyMe->GetWidth());
          }
         }else{
          el->bodyMe->SetSpeedX(-el->fSpeedHor);
          if( el->bodyMe->GetX() < el->iMinX ){
           el->bDirHor = 1;
           el->bodyMe->SetX(el->iMinX);
          }
         }
        if( el->iMinY != 0 && el->iMaxY != 0 )
         if( el->bDirVer ){
          el->bodyMe->SetSpeedY(el->fSpeedVer);
          if( el->bodyMe->GetY() > el->iMaxY ){
           el->bDirVer = 0;
           el->bodyMe->SetY(el->iMaxY-el->bodyMe->GetHeight());
          }
         }else{
          el->bodyMe->SetSpeedY(-el->fSpeedVer);
          if( el->bodyMe->GetY() < el->iMinY ){
           el->bDirVer = 1;
           el->bodyMe->SetY(el->iMinY);
          }
         }
         }else{
          el->bodyMe->SetSpeed(0,0);
         }
        }else{
#endif
        if (el->bSleep) continue;
        if (el->iMinX != 0 && el->iMaxX != 0)
            if (el->bDirHor) {
                el->fPosX += el->fSpeedHor * hge->Timer_GetDelta();
                if (el->fPosX > el->iMaxX) {
                    el->bDirHor = 0;
                    el->fPosX -= el->iMaxX - el->fPosX;
                    if (el->bOneWay) {
                        el->bSleep = 1;
                        continue;
                    }
                }
            } else {
                el->fPosX -= el->fSpeedHor * hge->Timer_GetDelta();
                if (el->fPosX < el->iMinX) {
                    el->bDirHor = 1;
                    el->fPosX += el->iMinX - el->fPosX;
                    if (el->bOneWay) {
                        el->bSleep = 1;
                        continue;
                    }
                }
            }
        if (el->iMinY != 0 && el->iMaxY != 0)
            if (el->bDirVer) {
                el->fPosY += el->fSpeedHor * hge->Timer_GetDelta();
                if (el->fPosY > el->iMaxY) {
                    el->bDirVer = 0;
                    el->fPosY -= el->iMaxY - el->fPosY;
                    if (el->bOneWay) {
                        el->bSleep = 1;
                        continue;
                    }
                }
            } else {
                el->fPosY -= el->fSpeedHor * hge->Timer_GetDelta();
                if (el->fPosY < el->iMinY) {
                    el->bDirVer = 1;
                    el->fPosY += el->iMinY - el->fPosY;
                    if (el->bOneWay) {
                        el->bSleep = 1;
                        continue;
                    }
                }
            }
        GetUserDataFromObj(el->hObj)->SetPos(el->fPosX, el->fPosY);
#ifdef REAL_SIM
        }
#endif
    }
    //if( bChange )
    mO->vPort->MarkToRedraw();
}

int cGameSimulator::GetTileColX(WWD::TileAtrib *atr, bool bRight) {
    if (!atr) return -1;
    if (atr->GetType() == WWD::AtribType_Single ||
        atr->GetMask().x1 == 0 && atr->GetMask().y1 == 0 && atr->GetMask().x2 == 63 && atr->GetMask().y2 == 63) {
        if (atr->GetAtribInside() == WWD::Atrib_Ground || atr->GetAtribInside() == WWD::Atrib_Solid) {
            return 0;
        } else {
            return -1;
        }
    }
    if (atr->GetMask().x1 == atr->GetMask().x2 || atr->GetMask().y1 == atr->GetMask().y2) {
        if (atr->GetAtribOutside() == WWD::Atrib_Ground || atr->GetAtribOutside() == WWD::Atrib_Solid) {
            return 0;
        } else
            return -1;
    }
    int insidey = GV->editState->GetActivePlane()->GetTileWidth(),
            outsidey = GV->editState->GetActivePlane()->GetTileWidth();
    if (atr->GetAtribInside() == WWD::Atrib_Ground || atr->GetAtribInside() == WWD::Atrib_Solid) {
        if (bRight) {
            insidey = GV->editState->GetActivePlane()->GetTileWidth() - 1 - atr->GetMask().x2;
        } else {
            insidey = atr->GetMask().x1;
        }
    }
    if (atr->GetAtribOutside() == WWD::Atrib_Ground || atr->GetAtribOutside() == WWD::Atrib_Solid) {
        if (atr->GetMask().y1 != 0 || atr->GetMask().y2 != outsidey - 1)
            outsidey = 0;
    }
    if (insidey != GV->editState->GetActivePlane()->GetTileWidth() ||
        outsidey != GV->editState->GetActivePlane()->GetTileWidth()) {
        return (insidey < outsidey ? insidey : outsidey);
    }
    return -1;
}

int cGameSimulator::GetTileColY(WWD::TileAtrib *atr) {
    if (!atr) return -1;
    if (atr->GetType() == WWD::AtribType_Single ||
        atr->GetMask().x1 == 0 && atr->GetMask().y1 == 0 && atr->GetMask().x2 == 63 && atr->GetMask().y2 == 63) {
        if (atr->GetAtribInside() == WWD::Atrib_Ground || atr->GetAtribInside() == WWD::Atrib_Solid) {
            return 0;
        } else
            return -1;
    }
    if (atr->GetMask().x1 == atr->GetMask().x2 || atr->GetMask().y1 == atr->GetMask().y2) {
        if (atr->GetAtribOutside() == WWD::Atrib_Ground || atr->GetAtribOutside() == WWD::Atrib_Solid) {
            return 0;
        } else
            return -1;
    }
    int insidey = GV->editState->GetActivePlane()->GetTileHeight(),
            outsidey = GV->editState->GetActivePlane()->GetTileHeight();
    if (atr->GetAtribInside() == WWD::Atrib_Ground || atr->GetAtribInside() == WWD::Atrib_Solid) {
        insidey = atr->GetMask().y1;
    }
    if (atr->GetAtribOutside() == WWD::Atrib_Ground || atr->GetAtribOutside() == WWD::Atrib_Solid) {
        if (atr->GetMask().x1 != 0 || atr->GetMask().x2 != outsidey - 1)
            outsidey = 0;
    }
    if (insidey != GV->editState->GetActivePlane()->GetTileHeight() ||
        outsidey != GV->editState->GetActivePlane()->GetTileHeight()) {
        return (insidey < outsidey ? insidey : outsidey);
    }
    return -1;
}
