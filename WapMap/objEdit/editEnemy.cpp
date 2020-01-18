#include "editEnemy.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include "../databanks/imageSets.h"

#define DEFAULT_ENEMY(x,y) (std::pair<std::string,std::string>(x, y))

extern HGE * hge;

namespace ObjEdit
{
 cEditObjEnemy::cEditObjEnemy(WWD::Object * obj, State::EditingWW * st) : cObjEdit(obj, st)
 {
    int baselvl = GV->editState->hParser->GetBaseLevel();
    if( baselvl == 1 || baselvl == 2 ){
     vstrpTypes.push_back(DEFAULT_ENEMY("Officer", "LEVEL_OFFICER"));
     vstrpTypes.push_back(DEFAULT_ENEMY("Soldier", "LEVEL_SOLDIER"));
     if( baselvl == 1 )
      vstrpTypes.push_back(DEFAULT_ENEMY("Rat", "LEVEL_RAT"));
     else
      vstrpTypes.push_back(DEFAULT_ENEMY("Raux", "LEVEL_RAUX"));
    }else if( baselvl == 3 || baselvl == 4 ){
     vstrpTypes.push_back(DEFAULT_ENEMY("RobberThief", "LEVEL_ROBBERTHIEF"));
     vstrpTypes.push_back(DEFAULT_ENEMY("CutThroat", "LEVEL_CUTTHROAT"));
     vstrpTypes.push_back(DEFAULT_ENEMY("Rat", "LEVEL_RAT"));
     if( baselvl == 4 )
      vstrpTypes.push_back(DEFAULT_ENEMY("Katherine", "LEVEL_KATHERINE"));
    }else if( baselvl == 5 || baselvl == 6 ){
     vstrpTypes.push_back(DEFAULT_ENEMY("TownGuard1", "LEVEL_TOWNGUARD1"));
     vstrpTypes.push_back(DEFAULT_ENEMY("TownGuard2", "LEVEL_TOWNGUARD2"));
     vstrpTypes.push_back(DEFAULT_ENEMY("Seagull", "LEVEL_SEAGULL"));
     if( baselvl == 6 ){
      vstrpTypes.push_back(DEFAULT_ENEMY("Rat", "LEVEL_RAT"));
      vstrpTypes.push_back(DEFAULT_ENEMY("Wolvington", "LEVEL_WOLVINGTON"));
     }
    }else if( baselvl == 7 || baselvl == 8 ){
     vstrpTypes.push_back(DEFAULT_ENEMY("RedTailPirate", "LEVEL_REDTAILPIRATE"));
     vstrpTypes.push_back(DEFAULT_ENEMY("BearSailor", "LEVEL_BEARSAILOR"));
     vstrpTypes.push_back(DEFAULT_ENEMY("Seagull", "LEVEL_SEAGULL"));
     if( baselvl == 7 )
      vstrpTypes.push_back(DEFAULT_ENEMY("HermitCrab", "LEVEL_HERMITCRAB"));
     else{
      vstrpTypes.push_back(DEFAULT_ENEMY("Gabriel", "LEVEL_GABRIEL"));
     }
    }else if( baselvl == 9 || baselvl == 10 ){
     vstrpTypes.push_back(DEFAULT_ENEMY("PegLeg", "LEVEL_PEGLEG"));
     vstrpTypes.push_back(DEFAULT_ENEMY("CrazyHook", "LEVEL_CRAZYHOOK"));
     vstrpTypes.push_back(DEFAULT_ENEMY("Seagull", "LEVEL_SEAGULL"));
     if( baselvl == 10 ){
      vstrpTypes.push_back(DEFAULT_ENEMY("Marrow", "LEVEL_MARROW"));
     }
    }else if( baselvl == 11 || baselvl == 12 ){
     vstrpTypes.push_back(DEFAULT_ENEMY("Mercat", "LEVEL_MERCAT"));
     vstrpTypes.push_back(DEFAULT_ENEMY("Siren", "LEVEL_SIREN"));
     vstrpTypes.push_back(DEFAULT_ENEMY("Fish", "LEVEL_FISH"));
     if( baselvl == 12 ){
      vstrpTypes.push_back(DEFAULT_ENEMY("Aquatis", "LEVEL_KINGAQUATIS"));
     }
    }else if( baselvl == 13 ){
     vstrpTypes.push_back(DEFAULT_ENEMY("RedTailPirate", "LEVEL_REDTAILPIRATE"));
     vstrpTypes.push_back(DEFAULT_ENEMY("BearSailor", "LEVEL_BEARSAILOR"));
     vstrpTypes.push_back(DEFAULT_ENEMY("Chameleon", "LEVEL_CHAMELEON"));
     vstrpTypes.push_back(DEFAULT_ENEMY("RedTail", "LEVEL_REDTAIL"));
    }else if( baselvl == 14 ){
     vstrpTypes.push_back(DEFAULT_ENEMY("TigerGuard", "LEVEL_TIGER"));
     vstrpTypes.push_back(DEFAULT_ENEMY("Chameleon", "LEVEL_CHAMELEON"));
     vstrpTypes.push_back(DEFAULT_ENEMY("Omar", "LEVEL_OMAR"));
    }

    iType = ObjEdit::enEnemy;
    win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Enemy", "WinCaption"));
    win->setDimension(gcn::Rectangle(0, 0, 300, 185));
    win->setClose(1);
    win->addActionListener(hAL);
    win->add(vpAdv);
    win->setMovable(0);
    st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY()+st->vPort->GetHeight()-win->getHeight());

