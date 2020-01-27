#include "commonFunc.h"

#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <direct.h>

#ifdef WAP_MAP
extern HGE *hge;
#endif

#ifdef WAP_GRAPHIC
extern HGE * hge;
#endif


#ifdef COMMON_SFS

hgeFont *SHR::LoadFontFromSFS(cSFS_Repository *phRepo, const char *pszDesc, const char *pszTex) {
    int desclen, texlen;
    char *tex = (char *) phRepo->GetFileAsRawData(pszTex, &texlen);
    char *desc = (char *) phRepo->GetFileAsRawData(pszDesc, &desclen);
    //printf("%s: tex: %d desc: %d\n", pszDesc, texlen, desclen);

    HTEXTURE htex = hge->Texture_Load(tex, texlen);

    hgeFont *fnt = new hgeFont(desc, desclen, htex, 1, 0, 0);
    delete[] desc;
    delete[] tex;
    return fnt;
}

#endif

unsigned int SHR::WinFileTimeToUnix(FILETIME ft) {
    ULARGE_INTEGER i;
    i.LowPart = ft.dwLowDateTime;
    i.HighPart = ft.dwHighDateTime;
    return (unsigned) (i.QuadPart / 10000000 - 11644473600LL);
}

char *SHR::GetDir(const char *filepath) {
    char *dir;
    const char *pch;
    pch = strchr(filepath, '/');
    int location = 0;
    while (pch != NULL) {
        location = pch - filepath;
        pch = strchr(pch + 1, '/');
    }
    pch = strchr(filepath, '\\');
    int location2 = 0;
    while (pch != NULL) {
        location2 = (pch - filepath + 1);
        pch = strchr(pch + 1, '\\');
    }
    int len = 0;
    if (location > location2)
        len = location;
    if (location2 > location)
        len = location2;
    dir = new char[len];
    strncpy(dir, filepath, len);
    dir[len - 1] = '\0';
    return dir;
}

char *SHR::GetFile(const char *filepath) {
    char *dir;
    const char *pch;
    pch = strchr(filepath, '/');
    int location = 0;
    while (pch != NULL) {
        location = pch - filepath;
        pch = strchr(pch + 1, '/');
    }
    pch = strchr(filepath, '\\');
    int location2 = 0;
    while (pch != NULL) {
        location2 = (pch - filepath + 1);
        pch = strchr(pch + 1, '\\');
    }
    int len = 0;
    if (location > location2)
        len = location;
    if (location2 > location)
        len = location2;
    dir = new char[strlen(filepath) - len + 1];
    for (int i = len; i < strlen(filepath); i++)
        dir[i - len] = filepath[i];
    dir[strlen(filepath) - len] = '\0';
    return dir;
}

char *SHR::Replace(const char *replacein, const char *searchfor, const char *replacefor) {
    if (strlen(replacein) == 0 || strlen(searchfor) == 0) {
        char *ret = new char[1];
        ret = '\0';
        return ret;
    }
    std::string a = replacein, b = searchfor, c = replacefor;
    int pos = -2;
    while (pos != -1) {
        if (pos == -2)
            pos = a.find(b);
        else {
            pos = a.find(b, pos + b.length());
        }
        if (pos != -1) {
            a.replace(pos, b.length(), c);
        }
    }
    char *ret = new char[a.length() + 1];
    strcpy(ret, a.c_str());
    ret[a.length()] = '\0';
    return ret;
}

int SHR::RoundTo2Power(int i) {
    for (int p = 1; p > 0; p += p) {
        if (i > p && i <= p * 2)
            return p * 2;
    }
}

char *SHR::GetExtension(const char *file) {
    const char *pch;
    pch = strrchr(file, '.');
    if (pch == NULL) return NULL;
    int last = pch - file + 1;

    if (strlen(file) - last <= 0)
        return NULL;

    char *ext = new char[strlen(file) - last + 1];
    for (int i = last; i < strlen(file) + 1; i++) {
        ext[i - last] = file[i];
    }
    return ext;
}

char *SHR::ToLower(const char *str) {
    char *ret = new char[strlen(str) + 1];
    strcpy(ret, str);
    for (int i = 0; i < strlen(ret); i++)
        ret[i] = tolower(ret[i]);
    return ret;
}

