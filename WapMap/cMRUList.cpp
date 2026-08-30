#include "cMRUList.h"

#include <codecvt>
#include <filesystem>

#include "globals.h"
#include "states/editing_ww.h"
#include <shlobj.h>

inline LPITEMIDLIST PIDLGetNextItem(LPITEMIDLIST pidl) {
    return pidl ? reinterpret_cast<LPITEMIDLIST>(reinterpret_cast<BYTE *>(pidl) + pidl->mkid.cb) : NULL;
}

cMruList::cMruList() {
    Reload();
}

cMruList::~cMruList() {

}

std::wstring ResolveShortcut(const std::wstring& lnkPath)
{
    std::wstring result;

    IShellLinkW* pShellLink = nullptr;
    IPersistFile* pPersistFile = nullptr;

    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr,
                                   CLSCTX_INPROC_SERVER,
                                   IID_IShellLinkW,
                                   (void**)&pShellLink)))
    {
        if (SUCCEEDED(pShellLink->QueryInterface(IID_IPersistFile,
                                                  (void**)&pPersistFile)))
        {
            if (SUCCEEDED(pPersistFile->Load(lnkPath.c_str(), STGM_READ)))
            {
                wchar_t path[MAX_PATH];
                if (SUCCEEDED(pShellLink->GetPath(path, MAX_PATH, nullptr, 0)))
                {
                    result = path;
                }
            }
            pPersistFile->Release();
        }
        pShellLink->Release();
    }

    return result;
}

bool HasExtension(const std::wstring& path, const std::wstring& ext)
{
    if (path.length() < ext.length())
        return false;

    return _wcsicmp(
        path.c_str() + path.length() - ext.length(),
        ext.c_str()
    ) == 0;
}

std::string ws2s(const std::wstring& wstr)
{
    typedef std::codecvt_utf8<wchar_t> convert_typeX;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}

struct entry {
    std::string path;
    std::filesystem::file_time_type time;
    entry(std::string p, std::filesystem::file_time_type t) : path(std::move(p)), time(t) {}
};

void cMruList::Reload() {
    bValid = false;
    iFilesCount = 0;

    if (GV->iOS >= OS_VISTA) {
        GV->Console->Print("~w~Reloading MRU list...");
        PWSTR recentPath = nullptr;
        if (FAILED(SHGetKnownFolderPath(FOLDERID_Recent, 0, nullptr, &recentPath)))
            return;
        CoInitialize(nullptr);
        std::filesystem::path recentDir(recentPath);
        CoTaskMemFree(recentPath);

        std::vector<entry> paths;
        for (const auto& entry : std::filesystem::directory_iterator(recentDir))
        {
            if (paths.size() >= 10)
                break;

            if (!entry.is_regular_file())
                continue;

            if (entry.path().extension() != L".lnk")
                continue;

            std::wstring resolved = ResolveShortcut(entry.path().wstring());
            if (resolved.empty())
                continue;

            if (!HasExtension(resolved, L".wwd"))
                continue;

            paths.emplace_back(
                ws2s(resolved),
                entry.last_write_time()
            );
        }
        CoUninitialize();

        bValid = true;
        std::sort(paths.begin(), paths.end(), [](entry& p1, entry& p2) {return p1.time > p2.time;});
        for (const auto& p : paths) {
            sRecentlyUsed[iFilesCount++] = p.path;
        }
        GV->Console->Printf("~g~WWD list loaded from registry [pidl] (%d positions).", iFilesCount);
    }
}

void cMruList::PushNewFile(const char *path, bool bSaveToRegistry) {
    bool added = false;
    for (int i = 0; i < iFilesCount; i++) {
        if (sRecentlyUsed[i] == path) {
            std::string pathStr = sRecentlyUsed[i];
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
    if (GV->iOS >= OS_VISTA) {
        SHAddToRecentDocs(SHARD_PATHA, szNew);
        GV->Console->Print("~g~WWD list saved to registry.");
    }
}

const char *cMruList::GetRecentlyUsedFile(int i) {
    if (i < 0 || i >= 10) return "";
    return sRecentlyUsed[i].c_str();
}
