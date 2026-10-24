#include "tileBrowser.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../../shared/commonFunc.h"
#include "../states/imageImport.h"
#include "../databanks/tiles.h"

#define TILES_GROUP_SPACE 32

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
    butRenameTileSet->setEnabled(false);
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
    butChangeTileID->setEnabled(false);
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
#ifdef WAPMAP_ENABLE_IMGUI
    ThinkNative(); return;
#endif
    cBankTile *hTileset = GV->editState->hTileset;
    if (!hTileset || hTileset->GetSetsCount() == 0) {
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
    int dx, dy;
    myWin.getAbsolutePosition(dx, dy);

    unsigned char alpha = myWin.getAlpha();
    DWORD col = SETA(0xFFFFFFFF, alpha);
    DWORD colDark = SETA(GV->colLineDark, alpha);
    DWORD colBright = SETA(GV->colLineBright, alpha);

    //tile sets list separator
    hge->Gfx_RenderLine(dx + 227, dy + 25, dx + 227, dy + myWin.getHeight(), colDark);
    hge->Gfx_RenderLine(dx + 228, dy + 25, dx + 228, dy + myWin.getHeight(), colBright);

    //tile set properties separator
    hge->Gfx_RenderLine(dx + 226, dy + 100, dx + myWin.getWidth(), dy + 100, colDark);
    hge->Gfx_RenderLine(dx + 226, dy + 100, dx + myWin.getWidth(), dy + 101, colBright);

    //tiles separator
    hge->Gfx_RenderLine(dx + 226, dy + 200, dx + myWin.getWidth(), dy + 200, colDark);
    hge->Gfx_RenderLine(dx + 226, dy + 201, dx + myWin.getWidth(), dy + 201, colBright);

    hge->Gfx_SetClipping(dx + 8, dy + 51, 204, 520);
    if (hTileset->GetSetsCount() > 0) {
        int startSet = saTileSets->getVerticalScrollAmount() / 68;
        for (int i = startSet; i < hTileset->GetSetsCount() && i < startSet + (saTileSets->getHeight() / 68) + 2; i++) {
            int drawX = dx + 8;
            int drawY = dy + 40 + 10 + i * 68 - saTileSets->getVerticalScrollAmount();
            cTileImageSet *ts = hTileset->GetSet(i);
            cTile *ico = ts->GetTileByIterator(0);

            if (i == iSelectedTileSet || i == iHighlightedTS) {
                SHR::SetQuad(&q, SETA(i == iSelectedTileSet ? 0x331796ff : 0x33a1a1a1, myWin.getAlphaModifier() * 0x33),
                             drawX, drawY, drawX + 198,drawY + 68);
                hge->Gfx_RenderQuad(&q);
            }

            hge->Gfx_RenderLine(drawX + 10 - 2, drawY + 10 - 1, drawX + 10 + 50, drawY + 10 - 1, colDark);
            hge->Gfx_RenderLine(drawX + 10 - 1, drawY + 10 + 50, drawX + 10 + 50, drawY + 10 + 50, colDark);
            hge->Gfx_RenderLine(drawX + 10 - 1, drawY + 10 - 1, drawX + 10 - 1, drawY + 10 + 50, colDark);
            hge->Gfx_RenderLine(drawX + 10 + 50, drawY + 10 - 1, drawX + 10 + 50, drawY + 10 + 50, colDark);

            hge->Gfx_RenderLine(drawX + 10 - 1, drawY + 10, drawX + 10 + 49, drawY + 10, colBright);
            hge->Gfx_RenderLine(drawX + 10, drawY + 10 + 49, drawX + 10 + 49, drawY + 10 + 49, colBright);
            hge->Gfx_RenderLine(drawX + 10, drawY + 10, drawX + 10, drawY + 10 + 49, colBright);
            hge->Gfx_RenderLine(drawX + 10 + 49, drawY + 10, drawX + 10 + 49, drawY + 10 + 49, colBright);

            ico->GetImage()->SetColor(col);
            ico->GetImage()->RenderStretch(drawX + 10, drawY + 10, drawX + 10 + 48, drawY + 10 + 48);
            GV->fntMyriad16->SetColor(SETA(0xFFc1c1c1, alpha));
            GV->fntMyriad16->Render(drawX + 20 + 48, drawY + 10 + 24 - 14, HGETEXT_LEFT, ts->GetName(), 0);
            GV->fntMyriad16->SetColor(SETA(0xFFe1e1e1, alpha));
            GV->fntMyriad16->printf(drawX + 20 + 48, drawY + 10 + 24, HGETEXT_LEFT, "%d %s", 0, ts->GetTilesCount(),
                                    GETL2S("Win_TileBrowser", "tiles"));
        }

        int tilePickX = dx + 230, tilePickW = myWin.getWidth() - 230 - 16,
            tilePickY = dy + 205, tilePickH = myWin.getHeight() - 210;
        cTileImageSet *tsPick = hTileset->GetSet(iSelectedTileSet);

        hge->Gfx_SetClipping(dx + 230, dy + 110, 64, 64);
        GV->sprCheckboard->SetColor(col);
        GV->sprCheckboard->Render(dx + 230, dy + 110);
        hge->Gfx_RenderLine(dx + 230 - 2, dy + 110 - 1, dx + 230 + 64, dy + 110 - 1, colDark);
        hge->Gfx_RenderLine(dx + 230 - 1, dy + 110 + 64, dx + 230 + 64, dy + 110 + 64, colDark);
        hge->Gfx_RenderLine(dx + 230 - 1, dy + 110 - 1, dx + 230 - 1, dy + 110 + 64, colDark);
        hge->Gfx_RenderLine(dx + 230 + 64, dy + 110 - 1, dx + 230 + 64, dy + 110 + 64, colDark);

        hge->Gfx_RenderLine(dx + 230 - 1, dy + 110, dx + 230 + 63, dy + 110, colBright);
        hge->Gfx_RenderLine(dx + 230, dy + 110 + 63, dx + 230 + 63, dy + 110 + 63, colBright);
        hge->Gfx_RenderLine(dx + 230, dy + 110, dx + 230, dy + 110 + 63, colBright);
        hge->Gfx_RenderLine(dx + 230 + 63, dy + 110, dx + 230 + 63, dy + 110 + 63, colBright);
        hge->Gfx_SetClipping();
        cTile *seltile = tsPick->GetTileByIterator(iSelectedTile);
        if (seltile) {
            seltile->GetImage()->SetColor(col);
            seltile->GetImage()->RenderStretch(dx + 230, dy + 110, dx + 230 + 64, dy + 110 + 64);
        }

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
                    GV->fntMyriad16->SetColor(col);
                    GV->fntMyriad16->Render(drawX + 3, drawY, HGETEXT_LEFT, label, 0);
                    RenderTileGroup(vtGroups[i], drawX, drawY + borderOffset * 3);
                    yPos += borderOffset * 4 + ((vtGroups[i].size() - 1) / tilesPerRow + 1) * 80;
                }
        }
    }
}

