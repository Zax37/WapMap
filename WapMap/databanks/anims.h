#ifndef H_C_ANIBANK
#define H_C_ANIBANK

#include <vector>
#include <hge.h>
#include "../shared/cREZ.h"
#include "../shared/cANI.h"
#include "guichan/listmodel.hpp"

class cAniBankAsset {
protected:
    char *m_szID;
    ANI::Animation *m_hAni;

    friend class cBankAni;

public:
    cAniBankAsset();

    ~cAniBankAsset();

    ANI::Animation *GetAni() { return m_hAni; };

    const char *GetID() { return (const char *) m_szID; };
};

class cBankAni : public gcn::ListModel {
private:

    std::vector<cAniBankAsset *> m_vAssets;

    char *szPath;
    REZ::Parser *hREZ;
    bool bUseREZ;
public:
    cBankAni();

    ~cBankAni();

    void LoadDirRecursive(REZ::Dir *dir, const char *pszPrefix);

    void Load(REZ::File *file, const char *pszID);

    cAniBankAsset *GetAssetByID(const char *pszID);

    cAniBankAsset *GetAssetByIterator(int iIT) {
        if (iIT < 0 || iIT >= m_vAssets.size()) return NULL;
        return m_vAssets[iIT];
    }

    int GetAssetsCount() { return m_vAssets.size(); };

    void AttachREZ(REZ::Parser *n) { hREZ = n; };

    //inherited from listmodel
    std::string getElementAt(int i);

    int getNumberOfElements();

    void SortAssets();
};

#endif
