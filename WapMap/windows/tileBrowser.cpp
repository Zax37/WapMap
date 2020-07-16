#include "tileBrowser.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../../shared/commonFunc.h"
#include "../states/imageImport.h"
#include "../databanks/tiles.h"

#define TILES_GROUP_SPACE 32

extern HGE *hge;

winTileBrowser::winTileBrowser() : cWindow(GETL2S("Win_TileBrowser", "WinCaption"), 800, 600) {
    iSelectedTileSet = iSelectedTile = 0;
    iHighlightedTS = iHighlightedT = -1;

    labTileSets = new SHR::Lab(GETL2S("Win_TileBrowser", "Tilesets"));
    labTileSets->adjustSize();
    myWin.add(labTileSets, 8, 15);

    conTileSets = new SHR::Container();
    conTileSets->setDimension(gcn::Rectangle(0, 0, 100, 100));
    conTileSets->setOpaque(false);
    saTileSets = new SHR::ScrollArea(conTileSets, SHR::ScrollArea::SHOW_AUTO, SHR::ScrollArea::SHOW_ALWAYS);
    saTileSets->setDimension(gcn::Rectangle(0, 0, 215, 520));
    myWin.add(saTileSets, 5, 35);
    conTileSets->setWidth(saTileSets->getChildrenArea().width);

    conTiles = new SHR::Container();
    conTiles->setDimension(gcn::Rectangle(0, 0, 100, 100));
    conTiles->setOpaque(0);
    saTiles = new SHR::ScrollArea(conTiles, SHR::ScrollArea::SHOW_AUTO, SHR::ScrollArea::SHOW_ALWAYS);
    saTiles->setDimension(gcn::Rectangle(0, 0, 574, 600 - 211));
    saTiles->setOpaque(0);
    myWin.add(saTiles, 220, 190);
    conTiles->setWidth(saTiles->getChildrenArea().width);

    butImportTileSet = new SHR::But(GV->hGfxInterface, GETL2S("Win_TileBrowser", "ImportTileset"));
    butImportTileSet->setDimension(gcn::Rectangle(0, 0, 145 + 74, 20));
    butImportTileSet->addActionListener(this);
    butImportTileSet->setIcon(GV->sprIcons16[Icon16_Add]);
    butImportTileSet->setEnabled(false);
    myWin.add(butImportTileSet, 3, 560);

    int caplen = 0;
    labTileSetName = new SHR::Lab(GETL2S("Win_TileBrowser", "TilesetName"));
    labTileSetName->adjustSize();
    myWin.add(labTileSetName, 230, 15);
    if (labTileSetName->getWidth() > caplen) caplen = labTileSetName->getWidth();

    labTileSetChecksum = new SHR::Lab(GETL2S("Win_TileBrowser", "TilesetChecksum"));
    labTileSetChecksum->adjustSize();
    myWin.add(labTileSetChecksum, 230, 35);
    if (labTileSetChecksum->getWidth() > caplen) caplen = labTileSetChecksum->getWidth();

    labTileCount = new SHR::Lab(GETL2S("Win_TileBrowser", "TileCount"));
    labTileCount->adjustSize();
    myWin.add(labTileCount, 230, 55);
    if (labTileCount->getWidth() > caplen) caplen = labTileCount->getWidth();

    labTileSetNameV = new SHR::Lab("");
    labTileSetNameV->setColor(0xFFFFFFFF);
    myWin.add(labTileSetNameV, 240 + caplen, 15);
    labTileSetChecksumV = new SHR::Lab("");
    labTileSetChecksumV->setColor(0xFFFFFFFF);
    myWin.add(labTileSetChecksumV, 240 + caplen, 35);
    labTileCountV = new SHR::Lab("");
    labTileCountV->setColor(0xFFFFFFFF);
    myWin.add(labTileCountV, 240 + caplen, 55);

    butAddTiles = new SHR::But(GV->hGfxInterface, GETL2S("Win_TileBrowser", "AddTiles"));
    butAddTiles->setDimension(gcn::Rectangle(0, 0, 150, 20));
    butAddTiles->setIcon(GV->sprIcons16[Icon16_Add]);
    butAddTiles->addActionListener(this);
    butAddTiles->setEnabled(false);
    myWin.add(butAddTiles, 645, 15);

    butRenameTileSet = new SHR::But(GV->hGfxInterface, GETL2S("Win_TileBrowser", "RenameTileset"));
    butRenameTileSet->setDimension(gcn::Rectangle(0, 0, 150, 20));
    butRenameTileSet->setIcon(GV->sprIcons16[Icon16_Pencil]);
    butRenameTileSet->addActionListener(this);
    myWin.add(butRenameTileSet, 645, 35);

    butDeleteTileSet = new SHR::But(GV->hGfxInterface, GETL2S("Win_TileBrowser", "DeleteTileset"));
    butDeleteTileSet->setDimension(gcn::Rectangle(0, 0, 150, 20));
    butDeleteTileSet->setIcon(GV->sprIcons16[Icon16_Trash]);
    butDeleteTileSet->addActionListener(this);
    myWin.add(butDeleteTileSet, 645, 55);

    caplen = 0;
    labTileID = new SHR::Lab(GETL2S("Win_TileBrowser", "TileID"));
    labTileID->adjustSize();
    myWin.add(labTileID, 230 + 70, 90);
    if (labTileID->getWidth() > caplen) caplen = labTileID->getWidth();

    labTileChecksum = new SHR::Lab(GETL2S("Win_TileBrowser", "TileChecksum"));
    labTileChecksum->adjustSize();
    myWin.add(labTileChecksum, 230 + 70, 110);
    if (labTileChecksum->getWidth() > caplen) caplen = labTileChecksum->getWidth();

    labTileModDate = new SHR::Lab(GETL2S("Win_TileBrowser", "TileModDate"));
    labTileModDate->adjustSize();
    myWin.add(labTileModDate, 230 + 70, 130);
    if (labTileModDate->getWidth() > caplen) caplen = labTileModDate->getWidth();

    labTileOrigin = new SHR::Lab(GETL2S("Win_TileBrowser", "TileOrigin"));
    labTileOrigin->adjustSize();
    myWin.add(labTileOrigin, 230 + 70, 150);
    if (labTileOrigin->getWidth() > caplen) caplen = labTileOrigin->getWidth();

    labTileIDV = new SHR::Lab("");
    labTileIDV->setColor(0xFFFFFFFF);
    myWin.add(labTileIDV, 240 + 70 + caplen, 90);
    labTileChecksumV = new SHR::Lab("");
    labTileChecksumV->setColor(0xFFFFFFFF);
    myWin.add(labTileChecksumV, 240 + 70 + caplen, 110);
    labTileModDateV = new SHR::Lab("");
    labTileModDateV->setColor(0xFFFFFFFF);
    myWin.add(labTileModDateV, 240 + 70 + caplen, 130);
    labTileOriginV = new SHR::Lab("");
    labTileOriginV->setColor(0xFFFFFFFF);
    myWin.add(labTileOriginV, 240 + 70 + caplen, 150);

    butChangeTileID = new SHR::But(GV->hGfxInterface, GETL2S("Win_TileBrowser", "ChangeID"));
    butChangeTileID->setDimension(gcn::Rectangle(0, 0, 150, 20));
    butChangeTileID->setIcon(GV->sprIcons16[Icon16_WriteID]);
    butChangeTileID->addActionListener(this);
    myWin.add(butChangeTileID, 645, 90);

    butBrowseFolder = new SHR::But(GV->hGfxInterface, GETL2S("Win_TileBrowser", "BrowseDirectory"));
    butBrowseFolder->setDimension(gcn::Rectangle(0, 0, 150, 20));
    butBrowseFolder->setIcon(GV->sprIcons16[Icon16_Open]);
    butBrowseFolder->addActionListener(this);
    myWin.add(butBrowseFolder, 645, 110);

    butImageDetails = new SHR::But(GV->hGfxInterface, GETL2S("Win_TileBrowser", "ImageDetails"));
    butImageDetails->setDimension(gcn::Rectangle(0, 0, 150, 20));
    butImageDetails->setIcon(GV->sprIcons16[Icon16_Flip]);
    butImageDetails->addActionListener(this);
    butImageDetails->setEnabled(false);
    myWin.add(butImageDetails, 645, 130);

    butDeleteTile = new SHR::But(GV->hGfxInterface, GETL2S("Win_TileBrowser", "DeleteTile"));
    butDeleteTile->setDimension(gcn::Rectangle(0, 0, 150, 20));
    butDeleteTile->setIcon(GV->sprIcons16[Icon16_Trash]);
    butDeleteTile->addActionListener(this);
    myWin.add(butDeleteTile, 645, 150);

    vpTileBrowser = new WIDG::Viewport(this, 0);
    myWin.add(vpTileBrowser, 5, 5);
}

