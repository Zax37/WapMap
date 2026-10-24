#include "cSoundPlayer.h"
#include "globals.h"
#include "langID.h"

#include "databanks/sounds.h"
#include <cstdio>
#ifdef WAPMAP_ENABLE_IMGUI
#include "states/editing_ww.h"
#include "imguiTheme.h"
#include "windows/assetBrowserUi.h"
#endif

extern HGE *hge;

void SoundPlayerAL::action(const gcn::ActionEvent &actionEvent) {
    m_hOwn->Action(actionEvent);
}

void SoundPlayerVpCB::Draw(int iCode) {
    m_hOwn->Render();
}

void cSoundPlayer::Play() {
    if (!hAsset || !hAsset->IsLoaded() || !hAsset->GetSound()) return;
    if (hChannel == NULL)
        hChannel = hge->Effect_PlayEx(hAsset->GetSound(), sliVolume->getValue());
    else
        hge->Channel_Resume(hChannel);
    if (hChannel) butPlay->setIcon(GV->sprIcons[Icon_Pause]);
}

cSoundPlayer::cSoundPlayer(SHR::Container *dest, int x, int y, cSndBankAsset *as, const char *szTitle) {
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
#ifdef WAPMAP_ENABLE_IMGUI
    nativeAssetName = as ? as->GetName() : "";
#endif

    vpCB = new SoundPlayerVpCB(this);
    vp = new WIDG::Viewport(vpCB, 0);
    winPlayer->add(vp);

    fDuration = 0;
}

cSoundPlayer::~cSoundPlayer() {
    if (hChannel) hge->Channel_Stop(hChannel);
    delete sliTime;
    delete sliVolume;
    delete butPlay;
    delete butStop;
    delete vp;
    delete winPlayer;
    delete hAL;
    delete vpCB;
}

void cSoundPlayer::Update() {
    if (!hChannel) return;
    if (fDuration == 0) {
        fDuration = hge->Channel_GetLength(hChannel);
        sliTime->setScaleEnd(fDuration);
        sliTime->setEnabled(fDuration > 0);
    }
    sliTime->setValue(hge->Channel_GetPos(hChannel));
    if (!hge->Channel_IsPlaying(hChannel) && butPlay->getIcon() == GV->sprIcons[Icon_Pause]) {
        hge->Channel_Stop(hChannel);
        hChannel = nullptr;
        sliTime->setValue(fDuration);
        butPlay->setIcon(GV->sprIcons[Icon_Play]);
    }
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
    GV->fntMyriad16->SetColor(0xFFe1e1e1);
    GV->fntMyriad16->printf(dx + 73, dy + 46, HGETEXT_LEFT, "%02d:%02d / %02d:%02d", 0, int(int(act) / 60),
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
        sliTime->setValue(0);
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
#ifdef WAPMAP_ENABLE_IMGUI
    nativeAssetName = as ? as->GetName() : "";
    nativeEffect = as ? as->GetSound() : nullptr;
#endif
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
#ifdef WAPMAP_ENABLE_IMGUI
void cSoundPlayer::DrawImGui() {
    auto* bank = GV->editState->hSndBank;
    auto* asset = bank && !nativeAssetName.empty() ? bank->GetAssetByID(nativeAssetName.c_str()) : nullptr;
    const auto effect = asset && asset->IsLoaded() ? asset->GetSound() : nullptr;
    if (asset != hAsset || effect != nativeEffect) {
        const auto name = nativeAssetName;
        SetAsset(asset, false);
        nativeAssetName = name; // A temporarily missing file can become available again.
    }
    Update();
    ImGui::PushID(this);
    const float x = static_cast<float>(winPlayer->getX()), y = ImGui::GetFrameHeight() + winPlayer->getY();
    const auto origin = ImGui::GetWindowPos();
    auto* draw = ImGui::GetWindowDrawList();
    draw->AddRect({origin.x + x, origin.y + y}, {origin.x + x + 350, origin.y + y + 73}, IM_COL32(75, 75, 75, 255));
    WapMapImGui::BitmapText(draw, *hge, *GV->fntMyriad16, {origin.x + x + 5, origin.y + y + 2}, winPlayer->getCaption().c_str());
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {4, 0});
    ImGui::SetCursorPos({x + 5, y + 19}); ImGui::SetNextItemWidth(340);
    float position = static_cast<float>(sliTime->getValue());
    ImGui::BeginDisabled(!hChannel || fDuration <= 0);
    if (ImGui::SliderFloat("##position", &position, 0, std::max(0.001f, fDuration), "", ImGuiSliderFlags_AlwaysClamp)) {
        sliTime->setValue(position); hge->Channel_SetPos(hChannel, position);
    }
    ImGui::EndDisabled();
    const gcn::Rectangle area(static_cast<int>(x), static_cast<int>(y + 13), 350, 73);
    for (auto* button : {butPlay, butStop})
        if (AssetBrowserUi::Button(*hge, *GV->fntMyriad16, *GV->hGfxInterface, area, button, effect != nullptr)) Action(gcn::ActionEvent(button, ""));
    char time[64]; const int current = static_cast<int>(position), duration = static_cast<int>(fDuration);
    std::snprintf(time, sizeof(time), "%02d:%02d / %02d:%02d", current / 60, current % 60, duration / 60, duration % 60);
    WapMapImGui::BitmapText(draw, *hge, *GV->fntMyriad16, {origin.x + x + 74, origin.y + y + 44}, time);
    WapMapImGui::Sprite(draw, *hge, GV->sprIcons[Icon_Sound], {origin.x + x + 257, origin.y + y + 39});
    ImGui::SetCursorPos({x + 293, y + 43}); ImGui::SetNextItemWidth(50);
    float volume = static_cast<float>(sliVolume->getValue());
    if (ImGui::SliderFloat("##volume", &volume, 0, 100, "", ImGuiSliderFlags_AlwaysClamp)) {
        sliVolume->setValue(volume); if (hChannel) hge->Channel_SetVolume(hChannel, static_cast<int>(volume));
    }
    ImGui::PopStyleVar(); ImGui::PopID();
}
#endif
