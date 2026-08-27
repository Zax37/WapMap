#ifndef H_C_SPRBANK
#define H_C_SPRBANK

#include <vector>
#include "hgesprite.h"
#include "../shared/cWWD.h"

#include "../cDataController.h"
#include "../cTextureAtlas.h"

#define SPRBANK_TEXJUMPVIOLATION 0
#define SPRBANK_TEXLOCKVIOLATION 1

class cBankImageSet;

class cSprBankAsset;

class cSprBankAssetIMG : public cAsset, public cImage {
private:
    cSprBankAsset *hIS;
    int m_iID;
    int m_iIT;

    cSprBankAssetIMG(cFile file, int it, cBankImageSet *par, cSprBankAsset *is, int id);

    ~cSprBankAssetIMG() override;

public:
    hgeSprite *GetSprite() { return imgSprite; }

    void RenderToTexture();

    int GetID() { return m_iID; };

    int GetIt() { return m_iIT; };

    void Load() override;

    void Unload() override;

    std::string GetMountPoint();

    friend class cBankImageSet;

    friend class cSprBankAsset;
};

class cSprBankAsset : public cAsset {
protected:
    std::string strHash;
    std::vector<cSprBankAssetIMG *> m_vSprites;
    int m_iSize = 0;
    int m_iMaxID = 0;
    int m_iMaxWidth = 0, m_iMaxHeight = 0;

    cSprBankAsset(std::string id) { _strName = id; }

    void DeleteIMG(cSprBankAssetIMG *img);

    void AddIMG(cSprBankAssetIMG *img);

    friend class cBankImageSet;
	friend class cSprBankAssetIMG;

public:
    ~cSprBankAsset() override;

    cSprBankAssetIMG *GetIMGByIterator(int it);

    cSprBankAssetIMG *GetIMGByID(int id);

    cSprBankAssetIMG *GetIMGByNoID(int noid);

    int GetMaxIMGID() { return m_iMaxID; };

    int GetMaxIMGWidth() { return m_iMaxWidth; };

    int GetMaxIMGHeight() { return m_iMaxHeight; };

    int GetSpritesCount() { return m_vSprites.size(); };

    const char *GetID() { return _strName.c_str(); };

    int GetSize() { return m_iSize; };

    int GetIndexOf(cSprBankAssetIMG* img) {
        for (int i = 0; i < m_vSprites.size(); i++) { if (img == m_vSprites[i]) return i; }
        return -1;
    };

    std::string GetHash() { return strHash; };

    void SortAndFixIterators();

    void UpdateHash();

    void Load() override {}

    void Unload() override {}
};

class cBankImageSet : public cAssetBank<cSprBankAsset> {
private:
    cTextureAtlaser atlaser;
    friend class cSprBankAssetIMG;
public:
    explicit cBankImageSet(cDataController *hDC) : cAssetBank<cSprBankAsset>(hDC) {}

    cSprBankAsset *GetAssetByID(const char *pszID);

    void SortAssets();

    hgeSprite *GetObjectSprite(WWD::Object *obj);

    hgeSprite *GetSpriteFromAsset(const char *asset, int frame);

    WWD::Rect GetObjectRenderRect(WWD::Object *obj);

    WWD::Rect GetSpriteRenderRect(hgeSprite *spr);

    static bool canReadExtension(const char* ext);

    void RedrawAssets();

    void BatchProcessStart() override;

    void BatchProcessEnd() override;

    const std::string& GetFolderName() override {
        static const std::string name = "IMAGES";
        static const std::string namez = "IMAGEZ";
        return hDC->GetGame() == WWD::Game_Gruntz ? namez : name;
    };

    std::string GetMountPointForFile(std::string strFilePath, std::string strPrefix) override;

    cSprBankAsset *AllocateAssetForMountPoint(cDC_MountEntry mountEntry) override;

    void DeleteAsset(cSprBankAsset *hAsset) override;
};

#endif