void winTileBrowser::Think() {
    cBankTile *hTileset = GV->editState->hTileset;
    if (hTileset->GetSetsCount() == 0) {
        return;
    }

    if (hTileset->GetModFlag()) {
        Synchronize();
    }

    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    if (myWin.getParent()->getWidgetAt(mx, my) != &myWin) return;
    int dx, dy;
    myWin.getAbsolutePosition(dx, dy);
    mx -= dx;
    my -= dy;

    if (mx > 6 && mx < 210 && my > 45 && my < 45 + 530) {
        int tsid = (my + saTileSets->getVerticalScrollAmount() - 50) / 68;
        iHighlightedTS = (tsid < hTileset->GetSetsCount() ? tsid : -1);
    } else
        iHighlightedTS = -1;

    if (iHighlightedTS != -1 && hge->Input_KeyDown(HGEK_LBUTTON)) {
        iSelectedTileSet = iHighlightedTS;
        iSelectedTile = 0;
        Synchronize();
        saTiles->setVerticalScrollAmount(0);
    }

    cTileImageSet *ts = hTileset->GetSet((iSelectedTileSet));

    int tilePickX = dx + 230, tilePickW = myWin.getWidth() - 230 - 16,
        tilePickY = dy + 205, tilePickH = myWin.getHeight() - 210;
    int tilesPerRow = (tilePickW / 60);
    int borderOffset = (tilePickW - (tilesPerRow * 60)) / 2;
    int scroll = saTiles->getVerticalScrollAmount();

    mx += dx;
    my += dy;
    if (mx > tilePickX && my > tilePickY && mx < tilePickX + tilePickW && my < tilePickY + tilePickH) {
        if (bSingleGroup) {
            iHighlightedT = MouseHandleGroup(*hSingleGroup, tilePickX + borderOffset,
                    tilePickY + borderOffset - scroll, tilesPerRow);
        } else {
            iHighlightedT = -1;
            int yPos = -TILES_GROUP_SPACE;
            for (auto & vtGroup : vtGroups)
                if (!vtGroup.empty()) {
                    yPos += TILES_GROUP_SPACE;
                    int drawX = tilePickX + borderOffset,
                        drawY = tilePickY + borderOffset - scroll + yPos;
                    int hl = MouseHandleGroup(vtGroup, drawX, drawY + borderOffset * 3, tilesPerRow);
                    if (hl != -1 && hl < vtGroup.size()) {
                        cTile *t = vtGroup[hl];
                        for (int j = 0; j < ts->GetTilesCount(); j++)
                            if (ts->GetTileByIterator(j) == t) {
                                iHighlightedT = j;
                                break;
                            }
                    }
                    yPos += borderOffset * 4 + ((vtGroup.size() - 1) / tilesPerRow + 1) * 80;
                }
        }
    } else {
        iHighlightedT = -1;
    }

    if (iHighlightedT != -1 && hge->Input_KeyDown(HGEK_LBUTTON)) {
        iSelectedTile = iHighlightedT;
        Synchronize();
    }
}

