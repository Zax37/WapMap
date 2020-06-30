#include "imgsetBrowser.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../../shared/commonFunc.h"
#include "../states/imageImport.h"
#include "../states/imageDetails.h"
#include "../databanks/imageSets.h"

#define CONST_IMGSETBROWSER_LISTSETH 90
#define CONST_IMGSETBROWSER_FRAMEICOSIZE 100
#define CONST_IMGSETBROWSER_ISLISTW  310

extern HGE *hge;

winImageSetBrowser::winImageSetBrowser() {
    iSelectedImageSet = iSelectedFrame = 0;
    iHighlightedIS = iHighlightedF = -1;
    myWin = new SHR::Win(&GV->gcnParts, GETL2S("Win_ImageSetBrowser", "WinCaption"));
    myWin->setDimension(gcn::Rectangle(0, 0, 950, 600));
    myWin->setVisible(false);
    myWin->setClose(true);
    myWin->addActionListener(this);

    labImageSets = new SHR::Lab(GETL2S("Win_ImageSetBrowser", "ImageSets"));
    labImageSets->adjustSize();
    myWin->add(labImageSets, 8, 15);

    conImageSets = new SHR::Container();
    conImageSets->setDimension(gcn::Rectangle(0, 0, 100, 100));
    conImageSets->setOpaque(false);
    saImageSets = new SHR::ScrollArea(conImageSets, SHR::ScrollArea::SHOW_AUTO, SHR::ScrollArea::SHOW_ALWAYS);
    saImageSets->setDimension(gcn::Rectangle(0, 0, CONST_IMGSETBROWSER_ISLISTW, 520));
    myWin->add(saImageSets, 5, 35);
    conImageSets->setWidth(saImageSets->getChildrenArea().width);

    conFrames = new SHR::Container();
    conFrames->setDimension(gcn::Rectangle(0, 0, 100, 100));
    conFrames->setOpaque(0);
    saFrames = new SHR::ScrollArea(conFrames, SHR::ScrollArea::SHOW_AUTO, SHR::ScrollArea::SHOW_ALWAYS);
    saFrames->setDimension(gcn::Rectangle(0, 0, 950 - CONST_IMGSETBROWSER_ISLISTW - 11, 600 - 271 - 18));
    saFrames->setOpaque(0);
    myWin->add(saFrames, CONST_IMGSETBROWSER_ISLISTW + 5, 269);
    conFrames->setWidth(saFrames->getChildrenArea().width);

    butImportImageSet = new SHR::But(GV->hGfxInterface, GETL2S("Win_ImageSetBrowser", "ImportImageSet"));
    butImportImageSet->setDimension(gcn::Rectangle(0, 0, CONST_IMGSETBROWSER_ISLISTW, 20));
    butImportImageSet->addActionListener(this);
    butImportImageSet->setIcon(GV->sprIcons16[Icon16_Add]);
    butImportImageSet->setEnabled(false);
    myWin->add(butImportImageSet, 3, 560);

    int caplen = 0;
    labImageSetName = new SHR::Lab(GETL2S("Win_ImageSetBrowser", "ImageSetName"));
    labImageSetName->adjustSize();
    myWin->add(labImageSetName, CONST_IMGSETBROWSER_ISLISTW + 15, 15);
    if (labImageSetName->getWidth() > caplen) caplen = labImageSetName->getWidth();

    labImageSetChecksum = new SHR::Lab(GETL2S("Win_ImageSetBrowser", "ImageSetChecksum"));
    labImageSetChecksum->adjustSize();
    myWin->add(labImageSetChecksum, CONST_IMGSETBROWSER_ISLISTW + 15, 35);
    if (labImageSetChecksum->getWidth() > caplen) caplen = labImageSetChecksum->getWidth();

    labFrameCount = new SHR::Lab(GETL2S("Win_ImageSetBrowser", "FrameCount"));
    labFrameCount->adjustSize();
    myWin->add(labFrameCount, CONST_IMGSETBROWSER_ISLISTW + 15, 55);
    if (labFrameCount->getWidth() > caplen) caplen = labFrameCount->getWidth();

    labImageSetNameV = new SHR::Lab("");
    labImageSetNameV->setColor(0xFFFFFFFF);
    myWin->add(labImageSetNameV, CONST_IMGSETBROWSER_ISLISTW + 25 + caplen, 15);
    labImageSetChecksumV = new SHR::Lab("");
    labImageSetChecksumV->setColor(0xFFFFFFFF);
    myWin->add(labImageSetChecksumV, CONST_IMGSETBROWSER_ISLISTW + 25 + caplen, 35);
    labFrameCountV = new SHR::Lab("");
    labFrameCountV->setColor(0xFFFFFFFF);
    myWin->add(labFrameCountV, CONST_IMGSETBROWSER_ISLISTW + 25 + caplen, 55);

    butAddFrames = new SHR::But(GV->hGfxInterface, GETL2S("Win_ImageSetBrowser", "AddFrames"));
    butAddFrames->setDimension(gcn::Rectangle(0, 0, 150, 20));
    butAddFrames->setIcon(GV->sprIcons16[Icon16_Add]);
    butAddFrames->addActionListener(this);
    butAddFrames->setEnabled(false);
    myWin->add(butAddFrames, 645 + 150, 15);

    butRenameImageSet = new SHR::But(GV->hGfxInterface, GETL2S("Win_ImageSetBrowser", "RenameImageSet"));
    butRenameImageSet->setDimension(gcn::Rectangle(0, 0, 150, 20));
    butRenameImageSet->setIcon(GV->sprIcons16[Icon16_Pencil]);
    butRenameImageSet->addActionListener(this);
    //myWin->add(butRenameImageSet, 645+150, 35);

    butDeleteImageSet = new SHR::But(GV->hGfxInterface, GETL2S("Win_ImageSetBrowser", "DeleteImageSet"));
    butDeleteImageSet->setDimension(gcn::Rectangle(0, 0, 150, 20));
    butDeleteImageSet->setIcon(GV->sprIcons16[Icon16_Trash]);
    butDeleteImageSet->addActionListener(this);
    myWin->add(butDeleteImageSet, 645 + 150, 35);

    caplen = 0;
    labFrameID = new SHR::Lab(GETL2S("Win_ImageSetBrowser", "FrameID"));
    labFrameID->adjustSize();
    myWin->add(labFrameID, CONST_IMGSETBROWSER_ISLISTW + 15, 90);
    if (labFrameID->getWidth() > caplen) caplen = labFrameID->getWidth();

    labImageDim = new SHR::Lab(GETL2S("Win_ImageSetBrowser", "FrameDim"));
    labImageDim->adjustSize();
    myWin->add(labImageDim, CONST_IMGSETBROWSER_ISLISTW + 15, 110);
    if (labImageDim->getWidth() > caplen) caplen = labImageDim->getWidth();

    labImageFileName = new SHR::Lab(GETL2S("Win_ImageSetBrowser", "FrameFileName"));
    labImageFileName->adjustSize();
    myWin->add(labImageFileName, CONST_IMGSETBROWSER_ISLISTW + 15, 130);
    if (labImageFileName->getWidth() > caplen) caplen = labImageFileName->getWidth();

    labImagePath = new SHR::Lab(GETL2S("Win_ImageSetBrowser", "FrameFilePath"));
    labImagePath->adjustSize();
    myWin->add(labImagePath, CONST_IMGSETBROWSER_ISLISTW + 15, 150);
    if (labImagePath->getWidth() > caplen) caplen = labImagePath->getWidth();

    labFrameChecksum = new SHR::Lab(GETL2S("Win_ImageSetBrowser", "FrameChecksum"));
    labFrameChecksum->adjustSize();
    myWin->add(labFrameChecksum, CONST_IMGSETBROWSER_ISLISTW + 15, 170);
    if (labFrameChecksum->getWidth() > caplen) caplen = labFrameChecksum->getWidth();

    labFrameModDate = new SHR::Lab(GETL2S("Win_ImageSetBrowser", "FrameModDate"));
    labFrameModDate->adjustSize();
    myWin->add(labFrameModDate, CONST_IMGSETBROWSER_ISLISTW + 15, 190);
    if (labFrameModDate->getWidth() > caplen) caplen = labFrameModDate->getWidth();

    labFrameOrigin = new SHR::Lab(GETL2S("Win_ImageSetBrowser", "FrameOrigin"));
    labFrameOrigin->adjustSize();
    myWin->add(labFrameOrigin, CONST_IMGSETBROWSER_ISLISTW + 15, 210);
    if (labFrameOrigin->getWidth() > caplen) caplen = labFrameOrigin->getWidth();

    labFrameIDV = new SHR::Lab("");
    labFrameIDV->setColor(0xFFFFFFFF);
    myWin->add(labFrameIDV, CONST_IMGSETBROWSER_ISLISTW + 25 + caplen, 90);

    labImageDimV = new SHR::Lab("");
    labImageDimV->setColor(0xFFFFFFFF);
    myWin->add(labImageDimV, CONST_IMGSETBROWSER_ISLISTW + 25 + caplen, 110);
    labImageFileNameV = new SHR::Lab("");
    labImageFileNameV->setColor(0xFFFFFFFF);
    myWin->add(labImageFileNameV, CONST_IMGSETBROWSER_ISLISTW + 25 + caplen, 130);
    labImagePathV = new SHR::Lab("");
    labImagePathV->setColor(0xFFFFFFFF);
    myWin->add(labImagePathV, CONST_IMGSETBROWSER_ISLISTW + 25 + caplen, 150);

    labFrameChecksumV = new SHR::Lab("");
    labFrameChecksumV->setColor(0xFFFFFFFF);
    myWin->add(labFrameChecksumV, CONST_IMGSETBROWSER_ISLISTW + 25 + caplen, 170);
    labFrameModDateV = new SHR::Lab("");
    labFrameModDateV->setColor(0xFFFFFFFF);
    myWin->add(labFrameModDateV, CONST_IMGSETBROWSER_ISLISTW + 25 + caplen, 190);
    labFrameOriginV = new SHR::Lab("");
    labFrameOriginV->setColor(0xFFFFFFFF);
    myWin->add(labFrameOriginV, CONST_IMGSETBROWSER_ISLISTW + 25 + caplen, 210);

    butChangeFrameID = new SHR::But(GV->hGfxInterface, GETL2S("Win_ImageSetBrowser", "ChangeID"));
    butChangeFrameID->setDimension(gcn::Rectangle(0, 0, 150, 20));
    butChangeFrameID->setIcon(GV->sprIcons16[Icon16_WriteID]);
    butChangeFrameID->addActionListener(this);
    //myWin->add(butChangeFrameID, CONST_IMGSETBROWSER_ISLISTW+24, 237);

    butDeleteFrame = new SHR::But(GV->hGfxInterface, GETL2S("Win_ImageSetBrowser", "DeleteFrame"));
    butDeleteFrame->setDimension(gcn::Rectangle(0, 0, 150, 20));
    butDeleteFrame->setIcon(GV->sprIcons16[Icon16_Trash]);
    butDeleteFrame->addActionListener(this);
    myWin->add(butDeleteFrame, CONST_IMGSETBROWSER_ISLISTW + 24 + 150 - 75, 237);

    butBrowseFolder = new SHR::But(GV->hGfxInterface, GETL2S("Win_ImageSetBrowser", "BrowseDirectory"));
    butBrowseFolder->setDimension(gcn::Rectangle(0, 0, 150, 20));
    butBrowseFolder->setIcon(GV->sprIcons16[Icon16_Open]);
    butBrowseFolder->addActionListener(this);
    myWin->add(butBrowseFolder, CONST_IMGSETBROWSER_ISLISTW + 24 + 300 - 75, 237);

    butImageDetails = new SHR::But(GV->hGfxInterface, GETL2S("Win_ImageSetBrowser", "ImageDetails"));
    butImageDetails->setDimension(gcn::Rectangle(0, 0, 150, 20));
    butImageDetails->setIcon(GV->sprIcons16[Icon16_Flip]);
    butImageDetails->addActionListener(this);
    butImageDetails->setEnabled(false);
    myWin->add(butImageDetails, CONST_IMGSETBROWSER_ISLISTW + 24 + 450 - 75, 237);

    vp = new WIDG::Viewport(this, 0);
    myWin->add(vp, 5, 5);
}

