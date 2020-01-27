#ifndef H_C_MRULIST
#define H_C_MRULIST

#include <string>

class cMruList {
private:
    std::string sRecentlyUsed[10];
    bool bValid;
    int iFilesCount;
public:
    cMruList();

    ~cMruList();

    void Reload();

    void PushNewFile(const char *path, bool bSaveToRegistry);

    void SaveToRegistry(const char *szNew);

    const char *GetRecentlyUsedFile(int i);

    bool IsValid() { return bValid; };

    int GetFilesCount() { return iFilesCount; };
};

#endif
