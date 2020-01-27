#include "cFileSystem.h"
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <windows.h>
#include "../shared/commonFunc.h"

DWORD WINAPI _cDiscWatchdogThread(LPVOID phData) {
    cDiscFeed *parent = (cDiscFeed *) phData;
    char retbuffer[2048];
    while (1) {
        DWORD bytes = 0;
        if (ReadDirectoryChangesW(parent->hDirToWatch, &retbuffer, 2048, 1,
                                  FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, &bytes, 0, 0)) {
            char *buffer = retbuffer;
            FILE_NOTIFY_INFORMATION *fni = (FILE_NOTIFY_INFORMATION *) buffer;
            std::vector<std::string> vstrNewBuf, vstrModBuf, vstrDelBuf;
            while (1) {

                int namelen = fni->FileNameLength / 2;
                WCHAR *namebuf = new WCHAR[namelen + 1];
                for (int i = 0; i < namelen; i++)
                    namebuf[i] = fni->FileName[i];
                namebuf[namelen] = '\0';
                char *cstr = new char[namelen + 1];
                wcstombs(cstr, namebuf, namelen + 1);
                delete[] namebuf;
                std::string namestr = cstr;
                delete[] cstr;

                if (fni->Action == FILE_ACTION_ADDED) {
                    vstrNewBuf.push_back(namestr);
                } else if (fni->Action == FILE_ACTION_MODIFIED) {
                    bool fnd = 0;
                    for (size_t i = 0; i < vstrNewBuf.size(); i++)
                        if (namestr.compare(vstrNewBuf[i]) == 0) {
                            fnd = 1;
                            break;
                        }
                    if (!fnd) {
                        for (size_t i = 0; i < vstrModBuf.size(); i++) {
                            if (namestr.compare(vstrModBuf[i]) == 0) {
                                fnd = 1;
                                break;
                            }
                        }
                        if (!fnd) {
                            vstrModBuf.push_back(namestr);
                        }
                    }
                } else if (fni->Action == FILE_ACTION_REMOVED) {
                    vstrDelBuf.push_back(namestr);
                } else if (fni->Action == FILE_ACTION_RENAMED_OLD_NAME) {
                    vstrDelBuf.push_back(namestr);
                } else if (fni->Action == FILE_ACTION_RENAMED_NEW_NAME) {
                    vstrNewBuf.push_back(namestr);
                }

                if (fni->NextEntryOffset == 0) break;
                buffer += fni->NextEntryOffset;
                fni = (FILE_NOTIFY_INFORMATION *) buffer;
            }

            EnterCriticalSection(&parent->myCS);
            std::vector<std::string> *srcV[3] = {&vstrNewBuf, &vstrModBuf, &vstrDelBuf},
                    *dstV[3] = {&parent->vstrNewFiles, &parent->vstrModFiles, &parent->vstrDelFiles};
            for (int i = 0; i < 3; i++) {
                while (!srcV[i]->empty()) {
                    std::string strTranslatedPath = srcV[i]->at(0);
                    size_t slashpos = strTranslatedPath.find('\\');
                    while (slashpos != std::string::npos) {
                        strTranslatedPath[slashpos] = '/';
                        slashpos = strTranslatedPath.find('\\');
                    }
                    dstV[i]->push_back(strTranslatedPath);
                    srcV[i]->erase(srcV[i]->begin());
                }
            }
            LeaveCriticalSection(&parent->myCS);
        }
    }
    return 0;
}

cDiscFeed::cDiscFeed(std::string strPath) {
    strAbsoluteLocation = strPath;
    myType = Feed_HardDrive;
    bIsHDD = (strAbsoluteLocation.empty());
    if (!bIsHDD)
        _CreateWatchdog();
    else
        hDirToWatch = INVALID_HANDLE_VALUE;
}

cDiscFeed::~cDiscFeed() {
    if (hDirToWatch != INVALID_HANDLE_VALUE)
        _DeleteWatchdog();
}

void cDiscFeed::Remount(std::string nPath) {
    strAbsoluteLocation = nPath;
    bIsHDD = (strAbsoluteLocation.empty());
    if (hDirToWatch != INVALID_HANDLE_VALUE)
        _DeleteWatchdog();
    if (!bIsHDD)
        _CreateWatchdog();
}

