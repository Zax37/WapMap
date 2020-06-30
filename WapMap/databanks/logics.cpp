#include "logics.h"

#include <algorithm>
#include <filesystem>
#include "../../shared/cProgressInfo.h"
#include "../../shared/commonFunc.h"
#include "../globals.h"
#include "../states/editing_ww.h"

extern structProgressInfo _ghProgressInfo;

bool cLogicsBank_Sort(cCustomLogic *a, cCustomLogic *b) {
    return (std::string(a->GetName()) < std::string(b->GetName()));
}

cCustomLogic::cCustomLogic(cFile hFile, std::string id) {
    SetFile(hFile);
    _strName = id;
    _bLoaded = 0;
}

cCustomLogic::~cCustomLogic() {

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

cBankLogic::cBankLogic(WWD::Parser *hParser) : cAssetBank(hParser) {
    hGlobalScript = 0;
    selectWhenAdding = false;
}

cBankLogic::~cBankLogic() {

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
        if (!strcmp(m_vAsset->GetName(), pszID))
            return m_vAsset;
    return 0;
}

std::string cBankLogic::getElementAt(int i) {
    if (i < 0 || i >= m_vAssets.size()) return "";
    return m_vAssets[i]->GetName();
}

int cBankLogic::getNumberOfElements() {
    return m_vAssets.size();
}

void cBankLogic::SortLogics() {
    std::sort(m_vAssets.begin(), m_vAssets.end(), cLogicsBank_Sort);
}

void cBankLogic::BatchProcessStart(cDataController *hDC) {
    _ghProgressInfo.iGlobalProgress = 8;
    _ghProgressInfo.strGlobalCaption = "Loading custom logics...";
}

bool cBankLogic::RenameLogic(cCustomLogic *hLogic, const std::string& strName) {
    if (strName == "main") return false;
    cFile origFile = hLogic->GetFile();

    std::string dirPath;
    if (origFile.strPath.find_last_of("/\\") != std::string::npos)
        dirPath = origFile.strPath.substr(0, origFile.strPath.find_last_of("/\\"));
    std::string nFilePath = dirPath;
    nFilePath += '/';
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
    }

    selectWhenAdding = true;

    if (rename(absOrigPath.string().c_str(), absNewPath.string().c_str()) != 0) {
        selectWhenAdding = false;
        return false;
    }

    return true;
}

std::string cBankLogic::GetMountPointForFile(std::string strFilePath, std::string strPrefix) {
    return std::string("/LOGICS/") + strPrefix + strFilePath;
}

cAsset *cBankLogic::AllocateAssetForMountPoint(cDataController *hDC, cDC_MountEntry mountEntry) {
    int nameStart = mountEntry.vFiles[0].strPath.find_last_of("/\\") + 1;
    int nameEnd = mountEntry.vFiles[0].strPath.find_last_of(".");
    std::string filename = mountEntry.vFiles[0].strPath.substr(nameStart, nameEnd - nameStart);

    auto customLogic = new cCustomLogic(mountEntry.vFiles[0], filename);
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

    if (filename == "main") {
        hGlobalScript = customLogic;
    }

    customLogic->_hBank = this;
    return customLogic;
}

void cBankLogic::DeleteAsset(cAsset *hLogic) {
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
