#include "cInventoryController.h"

#include "states/editing_ww.h"
#include "globals.h"
#include "../shared/gcnWidgets/wInventory.h"
#include "databanks/imageSets.h"

extern HGE *hge;

cInventoryController::cInventoryController() {
    hElements[0] = std::pair<std::string, int>("GAME_TREASURE_COINS", 33);
    hElements[1] = std::pair<std::string, int>("GAME_TREASURE_GOLDBARS", 1);
    hElements[2] = std::pair<std::string, int>("GAME_TREASURE_NECKLACE", 6);
    hElements[3] = std::pair<std::string, int>("GAME_MAPPIECE", 31);

    hElements[4] = std::pair<std::string, int>("GAME_TREASURE_RINGS_RED", 2);
    hElements[5] = std::pair<std::string, int>("GAME_TREASURE_RINGS_GREEN", 3);
    hElements[6] = std::pair<std::string, int>("GAME_TREASURE_RINGS_BLUE", 4);
    hElements[7] = std::pair<std::string, int>("GAME_TREASURE_RINGS_PURPLE", 5);

    hElements[8] = std::pair<std::string, int>("GAME_TREASURE_CHALICES_RED", 41);
    hElements[9] = std::pair<std::string, int>("GAME_TREASURE_CHALICES_GREEN", 42);
    hElements[10] = std::pair<std::string, int>("GAME_TREASURE_CHALICES_BLUE", 43);
    hElements[11] = std::pair<std::string, int>("GAME_TREASURE_CHALICES_PURPLE", 44);

    hElements[12] = std::pair<std::string, int>("GAME_TREASURE_CROSSES_RED", 7);
    hElements[13] = std::pair<std::string, int>("GAME_TREASURE_CROSSES_GREEN", 8);
    hElements[14] = std::pair<std::string, int>("GAME_TREASURE_CROSSES_BLUE", 9);
    hElements[15] = std::pair<std::string, int>("GAME_TREASURE_CROSSES_PURPLE", 10);

    hElements[16] = std::pair<std::string, int>("GAME_TREASURE_SCEPTERS_RED", 11);
    hElements[17] = std::pair<std::string, int>("GAME_TREASURE_SCEPTERS_GREEN", 12);
    hElements[18] = std::pair<std::string, int>("GAME_TREASURE_SCEPTERS_BLUE", 13);
    hElements[19] = std::pair<std::string, int>("GAME_TREASURE_SCEPTERS_PURPLE", 14);

    hElements[20] = std::pair<std::string, int>("GAME_TREASURE_GECKOS_RED", 15);
    hElements[21] = std::pair<std::string, int>("GAME_TREASURE_GECKOS_GREEN", 16);
    hElements[22] = std::pair<std::string, int>("GAME_TREASURE_GECKOS_BLUE", 17);
    hElements[23] = std::pair<std::string, int>("GAME_TREASURE_GECKOS_PURPLE", 18);

    hElements[24] = std::pair<std::string, int>("GAME_TREASURE_CROWNS_RED", 45);
    hElements[25] = std::pair<std::string, int>("GAME_TREASURE_CROWNS_GREEN", 46);
    hElements[26] = std::pair<std::string, int>("GAME_TREASURE_CROWNS_BLUE", 47);
    hElements[27] = std::pair<std::string, int>("GAME_TREASURE_CROWNS_PURPLE", 48);

    hElements[28] = std::pair<std::string, int>("GAME_TREASURE_JEWELEDSKULL_RED", 49);
    hElements[29] = std::pair<std::string, int>("GAME_TREASURE_JEWELEDSKULL_GREEN", 50);
    hElements[30] = std::pair<std::string, int>("GAME_TREASURE_JEWELEDSKULL_BLUE", 51);
    hElements[31] = std::pair<std::string, int>("GAME_TREASURE_JEWELEDSKULL_PURPLE", 52);

    hElements[32] = std::pair<std::string, int>("GAME_AMMO_SHOT", 20);
    hElements[33] = std::pair<std::string, int>("GAME_AMMO_SHOTBAG", 21);
    hElements[34] = std::pair<std::string, int>("GAME_AMMO_DEATHBAG", 19);
    hElements[35] = std::pair<std::string, int>("GAME_CATNIPS_NIP1", 22);

    hElements[36] = std::pair<std::string, int>("GAME_MAGIC_GLOW", 28);
    hElements[37] = std::pair<std::string, int>("GAME_MAGIC_STARGLOW", 29);
    hElements[38] = std::pair<std::string, int>("GAME_MAGICCLAW", 30);
    hElements[39] = std::pair<std::string, int>("GAME_CATNIPS_NIP2", 23);

    hElements[40] = std::pair<std::string, int>("LEVEL_HEALTH", 24);
    hElements[41] = std::pair<std::string, int>("GAME_HEALTH_POTION1", 26);
    hElements[42] = std::pair<std::string, int>("GAME_HEALTH_POTION2", 27);
    hElements[43] = std::pair<std::string, int>("GAME_HEALTH_POTION3", 25);

    hElements[44] = std::pair<std::string, int>("GAME_DYNAMITE", 34);
    hElements[45] = std::pair<std::string, int>("GAME_POWERUPS_GHOST", 53);
    hElements[46] = std::pair<std::string, int>("GAME_POWERUPS_INVULNERABLE", 54);
    hElements[47] = std::pair<std::string, int>("GAME_POWERUPS_EXTRALIFE", 55);

    hElements[48] = std::pair<std::string, int>("GAME_POWERUPS_FIRESWORD", 56);
    hElements[49] = std::pair<std::string, int>("GAME_POWERUPS_LIGHTNINGSWORD", 57);
    hElements[50] = std::pair<std::string, int>("GAME_POWERUPS_ICESWORD", 58);
    hElements[51] = std::pair<std::string, int>("GAME_CURSES_AMMO", 35);

    hElements[52] = std::pair<std::string, int>("GAME_CURSES_MAGIC", 36);
    hElements[53] = std::pair<std::string, int>("GAME_CURSES_HEALTH", 37);
    hElements[54] = std::pair<std::string, int>("GAME_CURSES_LIFE", 38);
    hElements[55] = std::pair<std::string, int>("GAME_CURSES_TREASURE", 39);

    hElements[56] = std::pair<std::string, int>("GAME_CURSES_FREEZE", 40);
    hElements[57] = std::pair<std::string, int>("GAME_WARP", 32);
    SetClipboardEmpty();

    fAnimTimer = 0;
    iAnimFrame = 0;
}

