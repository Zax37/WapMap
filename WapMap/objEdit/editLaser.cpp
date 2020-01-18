#include "editLaser.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"

extern HGE * hge;

namespace ObjEdit
{
 cEditObjLaser::cEditObjLaser(WWD::Object * obj, State::EditingWW * st) : cObjEdit(obj, st)
 {
    iType = ObjEdit::enLaser;
    win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Laser", "WinCaption"));
    win->setDimension(gcn::Rectangle(0, 0, 250, 125));
    win->setClose(1);
    win->addActionListener(hAL);
    win->add(vpAdv);
    win->setMovable(0);
    st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY()+st->vPort->GetHeight()-win->getHeight());

    int yoffset = 10;

    win->add(_butAddNext, 20, win->getHeight()-60);
    win->add(_butSave, 130, win->getHeight()-60);

    labTimeOff = new SHR::Lab(GETL2S("EditObj_Laser", "TimeOff"));
    labTimeOff->adjustSize();
    win->add(labTimeOff, 5, 5+yoffset);

    char tmp[50];
    sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_Counter));
    tfTimeOff = new SHR::TextField(tmp);
    tfTimeOff->setDimension(gcn::Rectangle(0,0,100,20));
    tfTimeOff->SetNumerical(1, 0);
    tfTimeOff->addActionListener(hAL);
    win->add(tfTimeOff, 140, 5+yoffset);
    tfTimeOff->setMarkedInvalid(hTempObj->GetParam(WWD::Param_Counter) < 0);

    labDmg = new SHR::Lab(GETL2S("EditObj_Laser", "Damage"));
    labDmg->adjustSize();
    win->add(labDmg, 5, 30+yoffset);

    int dmg = hTempObj->GetParam(WWD::Param_Damage);
    if( !dmg ) dmg = 1;
    sprintf(tmp, "%d", dmg);
    tfDmg = new SHR::TextField(tmp);
    tfDmg->setDimension(gcn::Rectangle(0,0,100,20));
    tfDmg->SetNumerical(1, 0);
    tfDmg->addActionListener(hAL);
    tfDmg->setMarkedInvalid(dmg < 0);
    win->add(tfDmg, 140, 30+yoffset);

    _butSave->setEnabled(!tfDmg->isMarkedInvalid()&&!tfTimeOff->isMarkedInvalid());
 }

 cEditObjLaser::~cEditObjLaser()
 {
    delete labDmg;
    delete labTimeOff;
    delete tfDmg;
    delete tfTimeOff;
    delete win;
    hState->vPort->MarkToRedraw(1);
 }

 void cEditObjLaser::Save()
 {
     hTempObj->SetParam(WWD::Param_Counter, atoi(tfTimeOff->getText().c_str()));
     hTempObj->SetParam(WWD::Param_Damage, atoi(tfDmg->getText().c_str()));
 }

 void cEditObjLaser::Action(const gcn::ActionEvent &actionEvent)
 {
    if( actionEvent.getSource() == win ){
     bKill = 1;
     return;
    }else if( actionEvent.getSource() == tfDmg ){
     int v = atoi(tfDmg->getText().c_str());
     tfDmg->setMarkedInvalid(v < 0);
    }else if( actionEvent.getSource() == tfTimeOff ){
     int v = atoi(tfTimeOff->getText().c_str());
     tfTimeOff->setMarkedInvalid(v < 0);
    }
    _butSave->setEnabled(!tfDmg->isMarkedInvalid()&&!tfTimeOff->isMarkedInvalid());
 }
}
