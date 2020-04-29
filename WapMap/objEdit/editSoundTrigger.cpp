#include "editDialog.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include "../databanks/sounds.h"

#define ADDSTANDSOUND(x) vsStandardSounds.push_back(std::pair<std::string,std::string>(x, GETL2S("Dialog_Translations", x)))

extern HGE *hge;

namespace ObjEdit {
    cEditObjSoundTrigger::cEditObjSoundTrigger(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enDialog;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_Dialog", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 364, 400));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        win->setShadow(1);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        switch (hState->hParser->GetBaseLevel()) {
            case 1:
                ADDSTANDSOUND("LEVEL_TRIGGER_1000");
                ADDSTANDSOUND("LEVEL_TRIGGER_1004");
                ADDSTANDSOUND("LEVEL_TRIGGER_1005");
                ADDSTANDSOUND("LEVEL_TRIGGER_1006");
                ADDSTANDSOUND("LEVEL_TRIGGER_1007");
                ADDSTANDSOUND("LEVEL_TRIGGER_1040");
                ADDSTANDSOUND("LEVEL_TRIGGER_1052");
                ADDSTANDSOUND("LEVEL_TRIGGER_1053");
                ADDSTANDSOUND("LEVEL_TRIGGER_1060");
                ADDSTANDSOUND("LEVEL_TRIGGER_DEATHTRAP");
                ADDSTANDSOUND("LEVEL_TRIGGER_TIMING");
                ADDSTANDSOUND("LEVEL_TRIGGER_SOFARSOGOOD");
                ADDSTANDSOUND("LEVEL_TRIGGER_SMELLGOLD");
                break;
            case 2:
                ADDSTANDSOUND("LEVEL_TRIGGER_1045");
                ADDSTANDSOUND("LEVEL_TRIGGER_1013");
                ADDSTANDSOUND("LEVEL_TRIGGER_1032");
                ADDSTANDSOUND("LEVEL_TRIGGER_1044");
                ADDSTANDSOUND("LEVEL_TRIGGER_1004");
                ADDSTANDSOUND("LEVEL_TRIGGER_1005");
                ADDSTANDSOUND("LEVEL_TRIGGER_1006");
                break;
            case 3:
                ADDSTANDSOUND("LEVEL_TRIGGER_1011");
                ADDSTANDSOUND("LEVEL_TRIGGER_1012");
                ADDSTANDSOUND("LEVEL_TRIGGER_1034");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110014");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110017");
                ADDSTANDSOUND("LEVEL_TRIGGER_1004");
                ADDSTANDSOUND("LEVEL_TRIGGER_1005");
                ADDSTANDSOUND("LEVEL_TRIGGER_1006");
                ADDSTANDSOUND("LEVEL_TRIGGER_1045");
                break;
            case 4:
                ADDSTANDSOUND("LEVEL_TRIGGER_DEADEND");
                ADDSTANDSOUND("LEVEL_TRIGGER_DEATHTRAP");
                ADDSTANDSOUND("LEVEL_TRIGGER_LOST");
                ADDSTANDSOUND("LEVEL_TRIGGER_ONESHOT");
                ADDSTANDSOUND("LEVEL_TRIGGER_PATHNOTONMAP");
                ADDSTANDSOUND("LEVEL_TRIGGER_SMELLGOLD");
                ADDSTANDSOUND("LEVEL_TRIGGER_SOFARSOGOOD");
                ADDSTANDSOUND("LEVEL_TRIGGER_1061");
                ADDSTANDSOUND("LEVEL_TRIGGER_TIMING");
                ADDSTANDSOUND("LEVEL_TRIGGER_1000");
                ADDSTANDSOUND("LEVEL_TRIGGER_1060");
                ADDSTANDSOUND("LEVEL_TRIGGER_1004");
                ADDSTANDSOUND("LEVEL_TRIGGER_1053");
                ADDSTANDSOUND("LEVEL_TRIGGER_1052");
                ADDSTANDSOUND("LEVEL_TRIGGER_1013");
                ADDSTANDSOUND("LEVEL_TRIGGER_1012");
                ADDSTANDSOUND("LEVEL_TRIGGER_1005");
                ADDSTANDSOUND("LEVEL_TRIGGER_1006");
                break;
            case 5:
                ADDSTANDSOUND("LEVEL_TRIGGER_1110020");
                ADDSTANDSOUND("LEVEL_TRIGGER_1010");
                ADDSTANDSOUND("LEVEL_TRIGGER_1049");
                ADDSTANDSOUND("LEVEL_TRIGGER_1062");
                ADDSTANDSOUND("LEVEL_TRIGGER_1006");
                ADDSTANDSOUND("LEVEL_TRIGGER_1044");
                ADDSTANDSOUND("LEVEL_TRIGGER_1004");
                ADDSTANDSOUND("LEVEL_TRIGGER_1005");
                ADDSTANDSOUND("LEVEL_TRIGGER_1011");
                ADDSTANDSOUND("LEVEL_TRIGGER_1045");
                break;
            case 6:
                ADDSTANDSOUND("LEVEL_TRIGGER_1015");
                ADDSTANDSOUND("LEVEL_TRIGGER_1016");
                ADDSTANDSOUND("LEVEL_TRIGGER_1017");
                ADDSTANDSOUND("LEVEL_TRIGGER_BLOODYBIRDS");
                ADDSTANDSOUND("LEVEL_TRIGGER_1043");
                ADDSTANDSOUND("LEVEL_TRIGGER_ONESHOT");
                ADDSTANDSOUND("LEVEL_TRIGGER_SMELLGOLD");
                ADDSTANDSOUND("LEVEL_TRIGGER_SOFARSOGOOD");
                ADDSTANDSOUND("LEVEL_TRIGGER_TIMING");
                ADDSTANDSOUND("LEVEL_TRIGGER_1010");
                ADDSTANDSOUND("LEVEL_TRIGGER_1013");
                ADDSTANDSOUND("LEVEL_TRIGGER_1053");
                ADDSTANDSOUND("LEVEL_TRIGGER_1060");
                ADDSTANDSOUND("LEVEL_TRIGGER_1005");
                ADDSTANDSOUND("LEVEL_TRIGGER_DEADEND");
                ADDSTANDSOUND("LEVEL_TRIGGER_1006");
                break;
            case 7:
                ADDSTANDSOUND("LEVEL_TRIGGER_1001");
                ADDSTANDSOUND("LEVEL_TRIGGER_1002");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110015");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110016");
                ADDSTANDSOUND("LEVEL_TRIGGER_1003");
                ADDSTANDSOUND("LEVEL_TRIGGER_1045");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110020");
                ADDSTANDSOUND("LEVEL_TRIGGER_1000");
                ADDSTANDSOUND("LEVEL_TRIGGER_1004");
                ADDSTANDSOUND("LEVEL_TRIGGER_1005");
                ADDSTANDSOUND("LEVEL_TRIGGER_1006");
                break;
            case 8:
                ADDSTANDSOUND("LEVEL_TRIGGER_1051");
                ADDSTANDSOUND("LEVEL_TRIGGER_1054");
                ADDSTANDSOUND("LEVEL_TRIGGER_1018");
                ADDSTANDSOUND("LEVEL_TRIGGER_1019");
                ADDSTANDSOUND("LEVEL_TRIGGER_1020");
                ADDSTANDSOUND("LEVEL_TRIGGER_1021");
                ADDSTANDSOUND("LEVEL_TRIGGER_1061");
                ADDSTANDSOUND("LEVEL_TRIGGER_1060");
                ADDSTANDSOUND("LEVEL_TRIGGER_BLOODYBIRDS");
                ADDSTANDSOUND("LEVEL_TRIGGER_1004");
                ADDSTANDSOUND("LEVEL_TRIGGER_1005");
                ADDSTANDSOUND("LEVEL_TRIGGER_1006");
                ADDSTANDSOUND("LEVEL_TRIGGER_TIMING");
                ADDSTANDSOUND("LEVEL_TRIGGER_1052");
                ADDSTANDSOUND("LEVEL_TRIGGER_1053");
                ADDSTANDSOUND("LEVEL_TRIGGER_ONESHOT");
                ADDSTANDSOUND("LEVEL_TRIGGER_SOFARSOGOOD");
                ADDSTANDSOUND("LEVEL_TRIGGER_1013");
                break;
            case 9:
                ADDSTANDSOUND("LEVEL_TRIGGER_1026");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110018");
                ADDSTANDSOUND("LEVEL_TRIGGER_1027");
                ADDSTANDSOUND("LEVEL_TRIGGER_1028");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110019");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110010");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110011");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110013");
                ADDSTANDSOUND("LEVEL_TRIGGER_1022");
                ADDSTANDSOUND("LEVEL_TRIGGER_1023");
                ADDSTANDSOUND("LEVEL_TRIGGER_1024");
                ADDSTANDSOUND("LEVEL_TRIGGER_1025");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110017");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110020");
                ADDSTANDSOUND("LEVEL_TRIGGER_1045");
                ADDSTANDSOUND("LEVEL_TRIGGER_1044");
                ADDSTANDSOUND("LEVEL_TRIGGER_1062");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110016");
                ADDSTANDSOUND("LEVEL_TRIGGER_1006");
                ADDSTANDSOUND("LEVEL_TRIGGER_1061");
                ADDSTANDSOUND("LEVEL_TRIGGER_1005");
                ADDSTANDSOUND("LEVEL_TRIGGER_1060");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110014");
                ADDSTANDSOUND("LEVEL_TRIGGER_1004");
                break;
            case 10:
                ADDSTANDSOUND("LEVEL_TRIGGER_1029");
                ADDSTANDSOUND("LEVEL_TRIGGER_TARZAN");
                ADDSTANDSOUND("LEVEL_TRIGGER_1026");
                ADDSTANDSOUND("LEVEL_TRIGGER_1027");
                ADDSTANDSOUND("LEVEL_TRIGGER_1028");
                ADDSTANDSOUND("LEVEL_TRIGGER_SOFARSOGOOD");
                ADDSTANDSOUND("LEVEL_TRIGGER_TIMING");
                ADDSTANDSOUND("LEVEL_TRIGGER_1013");
                ADDSTANDSOUND("LEVEL_TRIGGER_1053");
                ADDSTANDSOUND("LEVEL_TRIGGER_1000");
                ADDSTANDSOUND("LEVEL_TRIGGER_DEATHTRAP");
                ADDSTANDSOUND("LEVEL_TRIGGER_1004");
                ADDSTANDSOUND("LEVEL_TRIGGER_1023");
                ADDSTANDSOUND("LEVEL_TRIGGER_1061");
                ADDSTANDSOUND("LEVEL_TRIGGER_ONESHOT");
                ADDSTANDSOUND("LEVEL_TRIGGER_1005");
                ADDSTANDSOUND("LEVEL_TRIGGER_1024");
                ADDSTANDSOUND("LEVEL_TRIGGER_PATHNOTONMAP");
                ADDSTANDSOUND("LEVEL_TRIGGER_1006");
                break;
            case 11:
                ADDSTANDSOUND("LEVEL_TRIGGER_1030");
                ADDSTANDSOUND("LEVEL_TRIGGER_1031");
                ADDSTANDSOUND("LEVEL_TRIGGER_1014");
                ADDSTANDSOUND("LEVEL_TRIGGER_1033");
                ADDSTANDSOUND("LEVEL_TRIGGER_1054");
                ADDSTANDSOUND("LEVEL_TRIGGER_1038");
                ADDSTANDSOUND("LEVEL_TRIGGER_1003");
                ADDSTANDSOUND("LEVEL_TRIGGER_1060");
                ADDSTANDSOUND("LEVEL_TRIGGER_1004");
                ADDSTANDSOUND("LEVEL_TRIGGER_1005");
                ADDSTANDSOUND("LEVEL_TRIGGER_1006");
                ADDSTANDSOUND("LEVEL_TRIGGER_1044");
                ADDSTANDSOUND("LEVEL_TRIGGER_1045");
                ADDSTANDSOUND("LEVEL_TRIGGER_1052");
                ADDSTANDSOUND("LEVEL_TRIGGER_1034");
                ADDSTANDSOUND("LEVEL_TRIGGER_1053");
                ADDSTANDSOUND("LEVEL_TRIGGER_1032");
                break;
            case 12:
                ADDSTANDSOUND("LEVEL_TRIGGER_1050");
                ADDSTANDSOUND("LEVEL_TRIGGER_1035");
                ADDSTANDSOUND("LEVEL_TRIGGER_1007");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110018");
                ADDSTANDSOUND("LEVEL_TRIGGER_1045");
                ADDSTANDSOUND("LEVEL_TRIGGER_1027");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110019");
                ADDSTANDSOUND("LEVEL_TRIGGER_1028");
                ADDSTANDSOUND("LEVEL_TRIGGER_1010");
                ADDSTANDSOUND("LEVEL_TRIGGER_1049");
                ADDSTANDSOUND("LEVEL_TRIGGER_1011");
                ADDSTANDSOUND("LEVEL_TRIGGER_1030");
                ADDSTANDSOUND("LEVEL_TRIGGER_1031");
                ADDSTANDSOUND("LEVEL_TRIGGER_1032");
                ADDSTANDSOUND("LEVEL_TRIGGER_1013");
                ADDSTANDSOUND("LEVEL_TRIGGER_1052");
                ADDSTANDSOUND("LEVEL_TRIGGER_1014");
                ADDSTANDSOUND("LEVEL_TRIGGER_1033");
                ADDSTANDSOUND("LEVEL_TRIGGER_1015");
                ADDSTANDSOUND("LEVEL_TRIGGER_1034");
                ADDSTANDSOUND("LEVEL_TRIGGER_1053");
                ADDSTANDSOUND("LEVEL_TRIGGER_1016");
                ADDSTANDSOUND("LEVEL_TRIGGER_1054");
                ADDSTANDSOUND("LEVEL_TRIGGER_1017");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110010");
                ADDSTANDSOUND("LEVEL_TRIGGER_1038");
                ADDSTANDSOUND("LEVEL_TRIGGER_1000");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110011");
                ADDSTANDSOUND("LEVEL_TRIGGER_1001");
                ADDSTANDSOUND("LEVEL_TRIGGER_1040");
                ADDSTANDSOUND("LEVEL_TRIGGER_1021");
                ADDSTANDSOUND("LEVEL_TRIGGER_1002");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110013");
                ADDSTANDSOUND("LEVEL_TRIGGER_1022");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110014");
                ADDSTANDSOUND("LEVEL_TRIGGER_1003");
                ADDSTANDSOUND("LEVEL_TRIGGER_1060");
                ADDSTANDSOUND("LEVEL_TRIGGER_1023");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110015");
                ADDSTANDSOUND("LEVEL_TRIGGER_1004");
                ADDSTANDSOUND("LEVEL_TRIGGER_1061");
                ADDSTANDSOUND("LEVEL_TRIGGER_1043");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110016");
                ADDSTANDSOUND("LEVEL_TRIGGER_1005");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110017");
                ADDSTANDSOUND("LEVEL_TRIGGER_1006");
                ADDSTANDSOUND("LEVEL_TRIGGER_1044");
                break;
            case 13:
                ADDSTANDSOUND("LEVEL_TRIGGER_1047");
                ADDSTANDSOUND("LEVEL_TRIGGER_PATHNOTONMAP");
                ADDSTANDSOUND("LEVEL_TRIGGER_SMELLGOLD");
                ADDSTANDSOUND("LEVEL_TRIGGER_SOFARSOGOOD");
                ADDSTANDSOUND("LEVEL_TRIGGER_TARZAN");
                ADDSTANDSOUND("LEVEL_TRIGGER_TIMING");
                ADDSTANDSOUND("LEVEL_TRIGGER_1013");
                ADDSTANDSOUND("LEVEL_TRIGGER_1052");
                ADDSTANDSOUND("LEVEL_TRIGGER_1000");
                ADDSTANDSOUND("LEVEL_TRIGGER_1019");
                ADDSTANDSOUND("LEVEL_TRIGGER_1021");
                ADDSTANDSOUND("LEVEL_TRIGGER_1060");
                ADDSTANDSOUND("LEVEL_TRIGGER_1004");
                ADDSTANDSOUND("LEVEL_TRIGGER_1061");
                ADDSTANDSOUND("LEVEL_TRIGGER_DEADEND");
                ADDSTANDSOUND("LEVEL_TRIGGER_1005");
                ADDSTANDSOUND("LEVEL_TRIGGER_DEATHTRAP");
                ADDSTANDSOUND("LEVEL_TRIGGER_1006");
                ADDSTANDSOUND("LEVEL_TRIGGER_ONESHOT");
                break;
            case 14:
                ADDSTANDSOUND("LEVEL_TRIGGER_1045");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110018");
                ADDSTANDSOUND("LEVEL_TRIGGER_1048");
                ADDSTANDSOUND("LEVEL_TRIGGER_1031");
                ADDSTANDSOUND("LEVEL_TRIGGER_1034");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110011");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110014");
                ADDSTANDSOUND("LEVEL_TRIGGER_1004");
                ADDSTANDSOUND("LEVEL_TRIGGER_1005");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110016");
                ADDSTANDSOUND("LEVEL_TRIGGER_1006");
                ADDSTANDSOUND("LEVEL_TRIGGER_1044");
                ADDSTANDSOUND("LEVEL_TRIGGER_1110017");
                break;
        }

        ADDSTANDSOUND("CLAW_1110001");
        ADDSTANDSOUND("CLAW_1110012");
        ADDSTANDSOUND("CLAW_1110022");
        ADDSTANDSOUND("CLAW_1110033");
        ADDSTANDSOUND("CLAW_1110035");
        ADDSTANDSOUND("CLAW_1110037");
        ADDSTANDSOUND("CLAW_1110038");
        ADDSTANDSOUND("CLAW_1110043");
        ADDSTANDSOUND("CLAW_1110045");
        ADDSTANDSOUND("CLAW_1110056");
        ADDSTANDSOUND("CLAW_1110057");
        ADDSTANDSOUND("CLAW_1110058");
        ADDSTANDSOUND("CLAW_1110059");
        ADDSTANDSOUND("CLAW_1110063");
        ADDSTANDSOUND("CLAW_1110064");
        ADDSTANDSOUND("CLAW_1001");
        ADDSTANDSOUND("CLAW_1002");
        ADDSTANDSOUND("CLAW_1003");
        ADDSTANDSOUND("CLAW_1004A");
        ADDSTANDSOUND("CLAW_1004B");
        ADDSTANDSOUND("CLAW_1007A");
        ADDSTANDSOUND("CLAW_1007B");
        ADDSTANDSOUND("CLAW_1009");
        ADDSTANDSOUND("CLAW_1010");
        ADDSTANDSOUND("CLAW_1021");
        ADDSTANDSOUND("CLAW_1054");
        ADDSTANDSOUND("GAME_1055");
        ADDSTANDSOUND("CLAW_1055");
        ADDSTANDSOUND("CLAW_1056");
        ADDSTANDSOUND("CLAW_1057");
        ADDSTANDSOUND("CLAW_1058");
        ADDSTANDSOUND("CLAW_1062");

        win->add(_butAddNext, 157, 346);
        win->add(_butSave, 257, 346);

        char tmp[64];
        sprintf(tmp, "%p", this);
        rbStandard = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Dialog", "VerseStandard"), tmp);
        rbStandard->adjustSize();
        rbStandard->addActionListener(hAL);
        win->add(rbStandard, 5, 12);

        rbOther = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Dialog", "VerseOther"), tmp);
        rbOther->adjustSize();
        rbOther->addActionListener(hAL);
        win->add(rbOther, 5, 56);

        ddStandardDialogs = new SHR::DropDown();
        ddStandardDialogs->setListModel(this);
        ddStandardDialogs->setDimension(gcn::Rectangle(0, 0, 351, 20));
        ddStandardDialogs->addActionListener(hAL);
        ddStandardDialogs->SetGfx(&GV->gcnParts);
        ddStandardDialogs->adjustHeight();
        win->add(ddStandardDialogs, 5, 34);

        tddOtherDialogs = new SHR::TextDropDown("", hState->hSndBank);
        tddOtherDialogs->setDimension(gcn::Rectangle(0, 0, 351, 20));
        tddOtherDialogs->SetGfx(&GV->gcnParts);
        tddOtherDialogs->addActionListener(hAL);
        tddOtherDialogs->setText(hTempObj->GetAnim());
        win->add(tddOtherDialogs, 5, 78);

        if (strlen(hTempObj->GetAnim()) == 0) {
            rbStandard->setSelected(1);
            tddOtherDialogs->setEnabled(0);
            ddStandardDialogs->setSelected(0);
        } else if (hState->hSndBank->GetAssetByID(hTempObj->GetAnim()) == 0) {
            rbOther->setSelected(1);
            ddStandardDialogs->setEnabled(0);
            tddOtherDialogs->setText(hTempObj->GetAnim());
            tddOtherDialogs->setMarkedInvalid(1);
        } else {
            bool bfound = 0;
            for (int i = 0; i < vsStandardSounds.size(); i++) {
                if (!strcmp(hTempObj->GetAnim(), vsStandardSounds[i].first.c_str())) {
                    bfound = 1;
                    ddStandardDialogs->setSelected(i);
                    break;
                }
            }
            if (bfound) {
                rbStandard->setSelected(1);
                tddOtherDialogs->setEnabled(0);
            } else {
                ddStandardDialogs->setEnabled(0);
                rbOther->setSelected(1);
                tddOtherDialogs->setText(hTempObj->GetAnim());
            }
        }

        cbDialog = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_Dialog", "ShowExclamation"));
        cbDialog->adjustSize();
        cbDialog->addActionListener(hAL);
        cbDialog->setSelected(strstr(obj->GetLogic(), "Dialog") != 0);
        win->add(cbDialog, 5, 190);

        labActivateMode = new SHR::Lab(GETL2S("EditObj_Dialog", "ActivationPolicy"));
        labActivateMode->adjustSize();
        win->add(labActivateMode, 5, 240);

        sprintf(tmp, "%p_2", this);
        rbActivate[0] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Dialog", "ActivationPolicy1"), tmp);
        rbActivate[0]->adjustSize();
        rbActivate[0]->addActionListener(hAL);
        win->add(rbActivate[0], 5, 265);
        rbActivate[1] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Dialog", "ActivationPolicy2"), tmp);
        rbActivate[1]->adjustSize();
        rbActivate[1]->addActionListener(hAL);
        win->add(rbActivate[1], 5, 290);
        rbActivate[2] = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_Dialog", "ActivationPolicy3"), tmp);
        rbActivate[2]->adjustSize();
        rbActivate[2]->addActionListener(hAL);
        win->add(rbActivate[2], 5, 315);

        sprintf(tmp, "1");
        tfRespawnTimes = new SHR::TextField(tmp);
        tfRespawnTimes->setDimension(gcn::Rectangle(0, 0, 50, 20));
        tfRespawnTimes->setEnabled(0);
        tfRespawnTimes->addActionListener(hAL);
        win->add(tfRespawnTimes, 5 + 22 + rbActivate[2]->getWidth() + 15, 315);

        labActivationArea = new SHR::Lab(GETL2S("EditObj_Dialog", "FR_Size"));
        labActivationArea->adjustSize();
        win->add(labActivationArea, 5, 215);

        lmSizes = new lmEditObjDialogSizes();
        ddActivationArea = new SHR::DropDown();
        ddActivationArea->setListModel(lmSizes);
        ddActivationArea->setDimension(gcn::Rectangle(0, 0, 200, 20));
        ddActivationArea->addActionListener(hAL);
        ddActivationArea->SetGfx(&GV->gcnParts);
        ddActivationArea->adjustHeight();
        win->add(ddActivationArea, 157, 215);

        if (strstr(hTempObj->GetLogic(), "Tiny")) ddActivationArea->setSelected(0);
        else if (strstr(hTempObj->GetLogic(), "Small")) ddActivationArea->setSelected(1);
        else if (strstr(hTempObj->GetLogic(), "Big")) ddActivationArea->setSelected(3);
        else if (strstr(hTempObj->GetLogic(), "Huge")) ddActivationArea->setSelected(4);
        else if (strstr(hTempObj->GetLogic(), "Wide")) ddActivationArea->setSelected(5);
        else if (strstr(hTempObj->GetLogic(), "Tall")) ddActivationArea->setSelected(6);
        else ddActivationArea->setSelected(2);

        if (hTempObj->GetParam(WWD::Param_Smarts) > 0) {
            rbActivate[2]->setSelected(1);
            tfRespawnTimes->setEnabled(1);
            sprintf(tmp, "%d", hTempObj->GetParam(WWD::Param_Smarts));
            tfRespawnTimes->setText(tmp);
        } else if (hTempObj->GetParam(WWD::Param_Smarts) == 0) {
            rbActivate[1]->setSelected(1);
        } else {
            rbActivate[0]->setSelected(1);
        }

        if (cbDialog->isSelected()) {
            rbActivate[0]->setEnabled(0);
            rbActivate[1]->setEnabled(0);
            rbActivate[2]->setEnabled(0);
            tfRespawnTimes->setEnabled(0);
            rbActivate[1]->setSelected(1);
            labActivateMode->setColor(0xFF222222);
        }

        if (strlen(hTempObj->GetAnim()) == 0) {
            hTempObj->SetAnim(vsStandardSounds[0].first.c_str());
        }

        hPlayer = new cSoundPlayer(win, 5, 78 + 25,
                                   hState->hSndBank->GetAssetByID(hTempObj->GetAnim()),
                                   GETL2S("EditObj_Dialog", "Preview"));
    }

    cEditObjSoundTrigger::~cEditObjSoundTrigger() {
        for (int i = 0; i < 3; i++) delete rbActivate[i];
        delete lmSizes;
        delete cbDialog;
        delete labActivationArea;
        delete tfRespawnTimes;
        delete labActivateMode;
        delete tddOtherDialogs;
        delete ddStandardDialogs;
        delete rbStandard;
        delete rbOther;
        delete hPlayer;
        delete win;
        hState->vPort->MarkToRedraw(1);
    }

    void cEditObjSoundTrigger::Save() {
        if (rbActivate[0]->isSelected())
            hTempObj->SetParam(WWD::Param_Smarts, -1);
        else if (rbActivate[1]->isSelected())
            hTempObj->SetParam(WWD::Param_Smarts, 0);
        else
            hTempObj->SetParam(WWD::Param_Smarts, atoi(tfRespawnTimes->getText().c_str()));
    }

    void cEditObjSoundTrigger::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == rbStandard) {
            ddStandardDialogs->setEnabled(1);
            tddOtherDialogs->setEnabled(0);
            tddOtherDialogs->setMarkedInvalid(0);
            if (!tddOtherDialogs->isMarkedInvalid()) {
                for (int i = 0; i < vsStandardSounds.size(); i++) {
                    if (!strcmp(tddOtherDialogs->getText().c_str(), vsStandardSounds[i].first.c_str())) {
                        ddStandardDialogs->setSelected(i);
                        break;
                    }
                }
            }
            hTempObj->SetAnim(vsStandardSounds[ddStandardDialogs->getSelected()].first.c_str());
            hPlayer->SetAsset(
                    hState->hSndBank->GetAssetByID(vsStandardSounds[ddStandardDialogs->getSelected()].first.c_str()),
                    0);
        } else if (actionEvent.getSource() == rbOther) {
            ddStandardDialogs->setEnabled(0);
            tddOtherDialogs->setEnabled(1);
            tddOtherDialogs->setMarkedInvalid(hState->hSndBank->GetAssetByID(tddOtherDialogs->getText().c_str()) == 0);
        } else if (actionEvent.getSource() == ddStandardDialogs) {
            int x = ddStandardDialogs->getSelected();
            hTempObj->SetAnim(vsStandardSounds[x].first.c_str());
            hPlayer->SetAsset(hState->hSndBank->GetAssetByID(vsStandardSounds[x].first.c_str()), 0);
        } else if (actionEvent.getSource() == tddOtherDialogs) {
            tddOtherDialogs->setMarkedInvalid(hState->hSndBank->GetAssetByID(tddOtherDialogs->getText().c_str()) == 0);
            if (!tddOtherDialogs->isMarkedInvalid()) {
                hTempObj->SetAnim(tddOtherDialogs->getText().c_str());
                hPlayer->SetAsset(hState->hSndBank->GetAssetByID(tddOtherDialogs->getText().c_str()), 0);
            }
        } else if (actionEvent.getSource() == rbActivate[2]) {
            tfRespawnTimes->setEnabled(1);
        } else if (actionEvent.getSource() == rbActivate[0] || actionEvent.getSource() == rbActivate[1]) {
            tfRespawnTimes->setEnabled(0);
        } else if (actionEvent.getSource() == tfRespawnTimes) {
            if (atoi(tfRespawnTimes->getText().c_str()) == 0 && tfRespawnTimes->getText().c_str()[0] == '0') {
                char tmp[3] = "1";
                tfRespawnTimes->setText(tmp);
            }
        } else if (actionEvent.getSource() == ddActivationArea) {
            UpdateName();
        } else if (actionEvent.getSource() == cbDialog) {
            UpdateName();
            if (cbDialog->isSelected()) {
                rbActivate[1]->setSelected(1);
            }
            tfRespawnTimes->setEnabled(!cbDialog->isSelected() && rbActivate[2]->isSelected());
            rbActivate[0]->setEnabled(!cbDialog->isSelected());
            rbActivate[1]->setEnabled(!cbDialog->isSelected());
            rbActivate[2]->setEnabled(!cbDialog->isSelected());
            labActivateMode->setColor(cbDialog->isSelected() ? 0xFF222222 : 0xFF000000);
        }
    }

    void cEditObjSoundTrigger::UpdateName() {
        std::string dial = (cbDialog->isSelected() ? "ClawDialog" : "");
        std::string size;
        if (ddActivationArea->getSelected() == 0) size = "Tiny";
        else if (ddActivationArea->getSelected() == 1) size = "Small";
        else if (ddActivationArea->getSelected() == 2) size = "";
        else if (ddActivationArea->getSelected() == 3) size = "Big";
        else if (ddActivationArea->getSelected() == 4) size = "Huge";
        else if (ddActivationArea->getSelected() == 5) size = "Wide";
        else if (ddActivationArea->getSelected() == 6) size = "Tall";
        char nname[128];
        sprintf(nname, "%s%sSoundTrigger", dial.c_str(), size.c_str());
        hTempObj->SetLogic(nname);
    }

    void cEditObjSoundTrigger::Draw() {
        int dx, dy;
        win->getAbsolutePosition(dx, dy);

        hge->Gfx_RenderLine(dx, dy + 198, dx + win->getWidth(), dy + 198, GV->colLineDark);
        hge->Gfx_RenderLine(dx, dy + 199, dx + win->getWidth(), dy + 199, GV->colLineBright);

        hge->Gfx_RenderLine(dx, dy + 358, dx + win->getWidth(), dy + 358, GV->colLineDark);
        hge->Gfx_RenderLine(dx, dy + 359, dx + win->getWidth(), dy + 359, GV->colLineBright);

        for (int i = 0; i < 2; i++) {
            GV->fntMyriad13->SetColor(0xFF000000);
            GV->fntMyriad13->Render(dx + 754 + 137 * i, dy + 73, HGETEXT_LEFT, "X 1:", 0);
            GV->fntMyriad13->Render(dx + 754 + 137 * i, dy + 73 + 21, HGETEXT_LEFT, "Y 1:", 0);
            GV->fntMyriad13->Render(dx + 754 + 137 * i, dy + 73 + 42, HGETEXT_LEFT, "X 2:", 0);
            GV->fntMyriad13->Render(dx + 754 + 137 * i, dy + 73 + 63, HGETEXT_LEFT, "Y 2:", 0);
        }
    }

    std::string cEditObjSoundTrigger::getElementAt(int i) {
        if (i < 0 || i >= vsStandardSounds.size()) return "";
        return vsStandardSounds[i].second;
    }

    int cEditObjSoundTrigger::getNumberOfElements() {
        return vsStandardSounds.size();
    }

    std::string lmEditObjDialogSizes::getElementAt(int i) {
        if (i < 0 || i > 6) return "";
        char id[64];
        sprintf(id, "FirstRect%d", i + 1);
        return GETL2S("EditObj_Dialog", id);
    }

    int lmEditObjDialogSizes::getNumberOfElements() {
        return 7;
    }
}
