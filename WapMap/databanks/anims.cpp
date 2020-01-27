#include "anims.h"
#include "../shared/commonFunc.h"
#include "../globals.h"

extern HGE *hge;

bool cAniBank_SortAssets(cAniBankAsset *a, cAniBankAsset *b) {
    return (std::string(a->GetID()) < std::string(b->GetID()));
}

cBankAni::cBankAni() {
    hREZ = 0;
}

cBankAni::~cBankAni() {
    for (int i = 0; i < m_vAssets.size(); i++) {
        delete m_vAssets[i];
        m_vAssets[i] = 0;
    }
}

void cBankAni::LoadDirRecursive(REZ::Dir *dir, const char *pszPrefix) {
    for (int i = 0; i < dir->GetElementsCount(); i++) {
        if (dir->GetElement(i)->IsFile()) {
            REZ::File *el = (REZ::File *) dir->GetElement(i);
            char *ext = SHR::GetExtension(el->GetName());
            char *extl = SHR::ToLower(ext);
            if (!strcmp(extl, "ani")) {
                char *filename = SHR::GetFileWithoutExt(el->GetName());
                char *id = new char[strlen(filename) + strlen(pszPrefix) + 2];
                sprintf(id, "%s_%s", pszPrefix, filename);
                Load(el, id);
                delete[] id;
                delete[] filename;
            }
            delete[] extl;
            delete[] ext;
        } else if (dir->GetElement(i)->IsDir()) {
            REZ::Dir *el = (REZ::Dir *) dir->GetElement(i);
            char *prefix = new char[strlen(pszPrefix) + strlen(el->GetName()) + 2];
            sprintf(prefix, "%s_%s", pszPrefix, el->GetName());
            LoadDirRecursive(el, prefix);
            delete[] prefix;
        }
    }
}

void cBankAni::Load(REZ::File *file, const char *pszID) {
    //GV->Console->Printf("---------%s\n", pszID);
    cAniBankAsset *as = new cAniBankAsset();
    as->m_szID = new char[strlen(pszID) + 1];
    strcpy(as->m_szID, pszID);

    void *ptr;
    int len;
    ptr = file->GetData(&len);
    as->m_hAni = new ANI::Animation(ptr, len);
    delete[] ptr;
    if (!as->m_hAni->Valid()) {
        GV->Console->Printf("~r~Anim ~y~%s ~r~invalid!", pszID);
        delete as;
        return;
    }
    m_vAssets.push_back(as);
}

cAniBankAsset *cBankAni::GetAssetByID(const char *pszID) {
    for (int i = 0; i < m_vAssets.size(); i++) {
        if (!strcmp(m_vAssets[i]->m_szID, pszID)) {
            return m_vAssets[i];
        }
    }
    return NULL;
}

cAniBankAsset::cAniBankAsset() {
    m_szID = 0;
}

cAniBankAsset::~cAniBankAsset() {
    delete m_hAni;
    delete[] m_szID;
}

std::string cBankAni::getElementAt(int i) {
    if (i < 0 || i >= m_vAssets.size()) return "";
    return m_vAssets[i]->GetID();
}

int cBankAni::getNumberOfElements() {
    return m_vAssets.size();
}

void cBankAni::SortAssets() {
    sort(m_vAssets.begin(), m_vAssets.end(), cAniBank_SortAssets);
}
