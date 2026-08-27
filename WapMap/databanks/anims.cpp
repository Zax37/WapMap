#include "anims.h"
#include "../shared/commonFunc.h"
#include "../globals.h"
#include "../../shared/cProgressInfo.h"
#include "../cParallelLoop.h"

extern structProgressInfo _ghProgressInfo;
extern HGE *hge;

bool cAniBank_SortAssets(cAniBankAsset *a, cAniBankAsset *b) {
    return a->_strName < b->_strName;
}

void cAniBankAsset::Load() {
    unsigned int len;
    unsigned char *ptr = GetFile().hFeed->GetFileContent(GetFile().strPath.c_str(), len);
    if (len == 0) return;
    m_hAni = new ANI::Animation(ptr, len);
    delete[] ptr;
    _bLoaded = 1;
}

void cAniBankAsset::Unload() {
    delete m_hAni;
    _bLoaded = 0;
}

cAniBankAsset *cBankAni::GetAssetByID(const char *pszID) {
    for (auto & m_vAsset : m_vAssets) {
        if (!strcmp(m_vAsset->_strName.c_str(), pszID)) {
            return m_vAsset;
        }
    }
    return NULL;
}

cAniBankAsset::cAniBankAsset(cFile hFile, std::string id) {
    SetFile(hFile);
    _strName = id;
}

cAniBankAsset::~cAniBankAsset() {
    if (_bLoaded) {
        delete m_hAni;
    }
}

void cBankAni::SortAssets() {
    sort(m_vAssets.begin(), m_vAssets.end(), cAniBank_SortAssets);
}
void cBankAni::BatchProcessStart() {
    // bBatchProcessing = 1;
    GV->Console->Printf("Loading animations...");
    _ghProgressInfo.iGlobalProgress = 7;
    _ghProgressInfo.strGlobalCaption = "Loading animations...";
    _ghProgressInfo.strDetailedCaption = "Scanning animations...";
    _ghProgressInfo.iDetailedProgress = 0;
    _ghProgressInfo.iDetailedEnd = 100000;
    // iBatchPackageCount = 0;
}

void cBankAni::BatchProcessEnd() {
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
    // bBatchProcessing = false;
}

cAniBankAsset * cBankAni::AllocateAssetForMountPoint(cDC_MountEntry mountEntry) {
    std::string aniMountPoint(mountEntry.strMountPoint.c_str() + 6);
    aniMountPoint.resize(aniMountPoint.length() - 4);

    std::transform(aniMountPoint.begin(), aniMountPoint.end(), aniMountPoint.begin(), ::toupper);
    do {
        const size_t slash = aniMountPoint.find('/');
        if (slash == std::string::npos) break;
        aniMountPoint[slash] = '_';
    } while (true);

    cAniBankAsset *as = GetAssetByID(aniMountPoint.c_str());
    if (!as && !mountEntry.vFiles.empty()) {
        as = new cAniBankAsset(mountEntry.vFiles[0], aniMountPoint);
        m_vAssets.push_back(as);
    }

    return as;
}

std::string cBankAni::GetMountPointForFile(std::string strFilePath, std::string strPrefix) {
    const char* ext = strFilePath.c_str() + strFilePath.length() - 3;
    if (!(
        (ext[0] == 'a' || ext[0] == 'A')
        && (ext[1] == 'n' || ext[1] == 'N')
        && (ext[2] == 'i' || ext[2] == 'I')
    )) {
        return "";
    }

    return "/" + GetFolderName() + "/" + strPrefix + "/" + strFilePath;
}

void cBankAni::DeleteAsset(cAniBankAsset *hAsset) {
    for (size_t i = 0; i < m_vAssets.size(); i++) {
        if (m_vAssets[i] == hAsset) {
            m_vAssets.erase(m_vAssets.begin() + i);
            break;
        }
    }
    delete hAsset;
}
