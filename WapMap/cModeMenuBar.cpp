#include "cModeMenuBar.h"
#include "globals.h"
#include "states/editing_ww.h"
#include "langID.h"
#include "cAppMenu.h"
#include "../shared/gcnWidgets/wComboButton.h"

cModeMenuBar::cModeMenuBar(int startX) {
    bEnabled = 1;
    bVisible = 1;

    vSeparators.push_back(startX);
}

cModeMenuBar::~cModeMenuBar() {

}

void cModeMenuBar::SetVisible(bool bVis) {
    if (bVis == bVisible) return;
    bVisible = bVis;
    _setVisible(bVis);
}

void cModeMenuBar::SetEnabled(bool bEn) {
    if (bEn == bEnabled) return;
    bEnabled = bEn;
    _setEnabled(bEn);
}

cmmbTile::cmmbTile(int startX) : cModeMenuBar(startX) {
    ddActivePlane = new SHR::DropDown();
    ddActivePlane->setListModel(new State::EditingWWlModel(GV->editState, LMODEL_PLANES));
    ddActivePlane->setDimension(gcn::Rectangle(0, 0, 150, 20));
    ddActivePlane->addActionListener(GV->editState->al);
    ddActivePlane->SetGfx(&GV->gcnParts);
    ddActivePlane->adjustHeight();
    ddActivePlane->SetTooltip(GETL2("Tooltip", Lang_TT_ActivePlane));
    GV->editState->conMain->add(ddActivePlane, startX + 5, LAY_MODEBAR_Y + 3);

    startX += 160;
    vSeparators.push_back(startX);

    butIconSelect = GV->editState->MakeButton(startX, LAY_MODEBAR_Y - 3, Icon_Select, GV->editState->conMain);
    butIconSelect->setRenderBG(0);
    butIconSelect->SetTooltip(GETL2("Tooltip", Lang_TT_Select));
    startX += 32;
    butIconPencil = GV->editState->MakeButton(startX, LAY_MODEBAR_Y - 3, Icon_Pencil, GV->editState->conMain);
    butIconPencil->setRenderBG(0);
    butIconPencil->SetTooltip(GETL2("Tooltip", Lang_TT_Pencil));
    startX += 32;
    butIconBrush = GV->editState->MakeButton(startX, LAY_MODEBAR_Y - 3, Icon_Brush, GV->editState->conMain);
    butIconBrush->setRenderBG(0);
    butIconBrush->SetTooltip(GETL2("Tooltip", Lang_TT_Brush));
    startX += 32;
    butIconFill = GV->editState->MakeButton(startX, LAY_MODEBAR_Y - 3, Icon_Fill, GV->editState->conMain);
    butIconFill->setRenderBG(0);
    butIconFill->SetTooltip(GETL2("Tooltip", Lang_TT_Fill));
    startX += 32;
    butIconWriteID = GV->editState->MakeButton(startX, LAY_MODEBAR_Y - 3, Icon_WriteID, GV->editState->conMain);
    butIconWriteID->setRenderBG(0);
    butIconWriteID->SetTooltip(GETL2("Tooltip", Lang_TT_WriteID));
    startX += 32;
}

cmmbTile::~cmmbTile() {
    delete ddActivePlane;
    delete butIconSelect;
    delete butIconPencil;
    delete butIconBrush;
    delete butIconFill;
    delete butIconWriteID;
}

void cmmbTile::_setVisible(bool bVis) {
    ddActivePlane->setVisible(bVis);
    butIconSelect->setVisible(bVis);
    butIconPencil->setVisible(bVis);
    butIconBrush->setVisible(bVis);
    butIconFill->setVisible(bVis);
    butIconWriteID->setVisible(bVis);
}

void cmmbTile::_setEnabled(bool bEn) {
    ddActivePlane->setEnabled(bEn);
    butIconSelect->setEnabled(bEn);
    butIconPencil->setEnabled(bEn);
    butIconBrush->setEnabled(bEn);
    butIconFill->setEnabled(bEn);
    butIconWriteID->setEnabled(bEn);
}

