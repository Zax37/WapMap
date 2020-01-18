#include "editCandy.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../../shared/commonFunc.h"
#include "../cObjectUserData.h"
#include "../databanks/imageSets.h"

extern HGE * hge;

namespace ObjEdit
{

 cEditObjCandy::cEditObjCandy(WWD::Object * obj, State::EditingWW * st) : cObjEdit(obj, st)
 {
    iType = ObjEdit::enCandy;

    win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Candy", "WinCaption"));
    win->setDimension(gcn::Rectangle(0, 0, 300, 300));
    win->setClose(1);
    win->addActionListener(hAL);
    win->add(vpAdv);
    st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY());

    win->add(_butAddNext, 50, 240);
    win->add(_butSave, 150, 240);

    switch(hState->hParser->GetBaseLevel()){
     case 1:
      vsDefaultImgsets.push_back("TORCH");
      vsDefaultImgsets.push_back("TORCHSTAND");
      vsDefaultImgsets.push_back("FLOORCELL");
      vsDefaultImgsets.push_back("LITTLEPUDDLE");
      vsDefaultImgsets.push_back("BIGPUDDLE");
      vsDefaultImgsets.push_back("SKULL");
      vsDefaultImgsets.push_back("ARCHESFRONT");
      vsDefaultImgsets.push_back("HANDS4");
      vsDefaultImgsets.push_back("MANICAL1");
      vsDefaultImgsets.push_back("ARROWSIGN_UP");
      vsDefaultImgsets.push_back("ARROWSIGN_UR");
      vsDefaultImgsets.push_back("ARROWSIGN_RIGHT");
      vsDefaultImgsets.push_back("ARROWSIGN_LR");
      vsDefaultImgsets.push_back("ARROWSIGN_DOWN");
      vsDefaultImgsets.push_back("WEB1");
      vsDefaultImgsets.push_back("WEB3");
      break;
     case 2:
      vsDefaultImgsets.push_back("FLAGSSMALL");
      vsDefaultImgsets.push_back("TORCHSTAND");
      vsDefaultImgsets.push_back("CARPET");
      vsDefaultImgsets.push_back("LARODOOR1");
      vsDefaultImgsets.push_back("LARODOOR2");
      vsDefaultImgsets.push_back("RAUXPAINTING");
      vsDefaultImgsets.push_back("SINGLEBUBBLE");
      vsDefaultImgsets.push_back("TRIPLEBUBBLE");
      vsDefaultImgsets.push_back("TORCHSTAND");
      vsDefaultImgsets.push_back("ARROWSIGN_UP");
      vsDefaultImgsets.push_back("ARROWSIGN_UR");
      vsDefaultImgsets.push_back("ARROWSIGN_RIGHT");
      vsDefaultImgsets.push_back("ARROWSIGN_LR");
      vsDefaultImgsets.push_back("ARROWSIGN_DOWN");
      break;
     case 3:
      vsDefaultImgsets.push_back("STARTSKULLPOST");
      vsDefaultImgsets.push_back("SKULLSIGN");
      vsDefaultImgsets.push_back("TORCH");
      vsDefaultImgsets.push_back("STONES");
      vsDefaultImgsets.push_back("HAMMER");
      vsDefaultImgsets.push_back("BUSH1");
      vsDefaultImgsets.push_back("BUSH2");
      vsDefaultImgsets.push_back("SPIKES1");
      vsDefaultImgsets.push_back("SPIKE2");
      vsDefaultImgsets.push_back("BUTTERBLY");
      vsDefaultImgsets.push_back("ROPE1");
      vsDefaultImgsets.push_back("TREASUREMAP1");
      vsDefaultImgsets.push_back("ARROWSIGN_UP");
      vsDefaultImgsets.push_back("ARROWSIGN_UR");
      vsDefaultImgsets.push_back("ARROWSIGN_RIGHT");
      vsDefaultImgsets.push_back("ARROWSIGN_LR");
      vsDefaultImgsets.push_back("ARROWSIGN_DOWN");
      break;
     case 4:
      vsDefaultImgsets.push_back("TORCH");
      vsDefaultImgsets.push_back("TORCH2");
      vsDefaultImgsets.push_back("BUSH");
      vsDefaultImgsets.push_back("BUSH1");
      vsDefaultImgsets.push_back("BUSH2");
      vsDefaultImgsets.push_back("SMALLPLANT1");
      vsDefaultImgsets.push_back("BIGPLANT1");
      vsDefaultImgsets.push_back("FRONTWEEDS");
      vsDefaultImgsets.push_back("FLOWER1");
      vsDefaultImgsets.push_back("ENDSIGN1");
      vsDefaultImgsets.push_back("ENDSIGN2");
      vsDefaultImgsets.push_back("TREETRUNKCOVER");
      vsDefaultImgsets.push_back("SINGLEGOO");
      vsDefaultImgsets.push_back("TRIPLEGOO");
      vsDefaultImgsets.push_back("ARROWSIGN_UP");
      vsDefaultImgsets.push_back("ARROWSIGN_UR");
      vsDefaultImgsets.push_back("ARROWSIGN_RIGHT");
      vsDefaultImgsets.push_back("ARROWSIGN_LR");
      vsDefaultImgsets.push_back("ARROWSIGN_DOWN");
      vsDefaultImgsets.push_back("GOOLEFT");
      vsDefaultImgsets.push_back("GOOMIDDLE");
      vsDefaultImgsets.push_back("GOORIGHT");
      vsDefaultImgsets.push_back("GOOCOVERUP");
      break;
     case 6:
      vsDefaultImgsets.push_back("SEWERLAMP");
      vsDefaultImgsets.push_back("STREETLAMP");
      vsDefaultImgsets.push_back("PLANTERHEDGE");
      vsDefaultImgsets.push_back("POTTEDSHORTBUSH");
      vsDefaultImgsets.push_back("POTTEDTALLBUSH");
      vsDefaultImgsets.push_back("SMALLPLANTER1");
      vsDefaultImgsets.push_back("SMALLPLANTER2");
      vsDefaultImgsets.push_back("SMALLPLANTER3");
      vsDefaultImgsets.push_back("BIGPLANTER");
      vsDefaultImgsets.push_back("BLOW1");
      vsDefaultImgsets.push_back("ENDWINDOW");
      vsDefaultImgsets.push_back("SIGNPOST");
      vsDefaultImgsets.push_back("SIGN_UP");
      vsDefaultImgsets.push_back("SIGN_RIGHT");
      vsDefaultImgsets.push_back("SIGN_DOWN");
      vsDefaultImgsets.push_back("PORT1");
      vsDefaultImgsets.push_back("PORT2");
      vsDefaultImgsets.push_back("PORT3");
      vsDefaultImgsets.push_back("VERTTUBEALL");
      vsDefaultImgsets.push_back("HORIZONTALTUBEALL");
      vsDefaultImgsets.push_back("PIPETOPLEFT");
      vsDefaultImgsets.push_back("PIPETOPRIGHT");
      vsDefaultImgsets.push_back("TRIPLEGOO");
      vsDefaultImgsets.push_back("WANTED");
      vsDefaultImgsets.push_back("WATER1");
      vsDefaultImgsets.push_back("WATER2");
      vsDefaultImgsets.push_back("WINDOW1");
      vsDefaultImgsets.push_back("WINDOW2");
      vsDefaultImgsets.push_back("WINDOW3");
      vsDefaultImgsets.push_back("WINDOW4");
      vsDefaultImgsets.push_back("ENDSIGN");
      break;
    }
    if( strlen(hTempObj->GetImageSet()) == 0 || hState->SprBank->GetAssetByID(hTempObj->GetImageSet()) == 0 ){
     std::string nis = "LEVEL_";
     nis += vsDefaultImgsets[0].c_str();
     hTempObj->SetImageSet(nis.c_str());
    }

