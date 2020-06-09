#include "../editing_ww.h"
#include "../../globals.h"
#include "../error.h"
#include "../../../shared/commonFunc.h"
#include "../../langID.h"
#include "../../windows/options.h"

extern HGE *hge;

void State::EditingWW::FirstRun_Open() {
    if (FirstRun_data != 0) {
        conMain->moveToTop(FirstRun_data->win);
        return;
    }
    FirstRun_data = new win_FirstRun;
    FirstRun_data->win = new SHR::Win(&GV->gcnParts, GETL2S("FirstRun", "Caption"));
    int height = GV->fntMyriad16->GetHeightb(370, GETL2S("FirstRun", "Text")) + 80;
    FirstRun_data->win->setDimension(gcn::Rectangle(0, 0, 400, height + 60));
    FirstRun_data->win->setClose(1);
    FirstRun_data->win->addActionListener(al);
    conMain->add(FirstRun_data->win, hge->System_GetState(HGE_SCREENWIDTH) / 2 - 200,
                 hge->System_GetState(HGE_SCREENHEIGHT) / 2 - (height + 100) / 2);

    FirstRun_data->setClawDir = new SHR::But(GV->hGfxInterface, GETL2S("FirstRun", "SetClawDir"));
    FirstRun_data->setClawDir->setDimension(gcn::Rectangle(0, 0, 200, 33));
    FirstRun_data->setClawDir->addActionListener(al);
    FirstRun_data->win->add(FirstRun_data->setClawDir, 12, height - 5);

    FirstRun_data->website = new SHR::But(GV->hGfxInterface, GETL2S("FirstRun", "JoinDiscord"));
    FirstRun_data->website->setDimension(gcn::Rectangle(0, 0, 150, 33));
    FirstRun_data->website->addActionListener(al);
    FirstRun_data->win->add(FirstRun_data->website, 225, height - 5);

    FirstRun_data->vp = new WIDG::Viewport(vp, VP_FIRSTRUN);
    FirstRun_data->win->add(FirstRun_data->vp);

    FirstRun_data->bKill = 0;
}

void State::EditingWW::FirstRun_Close() {
    delete FirstRun_data->vp;
    delete FirstRun_data->website;
    delete FirstRun_data->setClawDir;
    delete FirstRun_data->win;
    delete FirstRun_data;
    FirstRun_data = 0;
}

void State::EditingWW::FirstRun_Think() {
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    int wx, wy;
    FirstRun_data->win->getAbsolutePosition(wx, wy);
    GV->fntMyriad16->SetColor(0xFF000000);
    GV->fntMyriad16->printfb(wx + 15, wy + 35, 370, 500, HGETEXT_LEFT | HGETEXT_TOP, 0, GETL2S("FirstRun", "Text"));
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