void winTileBrowser::Draw(int piCode) {
    cBankTile *hTileset = GV->editState->hTileset;
    cDataController *hDataCtrl = GV->editState->hDataCtrl;
    int dx, dy;
    myWin.getAbsolutePosition(dx, dy);

    //tile sets list separator
    hge->Gfx_RenderLine(dx + 227, dy + 25, dx + 227, dy + myWin.getHeight(), GV->colLineDark);
    hge->Gfx_RenderLine(dx + 228, dy + 25, dx + 228, dy + myWin.getHeight(), GV->colLineBright);

    //tile set properties separator
    hge->Gfx_RenderLine(dx + 226, dy + 100, dx + myWin.getWidth(), dy + 100, GV->colLineDark);
    hge->Gfx_RenderLine(dx + 226, dy + 100, dx + myWin.getWidth(), dy + 101, GV->colLineBright);

    //tiles separator
    hge->Gfx_RenderLine(dx + 226, dy + 200, dx + myWin.getWidth(), dy + 200, GV->colLineDark);
    hge->Gfx_RenderLine(dx + 226, dy + 201, dx + myWin.getWidth(), dy + 201, GV->colLineBright);

    hge->Gfx_SetClipping(dx + 8, dy + 51, 204, 520);
    if (hTileset->GetSetsCount() > 0) {
        int startSet = saTileSets->getVerticalScrollAmount() / 68;
        for (int i = startSet; i < hTileset->GetSetsCount() && i < startSet + (saTileSets->getHeight() / 68) + 2; i++) {
            int drawX = dx + 8;
            int drawY = dy + 40 + 10 + i * 68 - saTileSets->getVerticalScrollAmount();
            cTileImageSet *ts = hTileset->GetSet(i);
            cTile *ico = ts->GetTileByIterator(0);

            if (i == iSelectedTileSet || i == iHighlightedTS) {
                SHR::SetQuad(&q, (i == iSelectedTileSet ? 0x331796ff : 0x33a1a1a1), drawX, drawY, drawX + 198,
                             drawY + 68);
                hge->Gfx_RenderQuad(&q);
            }

            hge->Gfx_RenderLine(drawX + 10 - 2, drawY + 10 - 1, drawX + 10 + 50, drawY + 10 - 1, GV->colLineDark);
            hge->Gfx_RenderLine(drawX + 10 - 1, drawY + 10 + 50, drawX + 10 + 50, drawY + 10 + 50, GV->colLineDark);
            hge->Gfx_RenderLine(drawX + 10 - 1, drawY + 10 - 1, drawX + 10 - 1, drawY + 10 + 50, GV->colLineDark);
            hge->Gfx_RenderLine(drawX + 10 + 50, drawY + 10 - 1, drawX + 10 + 50, drawY + 10 + 50, GV->colLineDark);

            hge->Gfx_RenderLine(drawX + 10 - 1, drawY + 10, drawX + 10 + 49, drawY + 10, GV->colLineBright);
            hge->Gfx_RenderLine(drawX + 10, drawY + 10 + 49, drawX + 10 + 49, drawY + 10 + 49, GV->colLineBright);
            hge->Gfx_RenderLine(drawX + 10, drawY + 10, drawX + 10, drawY + 10 + 49, GV->colLineBright);
            hge->Gfx_RenderLine(drawX + 10 + 49, drawY + 10, drawX + 10 + 49, drawY + 10 + 49, GV->colLineBright);

            ico->GetImage()->SetColor(0xFFFFFFFF);
            ico->GetImage()->RenderStretch(drawX + 10, drawY + 10, drawX + 10 + 48, drawY + 10 + 48);
            GV->fntMyriad16->SetColor(0xFFc1c1c1);
            GV->fntMyriad16->Render(drawX + 20 + 48, drawY + 10 + 24 - 14, HGETEXT_LEFT, ts->GetName(), 0);
            GV->fntMyriad16->SetColor(0xFFe1e1e1);
            GV->fntMyriad16->printf(drawX + 20 + 48, drawY + 10 + 24, HGETEXT_LEFT, "%d %s", 0, ts->GetTilesCount(),
                                    GETL2S("Win_TileBrowser", "tiles"));
        }

        int tilePickX = dx + 230, tilePickW = myWin.getWidth() - 230 - 16,
            tilePickY = dy + 205, tilePickH = myWin.getHeight() - 210;
        cTileImageSet *tsPick = hTileset->GetSet(iSelectedTileSet);

        hge->Gfx_SetClipping(dx + 230, dy + 110, 64, 64);
        GV->sprCheckboard->Render(dx + 230, dy + 110);
        hge->Gfx_RenderLine(dx + 230 - 2, dy + 110 - 1, dx + 230 + 64, dy + 110 - 1, GV->colLineDark);
        hge->Gfx_RenderLine(dx + 230 - 1, dy + 110 + 64, dx + 230 + 64, dy + 110 + 64, GV->colLineDark);
        hge->Gfx_RenderLine(dx + 230 - 1, dy + 110 - 1, dx + 230 - 1, dy + 110 + 64, GV->colLineDark);
        hge->Gfx_RenderLine(dx + 230 + 64, dy + 110 - 1, dx + 230 + 64, dy + 110 + 64, GV->colLineDark);

        hge->Gfx_RenderLine(dx + 230 - 1, dy + 110, dx + 230 + 63, dy + 110, GV->colLineBright);
        hge->Gfx_RenderLine(dx + 230, dy + 110 + 63, dx + 230 + 63, dy + 110 + 63, GV->colLineBright);
        hge->Gfx_RenderLine(dx + 230, dy + 110, dx + 230, dy + 110 + 63, GV->colLineBright);
        hge->Gfx_RenderLine(dx + 230 + 63, dy + 110, dx + 230 + 63, dy + 110 + 63, GV->colLineBright);
        hge->Gfx_SetClipping();
        cTile *seltile = tsPick->GetTileByIterator(iSelectedTile);
        seltile->GetImage()->SetColor(0xFFFFFFFF);
        seltile->GetImage()->RenderStretch(dx + 230, dy + 110, dx + 230 + 64, dy + 110 + 64);

        int tilesPerRow = (tilePickW / 60);
        int borderOffset = (tilePickW - (tilesPerRow * 60)) / 2;
        int scroll = saTiles->getVerticalScrollAmount();
        if (scroll < 0) scroll = 0;

        hge->Gfx_SetClipping(tilePickX, tilePickY, tilePickW, tilePickH);
        if (bSingleGroup) {
            RenderTileGroup(*hSingleGroup, tilePickX + borderOffset, tilePickY + borderOffset - scroll);
        } else {
            int yPos = -TILES_GROUP_SPACE;
            for (int i = 0; i < 3; i++)
                if (!vtGroups[i].empty()) {
                    yPos += TILES_GROUP_SPACE;
                    int drawX = tilePickX + borderOffset,
                        drawY = tilePickY + borderOffset - scroll + yPos;
                    const char *label;
                    if (i == 0) label = GETL2S("Win_TileBrowser", "GroupREZ");
                    else if (i == 1) label = GETL2S("Win_TileBrowser", "GroupClaw");
                    else if (i == 2) label = GETL2S("Win_TileBrowser", "GroupCustom");
                    GV->fntMyriad16->SetColor(0xFFFFFFFF);
                    GV->fntMyriad16->Render(drawX + 3, drawY, HGETEXT_LEFT, label, 0);
                    RenderTileGroup(vtGroups[i], drawX, drawY + borderOffset * 3);
                    yPos += borderOffset * 4 + ((vtGroups[i].size() - 1) / tilesPerRow + 1) * 80;
                }
        }
    }
}

