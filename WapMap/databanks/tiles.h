#ifndef H_C_TILES
#define H_C_TILES

#include "hgeSprite.h"
#include "../cDataController.h"
#include "guichan/listmodel.hpp"

/*struct cLoadableTile
{
 cFile hFile;
 int iID;
 std::string strSet, strFilename;
};*/

namespace WWD
{
 class Parser;
}

class cTilesetTexture;
class cBankTile;
class cTileImageSet;
class cBrush;

class cTile : public cAsset, public cImage {
 protected:
  cTileImageSet * hTS;
  int iID;
  cTilesetTexture * myTexture;

  cTile(cImageInfo inf, int id, cTileImageSet * ts, cBankTile * bank);
  ~cTile();
 friend class cTileImageSet;
 friend class cBankTile;
 public:
  int GetID(){ return iID; };

  virtual void Load();
  virtual void Unload();
  virtual std::string GetMountPoint();
};

class cTileImageSet {
 private:
  char * m_szName;
  int m_iTileMaxID;
  WWD::Parser * m_hParser;
  std::string strHash;

  std::vector<cTile*> m_vTiles;
  std::vector<cBrush*> m_vBrushes;
 friend class cBankTile;
 public:
  cTileImageSet(const char * pszName);
  ~cTileImageSet();

  void AddTile(cTile * t);
  void DeleteTile(cTile * t);

  void Sort();
  cTile * GetTile(short piID);
  cTile * GetTileByIterator(int piIt){ if( piIt < 0 || piIt >= m_vTiles.size() ) return NULL; return m_vTiles[piIt]; };
  int GetTilesCount(){ return m_vTiles.size(); };
  int GetBrushesCount(){ return m_vBrushes.size(); };
  int GetMaxTileID(){ return m_iTileMaxID; };
  cBrush * GetBrushByIterator(int it){ return m_vBrushes[it]; };
  const char * GetName(){ return m_szName; };
  void UpdateHash();
  std::string GetHash(){ return strHash; };
};

class cTilesetTexture
{
 private:
  HTEXTURE hTex;
  hgeSprite ** hSlots;
  int iLastSlotX, iLastSlotY;
  int iW, iH, iUsedSlots;
 public:
  cTilesetTexture(int w, int h);
  ~cTilesetTexture();

  DWORD * GetFreeSlot();
  void SaveLastSlot(hgeSprite * hSpr);
  void FreeSlot(hgeSprite * hSpr);
  int GetRowSpan();
  int GetFreeSlotsNum();
  bool IsEmpty(){ return iUsedSlots == 0; };
  static void CalculateDimension(int iTileNum, int &iTexW, int &iTexH);
  void GetLastSlotPos(int &x, int &y){ x = iLastSlotX; y = iLastSlotY; };
  HTEXTURE GetTexture(){ return hTex; };
};

class cBankTile : public cAssetBank, public gcn::ListModel {
 private:
  std::vector<cTilesetTexture*> vTexes;
  std::vector<cTileImageSet*> m_vhSets;
  WWD::Parser * m_hParser;
  bool bBatching;
  int iBatchPackageCount;
  bool bReloadBrushes;
  void SortTilesets();
 public:
  cBankTile(WWD::Parser * pr);
  ~cBankTile();
  cTile * GetTile(const char * pszSet, short piID);
  cTile * FindTile(short piID);
  cTileImageSet * GetSet(int piID){ return m_vhSets[piID]; };
  cTileImageSet * GetSet(const char * pszSet, bool bCaseSensitive = 1);
  int             GetSetsCount(){ return m_vhSets.size(); };

  void AddTileset(cTileImageSet * ptr){ m_vhSets.push_back(ptr); };
  void ReloadBrushes();

  virtual void DeleteAsset(cAsset * hAsset);
  virtual std::string GetFolderName(){ return "TILES"; };

  virtual void BatchProcessStart(cDataController * hDC);
  virtual void BatchProcessEnd(cDataController * hDC);

  virtual std::string GetMountPointForFile(std::string strFilePath, std::string strPrefix);
  virtual cAsset * AllocateAssetForMountPoint(cDataController * hDC, cDC_MountEntry mountEntry);

  virtual std::string getElementAt(int i);
  virtual int getNumberOfElements();
};

void Tileset_FlushBlacklist();

#endif
