#ifndef H_C_TEXTUREATLAS
#define H_C_TEXTUREATLAS

#include <hgeSprite.h>
#include <vector>

class cTextureAtlas
{
 private:
  std::vector<hgeSprite*> vSprites;
  HTEXTURE hTex;
  int dimX, dimY, carX, carY, rowH;
 public:
  cTextureAtlas();
  ~cTextureAtlas();

  bool AddSprite(hgeSprite * spr);
  bool DeleteSprite(hgeSprite * spr);
  int GetSpritesCount(){ return vSprites.size(); };
};

class cTextureAtlaser
{
 private:
  std::vector<cTextureAtlas*> vAtlases;
  std::vector<hgeSprite*> vPendingSprites;
 public:
  cTextureAtlaser();
  ~cTextureAtlaser();

  void AddSprite(hgeSprite * spr);
  void DeleteSprite(hgeSprite * spr);
  void Pack();
};

#endif // H_C_TEXTUREATLAS
