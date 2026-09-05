#include "newMap.h"
#include "../langID.h"
#include "../states/dialog.h"
#include "../version.h"
#include "../states/editing_ww.h"
#include "../states/loadmap.h"
#include "../shared/cSFS.h"

winNewMap::winNewMap()
    : cWindow(GETL2S("NewMap", "NewMap"), 420, 550),
      labName(std::string(GETL2S("NewMap", "MapName")) + ':'),
      labAuthor(std::string(GETL2S("NewMap", "MapAuthor")) + ':'),
      labPlaneSize(std::string(GETL2S("NewMap", "MainPlaneSize")) + ':'),
      labTarget(std::string(GETL2S("NewMap", "TargetGame")) + ':'),
      labTargetValue("-"),
      tfName(), tfAuthor(), tfPlaneWidth("300"), labX("x"), tfPlaneHeight("100"),
      targetGameTree(), scrollArea(&targetGameTree),
      cbAddBonusPlanes(GV->hGfxInterface, GETL2S("NewMap", "AddBackFrontPlanes"), true),
      butOK(GV->hGfxInterface, GETL2S("NewMap", "Create"))
{
    labName.adjustSize();
    labAuthor.adjustSize();
    labPlaneSize.adjustSize();

    int labsWidthMax = std::max(std::max(labName.getWidth(), labAuthor.getWidth()), labPlaneSize.getWidth()) + 20,
        tfWidth = 380 - labsWidthMax, x = 20, y = 25;

    myWin.add(&labName, x, y);

    tfName.setDimension(gcn::Rectangle(0, 0, tfWidth, 20));
    tfName.addActionListener(this);
    tfName.setMarkedInvalid(true);
    tfName.setMaxLength(58);
    myWin.add(&tfName, x + labsWidthMax, y - 1);

    y += 25;

    myWin.add(&labAuthor, x, y);

    tfAuthor.setDimension(gcn::Rectangle(0, 0, tfWidth, 20));
    tfAuthor.addActionListener(this);
    tfAuthor.setMarkedInvalid(true);
    tfAuthor.setMaxLength(63);
    myWin.add(&tfAuthor, x + labsWidthMax, y - 1);

    y += 25;

    myWin.add(&labPlaneSize, x, y);

    int subW = (tfWidth - 20) / 2;

    tfPlaneWidth.SetNumerical(1, 0);
    tfPlaneWidth.setMaxLength(5);
    tfPlaneWidth.setDimension(gcn::Rectangle(0, 0, subW, 20));
    myWin.add(&tfPlaneWidth, x + labsWidthMax, y - 1);

    labX.adjustSize();
    myWin.add(&labX, x + labsWidthMax + subW + 7, y);

    tfPlaneHeight.SetNumerical(1, 0);
    tfPlaneHeight.setMaxLength(5);
    tfPlaneHeight.setDimension(gcn::Rectangle(0, 0, subW, 20));
    myWin.add(&tfPlaneHeight, x + labsWidthMax + tfWidth - subW, y - 1);

    y += 30;

    for (WWD::GAME game = WWD::GAME::Games_First; game <= WWD::GAME::Games_Last; ++game) {
        auto folder = std::make_shared<SHR::TreeFolder>(WWD::GAME_NAMES[game], GV->sprGamesSmall[game]);

        int levels = 0;
        bool separateLevelIcons = false;
        switch (game) {
            case WWD::Game_Claw:
                levels = 14;
                separateLevelIcons = true;
            break;
            case WWD::Game_Gruntz:
                separateLevelIcons = true;
            case WWD::Game_GetMedieval:
                levels = 8;
            break;
            case WWD::Game_Claw2:
                levels = 1;
            break;
            default:
            break;
        }

        if (levels) {
            if (separateLevelIcons) {
                wchar_t tmp[16];
                MultiByteToWideChar(CP_UTF8, 0, WWD::GAME_NAMES[game], -1, tmp, 16);
                wchar_t* end = tmp + strlen(WWD::GAME_NAMES[game]);
                for (int i = 1; i <= levels; ++i) {
                    wsprintfW(end, L"_%d", i);
                    folder->AddElement(std::make_shared<SHR::TreeElement>(GETL2SV("NewMap", tmp),
                            GV->sprLevelsMicro16[game - WWD::Games_First][i - 1]));
                }
            } else {
                char tmp[16]; char* num = NULL;
                strcpy(tmp, GETL2S("NewMap", "Area"));
                num = tmp + strlen(tmp);
                for (int i = 1; i <= levels; ++i) {
                    sprintf(num, " %d", i);
                    folder->AddElement(std::make_shared<SHR::TreeElement>(tmp, GV->sprGamesSmall[game]));
                }
            }
        }

        targetGameTree.AddElement(folder);
        if (!GV->gamePaths[game].empty()) {
            folder->SetOpened(true);
        }
    }

    targetGameTree.addActionListener(this);
    targetGameTree.adjustSize();
    scrollArea.setWidth(380);
    scrollArea.setHeight(290);

    myWin.add(&scrollArea, x, y);

    y += 300;

    myWin.add(&labTarget, x, y);
    myWin.add(&labTargetValue, x + labTarget.getWidth() + 5, y);

    y += 30;

    cbAddBonusPlanes.adjustSize();
    cbAddBonusPlanes.setVisible(false);
    myWin.add(&cbAddBonusPlanes, x, y);
    
    butOK.setDimension(gcn::Rectangle(0, 0, 100, 33));
    butOK.addActionListener(this);
    butOK.setEnabled(false);
    myWin.add(&butOK, 300, 480);
}

