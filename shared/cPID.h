#ifndef PID_HEADER
#define PID_HEADER
#define PID_V 2

#include <iostream>
#include <hge.h>
#include <hgeSprite.h>

#ifndef byte
typedef unsigned char byte;
#endif

#define RINT(i) read((char*)(&i), 4)
#define RBYTE(b) read((char*)(&b), 1)
#define RLEN(data, len) read((char*)(data), len)

namespace PID {

 enum FLAGS {
  Flag_Transparency = 1,
  Flag_VideoMemory = 2,
  Flag_SystemMemory = 4,
  Flag_Mirror = 8,
  Flag_Invert = 16,
  Flag_Compression = 32,
  Flag_Lights = 64,
  Flag_OwnPalette = 128
 };

 enum COLORKEY {
  Colorkey_None = 0,
  Colorkey_ByColor = 1,
  Colorkey_ByPalette = 2
 };

 class Palette {
  private:
   byte m_iColors[256][3];
   void Load(std::istream * pisSource);
   Palette(std::istream * pisSource);
  friend class Image;
  public:
   Palette();
   Palette(const char * pszFilename);
   Palette(const void * phMem, int iLen);
   DWORD GetColor(int piID){ return ARGB(255, m_iColors[piID][0], m_iColors[piID][1], m_iColors[piID][2]); };
   byte GetColorR(int piID){ return m_iColors[piID][0]; };
   byte GetColorG(int piID){ return m_iColors[piID][1]; };
   byte GetColorB(int piID){ return m_iColors[piID][2]; };
   void SetColorRGB(int i, byte r, byte g, byte b);
 };

 class HeaderInfo {
  private:
   int m_iW, m_iH, m_iID, m_iU[4];
   FLAGS m_iFlags;

  friend class Image;
  public:
   HeaderInfo();
   HeaderInfo(const void * phMem);
   int GetWidth(){ return m_iW; };
   int GetHeight(){ return m_iH; };
   int GetID(){ return m_iID; };
   FLAGS GetFlags(){ return m_iFlags; };
   int GetOffsetX(){ return m_iU[0]; };
   int GetOffsetY(){ return m_iU[1]; };
   int GetUserValue(int i){ return m_iU[i]; };

   void SetFlags(FLAGS n){ m_iFlags = n; };
   void SetID(int n){ m_iID = n; };
   void SetOffset(int x, int y){ m_iU[0] = x; m_iU[1] = y; };
   void SetUserValue(int i, int v){ m_iU[i] = v; };
 };

 class Image : public HeaderInfo {
  private:
   bool m_bInited, m_bFile, m_bDeletePal, m_bForceFlare;

   byte * m_iData;

   Palette * m_hPal;
   std::istream * isSource;

   COLORKEY iColorKeyType;
   DWORD dwColorKey;

   void CleanUp();
   void Load(std::istream * pisSource, bool pbForceUsing, bool pbForceTransparency);
   HGE * hHGE;
  public:
   Image(int w, int h);
   Image(HGE * phHGE = NULL);
   Image(const char * pszFilename, Palette * phPal = 0, bool pbForceUsing = 0, bool pbCanDelete = 0, HGE * phHGE = NULL, bool pbForceTransparency = 0);
   Image(const void * phMem, int iLen, Palette * phPal = 0, bool pbForceUsing = 0, bool pbCanDelete = 0, HGE * phHGE = NULL, bool pbForceTransparency = 0);
   ~Image();

   void LoadFile(const char * pszFilename, Palette * phPal, bool pbForceUsing = 0, bool pbCanDelete = 0, bool pbForceTransparency = 0);
   void LoadMemory(const void * phMem, int iLen, Palette * phPal, bool pbForceUsing = 0, bool pbCanDelete = 0, bool pbForceTransparency = 0);

   byte * GetRawData(int * oiSize);
   void RenderTo(DWORD * dest, int iTexW = 0, int piOffX = 0, int piOffY = 0);

   void AssignPalette(Palette * phPal, bool pbCanDelete = 0);

   void SetColorKey(byte r, byte g, byte b){ SetColorKey(ARGB(255, r, g, b)); }
   void SetColorKey(DWORD col){ iColorKeyType = Colorkey_ByColor; dwColorKey = col; }
   void SetColorKeyByPalette(byte id){ iColorKeyType = Colorkey_ByPalette; dwColorKey = id; };

   void ForceFlare(bool b){ m_bForceFlare = b; };

   void SetColorIdAt(int x, int y, byte val);
   DWORD GetColorAt(int x, int y);
   byte GetColorIdAt(int x, int y);

   void Save(const char * szOut);
 };
};

#endif
