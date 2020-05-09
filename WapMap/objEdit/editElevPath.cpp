#include "editElevPath.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../../shared/commonFunc.h"
#include "../cObjectUserData.h"
#include "../databanks/imageSets.h"

extern HGE *hge;

namespace ObjEdit {
    std::string lmImgSetEditObjElevPath::getElementAt(int i) {
        if (GV->editState->hParser->GetBaseLevel() == 1) {
            return "LEVEL_ELEVATORS";
        } else if (GV->editState->hParser->GetBaseLevel() == 13) {
            return "LEVEL_ELEVATOR1";
        } else if (GV->editState->hParser->GetBaseLevel() == 2 ||
                   GV->editState->hParser->GetBaseLevel() == 4 ||
                   GV->editState->hParser->GetBaseLevel() == 5 ||
                   GV->editState->hParser->GetBaseLevel() == 9 ||
                   GV->editState->hParser->GetBaseLevel() == 11 ||
                   GV->editState->hParser->GetBaseLevel() == 12 ||
                   GV->editState->hParser->GetBaseLevel() == 14) {
            return "LEVEL_ELEVATOR";
        } else if (GV->editState->hParser->GetBaseLevel() == 3) {
            if (i == 0) return "LEVEL_ELEVATOR1";
            else if (i == 1) return "LEVEL_ELEVATOR2";
        } else if (GV->editState->hParser->GetBaseLevel() == 6) {
            return "LEVEL_GRILLELEVATOR";
        } else if (GV->editState->hParser->GetBaseLevel() == 10) {
            if (i == 0) return "LEVEL_TRAPELEVATOR";
            else return "LEVEL_ELEVATOR1";
        } else if (GV->editState->hParser->GetBaseLevel() == 7) {
            return "LEVEL_AIRCART";
        } else if (GV->editState->hParser->GetBaseLevel() == 8) {
            return "LEVEL_SINKPOT";
        }
        return "";
    }

    int lmImgSetEditObjElevPath::getNumberOfElements() {
        if (GV->editState->hParser->GetBaseLevel() == 3 ||
            GV->editState->hParser->GetBaseLevel() == 10)
            return 2;
        return 1;
    }

    std::string lmEditObjElevPath::getElementAt(int i) {
        char tmp[64];
        if (m_hOwn->vSteps[i].first == 5) {
            sprintf(tmp, "%s (%dms)", GETL2S("EditObj_ElevPath", "Wait"), m_hOwn->vSteps[i].second);
        } else {
            sprintf(tmp, "%s (X: %d, Y: %d)", GETL2S("EditObj_ElevPath", "Move"), m_hOwn->GetMoveX(i),
                    m_hOwn->GetMoveY(i));
        }
        return tmp;
    }

    int lmEditObjElevPath::getNumberOfElements() {
        return m_hOwn->vSteps.size();
    }

    int cEditObjElevPath::GetMoveX(int i) {
        if (vSteps[i].first == 8 || vSteps[i].first == 2) return 0;
        if (vSteps[i].first == 7 || vSteps[i].first == 4 || vSteps[i].first == 1)
            return -vSteps[i].second;
        if (vSteps[i].first == 9 || vSteps[i].first == 6 || vSteps[i].first == 3)
            return vSteps[i].second;
        return 0;
    }

    int cEditObjElevPath::GetMoveY(int i) {
        if (vSteps[i].first == 4 || vSteps[i].first == 6) return 0;
        if (vSteps[i].first == 7 || vSteps[i].first == 8 || vSteps[i].first == 9)
            return -vSteps[i].second;
        if (vSteps[i].first == 1 || vSteps[i].first == 2 || vSteps[i].first == 3)
            return vSteps[i].second;
        return 0;
    }

