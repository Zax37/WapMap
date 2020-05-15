#include "editFloorSpike.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"
#include "../databanks/imageSets.h"

extern HGE *hge;

namespace ObjEdit {
    cEditObjFloorSpike::cEditObjFloorSpike(WWD::Object *obj, State::EditingWW *st) : cObjEdit(obj, st) {
        int baselvl = GV->editState->hParser->GetBaseLevel();
        if (baselvl == 3 || baselvl == 10 || baselvl == 13) {
            vszDefaultImgsets.push_back("LEVEL_FLOORSPIKES1");
            if (baselvl != 10) {
                vszDefaultImgsets.push_back("LEVEL_FLOORSPIKES2");
            }
        } else if (baselvl == 9) {
            isSaw = true;
            vszDefaultImgsets.push_back("LEVEL_FLOORSAW");
        } else {
            vszDefaultImgsets.push_back("LEVEL_FLOORSPIKES");
        }

        iType = ObjEdit::enFloorSpike;
        win = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_FloorSpike", "WinCaption"));
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

        labTimeOn = new SHR::Lab(GETL2S("EditObj_FloorSpike", "TimeOn"));
        labTimeOn->adjustSize();
        win->add(labTimeOn, 5, 5 + yoffset);

        int timeon, timeoff, delay, damage;
        timeon = hTempObj->GetParam(WWD::Param_SpeedX);
        if (timeon <= 0)
            timeon = 1500;
        timeoff = hTempObj->GetParam(WWD::Param_SpeedY);
        if (timeoff <= 0)
            timeoff = 1500;
        delay = hTempObj->GetParam(WWD::Param_Speed);
        if (delay < 0)
            delay = 0;
        if (!strcmp(hTempObj->GetLogic(), "FloorSpike2") || !strcmp(hTempObj->GetLogic(), "FloorSaw2")) delay = 750;
        else if (!strcmp(hTempObj->GetLogic(), "FloorSpike3") || !strcmp(hTempObj->GetLogic(), "FloorSaw3")) delay = 750 * 2;
        else if (!strcmp(hTempObj->GetLogic(), "FloorSpike4") || !strcmp(hTempObj->GetLogic(), "FloorSaw4")) delay = 750 * 3;
        damage = hTempObj->GetParam(WWD::Param_Damage);
        if (!damage) {
            damage = 10;
        }

        sprintf(tmp, "%d", timeon);
        tfTimeOn = new SHR::TextField(tmp);
        tfTimeOn->setDimension(gcn::Rectangle(0, 0, 75, 20));
        tfTimeOn->SetNumerical(1, 0);
        win->add(tfTimeOn, 165, 5 + yoffset);

        labTimeOff = new SHR::Lab(GETL2S("EditObj_FloorSpike", "TimeOff"));
        labTimeOff->adjustSize();
        win->add(labTimeOff, 5, 30 + yoffset);

        sprintf(tmp, "%d", timeoff);
        tfTimeOff = new SHR::TextField(tmp);
        tfTimeOff->setDimension(gcn::Rectangle(0, 0, 75, 20));
        tfTimeOff->SetNumerical(1, 0);
        win->add(tfTimeOff, 165, 30 + yoffset);

        labOffset = new SHR::Lab(GETL2S("EditObj_FloorSpike", "Delay"));
        labOffset->adjustSize();
        win->add(labOffset, 5, 55 + yoffset);

        sprintf(tmp, "%d", delay);
        tfOffset = new SHR::TextField(tmp);
        tfOffset->setDimension(gcn::Rectangle(0, 0, 75, 20));
        tfOffset->SetNumerical(1, 0);
        win->add(tfOffset, 165, 55 + yoffset);

        labDamage = new SHR::Lab(GETL2S("EditObj_FloorSpike", "Damage"));
        labDamage->adjustSize();
        win->add(labDamage, 5, 80 + yoffset);

        sprintf(tmp, "%d", damage);
        tfDamage = new SHR::TextField(tmp);
        tfDamage->setDimension(gcn::Rectangle(0, 0, 75, 20));
        tfDamage->SetNumerical(1, 0);
        win->add(tfDamage, 165, 80 + yoffset);

        winSeries = new SHR::Win(&GV->gcnParts, GETL2S("EditObj_FloorSpike", "Win2Caption"));
        winSeries->setDimension(gcn::Rectangle(0, 0, 240, 100));
        winSeries->setClose(0);
        winSeries->addActionListener(hAL);
        winSeries->setMovable(0);

        labIncrement = new SHR::Lab(GETL2S("EditObj_FloorSpike", "IncrementValues"));
        labIncrement->adjustSize();
        winSeries->add(labIncrement, 5, 10);

        labnobjDelay = new SHR::Lab(GETL2S("EditObj_FloorSpike", "Delay"));
        labnobjDelay->adjustSize();
        winSeries->add(labnobjDelay, 5, 30);

        tfnobjDelay = new SHR::TextField("250");
        tfnobjDelay->setDimension(gcn::Rectangle(0, 0, 70, 20));
        tfnobjDelay->SetNumerical(1, 0);
        winSeries->add(tfnobjDelay, 162, 30);

        winSeries->add(_butAddNext, 55, 50);
        st->conMain->add(winSeries, st->vPort->GetX() + win->getWidth(),
                         st->vPort->GetY() + st->vPort->GetHeight() - winSeries->getHeight());
    }