winImageSetBrowser::~winImageSetBrowser() {
    delete myWin;
}

void winImageSetBrowser::Think() {
    cBankImageSet *bank = GV->editState->SprBank;
    if (bank->GetAssetsCount() == 0)
        return;

    if (bank->GetModFlag()) {
        Synchronize();
    }

    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    if (myWin->getParent()->getWidgetAt(mx, my) != myWin) return;
    int dx, dy;
    myWin->getAbsolutePosition(dx, dy);
    mx -= dx;
    my -= dy;

    if (mx > 6 && mx < CONST_IMGSETBROWSER_ISLISTW - 5 && my > 45 && my < 45 + 530) {
        int tsid = (my + saImageSets->getVerticalScrollAmount() - 50) / CONST_IMGSETBROWSER_LISTSETH;
        iHighlightedIS = (tsid < bank->GetAssetsCount() ? tsid : -1);
    } else
        iHighlightedIS = -1;

    if (iHighlightedIS != -1 && hge->Input_KeyDown(HGEK_LBUTTON)) {
        iSelectedImageSet = iHighlightedIS;
        iSelectedFrame = 0;
        Synchronize();
        saFrames->setVerticalScrollAmount(0);
    }

    cSprBankAsset *ts = bank->GetAssetByIterator((iSelectedImageSet));


    int tilePickX = dx + CONST_IMGSETBROWSER_ISLISTW + 15, tilePickW =
            myWin->getWidth() - (CONST_IMGSETBROWSER_ISLISTW + 15) - 16,
            tilePickY = dy + 285, tilePickH = myWin->getHeight() - 290;
    int tilesPerRow = (tilePickW / CONST_IMGSETBROWSER_FRAMEICOSIZE),
            rowCount = tilePickH / (CONST_IMGSETBROWSER_FRAMEICOSIZE + 20) + 2;
    int borderoffset = (tilePickW - (tilesPerRow * CONST_IMGSETBROWSER_FRAMEICOSIZE)) / 2;
    int scroll = saFrames->getVerticalScrollAmount();

    mx += dx;
    my += dy;
    if (mx > tilePickX && my > tilePickY && mx < tilePickX + tilePickW && my < tilePickY + tilePickH) {
        if (bSingleGroup) {
            iHighlightedF = MouseHandleGroup(*hSingleGroup, tilePickX + borderoffset, tilePickY + 10 - scroll,
                                             tilesPerRow);
        } else {
            iHighlightedF = -1;
            int ypos = 0;
            for (int i = 0; i < 3; i++)
                if (vtGroups[i].size() > 0) {
                    int drawX = tilePickX + borderoffset,
                            drawY = tilePickY + 10 - scroll + ypos;
                    int hl = MouseHandleGroup(vtGroups[i], drawX, drawY + 20, tilesPerRow);
                    if (hl != -1) {
                        cSprBankAssetIMG *t = vtGroups[i][hl];
                        for (int i = 0; i < ts->GetSpritesCount(); i++)
                            if (ts->GetIMGByIterator(i) == t) {
                                iHighlightedF = i;
                                break;
                            }
                    }
                    ypos += 20 + (vtGroups[i].size() / tilesPerRow + 1) * 80;
                }
        }
    } else {
        iHighlightedF = -1;
    }

    if (iHighlightedF != -1 && hge->Input_KeyDown(HGEK_LBUTTON)) {
        iSelectedFrame = iHighlightedF;
        Synchronize();
    }
}