    cEditObjElevPath::cEditObjElevPath(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        iType = ObjEdit::enPathElevator;
        if (hTempObj->GetParam(WWD::Param_Powerup) == 1) {
            vSteps.push_back(std::pair<int, int>(6, 100));
            vSteps.push_back(std::pair<int, int>(7, 100));
            vSteps.push_back(std::pair<int, int>(6, 100));
            vSteps.push_back(std::pair<int, int>(1, 100));
        } else if (hTempObj->GetParam(WWD::Param_Powerup) == 2) {
            vSteps.push_back(std::pair<int, int>(6, 100));
            vSteps.push_back(std::pair<int, int>(7, 100));
            vSteps.push_back(std::pair<int, int>(5, 1000));
            vSteps.push_back(std::pair<int, int>(6, 100));
            vSteps.push_back(std::pair<int, int>(1, 100));
        } else {
            WWD::Rect tmprect = hTempObj->GetMoveRect();
            if (tmprect.y1 != 0)
                vSteps.push_back(std::pair<int, int>(tmprect.x1, tmprect.y1));
            if (tmprect.y2 != 0)
                vSteps.push_back(std::pair<int, int>(tmprect.x2, tmprect.y2));
            tmprect = hTempObj->GetHitRect();
            if (tmprect.y1 != 0)
                vSteps.push_back(std::pair<int, int>(tmprect.x1, tmprect.y1));
            if (tmprect.y2 != 0)
                vSteps.push_back(std::pair<int, int>(tmprect.x2, tmprect.y2));
            tmprect = hTempObj->GetAttackRect();
            if (tmprect.y1 != 0)
                vSteps.push_back(std::pair<int, int>(tmprect.x1, tmprect.y1));
            if (tmprect.y2 != 0)
                vSteps.push_back(std::pair<int, int>(tmprect.x2, tmprect.y2));
            tmprect = hTempObj->GetClipRect();
            if (tmprect.y1 != 0)
                vSteps.push_back(std::pair<int, int>(tmprect.x1, tmprect.y1));
            if (tmprect.y2 != 0)
                vSteps.push_back(std::pair<int, int>(tmprect.x2, tmprect.y2));
        }
        if (vSteps.size() == 0) vSteps.push_back(std::pair<int, int>(5, 0));

        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_ElevPath", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 250 - 6, 570 - 23 - 9 + 36 + 35));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY());

        butInvX = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_ElevPath", "InvertChainX"));
        butInvX->setDimension(gcn::Rectangle(0, 0, 115, 33));
        butInvX->addActionListener(hAL);
        win->add(butInvX, 5, 505 - 26);

        butInvY = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_ElevPath", "InvertChainY"));
        butInvY->setDimension(gcn::Rectangle(0, 0, 115, 33));
        butInvY->addActionListener(hAL);
        win->add(butInvY, 120, 505 - 26);

        butInvertPath = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_ElevPath", "InvertChain"));
        butInvertPath->setDimension(gcn::Rectangle(0, 0, 115, 33));
        butInvertPath->addActionListener(hAL);
        win->add(butInvertPath, 5, 505 - 26 + 35);

        butCloseChain = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_ElevPath", "CloseChain"));
        butCloseChain->setDimension(gcn::Rectangle(0, 0, 115, 33));
        butCloseChain->addActionListener(hAL);
        win->add(butCloseChain, 120, 505 - 26 + 35);

        butGenChain = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_ElevPath", "GenerateChain"));
        butGenChain->setDimension(gcn::Rectangle(0, 0, 115, 33));
        butGenChain->addActionListener(hAL);
        win->add(butGenChain, 5, 505 - 23 + 36 + 35);

        win->add(_butSave, 120, 505 - 23 + 36 + 35);

        int iOrigX = hTempObj->GetParam(WWD::Param_LocationX), iOrigY = hTempObj->GetParam(WWD::Param_LocationY);
        int iModX = iOrigX, iModY = iOrigY;
        for (int i = 0; i < vSteps.size(); i++) {
            iModX += GetMoveX(i);
            iModY += GetMoveY(i);
        }

        butGenChain->setEnabled(iModX == iOrigX && iModY == iOrigY);
        butCloseChain->setEnabled(iModX != iOrigX || iModY != iOrigY);

        int y = 15;
        labImgSet = new SHR::Lab(GETL2S("EditObj_ElevPath", "ImgSet"));
        labImgSet->adjustSize();
        win->add(labImgSet, 5, y);
        y += 20;

        lmDefImgSets = new lmImgSetEditObjElevPath();
        tddImageSet = new SHR::TextDropDown(hTempObj->GetImageSet(), lmDefImgSets);
        tddImageSet->setDimension(gcn::Rectangle(0, 0, 200, 20));
        tddImageSet->SetGfx(&GV->gcnParts);
        tddImageSet->addActionListener(hAL);
        win->add(tddImageSet, 5, y);
        y += 40;

        labSpeed = new SHR::Lab(GETL2S("EditObj_ElevPath", "Speed"));
        labSpeed->adjustSize();
        win->add(labSpeed, 5, y);
        y += 20;

        char tmpstr[256];
        sprintf(tmpstr, "%d", hTempObj->GetParam(WWD::Param_Speed) == 0 ? 125 : hTempObj->GetParam(WWD::Param_Speed));
        tfSpeed = new SHR::TextField(tmpstr);
        tfSpeed->SetNumerical(1, 0);
        tfSpeed->setDimension(gcn::Rectangle(0, 0, 100, 20));
        win->add(tfSpeed, 5, y);
        y += 40;

        labSteps = new SHR::Lab(GETL2S("EditObj_ElevPath", "Steps"));
        labSteps->adjustSize();
        win->add(labSteps, 5, y);
        y += 20;

        win->add(vpAdv);

        lmSteps = new lmEditObjElevPath(this);
        lbSteps = new SHR::ListBox(lmSteps);
        lbSteps->setDimension(gcn::Rectangle(0, 0, 230, 150));
        lbSteps->addActionListener(hAL);
        lbSteps->setSelected(0);
        win->add(lbSteps, 5, y);
        y += 160;
        butStepAdd = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_ElevPath", "AddStep"));
        butStepAdd->setDimension(gcn::Rectangle(0, 0, 115, 33));
        butStepAdd->addActionListener(hAL);
        win->add(butStepAdd, 5, y);
        butStepDel = new SHR::But(GV->hGfxInterface, GETL2S("EditObj_ElevPath", "DeleteStep"));
        butStepDel->setDimension(gcn::Rectangle(0, 0, 115, 33));
        butStepDel->addActionListener(hAL);
        win->add(butStepDel, 120, y);
        y += 40;

        labDir = new SHR::Lab(GETL2S("EditObj_ElevPath", "Direction"));
        labDir->adjustSize();
        win->add(labDir, 125, y);

        butMoveDir[0] = hState->MakeButton(125, y + 20, Icon_UpLeft, win);
        butMoveDir[1] = hState->MakeButton(125 + 32, y + 20, Icon_Up, win);
        butMoveDir[2] = hState->MakeButton(125 + 64, y + 20, Icon_UpRight, win);
        butMoveDir[3] = hState->MakeButton(125, y + 20 + 32, Icon_Left, win);
        butMoveDir[4] = hState->MakeButton(125 + 64, y + 20 + 32, Icon_Right, win);
        butMoveDir[5] = hState->MakeButton(125, y + 20 + 64, Icon_DownLeft, win);
        butMoveDir[6] = hState->MakeButton(125 + 32, y + 20 + 64, Icon_Down, win);
        butMoveDir[7] = hState->MakeButton(125 + 64, y + 20 + 64, Icon_DownRight, win);
        for (int i = 0; i < 8; i++)
            butMoveDir[i]->addActionListener(hAL);

        labType = new SHR::Lab(GETL2S("EditObj_ElevPath", "Type"));
        labType->adjustSize();
        win->add(labType, 5, y);
        y += 20;

        sprintf(tmpstr, "%p", this);
        rbMove = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_ElevPath", "Move"), tmpstr, (vSteps[0].first != 5));
        rbMove->adjustSize();
        rbMove->addActionListener(hAL);
        win->add(rbMove, 5, y);
        y += 20;
        rbWait = new SHR::RadBut(GV->hGfxInterface, GETL2S("EditObj_ElevPath", "Wait"), tmpstr, (vSteps[0].first == 5));
        rbWait->adjustSize();
        rbWait->addActionListener(hAL);
        win->add(rbWait, 5, y);
        y += 36;

        labMove = new SHR::Lab(GETL2S("EditObj_ElevPath", "Movement"));
        labMove->adjustSize();
        win->add(labMove, 5, y);

        labWaitTime = new SHR::Lab(GETL2S("EditObj_ElevPath", "WaitTime"));
        labWaitTime->adjustSize();
        win->add(labWaitTime, 5, y);
        y += 20;

        tfStepMove = new SHR::TextField("");
        tfStepMove->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tfStepMove->addActionListener(hAL);
        tfStepMove->SetNumerical(1, 0);
        win->add(tfStepMove, 5, y);

        tfStepWaitTime = new SHR::TextField("");
        tfStepWaitTime->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tfStepWaitTime->addActionListener(hAL);
        tfStepWaitTime->SetNumerical(1, 0);
        win->add(tfStepWaitTime, 5, y);
        y += 20;

        ChangeActiveStep();
        if (vSteps.size() == 8) butStepAdd->setEnabled(0);
        if (vSteps.size() == 1) butStepDel->setEnabled(0);

        bDragging = 0;
    }