cInventoryController::~cInventoryController() {

}

cInventoryItem cInventoryController::GetItemByIt(int i) {
    return hElements[i];
}

cInventoryItem cInventoryController::GetItemByID(int id) {
    for (int i = 0; i < InventoryItemsCount; i++)
        if (GetInventoryItemID(hElements[i]) == id)
            return hElements[i];
    if (id == 61)
        return hElements[40]; //hack for LEVEL_HEALTH both under 24 and 61
    return cInventoryItem("", -1);
}

void cInventoryController::SetItemInClipboard(cInventoryItem it, int offx, int offy) {
    hClipboard = it;
    iDragOffX = offx;
    iDragOffY = offy;
}

cInventoryItem cInventoryController::GetItemInClipboard() {
    return hClipboard;
}

void cInventoryController::SetClipboardEmpty() {
    hClipboard = cInventoryItem("", -1);
}

bool cInventoryController::IsClipboardEmpty() {
    if (GetInventoryItemID(hClipboard) == -1)
        return 1;
    return 0;
}

void cInventoryController::MapSwitch() {
    if (GV->editState->hParser->GetBaseLevel() % 2 == 1 && GV->editState->hParser->GetBaseLevel() != 13)
        hElements[3] = std::pair<std::string, int>("GAME_MAPPIECE", 31);
    else
        hElements[3] = std::pair<std::string, int>("LEVEL_GEM", 31);
}

void cInventoryController::DrawDraggedObject() {
    fAnimTimer += hge->Timer_GetDelta();
    while (fAnimTimer > 0.125f) {
        fAnimTimer -= 0.125f;
        iAnimFrame++;
        if (iAnimFrame > 23)
            iAnimFrame = 0;
    }
    if (IsClipboardEmpty()) return;
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    if (!hge->Input_GetKeyState(HGEK_LBUTTON)) {
        gcn::Widget *widg = GV->editState->conMain;
        gcn::Widget *wit = widg;
        while (wit != NULL) {
            widg = wit;
            wit = wit->getWidgetAt(mx, my);
            int x, y;
            if (wit != 0) {
                wit->getAbsolutePosition(x, y);
                mx -= x;
                my -= y;
            }
        }
        if (!strcmp(widg->getId().c_str(), "INVTAB") && widg->isEnabled()) {
            SHR::InvTab *tab = (SHR::InvTab *) widg;
            tab->ObjectDropped();
        }
        SetClipboardEmpty();
        return;
    }
    auto asset = GV->editState->SprBank->GetAssetByID(hClipboard.first.c_str());
    hgeSprite* spr = GV->sprSmiley;
    if (asset) {
        auto img = asset->GetIMGByIterator(0);
        if (img) spr = img->GetSprite();
    }
    spr->SetColor(0xFFFFFFFF);
    spr->Render(mx - iDragOffX, my - iDragOffY);
}