void winImageSetBrowser::Draw(int piCode) {
    cBankImageSet *bank = GV->editState->SprBank;
    cDataController *hDataCtrl = GV->editState->hDataCtrl;
    int dx, dy;
    myWin->getAbsolutePosition(dx, dy);

    //tile sets list separator
    hge->Gfx_RenderLine(dx + CONST_IMGSETBROWSER_ISLISTW + 12, dy + 25, dx + CONST_IMGSETBROWSER_ISLISTW + 12,
                        dy + myWin->getHeight(), GV->colLineDark);
    hge->Gfx_RenderLine(dx + CONST_IMGSETBROWSER_ISLISTW + 13, dy + 25, dx + CONST_IMGSETBROWSER_ISLISTW + 13,
                        dy + myWin->getHeight(), GV->colLineBright);

    //tile set properties separator
    hge->Gfx_RenderLine(dx + CONST_IMGSETBROWSER_ISLISTW + 11, dy + 100, dx + myWin->getWidth(), dy + 100,
                        GV->colLineDark);
    hge->Gfx_RenderLine(dx + CONST_IMGSETBROWSER_ISLISTW + 11, dy + 100, dx + myWin->getWidth(), dy + 101,
                        GV->colLineBright);

    //tiles separator
    hge->Gfx_RenderLine(dx + CONST_IMGSETBROWSER_ISLISTW + 11, dy + 280, dx + myWin->getWidth(), dy + 280,
                        GV->colLineDark);
    hge->Gfx_RenderLine(dx + CONST_IMGSETBROWSER_ISLISTW + 11, dy + 281, dx + myWin->getWidth(), dy + 281,
                        GV->colLineBright);


    hge->Gfx_SetClipping(dx + 8, dy + 51, CONST_IMGSETBROWSER_ISLISTW - 9, 520);
    if (bank->GetAssetsCount() > 0) {
        int startSet = saImageSets->getVerticalScrollAmount() / CONST_IMGSETBROWSER_LISTSETH;
        for (int i = startSet; i < bank->GetAssetsCount() &&
                               i < startSet + (saImageSets->getHeight() / CONST_IMGSETBROWSER_LISTSETH) + 2; i++) {
            int drawX = dx + 8;
            int drawY = dy + 40 + 10 + i * CONST_IMGSETBROWSER_LISTSETH - saImageSets->getVerticalScrollAmount();
            cSprBankAsset *ts = bank->GetAssetByIterator(i);
            hgeSprite *ico = 0;

            for (int x = 0; x < ts->GetSpritesCount(); x++)
                if (ts->GetIMGByIterator(x)->IsLoaded()) {
                    ico = ts->GetIMGByIterator(x)->GetImage();
                    break;
                }
            if (!ico) ico = GV->sprSmiley;

            if (i == iSelectedImageSet || i == iHighlightedIS) {
                SHR::SetQuad(&q, (i == iSelectedImageSet ? 0x331796ff : 0x33a1a1a1), drawX, drawY,
                             drawX + (CONST_IMGSETBROWSER_ISLISTW - 13), drawY + CONST_IMGSETBROWSER_LISTSETH);
                hge->Gfx_RenderQuad(&q);
            }

            int iPreviewDim = CONST_IMGSETBROWSER_LISTSETH - 20;

            hge->Gfx_RenderLine(drawX + 10 - 2, drawY + 10 - 1, drawX + 10 + iPreviewDim + 2, drawY + 10 - 1,
                                GV->colLineDark);
            hge->Gfx_RenderLine(drawX + 10 - 1, drawY + 10 + iPreviewDim + 2, drawX + 10 + iPreviewDim + 2,
                                drawY + 10 + iPreviewDim + 2, GV->colLineDark);
            hge->Gfx_RenderLine(drawX + 10 - 1, drawY + 10 - 1, drawX + 10 - 1, drawY + 10 + iPreviewDim + 2,
                                GV->colLineDark);
            hge->Gfx_RenderLine(drawX + 10 + iPreviewDim + 2, drawY + 10 - 1, drawX + 10 + iPreviewDim + 2,
                                drawY + 10 + iPreviewDim + 2, GV->colLineDark);

            hge->Gfx_RenderLine(drawX + 10 - 1, drawY + 10, drawX + 10 + iPreviewDim + 1, drawY + 10,
                                GV->colLineBright);
            hge->Gfx_RenderLine(drawX + 10, drawY + 10 + iPreviewDim + 1, drawX + 10 + iPreviewDim + 1,
                                drawY + 10 + iPreviewDim + 1, GV->colLineBright);
            hge->Gfx_RenderLine(drawX + 10, drawY + 10, drawX + 10, drawY + 10 + iPreviewDim + 1, GV->colLineBright);
            hge->Gfx_RenderLine(drawX + 10 + iPreviewDim + 1, drawY + 10, drawX + 10 + iPreviewDim + 1,
                                drawY + 10 + iPreviewDim + 1, GV->colLineBright);

            ico->SetColor(0xFFFFFFFF);
            float fScale = 1;
            if (ico->GetWidth() > iPreviewDim || ico->GetHeight() > iPreviewDim) {
                float fScaleX = float(iPreviewDim) / ico->GetWidth(),
                        fScaleY = float(iPreviewDim) / ico->GetHeight();
                fScale = (fScaleX < fScaleY ? fScaleX : fScaleY);
            }
            float ofx, ofy;
            ico->GetHotSpot(&ofx, &ofy);
            ofx -= ico->GetWidth() / 2;
            ofy -= ico->GetHeight() / 2;
            ofx *= fScale;
            ofy *= fScale;
            ico->RenderEx(drawX + 10 + iPreviewDim / 2 + ofx, drawY + 10 + iPreviewDim / 2 + ofy, 0, fScale, fScale);

            int strw = CONST_IMGSETBROWSER_ISLISTW - (20 + iPreviewDim) - 10;
            std::string str = ts->GetID();
            bool first = 1;
            while (GV->fntMyriad16->GetStringWidth(str.c_str(), 0) > strw) {
                str.erase(str.length() - 2 - (!first * 3));
                str.append("...");
                first = 0;
            }

            GV->fntMyriad16->SetColor(0xFFc1c1c1);
            GV->fntMyriad16->Render(drawX + 20 + iPreviewDim, drawY + 10 + 24 - 14, HGETEXT_LEFT, str.c_str(), 0);
            GV->fntMyriad16->SetColor(0xFFe1e1e1);
            GV->fntMyriad16->printf(drawX + 20 + iPreviewDim, drawY + 10 + 24, HGETEXT_LEFT, "%d %s", 0,
                                    ts->GetSpritesCount(), GETL2S("Win_ImageSetBrowser", "frames"));
        }

        int tilePickX = dx + (CONST_IMGSETBROWSER_ISLISTW + 15), tilePickW =
                myWin->getWidth() - (CONST_IMGSETBROWSER_ISLISTW + 15) - 16,
                tilePickY = dy + 285, tilePickH = myWin->getHeight() - 290;
        cSprBankAsset *tsPick = bank->GetAssetByIterator(iSelectedImageSet);

        int tilesPerRow = (tilePickW / CONST_IMGSETBROWSER_FRAMEICOSIZE),
            rowCount = tilePickH / (CONST_IMGSETBROWSER_FRAMEICOSIZE + 20) + 2;
        int borderOffset = (tilePickW - (tilesPerRow * CONST_IMGSETBROWSER_FRAMEICOSIZE)) / 2;
        int scroll = saFrames->getVerticalScrollAmount();
        if (scroll < 0) scroll = 0;

        hge->Gfx_SetClipping(tilePickX, tilePickY, tilePickW, tilePickH);
        if (bSingleGroup) {
            RenderFrameGroup(*hSingleGroup, tilePickX + borderOffset, tilePickY + 10 - scroll);
        } else {
            int yPos = 0;
            for (int i = 0; i < 3; i++)
                if (vtGroups[i].size() > 0) {
                    int drawX = tilePickX + borderOffset,
                            drawY = tilePickY + 10 - scroll + yPos;
                    const char *label = 0;
                    if (i == 0) label = GETL2S("Win_ImageSetBrowser", "GroupREZ");
                    else if (i == 1) label = GETL2S("Win_ImageSetBrowser", "GroupClaw");
                    else if (i == 2) label = GETL2S("Win_ImageSetBrowser", "GroupCustom");
                    hge->Gfx_RenderLine(drawX, drawY + 15, drawX + tilePickW - borderOffset * 2, drawY + 15,
                                        0xFFe1e1e1);
                    GV->fntMyriad16->SetColor(0xFFFFFFFF);
                    GV->fntMyriad16->Render(drawX, drawY, HGETEXT_LEFT, label, 0);
                    RenderFrameGroup(vtGroups[i], drawX, drawY + 20);
                    yPos += 20 + (vtGroups[i].size() / tilesPerRow + 1) * 80;
                }
        }
    }
}