char *SHR::GetLine(const char *source, char *ptr) {
    int i = 0;
    if (!source[i]) return 0;
    while (source[i] && source[i] != '\n' && source[i] != '\r') {
        ptr[i] = source[i];
        i++;
    }
    ptr[i] = 0;
    while (source[i] && (source[i] == '\n' || source[i] == '\r')) i++;
    return (char *) source + i;
}

char *SHR::ToUpper(const char *str) {
    char *ret = new char[strlen(str) + 1];
    strcpy(ret, str);
    for (int i = 0; i < strlen(ret); i++)
        ret[i] = toupper(ret[i]);
    return ret;
}

char *SHR::GetFileWithoutExt(const char *file) {
    const char *pch;

    pch = strchr(file, '.');
    if (pch == NULL) {
        char *ret = new char[strlen(file) + 1];
        strcpy(ret, file);
        return ret;
    }

    int last = 0;

    while (pch != NULL) {
        last = pch - file + 1;
        pch = strchr(pch + 1, '.');
    }

    if (strlen(file) - last <= 0) {
        char *ret = new char[strlen(file) + 1];
        strcpy(ret, file);
        return ret;
    }
    last;
    char *ret = new char[last];
    for (int i = 0; i < last; i++) {
        ret[i] = file[i];
        if (ret[i] == '.')
            ret[i] = '\0';

        if (ret[i] == '\0')
            return ret;
    }
}