    cEditObjFloorSpike::~cEditObjFloorSpike() {
        delete tfnobjDelay;
        delete labnobjDelay;
        delete labIncrement;
        delete winSeries;
        for (int i = 0; i < 3; i++)
            delete rbType[i];
        delete labTimeOn;
        delete labTimeOff;
        delete labOffset;
        delete labDamage;
        delete tfTimeOn;
        delete tfTimeOff;
        delete tfOffset;
        delete tfDamage;
        delete win;
        hState->vPort->MarkToRedraw(1);
    }

    void cEditObjFloorSpike::Save() {
        int timeon = atoi(tfTimeOn->getText().c_str()),
            timeoff = atoi(tfTimeOff->getText().c_str()),
            delay = atoi(tfOffset->getText().c_str()),
            damage = atoi(tfDamage->getText().c_str());
        if (timeon == 1500) timeon = 0;
        if (timeoff == 1500) timeoff = 0;

        std::string logic = isSaw ? "SawBlade" : "FloorSpike";
        if (delay == 750) {
            delay = 0;
            hTempObj->SetLogic((logic + "2").c_str());
        }
        else if (delay == 750 * 2) {
            delay = 0;
            hTempObj->SetLogic((logic + "3").c_str());
        }
        else if (delay == 750 * 3) {
            delay = 0;
            hTempObj->SetLogic((logic + "4").c_str());
        }
        else { hTempObj->SetLogic(logic.c_str()); }

        hTempObj->SetParam(WWD::Param_SpeedX, timeon);
        hTempObj->SetParam(WWD::Param_SpeedY, timeoff);
        hTempObj->SetParam(WWD::Param_Speed, delay);
        hTempObj->SetParam(WWD::Param_Damage, damage);
    }

    void cEditObjFloorSpike::Action(const gcn::ActionEvent &actionEvent) {
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

    void cEditObjFloorSpike::Draw() {
        int dx, dy;
        win->getAbsolutePosition(dx, dy);

        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        for (int i = 0; i < 3; i++) {
            if (rbType[i] != 0) {
                hgeSprite *spr = hState->SprBank->GetAssetByID(vszDefaultImgsets[i].c_str())->GetIMGByIterator(
                        isSaw ? 9 : 5)->GetSprite();
                int grdim = spr->GetWidth();
                if (spr->GetHeight() > grdim) grdim = spr->GetHeight();
                float fscale = (isSaw ? 64.0f : 28.0f) / float(grdim);
                spr->RenderEx(dx + (isSaw ? 48 : 37) + i * 73 + 13, dy + 56 + 8, 0, fscale);
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

    void *cEditObjFloorSpike::GenerateNextObjectData() {
        int *ptr = new int;
        *ptr = atoi(tfnobjDelay->getText().c_str());
        return ptr;
    }

    void cEditObjFloorSpike::ApplyDataFromPrevObject(void *ptr) {
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