void winImageSetBrowser::OnDocumentChange() {
    iSelectedImageSet = iSelectedFrame = 0;
    iHighlightedIS = iHighlightedF = -1;
    Synchronize();
}

void winImageSetBrowser::Synchronize() {
    cBankImageSet *bank = GV->editState->SprBank;
    cDataController *hDataCtrl = GV->editState->hDataCtrl;

    if (bank->GetAssetsCount() == 0)
        return;

    if (iSelectedImageSet < 0 || iSelectedImageSet >= bank->GetAssetsCount())
        iSelectedImageSet = 0;
    cSprBankAsset *ts = bank->GetAssetByIterator(iSelectedImageSet);
    if (iSelectedFrame < 0 || iSelectedFrame >= ts->GetSpritesCount())
        iSelectedFrame = 0;

    labImageSetNameV->setCaption(ts->GetID());
    labImageSetNameV->adjustSize();

    char tmp[128];
    sprintf(tmp, "%d", ts->GetSpritesCount());
    labFrameCountV->setCaption(tmp);
    labFrameCountV->adjustSize();

    labImageSetChecksumV->setCaption(ts->GetHash());
    labImageSetChecksumV->adjustSize();

    bool bCustomTS = 1;
    for (int i = 0; i < ts->GetSpritesCount(); i++)
        if (ts->GetIMGByIterator(i)->GetFile().hFeed != hDataCtrl->GetFeed(DB_FEED_CUSTOM)) {
            bCustomTS = 0;
            break;
        }

    butRenameImageSet->setVisible(bCustomTS);
    butDeleteImageSet->setVisible(bCustomTS);

    cSprBankAssetIMG *pickedtile = ts->GetIMGByIterator(iSelectedFrame);

    sprintf(tmp, "%d", pickedtile->GetID());
    labFrameIDV->setCaption(tmp);
    labFrameIDV->adjustSize();

    sprintf(tmp, "%dx%d", int(pickedtile->GetImage()->GetWidth()), int(pickedtile->GetImage()->GetHeight()));
    labImageDimV->setCaption(tmp);
    labImageDimV->adjustSize();

    std::string strPath = pickedtile->GetFile().strPath;
    size_t lslash = strPath.rfind('/');
    std::string fname = strPath.substr(lslash + 1),
            fpath = strPath.substr(0, lslash);
    labImageFileNameV->setCaption(fname);
    labImageFileNameV->adjustSize();

    labImagePathV->setCaption(fpath);
    labImagePathV->adjustSize();

    labFrameChecksumV->setCaption(pickedtile->GetHash());
    labFrameChecksumV->adjustSize();

    const char *orig = "NULL";
    cFileFeed *feed = pickedtile->GetFile().hFeed;
    if (feed == hDataCtrl->GetFeed(DB_FEED_REZ))
        orig = GETL2S("Win_ImageSetBrowser", "OriginREZ");
    else if (feed == hDataCtrl->GetFeed(DB_FEED_DISC))
        orig = GETL2S("Win_ImageSetBrowser", "OriginClaw");
    else if (feed == hDataCtrl->GetFeed(DB_FEED_CUSTOM))
        orig = GETL2S("Win_ImageSetBrowser", "OriginCustom");
    labFrameOriginV->setCaption(orig);
    labFrameOriginV->adjustSize();

    butChangeFrameID->setEnabled(feed == hDataCtrl->GetFeed(DB_FEED_CUSTOM));
    butDeleteFrame->setEnabled(feed == hDataCtrl->GetFeed(DB_FEED_CUSTOM));
    butBrowseFolder->setEnabled(feed == hDataCtrl->GetFeed(DB_FEED_CUSTOM));

    /*if( feed == hDataCtrl->GetFeed(DB_FEED_CUSTOM) ){
     butChangeFrameID->setVisible(1);
     butDeleteFrame->setVisible(1);
     butBrowseFolder->setVisible(1);
     butImageDetails->setY(130);
    }else{
     butChangeFrameID->setVisible(0);
     butDeleteFrame->setVisible(0);
     butBrowseFolder->setVisible(0);
     butImageDetails->setY(90);
    }*/

    char *buf = SHR::FormatTimeFromUnix(
            pickedtile->GetFile().hFeed->GetFileModTime(pickedtile->GetFile().strPath.c_str()));
    labFrameModDateV->setCaption(buf);
    labFrameModDateV->adjustSize();
    delete[] buf;

    for (int i = 0; i < 3; i++)
        vtGroups[i].clear();

    for (int i = 0; i < ts->GetSpritesCount(); i++) {
        cSprBankAssetIMG *t = ts->GetIMGByIterator(i);
        if (t->GetFile().hFeed == hDataCtrl->GetFeed(DB_FEED_REZ))
            vtGroups[0].push_back(t);
        else if (t->GetFile().hFeed == hDataCtrl->GetFeed(DB_FEED_DISC))
            vtGroups[1].push_back(t);
        else if (t->GetFile().hFeed == hDataCtrl->GetFeed(DB_FEED_CUSTOM))
            vtGroups[2].push_back(t);
    }

    bSingleGroup = 0;
    hSingleGroup = 0;
    if (vtGroups[0].size() > 0 && vtGroups[1].empty() && vtGroups[2].empty()) {
        bSingleGroup = 1;
        hSingleGroup = &vtGroups[0];
    } else if (vtGroups[0].empty() && vtGroups[1].size() > 0 && vtGroups[2].empty()) {
        bSingleGroup = 1;
        hSingleGroup = &vtGroups[1];
    } else if (vtGroups[0].empty() && vtGroups[1].empty() && vtGroups[2].size() > 0) {
        bSingleGroup = 1;
        hSingleGroup = &vtGroups[2];
    }

    conImageSets->setHeight((bank->GetAssetsCount() * CONST_IMGSETBROWSER_LISTSETH + 10));

    int tperrow = (myWin->getWidth() - (CONST_IMGSETBROWSER_ISLISTW + 15) - 16) / CONST_IMGSETBROWSER_FRAMEICOSIZE;
    if (bSingleGroup) {
        conFrames->setHeight((hSingleGroup->size() / tperrow + 1) * (CONST_IMGSETBROWSER_FRAMEICOSIZE + 20) + 20);
    } else {
        int len = 0;
        for (int i = 0; i < 3; i++)
            if (vtGroups[i].size() > 0) {
                len += (vtGroups[i].size() / tperrow + 1) * (CONST_IMGSETBROWSER_FRAMEICOSIZE + 20) + 20;
            }
        conFrames->setHeight(len);
    }
}

