#include "sounds.h"
#include "../globals.h"
#include "../cParallelLoop.h"
#include "../../shared/cProgressInfo.h"

extern structProgressInfo _ghProgressInfo;
extern HGE *hge;

bool cSndBank_SortAssets(cSndBankAsset *a, cSndBankAsset *b) {
    return (std::string(a->GetName()) < std::string(b->GetName()));
}

cSndBankAsset *cBankSound::GetAssetByID(const char *pszID) {
    for (auto & m_vAsset : m_vAssets) {
        if (m_vAsset->GetName() == pszID) {
            return m_vAsset;
        }
    }
    return NULL;
}

cSndBankAsset::cSndBankAsset(cFile hFile, std::string id) {
    SetFile(hFile);
    _strName = id;
}

cSndBankAsset::~cSndBankAsset() {
    Unload();
}

void cSndBankAsset::Load() {
    unsigned int len;
    unsigned char *ptr = GetFile().hFeed->GetFileContent(GetFile().strPath.c_str(), len);
    if (len == 0) return;
    m_snd = hge->Effect_Load((const char *) ptr, len);
    delete[] ptr;
    _bLoaded = (m_snd != 0);
}

void cSndBankAsset::Unload() {
    hge->Effect_Free(m_snd);
    _bLoaded = 0;
}

void cBankSound::SortAssets() {
    sort(m_vAssets.begin(), m_vAssets.end(), cSndBank_SortAssets);
}

void cBankSound::BatchProcessStart() {
    GV->Console->Printf("Loading sounds...");
    _ghProgressInfo.iGlobalProgress = 7;
    _ghProgressInfo.strGlobalCaption = "Loading sounds...";
    _ghProgressInfo.strDetailedCaption = "Scanning sounds...";
    _ghProgressInfo.iDetailedProgress = 0;
    _ghProgressInfo.iDetailedEnd = 100000;
}

void cBankSound::BatchProcessEnd() {
    _ghProgressInfo.iDetailedProgress = 50000;
    _ghProgressInfo.iDetailedEnd = 100000;
    _ghProgressInfo.strDetailedCaption = "Sorting...";
    SortAssets();
    if (hDC->GetLooper())
        hDC->GetLooper()->Tick();
    for (size_t i = 0; i < m_vAssets.size(); i++) {
        char buf[256];
        sprintf(buf, "Loading: %s [%d/%d]", m_vAssets[i]->GetName().c_str(), i, m_vAssets.size());
        _ghProgressInfo.strDetailedCaption = buf;
        _ghProgressInfo.iDetailedProgress = 50000 + (float(i) / float(m_vAssets.size())) * 50000.0f;
        if (hDC->GetLooper() != 0)
            hDC->GetLooper()->Tick();
        m_vAssets[i]->Load();
    }
}

cSndBankAsset * cBankSound::AllocateAssetForMountPoint(cDC_MountEntry mountEntry) {
    std::string soundMountPoint(mountEntry.strMountPoint.c_str() + 8);
    soundMountPoint.resize(soundMountPoint.length() - 4);

    std::transform(soundMountPoint.begin(), soundMountPoint.end(), soundMountPoint.begin(), ::toupper);
    do {
        const size_t slash = soundMountPoint.find('/');
        if (slash == std::string::npos) break;
        soundMountPoint[slash] = '_';
    } while (true);

    cSndBankAsset *as = GetAssetByID(soundMountPoint.c_str());
    if (!as && !mountEntry.vFiles.empty()) {
        as = new cSndBankAsset(mountEntry.vFiles[0], soundMountPoint);
        m_vAssets.push_back(as);
    }

    return as;
}

bool isWav(const char* ext) {
    return (ext[0] == 'w' || ext[0] == 'W')
        && (ext[1] == 'a' || ext[1] == 'A')
        && (ext[2] == 'v' || ext[2] == 'V');
}

std::string cBankSound::GetMountPointForFile(std::string strFilePath, std::string strPrefix) {
    if (!isWav(strFilePath.c_str() + strFilePath.length() - 3)) {
        return "";
    }

    return "/" + GetFolderName() + "/" + strPrefix + "/" + strFilePath;
}

void cBankSound::DeleteAsset(cSndBankAsset *hAsset) {
    for (size_t i = 0; i < m_vAssets.size(); i++) {
        if (m_vAssets[i] == hAsset) {
            m_vAssets.erase(m_vAssets.begin() + i);
            break;
        }
    }
    delete hAsset;
}