    for(int i=0;i<5;i++)
     rbType[i] = NULL;

    labType = new SHR::Lab(GETL2S("EditObj_Enemy", "Type"));
    labType->adjustSize();
    win->add(labType, 5, 10);

    char tmp[128];
    sprintf(tmp, "a_%p", this);
    for(int i=0;i<vstrpTypes.size();i++){
     char tmp2[128];
     sprintf(tmp2, "Logic_%s", vstrpTypes[i].first.c_str());
     rbType[i] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Enemy", tmp2), tmp, !strcmp(hTempObj->GetLogic(), vstrpTypes[i].first.c_str()));
     rbType[i]->adjustSize();
     rbType[i]->addActionListener(hAL);
     win->add(rbType[i], 5, 30+i*20);
    }

    labType2 = new SHR::Lab(GETL2S("EditObj_Enemy", "Variety"));
    labType2->adjustSize();
    win->add(labType2, 155, 30);

    sprintf(tmp, "b_%p", this);
    for(int i=0;i<2;i++){
     const char * cap;
     if( i )
      cap = GETL2S("EditObj_Enemy", (baselvl == 7 ? "Variety_Bomber" : "Variety_White"));
     else
      cap = GETL2S("EditObj_Enemy", "Variety_Normal");
     rbType2[i] = new SHR::RadBut(GV->hGfxInterface, cap, tmp, 0);
     rbType2[i]->adjustSize();
     rbType2[i]->addActionListener(hAL);
     win->add(rbType2[i], 155, 50+i*20);
    }
    rbType2[0]->setSelected(1);

    int yoffset = 40+vstrpTypes.size()*20;

    labBehaviour = new SHR::Lab(GETL2S("EditObj_Enemy", "Behaviour"));
    labBehaviour->adjustSize();
    win->add(labBehaviour, 5, yoffset);

    sprintf(tmp, "c_%p", this);
    for(int i=0;i<4;i++){
     char tmp2[128];
     sprintf(tmp2, "Behaviour_%d", i+1);
     rbFlags[i] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Enemy", tmp2), tmp, 0);
     rbFlags[i]->adjustSize();
     win->add(rbFlags[i], 5, yoffset+20+i*20);
    }
    yoffset += 100;

    cbPatrol = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Enemy", "Patrol"));
    cbPatrol->adjustSize();
    cbPatrol->addActionListener(hAL);
    cbPatrol->setSelected(hTempObj->GetUserValue(0)==0);
    win->add(cbPatrol, 5, yoffset);
    yoffset += 30;

    if( hTempObj->GetDynamicFlags() & WWD::Flag_dy_NoHit ){
     rbFlags[2]->setSelected(1);
    }else if( hTempObj->GetDynamicFlags() & WWD::Flag_dy_Safe ){
     rbFlags[1]->setSelected(1);
    }else if( hTempObj->GetDynamicFlags() & WWD::Flag_dy_AutoHitDamage ){
     rbFlags[3]->setSelected(1);
    }else{
     rbFlags[0]->setSelected(1);
    }

