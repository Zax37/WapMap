#include "editTogglePeg.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include "../databanks/imageSets.h"

extern HGE *hge;

namespace ObjEdit {
    cEditObjTogglePeg::cEditObjTogglePeg(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        int baselvl = GV->editState->hParser->GetBaseLevel();
        if (baselvl == 1 || baselvl == 8 || baselvl == 10 || baselvl == 12)
            vszDefaultImgsets.push_back("LEVEL_PEG");
        else if (baselvl == 2 || baselvl == 11)
            vszDefaultImgsets.push_back("LEVEL_PEGSLIDER");
        else if (baselvl == 5)
            vszDefaultImgsets.push_back("LEVEL_SLIDAWAYPLANK");
        else if (baselvl == 13)
            vszDefaultImgsets.push_back("LEVEL_BOSSPEG");
        else if (baselvl == 14)
            vszDefaultImgsets.push_back("LEVEL_SLIDERPEG");
        if (baselvl == 8) {
            vszDefaultImgsets.push_back("LEVEL_SINKBOEY");
            vszDefaultImgsets.push_back("LEVEL_CANNON2");
        }

        iType = ObjEdit::enTogglePeg;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_TogglePeg", "WinCaption"));
        win->setDimension(gcn::Rectangle(0, 0, 250, 185 + 64 * (!vszDefaultImgsets.empty())));
        win->setClose(1);
        win->addActionListener(hAL);
        win->add(vpAdv);
        win->setMovable(0);
        st->conMain->add(win, st->vPort->GetX(), st->vPort->GetY() + st->vPort->GetHeight() - win->getHeight());

        for (int i = 0; i < 3; i++)
            rbType[i] = NULL;

        if (GV->editState->SprBank->GetAssetByID(hTempObj->GetImageSet()) == NULL) {
            if (vszDefaultImgsets.size() != 0) {
                hTempObj->SetImageSet(vszDefaultImgsets[0].c_str());
            }
        }

        char tmp[64];
        sprintf(tmp, "%p", this);
        for (int i = 0; i < vszDefaultImgsets.size(); i++) {
            rbType[i] = new SHR::RadBut(GV->hGfxInterface, "", tmp,
                                        !strcmp(hTempObj->GetImageSet(), vszDefaultImgsets[i].c_str()));
            rbType[i]->adjustSize();
            rbType[i]->addActionListener(hAL);
            win->add(rbType[i], 5 + i * 73, 37);
        }

        int yoffset = 10 + 64 * (rbType[0] != 0);

        win->add(_butSave, 75, win->getHeight() - 60);

        labTimeOn = new SHR::Lab(GETL2S("EditObj_TogglePeg", "TimeOn"));
        labTimeOn->adjustSize();
        win->add(labTimeOn, 5, 5 + yoffset);

        int timeon, timeoff, delay;
        timeon = hTempObj->GetParam(WWD::Param_SpeedX);
        if (timeon <= 0)
            timeon = 1500;
        timeoff = hTempObj->GetParam(WWD::Param_SpeedY);
        if (timeoff <= 0)
            timeoff = 1500;
        delay = hTempObj->GetParam(WWD::Param_Speed);
        if (delay < 0)
            delay = 0;
        if (!strcmp(hTempObj->GetLogic(), "TogglePeg2")) delay = 750;
        else if (!strcmp(hTempObj->GetLogic(), "TogglePeg3")) delay = 750 * 2;
        else if (!strcmp(hTempObj->GetLogic(), "TogglePeg4")) delay = 750 * 3;

        sprintf(tmp, "%d", timeon);
        tfTimeOn = new SHR::TextField(tmp);
        tfTimeOn->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tfTimeOn->SetNumerical(1, 0);
        win->add(tfTimeOn, 140, 5 + yoffset);

        labTimeOff = new SHR::Lab(GETL2S("EditObj_TogglePeg", "TimeOff"));
        labTimeOff->adjustSize();
        win->add(labTimeOff, 5, 30 + yoffset);

        sprintf(tmp, "%d", timeoff);
        tfTimeOff = new SHR::TextField(tmp);
        tfTimeOff->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tfTimeOff->SetNumerical(1, 0);
        win->add(tfTimeOff, 140, 30 + yoffset);

        labOffset = new SHR::Lab(GETL2S("EditObj_TogglePeg", "Delay"));
        labOffset->adjustSize();
        win->add(labOffset, 5, 55 + yoffset);

        sprintf(tmp, "%d", delay);
        tfOffset = new SHR::TextField(tmp);
        tfOffset->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tfOffset->SetNumerical(1, 0);
        win->add(tfOffset, 140, 55 + yoffset);

        cbAlwaysOn = new SHR::CBox(GV->hGfxInterface, GETL2S("EditObj_TogglePeg", "AlwaysOn"));
        cbAlwaysOn->adjustSize();
        cbAlwaysOn->setSelected(hTempObj->GetParam(WWD::Param_Smarts) == 1);
        win->add(cbAlwaysOn, 5, 80 + yoffset);

        winSeries = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_TogglePeg", "Win2Caption"));
        winSeries->setDimension(gcn::Rectangle(0, 0, 210, 100));
        winSeries->setClose(0);
        winSeries->addActionListener(hAL);
        winSeries->setMovable(0);

        labIncrement = new SHR::Lab(GETL2S("EditObj_TogglePeg", "IncrementValues"));
        labIncrement->adjustSize();
        winSeries->add(labIncrement, 5, 10);

        labnobjDelay = new SHR::Lab(GETL2S("EditObj_TogglePeg", "Delay"));
        labnobjDelay->adjustSize();
        winSeries->add(labnobjDelay, 5, 30);

        tfnobjDelay = new SHR::TextField("250");
        tfnobjDelay->setDimension(gcn::Rectangle(0, 0, 100, 20));
        tfnobjDelay->SetNumerical(1, 0);
        winSeries->add(tfnobjDelay, 100, 30);

        winSeries->add(_butAddNext, 55, 50);
        st->conMain->add(winSeries, st->vPort->GetX() + win->getWidth(),
                         st->vPort->GetY() + st->vPort->GetHeight() - winSeries->getHeight());
    }

