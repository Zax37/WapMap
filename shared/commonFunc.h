#ifndef H_SHARED_COMMONFUNC
#define H_SHARED_COMMONFUNC

#ifdef WAP_MAP
#define COMMON_SFS
#include "hgefont.h"
#endif

#include <windows.h>

#include <vector>

#ifdef COMMON_SFS
#include "cSFS.h"
#endif

#ifndef DWORD
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
#endif

namespace SHR {
    struct DisplayMode {
        int iWidth, iHeight, iDepth, iFrequency;
    };
#ifdef COMMON_SFS

    hgeFont *LoadFontFromSFS(cSFS_Repository *phRepo, const char *pszDesc, const char *pszTex);

#endif

    char *FormatSize(DWORD bytes);

    char *GetDir(const char *filepath);

    char *GetFile(const char *filepath);

    char *GetExtension(const char *file);

    char *GetFileWithoutExt(const char *file);

    char *ToLower(const char *str);

    char *ToUpper(const char *str);

    char *Replace(const char *replacein, const char *searchfor, const char *replacefor);

    int RoundTo2Power(int i);

    char *GetClipboard();

    int SetClipboard(const char *str);

    bool CompareStrings(const char *str1, const char *str2);

    bool CompareStringsIntSupport(const char *str1, const char *str2);

#ifdef WAP_MAP
    void SetQuad(hgeQuad *q, DWORD col, float x1, float y1, float x2, float y2, float z = 1.0f);
#endif

    std::vector<DisplayMode> GetDisplayModes();

    char *GetLine(const char *source, char *ptr);

    unsigned int WinFileTimeToUnix(FILETIME ft);

    char *FormatTimeFromUnix(time_t tim);

    void CopyDirR(const char *szSource, const char *szDest);

    int RemoveDirR(const char *dirPath);
};

#endif
