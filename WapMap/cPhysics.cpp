#include "cPhysics.h"
#include "globals.h"
#include "states/editing_ww.h"
#include "bodyQuadTree.h"
#include "cObjectUserData.h"

extern HGE * hge;

cPhysicWorld::cPhysicWorld(WWD::Plane * hpPl)
{
    hPl = hpPl;
    hQT = new cBodyQuadTree(hPl);
}

void cPhysicWorld::Update()
{
    for(int i=0;i<vhActiveBodies.size();i++){
     vhActiveBodies[i]->Update();
     if( vhActiveBodies[i]->GetSpeedX() == 0 && vhActiveBodies[i]->GetSpeedY() == 0 && !vhActiveBodies[i]->SleepBanned() ){
      vhActiveBodies[i]->Sleep();
      vhActiveBodies.erase(vhActiveBodies.begin()+i);
      i--;
     }
    }
}

void cPhysicWorld::CompileTiles()
{
    bool assigned[hPl->GetPlaneWidth()][hPl->GetPlaneHeight()];
    for(int y=0;y<hPl->GetPlaneHeight();y++)
     for(int x=0;x<hPl->GetPlaneWidth();x++){
      assigned[x][y] = 0;
     }

    for(int y=0;y<hPl->GetPlaneHeight();y++)
     for(int x=0;x<hPl->GetPlaneWidth();x++){
      if( assigned[x][y] ) continue;
      if( hPl->GetTile(x,y)->GetID() != -1 ){
       WWD::TileAtrib * ta = GV->editState->hParser->GetTileAtribs(hPl->GetTile(x,y)->GetID());

       if( ta->GetType() == WWD::AtribType_Single ){
        if( ta->GetAtribInside() == WWD::Atrib_Clear )
         continue;
        int width = 1, height = 1;
        for(int x2=x+1;x2<hPl->GetPlaneWidth();x2++){
         WWD::TileAtrib * ta2 = GV->editState->hParser->GetTileAtribs(hPl->GetTile(x2,y)->GetID());
         if( !assigned[x2][y] && ta2->GetType() == WWD::AtribType_Single && ta2->GetAtribInside() == ta->GetAtribInside() ){
          width++;
          assigned[x2][y] = 1;
         }else
          break;
        }
        for(int y2=y+1;y2<hPl->GetPlaneHeight();y2++){
         bool rowOk = 1;
         for(int x2=x;x2<x+width;x2++){
          WWD::TileAtrib * ta2 = GV->editState->hParser->GetTileAtribs(hPl->GetTile(x2,y2)->GetID());
          if( assigned[x2][y2] || !(ta2->GetType() == WWD::AtribType_Single && ta2->GetAtribInside() == ta->GetAtribInside()) ){
           rowOk = 0;
           break;
          }
         }
         if( !rowOk ) break;
         height++;
         for(int x2=x;x2<x+width;x2++)
          assigned[x2][y2] = 1;
        }

        new cPhysicBody(this, x*hPl->GetTileWidth(), y*hPl->GetTileHeight(), hPl->GetTileWidth()*width, hPl->GetTileHeight()*height, ta->GetAtribInside());
       }else if( ta->GetType() == WWD::AtribType_Double ){
        if( ta->GetAtribInside() == WWD::Atrib_Clear && ta->GetAtribOutside() == WWD::Atrib_Clear )
         continue;
        if( ta->GetAtribInside() != WWD::Atrib_Clear ){
         int maskw = ta->GetMask().x2 - ta->GetMask().x1, maskh = ta->GetMask().y2 - ta->GetMask().y1;
         int width = 0, height = 0;
         if( maskw == 63 && ta->GetAtribOutside() == WWD::Atrib_Clear ){
          for(int x2=x+1;x2<hPl->GetPlaneWidth();x2++)
           if( hPl->GetTile(x2,y)->GetID() == hPl->GetTile(x,y)->GetID() && !assigned[x2][y] ){
            width++;
            assigned[x2][y] = 1;
           }else
            break;
         }
         if( maskh == 63 && ta->GetAtribOutside() == WWD::Atrib_Clear && width == 0 ){
          for(int y2=y+1;y2<hPl->GetPlaneHeight();y2++)
           if( hPl->GetTile(x,y2)->GetID() == hPl->GetTile(x,y)->GetID() && !assigned[x][y2] ){
            height++;
            assigned[x][y2] = 1;
           }else
            break;
         }

         width = width*64+maskw;
         height = height*64+maskh;
         new cPhysicBody(this, x*hPl->GetTileWidth()+ta->GetMask().x1, y*hPl->GetTileHeight()+ta->GetMask().y1, width, height, ta->GetAtribInside());
        }
        if( ta->GetAtribOutside() != WWD::Atrib_Clear ){
         if( ta->GetMask().y1 != 0 )
          new cPhysicBody(this, x*hPl->GetTileWidth(), y*hPl->GetTileHeight(), 64, ta->GetMask().y1, ta->GetAtribOutside());
         if( ta->GetMask().y2 != 63 )
          new cPhysicBody(this, x*hPl->GetTileWidth(), y*hPl->GetTileHeight()+ta->GetMask().y2, 64, 64-ta->GetMask().y2, ta->GetAtribOutside());
         if( ta->GetMask().x1 != 0 )
          new cPhysicBody(this, x*hPl->GetTileWidth(), y*hPl->GetTileHeight(), ta->GetMask().x1, ta->GetMask().y2, ta->GetAtribOutside());
         if( ta->GetMask().x2 != 63 )
          new cPhysicBody(this, x*hPl->GetTileWidth()+ta->GetMask().x2, y*hPl->GetTileHeight(), 64-ta->GetMask().x2, ta->GetMask().y2, ta->GetAtribOutside());
        }
       }
      }
     }
}