    void cEditObjElevPath::ChangeActiveStep() {
        labMove->setVisible(vSteps[lbSteps->getSelected()].first != 5);
        tfStepMove->setVisible(vSteps[lbSteps->getSelected()].first != 5);
        labWaitTime->setVisible(vSteps[lbSteps->getSelected()].first == 5);
        tfStepWaitTime->setVisible(vSteps[lbSteps->getSelected()].first == 5);
        for (int i = 0; i < 8; i++)
            butMoveDir[i]->setVisible(vSteps[lbSteps->getSelected()].first != 5);
        labDir->setVisible(vSteps[lbSteps->getSelected()].first != 5);
        if (vSteps[lbSteps->getSelected()].first == 5) {
            char tmpstr[64];
            sprintf(tmpstr, "%d", vSteps[lbSteps->getSelected()].second);
            tfStepWaitTime->setText(tmpstr);
            rbWait->setSelected(1);
        } else {
            char tmpstr[64];
            sprintf(tmpstr, "%d", vSteps[lbSteps->getSelected()].second);
            tfStepMove->setText(tmpstr);
            rbMove->setSelected(1);
            int dir = vSteps[lbSteps->getSelected()].first;
            butMoveDir[0]->setIconColor(dir == 7 ? 0xFFFFFFFF : 0x55FFFFFF);
            butMoveDir[1]->setIconColor(dir == 8 ? 0xFFFFFFFF : 0x55FFFFFF);
            butMoveDir[2]->setIconColor(dir == 9 ? 0xFFFFFFFF : 0x55FFFFFF);
            butMoveDir[3]->setIconColor(dir == 4 ? 0xFFFFFFFF : 0x55FFFFFF);
            butMoveDir[4]->setIconColor(dir == 6 ? 0xFFFFFFFF : 0x55FFFFFF);
            butMoveDir[5]->setIconColor(dir == 1 ? 0xFFFFFFFF : 0x55FFFFFF);
            butMoveDir[6]->setIconColor(dir == 2 ? 0xFFFFFFFF : 0x55FFFFFF);
            butMoveDir[7]->setIconColor(dir == 3 ? 0xFFFFFFFF : 0x55FFFFFF);
        }
    }

