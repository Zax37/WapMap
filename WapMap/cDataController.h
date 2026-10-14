#ifndef H_C_DATACTRL
#define H_C_DATACTRL

#include "../shared/cWWD.h"
#include "../shared/cPID.h"
#include "cFileSystem.h"
#include <hge.h>
#include <hgeSprite.h>
#include "guichan/listmodel.hpp"

namespace PID {
    class Palette;
}

#define DB_FEED_REZ    0
#define DB_FEED_DISC   1
#define DB_FEED_CUSTOM 2

#define cDC_STANDARD 0
#define cDC_CUSTOM   1

class cAsset;

class cAssetPackage;

class cParallelLoop;

template<typename T>
class cAssetBank;

class cDataController;

struct cFile {
    cFileFeed *hFeed;
    std::string strPath;
};

class cAsset {
private:
    cFile _hFile;
protected:
    std::string _strHash, _strName;
    cAssetPackage *hParent = 0;
    bool _bLoaded = false;
    time_t _iLoadedDate = 0;
    time_t _iLastDate = 0;
    unsigned int _iFileSize = 0;
    cAssetBank<cAsset> *_hBank = 0;
    bool _bForceReload = false;

    friend class cBankPalettes;
public:
    virtual ~cAsset() {}

    const std::string& GetName() { return _strName; }

    cFile& GetFile() { return _hFile; }

    void SetFile(cFile nFile);

    unsigned int GetFileSize() { return _iFileSize; }

    bool IsLoaded() { return _bLoaded; }

    void SetFileModTime(time_t tTime) { _iLastDate = tTime; }

    std::string GetHash() { return _strHash; }

    bool IsActual() { return _bLoaded && (_iLoadedDate != 0 && _iLoadedDate == _iLastDate || _iLoadedDate == 0); }

    virtual void Load() = 0;

    virtual void Unload() = 0;

    void Reload();

    void SetPackage(cAssetPackage *ptr) { hParent = ptr; }

    cAssetPackage *GetPackage() { return hParent; }

    cAssetBank<cAsset> *GetAssignedBank() { return _hBank; }

    void SetForceReload(bool b) { _bForceReload = b; }

    bool NeedReload() { return _bForceReload; }
};

struct cDC_MountEntry {
    std::vector<cFile> vFiles;
    std::string strMountPoint;
    cAsset *hAsset;
};

template<typename T>
class cAssetBank : public gcn::ListModel {
private:
    bool _bModFlag;
    int _iModNew, _iModChange, _iModDel;

    friend class cDataController;

protected:
    cDataController *hDC;
    std::vector<T *> m_vAssets;

public:
    cAssetBank(cDataController * hDC) : _bModFlag(false), _iModNew(0), _iModChange(0), _iModDel(0), hDC(hDC) {}

    ~cAssetBank() override {
        for (int i = 0; i < m_vAssets.size(); i++) {
            delete m_vAssets[i];
        }
    }

    T *GetAssetByIterator(int iIT) { return m_vAssets[iIT]; }

    virtual void DeleteAsset(T *hAsset) = 0;

    virtual const std::string& GetFolderName() = 0;

    virtual void BatchProcessStart() {};

    virtual void BatchProcessEnd() {};

    virtual std::string GetMountPointForFile(std::string strFilePath, std::string strPrefix) = 0;

    virtual T *AllocateAssetForMountPoint(cDC_MountEntry mountEntry) = 0;

    bool GetModFlag() { return _bModFlag; };

    int GetModCounterNew() { return _iModNew; };

    int GetModCounterChanged() { return _iModChange; };

    int GetModCounterDeleted() { return _iModDel; };

    // inherited from gcn::ListModel:

    std::string getElementAt(int i) override { return (m_vAssets.size() > 0 ? m_vAssets[i]->GetName() : ""); }

    int getNumberOfElements() override { return m_vAssets.size(); }
};

class cAssetPackage {
private:
    int iLoadPolicy = cDC_STANDARD;
    std::string strPrefix, strPath;
    //std::vector<cAsset*> * hvAssetHeap;
    cDataController *hParent;

    void Update(cAssetBank<cAsset> *hBank);

    cAssetPackage(cDataController *parent) : hParent(parent) {}

