#include "meta.h"

static const std::string prefix = "LEVEL";

cBankMeta::cBankMeta(cDataController *hDC, cAssetPackage* levelPackage, cBankImageSet *hImageBank, cBankSound *hSoundBank, cBankAni *hAniBank): cAssetBank<cMetaAsset>(hDC) {
    hPackage = levelPackage;
    hBanks.push_back((cAssetBank<cAsset>*)hAniBank);
    hBanks.push_back((cAssetBank<cAsset>*)hSoundBank);
    hBanks.push_back((cAssetBank<cAsset>*)hImageBank);
}

cMetaAsset * cBankMeta::AllocateAssetForMountPoint(cDC_MountEntry mountEntry) {
    cAssetBank<cAsset>* hBank = hBanks[mountEntry.strMountPoint.c_str()[7] - '0'];
    cDC_MountEntry metaMountEntry;
    metaMountEntry.strMountPoint = mountEntry.strMountPoint.c_str() + 8;
    metaMountEntry.vFiles = mountEntry.vFiles;
    auto metaAsset = new cMetaAsset(hBank, hBank->AllocateAssetForMountPoint(metaMountEntry));
    m_vAssets.push_back(metaAsset);
    return metaAsset;
}

std::string cBankMeta::GetMountPointForFile(std::string strFilePath, std::string strPrefix) {
    for (int i = 0; i < hBanks.size(); i++) {
        std::string mountPoint = hBanks[i]->GetMountPointForFile(strFilePath, strPrefix);
        if (!mountPoint.empty()) {
            mountPoint.replace(mountPoint.find(strPrefix), strPrefix.length(), prefix);
            return "/" + prefix + "/" + std::to_string(i) + mountPoint.c_str();
        }
    }

    return "";
}

void cBankMeta::BatchProcessEnd() {
    for (auto & m_vAsset : m_vAssets) {
        m_vAsset->hAsset->SetPackage(hPackage);
        m_vAsset->Load();
    }
}

const std::string & cBankMeta::GetFolderName() {
    return prefix;
}

void cBankMeta::DeleteAsset(cMetaAsset *hAsset) {
    for (size_t i = 0; i < m_vAssets.size(); i++) {
        if (m_vAssets[i] == hAsset) {
            m_vAssets.erase(m_vAssets.begin() + i);
            break;
        }
    }
    delete hAsset;
}
