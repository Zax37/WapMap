#ifndef H_C_DATACTRL
#define H_C_DATACTRL

#include "../shared/cPID.h"
#include "cFileSystem.h"
#include <hge.h>

namespace PID {
    class Palette;
}

#define DB_FEED_REZ    0
#define DB_FEED_DISC   1
#define DB_FEED_CUSTOM 2

class cAssetPackage;

class cParallelLoop;

class cAssetBank;

struct cFile {
    cFileFeed *hFeed;
    std::string strPath;
};

class cAsset {
private:
    cFile _hFile;
protected:
    std::string _strHash, _strName;
    cAssetPackage *hParent;
    bool _bLoaded;
    time_t _iLoadedDate;
    time_t _iLastDate;
    unsigned int _iFileSize;
    cAssetBank *_hBank;
    bool _bForceReload;
public:
    cAsset();

    virtual ~cAsset();

    const char *GetName() { return _strName.c_str(); };

    cFile GetFile() { return _hFile; };

    void SetFile(cFile nFile);

    unsigned int GetFileSize() { return _iFileSize; };

    bool IsLoaded() { return _bLoaded; };

    void SetFileModTime(time_t tTime) { _iLastDate = tTime; };

    std::string GetHash() { return _strHash; };

    bool IsActual() { return _bLoaded && (_iLoadedDate != 0 && _iLoadedDate == _iLastDate || _iLoadedDate == 0); };

    virtual void Load() {};

    virtual void Unload() {};

    virtual void Reload();

    void SetPackage(cAssetPackage *ptr) { hParent = ptr; };

    cAssetPackage *GetPackage() { return hParent; };

    virtual std::string GetMountPoint() { return ""; };

    cAssetBank *GetAssignedBank() { return _hBank; };

    void SetForceReload(bool b) { _bForceReload = b; };

    bool NeedReload() { return _bForceReload; };
};

class cDataController;

struct cDC_MountEntry {
    std::vector<cFile> vFiles;
    std::string strMountPoint;
    cAsset *hAsset;
};

class cAssetBank {
private:
    bool _bModFlag;
    int _iModNew, _iModChange, _iModDel;

    friend class cDataController;

public:
    virtual void DeleteAsset(cAsset *hAsset) {};

    virtual const std::string& GetFolderName() {
        static const std::string name;
        return name;
    };

    virtual void BatchProcessStart(cDataController *hDC) {};

    virtual void BatchProcessEnd(cDataController *hDC) {};

    virtual std::string GetMountPointForFile(std::string strFilePath, std::string strPrefix) { return ""; };

    virtual cAsset *AllocateAssetForMountPoint(cDataController *hDC, cDC_MountEntry mountEntry) { return NULL; };

    bool GetModFlag() { return _bModFlag; };

    int GetModCounterNew() { return _iModNew; };

    int GetModCounterChanged() { return _iModChange; };

    int GetModCounterDeleted() { return _iModDel; };
};

class cAssetPackage {
private:
    int iLoadPolicy;
    std::string strPrefix, strPath;
    //std::vector<cAsset*> * hvAssetHeap;
    cDataController *hParent;

    void Update(cAssetBank *hBank);

    cAssetPackage(cDataController *parent);

    ~cAssetPackage();

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

#define cDC_STANDARD 0
#define cDC_CUSTOM   1

class cDataController {
private:
    std::vector<cAssetPackage *> vhPackages;
    std::vector<cAsset *> vhAllAssets;
    std::string strClawDir, strFileDir, strFilename;
    cRezFeed *hREZ;
    cDiscFeed *hDisc, *hCustom;
    std::vector<cAssetBank *> vhBanks;
    PID::Palette *hPalette;
    cParallelLoop *hLooper;
    float fFeedRefreshTime;

    std::vector<cDC_MountEntry> vMountEntries;

    void _SortMountEntries();

    void _SortMountEntry(size_t id);

public:
    cDataController(std::string strCD, std::string strFD, std::string strFN);

    ~cDataController();

    void RelocateDocument(std::string strDocPath);

    cFileFeed *GetFeed(int i);

    int GetFeedPriority(cFileFeed *hFeed);

    cAssetPackage *CreatePackage(std::string strPath, std::string strPref, int iLoadPolicy = cDC_STANDARD);

    void DeletePackage(cAssetPackage *ptr);

    std::vector<cAssetPackage *> GetPackages() { return vhPackages; };

    void UpdateAllPackages();

    std::vector<cFile> GetFilesList(std::string strPath, int iLoadPolicy);

    void RegisterAssetBank(cAssetBank *hPtr);

    std::vector<cAssetBank *> GetBanks() { return vhBanks; };

    bool SetPalette(std::string strPath);

    PID::Palette *GetPalette() { return hPalette; };

    bool IsLoadableImage(cFile hFile, cImageInfo *inf = 0, cImageInfo::Level iInfoLevel = cImageInfo::Full);

    byte *GetImageRaw(cFile hFile, int *w, int *h);

    bool
    RenderImageRaw(byte *hData, HTEXTURE texDest, int iRx, int iRy, int iRowSpan, int w, int h, PID::Palette *pal = 0);

    bool RenderImage(cFile hFile, HTEXTURE texDest, int iRx, int iRy, int iRowSpan);

    void SetLooper(cParallelLoop *h) { hLooper = h; };

    cParallelLoop *GetLooper() { return hLooper; };

    void FixCustomDir();

    void OpenCodeEditor(std::string logicName, bool nonExisting = false);

    cFile AssignFileForLogic(std::string strLogicName);

    void Think();

    bool MountFile(std::string strMountPoint, cFile f);

    void UnmountFile(std::string strMountPoint, cFile hFile);

    cAssetPackage *GetAssetPackageByFile(cFile hFile);

    int GetMountPointID(std::string strMountPoint);

    cDC_MountEntry* GetMountEntry(const std::string& strMountPoint);

};

#endif // H_C_DATACTRL