char *SHR::GetClipboard() {
#ifdef WAP_MAP
    if (!OpenClipboard(hge->System_GetState(HGE_HWND))) {
#else
        if (!OpenClipboard(NULL)) {
#endif
        return 0;
    }

    HANDLE text = GetClipboardData(CF_TEXT);
    if (text == NULL) return NULL;

    char *buffer = (char *) GlobalLock(text);
    char *ret = new char[strlen(buffer) + 1];
    strcpy(ret, buffer);
    GlobalUnlock(text);
    CloseClipboard();
    return ret;
}

int SHR::SetClipboard(const char *str) {
#ifdef WAP_MAP
    if (!OpenClipboard(hge->System_GetState(HGE_HWND))) {
#else
        if (!OpenClipboard(NULL)) {
#endif
        return 0;
    }
    if (!EmptyClipboard()) {
        return 0;
    }

    HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, strlen(str) + 1);
    strcpy((char *) hGlob, str);

    if (::SetClipboardData(CF_TEXT, hGlob) == NULL) {
        CloseClipboard();
        GlobalFree(hGlob);
        return -1;
    }
    CloseClipboard();
    return 1;
}

bool SHR::CompareStrings(const char *str1, const char *str2) {
    for (int i = 0; i < strlen(str1) && i < strlen(str2); i++) {
        if (tolower(str1[i]) < tolower(str2[i])) return 1;
        if (tolower(str1[i]) > tolower(str2[i])) return 0;
    }
    return 0;
}

bool SHR::CompareStringsIntSupport(const char *str1, const char *str2) {
    for (int i = 0; i < strlen(str1) && i < strlen(str2); i++) {
        bool num1 = (str1[i] >= 48 && str1[i] <= 57), num2 = (str2[i] >= 48 && str2[i] <= 57);
        if (num1 == 1 && num2 == 0)
            return 0;
        if (num2 == 1 && num1 == 0)
            return 1;
        if (num1 == 0 && num2 == 0) {
            if (tolower(str1[i]) < tolower(str2[i])) return 1;
            if (tolower(str1[i]) > tolower(str2[i])) return 0;
            if (tolower(str1[i]) == tolower(str2[i])) continue;
        }
        int numend1 = i, numend2 = i;
        for (int y = i + 1; y < strlen(str1); y++)
            if (!(str1[y] >= 48 && str1[y] <= 57)) {
                numend1 = y - 1;
                break;
            }
        for (int y = i + 1; y < strlen(str2); y++)
            if (!(str2[y] >= 48 && str2[y] <= 57)) {
                numend2 = y - 1;
                break;
            }
        int i1, i2;
        if (numend1 == i) i1 = str1[i] - 48;
        else {
            i1 = str1[numend1] - 48;
            for (int y = numend1 - 1; y >= i; y--) {
                if (str1[y] != 48)
                    i1 += (str1[y] - 48) * pow(10, int(numend1 - y));
            }
        }
        if (numend2 == i) i2 = str2[i] - 48;
        else {
            i2 = str2[numend2] - 48;
            for (int y = numend2 - 1; y >= i; y--) {
                if (str2[y] != 48) {
                    //printf("adding to %d %d\n", i2, (str2[y]-48)*pow(10, int(numend2-y)));
                    i2 += (str2[y] - 48) * pow(10, int(numend2 - y));
                    //printf("char %c adding %d (10 to power %d (%d-%d))\n", str2[y], pow(10, numend2-y+1), numend2-y+1, numend2, y);
                    //printf("power %d\n", int(numend2-y));
                }
            }
        }
        return i1 < i2;
    }
    return 0;
}

#ifdef WAP_MAP

void SHR::SetQuad(hgeQuad *q, DWORD col, float x1, float y1, float x2, float y2, float z) {
    q->v[0].col = q->v[1].col = q->v[2].col = q->v[3].col = col;
    q->v[0].z = q->v[1].z = q->v[2].z = q->v[3].z = z;

    q->v[0].x = x1;
    q->v[0].y = y1;
    q->v[1].x = x2;
    q->v[1].y = y1;
    q->v[2].x = x2;
    q->v[2].y = y2;
    q->v[3].x = x1;
    q->v[3].y = y2;
}

#endif

char *SHR::FormatSize(DWORD bytes) {
    char *ret = new char[64];
    if (bytes < 1024)
        sprintf(ret, "%db", bytes);
    else if (bytes < 1024 * 1024)
        sprintf(ret, "%.2fkB", bytes / 1024.0F);
    else if (bytes < 1024 * 1024 * 1024)
        sprintf(ret, "%.2fMB", bytes / float(1024 * 1024));
    else
        sprintf(ret, "%.2fGB", bytes / float(1024 * 1024 * 1024));
    return ret;
}

std::vector<SHR::DisplayMode> SHR::GetDisplayModes() {
    std::vector<DisplayMode> vec;
    DEVMODE dm;
    int i = 0;
    while (EnumDisplaySettings(NULL, i, &dm)) {
        DisplayMode n;
        n.iWidth = dm.dmPelsWidth;
        n.iHeight = dm.dmPelsHeight;
        n.iDepth = dm.dmBitsPerPel;
        n.iFrequency = dm.dmDisplayFrequency;
        vec.push_back(n);
        i++;
    }
    return vec;
}

char *SHR::FormatTimeFromUnix(time_t tim) {
    struct tm *tm;
    tm = localtime(&tim);
    char buf[64];
    strftime(buf, sizeof(buf), "%H:%M:%S %d.%m.%Y", tm);
    char *ret = new char[strlen(buf) + 1];
    strcpy(ret, buf);
    return ret;
}

void SHR::CopyDirR(const char *szSource, const char *szDest) {
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fdata;
    hFind = FindFirstFile(szSource, &fdata);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }
    std::string strSourcePath = szSource;
    do {
        if (fdata.cFileName[0] == '.' &&
            (fdata.cFileName[1] == '\0' || fdata.cFileName[1] == '.' && fdata.cFileName[2] == '\0'))
            continue;
        std::string strAbsEntity = strSourcePath + "/" + fdata.cFileName,
                strAbsDest = std::string(szDest) + "/" + fdata.cFileName;
        if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            mkdir(strAbsDest.c_str());
            CopyDirR(strAbsEntity.c_str(), strAbsDest.c_str());
        } else {
            std::ifstream src(strAbsEntity.c_str(), std::ios::binary);
            std::ofstream dst(strAbsDest.c_str(), std::ios::binary);
            dst << src.rdbuf();
        }
    } while (FindNextFile(hFind, &fdata) != 0);
}

int SHR::RemoveDirR(const char *dirPath) {
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fdata;
    hFind = FindFirstFile(dirPath, &fdata);
    size_t path_len = strlen(dirPath);
    int r = -1;

    if (hFind != INVALID_HANDLE_VALUE) {
        struct dirent *p;
        r = 0;
        do {
            int r2 = -1;
            char *buf;
            size_t len;
            if (!strcmp(fdata.cFileName, ".") || !strcmp(fdata.cFileName, "..")) {
                continue;
            }
            len = path_len + strlen(fdata.cFileName) + 2;
            buf = (char *) malloc(len);
            if (buf) {
                snprintf(buf, len, "%s/%s", dirPath, fdata.cFileName);
                if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    r2 = RemoveDirR(buf);
                } else {
                    r2 = unlink(buf);
                }
                free(buf);
            }
            r = r2;
        } while (FindNextFile(hFind, &fdata) != 0);
    }
    if (!r) {
        r = rmdir(dirPath);
    }
    return r;
}