cPhysicWorld::~cPhysicWorld()
{
    delete hQT;
}

void cPhysicWorld::RegisterBody(cPhysicBody * body)
{
    hQT->UpdateBody(body);
}

void cPhysicWorld::NotifyDeletion(cPhysicBody * body)
{

}

void cPhysicWorld::BodyWakeUpNotify(cPhysicBody * body)
{
    vhActiveBodies.push_back(body);
}

void cPhysicBody::ClearCellReferences()
{
    for(int i=0;i<m_vhCells.size();i++)
     m_vhCells[i]->DeleteBody(this);
    m_vhCells.clear();
}

cPhysicBody::cPhysicBody(cPhysicWorld * w, float x1, float y1, float width, float height, WWD::Object * obj, enBodyType it)
{
    fX = x1;
    fY = y1;
    fW = width;
    fH = height;
    w->RegisterBody(this);
    hOwner = w;
    if( it == BODY_ELEVATOR )
     iAtrib = WWD::Atrib_Ground;
    else
     iAtrib = WWD::Atrib_Clear;
    hObj = obj;
    bSleep = 1;
    bGlueObject = 0;
    iObjOffX = iObjOffY = 0;
    fSpeedX = fSpeedY = 0;
    bFly = 0;
    WakeUp();
    bEnableGravity = 1;
    iType = it;
    bHidden = 0;
}

void cPhysicBody::WakeUp()
{
    if( !bSleep ) return;
    bSleep = 0;
    hOwner->BodyWakeUpNotify(this);
}

cPhysicBody::cPhysicBody(cPhysicWorld * w, float x1, float y1, float width, float height, WWD::TILE_ATRIB atr)
{
    fX = x1;
    fY = y1;
    fW = width;
    fH = height;
    w->RegisterBody(this);
    hOwner = w;
    iAtrib = atr;
    bSleep = 1;
    hObj = NULL;
    bGlueObject = 0;
    bFly = 0;
    fSpeedX = fSpeedY = 0;
    bEnableGravity = 0;
    iType = BODY_TILE;
    bHidden = 0;
}


cPhysicBody::cPhysicBody(cPhysicWorld * w, float x1, float y1, float width, float height, void (*pcolCB)(void * hUserData, cPhysicBody * trigger, cPhysicBody * body), void * phUD, WWD::Object * obj) //trigger
{
    fX = x1;
    fY = y1;
    fW = width;
    fH = height;
    w->RegisterBody(this);
    hOwner = w;
    iAtrib = WWD::Atrib_Clear;
    hObj = obj;
    bSleep = 1;
    bGlueObject = 0;
    bFly = 0;
    fSpeedX = fSpeedY = 0;
    bEnableGravity = 0;
    iType = BODY_TRIGGER;
    colCB = pcolCB;
    hUD = phUD;
    bHidden = 0;
}

