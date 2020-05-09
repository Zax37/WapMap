#include "cMRUList.h"
#include "globals.h"
#include "states/editing_ww.h"
#include <Shlobj.h>

inline LPITEMIDLIST PIDLGetNextItem(LPITEMIDLIST pidl) {
    return pidl ? reinterpret_cast<LPITEMIDLIST>(reinterpret_cast<BYTE *>(pidl) + pidl->mkid.cb) : NULL;
}

cMruList::cMruList() {
    Reload();
}

cMruList::~cMruList() {

}

void cMruList::Reload() {
    GV->Console->Print("~w~Reloading MRU list...");
    HKEY key;
    if (GV->iOS == OS_VISTA || GV->iOS == OS_7) {
        RegOpenKeyEx(HKEY_CURRENT_USER,
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\OpenSavePidlMRU\\WWD", 0,
                     KEY_QUERY_VALUE, &key);
        char mrulist[100];
        DWORD size = 100;
        if (RegQueryValueEx(key, "MRUListEx", NULL, NULL, (LPBYTE) (&mrulist), &size) != ERROR_SUCCESS) {
            GV->Console->Print("~r~Unable to open PidlMRU list.");
            bValid = 0;
            iFilesCount = 0;
            return;
        } else {
            iFilesCount = 0;
            bool zeroexist = 0;
            for (int i = 0; (i < (size / 4) - 1 && i < 10); i++) {
                char temp[1024];
                size = 1024;
                char buf[8];
                sprintf(buf, "%d", mrulist[i * 4]);
                if (mrulist[i * 4] == 0) {
                    if (zeroexist) continue;
                    else zeroexist = 1;
                }
                if (RegQueryValueEx(key, buf, NULL, NULL, (LPBYTE) (&temp), &size) != ERROR_SUCCESS) {
                    GV->Console->Printf("~r~Error querying MRU position %d [#%d]", mrulist[i * 4], i);
                    sRecentlyUsed[i] = "";
                    continue;
                }
                char tmp[MAX_PATH];
                if (SHGetPathFromIDList((ITEMIDLIST *) &temp, (CHAR *) &tmp) == TRUE) {
                    iFilesCount++;
                    sRecentlyUsed[i] = tmp;
                } else {
                    GV->Console->Printf("~r~Error querying MRU position %d [#%d]", mrulist[i * 4], i);
                    continue;
                    sRecentlyUsed[i] = "";
                }
            }
            RegCloseKey(key);
            bValid = 1;
            for (int i = 0; i < 10; i++)
                if (sRecentlyUsed[i].length() == 0) {
                    for (int z = i + 1; z < 10; z++) {
                        sRecentlyUsed[z - 1] = sRecentlyUsed[z];
                    }
                }
            GV->Console->Printf("~g~WWD list loaded from registry [pidl] (%d positions).", iFilesCount);
            return;
        }
    } else if (RegOpenKeyEx(HKEY_CURRENT_USER,
                            "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\OpenSaveMRU\\WWD", 0,
                            KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
        GV->Console->Print("~r~Error opening registry (WWD list).");
        bValid = 0;
        iFilesCount = 0;
        return;
    } else {
        char mrulist[11];
        ZeroMemory(&mrulist, 11);
        DWORD size = 11;

        RegQueryValueEx(key, "MRUList", NULL, NULL, (LPBYTE) (&mrulist), &size);
        iFilesCount = strlen(mrulist);

        for (int i = 0; i < iFilesCount; ++i) {
            char keyname[2];
            keyname[0] = mrulist[i];
            keyname[1] = '\0';

            char tmp[MAX_PATH + 1];
            size = MAX_PATH + 1;
            RegQueryValueEx(key, keyname, NULL, NULL, (LPBYTE) (&tmp), &size);
            char full[MAX_PATH + 1];
            strcpy(full, tmp);

            sRecentlyUsed[i] = tmp;
        }
        RegCloseKey(key);
        GV->Console->Print("~g~WWD list loaded from registry.");
        bValid = 1;
    }
}

void cMruList::PushNewFile(const char *path, bool bSaveToRegistry) {
    bool added = false;
    for (int i = 0; i < iFilesCount; i++) {
        if (sRecentlyUsed[i] == path) {
            const std::string& pathStr = sRecentlyUsed[i];
            for (int y = i; y > 0; --y)
                sRecentlyUsed[y] = sRecentlyUsed[y - 1];
            sRecentlyUsed[0] = pathStr;
            added = true;
            break;
        }
    }
    if (!added) {
        for (int i = (iFilesCount < 10 ? iFilesCount : 9); i > 0; i--)
            sRecentlyUsed[i] = sRecentlyUsed[i - 1];
        if (iFilesCount < 10) iFilesCount++;
        sRecentlyUsed[0] = path;
    }
    if (bSaveToRegistry && bValid)
        SaveToRegistry(path);
    GV->editState->MruListUpdated();
}

void cMruList::SaveToRegistry(const char *szNew) {
    HKEY key;
    if (GV->iOS == OS_VISTA || GV->iOS == OS_7) {
        RegOpenKeyEx(HKEY_CURRENT_USER,
                     "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\OpenSavePidlMRU\\WWD", 0,
                     KEY_QUERY_VALUE | KEY_SET_VALUE, &key);
        char mrulist[100];
        DWORD size = 100;
        if (RegQueryValueEx(key, "MRUListEx", NULL, NULL, (LPBYTE) (&mrulist), &size) != ERROR_SUCCESS) {
            GV->Console->Print("~r~Unable to open PidlMRU list to save.");
            return;
        } else {
            std::vector<std::pair<int, std::string> > vals;
            for (int i = 0; (i < (size / 4) - 1) && i < 20; i++) {
                char temp[1024];
                size = 1024;
                char buf[8];
                sprintf(buf, "%d", mrulist[i * 4]);
                if (RegQueryValueEx(key, buf, NULL, NULL, (LPBYTE) (&temp), &size) != ERROR_SUCCESS) {
                    GV->Console->Printf("~r~Error querying MRU position %d [#%d]", mrulist[i * 4], i);
                    continue;
                }
                char tmp[MAX_PATH];
                SHGetPathFromIDList((ITEMIDLIST *) &temp, (CHAR *) &tmp);
                vals.push_back(std::pair<int, std::string>(mrulist[i * 4], tmp));
            }

            int iEditId = -1;
            bool bDone = 0;
            for (int i = 0; i < vals.size(); i++) {
                if (!strcmp(vals[i].second.c_str(), szNew)) {
                    vals.insert(vals.begin(), vals[i]);
                    vals.erase(vals.begin() + i + 1);
                    bDone = 1;
                    break;
                }
            }
            if (!bDone) {
                iEditId = vals.back().first;
                vals.pop_back();
                vals.insert(vals.begin(), std::pair<int, std::string>(iEditId, szNew));
                bDone = 1;
            }

            int *regdata = new int[vals.size() + 1];
            for (int i = 0; i < vals.size(); i++)
                regdata[i] = vals[i].first;
            regdata[vals.size()] = 0xFFFFFFFF;

            if (RegSetValueEx(key, "MRUListEx", NULL, REG_BINARY, (const BYTE *) &regdata,
                              DWORD((vals.size() + 1) * 4)) != ERROR_SUCCESS) {
                RegCloseKey(key);
                GV->Console->Printf("~r~Error saving MRUListEx.");
                delete[] regdata;
                return;
            }
            delete[] regdata;

            if (iEditId != -1) {
                LPITEMIDLIST pidl;
                OLECHAR olePath[MAX_PATH];
                LPSHELLFOLDER pDesktopFolder;
                SHGetDesktopFolder(&pDesktopFolder);
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szNew, -1, olePath, MAX_PATH);
                pDesktopFolder->ParseDisplayName(NULL,
                                                 NULL,
                                                 olePath,
                                                 NULL,
                                                 &pidl,
                                                 NULL);

                char keyname[32];
                sprintf(keyname, "%d", iEditId);
                DWORD len = sizeof(ITEMIDLIST);
                LPITEMIDLIST pidl2 = pidl;
                while (pidl2->mkid.cb != 0) {
                    len += pidl2->mkid.cb;
                    pidl2 = PIDLGetNextItem(pidl2);
                }
                RegSetValueEx(key, keyname, NULL, REG_BINARY, (const BYTE *) pidl, len);
                pDesktopFolder->Release();
                CoTaskMemFree(pidl);
            }

            RegCloseKey(key);
            GV->Console->Print("~g~WWD list saved to registry [pidl].");
            return;
        }
    } else if (RegOpenKeyEx(HKEY_CURRENT_USER,
                            "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\OpenSaveMRU\\WWD", 0,
                            KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
        GV->Console->Print("~r~Error opening registry to save WWD list.");
        return;
    } else {

        RegCloseKey(key);
        GV->Console->Print("~g~WWD list saved to registry.");
    }
}

const char *cMruList::GetRecentlyUsedFile(int i) {
    if (i < 0 || i >= 10) return "";
    return sRecentlyUsed[i].c_str();
}