cmmbObject::cmmbObject(int startX) : cModeMenuBar(startX) {
    conNewObject = new SHR::Context(&GV->gcnParts, GV->fntMyriad13);
    conNewObject->hide();
    conNewObject->addActionListener(this);

    int yOff = LAY_MODEBAR_Y - 3;

    butIconSearchObject = GV->editState->MakeButton(69, yOff, Icon_Zoom, GV->editState->conMain, 1, 0);
    butIconSearchObject->SetTooltip(GETL2("Tooltip", Lang_TT_SearchObject));

    int xOff = 360;

    butIconNewObjEmpty = GV->editState->MakeButton(xOff, yOff, Icon_Star, GV->editState->conMain, 1, 0);
    butIconNewObjEmpty->SetTooltip(GETL2("Tooltip", Lang_TT_NewObject));

    butIconCrumblinPeg = GV->editState->MakeButton(xOff, yOff, Icon_CrumblinPeg, GV->editState->conMain, 1, 0);
    butIconCrumblinPeg->SetTooltip(GETL2("Tooltip", Lang_TT_NewCrumblingPeg));
    butIconBreakPlank = GV->editState->MakeButton(xOff, yOff, Icon_BreakPlank, GV->editState->conMain, 1, 0);
    butIconBreakPlank->SetTooltip(GETL2("Tooltip", Lang_TT_NewBreakPlank));
    butIconTogglePeg = GV->editState->MakeButton(xOff, yOff, Icon_TogglePeg, GV->editState->conMain, 1, 0);
    butIconTogglePeg->SetTooltip(GETL2("Tooltip", Lang_TT_NewTogglePeg));
    butIconElevator = GV->editState->MakeButton(xOff, yOff, Icon_SlidingPeg, GV->editState->conMain, 1, 0);
    butIconElevator->SetTooltip(GETL2("Tooltip", Lang_TT_NewElevator));
    butIconPathElevator = GV->editState->MakeButton(xOff, yOff, Icon_ElevatorPath, GV->editState->conMain, 1, 0);
    butIconPathElevator->SetTooltip(GETL2("Tooltip", Lang_TT_NewPathElevator));
    butIconSpringBoard = GV->editState->MakeButton(xOff, yOff, Icon_SpringBoard, GV->editState->conMain, 1, 0);
    butIconSpringBoard->SetTooltip(GETL2("Tooltip", Lang_TT_NewSpringBoard));
    butIconRope = GV->editState->MakeButton(xOff, yOff, Icon_Rope, GV->editState->conMain, 1, 0);
    butIconRope->SetTooltip(GETL2("Tooltip", Lang_TT_NewRope));

    butIconTreasure = GV->editState->MakeButton(xOff, yOff, Icon_Treasure, GV->editState->conMain, 1, 0);
    butIconTreasure->SetTooltip(GETL2("Tooltip", Lang_TT_NewTreasure));
    butIconHealth = GV->editState->MakeButton(xOff, yOff, Icon_Health, GV->editState->conMain, 1, 0);
    butIconHealth->SetTooltip(GETL2("Tooltip", Lang_TT_NewPickup));
    butIconCatnip = GV->editState->MakeButton(xOff, yOff, Icon_Catnip, GV->editState->conMain, 1, 0);
    butIconCatnip->SetTooltip(GETL2("Tooltip", Lang_TT_NewPowerup));
    butIconCurse = GV->editState->MakeButton(xOff, yOff, Icon_Curse, GV->editState->conMain, 1, 0);
    butIconCurse->SetTooltip(GETL2("Tooltip", Lang_TT_NewCurse));
    butIconCrate = GV->editState->MakeButton(xOff, yOff, Icon_Crate, GV->editState->conMain, 1, 0);
    butIconCrate->SetTooltip(GETL2("Tooltip", Lang_TT_NewCrate));
    butIconStatue = GV->editState->MakeButton(xOff, yOff, Icon_Statue, GV->editState->conMain, 1, 0);
    butIconStatue->SetTooltip(GETL2("Tooltip", Lang_TT_NewStatue));

    butIconEnemy = GV->editState->MakeButton(xOff, yOff, Icon_NPC_Rat, GV->editState->conMain, 1, 0);
    butIconEnemy->SetTooltip(GETL2("Tooltip", Lang_TT_Enemy));
    butIconPowderKeg = GV->editState->MakeButton(xOff, yOff, Icon_PowderKeg, GV->editState->conMain, 1, 0);
    butIconPowderKeg->SetTooltip(GETL2("Tooltip", Lang_TT_PowderKeg));
    butIconCannon = GV->editState->MakeButton(xOff, yOff, Icon_CannonWall, GV->editState->conMain, 1, 0);
    butIconCannon->SetTooltip(GETL2("Tooltip", Lang_TT_Cannon));
    butIconSpikes = GV->editState->MakeButton(xOff, yOff, Icon_Spikes, GV->editState->conMain, 1, 0);
    butIconSpikes->SetTooltip(GETL2("Tooltip", Lang_TT_FloorSpike));
    butIconProjectile = GV->editState->MakeButton(xOff, yOff, Icon_Projectile, GV->editState->conMain, 1, 0);
    butIconProjectile->SetTooltip(GETL2("Tooltip", Lang_TT_Shooter));
    butIconCrabNest = GV->editState->MakeButton(xOff, yOff, Icon_CrabNest, GV->editState->conMain, 1, 0);
    butIconCrabNest->SetTooltip(GETL2("Tooltip", Lang_TT_CrabNest));
    butIconStalactite = GV->editState->MakeButton(xOff, yOff, Icon_Stalactite, GV->editState->conMain, 1, 0);
    butIconStalactite->SetTooltip(GETL2("Tooltip", Lang_TT_Stalactite));
    butIconLaser = GV->editState->MakeButton(xOff, yOff, Icon_Laser, GV->editState->conMain, 1, 0);
    butIconLaser->SetTooltip(GETL2("Tooltip", Lang_TT_Laser));

    butIconEyeCandy = GV->editState->MakeButton(xOff, yOff, Icon_Animated, GV->editState->conMain, 1, 0);
    butIconEyeCandy->SetTooltip(GETL2("Tooltip", Lang_TT_Decor));
    butIconText = GV->editState->MakeButton(xOff, yOff, Icon_Text, GV->editState->conMain, 1, 0);
    butIconText->SetTooltip(GETL2("Tooltip", Lang_TT_AddText));
    butIconShake = GV->editState->MakeButton(xOff, yOff, Icon_Shake, GV->editState->conMain, 1, 0);
    butIconShake->SetTooltip(GETL2("Tooltip", Lang_TT_Shake));
    butIconCheckpoint = GV->editState->MakeButton(xOff, yOff, Icon_Flag, GV->editState->conMain, 1, 0);
    butIconCheckpoint->SetTooltip(GETL2("Tooltip", Lang_TT_Checkpoint));
    butIconWarp = GV->editState->MakeButton(xOff, yOff, Icon_Warp, GV->editState->conMain, 1, 0);
    butIconWarp->SetTooltip(GETL2("Tooltip", Lang_TT_Warp));
    butIconDialog = GV->editState->MakeButton(xOff, yOff, Icon_Exclamation, GV->editState->conMain, 1, 0);
    butIconDialog->SetTooltip(GETL2("Tooltip", Lang_TT_DialogTrigger));
    butIconSound = GV->editState->MakeButton(xOff, yOff, Icon_Music, GV->editState->conMain, 1, 0);
    butIconSound->SetTooltip(GETL2("Tooltip", Lang_TT_AmbientSound));
    butIconMapPiece = GV->editState->MakeButton(xOff, yOff, Icon_Map, GV->editState->conMain, 1, 0);
    butIconMapPiece->SetTooltip(GETL2("Tooltip", Lang_TT_EndOfLevel));

    vButtons.push_back(butIconSearchObject);
    vButtons.push_back(butIconNewObjEmpty);
    vButtons.push_back(butIconCrumblinPeg);
    vButtons.push_back(butIconBreakPlank);
    vButtons.push_back(butIconTogglePeg);
    vButtons.push_back(butIconElevator); //5
    vButtons.push_back(butIconPathElevator);
    vButtons.push_back(butIconSpringBoard);
    vButtons.push_back(butIconRope);
    vButtons.push_back(butIconTreasure);
    vButtons.push_back(butIconHealth); //10
    vButtons.push_back(butIconCatnip);
    vButtons.push_back(butIconCurse);
    vButtons.push_back(butIconCrate);
    vButtons.push_back(butIconStatue);
    vButtons.push_back(butIconEnemy); //15-19
    vButtons.push_back(butIconPowderKeg); //20
    vButtons.push_back(butIconCannon);
    vButtons.push_back(butIconSpikes);
    vButtons.push_back(butIconProjectile);
    vButtons.push_back(butIconCrabNest); //25
    vButtons.push_back(butIconStalactite);
    vButtons.push_back(butIconLaser);
    vButtons.push_back(butIconEyeCandy);
    vButtons.push_back(butIconText);
    vButtons.push_back(butIconShake); //30
    vButtons.push_back(butIconCheckpoint);
    vButtons.push_back(butIconWarp);
    vButtons.push_back(butIconDialog);
    vButtons.push_back(butIconSound);
    vButtons.push_back(butIconMapPiece); //35

    for (int i = 0; i < vButtons.size(); i++)
        vButtons[i]->setRenderBG(0);

    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_ADV, GETL2S("ContextNewObj", "Empty"), GV->sprIcons16[Icon16_Star],
                             butIconNewObjEmpty));
    vContextElements.push_back(cNewObjContextEl(NOBJCON_CRUMBLE, GETL2S("ContextNewObj", "CrumblingPeg"),
                                                GV->sprIcons16[Icon16_CrumblingPeg], butIconCrumblinPeg));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_BREAKPL, GETL2S("ContextNewObj", "BreakPlank"), GV->sprIcons16[Icon16_BreakPlank],
                             butIconBreakPlank));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_TOGGLE, GETL2S("ContextNewObj", "TogglePeg"), GV->sprIcons16[Icon16_TogglePeg],
                             butIconTogglePeg));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_ELEVATOR, GETL2S("ContextNewObj", "Elevator"), GV->sprIcons16[Icon16_SlidingPeg],
                             butIconElevator));
    vContextElements.push_back(cNewObjContextEl(NOBJCON_PATHELEV, GETL2S("ContextNewObj", "PathElevator"),
                                                GV->sprIcons16[Icon16_ElevatorPath], butIconPathElevator));
    vContextElements.push_back(cNewObjContextEl(NOBJCON_SPRINGBRD, GETL2S("ContextNewObj", "SpringBoard"),
                                                GV->sprIcons16[Icon16_SpringBoard], butIconSpringBoard));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_ROPE, GETL2S("ContextNewObj", "Rope"), GV->sprIcons16[Icon16_Rope], butIconRope));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_TREASURE, GETL2S("ContextNewObj", "Treasure"), GV->sprIcons16[Icon16_Treasure],
                             butIconTreasure));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_HEALTH, GETL2S("ContextNewObj", "Health"), GV->sprIcons16[Icon16_Health],
                             butIconHealth));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_CATNIP, GETL2S("ContextNewObj", "Catnip"), GV->sprIcons16[Icon16_Catnip],
                             butIconCatnip));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_CURSE, GETL2S("ContextNewObj", "Curse"), GV->sprIcons16[Icon16_Curse],
                             butIconCurse));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_CRATE, GETL2S("ContextNewObj", "Crate"), GV->sprIcons16[Icon16_Crate],
                             butIconCrate));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_STATUE, GETL2S("ContextNewObj", "Statue"), GV->sprIcons16[Icon16_Statue],
                             butIconStatue));
    //vContextElements.push_back(cNewObjContextEl(NOBJCON_NPC,        GETL2S("ContextNewObj", "Enemy"), GV->sprIcons16[Icon16_NPC_Rat], butIconNPC));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_POWDER, GETL2S("ContextNewObj", "PowderKeg"), GV->sprIcons16[Icon16_PowderKeg],
                             butIconPowderKeg));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_WALCAN, GETL2S("ContextNewObj", "Cannon"), GV->sprIcons16[Icon16_CannonWall],
                             butIconCannon));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_SPIKES, GETL2S("ContextNewObj", "Spikes"), GV->sprIcons16[Icon16_Spikes],
                             butIconSpikes));
    vContextElements.push_back(cNewObjContextEl(NOBJCON_PROJECTILE, GETL2S("ContextNewObj", "Projectile"),
                                                GV->sprIcons16[Icon16_Projectile], butIconProjectile));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_CRABNEST, GETL2S("ContextNewObj", "CrabNest"), GV->sprIcons16[Icon16_CrabNest],
                             butIconCrabNest));
    vContextElements.push_back(cNewObjContextEl(NOBJCON_STALACTITE, GETL2S("ContextNewObj", "Stalactite"),
                                                GV->sprIcons16[Icon16_Stalactite], butIconStalactite));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_LASER, GETL2S("ContextNewObj", "Laser"), GV->sprIcons16[Icon16_Laser],
                             butIconLaser));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_EYECANDY, GETL2S("ContextNewObj", "EyeCandy"), GV->sprIcons16[Icon16_Animated],
                             butIconEyeCandy));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_TEXT, GETL2S("ContextNewObj", "Text"), GV->sprIcons16[Icon16_Text], butIconText));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_SHAKE, GETL2S("ContextNewObj", "Shake"), GV->sprIcons16[Icon16_Shake],
                             butIconShake));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_CPOINT, GETL2S("ContextNewObj", "Checkpoint"), GV->sprIcons16[Icon16_Flag],
                             butIconCheckpoint));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_WARP, GETL2S("ContextNewObj", "Warp"), GV->sprIcons16[Icon16_Warp], butIconWarp));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_DIALOG, GETL2S("ContextNewObj", "Dialog"), GV->sprIcons16[Icon16_Exclamation],
                             butIconDialog));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_SOUND, GETL2S("ContextNewObj", "Sound"), GV->sprIcons16[Icon16_Music],
                             butIconSound));
    vContextElements.push_back(
            cNewObjContextEl(NOBJCON_MAP, GETL2S("ContextNewObj", "EndOfLevel"), GV->sprIcons16[Icon16_Map],
                             butIconMapPiece));
}

