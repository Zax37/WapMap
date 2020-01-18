#include "editCrumblingPeg.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include "../databanks/imageSets.h"

extern HGE * hge;

namespace ObjEdit
{
 cEditObjCrumblingPeg::cEditObjCrumblingPeg(WWD::Object * obj, State::EditingWW * st) : cObjEdit(obj, st)
 {
    iType = ObjEdit::enCrumblingPeg;
    win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_CrumblingPeg", "WinCaption"));
    win->setDimension(gcn::Rectangle(0, 0, 200, 90));
    win->setClose(1);
    win->addActionListener(hAL);
    win->add(vpAdv);
    st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY()+st->vPort->GetHeight()-win->getHeight());

    int yoff = 0;

    if( hState->hParser->GetBaseLevel() == 3 ){
     char tmp[24];
     sprintf(tmp, "%p", this);
     rbType[0] = new SHR::RadBut(GV->hGfxInterface, " ", tmp, !strcmp(hTempObj->GetImageSet(), "LEVEL_CRUMBLINPEG2"));
     rbType[0]->adjustSize();
     rbType[0]->addActionListener(hAL);
     win->add(rbType[0], 5, 50);
     rbType[1] = new SHR::RadBut(GV->hGfxInterface, " ", tmp, !strcmp(hTempObj->GetImageSet(), "LEVEL_CRUMBLINPEG1"));
     rbType[1]->adjustSize();
     rbType[1]->addActionListener(hAL);
     win->add(rbType[1], 95, 50);
     if( !rbType[0]->isSelected() && !rbType[1]->isSelected() ){
      rbType[0]->setSelected(1);
      hTempObj->SetImageSet("LEVEL_CRUMBLINPEG2");
      hState->vPort->MarkToRedraw(1);
     }
     win->setHeight(win->getHeight()+90);
     win->setY(win->getY()-90);
     yoff = 80;
    }else{
     rbType[0] = NULL;
     rbType[1] = NULL;
    }

    cbNoRespawn = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_CrumblingPeg", "NoRespawn"));
    cbNoRespawn->adjustSize();
    cbNoRespawn->addActionListener(hAL);
    cbNoRespawn->setSelected(!strcmp(obj->GetLogic(), "CrumblingPegNoRespawn"));
    win->add(cbNoRespawn, 5, yoff+15);

    win->add(_butAddNext, 0, yoff+40);
    win->add(_butSave, 100, yoff+40);
 }

 cEditObjCrumblingPeg::~cEditObjCrumblingPeg()
 {
    if( rbType[0] != NULL )
     for(int i=0;i<2;i++)
      delete rbType[i];
    delete cbNoRespawn;
    delete win;
    hState->vPort->MarkToRedraw(1);
 }

 void cEditObjCrumblingPeg::Action(const gcn::ActionEvent &actionEvent)
 {
    if( actionEvent.getSource() == win ){
     bKill = 1;
     return;
    }else if( rbType[0] != 0 && actionEvent.getSource() == rbType[0] ){
     hTempObj->SetImageSet("LEVEL_CRUMBLINPEG2");
     hState->vPort->MarkToRedraw(1);
    }else if( rbType[1] != 0 && actionEvent.getSource() == rbType[1] ){
     hTempObj->SetImageSet("LEVEL_CRUMBLINPEG1");
     hState->vPort->MarkToRedraw(1);
    }else if( actionEvent.getSource() == cbNoRespawn ){
     if( cbNoRespawn->isSelected() )
      hTempObj->SetLogic("CrumblingPegNoRespawn");
     else
      hTempObj->SetLogic("CrumblingPeg");
    }
 }

 void cEditObjCrumblingPeg::Draw()
 {
    int dx, dy;
    win->getAbsolutePosition(dx, dy);

    if( rbType[0] != 0 ){
     GV->fntMyriad13->SetColor(0xFF000000);
     GV->fntMyriad13->Render(dx+6, dy+30, HGETEXT_LEFT, GETL2S("EditObj_CrumblingPeg", "Imageset"));
     hState->SprBank->GetAssetByID("LEVEL_CRUMBLINPEG2")->GetIMGByIterator(0)->GetSprite()->Render(dx+50, dy+105);
     hState->SprBank->GetAssetByID("LEVEL_CRUMBLINPEG1")->GetIMGByIterator(0)->GetSprite()->Render(dx+140, dy+105);
    }
 }
}
