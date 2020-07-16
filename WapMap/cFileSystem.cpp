#include "cFileSystem.h"
#include <algorithm>
#include <utility>
#include <sys/stat.h>
#include "../shared/commonFunc.h"

cDiscFeed::cDiscFeed(std::string strPath) {
    strAbsoluteLocation = std::move(strPath);
    myType = Feed_HardDrive;
    bIsHDD = (strAbsoluteLocation.empty());
    hDirToWatch = INVALID_HANDLE_VALUE;
    stop = false;
    retrying = false;
    if (!bIsHDD) {
        _CreateWatchdog();
    }
}

cDiscFeed::~cDiscFeed() {
    _DeleteWatchdog();
}

void cDiscFeed::Remount(std::string nPath) {
    strAbsoluteLocation = nPath;
    bIsHDD = (strAbsoluteLocation.empty());
    if (hDirToWatch != INVALID_HANDLE_VALUE) {
        _DeleteWatchdog();
        stop = false;
    }
    if (!bIsHDD) {
        _CreateWatchdog();
    }
}

bool cDiscFeed::_CreateWatchdog() {
    hDirToWatch = CreateFile(strAbsoluteLocation.c_str(), FILE_LIST_DIRECTORY | GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                             FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
    if (hDirToWatch == INVALID_HANDLE_VALUE) {
        if (!retrying) {
            retrying = true;
            retry = std::async(std::launch::async, &cDiscFeed::_RetryCreateWatchdog, this);
        }
        return false;
    } else {
        stop = false;
        watchdog = std::async(std::launch::async, &cDiscFeed::_WatchdogProc, this);
        return true;
    }
}

void cDiscFeed::_RetryCreateWatchdog()
{
    while (!stop && !_CreateWatchdog()) {
        std::unique_lock<std::mutex> lock(cmutex);
        cv.wait_for(lock, std::chrono::milliseconds(500));
    }
    retrying = false;
}

void cDiscFeed::_WatchdogProc() {
#define LISTEN_FOR_CHANGES() ReadDirectoryChangesW(hDirToWatch, &changesBuffer, 2048, 1, \
    FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, &ovl, 0)

    char changesBuffer[2048];
    DWORD dw; OVERLAPPED ovl = {};
    ovl.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!LISTEN_FOR_CHANGES()) {
        retrying = true;
        retry = std::async(std::launch::async, &cDiscFeed::_RetryCreateWatchdog, this);
    } else {
        while (!stop) {
            std::unique_lock<std::mutex> lock(cmutex);
            cv.wait_for(lock, std::chrono::milliseconds(500));
            switch (WaitForSingleObject(ovl.hEvent, 0)) {
            case WAIT_TIMEOUT:
                continue;
            case WAIT_OBJECT_0:
                GetOverlappedResult(hDirToWatch, &ovl, &dw, TRUE);

                {
                    char* buffer = changesBuffer;
                    auto* fni = (FILE_NOTIFY_INFORMATION*)buffer;
                    std::vector<std::string> addedFiles, modifiedFiles, deletedFiles;
                    while (true) {
                        int namelen = fni->FileNameLength / 2;
                        WCHAR* namebuf = new WCHAR[namelen + 1];
                        for (int i = 0; i < namelen; i++)
                            namebuf[i] = fni->FileName[i];
                        namebuf[namelen] = '\0';
                        char* cstr = new char[namelen + 1];
                        wcstombs(cstr, namebuf, namelen + 1);
                        delete[] namebuf;
                        std::string namestr = cstr;
                        delete[] cstr;

                        switch (fni->Action) {
                        case FILE_ACTION_ADDED:
                        case FILE_ACTION_RENAMED_NEW_NAME:
                            addedFiles.emplace_back(namestr);
                            break;
                        case FILE_ACTION_MODIFIED:
                            if (std::find(addedFiles.begin(), addedFiles.end(), namestr) == addedFiles.end()
                                && std::find(modifiedFiles.begin(), modifiedFiles.end(), namestr) == modifiedFiles.end()) {
                                modifiedFiles.emplace_back(namestr);
                            }
                            break;
                        case FILE_ACTION_REMOVED:
                        case FILE_ACTION_RENAMED_OLD_NAME:
                            deletedFiles.emplace_back(namestr);
                            break;
                        }

                        if (fni->NextEntryOffset == 0) break;
                        buffer += fni->NextEntryOffset;
                        fni = (FILE_NOTIFY_INFORMATION*)buffer;
                    }

                    {
                        std::vector<std::string>* srcV[3] = { &addedFiles, &modifiedFiles, &deletedFiles },
                            * dstV[3] = { &vNewFiles, &vModFiles, &vDelFiles };
                        for (int i = 0; i < 3; i++) {
                            for (auto& strTranslatedPath : *srcV[i]) {
                                size_t slashpos = strTranslatedPath.find('\\');
                                while (slashpos != std::string::npos) {
                                    strTranslatedPath[slashpos] = '/';
                                    slashpos = strTranslatedPath.find('\\');
                                }
                                dstV[i]->emplace_back(strTranslatedPath);
                            }
                            srcV[i]->clear();
                        }
                    }
                }

                break;
            }

            ResetEvent(ovl.hEvent);
            if (!LISTEN_FOR_CHANGES()) {
                retrying = true;
                retry = std::async(std::launch::async, &cDiscFeed::_RetryCreateWatchdog, this);
                return;
            }
        }
    }
}

