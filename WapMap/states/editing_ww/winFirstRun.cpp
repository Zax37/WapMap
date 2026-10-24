#include "../editing_ww.h"
#include "../dialog.h"
#include "../../../shared/commonFunc.h"
#include "../../langID.h"
#include "../../windows/options.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../../imguiTheme.h"
#include "../../legacyText.h"
#include "../../cInterfaceSheet.h"
#include <cstring>


#endif

extern HGE *hge;
extern cAutoUpdater *_AU_GLOBAL_PTR;

#define HEIGHT int height = GV->fntMyriad16->GetStringBlockHeight(370, GETL2S("FirstRun", "Text")) + 100;

constexpr bool hasUpdateReady() {
    return GV->bAutoUpdate && _AU_GLOBAL_PTR && _AU_GLOBAL_PTR->releaseAsset.size;
}

void State::EditingWW::FirstRun_Open() {
    if (hasUpdateReady()) {
        return;
    }
    if (FirstRun_data != 0) {
#ifdef WAPMAP_ENABLE_IMGUI
        FirstRun_data->updatedPosition = false;
#else
        conMain->moveToTop(FirstRun_data->win);
#endif
        return;
    }
    FirstRun_data = new win_FirstRun;
    FirstRun_data->win = new SHR::Win(&GV->gcnParts, GETL2S("FirstRun", "Caption"));
    HEIGHT
    FirstRun_data->win->setDimension(gcn::Rectangle(0, 0, 400, height + 60));
    FirstRun_data->win->setClose(1);
    FirstRun_data->win->addActionListener(mainListener);
    conMain->add(FirstRun_data->win, - 200, -height);

    FirstRun_data->setClawDir = new SHR::But(GV->hGfxInterface, GETL2S("FirstRun", "SetClawDir"));
    FirstRun_data->setClawDir->setDimension(gcn::Rectangle(0, 0, 200, 33));
    FirstRun_data->setClawDir->addActionListener(mainListener);
    FirstRun_data->win->add(FirstRun_data->setClawDir, 12, height - 5);

    FirstRun_data->website = new SHR::But(GV->hGfxInterface, GETL2S("FirstRun", "JoinDiscord"));
    FirstRun_data->website->setDimension(gcn::Rectangle(0, 0, 150, 33));
    FirstRun_data->website->addActionListener(mainListener);
    FirstRun_data->win->add(FirstRun_data->website, 225, height - 5);

    FirstRun_data->vp = new WIDG::Viewport(vp, VP_FIRSTRUN);
    FirstRun_data->win->add(FirstRun_data->vp);

    FirstRun_data->bKill = 0;
#ifdef WAPMAP_ENABLE_IMGUI
    FirstRun_data->win->setVisible(false);
#endif
}

#ifdef WAPMAP_ENABLE_IMGUI
void State::EditingWW::FirstRun_DrawImGui() {
    if (!FirstRun_data || FirstRun_data->bKill || !_isOnTop()) return;
    auto& data = *FirstRun_data;
    const auto* viewport = ImGui::GetMainViewport();
    if (!data.updatedPosition) {
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, {0.5f, 0.5f});
        ImGui::SetNextWindowFocus();
        data.updatedPosition = true;
    }
    const float height = std::max(220.f, std::min(static_cast<float>(data.win->getHeight()), viewport->WorkSize.y - 20.f));
    ImGui::SetNextWindowSize({400, height}, ImGuiCond_Always);
    const auto title = LegacyText::ToUtf8(GETL2S("FirstRun", "Caption"), LegacyText::CodePage(GV->Lang->GetCode())) + "###WapMapFirstRun";
    bool visible = true;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse, false)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, GETL2S("FirstRun", "Caption"));
        ImGui::SetCursorPos({15, 35});
        if (ImGui::BeginChild("welcomeText", {370, height - 95}, ImGuiChildFlags_NavFlattened)) {
            WapMapImGui::WrappedBitmapMarkupText(*hge, *GV->fntMyriad16, GETL2S("FirstRun", "Text"), 350);
        }
        ImGui::EndChild();
        ImGui::SetCursorPos({12, height - 43});
        ImGui::BeginDisabled(data.nativeAction != 0);
        if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL2S("FirstRun", "SetClawDir"), {200, 33})) data.nativeAction = 1;
        ImGui::SetCursorPos({225, height - 43});
        if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, GETL2S("FirstRun", "JoinDiscord"), {150, 33})) data.nativeAction = 2;
        ImGui::EndDisabled();
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) visible = false;
    }
    ImGui::End();
    if (!visible) { data.bKill = true; data.nativeAction = 0; }
}
#endif

void State::EditingWW::FirstRun_Close() {
    delete FirstRun_data->vp;
    delete FirstRun_data->website;
    delete FirstRun_data->setClawDir;
    delete FirstRun_data->win;
    delete FirstRun_data;
    FirstRun_data = 0;

    if (hasUpdateReady()) {
        _AU_GLOBAL_PTR->PopupQuestion();
    }
}

void State::EditingWW::FirstRun_Think() {
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    int wx, wy;
    FirstRun_data->win->getAbsolutePosition(wx, wy);
    GV->fntMyriad16->SetColor(0xFF000000);
    GV->fntMyriad16->printfb(wx + 15, wy + 35, 370, 500, HGETEXT_LEFT | HGETEXT_TOP, 0, GETL2S("FirstRun", "Text"));

    if(!FirstRun_data->updatedPosition) {
        HEIGHT
        FirstRun_data->win->setPosition(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 200,
                     hge->System_GetState(HGE_SCREENHEIGHT) / 2 - (height + 100) / 2);
        FirstRun_data->updatedPosition = true;
    }
}

void State::EditingWW::FirstRun_Action(bool but) {
    if (!but) {
        FirstRun_data->bKill = true;
        GV->editState->hwinOptions->Open(WWD::Game_Claw);
        GV->editState->hwinOptions->PickAndSetGameLocation(WWD::Game_Claw);
    } else {
        ShellExecute(hge->System_GetState(HGE_HWND), "open", "http://discord.gg/tDRsAqg", NULL, NULL, SW_SHOWNORMAL);
    }
}