void cmmbObject::DocumentSwitched() {
    WWD::GAME game = GV->editState->hParser->GetGame();
    int base = GV->editState->hParser->GetBaseLevel();
    int xOff = GV->editState->cbutActiveMode->getWidth() + 12;

    vSeparators.resize(1);
    for (auto but : vButtons) {
        but->setId("OFF");
        but->setVisible(false);
    }

#define ADD_BUTTON(button, condition) if (condition) { \
    button->setId(""); \
    button->setVisible(bVisible); \
    button->setX(xOff); \
    xOff += 32; \
}

#define ADD_SEPARATOR() vSeparators.push_back(xOff + 3); xOff += 7;

    ADD_BUTTON(butIconSearchObject, true);
    ADD_SEPARATOR();
    ADD_BUTTON(butIconNewObjEmpty, true);
    if (game == WWD::Game_Claw) {
        ADD_SEPARATOR();
        ADD_BUTTON(butIconCrumblinPeg,
                   !(base == 2 || base == 5 || base == 8 || base == 9 || base == 11));
        ADD_BUTTON(butIconBreakPlank, (base == 5 || base == 11));
        ADD_BUTTON(butIconTogglePeg,
                   !(base == 3 || base == 4 || base == 6 || base == 7 || base == 9));
        ADD_BUTTON(butIconElevator, true);
        ADD_BUTTON(butIconPathElevator, true);
        ADD_BUTTON(butIconSpringBoard, base == 4 || base == 6 || base == 7 || base == 9 || base == 12 || base == 13);
        ADD_BUTTON(butIconRope, base == 3 || base == 6 || base == 8 || base == 9 || base == 10 || base == 13 || base == 14);
        ADD_SEPARATOR();
        ADD_BUTTON(butIconTreasure, true);
        ADD_BUTTON(butIconHealth, true);
        ADD_BUTTON(butIconCatnip, true);
        ADD_BUTTON(butIconCurse, true);
        ADD_BUTTON(butIconCrate, base != 12);
        ADD_BUTTON(butIconStatue, base == 5);
        ADD_SEPARATOR();
        ADD_BUTTON(butIconEnemy, true);
        ADD_BUTTON(butIconPowderKeg, !(base == 5 || base == 7 || base == 10 || base == 11 || base == 12 || base == 14));
        ADD_BUTTON(butIconCannon, base == 2 || base == 9);
        ADD_BUTTON(butIconSpikes, base == 3 || base == 4 || base == 9 || base == 10 || base >= 12);
        ADD_BUTTON(butIconProjectile, base == 9 || base == 10 || base == 13 || base == 14);
        ADD_BUTTON(butIconCrabNest, base == 7);
        ADD_BUTTON(butIconStalactite, base == 12);
        ADD_BUTTON(butIconLaser, base == 11);
        ADD_SEPARATOR();
        ADD_BUTTON(butIconEyeCandy, true);
        ADD_BUTTON(butIconText, true);
        ADD_BUTTON(butIconShake, true);
        ADD_BUTTON(butIconCheckpoint, true);
        ADD_BUTTON(butIconWarp, true);
        ADD_BUTTON(butIconDialog, true);
        ADD_BUTTON(butIconSound, true);
        ADD_BUTTON(butIconMapPiece, base % 2 == 1 && base != 13);
    }

    iContextOffset = 0;
    RebuildContext();
}