    cEditObjElevPath::~cEditObjElevPath() {
        delete butInvX;
        delete butInvY;
        delete tddImageSet;
        delete butInvertPath;
        delete labMove;
        delete labWaitTime;
        delete labType;
        delete labSteps;
        delete labSpeed;
        delete labImgSet;
        delete labDir;
        delete rbMove;
        delete rbWait;
        for (int i = 0; i < 8; i++) delete butMoveDir[i];
        delete tfStepMove;
        delete tfStepWaitTime;
        delete tfSpeed;
        delete lbSteps;
        delete lmDefImgSets;
        delete lmSteps;
        delete butStepAdd;
        delete butStepDel;
        delete butGenChain;
        delete butCloseChain;
        delete win;
        hState->vPort->MarkToRedraw(1);
    }

    int cEditObjElevPath::GetMoveDir(int x, int y) {
        if (x < 0 && y < 0) return 7;
        if (x == 0 && y < 0) return 8;
        if (x > 0 && y < 0) return 9;
        if (x < 0 && y == 0) return 4;
        if (x == 0 && y == 0) return 5;
        if (x > 0 && y == 0) return 6;
        if (x < 0 && y > 0) return 1;
        if (x == 0 && y > 0) return 2;
        if (x > 0 && y > 0) return 3;
        return 0;
    }

    void cEditObjElevPath::CloseChain(bool bChangeQuestioned) {
        bool bOneStep = 1;
        int iOrigX = hTempObj->GetParam(WWD::Param_LocationX), iOrigY = hTempObj->GetParam(WWD::Param_LocationY);
        int iEndX = iOrigX, iEndY = iOrigY;
        for (int i = 0; i < vSteps.size(); i++) {
            iEndX += GetMoveX(i);
            iEndY += GetMoveY(i);
        }
        int iDiffX = iOrigX - iEndX, iDiffY = iOrigY - iEndY;
        if (abs(iDiffX) != abs(iDiffY) && iDiffX != 0 && iDiffY != 0)
            bOneStep = 0;
        int iNewStepDir = GetMoveDir(iDiffX, iDiffY);
        if (bOneStep) {
            if (vSteps.back().first == iNewStepDir) {
                vSteps.back().first = iNewStepDir;
                vSteps.back().second += (iDiffX != 0 ? abs(iDiffX) : abs(iDiffY));
                SynchronizeObj();
                ChangeActiveStep();
                return;
            }
            if (vSteps.size() == 8) {
                if (!bChangeQuestioned) {
                    bChangeQuestioned = 1;
                    if (MessageBox(hge->System_GetState(HGE_HWND), GETL2S("EditObj_ElevPath", "MsgCloseChainStepCut"),
                                   GETL(Lang_Message), MB_YESNO | MB_ICONINFORMATION) == IDNO)
                        return;
                }
                vSteps.pop_back();
                CloseChain(bChangeQuestioned);
                return;
            }
            vSteps.push_back(std::pair<int, int>(iNewStepDir, (iDiffX != 0 ? abs(iDiffX) : abs(iDiffY))));
            SynchronizeObj();
            ChangeActiveStep();
            return;
        } else {
            if (vSteps.size() < 7) {
                int minmove = (abs(iDiffX) > abs(iDiffY) ? abs(iDiffY) : abs(iDiffX));
                int mindir = GetMoveDir(iDiffX, iDiffY);
                vSteps.push_back(std::pair<int, int>(mindir, minmove));
                SynchronizeObj();
                ChangeActiveStep();
                CloseChain(bChangeQuestioned);
                return;
            }
            if (!bChangeQuestioned) {
                bChangeQuestioned = 1;
                if (MessageBox(hge->System_GetState(HGE_HWND), GETL2S("EditObj_ElevPath", "MsgCloseChainStepCut"),
                               GETL(Lang_Message), MB_YESNO | MB_ICONINFORMATION) == IDNO)
                    return;
            }
            vSteps.pop_back();
            SynchronizeObj();
            ChangeActiveStep();
            CloseChain(bChangeQuestioned);
            return;
        }
    }

