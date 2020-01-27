#ifndef H_C_NATIVECTRL
#define H_C_NATIVECTRL

#include <string>
#include <windows.h>

/**
 Class used to connect WM with installed game. It's responsible for version query, recognition and features access.
**/

class cNativeController {
private:
    std::string strNativePath;
    std::string strExeHash;
    std::string strVersion;
    bool bValid;
    int iVersion[3];
    float fExeCheckTimer;
    FILETIME ftExeModification;
    bool bGod, bArmor, bDebug;
    int iDisplayW, iDisplayH;

public:
    cNativeController();

    cNativeController(std::string strPath);

    ~cNativeController();

    bool IsValid();

    bool SetPath(std::string strPath);

    std::string GetPath() { return strNativePath; };

    /* Returns -1 if not valid. Param `i` is one of MAJOR, MINOR, BUILD enums. */
    int GetVersion(int iPart);

    std::string GetVersionStr() { return strVersion; };

    bool ExecutableChanged();

    bool IsCrazyHookAvailable();

    void SetDisplayResolution(int w, int h);

    void SetGodMode(bool b);

    void SetArmorMode(bool b);

    void SetDebugInfo(bool b);

    void RunGame(std::string strMap, int iPosX = -1, int iPosY = -1);

    void GetDisplayResolution(int *w, int *h);

    bool IsGodModeOn() { return bGod; };

    bool IsArmorModeOn() { return bArmor; };

    bool IsDebugInfoOn() { return bDebug; };

    enum {
        MAJOR = 0,
        MINOR,
        BUILD
    };
};

#endif // H_C_NATIVECTRL