void cmmbObject::RebuildContext() {
    conNewObject->ClearElements();
    conNewObject->AddElement(NOBJCON_PREV, GETL2S("ContextNewObj", "Previous"), GV->sprIcons16[Icon16_Up]);
    if (iContextOffset == 0)
        conNewObject->GetElementByID(NOBJCON_PREV)->SetEnabled(0);

    int passnum = 10;
    for (int i = iContextOffset;
         i < iContextOffset + passnum && i < vContextElements.size();
         i++)
        if (vContextElements[i].hButton->getId() != "OFF")
            conNewObject->AddElement(vContextElements[i].iID, vContextElements[i].strCaption.c_str(),
                                     vContextElements[i].sprIcon);
        else
            passnum++;

    conNewObject->AddElement(NOBJCON_NEXT, GETL2S("ContextNewObj", "Next"), GV->sprIcons16[Icon16_Down]);

    GV->editState->conmodAtEmpty->GetElementByID(OBJMENU_NEWOBJ)->SetCascade(conNewObject->GetElementsCount() > 3 ? conNewObject : NULL);
    GV->editState->conmodPaste->GetElementByID(OBJMENU_NEWOBJ)->SetCascade(conNewObject->GetElementsCount() > 3 ? conNewObject : NULL);

    if (conNewObject->GetElementsCount() > 3) {
        int passn = 0;
        for (int acc = 0; acc < 10; passn++) {
            if (vContextElements.size() - 1 - passn < 0) break;
            if (vContextElements[vContextElements.size() - 1 - passn].hButton->getId() != "OFF") acc++;
        }
        if (iContextOffset == vContextElements.size() - passn)
            conNewObject->GetElementByID(NOBJCON_NEXT)->SetEnabled(0);

        conNewObject->adjustSize();
    }
}

