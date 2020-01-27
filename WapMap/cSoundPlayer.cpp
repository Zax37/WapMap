#include "cSoundPlayer.h"
#include "globals.h"
#include "langID.h"

#include "databanks/sounds.h"

extern HGE *hge;

void SoundPlayerAL::action(const gcn::ActionEvent &actionEvent) {
    m_hOwn->Action(actionEvent);
}

void SoundPlayerVpCB::Draw(int iCode) {
    m_hOwn->Render();
}

void cSoundPlayer::Play() {
    if (hChannel == NULL)
        hChannel = hge->Effect_PlayEx(hAsset->GetSound(), sliVolume->getValue());
    else
        hge->Channel_Resume(hChannel);
    butPlay->setIcon(GV->sprIcons[Icon_Pause]);
}

cSoundPlayer::cSoundPlayer(SHR::Contener *dest, int x, int y, cSndBankAsset *as, const char *szTitle) {
    hAL = new SoundPlayerAL(this);
    if (szTitle != NULL)
        winPlayer = new SHR::Win(&GV->gcnParts, szTitle);
    else {
        if (as != NULL)
            winPlayer = new SHR::Win(&GV->gcnParts, as->GetName());
        else
            winPlayer = new SHR::Win(&GV->gcnParts, GETL(Lang_Sound));
    }
    winPlayer->setDimension(gcn::Rectangle(0, 0, 350, 73));
    winPlayer->setMovable(0);
    winPlayer->setShadow(0);
    dest->add(winPlayer, x, y);

    butPlay = new SHR::But(GV->hGfxInterface, GV->sprIcons[Icon_Play]);
    butPlay->setDimension(gcn::Rectangle(0, 0, 32, 32));
    butPlay->addActionListener(hAL);
    butPlay->setRenderBG(0);
    winPlayer->add(butPlay, 2, 23);
    butStop = new SHR::But(GV->hGfxInterface, GV->sprIcons[Icon_Stop]);
    butStop->setDimension(gcn::Rectangle(0, 0, 32, 32));
    butStop->addActionListener(hAL);
    butStop->setRenderBG(0);
    winPlayer->add(butStop, 37, 23);

    sliTime = new SHR::Slider(SHR::Slider::HORIZONTAL);
    sliTime->setScale(0.0f, 1.0f);
    sliTime->setDimension(gcn::Rectangle(0, 0, 340, 14));
    sliTime->addActionListener(hAL);
    sliTime->setEnabled(0);
    winPlayer->add(sliTime, 3, 10);

    sliVolume = new SHR::Slider(SHR::Slider::HORIZONTAL);
    sliVolume->setScale(0.0f, 100.0f);
    sliVolume->setValue(50.0f);
    sliVolume->setDimension(gcn::Rectangle(0, 0, 50, 14));
    sliVolume->addActionListener(hAL);
    winPlayer->add(sliVolume, 293, 34);

    if (as == NULL) {
        butPlay->setEnabled(0);
        butStop->setEnabled(0);
    }

    hChannel = NULL;
    hAsset = as;

    vpCB = new SoundPlayerVpCB(this);
    vp = new WIDG::Viewport(vpCB, 0);
    winPlayer->add(vp);

    fDuration = 0;
}

cSoundPlayer::~cSoundPlayer() {
    delete butPlay;
    delete butStop;
    delete vp;
    delete winPlayer;
    delete hAL;
    delete vpCB;
}

void cSoundPlayer::Update() {

}

void cSoundPlayer::Render() {
    int dx, dy;
    winPlayer->getAbsolutePosition(dx, dy);
    float act = 0;
    if (hChannel != NULL) {
        if (fDuration == 0) {
            fDuration = hge->Channel_GetLength(hChannel);
            sliTime->setScaleEnd(fDuration);
            sliTime->setEnabled(1);
        }
        act = hge->Channel_GetPos(hChannel);
        sliTime->setValue(act);
    }
    GV->fntMyriad13->SetColor(0xFFa1a1a1);
    GV->fntMyriad13->printf(dx + 73, dy + 46, HGETEXT_LEFT, "%02d:%02d / %02d:%02d", 0, int(int(act) / 60),
                            int(act) % 60, int(int(fDuration) / 60), int(fDuration) % 60);
    if (hChannel != NULL && !hge->Channel_IsPlaying(hChannel) && butPlay->getIcon() == GV->sprIcons[Icon_Pause]) {
        hge->Channel_Stop(hChannel);
        hChannel = NULL;
        butPlay->setIcon(GV->sprIcons[Icon_Play]);
    }
    GV->sprIcons[Icon_Sound]->Render(dx + 259, dy + 40);
}

void cSoundPlayer::Action(const gcn::ActionEvent &actionEvent) {
    if (actionEvent.getSource() == butPlay && butPlay->getIcon() == GV->sprIcons[Icon_Play]) {
        Play();
    } else if (actionEvent.getSource() == butPlay && butPlay->getIcon() == GV->sprIcons[Icon_Pause]) {
        hge->Channel_Pause(hChannel);
        butPlay->setIcon(GV->sprIcons[Icon_Play]);
    } else if (actionEvent.getSource() == butStop) {
        hge->Channel_Stop(hChannel);
        hChannel = NULL;
        butPlay->setIcon(GV->sprIcons[Icon_Play]);
    } else if (actionEvent.getSource() == sliVolume) {
        if (hChannel != NULL)
            hge->Channel_SetVolume(hChannel, sliVolume->getValue());
    } else if (actionEvent.getSource() == sliTime) {
        if (hChannel != NULL)
            hge->Channel_SetPos(hChannel, sliTime->getValue());
    }
}

void cSoundPlayer::SetAsset(cSndBankAsset *as, bool bUpdateTitle) {
    hAsset = as;
    fDuration = 0;
    if (hChannel != NULL) {
        hge->Channel_Stop(hChannel);
        hChannel = NULL;
    }
    sliTime->setValue(0);
    sliTime->setEnabled(0);
    butPlay->setIcon(GV->sprIcons[Icon_Play]);
    if (bUpdateTitle) {
        if (as == NULL)
            winPlayer->setCaption(GETL(Lang_Sound));
        else
            winPlayer->setCaption(as->GetName());
    }
    butPlay->setEnabled(as != NULL);
    butStop->setEnabled(as != NULL);
}
