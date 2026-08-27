#include "palettes.h"
#include "../globals.h"
#include "../states/editing_ww.h"

void cBankPalettes::BatchProcessEnd() {
    for (int i = m_vAssets.size() - 1; i >= 0; i--) {
        const std::string& n = m_vAssets[i]->_strName;
        if (n.size() >= 8 && n.compare(n.size() - 8, 8, "MAIN.PAL") == 0) {
            hDC->SetPalette(m_vAssets[i]->GetPalette());
            return;
        }
    }
}

std::string cBankPalettes::GetMountPointForFile(std::string strFilePath, std::string strPrefix) {
    const char* ext = strFilePath.c_str() + strFilePath.length() - 3;
    if (!(
        (ext[0] == 'p' || ext[0] == 'P')
        && (ext[1] == 'a' || ext[1] == 'A')
        && (ext[2] == 'l' || ext[2] == 'L')
    )) {
        return "";
    }

    return "/" + GetFolderName() + "/" + strPrefix + "/" + strFilePath;
}

cPalette * cBankPalettes::AllocateAssetForMountPoint(cDC_MountEntry mountEntry) {
    unsigned char *data;
    unsigned int len;
    data = mountEntry.vFiles[0].hFeed->GetFileContent(mountEntry.vFiles[0].strPath.c_str(), len);
    if (len != 768) {
        delete[] data;
        return 0;
    }
    auto hPalette = new PID::Palette(data, len);
    auto hcPalette = new cPalette(hPalette, mountEntry.strMountPoint);
    m_vAssets.push_back(hcPalette);
    delete[] data;
    return hcPalette;
}

void cBankPalettes::DeleteAsset(cPalette *hPalette) {
    for (size_t i = 0; i < m_vAssets.size(); i++) {
        if (m_vAssets[i] == hPalette) {
            m_vAssets.erase(m_vAssets.begin() + i);
            break;
        }
    }

    if (hPalette->GetPalette() == hDC->GetPalette()) {
        hDC->SetPalette(m_vAssets.back()->GetPalette());
    }

    delete hPalette;
}

int cBankPalettes::getSelectedPaletteIndex() {
    PID::Palette *palette = hDC->GetPalette();
    for (size_t i = 0; i < m_vAssets.size(); i++) {
        if (m_vAssets[i]->GetPalette() == palette) {
            return i;
        }
    }
    return -1;
}

void cBankPalettes::setSelectedPaletteIndex(int i) {
    PID::Palette *palette = m_vAssets[i]->GetPalette();
    if (hDC->GetPalette() == palette) return;

    hDC->SetPalette(palette);
    imagesBank->RedrawAssets();
    tilesBank->RedrawAssets();
    GV->editState->vPort->MarkToRedraw();
}
