#ifndef H_C_SOUNDPLAYER
#define H_C_SOUNDPLAYER

#include "../shared/gcnWidgets/wWin.h"
#include "../shared/gcnWidgets/wButton.h"
#include "../shared/gcnWidgets/wSlider.h"
#include "wViewport.h"

class cSndBankAsset;

class cSoundPlayer;

class SoundPlayerAL : public gcn::ActionListener {
private:
    cSoundPlayer *m_hOwn;
public:
    SoundPlayerAL(cSoundPlayer *owner) { m_hOwn = owner; };

    void action(const gcn::ActionEvent &actionEvent);
};

class SoundPlayerVpCB : public WIDG::VpCallback {
private:
    cSoundPlayer *m_hOwn;
public:
    SoundPlayerVpCB(cSoundPlayer *owner) { m_hOwn = owner; };

    virtual void Draw(int iCode);
};

class cSoundPlayer {
private:
    SHR::Win *winPlayer;
    SHR::But *butPlay, *butStop;
    SHR::Slider *sliTime, *sliVolume;
    WIDG::Viewport *vp;
    SoundPlayerVpCB *vpCB;
    SoundPlayerAL *hAL;
    HCHANNEL hChannel;
    cSndBankAsset *hAsset;
    float fDuration;
public:
    cSoundPlayer(SHR::Contener *dest, int x, int y, cSndBankAsset *as, const char *szTitle = NULL);

    ~cSoundPlayer();

    void Update();

    void Render();

    void Action(const gcn::ActionEvent &actionEvent);

    void SetAsset(cSndBankAsset *as, bool bUpdateTitle = 1);

    void Play();
};

#endif
