#include "stats.h"
#include "../globals.h"
#include "editing_ww.h"
#include "../../shared/commonFunc.h"
#include "../langID.h"

extern HGE *hge;

void State::cMapStatsAL::action(const gcn::ActionEvent &actionEvent) {
    if (actionEvent.getSource() == m_hOwn->winStat) {
        m_hOwn->bKill = 1;
        return;
    }
}

State::MapStats::MapStats(WWD::Parser *hp) {
    hMap = hp;
    bKill = 0;
}

void State::MapStats::Init() {
    gui = new gcn::Gui();
    gui->setGraphics(GV->gcnGraphics);
    gui->setInput(GV->gcnInput);
    conMain = new SHR::Contener();
    conMain->setDimension(
            gcn::Rectangle(0, 0, hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT)));
    conMain->setOpaque(0);
    gui->setTop(conMain);

    hAL = new cMapStatsAL(this);

    winStat = new SHR::Win(&GV->gcnParts, GETL2S("Stats", "WindowCaption"));
    winStat->setDimension(gcn::Rectangle(0, 0, 640, 480));
    winStat->setClose(1);
    winStat->setMovable(0);
    winStat->addActionListener(hAL);
    conMain->add(winStat, hge->System_GetState(HGE_SCREENWIDTH) / 2 - 320,
                 hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 240);

    tabbedArea = new SHR::TabbedArea();
    tabbedArea->setDimension(gcn::Rectangle(0, 0, 625, 470));

    conGen = new SHR::Contener();
    conGen->setDimension(gcn::Rectangle(0, 0, 620, 460));
    conObjects = new SHR::Contener();
    conObjects->setDimension(gcn::Rectangle(0, 0, 620, 460));
    statsPl = new PlaneStat[hMap->GetPlanesCount()];
    iObjCount = 0;
    int treasures[10];
    for (int i = 0; i < 10; i++) treasures[i] = 0;
    for (int i = 0; i < hMap->GetPlanesCount(); i++) {
        statsPl[i].conPl = new SHR::Contener();
        statsPl[i].conPl->setDimension(gcn::Rectangle(0, 0, 620, 460));
        statsPl[i].tileTypes = new SHR::cPieChart();
        int atrcount[6];
        for (int a = 0; a < 5; a++) atrcount[a] = 0;
        for (int y = 0; y < hMap->GetPlane(i)->GetPlaneHeight(); y++)
            for (int x = 0; x < hMap->GetPlane(i)->GetPlaneWidth(); x++) {
                if (hMap->GetPlane(i)->GetTile(x, y)->IsFilled() || hMap->GetPlane(i)->GetTile(x, y)->IsInvisible())
                    atrcount[0]++;
                else {
                    WWD::TileAtrib *atr = hMap->GetTileAtribs(hMap->GetPlane(i)->GetTile(x, y)->GetID());
                    if (atr == NULL) {
                        atrcount[0]++;
                        continue;
                    }

                    unsigned int attr = int(atr->GetAtribInside());
                    if (attr > 5) attr = 5;
                    atrcount[attr]++;

                    if (atr->GetType() == WWD::AtribType_Double) {
                        attr = int(atr->GetAtribOutside());
                        if (attr > 5) attr = 5;
                        atrcount[attr]++;
                    }
                }
            }
        if (atrcount[0] > 0)
            statsPl[i].tileTypes->AddPart("puste", atrcount[0], 0xFFFFFFFF);
        if (atrcount[1] > 0)
            statsPl[i].tileTypes->AddPart("solid", atrcount[1], COLOR_SOLID);
        if (atrcount[2] > 0)
            statsPl[i].tileTypes->AddPart("ground", atrcount[2], COLOR_GROUND);
        if (atrcount[3] > 0)
            statsPl[i].tileTypes->AddPart("climb", atrcount[3], COLOR_CLIMB);
        if (atrcount[4] > 0)
            statsPl[i].tileTypes->AddPart("death", atrcount[4], COLOR_DEATH);
        if (atrcount[5] > 0)
            statsPl[i].tileTypes->AddPart("invalid", atrcount[5], 0);
        statsPl[i].tileTypes->SetBgCol(0xFF4d4d4d);
        statsPl[i].tileTypes->Render(200);
        /*GAME_TREASURE_GOLDBARS, //sztaba
        GAME_TREASURE_COINS, //moneta
        GAME_TREASURE_GECKOS //smok
        SCEPTERS //berlo
        CROWNS //korona
        CROSSES //krzyz
        RINGS pierscien
        CHALICES puchar
        JEWELEDSKULL
        NECKLACE*/
        iObjCount += hMap->GetPlane(i)->GetObjectsCount();
        for (int o = 0; o < hMap->GetPlane(i)->GetObjectsCount(); o++) {
            WWD::Object *obj = hMap->GetPlane(i)->GetObjectByIterator(o);
            char *lgfx = SHR::ToLower(obj->GetImageSet());
            if (strstr(lgfx, "coins") != NULL) treasures[0]++;
            else if (strstr(lgfx, "goldbars") != NULL) treasures[1]++;
            else if (strstr(lgfx, "rings") != NULL) treasures[2]++;
            else if (strstr(lgfx, "chalices") != NULL) treasures[3]++;
            else if (strstr(lgfx, "necklace") != NULL) treasures[4]++;
            else if (strstr(lgfx, "crosses") != NULL) treasures[5]++;
            else if (strstr(lgfx, "scepters") != NULL) treasures[6]++;
            else if (strstr(lgfx, "geckos") != NULL) treasures[7]++;
            else if (strstr(lgfx, "crowns") != NULL) treasures[8]++;
            else if (strstr(lgfx, "jeweledskull") != NULL) treasures[9]++;
            delete[] lgfx;
        }
    }
    iTreasureCount = 0;
    for (int i = 0; i < 10; i++)
        iTreasureCount += treasures[i];
    iPerfectNeed =
            treasures[0] * 100 + treasures[1] * 500 + treasures[2] * 1500 + treasures[3] * 2500 + treasures[4] * 2500 +
            treasures[5] * 5000 + treasures[6] * 7500 + treasures[7] * 10000 + treasures[8] * 15000 +
            treasures[9] * 25000;

    pieTreasures = new SHR::cPieChart();
    for (int i = 0; i < 10; i++) {
        char tmp[15];
        sprintf(tmp, "Lab_Treasure%d", i + 1);
        pieTreasures->AddPart(GETL2S("Stats", tmp), treasures[i]);
    }
    pieTreasures->SetBgCol(0xFF4d4d4d);
    pieTreasures->Render(200);

    char tmp[256];
    sprintf(tmp, "%s (~y~%d~l~):", GETL2S("Stats", "Lab_Treasures"), iTreasureCount);
    labTreasures = new SHR::Lab(tmp);
    labTreasures->adjustSize();
    conObjects->add(labTreasures, 210, 5);

    sprintf(tmp, "%s: ~y~%s~l~", "name", hMap->GetName());
    labMapName = new SHR::Lab(tmp);
    labMapName->adjustSize();
    conGen->add(labMapName, 5, 15);
    sprintf(tmp, "%s: ~y~%s~l~", "auth", hMap->GetAuthor());
    labMapAuthor = new SHR::Lab(tmp);
    labMapAuthor->adjustSize();
    conGen->add(labMapAuthor, 5, 35);
    sprintf(tmp, "%s: ~y~%s~l~", "mod. date", hMap->GetDate());
    labMapDate = new SHR::Lab(tmp);
    labMapDate->adjustSize();
    conGen->add(labMapDate, 5, 55);
    sprintf(tmp, "%s: ~y~%d~l~", "lay. count", hMap->GetPlanesCount());
    labMapLayersCount = new SHR::Lab(tmp);
    labMapLayersCount->adjustSize();
    conGen->add(labMapLayersCount, 5, 85);
    sprintf(tmp, "%s: ~y~%d~l~", "obj. count", iObjCount);
    labMapObjectCount = new SHR::Lab(tmp);
    labMapObjectCount->adjustSize();
    conGen->add(labMapObjectCount, 5, 105);
    sprintf(tmp, "%s: ~y~%d~l~", "perf. score", iPerfectNeed);
    labMapPerfectScore = new SHR::Lab(tmp);
    labMapPerfectScore->adjustSize();
    conGen->add(labMapPerfectScore, 5, 125);


    tabbedArea->addTab(GETL2S("Stats", "Tab_General"), conGen);
    tabbedArea->addTab(GETL2S("Stats", "Tab_Objects"), conObjects);
    for (int i = 0; i < hMap->GetPlanesCount(); i++)
        tabbedArea->addTab(hMap->GetPlane(i)->GetName(), statsPl[i].conPl);
    winStat->add(tabbedArea, 5, 15);
}