void cDiscFeed::_CreateWatchdog() {
    InitializeCriticalSection(&myCS);
    hDirToWatch = CreateFile(strAbsoluteLocation.c_str(), FILE_LIST_DIRECTORY,
                             FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, 0, OPEN_EXISTING,
                             FILE_FLAG_BACKUP_SEMANTICS, 0);
    DWORD dw;
    hWatchThread = CreateThread(0, 0, _cDiscWatchdogThread, this, 0, &dw);
}

void cDiscFeed::_DeleteWatchdog() {
    if (hDirToWatch == INVALID_HANDLE_VALUE) return;
    TerminateThread(hWatchThread, 0);
    CloseHandle(hDirToWatch);
    hDirToWatch = INVALID_HANDLE_VALUE;
    DeleteCriticalSection(&myCS);
}

void cDiscFeed::UpdateModificationFlag() {

}

bool cDiscFeed::GetModificationFlag() {
    return (!vstrNewFiles.empty() || !vstrModFiles.empty() || !vstrDelFiles.empty());
}

void cDiscFeed::ResetModificationFlag() {
    vstrNewFiles.clear();
    vstrModFiles.clear();
    vstrDelFiles.clear();
}

time_t cDiscFeed::GetFileModTime(const char *path) {
    std::string npath = (bIsHDD ? path : strAbsoluteLocation + "/" + path);
    struct stat filestatus;
    if (stat(npath.c_str(), &filestatus) >= 0)
        return filestatus.st_mtime;
    return 0;
}