void cPhysicBody::Update()
{
    if( bSleep ) return;

    if( bFly && bEnableGravity ){
     fSpeedY += PHYS_GRAVITY*hge->Timer_GetDelta();
    }
    if( fSpeedX != 0 || fSpeedY != 0 ){
     float fBaseX = fX, fBaseY = fY;
     fX += fSpeedX*hge->Timer_GetDelta();
     fY += fSpeedY*hge->Timer_GetDelta();

     hOwner->hQT->UpdateBody(this);

     if( iType == BODY_ELEVATOR ){
      for(int i=0;i<vhCols.size();i++){
       if( vhCols[i]->GetType() == BODY_PLAYER ){
        if( vhCols[i]->GetY() < fY ){
         vhCols[i]->SetPos(vhCols[i]->GetX()+fSpeedX*hge->Timer_GetDelta(), vhCols[i]->GetY()+fSpeedY*hge->Timer_GetDelta());
        }
       }
      }
     }

     std::vector<cPhysicBody*> col = hOwner->hQT->GetBodiesByArea(fX, fY, fW, fH, 0);
     if( col.size() != 0 ){
      for(int i=0;i<col.size();i++){

       if( col[i]->GetType() == BODY_TRIGGER ){
        bool bAlreadyTrig = 0;
        for(int y=0;y<vhCols.size();y++)
         if( vhCols[y] == col[i] ){
          bAlreadyTrig = 1;
          break;
         }
        if( !bAlreadyTrig )
         col[i]->Triggered(this);
        continue;
       }

       if( col[i]->GetAtrib() != WWD::Atrib_Climb && col[i]->GetAtrib() != WWD::Atrib_Ground )
        if( fSpeedX > 0 && col[i]->GetX() > fX && abs(col[i]->GetY()-(fY+fH)) > 3 ){
         fX = col[i]->GetX()-fW;
         fSpeedX = 0;
        }else if( fSpeedX < 0 && col[i]->GetX() < fX && abs(col[i]->GetY()-(fY+fH)) > 3 ){
         fX = col[i]->GetX()+col[i]->GetWidth();
         fSpeedX = 0;
        }

       if( fSpeedY > 0 && col[i]->GetY() > fY ){
        if( col[i]->GetAtrib() == WWD::Atrib_Ground || col[i]->GetAtrib() == WWD::Atrib_Climb ){
         if( fBaseY+fH < col[i]->GetY() ){
          fY = col[i]->GetY()-fH;
          fSpeedY = 0;
         }
        }else{
         fY = col[i]->GetY()-fH;
         fSpeedY = 0;
        }
       }else if( fSpeedY < 0 && col[i]->GetY() < fY && col[i]->GetAtrib() != WWD::Atrib_Climb && col[i]->GetAtrib() != WWD::Atrib_Ground ){
        fY = col[i]->GetY()+col[i]->GetHeight();
        fSpeedY = 0;
       }

      }
     }
    }

    bFly = 1;
    std::vector<cPhysicBody*> under = hOwner->hQT->GetBodiesByArea(fX, fY+fH, fW, 2, 0);
    for(int i=0;i<under.size();i++){
     if( under[i]->GetAtrib() == WWD::Atrib_Ground || under[i]->GetAtrib() == WWD::Atrib_Solid || under[i]->GetAtrib() == WWD::Atrib_Climb ){
      if( under[i]->GetY() >= fY+fH ){
       bFly = 0;
       break;
      }
     }
    }
    vhCols = hOwner->hQT->GetBodiesByArea(fX, fY, fW, fH, 0);

    if( hObj != NULL ){
     if( bGlueObject ){
      GetUserDataFromObj(hObj)->SetPos(fX+fW/2+iObjOffX*(GetUserDataFromObj(hObj)->GetFlipX() ? -1 : 1), fY+fH/2+iObjOffY);
     }
    }
}

cPhysicBody::~cPhysicBody()
{
    ClearCellReferences();
    hOwner->NotifyDeletion(this);
}

void cPhysicBody::Triggered(cPhysicBody * b)
{
    if( colCB != NULL )
     (colCB)(hUD, this, b);
}

void cPhysicBody::SetPos(float x, float y)
{
    if( fX == x && fY == y ) return;
    fX = x;
    fY = y;
    hOwner->hQT->UpdateBody(this);
}

void cPhysicBody::SetX(float x)
{
    if( fX == x ) return;
    fX = x;
    hOwner->hQT->UpdateBody(this);
}

void cPhysicBody::SetY(float y)
{
    if( fY == y ) return;
    fY = y;
    hOwner->hQT->UpdateBody(this);
}