void cmmbObject::action(const gcn::ActionEvent &actionEvent) {
    if (actionEvent.getSource() == conNewObject) {
        if (conNewObject->GetSelectedID() == NOBJCON_PREV) {
            while (1) {
                if (iContextOffset == 0) break;
                iContextOffset--;
                if (vContextElements[iContextOffset].hButton->getId() != "OFF")
                    break;
            }
            RebuildContext();
            return;
        } else if (conNewObject->GetSelectedID() == NOBJCON_NEXT) {
            int passn = 0;
            for (int acc = 0; acc < 10; passn++) {
                if (vContextElements.size() - 1 - passn < 0) break;
                if (vContextElements[vContextElements.size() - 1 - passn].hButton->getId() != "OFF") acc++;
            }
            int lastpos = vContextElements.size() - passn;
            while (1) {
                if (iContextOffset == lastpos) break;
                iContextOffset++;
                if (vContextElements[iContextOffset].hButton->getId() != "OFF")
                    break;
            }
            RebuildContext();
            return;
        } else {
            for (int i = 0; i < vContextElements.size(); i++)
                if (vContextElements[i].iID == conNewObject->GetSelectedID()) {
                    vContextElements[i].hButton->simulatePress();
                    GV->editState->objContext->setVisible(0);
                    return;
                }
        }
    }
}

cmmbObject::~cmmbObject() {
    for (int i = 0; i < vButtons.size(); i++)
        delete vButtons[i];
}

void cmmbObject::_setVisible(bool bVis) {
    for (int i = 0; i < vButtons.size(); i++)
        vButtons[i]->setVisible(vButtons[i]->getId() == "OFF" ? 0 : bVis);
}

void cmmbObject::_setEnabled(bool bEn) {
    for (int i = 0; i < vButtons.size(); i++)
        vButtons[i]->setEnabled(bEn);
}