    labImageSet = new SHR::Lab(GETL2S("EditObj_Candy", "Imageset"));
    labImageSet->adjustSize();
    win->add(labImageSet, 5, 15);

    tddImageSet = new SHR::TextDropDown(hTempObj->GetImageSet(), this);
    tddImageSet->setDimension(gcn::Rectangle(0, 0, 200, 20));
    tddImageSet->SetGfx(&GV->gcnParts);
    tddImageSet->addActionListener(hAL);
    tddImageSet->setText(hTempObj->GetImageSet());
    win->add(tddImageSet, 10+labImageSet->getWidth(), 15);

    labFrame = new SHR::Lab(GETL2S("EditObj_Candy", "Frame"));
    labFrame->adjustSize();
    win->add(labFrame, 5, 43);

    char tmp[64];
    sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_LocationI));
    tfFrame = new SHR::TextField(tmp);
    tfFrame->addActionListener(hAL);
    tfFrame->setDimension(gcn::Rectangle(0,0, 125, 20));
    tfFrame->SetNumerical(1,1);
    win->add(tfFrame, 110, 45);

    cSprBankAsset * as = hState->SprBank->GetAssetByID(hTempObj->GetImageSet());

    labFrame->setColor(!as ? 0xFF323232 : 0xFF000000);
    tfFrame->setEnabled(as!=0);

    cbAnimated = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Candy", "Animated"));
    cbAnimated->adjustSize();
    cbAnimated->addActionListener(hAL);
    win->add(cbAnimated, 5, 69);
    cbAnimated->setSelected(strstr(hTempObj->GetLogic(), "Ani")!=0);

    labAnimation = new SHR::Lab(GETL2S("EditObj_Candy", "Animation"));
    labAnimation->adjustSize();
    labAnimation->setColor(cbAnimated->isSelected() ? 0xFF000000 : 0xFF323232);
    win->add(labAnimation, 5, 90);

    tfAnimation = new SHR::TextField(hTempObj->GetAnim());
    tfAnimation->setDimension(gcn::Rectangle(0,0, 275-labAnimation->getWidth(), 20));
    tfAnimation->setEnabled(cbAnimated->isSelected());
    tfAnimation->addActionListener(hAL);
    win->add(tfAnimation, 10+labAnimation->getWidth(), 90);

    labAlign = new SHR::Lab(GETL2S("EditObj_Candy", "Align"));
    labAlign->adjustSize();
    win->add(labAlign, 5, 119);

    sprintf(tmp, "%p", this);
    rbType[0] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Candy", "Behind"), tmp, strstr(hTempObj->GetLogic(), "Behind"));
    rbType[0]->adjustSize();
    rbType[0]->addActionListener(hAL);
    win->add(rbType[0], 5, 140);

    rbType[1] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Candy", "Nothing"), tmp, strstr(hTempObj->GetLogic(), "DoNothing"));
    rbType[1]->adjustSize();
    rbType[1]->addActionListener(hAL);
    if( !strcmp(hTempObj->GetLogic(), "AniCycle") )
     rbType[1]->setSelected(1);
    win->add(rbType[1], 105, 140);

    rbType[2] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Candy", "Front"), tmp, strstr(hTempObj->GetLogic(), "Front"));
    rbType[2]->adjustSize();
    rbType[2]->addActionListener(hAL);
    win->add(rbType[2], 205, 140);
 }

 cEditObjCandy::~cEditObjCandy()
 {
    delete labAlign;
    for(int i=0;i<3;i++)
     delete rbType[i];
    delete tfAnimation;
    delete labAnimation;
    delete cbAnimated;
    delete labFrame;
    delete tfFrame;
    delete labImageSet;
    delete tddImageSet;
    delete win;
    hState->vPort->MarkToRedraw(1);
 }

 void cEditObjCandy::Action(const gcn::ActionEvent &actionEvent)
 {
    if( actionEvent.getSource() == win ){
     bKill = 1;
     return;
    }else if( actionEvent.getSource() == tddImageSet ){
     if( actionEvent.getId() == "SELECTION" ){
      char ntext[strlen(tddImageSet->getText().c_str())+10];
      sprintf(ntext, "LEVEL_%s", tddImageSet->getText().c_str());
      tddImageSet->setText(ntext);
     }
     hTempObj->SetImageSet(tddImageSet->getText().c_str());
     cSprBankAsset * as = hState->SprBank->GetAssetByID(hTempObj->GetImageSet());
     tfFrame->setEnabled(as!=0);
     labFrame->setColor(!as ? 0xFF323232 : 0xFF000000);
     if( as ){
      if( hTempObj->GetParam(WWD::Param_LocationI) >= as->GetSpritesCount() ){
       hTempObj->SetParam(WWD::Param_LocationI, as->GetSpritesCount()-1);
       char n[64];
       sprintf(n, "%d", as->GetSpritesCount()-1);
       tfFrame->setText(n);
      }
      char n[256];
      sprintf(n, "%s: [%d]", GETL2S("EditObj_Candy", "Frame"), hTempObj->GetParam(WWD::Param_LocationI));
      labFrame->setCaption(n);
      labFrame->adjustSize();
     }
     hState->vPort->MarkToRedraw(1);
    }else if( actionEvent.getSource() == tfFrame ){
     hTempObj->SetParam(WWD::Param_LocationI, atoi(tfFrame->getText().c_str()));
     GetUserDataFromObj(hTempObj)->SyncToObj();
     hState->vPort->MarkToRedraw(1);
    }else if( actionEvent.getSource() == cbAnimated ){
     tfAnimation->setEnabled(cbAnimated->isSelected());
     hTempObj->SetAnim(cbAnimated->isSelected() ? tfAnimation->getText().c_str() : "");
     UpdateLogic();
    }else if( actionEvent.getSource() == tfAnimation ){
     hTempObj->SetAnim(tfAnimation->getText().c_str());
    }
    for(int i=0;i<3;i++)
     if( actionEvent.getSource() == rbType[i] ){
      UpdateLogic();
      if( i == 0 ) hTempObj->SetParam(WWD::Param_LocationZ, 1000);
      if( i == 2 ) hTempObj->SetParam(WWD::Param_LocationZ, 5000);
     }
 }

 void cEditObjCandy::Draw()
 {
    int dx, dy;
    win->getAbsolutePosition(dx, dy);
 }

 std::string cEditObjCandy::getElementAt(int i)
 {
     if( i < 0 || i >= vsDefaultImgsets.size() ) return "";
     return vsDefaultImgsets[i];
 }

 int cEditObjCandy::getNumberOfElements()
 {
     return vsDefaultImgsets.size();
 }

 void cEditObjCandy::UpdateLogic()
 {
     if( rbType[0]->isSelected() ){
      hTempObj->SetLogic(cbAnimated->isSelected() ? "BehindAniCandy" : "BehindCandy");
     }else if( rbType[1]->isSelected() ){
      hTempObj->SetLogic(cbAnimated->isSelected() ? "AniCycle" : "DoNothing");
     }else if( rbType[2]->isSelected() ){
      hTempObj->SetLogic(cbAnimated->isSelected() ? "FrontAniCandy" : "FrontCandy");
     }
 }
}