void winImageSetBrowser::Open() {
    myWin->setPosition((hge->System_GetState(HGE_SCREENWIDTH) - myWin->getWidth()) / 2,
                       (hge->System_GetState(HGE_SCREENHEIGHT) - myWin->getHeight()) / 2);
    myWin->setVisible(true);
    myWin->getParent()->moveToTop(myWin);
    Synchronize();
}

void winImageSetBrowser::Close() {
    myWin->setVisible(false);
}

void winImageSetBrowser::action(const ActionEvent &actionEvent) {
    if (actionEvent.getSource() == butBrowseFolder) {
        if (iSelectedImageSet) {
            cFile f = GV->editState->SprBank->GetAssetByIterator(iSelectedImageSet)->GetIMGByIterator(
                iSelectedFrame)->GetFile();
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
    } else if (actionEvent.getSource() == butAddFrames) {
        if (iSelectedImageSet) {
            GV->StateMgr->Push(new State::ImageImport(State::ImageImportObject,
                GV->editState->SprBank->GetAssetByIterator(iSelectedImageSet)));
        }
    } else if (actionEvent.getSource() == butImportImageSet) {
        GV->StateMgr->Push(new State::ImageImport(State::ImageImportObject, 0));
    } else if (actionEvent.getSource() == butDeleteFrame) {
        if (iSelectedImageSet && iSelectedFrame) {
            cSprBankAssetIMG* t = GV->editState->SprBank->GetAssetByIterator(iSelectedImageSet)->GetIMGByIterator(
                iSelectedFrame);
            if (iSelectedFrame == GV->editState->SprBank->GetAssetByIterator(iSelectedImageSet)->GetSpritesCount() - 1)
                iSelectedFrame--;

            std::string strPath = t->GetFile().hFeed->GetAbsoluteLocation() + "/" + t->GetFile().strPath;
            if (remove(strPath.c_str()) != 0) {
                GV->Console->Printf("~r~Failed to remove: ~w~%s", strPath.c_str());
            }
        }
    } else if (actionEvent.getSource() == butDeleteImageSet) {
        if (iSelectedImageSet) {
            cSprBankAsset* ts = GV->editState->SprBank->GetAssetByIterator(iSelectedImageSet);
            cSprBankAssetIMG* fimg = ts->GetIMGByIterator(0);

            std::string strPath = GV->editState->hDataCtrl->GetFeed(DB_FEED_CUSTOM)->GetAbsoluteLocation() + "/" +
                fimg->GetFile().strPath;
            strPath = strPath.substr(0, strPath.rfind('/'));
            SHR::RemoveDirR(strPath.c_str());
        }
    } else if (actionEvent.getSource() == butImageDetails) {
        if (iSelectedImageSet) {
            GV->StateMgr->Push(new State::ImageDetails(
                (cImage*)(GV->editState->SprBank->GetAssetByIterator(iSelectedImageSet)->GetIMGByIterator(
                    iSelectedFrame))));
        }
    }
}

void winImageSetBrowser::RenderFrameGroup(std::vector<cSprBankAssetIMG *> tiles, int x, int y) {
    cSprBankAssetIMG *selTile = (iSelectedFrame == -1 ? 0 : GV->editState->SprBank->GetAssetByIterator(
            iSelectedImageSet)->GetIMGByIterator(iSelectedFrame)),
            *hlTile = (iHighlightedF == -1 ? 0 : GV->editState->SprBank->GetAssetByIterator(
            iSelectedImageSet)->GetIMGByIterator(iHighlightedF));

    int tilePickX, tilePickY, tilePickW, tilePickH;
    hge->Gfx_GetClipping(&tilePickX, &tilePickY, &tilePickW, &tilePickH);
    int tilesPerRow = tilePickW / CONST_IMGSETBROWSER_FRAMEICOSIZE;
    for (size_t i = 0; i < tiles.size(); i++) {
        int gridX = (i % tilesPerRow),
                gridY = (i / tilesPerRow);
        int drawX = x + gridX * CONST_IMGSETBROWSER_FRAMEICOSIZE,
                drawY = y + gridY * (CONST_IMGSETBROWSER_FRAMEICOSIZE + 20);

        cSprBankAssetIMG *tile = tiles[i];

        DWORD colBorder = GV->colLineBright;
        if (tile == selTile) colBorder = GV->colActive;
        else if (tile == hlTile) colBorder = TILE_HIGHLIGHT_COLOR;

        int iPreviewDim = CONST_IMGSETBROWSER_FRAMEICOSIZE - 10;
        hge->Gfx_RenderLine(drawX + 5 - 2, drawY + 5 - 1, drawX + 5 + (iPreviewDim + 2), drawY + 5 - 1,
                            GV->colLineDark);
        hge->Gfx_RenderLine(drawX + 5 - 1, drawY + 5 + (iPreviewDim + 2), drawX + 5 + (iPreviewDim + 2),
                            drawY + 5 + (iPreviewDim + 2), GV->colLineDark);
        hge->Gfx_RenderLine(drawX + 5 - 1, drawY + 5 - 1, drawX + 5 - 1, drawY + 5 + (iPreviewDim + 2),
                            GV->colLineDark);
        hge->Gfx_RenderLine(drawX + 5 + (iPreviewDim + 2), drawY + 5 - 1, drawX + 5 + (iPreviewDim + 2),
                            drawY + 5 + (iPreviewDim + 2), GV->colLineDark);


        hge->Gfx_RenderLine(drawX + 5 - 1, drawY + 5, drawX + 5 + (iPreviewDim + 1), drawY + 5, colBorder);
        hge->Gfx_RenderLine(drawX + 5, drawY + 5 + (iPreviewDim + 1), drawX + 5 + (iPreviewDim + 1),
                            drawY + 5 + (iPreviewDim + 1), colBorder);
        hge->Gfx_RenderLine(drawX + 5, drawY + 5, drawX + 5, drawY + 5 + (iPreviewDim + 1), colBorder);
        hge->Gfx_RenderLine(drawX + 5 + (iPreviewDim + 1), drawY + 5, drawX + 5 + (iPreviewDim + 1),
                            drawY + 5 + (iPreviewDim + 1), colBorder);

        hgeSprite *spr = (tile->IsLoaded() ? tile->GetImage() : GV->sprSmiley);

        spr->SetColor(0xFFFFFFFF);

        int clipY = std::max(drawY + 5, tilePickY), clipH =
                std::min(drawY + 5 + iPreviewDim, tilePickY + tilePickH) - clipY;
        if (clipH > 0) {
            hge->Gfx_SetClipping(drawX + 5, clipY, iPreviewDim, clipH);
            GV->sprCheckboard->Render(drawX + 5, drawY + 5);
            hge->Gfx_SetClipping(tilePickX, tilePickY, tilePickW, tilePickH);
        }

        float fScale = 1;
        if (spr->GetWidth() > iPreviewDim || spr->GetHeight() > iPreviewDim) {
            float fScaleX = float(iPreviewDim) / spr->GetWidth(),
                    fScaleY = float(iPreviewDim) / spr->GetHeight();
            fScale = (fScaleX < fScaleY ? fScaleX : fScaleY);
        }
        float ofx, ofy;
        spr->GetHotSpot(&ofx, &ofy);
        ofx -= spr->GetWidth() / 2;
        ofy -= spr->GetHeight() / 2;
        ofx *= fScale;
        ofy *= fScale;
        spr->RenderEx(drawX + 5 + iPreviewDim / 2 + ofx, drawY + 5 + iPreviewDim / 2 + ofy, 0, fScale, fScale);

        GV->fntMyriad16->SetColor((colBorder != GV->colLineBright ? 0xFFFFFFFF : 0xFFe1e1e1));
        GV->fntMyriad16->printf(drawX + iPreviewDim / 2 + 5, drawY + 10 + iPreviewDim, HGETEXT_CENTER, "%d", 0,
                                tile->GetID());
    }
}

int winImageSetBrowser::MouseHandleGroup(std::vector<cSprBankAssetIMG *> tiles, int x, int y, int tilesPerRow) {
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);

    saFrames->setVerticalScrollAmount(saFrames->getVerticalScrollAmount() + 150 * (-hge->Input_GetMouseWheel()));

    if (mx > x && mx < x + tilesPerRow * CONST_IMGSETBROWSER_FRAMEICOSIZE - 5 && my > y + 5) {
        int dx = (mx - x) / CONST_IMGSETBROWSER_FRAMEICOSIZE,
                dy = (my - y) / (CONST_IMGSETBROWSER_FRAMEICOSIZE + 20);
        int imousetile = (dy * tilesPerRow) + dx;
        if (imousetile < GV->editState->SprBank->GetAssetByIterator(iSelectedImageSet)->GetSpritesCount() &&
            mx > (x + dx * CONST_IMGSETBROWSER_FRAMEICOSIZE + 5) &&
            mx < (x + (dx + 1) * CONST_IMGSETBROWSER_FRAMEICOSIZE - 10) &&
            my > (y + dy * (CONST_IMGSETBROWSER_FRAMEICOSIZE + 20) + 5))
            return imousetile;
    }
    return -1;
}