void winTileBrowser::OnDocumentChange() {
    iSelectedTileSet = iSelectedTile = 0;
    iHighlightedTS = iHighlightedT = -1;
    Synchronize();
}

void winTileBrowser::Synchronize() {
    cBankTile *hTileset = GV->editState->hTileset;
    cDataController *hDataCtrl = GV->editState->hDataCtrl;

    if (hTileset->GetSetsCount() == 0)
        return;

    if (iSelectedTileSet < 0 || iSelectedTileSet >= hTileset->GetSetsCount())
        iSelectedTileSet = 0;

    cTileImageSet *ts = hTileset->GetSet(iSelectedTileSet);
    if (iSelectedTile < 0 || iSelectedTile >= ts->GetTilesCount())
        iSelectedTile = 0;

    labTileSetNameV->setCaption(ts->GetName());
    labTileSetNameV->adjustSize();

    char tmp[64];
    sprintf(tmp, "%d", ts->GetTilesCount());
    labTileCountV->setCaption(tmp);
    labTileCountV->adjustSize();

    labTileSetChecksumV->setCaption(ts->GetHash());
    labTileSetChecksumV->adjustSize();

    bool bCustomTS = 1;
    for (int i = 0; i < ts->GetTilesCount(); i++)
        if (ts->GetTileByIterator(i)->GetFile().hFeed != hDataCtrl->GetFeed(DB_FEED_CUSTOM)) {
            bCustomTS = 0;
            break;
        }

    butRenameTileSet->setVisible(bCustomTS);
    butDeleteTileSet->setVisible(bCustomTS);

    cTile *pickedtile = ts->GetTileByIterator(iSelectedTile);

    sprintf(tmp, "%d", pickedtile->GetID());
    labTileIDV->setCaption(tmp);
    labTileIDV->adjustSize();

    labTileChecksumV->setCaption(pickedtile->GetHash());
    labTileChecksumV->adjustSize();

    const char *orig = "NULL";
    cFileFeed *feed = pickedtile->GetFile().hFeed;
    if (feed == hDataCtrl->GetFeed(DB_FEED_REZ))
        orig = GETL2S("Win_TileBrowser", "OriginREZ");
    else if (feed == hDataCtrl->GetFeed(DB_FEED_DISC))
        orig = GETL2S("Win_TileBrowser", "OriginClaw");
    else if (feed == hDataCtrl->GetFeed(DB_FEED_CUSTOM))
        orig = GETL2S("Win_TileBrowser", "OriginCustom");
    labTileOriginV->setCaption(orig);
    labTileOriginV->adjustSize();

    if (feed == hDataCtrl->GetFeed(DB_FEED_CUSTOM)) {
        butChangeTileID->setVisible(1);
        butDeleteTile->setVisible(1);
        butBrowseFolder->setVisible(1);
        butImageDetails->setY(130);
    } else {
        butChangeTileID->setVisible(0);
        butDeleteTile->setVisible(0);
        butBrowseFolder->setVisible(0);
        butImageDetails->setY(90);
    }

    char *buf = SHR::FormatTimeFromUnix(
            pickedtile->GetFile().hFeed->GetFileModTime(pickedtile->GetFile().strPath.c_str()));
    labTileModDateV->setCaption(buf);
    labTileModDateV->adjustSize();
    delete[] buf;

    for (auto & vtGroup : vtGroups)
        vtGroup.clear();

    for (int i = 0; i < ts->GetTilesCount(); i++) {
        cTile *t = ts->GetTileByIterator(i);
        if (t->GetFile().hFeed == hDataCtrl->GetFeed(DB_FEED_REZ))
            vtGroups[0].push_back(t);
        else if (t->GetFile().hFeed == hDataCtrl->GetFeed(DB_FEED_DISC))
            vtGroups[1].push_back(t);
        else if (t->GetFile().hFeed == hDataCtrl->GetFeed(DB_FEED_CUSTOM))
            vtGroups[2].push_back(t);
    }

    bSingleGroup = false;
    hSingleGroup = 0;
    if (!vtGroups[0].empty() && vtGroups[1].empty() && vtGroups[2].empty()) {
        bSingleGroup = true;
        hSingleGroup = &vtGroups[0];
    } else if (vtGroups[0].empty() && !vtGroups[1].empty() && vtGroups[2].empty()) {
        bSingleGroup = true;
        hSingleGroup = &vtGroups[1];
    } else if (vtGroups[0].empty() && vtGroups[1].empty() && !vtGroups[2].empty()) {
        bSingleGroup = true;
        hSingleGroup = &vtGroups[2];
    }

    conTileSets->setHeight((hTileset->GetSetsCount() * 68 + 10));

    int tilePickW = myWin.getWidth() - 230 - 16;
    int tilesPerRow = (tilePickW / 60);
    int borderOffset = (tilePickW - (tilesPerRow * 60)) / 2;

    if (bSingleGroup) {
        conTiles->setHeight(borderOffset * 2 + ((hSingleGroup->size() - 1) / tilesPerRow + 1) * 80);
    } else {
        int len = borderOffset - TILES_GROUP_SPACE;
        for (auto & vtGroup : vtGroups)
            if (!vtGroup.empty()) {
                len += borderOffset * 4 + ((vtGroup.size() - 1) / tilesPerRow + 1) * 80 + TILES_GROUP_SPACE;
            }
        conTiles->setHeight(len);
    }
}