void winTileBrowser::OnDocumentChange() {
#ifdef WAPMAP_ENABLE_IMGUI
    Close();
#endif
    iSelectedTileSet = iSelectedTile = 0;
    iHighlightedTS = iHighlightedT = -1;
    Synchronize();
}

void winTileBrowser::Synchronize() {
    if (!GV->editState->hParser) return;
    cBankTile *hTileset = GV->editState->hTileset;
    cDataController *hDataCtrl = GV->editState->hDataCtrl;

    if (!hTileset || hTileset->GetSetsCount() == 0)
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
    if (!pickedtile) {
        for (auto& group : vtGroups) group.clear();
        bSingleGroup = false; hSingleGroup = nullptr;
        return;
    }

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
#ifdef WAPMAP_ENABLE_IMGUI
    if (!GV->editState->hParser) return;
    native.document = GV->editState->hParser;
    native.open = native.position = native.resetGrid = true;
    native.action = nullptr;
    butImportTileSet->setEnabled(true);
    butAddTiles->setEnabled(true);
    myWin.setVisible(false);
#else
    cWindow::Open();
#endif
    Synchronize();
}
void winTileBrowser::Close() {
#ifdef WAPMAP_ENABLE_IMGUI
    native.Close();
#endif
    cWindow::Close();
}