    cEditObjTogglePeg::~cEditObjTogglePeg() {
        delete tfnobjDelay;
        delete labnobjDelay;
        delete labIncrement;
        delete winSeries;
        for (int i = 0; i < 3; i++)
            delete rbType[i];
        delete labTimeOn;
        delete labTimeOff;
        delete labOffset;
        delete tfTimeOn;
        delete tfTimeOff;
        delete tfOffset;
        delete cbAlwaysOn;
        delete win;
        hState->vPort->MarkToRedraw(1);
    }

    void cEditObjTogglePeg::Save() {
        int timeon = atoi(tfTimeOn->getText().c_str()),
                timeoff = atoi(tfTimeOff->getText().c_str()),
                delay = atoi(tfOffset->getText().c_str());
        if (timeon == 1500) timeon = 0;
        if (timeoff == 1500) timeoff = 0;

        if (delay == 750) {
            delay = 0;
            hTempObj->SetLogic("TogglePeg2");
        }
        else if (delay == 750 * 2) {
            delay = 0;
            hTempObj->SetLogic("TogglePeg3");
        }
        else if (delay == 750 * 3) {
            delay = 0;
            hTempObj->SetLogic("TogglePeg4");
        }
        else { hTempObj->SetLogic("TogglePeg"); }

        hTempObj->SetParam(WWD::Param_SpeedX, timeon);
        hTempObj->SetParam(WWD::Param_SpeedY, timeoff);
        hTempObj->SetParam(WWD::Param_Speed, delay);

        hTempObj->SetParam(WWD::Param_Smarts, cbAlwaysOn->isSelected());
    }

    void cEditObjTogglePeg::Action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == win) {
            bKill = 1;
            return;
        }
        for (int i = 0; i < 3; i++)
            if (rbType[i] != 0 && actionEvent.getSource() == rbType[i]) {
                hTempObj->SetImageSet(vszDefaultImgsets[i].c_str());
                hState->vPort->MarkToRedraw(1);
            }
    }

    void cEditObjTogglePeg::Draw() {
        int dx, dy;
        win->getAbsolutePosition(dx, dy);

        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        for (int i = 0; i < 3; i++) {
            if (rbType[i] != 0) {
                hgeSprite *spr = hState->SprBank->GetAssetByID(vszDefaultImgsets[i].c_str())->GetIMGByIterator(
                        0)->GetSprite();
                int grdim = spr->GetWidth();
                if (spr->GetHeight() > grdim) grdim = spr->GetHeight();
                float fscale = 48.0f / float(grdim);
                spr->RenderEx(dx + 37 + i * 73 + 13, dy + 56 + 8, 0, fscale);
                WWD::Rect r = hState->SprBank->GetSpriteRenderRect(spr);
                if (mx > dx + 37 + i * 73 + 13 + float(r.x1) * fscale &&
                    mx < dx + 37 + i * 73 + 13 + float(r.x2) * fscale &&
                    my > dy + 56 + 8 + float(r.y1) * fscale && my < dy + 56 + 8 + float(r.y2) * fscale &&
                    hState->conMain->getWidgetAt(mx, my) == win) {
                    hState->bShowHand = 1;
                    if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                        rbType[i]->setSelected(1);
                        Action(gcn::ActionEvent(rbType[i], ""));
                    }
                }
            }
        }
    }

    void *cEditObjTogglePeg::GenerateNextObjectData() {
        int *ptr = new int;
        *ptr = atoi(tfnobjDelay->getText().c_str());
        return ptr;
    }

    void cEditObjTogglePeg::ApplyDataFromPrevObject(void *ptr) {
        int *delay = (int *) ptr;
        char tmp[64];
        sprintf(tmp, "%d", *delay);
        tfnobjDelay->setText(tmp);
        int actdel = atoi(tfOffset->getText().c_str());
        sprintf(tmp, "%d", actdel + (*delay));
        tfOffset->setText(tmp);
        delete delay;
    }
}