    friend class cDataController;

public:
    void RegisterAsset(cAsset *hPtr);

    void UnregisterAsset(cAsset *hPtr);

    std::string GetPrefix() { return strPrefix; };

    std::string GetPath() { return strPath; };

    cDataController *GetParent() { return hParent; };

    int GetLoadPolicy() { return iLoadPolicy; };
};

struct cImageInfo {
    enum Format {
        PID = 0,
        BMP,
        PCX
    };
    enum Level {
        Dimensions,
        Full
    };

    Format iType;

    int iWidth;
    int iHeight;

    int iOffsetX, iOffsetY;
    int iUser1, iUser2;
    PID::FLAGS iFlags;
};

class cImage {
protected:
    hgeSprite *imgSprite;
    cImageInfo imgInfo;
public:
    hgeSprite *GetImage() { return imgSprite; };

    cImageInfo GetImageInfo() { return imgInfo; };
};

class cDataController {
private:
    std::vector<cAssetPackage *> vhPackages;
    std::vector<cAsset *> vhAllAssets;
    std::string strGameDir, strFileDir, strFilename;
    cRezFeed *hREZ;
    cDiscFeed *hDisc, *hCustom;
    std::vector<cAssetBank<cAsset> *> vhBanks;
    PID::Palette *hPalette;
    cParallelLoop *hLooper;
    float fFeedRefreshTime;
    WWD::Parser* hParser;

    std::vector<cDC_MountEntry> vMountEntries;

    void _SortMountEntries();

    void _SortMountEntry(size_t id);

public:
    cDataController(WWD::Parser* hParser, std::string strGD, std::string strFD, std::string strFN);

    ~cDataController();

    void RelocateDocument(std::string strDocPath);

    cFileFeed *GetFeed(int i);

    int GetFeedPriority(cFileFeed *hFeed);

    cAssetPackage *CreatePackage(std::string strPath, std::string strPref, int iLoadPolicy = cDC_STANDARD);

    void DeletePackage(cAssetPackage *ptr);

    std::vector<cAssetPackage *> GetPackages() { return vhPackages; };

    void UpdateAllPackages();

    std::vector<cFile> GetFilesList(std::string strPath, int iLoadPolicy);

    template<typename T>
    void RegisterAssetBank(cAssetBank<T> *hPtr) {
        vhBanks.push_back((cAssetBank<cAsset>*) hPtr);
    }

    WWD::GAME GetGame() { return hParser->GetGame(); }
    int GetBaseLevel() { return hParser->GetBaseLevel(); }
    int GetTileAttribsCount() { return hParser->GetTileAttribsCount(); }

    std::vector<cAssetBank<cAsset> *> GetBanks() { return vhBanks; }

    void SetPalette(PID::Palette * palette) { hPalette = palette; }
    PID::Palette *GetPalette() { return hPalette; }

    bool IsLoadableImage(cFile hFile, cImageInfo *inf = 0, cImageInfo::Level iInfoLevel = cImageInfo::Full);

    byte *GetImageRaw(cFile hFile, int *w, int *h, PID::Palette** pal);

    bool RenderImageRaw(byte *hData, HTEXTURE texDest, int iRx, int iRy, int iRowSpan, int w, int h, PID::Palette *pal = 0);

    bool RenderImage(cFile hFile, HTEXTURE texDest, int iRx, int iRy, int iRowSpan);

    void SetLooper(cParallelLoop *h) { hLooper = h; };

    cParallelLoop *GetLooper() { return hLooper; };

    void FixCustomDir();

    void FixTileProperties(std::vector<int>& idsToAdd);

    void OpenCodeEditor(std::string logicName, bool nonExisting = false);

    void OpenCodeEditor(class cCustomLogic *logic);

    cFile AssignFileForLogic(const std::string& strLogicName);

    void Think();

    bool MountFile(std::string strMountPoint, cFile f);

    void UnmountFile(std::string strMountPoint, cFile hFile);

    cAssetPackage *GetAssetPackageByFile(const cFile& hFile);

    int GetMountPointID(std::string strMountPoint);

    cDC_MountEntry* GetMountEntry(const std::string& strMountPoint);

};

#endif // H_C_DATACTRL