    void cEditObjElevPath::Save() {
        int initX = hTempObj->GetParam(WWD::Param_LocationX), initY = hTempObj->GetParam(WWD::Param_LocationY);
        int endX = initX, endY = initY;
        for (int i = 0; i < vSteps.size(); i++) {
            endX += GetMoveX(i);
            endY += GetMoveY(i);
        }
        if ((endX != initX || endY != initY) &&
            MessageBox(hge->System_GetState(HGE_HWND), GETL2S("EditObj_ElevPath", "MsgDifferentEnd"),
                       GETL(Lang_Message), MB_YESNO | MB_ICONWARNING) == IDNO)
            return;
        int tmp = atoi(tfSpeed->getText().c_str());
        if (tmp <= 0)
            tmp = 125;
        hTempObj->SetParam(WWD::Param_Speed, tmp);
    }

    void cEditObjElevPath::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        } else if (actionEvent.getSource() == lbSteps) {
            ChangeActiveStep();
        } else if (actionEvent.getSource() == butStepAdd) {
            vSteps.insert(vSteps.begin() + lbSteps->getSelected() + 1, std::pair<int, int>(2, 100));
            lbSteps->setSelected(lbSteps->getSelected() + 1);
            ChangeActiveStep();
            if (vSteps.size() == 8) butStepAdd->setEnabled(0);
            if (vSteps.size() > 1) butStepDel->setEnabled(1);
            SynchronizeObj();
        } else if (actionEvent.getSource() == butStepDel) {
            bool boff = 0;
            if (lbSteps->getSelected() == vSteps.size() - 1) {
                lbSteps->setSelected(lbSteps->getSelected() - 1);
                boff = 1;
            }
            vSteps.erase(vSteps.begin() + lbSteps->getSelected() + boff);
            SynchronizeObj();
            if (vSteps.size() < 8) butStepAdd->setEnabled(1);
            if (vSteps.size() == 1) butStepDel->setEnabled(0);
        } else if (actionEvent.getSource() == tfStepMove) {
            vSteps[lbSteps->getSelected()].second = atoi(tfStepMove->getText().c_str());
            SynchronizeObj();
        } else if (actionEvent.getSource() == tfStepWaitTime) {
            vSteps[lbSteps->getSelected()].second = atoi(tfStepWaitTime->getText().c_str());
            SynchronizeObj();
        } else if (actionEvent.getSource() == rbMove) {
            vSteps[lbSteps->getSelected()].first = 2;
            vSteps[lbSteps->getSelected()].second = 100;
            ChangeActiveStep();
            SynchronizeObj();
        } else if (actionEvent.getSource() == rbWait) {
            vSteps[lbSteps->getSelected()].first = 5;
            vSteps[lbSteps->getSelected()].second = 100;
            ChangeActiveStep();
            SynchronizeObj();
        } else if (actionEvent.getSource() == tddImageSet) {
            hTempObj->SetImageSet(tddImageSet->getText().c_str());
            hState->vPort->MarkToRedraw(1);
        } else if (actionEvent.getSource() == butGenChain) {
            int tmp = atoi(tfSpeed->getText().c_str());
            if (tmp <= 0)
                tmp = 125;
            hTempObj->SetParam(WWD::Param_Speed, tmp);

            if (ChangesMade())
                if (MessageBox(hge->System_GetState(HGE_HWND), GETL2S("EditObj_ElevPath", "MsgGenChainChangesMade"),
                               GETL(Lang_Message), MB_YESNO | MB_ICONINFORMATION) == IDNO)
                    return;

            int iPosX = hTempObj->GetParam(WWD::Param_LocationX), iPosY = hTempObj->GetParam(WWD::Param_LocationY);
            int iModX = iPosX, iModY = iPosY;
            bool bNewObjCreated = 0;
            for (int i = 0; i < vSteps.size() - 1; i++) {
                if (vSteps[0].first == 5) {
                    std::pair<int, int> tmppair = vSteps[0];
                    vSteps.erase(vSteps.begin());
                    vSteps.push_back(tmppair);
                    continue;
                }
                iModX += GetMoveX(0);
                iModY += GetMoveY(0);
                std::pair<int, int> tmppair = vSteps[0];
                vSteps.erase(vSteps.begin());
                vSteps.push_back(tmppair);
                hTempObj->SetParam(WWD::Param_LocationX, iModX);
                hTempObj->SetParam(WWD::Param_LocationY, iModY);
                SynchronizeObj();
                WWD::Object *nobj = new WWD::Object(hTempObj);
                hState->GetActivePlane()->AddObjectAndCalcID(nobj);
                nobj->SetUserData(new cObjUserData(nobj));
                hState->hPlaneData[hState->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(nobj);
                hState->vObjectsPicked.push_back(nobj);
                bNewObjCreated = 1;
            }
            vSteps.push_back(vSteps[0]);
            vSteps.erase(vSteps.begin());
            hTempObj->SetParam(WWD::Param_LocationX, iPosX);
            hTempObj->SetParam(WWD::Param_LocationY, iPosY);
            SynchronizeObj();
            _butSave->simulatePress();
            if (bNewObjCreated)
                hState->MarkUnsaved();
            return;
        } else if (actionEvent.getSource() == butCloseChain) {
            CloseChain();
        } else if (actionEvent.getSource() == butInvertPath ||
                   actionEvent.getSource() == butInvX ||
                   actionEvent.getSource() == butInvY) {
            bool bX = actionEvent.getSource() == butInvertPath || actionEvent.getSource() == butInvY,
                    bY = actionEvent.getSource() == butInvertPath || actionEvent.getSource() == butInvX;
            std::vector<std::pair<int, int> > vOrigSteps = vSteps;
            vSteps.clear();
            for (int i = vOrigSteps.size() - 1; i >= 0; i--) {
                std::pair<int, int> nstep;
                nstep = vOrigSteps[i];
                if (nstep.first == 7) {
                    if (bX && bY) nstep.first = 3;
                    else if (bX) nstep.first = 9;
                    else if (bY) nstep.first = 1;
                } else if (nstep.first == 8 && bY) nstep.first = 2;
                else if (nstep.first == 9) {
                    if (bX && bY) nstep.first = 1;
                    else if (bX) nstep.first = 7;
                    else if (bY) nstep.first = 3;
                } else if (nstep.first == 4 && bX) nstep.first = 6;
                else if (nstep.first == 6 && bX) nstep.first = 4;
                else if (nstep.first == 1) {
                    if (bX && bY) nstep.first = 9;
                    else if (bX) nstep.first = 3;
                    else if (bY) nstep.first = 7;
                } else if (nstep.first == 2 && bY) nstep.first = 8;
                else if (nstep.first == 3) {
                    if (bX && bY) nstep.first = 7;
                    else if (bX) nstep.first = 1;
                    else if (bY) nstep.first = 9;
                }
                vSteps.push_back(nstep);
                SynchronizeObj();
                ChangeActiveStep();
            }
        }
        for (int i = 0; i < 8; i++)
            if (actionEvent.getSource() == butMoveDir[i]) {
                for (int z = 0; z < 8; z++)
                    butMoveDir[z]->setIconColor(0x55FFFFFF);
                butMoveDir[i]->setIconColor(0xFFFFFFFF);
                int dir = 0;
                if (i == 0) dir = 7;
                else if (i == 1) dir = 8;
                else if (i == 2) dir = 9;
                else if (i == 3) dir = 4;
                else if (i == 4) dir = 6;
                else if (i == 5) dir = 1;
                else if (i == 6) dir = 2;
                else if (i == 7) dir = 3;
                vSteps[lbSteps->getSelected()].first = dir;
                SynchronizeObj();
                break;
            }
    }

    void cEditObjElevPath::SynchronizeObj() {
        hTempObj->SetParam(WWD::Param_Powerup, 0);
        hTempObj->SetMoveRect(WWD::Rect(0, 0, 0, 0));
        hTempObj->SetHitRect(WWD::Rect(0, 0, 0, 0));
        hTempObj->SetAttackRect(WWD::Rect(0, 0, 0, 0));
        hTempObj->SetClipRect(WWD::Rect(0, 0, 0, 0));
        WWD::Rect tmp(0, 0, 0, 0);
        int iRect = 0;
        bool bSwitchRect = 0;
        int iOrigX = hTempObj->GetParam(WWD::Param_LocationX), iOrigY = hTempObj->GetParam(WWD::Param_LocationY);
        int iModX = iOrigX, iModY = iOrigY;
        for (int i = 0; i < vSteps.size(); i++) {
            iModX += GetMoveX(i);
            iModY += GetMoveY(i);
            if (vSteps[i].first != 0 && vSteps[i].second != 0) {
                if (!bSwitchRect) {
                    tmp.x1 = vSteps[i].first;
                    tmp.y1 = vSteps[i].second;
                } else {
                    tmp.x2 = vSteps[i].first;
                    tmp.y2 = vSteps[i].second;
                    if (iRect == 0) {
                        hTempObj->SetMoveRect(tmp);
                    } else if (iRect == 1) {
                        hTempObj->SetHitRect(tmp);
                    } else if (iRect == 2) {
                        hTempObj->SetAttackRect(tmp);
                    } else if (iRect == 3) {
                        hTempObj->SetClipRect(tmp);
                    }
                    iRect++;
                    tmp = WWD::Rect(0, 0, 0, 0);
                }
                bSwitchRect = !bSwitchRect;
            }
        }
        if (tmp.x1 != 0 || tmp.y1 != 0) {
            if (iRect == 0) {
                hTempObj->SetMoveRect(tmp);
            } else if (iRect == 1) {
                hTempObj->SetHitRect(tmp);
            } else if (iRect == 2) {
                hTempObj->SetAttackRect(tmp);
            } else if (iRect == 3) {
                hTempObj->SetClipRect(tmp);
            }
        }
        butGenChain->setEnabled(iModX == iOrigX && iModY == iOrigY);
        butCloseChain->setEnabled(iModX != iOrigX || iModY != iOrigY);
        butInvertPath->setEnabled(!butCloseChain->isEnabled());
    }

    void cEditObjElevPath::_Think(bool bMouseConsumed) {
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        bool bMouseIn = 1;
        if (mx < hState->vPort->GetX() || my < hState->vPort->GetY() ||
            mx > hState->vPort->GetX() + hState->vPort->GetWidth() ||
            my > hState->vPort->GetY() + hState->vPort->GetHeight())
            bMouseIn = 0;
        int imgw, imgh;
        hgeSprite *spr = hState->SprBank->GetObjectSprite(hTempObj);
        imgw = spr->GetWidth();
        imgh = spr->GetHeight();
        int origx = hTempObj->GetParam(WWD::Param_LocationX),
                origy = hTempObj->GetParam(WWD::Param_LocationY);
        mx = hState->Scr2WrdX(hState->GetActivePlane(), mx);
        my = hState->Scr2WrdY(hState->GetActivePlane(), my);
        if (bDragging) {
            if (!hge->Input_GetKeyState(HGEK_LBUTTON)) {
                bDragging = 0;
                bAllowDragging = 1;
            }
            if (!bMouseIn || bMouseConsumed || hge->Input_KeyDown(HGEK_RBUTTON)) {
                vSteps[iDragStep].first = iDragStepOrigDir;
                vSteps[iDragStep].second = iDragStepOrigMove;
            } else {
                int diffx = mx - iDragStartX, diffy = my - iDragStartY;
                float ratio = float(diffx) / float(diffy);
                int move = 0, dir = 0;
                if (ratio > 2.5 || ratio < -2.5) {
                    dir = diffx > 0 ? 6 : 4;
                    move = diffx;
                } else if (ratio < 0.5 && ratio > -0.5) {
                    dir = diffy > 0 ? 2 : 8;
                    move = diffy;
                } else {
                    if (diffx < 0 && diffy < 0) dir = 7;
                    else if (diffx > 0 && diffy > 0) dir = 3;
                    else if (diffx > 0 && diffy < 0) dir = 9;
                    else if (diffx < 0 && diffy > 0) dir = 1;
                    diffx = abs(diffx);
                    diffy = abs(diffy);
                    if (diffx > diffy) move = diffx;
                    else if (diffy > diffx) move = diffy;
                    else if (diffx == diffy) move = diffx;
                }
                if (dir != 0) {
                    vSteps[iDragStep].first = dir;
                    vSteps[iDragStep].second = abs(move);
                    SynchronizeObj();
                    ChangeActiveStep();
                    hState->vPort->MarkToRedraw(1);
                }
            }
            return;
        }
        if (bMouseConsumed || !bMouseIn) return;
        int modx = 0, mody = 0;
        for (int i = 0; i < vSteps.size(); i++) {
            if (vSteps[i].first == 5) continue;
            modx += GetMoveX(i);
            mody += GetMoveY(i);
            if (mx > origx + modx - imgw / 2 && mx < origx + modx + imgw / 2 &&
                my > origy + mody - imgh / 2 && my < origy + mody + imgh / 2 &&
                hge->Input_KeyDown(HGEK_LBUTTON)) {
                bAllowDragging = 0;
                if (lbSteps->getSelected() == i) {
                    bDragging = 1;
                    iDragStartX = mx - GetMoveX(i);
                    iDragStartY = my - GetMoveY(i);
                    iDragStepOrigDir = vSteps[i].first;
                    iDragStepOrigMove = vSteps[i].second;
                    iDragStep = i;
                } else {
                    lbSteps->setSelected(i);
                    ChangeActiveStep();
                    return;
                }
            }
        }
    }

    void cEditObjElevPath::Draw() {
        int dx, dy;
        win->getAbsolutePosition(dx, dy);

        hge->Gfx_RenderLine(dx, dy + 145, dx + win->getWidth(), dy + 145, 0xFF1f1f1f);
        hge->Gfx_RenderLine(dx, dy + 146, dx + win->getWidth(), dy + 146, 0xFF5c5c5c);

        hgeQuad q;
        q.tex = 0;
        q.blend = BLEND_DEFAULT;
        SHR::SetQuad(&q, 0xFF545454,
                     dx + lbSteps->getX() + 2, dy + lbSteps->getY() + 16,
                     dx + lbSteps->getX() + lbSteps->getWidth() + 2, dy + lbSteps->getY() + 152 + 16);
        hge->Gfx_RenderQuad(&q);

        hge->Gfx_RenderLine(dx + lbSteps->getX(), dy + lbSteps->getY() + 15,
                            dx + lbSteps->getX() + lbSteps->getWidth() + 3, dy + lbSteps->getY() + 15, 0xFF1f1f1f);
        hge->Gfx_RenderLine(dx + lbSteps->getX(), dy + lbSteps->getY() + 16,
                            dx + lbSteps->getX() + lbSteps->getWidth() + 4, dy + lbSteps->getY() + 16, 0xFF5c5c5c);
        hge->Gfx_RenderLine(dx + lbSteps->getX(), dy + lbSteps->getY() + 152 + 17,
                            dx + lbSteps->getX() + lbSteps->getWidth() + 3, dy + lbSteps->getY() + 152 + 17,
                            0xFF1f1f1f);
        hge->Gfx_RenderLine(dx + lbSteps->getX(), dy + lbSteps->getY() + 152 + 18,
                            dx + lbSteps->getX() + lbSteps->getWidth() + 4, dy + lbSteps->getY() + 152 + 18,
                            0xFF5c5c5c);

        hge->Gfx_RenderLine(dx + lbSteps->getX() + 1, dy + lbSteps->getY() + 15, dx + lbSteps->getX() + 1,
                            dy + lbSteps->getY() + 152 + 16, 0xFF1f1f1f);
        hge->Gfx_RenderLine(dx + lbSteps->getX() + 2, dy + lbSteps->getY() + 16, dx + lbSteps->getX() + 2,
                            dy + lbSteps->getY() + 152 + 16, 0xFF5c5c5c);
        hge->Gfx_RenderLine(dx + lbSteps->getX() + lbSteps->getWidth() + 3, dy + lbSteps->getY() + 15,
                            dx + lbSteps->getX() + lbSteps->getWidth() + 3, dy + lbSteps->getY() + 152 + 16,
                            0xFF1f1f1f);
        hge->Gfx_RenderLine(dx + lbSteps->getX() + lbSteps->getWidth() + 4, dy + lbSteps->getY() + 15,
                            dx + lbSteps->getX() + lbSteps->getWidth() + 4, dy + lbSteps->getY() + 152 + 16,
                            0xFF5c5c5c);

        hge->Gfx_RenderLine(dx, dy + 368, dx + win->getWidth(), dy + 368, 0xFF1f1f1f);
        hge->Gfx_RenderLine(dx, dy + 369, dx + win->getWidth(), dy + 369, 0xFF5c5c5c);

        hge->Gfx_RenderLine(dx, dy + 515 - 23, dx + win->getWidth(), dy + 515 - 23, 0xFF1f1f1f);
        hge->Gfx_RenderLine(dx, dy + 516 - 23, dx + win->getWidth(), dy + 516 - 23, 0xFF5c5c5c);
        hge->Gfx_RenderLine(dx, dy + 515 - 23 + 40 + 35, dx + win->getWidth(), dy + 515 - 23 + 40 + 35, 0xFF1f1f1f);
        hge->Gfx_RenderLine(dx, dy + 516 - 23 + 40 + 35, dx + win->getWidth(), dy + 516 - 23 + 40 + 35, 0xFF5c5c5c);
    }
}
