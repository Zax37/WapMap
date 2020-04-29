#ifndef H_STATE_IMGIMPORT
#define H_STATE_IMGIMPORT

#include "../../shared/cStateMgr.h"
#include "../../shared/gcnWidgets/wWin.h"
#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wScrollArea.h"
#include "../../shared/gcnWidgets/wTextField.h"
#include "../../shared/gcnWidgets/wDropDown.h"
#include "../../shared/cWWD.h"
#include "guichan.hpp"
#include "../wViewport.h"

class cTextureAtlaser;

class cTileImageSet;

class cSprBankAsset;

namespace State {
    enum ImageImportError {
        ImgImp_InvalidFormat = 0,
        ImgImp_ReadError,
        ImgImp_InvalidTileSize
    };

    enum ImageImportType {
        ImageImportTile = 0,
        ImageImportObject
    };

    struct stImageToImport {
        std::string strFilePath, strStdFilePath;
        int iFileSize;
        SHR::Lab *labFilePath, *labID, *labWarnID;
        SHR::TextField *tfID;
        SHR::But *butRemove;
        byte *hByteData;
        hgeSprite *hSprite;
        bool bBadFlag;
    };

    class ImageImport : public SHR::cState, public gcn::ActionListener, public WIDG::VpCallback, public gcn::ListModel {
    public:
        ImageImport(ImageImportType type, void *hDest);

        ~ImageImport();

        virtual bool Opaque();

        virtual void Init();

        virtual void Destroy();

        virtual bool Think();

        virtual bool Render();

        virtual void GainFocus(int iReturnCode, bool bFlipped);

        void action(const gcn::ActionEvent &actionEvent);

        virtual void Draw(int piCode);

        virtual void FileDropped();

    private:
        gcn::Gui *gui;
        SHR::Win *myWin;
        SHR::ScrollArea *saFileList;
        SHR::Lab *labHint[2], *labFiles, *labMountOut, *labMountOutV;
        SHR::But *butSave, *butSelectFiles, *butCancel, *butAutoID;
        SHR::Container *conFiles;
        SHR::Lab *labOutputFormat;
        SHR::DropDown *ddOutputFormat;

        ImageImportType iImportType;
        WIDG::Viewport *vpUnderList, *vpOverList;
        std::vector<stImageToImport> vImages;
        int iListElementHeight;
        int iFileToDelete;
        cTextureAtlaser *hAtlaser;
        int iBadRefCount;

        void AddFile(std::string strFilePath);

        void DeleteFile(size_t imgID);

        void FixPosition(size_t imgID);

        void FileAddError(std::string strFile, ImageImportError err);

        SHR::Lab *labDestName, *labWarnDestName;
        SHR::TextField *tfDestName;

        //tile import
        cTileImageSet *hDestinationTS;
        cSprBankAsset *hDestinationIS;

        virtual std::string getElementAt(int i);

        virtual int getNumberOfElements();
    };
};

#endif