    if( baselvl == 7 && rbType[2]->isSelected() ){
     rbType2[hTempObj->GetUserValue(0)>0]->setSelected(1);
    }else if( baselvl == 14 && rbType[0]->isSelected() ){
     rbType2[hTempObj->GetParam(WWD::Param_Smarts)==1]->setSelected(1);
    }

    labTreasures = new SHR::Lab(GETL2S("EditObj_Enemy", "Treasures"));
    labTreasures->adjustSize();
    win->add(labTreasures, 140, yoffset);
    for(int y=0;y<3;y++)
     for(int x=0;x<3;x++){
      invTabs[y*3+x] = new SHR::InvTab(&GV->gcnParts);
      invTabs[y*3+x]->adjustSize();
      invTabs[y*3+x]->addActionListener(hAL);
      win->add(invTabs[y*3+x], 140+x*50, yoffset+y*50);
     }
    int taboff = 0;
    if( hTempObj->GetParam(WWD::Param_Powerup) > 0 ){
     cInventoryItem it = GV->editState->hInvCtrl->GetItemByID(hTempObj->GetParam(WWD::Param_Powerup));
     if( it.second != -1 ){
      invTabs[taboff]->SetItem(it);
      taboff++;
     }
    }
    for(int i=0;i<2;i++){
     WWD::Rect r = hTempObj->GetUserRect(i);
     cInventoryItem it = GV->editState->hInvCtrl->GetItemByID(r.x1);
     if( it.second != -1 ){
      invTabs[taboff]->SetItem(it);
      taboff++;
     }
     it = GV->editState->hInvCtrl->GetItemByID(r.y1);
     if( it.second != -1 ){
      invTabs[taboff]->SetItem(it);
      taboff++;
     }
     it = GV->editState->hInvCtrl->GetItemByID(r.x2);
     if( it.second != -1 ){
      invTabs[taboff]->SetItem(it);
      taboff++;
     }
     it = GV->editState->hInvCtrl->GetItemByID(r.y2);
     if( it.second != -1 ){
      invTabs[taboff]->SetItem(it);
      taboff++;
     }
    }

    labMoveArea = new SHR::Lab(GETL2S("EditObj_Enemy", "MoveArea"));
    labMoveArea->adjustSize();
    win->add(labMoveArea, 5, yoffset);

    hRectPick = new cProcPickRect(hTempObj);
    hRectPick->SetActionListener(hAL);
    hRectPick->SetType(PickRect_MinMax);
    hRectPick->AddWidgets(win, 5, yoffset+20);
    hRectPick->SetAllowEmpty(1);
    hRectPick->SetAllowEmptyAxis(1);
    hRectPick->GetPickButton()->addActionListener(hAL);

    butClearArea = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Enemy", "Clear"));
    butClearArea->setDimension(gcn::Rectangle(0, 0, 125, 33));
    butClearArea->addActionListener(hAL);
    win->add(butClearArea, 5, yoffset+20+88+33);

    labDamage = new SHR::Lab(GETL2S("EditObj_Enemy", "Damage"));
    win->add(labDamage, 5, yoffset+20+88+70+4);

    sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_Damage));
    tfDamage = new SHR::TextField(tmp);
    tfDamage->setDimension(gcn::Rectangle(0,0, 150, 20));
    tfDamage->SetNumerical(1, 0);
    win->add(tfDamage, 140, yoffset+20+88+70+4);

    labWarpDest = new SHR::Lab("");
    labWarpDest->adjustSize();
    win->add(labWarpDest, 5, yoffset+20+88+70+4);

    sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedX));
    tfSpeedX = new SHR::TextField(tmp);
    tfSpeedX->setDimension(gcn::Rectangle(0,0, 100, 20));
    tfSpeedX->SetNumerical(1, 0);
    win->add(tfSpeedX, 25, yoffset+20+88+70+4+20);

    sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_SpeedY));
    tfSpeedY = new SHR::TextField(tmp);
    tfSpeedY->setDimension(gcn::Rectangle(0,0, 100, 20));
    tfSpeedY->SetNumerical(1, 0);
    win->add(tfSpeedY, 25, yoffset+20+88+70+4+40);

    butPickSpeedXY = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_Enemy", "Pick"));
    butPickSpeedXY->setDimension(gcn::Rectangle(0, 0, 125, 33));
    butPickSpeedXY->addActionListener(hAL);
    win->add(butPickSpeedXY, 150, yoffset+20+88+70+4+14);

    win->add(_butAddNext, 50, 15);
    win->add(_butSave, 150, 15);

    hInventory = new cInvPickbox();
    hInventory->SetPosition(hState->vPort->GetX()+hState->vPort->GetWidth()-hInventory->GetWidth(),
                            hState->vPort->GetY()+hState->vPort->GetHeight()-hInventory->GetHeight());

    bPickSpeedXY = 0;

    RebuildWindow();
 }

 cEditObjEnemy::~cEditObjEnemy()
 {
    delete butPickSpeedXY;
    delete labDamage;
    delete tfDamage;
    delete labWarpDest;
    delete tfSpeedX;
    delete tfSpeedY;
    delete butClearArea;
    delete labMoveArea;
    delete hRectPick;
    delete hInventory;
    delete cbPatrol;
    delete labBehaviour;
    for(int i=0;i<4;i++)
     delete rbFlags[i];
    delete labType2;
    for(int i=0;i<5;i++)
     delete rbType[i];
    for(int i=0;i<2;i++)
     delete rbType2[i];
    delete labType;
    delete win;
 }

 void cEditObjEnemy::ApplyInventoryToObject()
 {
     if( !invTabs[0]->isVisible() || invTabs[0]->GetItem().second == -1 )
      hTempObj->SetParam(WWD::Param_Powerup, 0);
     else
      hTempObj->SetParam(WWD::Param_Powerup, invTabs[0]->GetItem().second);
     for(int i=0;i<2;i++){
      WWD::Rect r;
      if( !invTabs[i*4+1]->isVisible() ||invTabs[i*4+1]->GetItem().second == -1 ) r.x1 = 0;
      else r.x1 = invTabs[i*4+1]->GetItem().second;
      if( !invTabs[i*4+2]->isVisible() ||invTabs[i*4+2]->GetItem().second == -1 ) r.y1 = 0;
      else r.y1 = invTabs[i*4+2]->GetItem().second;
      if( !invTabs[i*4+3]->isVisible() ||invTabs[i*4+3]->GetItem().second == -1 ) r.x2 = 0;
      else r.x2 = invTabs[i*4+3]->GetItem().second;
      if( !invTabs[i*4+4]->isVisible() ||invTabs[i*4+4]->GetItem().second == -1 ) r.y2 = 0;
      else r.y2 = invTabs[i*4+4]->GetItem().second;
      hTempObj->SetUserRect(i, r);
     }
 }

 void cEditObjEnemy::RebuildWindow()
 {
     bool bHaveTreasures = 1;
     if( !strcmp(hTempObj->GetLogic(), "Fish")    || !strcmp(hTempObj->GetLogic(), "Gabriel") ||
         !strcmp(hTempObj->GetLogic(), "Aquatis") || !strcmp(hTempObj->GetLogic(), "RedTail") ||
         !strcmp(hTempObj->GetLogic(), "Omar")    || !strcmp(hTempObj->GetLogic(), "Rat") )
      bHaveTreasures = 0;

     hInventory->SetVisible(bHaveTreasures);

     int addheight = 170;
     if( bHaveTreasures ){
      if( !strcmp(hTempObj->GetLogic(), "HermitCrab") ){
       labTreasures->setVisible(1);
       invTabs[0]->setVisible(1);
       invTabs[0]->setPosition(190, 30+vstrpTypes.size()*20+40+130);
       for(int i=1;i<9;i++){
        invTabs[i]->setVisible(0);
       }
      }else{
       labTreasures->setVisible(1);
       for(int i=0;i<9;i++){
        invTabs[i]->setVisible(1);
        invTabs[i]->setPosition(140+(i%3)*50, 30+vstrpTypes.size()*20+30+130+50*int(i/3));
       }
      }
     }else{
      labTreasures->setVisible(0);
      for(int i=0;i<9;i++)
       invTabs[i]->setVisible(0);
     }
     ApplyInventoryToObject();
     for(int i=0;i<9;i++)
      invTabs[i]->setEnabled(!hRectPick->IsPicking()&&!bPickSpeedXY);

     bool dmgvis = !strcmp(hTempObj->GetLogic(), "Fish") || !strcmp(hTempObj->GetLogic(), "HermitCrab");
     if( dmgvis && !labDamage->isVisible() ){
      tfDamage->setText(!strcmp(hTempObj->GetLogic(), "Fish") ? "10" : "5");
     }
     labDamage->setVisible(dmgvis);
     tfDamage->setVisible(dmgvis);

     tfDamage->setEnabled(!hRectPick->IsPicking()&&!bPickSpeedXY);
     labDamage->setColor(tfDamage->isEnabled() ? 0xFF000000 : 0xFF222222);

     bool warpvis = 0;
     for(int i=0;i<9;i++)
      if( invTabs[i]->GetItem().second == 32 && invTabs[i]->isVisible() ){
       warpvis = 1;
       break;
      }
     if( warpvis && !strcmp(hTempObj->GetLogic(), "HermitCrab") )
      warpvis = 0;
     bool gemvis = (!strcmp(hTempObj->GetLogic(), "Raux") ||
                    !strcmp(hTempObj->GetLogic(), "Katherine") ||
                    !strcmp(hTempObj->GetLogic(), "Wolvington") ||
                    !strcmp(hTempObj->GetLogic(), "Gabriel") ||
                    !strcmp(hTempObj->GetLogic(), "Marrow") ||
                    !strcmp(hTempObj->GetLogic(), "Aquatis") ||
                    !strcmp(hTempObj->GetLogic(), "Omar") ||
                    !strcmp(hTempObj->GetLogic(), "RedTail"));
     bPickGem = gemvis;
     labWarpDest->setVisible(warpvis||gemvis);
     tfSpeedX->setVisible(warpvis||gemvis);
     tfSpeedY->setVisible(warpvis||gemvis);
     tfSpeedX->setEnabled(!hRectPick->IsPicking()&&!bPickSpeedXY);
     tfSpeedY->setEnabled(!hRectPick->IsPicking()&&!bPickSpeedXY);
     labWarpDest->setColor(tfSpeedX->isEnabled() ? 0xFF000000 : 0xFF222222);
     butPickSpeedXY->setVisible(warpvis||gemvis);
     butPickSpeedXY->setEnabled(!hRectPick->IsPicking());
     if( warpvis||gemvis ){
      labWarpDest->setCaption(GETL2S("EditObj_Enemy", (gemvis ? "GemDest" : "WarpDest")));
      labWarpDest->adjustSize();
     }

     win->setHeight(95+150+addheight+vstrpTypes.size()*20+dmgvis*28+(warpvis||gemvis)*68);
     win->setY(GV->editState->vPort->GetY());
     _butSave->setY(win->getHeight()-60);
     _butAddNext->setY(win->getHeight()-60);

     _butSave->setEnabled(!hRectPick->IsPicking()&&!bPickSpeedXY&&
                          !tfSpeedX->isMarkedInvalid() && !tfSpeedY->isMarkedInvalid());

     for(int i=0;i<5;i++)
      if( rbType[i] != NULL )
       rbType[i]->setEnabled(!hRectPick->IsPicking()&&!bPickSpeedXY);
     for(int i=0;i<4;i++)
      rbFlags[i]->setEnabled(!hRectPick->IsPicking()&&!bPickSpeedXY);

     bool varietyvisible = (GV->editState->hParser->GetBaseLevel() == 7 && rbType[3]->isSelected() ||
                            GV->editState->hParser->GetBaseLevel() == 14 && rbType[0]->isSelected());
     labType2->setVisible(varietyvisible);
     rbType2[0]->setVisible(varietyvisible);
     rbType2[1]->setVisible(varietyvisible);

     rbType2[0]->setEnabled(!hRectPick->IsPicking()&&!bPickSpeedXY);
     rbType2[1]->setEnabled(!hRectPick->IsPicking()&&!bPickSpeedXY);

     if( hRectPick->IsPicking() || bPickSpeedXY ){
      cbPatrol->setEnabled(0);
     }else{
      if( !strcmp(hTempObj->GetLogic(), "Seagull") || !strcmp(hTempObj->GetLogic(), "Fish") ){
       cbPatrol->setSelected(1);
       cbPatrol->setEnabled(0);
      }else if( !strcmp(hTempObj->GetLogic(), "HermitCrab") ){
       cbPatrol->setSelected(0);
       cbPatrol->setEnabled(0);
      }else
       cbPatrol->setEnabled(1);
     }

     butPickSpeedXY->setCaption(GETL2S("EditObj_Enemy", bPickSpeedXY ? "Cancel" : "Pick"));

     hRectPick->Enable(cbPatrol->isSelected()&&!bPickSpeedXY);
     butClearArea->setEnabled(cbPatrol->isSelected()&&!bPickSpeedXY&&!hRectPick->IsPicking());
     hTempObj->SetParam(WWD::Param_MinX, cbPatrol->isSelected() ? hRectPick->GetValue(0) : 0);
     hTempObj->SetParam(WWD::Param_MinY, cbPatrol->isSelected() ? hRectPick->GetValue(1) : 0);
     hTempObj->SetParam(WWD::Param_MaxX, cbPatrol->isSelected() ? hRectPick->GetValue(2) : 0);
     hTempObj->SetParam(WWD::Param_MaxY, cbPatrol->isSelected() ? hRectPick->GetValue(3) : 0);
 }

 void cEditObjEnemy::Save()
 {
    int flags = 0;
    if( hTempObj->GetDynamicFlags() & WWD::Flag_dy_AlwaysActive )
     flags += (int)WWD::Flag_dy_AlwaysActive;
    if( rbFlags[1]->isSelected() )
     flags += (int)WWD::Flag_dy_Safe;
    else if( rbFlags[2]->isSelected() )
     flags += (int)WWD::Flag_dy_NoHit;
    else if( rbFlags[3]->isSelected() )
     flags += (int)WWD::Flag_dy_AutoHitDamage;
    hTempObj->SetDynamicFlags((WWD::OBJ_DYNAMIC_FLAGS)flags);
    if( !strcmp(hTempObj->GetLogic(), "Seagull") || !strcmp(hTempObj->GetLogic(), "Fish") )
     hTempObj->SetUserValue(0, 0);
    else if( strcmp(hTempObj->GetLogic(), "HermitCrab") != 0 )
     hTempObj->SetUserValue(0, !cbPatrol->isSelected());

    int iWarpIndex = -1;
    for(int i=0;i<9;i++)
     if( invTabs[i]->GetItem().second == 32 ){
      iWarpIndex = i;
      break;
     }
    if( iWarpIndex != -1 && iWarpIndex != 0 ){
     cInventoryItem tmp;
     tmp = invTabs[0]->GetItem();
     invTabs[0]->SetItem(invTabs[iWarpIndex]->GetItem());
     invTabs[iWarpIndex]->SetItem(tmp);
    }
    ApplyInventoryToObject();
    hTempObj->SetParam(WWD::Param_Damage, tfDamage->isVisible() ? atoi(tfDamage->getText().c_str()) : 0);
    if( tfSpeedX->isVisible() ){
     hTempObj->SetParam(WWD::Param_SpeedX, atoi(tfSpeedX->getText().c_str()));
     hTempObj->SetParam(WWD::Param_SpeedY, atoi(tfSpeedY->getText().c_str()));
    }else{
     hTempObj->SetParam(WWD::Param_SpeedX, 0);
     hTempObj->SetParam(WWD::Param_SpeedY, 0);
    }
 }

 void cEditObjEnemy::Action(const gcn::ActionEvent &actionEvent)
 {
    if( actionEvent.getSource() == win ){
     bKill = 1;
     return;
    }else if( actionEvent.getSource() == butClearArea ){
     hRectPick->SetValues(0,0,0,0);
     return;
    }else if( actionEvent.getSource() == cbPatrol ){
     RebuildWindow();
     return;
    }else if( actionEvent.getSource() == hRectPick->GetPickButton() ){
     RebuildWindow();
     return;
    }else if( actionEvent.getSource() == butPickSpeedXY ){
     bPickSpeedXY = !bPickSpeedXY;
     RebuildWindow();
    }else if( actionEvent.getSource() == tfSpeedX ){
     int v = atoi(tfSpeedX->getText().c_str());
     bool marked = tfSpeedX->isMarkedInvalid();
     tfSpeedX->setMarkedInvalid(v == 0 || v >= GV->editState->GetActivePlane()->GetPlaneWidthPx());
     if( tfSpeedX->isMarkedInvalid() != marked )
      RebuildWindow();
     return;
    }else if( actionEvent.getSource() == tfSpeedY ){
     int v = atoi(tfSpeedY->getText().c_str());
     bool marked = tfSpeedX->isMarkedInvalid();
     tfSpeedY->setMarkedInvalid(v == 0 || v >= GV->editState->GetActivePlane()->GetPlaneWidthPx());
     if( tfSpeedY->isMarkedInvalid() != marked )
      RebuildWindow();
     return;
    }
    for(int i=0;i<5;i++)
     if( rbType[i] != NULL && actionEvent.getSource() == rbType[i] ){
      hTempObj->SetLogic(vstrpTypes[i].first.c_str());
      hTempObj->SetImageSet(vstrpTypes[i].second.c_str());
      GV->editState->vPort->MarkToRedraw(1);
      RebuildWindow();
      return;
     }
    for(int i=0;i<2;i++)
     if( actionEvent.getSource() == rbType2[i] ){
      if( GV->editState->hParser->GetBaseLevel() == 7 ){
       hTempObj->SetUserValue(0, i==1);
      }else{
       hTempObj->SetParam(WWD::Param_Smarts, i==1);
      }
      GV->editState->vPort->MarkToRedraw(1);
     }
    for(int i=0;i<9;i++)
     if( actionEvent.getSource() == invTabs[i] ){
      RebuildWindow();
     }
 }

 void cEditObjEnemy::_Think(bool bMouseConsumed)
 {
    if( !bMouseConsumed )
     hRectPick->Think();
    hInventory->Think();
    if( bPickSpeedXY ){
     int posx, posy;
     bool save = 0;
     float mx, my;
     hge->Input_GetMousePos(&mx, &my);
     if( bMouseConsumed || mx < GV->editState->vPort->GetX() || my < GV->editState->vPort->GetY() ||
         mx > GV->editState->vPort->GetX()+GV->editState->vPort->GetWidth() ||
         my > GV->editState->vPort->GetY()+GV->editState->vPort->GetHeight() ){
      posx = hTempObj->GetParam(WWD::Param_SpeedX);
      posy = hTempObj->GetParam(WWD::Param_SpeedY);
     }else{
      int wmx = GV->editState->Scr2WrdX(GV->editState->GetActivePlane(), mx),
          wmy = GV->editState->Scr2WrdY(GV->editState->GetActivePlane(), my);
      posx = wmx;
      posy = wmy;
      if( hge->Input_KeyDown(HGEK_LBUTTON) ){
       hTempObj->SetParam(WWD::Param_SpeedX, wmx);
       hTempObj->SetParam(WWD::Param_SpeedY, wmy);
       save = 1;
      }
     }
     char tmp[64];
     sprintf(tmp,"%d", posx);
     tfSpeedX->setText(tmp);
     sprintf(tmp, "%d", posy);
     tfSpeedY->setText(tmp);
     if( save ){
      bPickSpeedXY = 0;
      RebuildWindow();
     }
    }
 }

 void cEditObjEnemy::RenderObjectOverlay()
 {
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    if( bPickSpeedXY && mx > GV->editState->vPort->GetX() && my > GV->editState->vPort->GetY() &&
        mx < GV->editState->vPort->GetX()+GV->editState->vPort->GetWidth() &&
        my < GV->editState->vPort->GetY()+GV->editState->vPort->GetHeight() &&
        GV->editState->conMain->getWidgetAt(mx,my) == GV->editState->vPort->GetWidget()  ){
      int wmx = GV->editState->Scr2WrdX(GV->editState->GetActivePlane(), mx),
          wmy = GV->editState->Scr2WrdY(GV->editState->GetActivePlane(), my);
      hgeSprite * spr;
      if( bPickGem )
       spr = GV->editState->SprBank->GetAssetByID("LEVEL_GEM")->GetIMGByIterator(0)->GetSprite();
      else
       spr = GV->editState->SprBank->GetAssetByID("CLAW")->GetIMGByID(401)->GetSprite();
      spr->SetColor(0xBBFFFFFF);
      spr->RenderEx(mx, my, 0, GV->editState->fZoom);
      GV->fntMyriad13->printf(mx+spr->GetWidth()/2+1, my+1, HGETEXT_LEFT, "~l~%s: %d, %d", 0,
                              GETL2S("EditObj_Warp", "SpawnPos"), wmx, wmy);
      GV->fntMyriad13->printf(mx+spr->GetWidth()/2, my, HGETEXT_LEFT, "~w~%s: ~y~%d~w~, ~y~%d", 0,
                             GETL2S("EditObj_Warp", "SpawnPos"), wmx, wmy);
    }
 }

 void cEditObjEnemy::Draw()
 {
    int dx, dy;
    win->getAbsolutePosition(dx, dy);
    dy+=24;

    if( labType2->isVisible() ){
     hge->Gfx_RenderLine(dx+137, dy, dx+137, dy+30+vstrpTypes.size()*20, GV->colLineDark);
     hge->Gfx_RenderLine(dx+138, dy, dx+138, dy+30+vstrpTypes.size()*20, GV->colLineBright);
    }

    hge->Gfx_RenderLine(dx, dy+30+vstrpTypes.size()*20, dx+win->getWidth(), dy+30+vstrpTypes.size()*20, GV->colLineDark);
    hge->Gfx_RenderLine(dx, dy+31+vstrpTypes.size()*20, dx+win->getWidth(), dy+31+vstrpTypes.size()*20, GV->colLineBright);

    hge->Gfx_RenderLine(dx, dy+160+vstrpTypes.size()*20, dx+win->getWidth(), dy+160+vstrpTypes.size()*20, GV->colLineDark);
    hge->Gfx_RenderLine(dx, dy+161+vstrpTypes.size()*20, dx+win->getWidth(), dy+161+vstrpTypes.size()*20, GV->colLineBright);

    hge->Gfx_RenderLine(dx, dy+160+173+5+vstrpTypes.size()*20, dx+win->getWidth(), dy+160+173+5+vstrpTypes.size()*20, GV->colLineDark);
    hge->Gfx_RenderLine(dx, dy+161+173+5+vstrpTypes.size()*20, dx+win->getWidth(), dy+161+173+5+vstrpTypes.size()*20, GV->colLineBright);

    if( labTreasures->isVisible() ){
     hge->Gfx_RenderLine(dx+137, dy+160+vstrpTypes.size()*20, dx+137, dy+161+173+3+vstrpTypes.size()*20, GV->colLineDark);
     hge->Gfx_RenderLine(dx+138, dy+160+vstrpTypes.size()*20, dx+138, dy+161+173+3+vstrpTypes.size()*20, GV->colLineBright);
    }

    if( tfDamage->isVisible() ){
     hge->Gfx_RenderLine(dx, dy+160+173+28+8+vstrpTypes.size()*20, dx+win->getWidth(), dy+160+28+173+8+vstrpTypes.size()*20, GV->colLineDark);
     hge->Gfx_RenderLine(dx, dy+161+173+28+8+vstrpTypes.size()*20, dx+win->getWidth(), dy+161+28+173+8+vstrpTypes.size()*20, GV->colLineBright);
    }else if( tfSpeedX->isVisible() ){
     GV->fntMyriad13->SetColor(tfSpeedX->isEnabled() ? 0xFF000000 : 0xFF222222);
     GV->fntMyriad13->Render(dx+5, dy+160+170+28+8+vstrpTypes.size()*20, HGETEXT_LEFT, "X:");
     GV->fntMyriad13->Render(dx+5, dy+160+170+48+8+vstrpTypes.size()*20, HGETEXT_LEFT, "Y:");
     hge->Gfx_RenderLine(dx, dy+160+173+68+8+vstrpTypes.size()*20, dx+win->getWidth(), dy+160+173+68+8+vstrpTypes.size()*20, GV->colLineDark);
     hge->Gfx_RenderLine(dx, dy+161+173+68+8+vstrpTypes.size()*20, dx+win->getWidth(), dy+161+173+68+8+vstrpTypes.size()*20, GV->colLineBright);
    }
 }
}