void cDiscFeed::_DeleteWatchdog() {
    stop = true;
    if (retrying) {
        cv.notify_all();
        retry.wait();
    } else {
        cv.notify_all();
        watchdog.wait();
    }

    if (hDirToWatch != INVALID_HANDLE_VALUE) {
        CloseHandle(hDirToWatch);
        hDirToWatch = INVALID_HANDLE_VALUE;
    }
}

void cDiscFeed::UpdateModificationFlag() {

}

bool cDiscFeed::GetModificationFlag() {
    return (!vNewFiles.empty() || !vModFiles.empty() || !vDelFiles.empty());
}

void cDiscFeed::ResetModificationFlag() {
    vNewFiles.clear();
    vModFiles.clear();
    vDelFiles.clear();
}

time_t cDiscFeed::GetFileModTime(const char *path) {
    std::string filePath = (bIsHDD ? path : strAbsoluteLocation + '/' + path);
    struct stat fileStatus {};
    if (stat(filePath.c_str(), &fileStatus) >= 0)
        return fileStatus.st_mtime;
    return 0;
}

bool cDiscFeed::FileExists(const char *path) {
    std::string filePath = (bIsHDD ? path
                                   : strAbsoluteLocation + '/' + path);
    FILE *f = fopen(filePath.c_str(), "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

void cDiscFeed::SetFileContent(const char *path, const std::string& strContent) {
    std::string filePath = (bIsHDD ? path
                                   : strAbsoluteLocation + '/' + path);
    FILE *f = fopen(filePath.c_str(), "wb");
    if (!f) return;
    fwrite(strContent.c_str(), strContent.length(), 1, f);
    fclose(f);
}

unsigned char *cDiscFeed::GetFileContent(const char *path, unsigned int &oiDataLength, unsigned int iReadStart,
                                         unsigned int iReadLen) {
    std::string filePath(bIsHDD ? path : strAbsoluteLocation + '/' + path);
    FILE *f = fopen(filePath.c_str(), "rb");
    if (!f) {
        oiDataLength = 0;
        return 0;
    }
    fseek(f, 0, SEEK_END);
    int ifilelen = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (iReadStart >= ifilelen || (iReadLen != 0 && iReadLen + iReadStart > ifilelen)) {
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
    std::unique_lock<std::mutex> lock(cmutex);
    std::vector<std::string> ret = vModFiles;
    return ret;
}

std::vector<std::string> cDiscFeed::GetDeletedFiles() {
    std::unique_lock<std::mutex> lock(cmutex);
    std::vector<std::string> ret = vDelFiles;
    return ret;
}

std::vector<std::string> cDiscFeed::GetNewFiles() {
    std::unique_lock<std::mutex> lock(cmutex);
    std::vector<std::string> ret = vNewFiles;
    return ret;
}

int cDiscFeed::GetFileSize(const char *path) {
    std::string filePath = (bIsHDD ? path
                                   : strAbsoluteLocation + '/' + path);
    struct stat fileStatus {};
    if (stat(filePath.c_str(), &fileStatus) >= 0)
        return fileStatus.st_size;
    return 0;
}

bool cDiscFeed::DirectoryExists(const char *path) {
    std::string abspath = (bIsHDD ? std::string(path)
                                  : strAbsoluteLocation + '/' + path);
    HANDLE hFind;
    WIN32_FIND_DATA fdata;
    hFind = FindFirstFile(abspath.c_str(), &fdata);
    return hFind != INVALID_HANDLE_VALUE;
}

std::vector<std::string> cDiscFeed::GetDirectoryContents(const char *path, bool bRecursive) {
    std::string abspath = (bIsHDD ? std::string(path)
                                  : strAbsoluteLocation + '/' + path) + "/*";
    std::vector<std::string> r = {};
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fdata;
    hFind = FindFirstFile(abspath.c_str(), &fdata);
    if (hFind == INVALID_HANDLE_VALUE) return r;

    do {
        if (fdata.cFileName[0] == '.' &&
            ((fdata.cFileName[1] == '.' && fdata.cFileName[2] == '\0') || fdata.cFileName[1] == '\0'))
            continue;
        std::string entry = fdata.cFileName;
        r.push_back(entry);
        if (bRecursive) {
            std::string np(path);
            np += '/';
            np += entry;
            std::vector<std::string> r2 = GetDirectoryContents(np.c_str(), true);
            entry += '/';
            for (const auto & fileName : r2)
                r.push_back(entry + fileName);
        }
    } while (FindNextFile(hFind, &fdata) != 0);
    return r;
}

cRezFeed::cRezFeed(const std::string& strPath) {
    strAbsoluteLocation = strPath;
    iModificationFlag = false;
    iModTime = 0;
    myType = Feed_REZ;
    hREZ = new REZ::Parser(strPath.c_str());
    cRezFeed::UpdateModificationFlag();
}

cRezFeed::~cRezFeed() {
    delete hREZ;
}

time_t cRezFeed::GetFileModTime(const char *path) {
    return iModTime;
}

int cRezFeed::GetFileSize(const char *path) {
    REZ::Element *el = hREZ->GetRoot()->GetElement(path);
    return el && el->GetSize();
}

bool cRezFeed::FileExists(const char *path) {
    REZ::Element *el = hREZ->GetRoot()->GetElement(path);
    return el && el->IsFile();
}

unsigned char * cRezFeed::GetFileContent(const char *path, unsigned int &oiDataLength, unsigned int iReadStart, unsigned int iReadEnd) {
    REZ::Element *el = hREZ->GetRoot()->GetElement(path);
    if (!el || !el->IsFile()) {
        oiDataLength = 0;
        return 0;
    };
    auto *file = (REZ::File *) el;
    int dl = 0;
    unsigned char *p = (unsigned char *) file->GetData(&dl);
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
    iModificationFlag = false;
}

bool cRezFeed::DirectoryExists(const char *path) {
    REZ::Element *el = hREZ->GetRoot()->GetElement(path);
    return el && el->IsDir();
}

std::vector<std::string> cRezFeed::GetDirectoryContents(const char *path, bool bRecursive) {
    REZ::Element *el = hREZ->GetRoot()->GetElement(path);
    std::vector<std::string> r = {};
    if (!el || !el->IsDir()) return r;
    auto *dir = (REZ::Dir *) el;
    for (int i = 0; i < dir->GetElementsCount(); i++) {
        std::string tempstr = std::string(dir->GetElement(i)->GetName());
        r.push_back(tempstr);
        if (dir->GetElement(i)->IsDir() && bRecursive) {
            std::string npath(path);
            npath.append("/");
            npath.append(dir->GetElement(i)->GetName());
            std::vector<std::string> x = GetDirectoryContents(npath.c_str(), true);
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
        iModificationFlag = true;
    }
    iModTime = n;
}