void winTileBrowser::Open() {
    cWindow::Open();
    Synchronize();
}

void winTileBrowser::action(const ActionEvent &actionEvent) {
    if (actionEvent.getSource() == butBrowseFolder) {
        if (iSelectedTileSet && iSelectedTile) {
            cFile f = GV->editState->hTileset->GetSet(iSelectedTileSet)->GetTileByIterator(iSelectedTile)->GetFile();
            std::string filepath = f.hFeed->GetAbsoluteLocation() + "/" + f.strPath;
            size_t pos = filepath.find('/');
            while (pos != std::string::npos) {
                filepath[pos] = '\\';
                pos = filepath.find('/');
            }
            char buf[1024];
            sprintf(buf, "Explorer.exe /select,%s", filepath.c_str());
            system(buf);
        }
    } else if (actionEvent.getSource() == butAddTiles) {
        GV->StateMgr->Push(
                new State::ImageImport(State::ImageImportTile, GV->editState->hTileset->GetSet(iSelectedTileSet)));
    } else if (actionEvent.getSource() == butImportTileSet) {
        GV->StateMgr->Push(new State::ImageImport(State::ImageImportTile, 0));
    } else if (actionEvent.getSource() == butDeleteTile) {
        if (iSelectedTileSet && iSelectedTile) {
            cTile* t = GV->editState->hTileset->GetSet(iSelectedTileSet)->GetTileByIterator(iSelectedTile);
            if (iSelectedTile == GV->editState->hTileset->GetSet(iSelectedTileSet)->GetTilesCount() - 1)
                iSelectedTile--;

            std::string strPath = t->GetFile().hFeed->GetAbsoluteLocation() + "/" + t->GetFile().strPath;
            if (remove(strPath.c_str()) != 0) {
                GV->Console->Printf("~r~Failed to remove: ~w~%s", strPath.c_str());
            }
        }
    } /*else if (actionEvent.getSource() == butDeleteTileSet) {
        if (iSelectedTileSet) {
            cTileImageSet* ts = GV->editState->hTileset->GetSet(iSelectedTileSet);
            std::string strPath =
                GV->editState->hDataCtrl->GetFeed(DB_FEED_CUSTOM)->GetAbsoluteLocation() + "/TILES/" + ts->GetName();
            SHR::RemoveDirR(strPath.c_str());
        }
    }*/
}