bool cDiscFeed::FileExists(const char *path) {
    std::string npath = (bIsHDD ? std::string(path)
                                : strAbsoluteLocation + "/" + std::string(path));
    FILE *f = fopen(npath.c_str(), "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

void cDiscFeed::SetFileContent(const char *path, std::string strContent) {
    std::string realpath = (bIsHDD ? std::string(path)
                                   : strAbsoluteLocation + "/" + std::string(path));
    FILE *f = fopen(realpath.c_str(), "wb");
    if (!f) return;
    fwrite(strContent.c_str(), strContent.length(), 1, f);
    fclose(f);
}

unsigned char *cDiscFeed::GetFileContent(const char *path, unsigned int &oiDataLength, unsigned int iReadStart,
                                         unsigned int iReadLen) {
    std::string npath = (bIsHDD ? std::string(path)
                                : strAbsoluteLocation + "/" + std::string(path));
    FILE *f = fopen(npath.c_str(), "rb");
    if (!f) {
        oiDataLength = 0;
        return 0;
    }
    fseek(f, 0, SEEK_END);
    int ifilelen = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (iReadStart >= ifilelen || iReadLen != 0 && iReadLen + iReadStart > ifilelen) {
        oiDataLength = 0;
        fclose(f);
        return 0;
    }
    if (iReadLen == 0)
        iReadLen = ifilelen - iReadStart;

    oiDataLength = iReadLen;
    unsigned char *data = new unsigned char[oiDataLength];
    fread(data, 1, oiDataLength, f);
    fclose(f);
    return data;
}

std::vector<std::string> cDiscFeed::GetModifiedFiles() {
    EnterCriticalSection(&myCS);
    std::vector<std::string> ret = vstrModFiles;
    LeaveCriticalSection(&myCS);
    return ret;
}

std::vector<std::string> cDiscFeed::GetDeletedFiles() {
    EnterCriticalSection(&myCS);
    std::vector<std::string> ret = vstrDelFiles;
    LeaveCriticalSection(&myCS);
    return ret;
}

std::vector<std::string> cDiscFeed::GetNewFiles() {
    EnterCriticalSection(&myCS);
    std::vector<std::string> ret = vstrNewFiles;
    LeaveCriticalSection(&myCS);
    return ret;
}

int cDiscFeed::GetFileSize(const char *path) {
    std::string npath = (bIsHDD ? std::string(path)
                                : strAbsoluteLocation + "/" + std::string(path));
    struct stat filestatus;
    if (stat(npath.c_str(), &filestatus) >= 0)
        return filestatus.st_size;
    return 0;
}

bool cDiscFeed::DirectoryExists(const char *path) {
    std::string abspath = (bIsHDD ? std::string(path)
                                  : strAbsoluteLocation + "/" + std::string(path));
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fdata;
    hFind = FindFirstFile(abspath.c_str(), &fdata);
    if (hFind == INVALID_HANDLE_VALUE) return 0;
    return 1;
}

std::vector<std::string> cDiscFeed::GetDirectoryContents(const char *path, bool bRecursive) {
    std::string pstr = std::string(path);
    //std::transform(pstr.begin(), pstr.end(), pstr.begin(), ::tolower);
    std::string abspath = (bIsHDD ? std::string(path)
                                  : strAbsoluteLocation + "/" + std::string(path)) + "/*";
    std::vector<std::string> r;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fdata;
    hFind = FindFirstFile(abspath.c_str(), &fdata);
    if (hFind == INVALID_HANDLE_VALUE) return r;

    do {
        if (fdata.cFileName[0] == '.' &&
            (fdata.cFileName[1] == '.' && fdata.cFileName[2] == '\0' || fdata.cFileName[1] == '\0'))
            continue;
        std::string entry = fdata.cFileName;
        //std::transform(entry.begin(), entry.end(), entry.begin(), ::tolower);
        r.push_back(entry);
        if (bRecursive) {
            std::string np = pstr + "/" + entry;
            std::vector<std::string> r2 = GetDirectoryContents(np.c_str(), 1);
            for (size_t z = 0; z < r2.size(); z++)
                r.push_back(entry + "/" + r2[z]);
        }
    } while (FindNextFile(hFind, &fdata) != 0);
    return r;
}

cRezFeed::cRezFeed(std::string strPath) {
    strAbsoluteLocation = strPath;
    iModificationFlag = 0;
    iModTime = 0;
    myType = Feed_REZ;
    hREZ = new REZ::Parser(strPath.c_str());
    UpdateModificationFlag();
}

cRezFeed::~cRezFeed() {
    delete hREZ;
}

time_t cRezFeed::GetFileModTime(const char *path) {
    return iModTime;
}

int cRezFeed::GetFileSize(const char *path) {
    REZ::Element *el = hREZ->GetRoot()->GetElement(path);
    if (el != 0)
        return el->GetSize();
    return 0;
}

bool cRezFeed::FileExists(const char *path) {
    REZ::Element *el = hREZ->GetRoot()->GetElement(path);
    if (!el) return 0;
    return el->IsFile();
}

unsigned char *
cRezFeed::GetFileContent(const char *path, unsigned int &oiDataLength, unsigned int iReadStart, unsigned int iReadEnd) {
    REZ::Element *el = hREZ->GetRoot()->GetElement(path);
    if (!el || !el->IsFile()) {
        oiDataLength = 0;
        return 0;
    };
    REZ::File *f = (REZ::File *) el;
    int dl = 0;
    unsigned char *p = (unsigned char *) f->GetData(&dl);
    oiDataLength = dl;
    return p;
}

void cRezFeed::ReadingStart() {
    hREZ->AttachFile();
}

void cRezFeed::ReadingStop() {
    hREZ->DetachFile();
}

void cRezFeed::Reload() {
    hREZ->Reload();
    iModificationFlag = 0;
}

bool cRezFeed::DirectoryExists(const char *path) {
    REZ::Element *el = hREZ->GetRoot()->GetElement(path);
    if (!el) return 0;
    return el->IsDir();
}

std::vector<std::string> cRezFeed::GetDirectoryContents(const char *path, bool bRecursive) {
    REZ::Element *el = hREZ->GetRoot()->GetElement(path);
    std::vector<std::string> r;
    if (!el || !el->IsDir()) return r;
    REZ::Dir *d = (REZ::Dir *) el;
    for (int i = 0; i < d->GetElementsCount(); i++) {
        std::string tempstr = std::string(d->GetElement(i)->GetName());
        //std::transform(tempstr.begin(), tempstr.end(), tempstr.begin(), ::tolower);
        r.push_back(tempstr);
        if (d->GetElement(i)->IsDir() && bRecursive) {
            std::string npath(path);
            npath.append("/");
            npath.append(d->GetElement(i)->GetName());
            std::vector<std::string> x = GetDirectoryContents(npath.c_str(), 1);
            while (!x.empty()) {
                std::string pn = tempstr;
                pn.append("/");
                pn.append(x.front());
                r.push_back(pn);
                x.erase(x.begin());
            }
        }
    }
    return r;
}

void cRezFeed::UpdateModificationFlag() {
    time_t n = hREZ->GetActualModTime();
    if (iModTime != 0 && n != iModTime) {
        iModificationFlag = 1;
    }
    iModTime = n;
}
