#include "imageImport.h"
#include "../globals.h"
#include "../cInterfaceSheet.h"
#include "editing_ww.h"
#include "../cTextureAtlas.h"
#include <algorithm>
#include "../cDataController.h"
#include "../databanks/tiles.h"
#include "../databanks/imageSets.h"
#include <direct.h>
#include "../langID.h"

extern HGE *hge;

namespace State {

    ImageImport::ImageImport(ImageImportType type, void *hDest) {
        hAtlaser = new cTextureAtlaser();
        iImportType = type;
        iListElementHeight = 90;
        iFileToDelete = -1;
        iBadRefCount = 0;
        labMountOut = labMountOutV = 0;

        gui = new gcn::Gui();
        gui->setGraphics(GV->gcnGraphics);
        gui->setInput(GV->gcnInput);

        int yOffset = (hDest == 0 ? 40 + (iImportType == ImageImportObject) * 20 : 0);

        myWin = new SHR::Win(&GV->gcnParts,
                             GETL2S("Win_ImageImport", iImportType == ImageImportTile ? "WinCaption" : "WinCaption2"));
        //int height = hge->System_GetState(HGE_SCREENHEIGHT)-200;
        myWin->setDimension(gcn::Rectangle(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 300,
                                           hge->System_GetState(HGE_SCREENHEIGHT) / 2 - 250 - yOffset / 2, 600,
                                           565 + yOffset));
        myWin->setMovable(0);
        myWin->setClose(1);
        myWin->addActionListener(this);

        hDestinationIS = 0;
        hDestinationTS = 0;

        if (iImportType == ImageImportTile)
            hDestinationTS = (cTileImageSet *) hDest;
        else if (iImportType == ImageImportObject)
            hDestinationIS = (cSprBankAsset *) hDest;

        if (!hDest) {
            labDestName = new SHR::Lab(
                    GETL2S("Win_ImageImport", iImportType == ImageImportTile ? "TilesetName" : "ImagesetName"));
            myWin->add(labDestName, 5, 10);
            int descw = labDestName->getWidth();

            if (iImportType == ImageImportObject) {
                labMountOut = new SHR::Lab(GETL2S("Win_ImageImport", "ResultName"));
                myWin->add(labMountOut, 5, 50);
                if (labMountOut->getWidth() > descw)
                    descw = labMountOut->getWidth();

                labMountOutV = new SHR::Lab("-");
                labMountOutV->setColor(0xFFFFFF);
                myWin->add(labMountOutV, 10 + descw, 50);
            }

            tfDestName = new SHR::TextField("");
            tfDestName->setDimension(gcn::Rectangle(0, 0, 250, 20));
            tfDestName->addActionListener(this);
            myWin->add(tfDestName, 10 + descw, 10);

            labWarnDestName = new SHR::Lab(GETL2S("Win_ImageImport",
                                                  iImportType == ImageImportTile ? "WarnSpecifyTileset"
                                                                                 : "WarnSpecifyImageset"));
            labWarnDestName->setColor(0xA10808);
            myWin->add(labWarnDestName, tfDestName->getX(), 30);
        } else {
            labDestName = 0;
            tfDestName = 0;
            labWarnDestName = 0;
        }

        labFiles = new SHR::Lab(GETL2S("Win_ImageImport", "FilesToImport"));
        labFiles->adjustSize();
        myWin->add(labFiles, 5, 13 + yOffset);

        labHint[0] = new SHR::Lab(GETL2S("Win_ImageImport", "SupportedTypes"));
        myWin->add(labHint[0], 5, 450 + 7 + yOffset);
        labHint[1] = new SHR::Lab(GETL2S("Win_ImageImport", "TruecolorNote"));
        myWin->add(labHint[1], 5, 450 + 22 + yOffset);

        conFiles = new SHR::Container();
        conFiles->setDimension(gcn::Rectangle(0, 0, 100, 100));
        conFiles->setOpaque(0);

        saFileList = new SHR::ScrollArea(conFiles, SHR::ScrollArea::SHOW_AUTO, SHR::ScrollArea::SHOW_ALWAYS);
        saFileList->setDimension(gcn::Rectangle(0, 0, 590, 400));
        saFileList->setOpaque(0);
        myWin->add(saFileList, 5, 35 + yOffset);

        conFiles->setWidth(saFileList->getChildrenArea().width);

        vpUnderList = new WIDG::Viewport(this, 0);
        conFiles->add(vpUnderList, 0, 0);

        vpOverList = new WIDG::Viewport(this, 1);
        myWin->add(vpOverList, 0, 0);

        /* n.labOutputFormat = new SHR::Lab("Output format:");
         conFiles->add(n.labOutputFormat);

         n.ddOutputFormat = new SHR::DropDown(this);
         n.ddOutputFormat->setDimension(gcn::Rectangle(0,0,75, 20));
         n.ddOutputFormat->adjustHeight();
         conFiles->add(n.ddOutputFormat);*/

        butSelectFiles = new SHR::But(GV->hGfxInterface, GETL2S("Win_ImageImport", "SelectFiles"));
        butSelectFiles->setDimension(gcn::Rectangle(0, 0, 150, 25));
        butSelectFiles->addActionListener(this);
        butSelectFiles->setIcon(GV->sprIcons16[Icon16_Open]);
        myWin->add(butSelectFiles, 425, 450 + yOffset);

        butAutoID = new SHR::But(GV->hGfxInterface, GETL2S("Win_ImageImport", "AutoID"));
        butAutoID->setDimension(gcn::Rectangle(0, 0, 150, 25));
        butAutoID->addActionListener(this);
        butAutoID->setIcon(GV->sprIcons16[Icon16_WriteID]);
        butAutoID->setEnabled(0);
        myWin->add(butAutoID, 425, 450 + 10 + 15 + yOffset);

        butSave = new SHR::But(GV->hGfxInterface, GETL2S("Win_ImageImport", "Save"));
        butSave->setDimension(gcn::Rectangle(0, 0, 150, 25));
        butSave->setEnabled(0);
        butSave->addActionListener(this);
        myWin->add(butSave, 300 - 155, myWin->getHeight() - 50);

        butCancel = new SHR::But(GV->hGfxInterface, GETL2S("Win_ImageImport", "Cancel"));
        butCancel->setDimension(gcn::Rectangle(0, 0, 150, 25));
        butCancel->addActionListener(this);
        myWin->add(butCancel, 300 + 5, myWin->getHeight() - 50);

        gui->setTop(myWin);
    }