void winTileBrowser::action(const ActionEvent &actionEvent) {
    if (actionEvent.getSource() == butBrowseFolder) {
        if (iSelectedTileSet != -1 && iSelectedTile != -1) {
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
        if (iSelectedTileSet != -1 && iSelectedTile != -1) {
            cTileImageSet* ts = GV->editState->hTileset->GetSet(iSelectedTileSet);
            cTile* t = ts->GetTileByIterator(iSelectedTile);
            int spritesCount = ts->GetTilesCount() - 1;
            if (iSelectedTile == spritesCount)
                iSelectedTile--;

            std::string strPath = t->GetFile().hFeed->GetAbsoluteLocation() + "/" + t->GetFile().strPath;
            if (std::filesystem::remove(strPath.c_str())) {
                if (!spritesCount) {
                    strPath = strPath.substr(0, strPath.rfind('/'));
                    std::filesystem::remove(strPath.c_str());
                }
            } else {
                GV->Console->Printf("~r~Failed to remove: ~w~%s", strPath.c_str());
            }
        }
    } else if (actionEvent.getSource() == butDeleteTileSet) {
        if (iSelectedTileSet != -1) {
            cTileImageSet* ts = GV->editState->hTileset->GetSet(iSelectedTileSet);
            int setsCount = GV->editState->hTileset->GetSetsCount() - 1;
            if (iSelectedTileSet == setsCount) {
                --iSelectedTileSet;
            }

            std::string strPath =
                GV->editState->hDataCtrl->GetFeed(DB_FEED_CUSTOM)->GetAbsoluteLocation() + "/TILES/" + ts->GetName();
            if (!std::filesystem::remove_all(strPath)) {
                GV->Console->Printf("~r~Failed to remove: ~w~%s", strPath.c_str());
            }
        }
    }
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

        unsigned char alpha = myWin.getAlpha();
        DWORD col = SETA(0xFFFFFFFF, alpha);
        DWORD colDark = SETA(GV->colLineDark, alpha);
        DWORD colBright = SETA(GV->colLineBright, alpha);
        DWORD colBorder = colBright;
        if (tile == selTile) colBorder = SETA(GV->colActive, alpha);
        else if (tile == hlTile) colBorder = SETA(TILE_HIGHLIGHT_COLOR, 0x77 * myWin.getAlphaModifier());
        hge->Gfx_RenderLine(drawX + 5 - 2, drawY + 5 - 1, drawX + 5 + 50, drawY + 5 - 1, colDark);
        hge->Gfx_RenderLine(drawX + 5 - 1, drawY + 5 + 50, drawX + 5 + 50, drawY + 5 + 50, colDark);
        hge->Gfx_RenderLine(drawX + 5 - 1, drawY + 5 - 1, drawX + 5 - 1, drawY + 5 + 50, colDark);
        hge->Gfx_RenderLine(drawX + 5 + 50, drawY + 5 - 1, drawX + 5 + 50, drawY + 5 + 50, colDark);


        hge->Gfx_RenderLine(drawX + 5 - 1, drawY + 5, drawX + 5 + 49, drawY + 5, colBorder);
        hge->Gfx_RenderLine(drawX + 5, drawY + 5 + 49, drawX + 5 + 49, drawY + 5 + 49, colBorder);
        hge->Gfx_RenderLine(drawX + 5, drawY + 5, drawX + 5, drawY + 5 + 49, colBorder);
        hge->Gfx_RenderLine(drawX + 5 + 49, drawY + 5, drawX + 5 + 49, drawY + 5 + 49, colBorder);

        tile->GetImage()->SetColor(col);

        int clipY = std::max(drawY + 5, tilePickY), clipH = std::min(drawY + 5 + 48, tilePickY + tilePickH) - clipY;
        if (clipH > 0) {
            hge->Gfx_SetClipping(drawX + 5, clipY, 48, clipH);
            GV->sprCheckboard->SetColor(col);
            GV->sprCheckboard->Render(drawX + 5, drawY + 5);
            hge->Gfx_SetClipping(tilePickX, tilePickY, tilePickW, tilePickH);
        }
        tile->GetImage()->RenderStretch(drawX + 5, drawY + 5, drawX + 5 + 48, drawY + 5 + 48);

        GV->fntMyriad16->SetColor(SETA(colBorder != colBright ? 0xFFFFFFFF : 0xFFe1e1e1, alpha));
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

#ifdef WAPMAP_ENABLE_IMGUI
#include "../legacyText.h"
void winTileBrowser::ThinkNative() {
    if (!native.open || native.busy) return;
    auto& e = *GV->editState;
    if (native.document != e.hParser || !native.document) { Close(); return; }
    if (!e._isOnTop()) return;
    auto* bank = e.hTileset;
    if (!bank) { Close(); return; }
    if (native.action) {
        auto* widget = native.action; native.action = nullptr;
        int setIndex = -1, assetIndex = -1;
        for (int i = 0; i < bank->GetSetsCount(); ++i) if (bank->GetSet(i) == native.set) { setIndex = i; break; }
        if (setIndex >= 0) {
            auto* set = bank->GetSet(setIndex);
            for (int i = 0; i < set->GetTilesCount(); ++i) if (set->GetTileByIterator(i) == native.asset) { assetIndex = i; break; }
        }
        if (widget == butImportTileSet || (setIndex >= 0 && (assetIndex >= 0 || widget == butAddTiles || widget == butDeleteTileSet))) {
            iSelectedTileSet = setIndex; iSelectedTile = assetIndex;
            native.busy = true;
            action(gcn::ActionEvent(widget, ""));
            native.busy = false;
            Synchronize();
        }
    }
    if (bank->GetModFlag()) Synchronize();
}
void winTileBrowser::DrawImGui() {
    if (!native.open || native.busy) return;
    auto& e = *GV->editState;
    if (native.document != e.hParser || !native.document) { Close(); return; }
    if (!e._isOnTop()) return;
    auto* bank = e.hTileset; if (!bank) { Close(); return; }
    if (native.position) {
        auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_FirstUseEver, {0.5f, 0.5f});
        ImGui::SetNextWindowFocus(); native.position = false;
    }
    ImGui::SetNextWindowSize({800.f, 600}, ImGuiCond_Always);
    const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
    const auto title = LegacyText::ToUtf8(GETL2S("Win_TileBrowser", "WinCaption"), cp) + "###winTileBrowser";
    bool visible = true;
    if (WapMapImGui::BeginFixedWindow(title.c_str(), &visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)) {
        WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, GV->sprTabCloseButton, GETL2S("Win_TileBrowser", "WinCaption"));
        auto* draw = ImGui::GetWindowDrawList(); const auto origin = ImGui::GetWindowPos(); const auto area = myWin.getChildrenArea();
        AssetBrowserUi::Label(*hge, *GV->fntMyriad16, origin, area, labTileSets);
        AssetBrowserUi::Position(area, 5, 35);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, WapMapImGui::Grey(0x13));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0}); ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});
        int pickedSet = -1;
        if (ImGui::BeginChild("sets", {215.f, 520}, ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            ImGuiListClipper clipper; clipper.Begin(bank->GetSetsCount(), 68.f);
            while (clipper.Step()) for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                auto* set = bank->GetSet(i); ImGui::PushID(set); const auto p = ImGui::GetCursorScreenPos();
                if (ImGui::InvisibleButton("##set", {ImGui::GetContentRegionAvail().x, 68.f}, ImGuiButtonFlags_EnableNav)) pickedSet = i;
                auto* childDraw = ImGui::GetWindowDrawList();
                if (iSelectedTileSet == i || ImGui::IsItemHovered() || ImGui::IsItemFocused())
                    childDraw->AddRectFilled(p, {p.x + ImGui::GetContentRegionAvail().x, p.y + 68.f}, iSelectedTileSet == i ? IM_COL32(23, 150, 255, 51) : IM_COL32(161, 161, 161, 51));
                auto* first = set->GetTilesCount() ? set->GetTileByIterator(0) : nullptr;
                AssetBrowserUi::Preview(*hge, first && first->GetImage() ? first->GetImage() : GV->sprSmiley, {p.x + 10, p.y + 10}, 48.f, true, false);
                WapMapImGui::BitmapText(childDraw, *hge, *GV->fntMyriad16, {p.x + 48.f + 20, p.y + 68.f / 2 - 14}, set->GetName());
                const auto count = std::to_string(set->GetTilesCount()) + " " + GETL2S("Win_TileBrowser", "tiles");
                WapMapImGui::BitmapText(childDraw, *hge, *GV->fntMyriad16, {p.x + 48.f + 20, p.y + 68.f / 2}, count.c_str());
                ImGui::PopID();
            }
        }
        ImGui::EndChild(); ImGui::PopStyleColor();
        if (pickedSet >= 0) { iSelectedTileSet = pickedSet; iSelectedTile = 0; Synchronize(); native.resetGrid = true; }
        auto* set = iSelectedTileSet >= 0 && iSelectedTileSet < bank->GetSetsCount() ? bank->GetSet(iSelectedTileSet) : nullptr;
        auto* selected = set && iSelectedTile >= 0 && iSelectedTile < set->GetTilesCount() ? set->GetTileByIterator(iSelectedTile) : nullptr;
        if (set) for (auto* label : {labTileSetName, labTileSetChecksum, labTileCount, labTileSetNameV, labTileSetChecksumV, labTileCountV}) AssetBrowserUi::Label(*hge, *GV->fntMyriad16, origin, area, label);
        if (selected) for (auto* label : {labTileID, labTileModDate, labTileChecksum, labTileOrigin, labTileIDV, labTileModDateV, labTileChecksumV, labTileOriginV}) AssetBrowserUi::Label(*hge, *GV->fntMyriad16, origin, area, label);
        if (selected) AssetBrowserUi::Preview(*hge, selected->GetImage(), {origin.x + 230.f, origin.y + 110.f}, 64.f, true, true);
        for (auto* button : {butImportTileSet, butAddTiles, butRenameTileSet, butDeleteTileSet, butBrowseFolder, butDeleteTile, butChangeTileID, butImageDetails}) if (AssetBrowserUi::Button(*hge, *GV->fntMyriad16, *GV->hGfxInterface, area, button,
            button == butImportTileSet || (button == butAddTiles || button == butDeleteTileSet || button == butRenameTileSet ? set != nullptr : selected != nullptr))) {
            native.action = button; native.set = set; native.asset = selected;
        }
        AssetBrowserUi::Position(area, 220.f, 190.f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, WapMapImGui::Grey(0x2f));
        cTile* picked = nullptr;
        if (ImGui::BeginChild("assets", {574.f, 389.f}, ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            if (native.resetGrid) { ImGui::SetScrollY(0); native.resetGrid = false; }
            if (selected) {
                const char* captions[] = {GETL2S("Win_TileBrowser", "GroupREZ"), GETL2S("Win_TileBrowser", "GroupClaw"), GETL2S("Win_TileBrowser", "GroupCustom")};
                for (int group = 0; group < 3; ++group) if (!vtGroups[group].empty()) {
                    if (!bSingleGroup) {
                        const auto p = ImGui::GetCursorScreenPos();
                        WapMapImGui::BitmapText(ImGui::GetWindowDrawList(), *hge, *GV->fntMyriad16, p, captions[group]);
                        ImGui::Dummy({0, 24});
                    }
                    if (auto* hit = AssetBrowserUi::Grid(*hge, *GV->fntMyriad16, vtGroups[group], selected, false)) picked = hit;
                    if (!bSingleGroup) ImGui::Dummy({0, 20});
                }
            }
        }
        ImGui::EndChild(); ImGui::PopStyleColor(); ImGui::PopStyleVar(2);
        if (picked && set) for (int i = 0; i < set->GetTilesCount(); ++i) if (set->GetTileByIterator(i) == picked) { iSelectedTile = i; Synchronize(); break; }
        draw->AddLine({origin.x + 228.f, origin.y + 25}, {origin.x + 228.f, origin.y + 600}, IM_COL32(86, 86, 86, 255));
        for (float y : {100.f, 200.f}) draw->AddLine({origin.x + 228.f, origin.y + y}, {origin.x + 800.f, origin.y + y}, IM_COL32(86, 86, 86, 255));
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) visible = false;
    }
    ImGui::End(); if (!visible) Close();
}
#endif
