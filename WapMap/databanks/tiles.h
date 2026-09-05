#ifndef H_C_TILES
#define H_C_TILES

#include "hgesprite.h"
#include "../cDataController.h"

namespace WWD {
    class Parser;
}

class cTilesetTexture;

class cBankTile;

class cTileImageSet;

class cBrush;

class cTile : public cAsset, public cImage {
protected:
    cTileImageSet *hTS;
    int iID;
    cTilesetTexture *myTexture;

    cTile(cImageInfo inf, int id, cTileImageSet *ts, cBankTile *bank);

    ~cTile() override;

    friend class cTileImageSet;

    friend class cBankTile;

public:
    int GetID() { return iID; }

    void Load() override;

    void Unload() override;

    std::string GetMountPoint();
};

class cTileImageSet : public cAsset {
private:
    char *m_szName;
    int m_iTileMaxID;
    WWD::Parser *m_hParser;
    std::string strHash;
    int tileWidth, tileHeight;

    std::vector<cTile *> m_vTiles;
    std::vector<cBrush *> m_vBrushes;

    bool isActionTileset;

    friend class cBankTile;

public:
    cTileImageSet(int tileWidth, int tileHeight, const char *pszName);

    ~cTileImageSet() override;

    void AddTile(cTile *t);

    void DeleteTile(cTile *t);

    void Sort();

    cTile *GetTile(short piID);

    cTile *GetTileByIterator(int piIt) {
        if (piIt < 0 || piIt >= m_vTiles.size()) return NULL;
        return m_vTiles[piIt];
    }

    int GetTilesCount() { return m_vTiles.size(); }

    int GetBrushesCount() { return m_vBrushes.size(); }

    int GetMaxTileID() { return m_iTileMaxID; }

    cBrush *GetBrushByIterator(int it) { return m_vBrushes[it]; }

    const char *GetName() { return m_szName; }

    void UpdateHash();

    std::string GetHash() { return strHash; }

    void Load() override {}

    void Unload() override {}
};

class cTilesetTexture {
private:
    HTEXTURE hTex;
    hgeSprite **hSlots;
    int iLastSlotX, iLastSlotY;
    int iW, iH, iTW, iTH, iUsedSlots;
public:
    cTilesetTexture(int w, int h, int tw, int th);

    ~cTilesetTexture();

    DWORD *GetFreeSlot();

    void SaveLastSlot(hgeSprite *hSpr);

    void FreeSlot(hgeSprite *hSpr);

    int GetRowSpan();

    int GetFreeSlotsNum();

    bool IsEmpty() { return iUsedSlots == 0; }

    static void CalculateDimension(int iTileNum, int tileW, int tileH, int &iTexW, int &iTexH);

    void GetLastSlotPos(int &x, int &y) {
        x = iLastSlotX;
        y = iLastSlotY;
    }

    HTEXTURE GetTexture() { return hTex; }

    friend class cTile;
};

class cBankTile : public cAssetBank<cTileImageSet> {
private:
    std::vector<cTilesetTexture *> vTexes;
    bool bReloadBrushes = false;

    void SortTilesets();

public:
    explicit cBankTile(cDataController *hDC) : cAssetBank<cTileImageSet>(hDC) {}

    cTile *GetTile(const char *pszSet, short piID);

    cTile *FindTile(short piID);

    cTileImageSet *GetSet(int piID) { return m_vAssets[piID]; }

    cTileImageSet *GetSet(const char *pszSet, bool bCaseSensitive = 1);

    int GetSetsCount() { return m_vAssets.size(); }

    void AddTileset(cTileImageSet *ptr) { m_vAssets.push_back(ptr); }

    void ReloadBrushes();

    void DeleteAsset(cTileImageSet *hAsset) override;

    const std::string& GetFolderName() override {
        static const std::string name = "TILES";
        static const std::string namez = "TILEZ";
        return hDC->GetGame() == WWD::Game_Gruntz ? namez : name;
    }

    void BatchProcessStart() override;

    void BatchProcessEnd() override;

    std::string GetMountPointForFile(std::string strFilePath, std::string strPrefix) override;

    void RedrawAssets();

    cTileImageSet *AllocateAssetForMountPoint(cDC_MountEntry mountEntry) override;
};

#endif
