#ifndef H_C_FILESYSTEM
#define H_C_FILESYSTEM

#include "../shared/cREZ.h"
#include <windows.h>

enum enFeedType {
    Feed_Unknown = 0,
    Feed_HardDrive,
    Feed_REZ,
    Feed_ZIP,
    Feed_Multiplexer
};

class cFileFeed {
protected:
    enFeedType myType;
    std::string strAbsoluteLocation;
public:
    cFileFeed() {};

    virtual ~cFileFeed() {};

    virtual time_t GetFileModTime(const char *path) { return 0; };

    virtual bool FileExists(const char *path) { return 0; };

    virtual unsigned char *GetFileContent(const char *path, unsigned int &oiDataLength, unsigned int iReadStart = 0,
                                          unsigned int iReadLen = 0) {
        oiDataLength = 0;
        return NULL;
    };

    virtual int GetFileSize(const char *path) { return 0; };

    virtual void SetFileContent(const char *path, const std::string& strContent) {};

    virtual void ReadingStart() {};

    virtual void ReadingStop() {};

    virtual void Reload() {};

    virtual bool DirectoryExists(const char *path) { return false; };

    virtual std::vector<std::string>
    GetDirectoryContents(const char *path, bool bRecursive) { return std::vector<std::string>(); };

    virtual std::vector<std::string>
    GetFilesInDirectory(std::string strDirectory) { return std::vector<std::string>(); };

    virtual std::vector<std::string>
    GetDirsInDirectory(std::string strDirectory) { return std::vector<std::string>(); };

    enFeedType GetType() { return myType; };

    time_t GetModificationTime() { return 0; };

    std::string GetAbsoluteLocation() { return strAbsoluteLocation; };

    virtual void Remount(std::string nPath) {};

    virtual void UpdateModificationFlag() {};

    virtual bool GetModificationFlag() { return 0; };

    virtual void ResetModificationFlag() {};

    virtual bool NeedFullReload() { return 1; };

    virtual std::vector<std::string> GetModifiedFiles() { return std::vector<std::string>(); };

    virtual std::vector<std::string> GetDeletedFiles() { return std::vector<std::string>(); };

    virtual std::vector<std::string> GetNewFiles() { return std::vector<std::string>(); };
};

class cDiscFeed : public cFileFeed {
private:
    bool bIsHDD;

    void _CreateWatchdog();

    void _DeleteWatchdog();

public:
    cDiscFeed(std::string strPath);

    ~cDiscFeed();

    virtual time_t GetFileModTime(const char *path);

    virtual bool FileExists(const char *path);

    virtual int GetFileSize(const char *path);

    virtual unsigned char *GetFileContent(const char *path, unsigned int &oiDataLength, unsigned int iReadStart = 0,
                                          unsigned int iReadLen = 0);

    virtual void SetFileContent(const char *path, const std::string& strContent);

    virtual bool DirectoryExists(const char *path);

    virtual std::vector<std::string> GetDirectoryContents(const char *path, bool bRecursive);

    virtual void Remount(std::string nPath);

    static cDiscFeed &GetHardDrive() {
        static cDiscFeed instance("");
        return instance;
    }

    virtual void UpdateModificationFlag();

    virtual bool GetModificationFlag();

    virtual void ResetModificationFlag();

    virtual bool NeedFullReload() { return 0; };

    virtual std::vector<std::string> GetModifiedFiles();

    virtual std::vector<std::string> GetDeletedFiles();

    virtual std::vector<std::string> GetNewFiles();

    //threaded
    HANDLE hDirToWatch;
    HANDLE hWatchThread;
    CRITICAL_SECTION myCS;
    std::vector<std::string> vstrNewFiles, vstrModFiles, vstrDelFiles;
};

class cRezFeed : public cFileFeed {
private:
    REZ::Parser *hREZ;
    time_t iModTime;
    bool iModificationFlag;
public:
    cRezFeed(const std::string& strPath);

    ~cRezFeed();

    virtual time_t GetFileModTime(const char *path);

    virtual bool FileExists(const char *path);

    virtual int GetFileSize(const char *path);

    virtual unsigned char *GetFileContent(const char *path, unsigned int &oiDataLength, unsigned int iReadStart = 0,
                                          unsigned int iReadLen = 0);

    virtual void ReadingStart();

    virtual void ReadingStop();

    virtual void Reload();

    virtual bool DirectoryExists(const char *path);

    virtual std::vector<std::string> GetDirectoryContents(const char *path, bool bRecursive);

    virtual void UpdateModificationFlag();

    virtual bool GetModificationFlag() { return iModificationFlag; };

    virtual void ResetModificationFlag() { iModificationFlag = 0; };
};

#endif // H_C_FILESYSTEM
