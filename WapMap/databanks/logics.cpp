#include "logics.h"

#include <algorithm>
#include <filesystem>
#include "../../shared/cProgressInfo.h"
#include "../globals.h"
#include "../states/editing_ww.h"

extern structProgressInfo _ghProgressInfo;

bool cLogicsBank_Sort(cCustomLogic *a, cCustomLogic *b) {
    return (std::string(a->GetName()) < std::string(b->GetName()));
}

cCustomLogic::cCustomLogic(cFile hFile, std::string id) {
    SetFile(hFile);
    _strName = id;
    _bLoaded = false;
}

void cCustomLogic::Load() {
    if (_bLoaded) return;
    unsigned int len;
    unsigned char *ptr = GetFile().hFeed->GetFileContent(GetFile().strPath.c_str(), len);
    if (len == 0) return;
    strContent = std::string((const char *) ptr, len);
    delete[] ptr;
    _bLoaded = 1;
}

void cCustomLogic::Unload() {
    strContent = "";
    _bLoaded = 0;
}

void cCustomLogic::SetContent(std::string str) {
    strContent = str;
}

void cCustomLogic::Save() {
    cFile n;
    if (GetFile().hFeed == 0) {
        n = GV->editState->hDataCtrl->AssignFileForLogic(GetName());
    } else
        n = GetFile();
    n.hFeed->SetFileContent(n.strPath.c_str(), strContent);
    SetFile(n);
}

void cCustomLogic::DeleteFile() {
    remove(GetPath().c_str());
}

std::string cCustomLogic::GetPath() {
    return GetFile().hFeed->GetAbsoluteLocation() + '\\' + GetFile().strPath;
}

void cBankLogic::SetGlobalScript(cCustomLogic *h) {
    hGlobalScript = h;
}

void cBankLogic::RegisterLogic(cCustomLogic *h) {
    m_vAssets.push_back(h);
    SortLogics();
}

cCustomLogic *cBankLogic::GetLogicByName(const char *pszID) {
    for (auto & m_vAsset : m_vAssets)
        if (m_vAsset->GetName() == pszID)
            return m_vAsset;
    return 0;
}

void cBankLogic::SortLogics() {
    std::sort(m_vAssets.begin(), m_vAssets.end(), cLogicsBank_Sort);
}

void cBankLogic::BatchProcessStart() {
    _ghProgressInfo.iGlobalProgress = 8;
    _ghProgressInfo.strGlobalCaption = "Loading custom logics...";
}

bool cBankLogic::RenameLogic(cCustomLogic *hLogic, const std::string& strName) {
    if (strName == "main") return false;
    cFile origFile = hLogic->GetFile();

    for (char* c = (char*)strName.c_str(); *c != '\0'; c++) {
        if (*c == ':') *c = '/';
    }

    std::string nFilePath(origFile.strPath.substr(0, origFile.strPath.size() - hLogic->GetName().size() - 4));
    nFilePath += strName;
    nFilePath += ".lua";

    std::filesystem::path absOrigPath(hLogic->GetPath(), std::filesystem::path::format::native_format);
    std::filesystem::path absNewPath(origFile.hFeed->GetAbsoluteLocation() + '/' + nFilePath,
            std::filesystem::path::format::native_format);

    if (std::filesystem::exists(absNewPath)) {
        std::string s1 = absOrigPath.make_preferred().string(), s2 = absNewPath.make_preferred().string();
        if (std::equal(s1.begin(), s1.end(), s2.begin(), s2.end(),[](char a, char b) {
            return tolower(a) == tolower(b);
        })) {
            origFile.strPath = nFilePath;
            hLogic->SetFile(origFile);
            hLogic->SetName(strName);
            SortLogics();

            for (int i = 0; i < m_vAssets.size(); ++i) {
                if (m_vAssets[i] == hLogic) {
                    GV->editState->lbbrlLogicList->setSelected(i);
                    GV->editState->SyncLogicBrowser();
                    break;
                }
            }

            return rename(absOrigPath.string().c_str(), absNewPath.string().c_str()) == 0;
        } else {
            return false;
        }
    } else {
        std::error_code _Ec;
        std::filesystem::path dir = absNewPath.parent_path();
        std::filesystem::create_directories(dir, _Ec);
    }

    selectWhenAdding = true;

    if (rename(absOrigPath.string().c_str(), absNewPath.string().c_str()) != 0) {
        selectWhenAdding = false;
        return false;
    }

    return true;
}

std::string cBankLogic::GetMountPointForFile(std::string strFilePath, std::string strPrefix) {
    const char* ext = strFilePath.c_str() + strFilePath.length() - 3;
    if (!(
        (ext[0] == 'l' || ext[0] == 'L')
        && (ext[1] == 'u' || ext[1] == 'U')
        && (ext[2] == 'a' || ext[2] == 'A')
    )) {
        return "";
    }

    return std::string("/LOGICS/") + strPrefix + '/' + strFilePath;
}

cCustomLogic *cBankLogic::AllocateAssetForMountPoint(cDC_MountEntry mountEntry) {
    const char* start = mountEntry.strMountPoint.c_str();
    const char* end = strchr(start + 8, '/') + 1;
    size_t length = mountEntry.strMountPoint.size() - (end - start);
    std::string logicName(mountEntry.vFiles[0].strPath.c_str() + mountEntry.vFiles[0].strPath.size() - length);
    logicName.resize(logicName.size() - 4);

    for (char* c = (char*)logicName.c_str(); *c != '\0'; c++) {
        if (*c == '/' || *c == '\\') { *c = ':'; }
    }

    auto customLogic = new cCustomLogic(mountEntry.vFiles[0], logicName);
    m_vAssets.push_back(customLogic);
    SortLogics();

    if (selectWhenAdding) {
        for (int i = 0; i < m_vAssets.size(); ++i) {
            if (m_vAssets[i] == customLogic) {
                GV->editState->lbbrlLogicList->setSelected(i);
                selectWhenAdding = false;
                GV->editState->SyncLogicBrowser();
                break;
            }
        }
    }

    if (logicName == "main") {
        hGlobalScript = customLogic;
    }

    customLogic->_hBank = (cAssetBank<cAsset>*)this;
    return customLogic;
}

void cBankLogic::DeleteAsset(cCustomLogic *hLogic) {
    if (hLogic == hGlobalScript) {
        hGlobalScript = 0;
    }

    for (size_t i = 0; i < m_vAssets.size(); i++) {
        if (m_vAssets[i] == hLogic) {
            if (GV->editState->lbbrlLogicList->getSelected() == i) {
                GV->editState->lbbrlLogicList->setSelected(-1);
                GV->editState->SyncLogicBrowser();
            } else if (GV->editState->lbbrlLogicList->getSelected() > i) {
                GV->editState->lbbrlLogicList->setSelected(GV->editState->lbbrlLogicList->getSelected() - 1);
            }
            m_vAssets.erase(m_vAssets.begin() + i);
            break;
        }
    }
    delete hLogic;
}