void State::MapStats::Destroy() {

}

bool State::MapStats::Think() {
    try {
        gui->logic();
    }
    catch (gcn::Exception &exc) {
        GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(),
                            exc.getLine());
    }
    if (bKill)
        _popMe(0);
    return 0;
}

bool State::MapStats::Render() {
    try {
        gui->draw();
    }
    catch (gcn::Exception &exc) {
        GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(),
                            exc.getLine());
    }
    if (tabbedArea->getSelectedTabIndex() == 1) {
        int rx = hge->System_GetState(HGE_SCREENWIDTH) / 2 - 320, ry = hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 240;
        pieTreasures->GetSprite()->Render(rx + 15, ry + 65);
        for (int i = 0; i < 10; i++) {
            SHR::cPieChartEl el = pieTreasures->GetPart(i);
            int dx = rx + 220, dy = ry + 85;
            if (i > 3) {
                dx += 130 * (i > 6 ? 2 : 1);
                dy += 20 * ((i - 4) % 3);
            } else
                dy += 20 * i;

            hgeQuad q;
            q.tex = 0;
            q.blend = BLEND_DEFAULT;
            SHR::SetQuad(&q, el.iColor, dx, dy + 5, dx + 10, dy + 15);
            hge->Gfx_RenderQuad(&q);
            GV->fntMyriad13->printf(dx + 15, dy, HGETEXT_LEFT, "%s ~l~(~y~%d~l~)", 0, el.szDesc, el.iPart);
        }
    } else if (tabbedArea->getSelectedTabIndex() > 1) {
        int plid = tabbedArea->getSelectedTabIndex() - 2;
        statsPl[plid].tileTypes->GetSprite()->Render(500, 500);
    }
    GV->IF->Render();
    GV->Console->Render();
    return 1;
}