    ImageImport::~ImageImport() {
        while (!vImages.empty())
            DeleteFile(0);

        if (tfDestName != 0) {
            delete tfDestName;
            delete labDestName;
            delete labWarnDestName;
        }

        if (labMountOut != 0) {
            delete labMountOut;
            delete labMountOutV;
        }

        delete hAtlaser;
        for (int i = 0; i < 2; i++)
            delete labHint[i];
        delete vpOverList;
        delete vpUnderList;
        delete conFiles;
        delete saFileList;
        delete butAutoID;
        delete butSave;
        delete butCancel;
        delete myWin;
        delete gui;
    }

    bool ImageImport::Opaque() {
        return 0;
    }

    void ImageImport::Init() {

    }

    void ImageImport::Destroy() {

    }

    bool ImageImport::Think() {
        if (iFileToDelete != -1) {
            DeleteFile(iFileToDelete);
            iFileToDelete = -1;
        }
        try {
            gui->logic();
        }
        catch (gcn::Exception &exc) {
            GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(),
                                exc.getFilename().c_str(), exc.getLine());
        }
        return 0;
    }

    bool ImageImport::Render() {
        try {
            gui->draw();
        }
        catch (gcn::Exception &exc) {
            GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(),
                                exc.getFilename().c_str(), exc.getLine());
        }
        return 0;
    }

    void ImageImport::GainFocus(int iReturnCode, bool bFlipped) {

    }

    void ImageImport::action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == myWin || actionEvent.getSource() == butCancel) {
            _popMe(0);
            return;
        } /*else if (actionEvent.getSource() == butSave) {
            if (vImages.size() > 0) {
                GV->editState->hDataCtrl->FixCustomDir();
                std::string strDestDir = GV->editState->hDataCtrl->GetFeed(DB_FEED_CUSTOM)->GetAbsoluteLocation();
                std::string strSubPath;
                if (iImportType == ImageImportTile) {
                    strDestDir += "/TILES/";
                    if (hDestinationTS != 0) {
                        strSubPath = hDestinationTS->GetName();
                        HANDLE hFind = INVALID_HANDLE_VALUE;
                        WIN32_FIND_DATA fdata;
                        hFind = FindFirstFile(strSubPath.c_str(), &fdata);

                        if (hFind == INVALID_HANDLE_VALUE)
                            mkdir(strSubPath.c_str());
                    } else {
                        std::string dirname = tfDestName->getText();
                        std::transform(dirname.begin(), dirname.end(), dirname.begin(), ::toupper);
                        strSubPath = dirname;
                        mkdir(strSubPath.c_str());
                    }
                } else {
                    strDestDir += "/IMAGES/";
                    if (hDestinationIS != 0) {
                        std::string path = hDestinationIS->GetIMGByIterator(0)->GetFile().strPath;
                        std::string lpath = path;
                        std::transform(lpath.begin(), lpath.end(), lpath.begin(), ::tolower);
                        size_t restpos = lpath.find("images/");
                        std::string subpath = path.substr(restpos + 7);
                        restpos = subpath.rfind('/');
                        strSubPath = subpath.substr(0, restpos);
                    } else {
                        std::string dirname = tfDestName->getText();
                        size_t undersc = dirname.find('_');
                        while (undersc != std::string::npos) {
                            dirname[undersc] = '/';
                            undersc = dirname.find('_');
                        }
                        strSubPath = dirname;
                    }

                    size_t slashdelim = strSubPath.find('/');
                    while (1) {
                        std::string strSubCheck = strSubPath.substr(0, slashdelim);
                        std::string strAbsPath = strDestDir + strSubCheck;

                        HANDLE hFind = INVALID_HANDLE_VALUE;
                        WIN32_FIND_DATA fdata;
                        hFind = FindFirstFile(strAbsPath.c_str(), &fdata);

                        if (hFind == INVALID_HANDLE_VALUE)
                            mkdir(strAbsPath.c_str());

                        if (slashdelim == std::string::npos)
                            break;
                        slashdelim = strSubPath.find('/', slashdelim + 1);
                    }
                }

                std::string strFinalPath = strDestDir + strSubPath;

                GV->Console->Printf("Saving imported images to: %s", strDestDir.c_str());

                strFinalPath += "/";
                for (size_t i = 0; i < vImages.size(); i++) {
                    int w = vImages[i].hSprite->GetWidth(),
                            h = vImages[i].hSprite->GetHeight();
                    char buffer[1024];
                    sprintf(buffer, "%s%d.pid", strFinalPath.c_str(), atoi(vImages[i].tfID->getText().c_str()));
                    PID::Image *img = new PID::Image(w, h);
                    for (int y = 0; y < h; y++)
                        for (int x = 0; x < w; x++)
                            img->SetColorIdAt(x, y, vImages[i].hByteData[y * w + x]);
                    img->Save(buffer);
                    delete img;
                }
            }
            _popMe(0);
            return;
        }*/ else if (tfDestName != 0 && actionEvent.getSource() == tfDestName) {
            if (tfDestName->getText().length() == 0) {
                labWarnDestName->setCaption(GETL2S("Win_ImageImport",
                                                   iImportType == ImageImportTile ? "WarnSpecifyTileset"
                                                                                  : "WarnSpecifyImageset"));
                labWarnDestName->adjustSize();
                labWarnDestName->setVisible(1);
                if (labMountOutV != 0) {
                    labMountOutV->setCaption("-");
                    labMountOutV->adjustSize();
                }
            } else if (GV->editState->hTileset->GetSet(tfDestName->getText().c_str(), 0) != 0) {
                labWarnDestName->setCaption(GETL2S("Win_ImageImport", "NameUsed"));
                labWarnDestName->adjustSize();
                labWarnDestName->setVisible(1);
                if (labMountOutV != 0) {
                    labMountOutV->setCaption("-");
                    labMountOutV->adjustSize();
                }
            } else {
                labWarnDestName->setVisible(0);
                if (labMountOutV != 0) {
                    std::string imgset = tfDestName->getText();

                    size_t slash = imgset.find_first_of("/\\");
                    while (slash != std::string::npos) {
                        imgset[slash] = '_';
                        slash = imgset.find_first_of("/\\");
                    }
                    std::transform(imgset.begin(), imgset.end(), imgset.begin(), ::toupper);
                    imgset = "CUSTOM_" + imgset;

                    labMountOutV->setCaption(imgset);
                    labMountOutV->adjustSize();
                }
            }
            return;
        } else if (actionEvent.getSource() == butAutoID) {
            int istartid = 0;
            if (iImportType == ImageImportTile)
                istartid = (hDestinationTS == 0 ? 1 : hDestinationTS->GetMaxTileID() + 1);
            else
                istartid = (hDestinationIS == 0 ? 1 : hDestinationIS->GetMaxIMGID() + 1);
            for (size_t i = 0; i < vImages.size(); i++) {
                char tmp[32];
                sprintf(tmp, "%d", istartid + i);
                vImages[i].tfID->setText(tmp, 1);
            }
            return;
        }
        for (size_t i = 0; i < vImages.size(); i++) {
            if (actionEvent.getSource() == vImages[i].butRemove) {
                iFileToDelete = i;
                return;
            } else if (actionEvent.getSource() == vImages[i].tfID) {
                int nID = atoi(vImages[i].tfID->getText().c_str());

                cAsset *asFnd = 0;
                if (iImportType == ImageImportTile)
                    asFnd = (hDestinationTS != 0 ? hDestinationTS->GetTile(nID) : 0);
                else
                    asFnd = (hDestinationIS != 0 ? hDestinationIS->GetIMGByID(nID) : 0);

                bool doubledid = 0;
                for (size_t x = 0; x < vImages.size(); x++) {
                    if (x == i) continue;
                    if (nID == atoi(vImages[x].tfID->getText().c_str())) {
                        doubledid = 1;
                        break;
                    }
                }
                if (vImages[i].tfID->getText().length() == 0) {
                    vImages[i].labWarnID->setVisible(1);
                    vImages[i].labWarnID->setCaption(GETL2S("Win_ImageImport", "WarnSpecifyID"));
                    vImages[i].labWarnID->setColor(0xa13a3a);
                    vImages[i].labWarnID->adjustSize();
                    if (!vImages[i].bBadFlag) {
                        vImages[i].bBadFlag = 1;
                        iBadRefCount++;
                    }
                } else if (doubledid) {
                    vImages[i].labWarnID->setVisible(1);
                    vImages[i].labWarnID->setCaption(GETL2S("Win_ImageImport", "WarnMultipleID"));
                    vImages[i].labWarnID->setColor(0xa13a3a);
                    vImages[i].labWarnID->adjustSize();
                    if (!vImages[i].bBadFlag) {
                        vImages[i].bBadFlag = 1;
                        iBadRefCount++;
                    }
                } else if (asFnd && asFnd->GetFile().hFeed == GV->editState->hDataCtrl->GetFeed(DB_FEED_CUSTOM)) {
                    vImages[i].labWarnID->setVisible(1);
                    vImages[i].labWarnID->setCaption(GETL2S("Win_ImageImport",
                                                            iImportType == ImageImportTile ? "WarnUsedByCustomTile"
                                                                                           : "WarnUsedByCustomImage"));
                    vImages[i].labWarnID->setColor(0xa13a3a);
                    vImages[i].labWarnID->adjustSize();
                    if (!vImages[i].bBadFlag) {
                        vImages[i].bBadFlag = 1;
                        iBadRefCount++;
                    }
                } else if (asFnd) {
                    vImages[i].labWarnID->setVisible(1);
                    vImages[i].labWarnID->setCaption(GETL2S("Win_ImageImport",
                                                            iImportType == ImageImportTile ? "InfoTileReplace"
                                                                                           : "InfoImageReplace"));
                    vImages[i].labWarnID->setColor(0xa1a1a1);
                    vImages[i].labWarnID->adjustSize();
                } else {
                    vImages[i].labWarnID->setVisible(0);
                    if (vImages[i].bBadFlag) {
                        vImages[i].bBadFlag = 0;
                        iBadRefCount--;
                    }
                }
                butSave->setEnabled(iBadRefCount == 0);
                if (labWarnDestName != 0 && labWarnDestName->isVisible())
                    butSave->setEnabled(0);
                return;
            }
        }
    }

    void ImageImport::Draw(int piCode) {
        int dx, dy;
        myWin->getAbsolutePosition(dx, dy);
        gcn::Rectangle rchild = saFileList->getChildrenArea();
        int drx, dry, drw, drh;
        saFileList->getAbsolutePosition(drx, dry);
        drx += rchild.x;
        dry += rchild.y;
        drw = rchild.width;
        drh = rchild.height;
        if (piCode == 0) {
            hge->Gfx_SetClipping(drx, dry, drw, drh);
            for (int y = 0; y < drh / 128 + 1; y++)
                for (int x = 0; x < drw / 128 + 1; x++)
                    GV->hGfxInterface->sprMainBackground->Render(drx + x * 128, dry + y * 128);

            if (vImages.empty()) {
                GV->fntMyriad16->SetColor(0xFFe1e1e1);
                GV->fntMyriad16->Render(drx + drw / 2, dry + drh / 2 - 7, HGETEXT_CENTER,
                                        GETL2S("Win_ImageImport", "NoFiles"), 0);
                GV->fntMyriad16->Render(drx + drw / 2, dry + drh / 2 + 7, HGETEXT_CENTER,
                                        GETL2S("Win_ImageImport", "DragDropHint"), 0);
            } else {
                int scroll = saFileList->getVerticalScrollAmount();
                int first = scroll / iListElementHeight,
                        amount = saFileList->getHeight() / iListElementHeight + 2;
                for (size_t i = first; i < vImages.size() && i < (first + amount); i++) {
                    int yPos = dry + i * iListElementHeight + 10 - scroll;
                    int yClip = std::max(dry, yPos + 20),
                            hClip = std::min(yPos + 20 + 64, dry + drh) - yClip;
                    if (hClip > 0) {
                        hge->Gfx_SetClipping(drx + 10, yClip, 64, hClip);
                        GV->sprCheckboard->Render(drx + 10, yPos + 20);
                        hge->Gfx_SetClipping(drx, dry, drw, drh);
                    }

                    hge->Gfx_RenderLine(drx + 10 - 2, yPos + 20 - 1, drx + 10 + 66, yPos + 20 - 1, GV->colLineDark);
                    hge->Gfx_RenderLine(drx + 10 - 2, yPos + 20 + 66, drx + 10 + 66, yPos + 20 + 66, GV->colLineDark);
                    hge->Gfx_RenderLine(drx + 10 - 1, yPos + 20 - 1, drx + 10 - 1, yPos + 20 + 65, GV->colLineDark);
                    hge->Gfx_RenderLine(drx + 10 + 66, yPos + 20 - 1, drx + 10 + 66, yPos + 20 + 65, GV->colLineDark);

                    hge->Gfx_RenderLine(drx + 10 - 1, yPos + 20, drx + 10 + 65, yPos + 20, GV->colLineBright);
                    hge->Gfx_RenderLine(drx + 10 - 1, yPos + 20 + 65, drx + 10 + 65, yPos + 20 + 65, GV->colLineBright);
                    hge->Gfx_RenderLine(drx + 10, yPos + 20, drx + 10, yPos + 20 + 64, GV->colLineBright);
                    hge->Gfx_RenderLine(drx + 10 + 65, yPos + 20, drx + 10 + 65, yPos + 20 + 64, GV->colLineBright);

                    float fScale = 1;
                    if (vImages[i].hSprite->GetWidth() > 64 || vImages[i].hSprite->GetHeight() > 64) {
                        float fScaleX = float(64) / vImages[i].hSprite->GetWidth(),
                                fScaleY = float(64) / vImages[i].hSprite->GetHeight();
                        fScale = (fScaleX < fScaleY ? fScaleX : fScaleY);
                    }
                    int movx = vImages[i].hSprite->GetWidth() / 2 * fScale,
                            movy = vImages[i].hSprite->GetHeight() / 2 * fScale;
                    vImages[i].hSprite->RenderEx(drx + 10 + 32 - movx, yPos + 20 + 32 - movy, 0, fScale, fScale);

                    hge->Gfx_RenderLine(drx, yPos + iListElementHeight - 2, drx + drw, yPos + iListElementHeight - 2,
                                        GV->colLineBright);
                    hge->Gfx_RenderLine(drx, yPos + iListElementHeight - 1, drx + drw, yPos + iListElementHeight - 1,
                                        GV->colLineDark);
                }
            }

            hge->Gfx_SetClipping();
        }
    }

    void ImageImport::FileDropped() {
        /*std::vector<char*> vszFiles = hge->System_GetDroppedFiles();
        GV->Console->Printf("~w~Importing ~y~%d ~w~images.", int(vszFiles.size()));
        for(size_t i=0;i<vszFiles.size();i++)
         AddFile(std::string(vszFiles[i]));*/
    }

    void ImageImport::FileAddError(std::string strFilePath, ImageImportError err) {
        GV->Console->Printf("~r~Import error: #%d", int(err));
        char tmp[256];
        sprintf(tmp, "err #%d", int(err));
        MessageBox(hge->System_GetState(HGE_HWND), tmp, "WapMap", MB_ICONERROR | MB_OK);
    }

    void ImageImport::AddFile(std::string strFilePath) {
        std::string stdPath = strFilePath;
        std::transform(stdPath.begin(), stdPath.end(), stdPath.begin(), ::tolower);
        size_t slashpos = stdPath.find('\\');
        while (slashpos != std::string::npos) {
            stdPath[slashpos] = '/';
            slashpos = stdPath.find('\\');
        }
        for (size_t i = 0; i < vImages.size(); i++) {
            if (stdPath.compare(vImages[i].strStdFilePath) == 0)
                return;
        }
        std::string strExtension;
        size_t dotpos = strFilePath.rfind('.');
        if (dotpos == std::string::npos) {
            FileAddError(strFilePath, ImgImp_InvalidFormat);
            return;
        }
        strExtension = strFilePath.substr(dotpos + 1);
        std::transform(strExtension.begin(), strExtension.end(), strExtension.begin(), ::tolower);

        byte *byteData = 0;
        int imgW = 0, imgH = 0;
        if (!strExtension.compare("pid") || !strExtension.compare("pcx")) {
            GV->Console->Printf("Importing: '~y~%s~w~' through DC.", strFilePath.c_str());
            cImageInfo imgInfo;
            cFile dcFile;
            dcFile.hFeed = &(cDiscFeed::GetHardDrive());
            dcFile.strPath = strFilePath;
            if (!GV->editState->hDataCtrl->IsLoadableImage(dcFile, &imgInfo)) {
                FileAddError(strFilePath, ImgImp_InvalidFormat);
                return;
            }
            if (iImportType == ImageImportTile && (imgInfo.iWidth != 64 || imgInfo.iHeight != 64)) {
                FileAddError(strFilePath, ImgImp_InvalidTileSize);
                return;
            }
            int w, h;
            PID::Palette* pal;
            byteData = GV->editState->hDataCtrl->GetImageRaw(dcFile, &w, &h, &pal);
        }/*else if( !strExtension.compare("bmp") || !strExtension.compare("jpg") || !strExtension.compare("jpeg") || !strExtension.compare("png") ){
		 GV->Console->Printf("Importing: '~y~%s~w~' through HGE.", strFilePath.c_str());
		 HTEXTURE tex = hge->Texture_Load(strFilePath.c_str());
		 if( !tex ){
		  FileAddError(strFilePath, ImgImp_InvalidFormat);
		  return;
		 }
		 imgW = hge->Texture_GetWidth(tex, 1);
		 imgH = hge->Texture_GetHeight(tex, 1);
		 if( iImportType == ImageImportTile && (imgW != 64 || imgH != 64) ){
		  FileAddError(strFilePath, ImgImp_InvalidTileSize);
		  hge->Texture_Free(tex);
		  return;
		 }
		 int srcW = hge->Texture_GetWidth(tex);
		 DWORD * srcimg = hge->Texture_Lock(tex);
		 //for(int i=0;i<256;i++)
		 // gdImageColorAllocate(img, GV->editState->hDataCtrl->GetPalette()->GetColorR(i),
		 //                           GV->editState->hDataCtrl->GetPalette()->GetColorG(i),
		 //                           GV->editState->hDataCtrl->GetPalette()->GetColorB(i));
		 byteData = new byte[imgW*imgH];
		 for(int y=0;y<imgH;y++)
		  for(int x=0;x<imgW;x++){
		   DWORD srccol = srcimg[y*srcW+x];
		   if( GETA(srccol) < 127 )
			byteData[y*imgW+x] = 0;
		   else
			byteData[y*imgW+x] = srccol; //gdImageColorClosest(img, GETR(srccol), GETG(srccol), GETB(srccol));
		  }
		 hge->Texture_Unlock(tex);
		 hge->Texture_Free(tex);
		}else if( !strExtension.compare("gif") ){
		 GV->Console->Printf("Importing: '~y~%s~w~' through GD.", strFilePath.c_str());
		 gdImagePtr img;
		 FILE * f = fopen(strFilePath.c_str(), "rb");
		 if( !f ){
		  FileAddError(strFilePath, ImgImp_ReadError);
		  return;
		 }
		 fclose(f);
		 img = gdImageCreateFromGif(f);
		 if( !img ){
		  FileAddError(strFilePath, ImgImp_InvalidFormat);
		  return;
		 }
		 if(  iImportType == ImageImportTile && (gdImageSX(img) != 64 || gdImageSY(img) != 64) ){
		  gdImageDestroy(img);
		  FileAddError(strFilePath, ImgImp_InvalidTileSize);
		  return;
		 }
		 if( img->trueColor ){

		 }else{
		  for(int y=0;y<gdImageSY(img);y++)
		   for(int x=0;x<gdImageSX(img);x++)
			byteData[y*gdImageSY(img)+x] = gdImageGetPixel(img, x, y);
		 }
		 gdImageDestroy(img);
		}*/else {
            FileAddError(strFilePath, ImgImp_InvalidFormat);
            return;
        }

        if (!byteData) return;

        stImageToImport n;

        n.hSprite = new hgeSprite(0, 0, 0, imgW, imgH);
        hAtlaser->AddSprite(n.hSprite);
        hAtlaser->Pack();

        n.hByteData = byteData;

        float x, y, w, h;
        n.hSprite->GetTextureRect(&x, &y, &w, &h);
        GV->editState->hDataCtrl->RenderImageRaw(byteData, n.hSprite->GetTexture(), x, y, 2048, imgW, imgH);

        n.strFilePath = strFilePath;
        n.strStdFilePath = stdPath;
        n.labFilePath = new SHR::Lab(strFilePath);
        n.labFilePath->setColor(0xFFFFFF);
        conFiles->add(n.labFilePath);
        n.butRemove = new SHR::But(GV->hGfxInterface, GV->sprIcons[Icon_Trash]);
        n.butRemove->setDimension(gcn::Rectangle(0, 0, 32, 32));
        n.butRemove->addActionListener(this);
        n.butRemove->setRenderBG(0);
        conFiles->add(n.butRemove);
        n.labID = new SHR::Lab(GETL2S("Win_ImageImport", iImportType == ImageImportTile ? "TileID" : "ImageID"));
        conFiles->add(n.labID);
        n.tfID = new SHR::TextField();
        n.tfID->SetNumerical(1, 0);
        n.tfID->addActionListener(this);
        n.tfID->setDimension(gcn::Rectangle(0, 0, 75, 20));
        conFiles->add(n.tfID);
        n.labWarnID = new SHR::Lab("");
        conFiles->add(n.labWarnID);
        n.bBadFlag = 0;

        int tileID = -1;
        size_t searchstart = strFilePath.find_last_of("/\\");
        if (searchstart = std::string::npos) searchstart = 0;
        size_t numstart = strFilePath.find_first_of("0123456789", searchstart);
        if (numstart != std::string::npos) {
            for (size_t i = numstart; i < strFilePath.length(); i++) {
                if (strFilePath[i] < 48 || strFilePath[i] > 57) break;
                int val = strFilePath[i] - 48;
                tileID = tileID * 10 + val;
            }
        }
        char temp[64];
        if (tileID > 0)
            sprintf(temp, "%d", tileID);
        else
            temp[0] = '\0';
        n.tfID->setText(temp);

        vImages.push_back(n);
        FixPosition(vImages.size() - 1);
        conFiles->setHeight(vImages.size() * iListElementHeight + 10);
        n.tfID->simulateAction();
        butAutoID->setEnabled(1);
    }

    void ImageImport::DeleteFile(size_t imgID) {
        if (vImages[imgID].bBadFlag)
            iBadRefCount--;
        conFiles->remove(vImages[imgID].butRemove);
        conFiles->remove(vImages[imgID].labID);
        conFiles->remove(vImages[imgID].tfID);
        conFiles->remove(vImages[imgID].labFilePath);
        conFiles->remove(vImages[imgID].labWarnID);
        delete vImages[imgID].butRemove;
        delete vImages[imgID].labID;
        delete vImages[imgID].tfID;
        delete vImages[imgID].labFilePath;
        delete vImages[imgID].labWarnID;

        hAtlaser->DeleteSprite(vImages[imgID].hSprite);
        delete vImages[imgID].hSprite;
        delete[] vImages[imgID].hByteData;

        vImages.erase(vImages.begin() + imgID);
        for (size_t i = imgID; i < vImages.size(); i++)
            FixPosition(i);
        conFiles->setHeight(vImages.size() * iListElementHeight + 10);
        butSave->setEnabled(vImages.size() > 0 && iBadRefCount == 0);
        if (labWarnDestName != 0 && labWarnDestName->isVisible())
            butSave->setEnabled(0);
        butAutoID->setEnabled(vImages.size() > 0);
    }

    void ImageImport::FixPosition(size_t imgID) {
        int yPos = imgID * iListElementHeight + 10;
        vImages[imgID].labFilePath->setPosition(5, yPos);
        vImages[imgID].labID->setPosition(5 + 64 + 10, yPos + 20 + 32 - 10);
        vImages[imgID].butRemove->setPosition(saFileList->getChildrenArea().width - 5 - 32,
                                              yPos + iListElementHeight / 2 - 16);
        vImages[imgID].tfID->setPosition(5 + 64 + 5 + vImages[imgID].labID->getWidth() + 5, yPos + 20 + 32 - 10);
        vImages[imgID].labWarnID->setPosition(5 + 64 + 5 + vImages[imgID].labID->getWidth() + 5, yPos + 20 + 32 + 10);
    }

    std::string ImageImport::getElementAt(int i) {
        if (i == 0)
            return "BMP";
        if (i == 1)
            return "PCX";
        if (i == 2)
            return "PID";
        return "";
    }

    int ImageImport::getNumberOfElements() {
        return 3;
    }

}