void winTileBrowser::RenderTileGroup(std::vector<cTile *> tiles, int x, int y) {
    cTile *selTile = (iSelectedTile == -1 ? 0 : GV->editState->hTileset->GetSet(iSelectedTileSet)->GetTileByIterator(
            iSelectedTile)),
            *hlTile = (iHighlightedT == -1 ? 0 : GV->editState->hTileset->GetSet(iSelectedTileSet)->GetTileByIterator(
            iHighlightedT));

    int tilePickX, tilePickY, tilePickW, tilePickH;
    hge->Gfx_GetClipping(&tilePickX, &tilePickY, &tilePickW, &tilePickH);
    int tilesPerRow = tilePickW / 60;
    for (size_t i = 0; i < tiles.size(); i++) {
        int gridX = (i % tilesPerRow),
            gridY = (i / tilesPerRow);
        int drawX = x + gridX * 60,
            drawY = y + gridY * 80;

        cTile *tile = tiles[i];

        DWORD colBorder = GV->colLineBright;
        if (tile == selTile) colBorder = GV->colActive;
        else if (tile == hlTile) colBorder = TILE_HIGHLIGHT_COLOR;
        hge->Gfx_RenderLine(drawX + 5 - 2, drawY + 5 - 1, drawX + 5 + 50, drawY + 5 - 1, GV->colLineDark);
        hge->Gfx_RenderLine(drawX + 5 - 1, drawY + 5 + 50, drawX + 5 + 50, drawY + 5 + 50, GV->colLineDark);
        hge->Gfx_RenderLine(drawX + 5 - 1, drawY + 5 - 1, drawX + 5 - 1, drawY + 5 + 50, GV->colLineDark);
        hge->Gfx_RenderLine(drawX + 5 + 50, drawY + 5 - 1, drawX + 5 + 50, drawY + 5 + 50, GV->colLineDark);


        hge->Gfx_RenderLine(drawX + 5 - 1, drawY + 5, drawX + 5 + 49, drawY + 5, colBorder);
        hge->Gfx_RenderLine(drawX + 5, drawY + 5 + 49, drawX + 5 + 49, drawY + 5 + 49, colBorder);
        hge->Gfx_RenderLine(drawX + 5, drawY + 5, drawX + 5, drawY + 5 + 49, colBorder);
        hge->Gfx_RenderLine(drawX + 5 + 49, drawY + 5, drawX + 5 + 49, drawY + 5 + 49, colBorder);

        tile->GetImage()->SetColor(0xFFFFFFFF);

        int clipY = std::max(drawY + 5, tilePickY), clipH = std::min(drawY + 5 + 48, tilePickY + tilePickH) - clipY;
        if (clipH > 0) {
            hge->Gfx_SetClipping(drawX + 5, clipY, 48, clipH);
            GV->sprCheckboard->Render(drawX + 5, drawY + 5);
            hge->Gfx_SetClipping(tilePickX, tilePickY, tilePickW, tilePickH);
        }
        tile->GetImage()->RenderStretch(drawX + 5, drawY + 5, drawX + 5 + 48, drawY + 5 + 48);

        GV->fntMyriad16->SetColor((colBorder != GV->colLineBright ? 0xFFFFFFFF : 0xFFe1e1e1));
        GV->fntMyriad16->printf(drawX + 5 + 25, drawY + 50 + 8, HGETEXT_CENTER, "%d", 0, tile->GetID());
    }
}

int winTileBrowser::MouseHandleGroup(std::vector<cTile *> tiles, int x, int y, int tilesPerRow) {
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);

    saTiles->setVerticalScrollAmount(saTiles->getVerticalScrollAmount() + 150 * (-hge->Input_GetMouseWheel()));

    if (mx > x && mx < x + tilesPerRow * 60 - 5 && my > y + 5) {
        int dx = (mx - x) / 60,
                dy = (my - y) / 80;
        int imousetile = (dy * tilesPerRow) + dx;
        if (imousetile < GV->editState->hTileset->GetSet(iSelectedTileSet)->GetTilesCount() && mx > (x + dx * 60 + 5) &&
            mx < (x + dx * 60 + 55) && my > (y + dy * 80 + 5))
            return imousetile;
    }
    return -1;
}