void winNewMap::Draw(int piCode) {

}

void winNewMap::Think() {
}

void winNewMap::action(const ActionEvent &actionEvent) {
    auto element = targetGameTree.GetSelectedElement();

    if (actionEvent.getSource() == &targetGameTree) {
        std::string newCaption("-");
        if (element) {
            newCaption = std::string(element->GetParent()->GetName()) + " - " + element->GetName();
            while (GV->fntMyriad16->GetStringWidth(newCaption.c_str()) > 380 - labTarget.getWidth()) {
                newCaption.resize(newCaption.size() - 5);
                newCaption += "...";
            }
        }
        labTargetValue.setCaption(newCaption);
        labTargetValue.adjustSize();

        cbAddBonusPlanes.setVisible(element && element->GetRoot()->GetIndexOf(element->GetParent()) == WWD::Game_Claw - WWD::Games_First);
    } else if (actionEvent.getSource() == &butOK) {
        int plw = atoi(tfPlaneWidth.getText().c_str()), plh = atoi(tfPlaneHeight.getText().c_str());
        if (plw < 50 || plh < 50) {
            GV->StateMgr->Push(
                    new State::Dialog(PRODUCT_NAME, GETL2S("NewMap", "InvalidPlaneSize"), ST_DIALOG_ICON_ERROR, ST_DIALOG_BUT_OK));
            return;
        }

        int game = element->GetRoot()->GetIndexOf(element->GetParent()) + WWD::Games_First;
        int baseLevel = element->GetParent()->GetIndexOf(element) + 1;
        char name[64], author[64];
        char *fixname = GV->editState->FixLevelName(baseLevel, tfName.getText().c_str());
        strcpy(name, fixname);
        delete[] fixname;
        strcpy(author, tfAuthor.getText().c_str());

        if (game == WWD::Game_Claw) {
            int size;
            char tmp[10];
            sprintf(tmp, "lvl%d.wms", baseLevel);
            cSFS_Repository repo("data.sfs");
            char *ptr = repo.GetFileAsRawData(tmp, &size);
            GV->StateMgr->Push(
                    new State::LoadMap(ptr, size, cbAddBonusPlanes.isSelected(), plw, plh, name, author)
            );
        } else {
            auto* parser = new WWD::Parser((WWD::GAME)game, baseLevel, plw, plh);
            parser->SetName(name);
            parser->SetAuthor(author);
            GV->StateMgr->Push(new State::LoadMap(parser));
        }
        Close();
        return;
    }

    butOK.setEnabled(element && !tfName.getText().empty() && !tfAuthor.getText().empty()
        && !tfPlaneWidth.getText().empty() && !tfPlaneHeight.getText().empty());
}

void winNewMap::Open() {
    tfName.setText("");
    tfAuthor.setText("");
    tfPlaneWidth.setText("300");
    tfPlaneHeight.setText("100");
    targetGameTree.SetSelectedElement(NULL);
    scrollArea.setVerticalScrollAmount(0);
    cbAddBonusPlanes.setSelected(true);
    cWindow::Open();
}
