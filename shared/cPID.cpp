#include "cPID.h"

#include <fstream>
#include <sstream>

#ifndef WLEN
#define WLEN(data, len) write((char*)(&data), len)
#endif

PID::Palette::Palette()
{
    for(int i=0;i<256;i++){
     m_iColors[i][0] = i;
     m_iColors[i][1] = i;
     m_iColors[i][2] = i;
    }
}

PID::Palette::Palette(const char * pszFilename)
{
    std::ifstream isSource(pszFilename, std::ios_base::binary | std::ios_base::in);
    Load(&isSource);
    isSource.close();
}

PID::Palette::Palette(std::istream * pisSource)
{
    Load(pisSource);
}

PID::Palette::Palette(const void * phMem, int iLen)
{
    std::istringstream isSource(std::string((char*)phMem, iLen), std::ios_base::binary | std::ios_base::in);
    Load(&isSource);
}

void PID::Palette::Load(std::istream * pisSource)
{
    pisSource->RLEN(&m_iColors, 768);
}

void PID::Palette::SetColorRGB(int i, byte r, byte g, byte b)
{
    m_iColors[i][0] = r;
    m_iColors[i][1] = g;
    m_iColors[i][2] = b;
}

PID::HeaderInfo::HeaderInfo()
{
    m_iW = m_iH = m_iID = 0;
    m_iFlags = (PID::FLAGS)0;
    for(int i=0;i<4;i++)
     m_iU[i] = 0;
}

PID::HeaderInfo::HeaderInfo(const void * phMem)
{
    std::istringstream pisSource(std::string((char*)phMem, 32));
    pisSource.RINT(m_iID);
    pisSource.RINT(m_iFlags);
    pisSource.RINT(m_iW);
    pisSource.RINT(m_iH);
    pisSource.RLEN(m_iU, 16);
}

PID::Image::Image(HGE * phHGE)
{
    hHGE = phHGE;
    m_bForceFlare = 0;
    m_bInited = 0;
    //m_iPixels = NULL;
    m_hPal = NULL;
    iColorKeyType = Colorkey_None;
}

PID::Image::Image(int w, int h)
{
    hHGE = 0;
    m_bForceFlare = 0;
    iColorKeyType = Colorkey_None;
    m_hPal = 0;
    m_bDeletePal = 0;
    m_bInited = 1;
    m_iW = w;
    m_iH = h;
    m_iID = 0;
    for(int i=0;i<4;i++) m_iU[i] = 0;
    m_iFlags = (FLAGS)0;
    m_iData = new byte[m_iW*m_iH];
}

PID::Image::Image(const char * pszFilename, Palette * phPal, bool pbForceUsing, bool pbCanDelete, HGE * phHGE, bool pbForceTransparency)
{
    hHGE = phHGE;
    m_bForceFlare = 0;
    iColorKeyType = Colorkey_None;
    LoadFile(pszFilename, phPal, pbForceUsing, pbCanDelete, pbForceTransparency);
}

PID::Image::Image(const void * phMem, int iLen, Palette * phPal, bool pbForceUsing, bool pbCanDelete, HGE * phHGE, bool pbForceTransparency)
{
    hHGE = phHGE;
    m_bForceFlare = 0;
    iColorKeyType = Colorkey_None;
    LoadMemory(phMem, iLen, phPal, pbForceUsing, pbCanDelete, pbForceTransparency);
}

void PID::Image::LoadFile(const char * pszFilename, Palette * phPal, bool pbForceUsing, bool pbCanDelete, bool pbForceTransparency)
{
    m_hPal = phPal;
    m_bDeletePal = pbCanDelete;
    std::ifstream isSource(pszFilename, std::ios_base::binary | std::ios_base::in);
    Load(&isSource, pbForceUsing, pbForceTransparency);
    isSource.close();
    m_bInited = 1;
}

void PID::Image::LoadMemory(const void * phMem, int iLen, Palette * phPal, bool pbForceUsing, bool pbCanDelete, bool pbForceTransparency)
{
    m_hPal = phPal;
    m_bDeletePal = pbCanDelete;
    std::istringstream isSource(std::string((char*)phMem, iLen), std::ios_base::binary | std::ios_base::in);
    Load(&isSource, pbForceUsing, pbForceTransparency);
    m_bInited = 1;
}

byte * PID::Image::GetRawData(int * oiSize)
{
    *oiSize = m_iW*m_iH;
    return m_iData;
}

void PID::Image::Save(const char * szOut)
{
    std::ofstream os(szOut, std::ios_base::binary | std::ios_base::out);
    std::ofstream * osOut = &os;
    osOut->WLEN(m_iID, 4);
    osOut->WLEN(m_iFlags, 4);
    osOut->WLEN(m_iW, 4);
    osOut->WLEN(m_iH, 4);
    osOut->WLEN(m_iU, 16);

    int len = m_iW*m_iH;
    if( (m_iFlags & Flag_Compression) ){
     int pos = 0;
     while(1){
      if( m_iData[pos] == 0 ){
       int spacingnum = 1;
       for(int i=pos+1;i<len;i++){
        if( spacingnum >= 127 || m_iData[i] != 0 || i%m_iW == 0 ) break;
        spacingnum++;
       }
       byte b = spacingnum+128;
       osOut->WLEN(b, 1);
       pos += spacingnum;
      }else{
       byte buf[128];
       int buflen = 0;
       for(int i=pos;i<=pos+128&&i<len;i++){
        if( m_iData[i] == 0 ){
         break;
        }else{
         buf[buflen] = m_iData[i];
         buflen++;
        }
       }
       byte blen = buflen;
       osOut->WLEN(blen, 1);
       for(int i=0;i<buflen;i++)
        osOut->WLEN(buf[i], 1);
       pos += buflen;
      }
      if( pos >= len ) break;
     }
    }else{
     int pos = 0;
     while(1){
      int num = 1;
      for(int i=pos+1;i<pos+63&&i<len;i++){
       if( m_iData[i] == m_iData[pos] ){
        num++;
       }else{
        break;
       }
      }
      if( num == 1 ){
       byte b;
       if( m_iData[pos] >= 192 ){
        b = 193;
        osOut->WLEN(b, 1);
        b = m_iData[pos];
        osOut->WLEN(b, 1);
       }else{
        b = m_iData[pos];
        osOut->WLEN(b, 1);
       }
      }else{
       byte b = num+192;
       osOut->WLEN(b, 1);
       b = m_iData[pos];
       osOut->WLEN(b, 1);
      }
      pos += num;
      if( pos >= len ) break;
     }
    }
    os.close();
}

void PID::Image::Load(std::istream * pisSource, bool pbForceUsing, bool pbForceTransparency)
{
    pisSource->RINT(m_iID);
    pisSource->RINT(m_iFlags);
    pisSource->RINT(m_iW);
    pisSource->RINT(m_iH);
    pisSource->RLEN(m_iU, 16);

    if( !(m_iFlags & Flag_Transparency) && pbForceTransparency ){
     int i = m_iFlags + Flag_Transparency;
     m_iFlags = (FLAGS)i;
    }

    if( (m_iFlags & Flag_OwnPalette) && !pbForceUsing ){
     pisSource->seekg(-768, std::ios_base::end);
     m_bDeletePal = 1;
     m_hPal = new Palette(pisSource);
     pisSource->seekg(32);
    }

    m_iData = new byte[m_iW*m_iH];
    ZeroMemory(m_iData, m_iW*m_iH);
    //if( m_hPal != NULL )
     //m_iPixels = new byte[m_iW*m_iH*4];

    int x = 0, y = 0, end = m_iW*m_iH;
    if( (m_iFlags & Flag_Compression) ){
     while(1){
     BYTE num, pixel;
     pisSource->RBYTE(num);
     if( num > 128 ){
      int spacing = num - 128;
      for(int iter=0;iter<spacing;iter++){
       m_iData[y*m_iW+x] = 0;
       x++;
       if( x == m_iW )
        break;
      }
      if( x == m_iW ){
       x = 0;
       y++;
      }
      if( y >= m_iH ) break;
      continue;
     }
     for(int i=0;i<num;i++){
      pisSource->RBYTE(pixel);
      m_iData[y*m_iW+x] = pixel;
      x++;
      if( x == m_iW ){
       x = 0;
       y++;
      }
      if( y >= m_iH ) break;
     }
     if( y >= m_iH ) break;
     }
    }else{
     while(1){
      byte num, pixel;
      pisSource->RBYTE(pixel);
      if( pixel > 192 ){
       num = pixel - 192;
       pisSource->RBYTE(pixel);
       m_iData[y*m_iW+x] = pixel;
      }else{
       num = 1;
      }
      for(int i=0;i<num;i++){
       m_iData[y*m_iW+x] = pixel;
       x++;
       if( x == m_iW ){
        x = 0;
        y++;
       }
       if( y >= m_iH ) break;
      }
      if( y >= m_iH ) break;
     }
    }
}

void PID::Image::AssignPalette(Palette * phPal, bool pbCanDelete)
{
    if( m_bDeletePal ){
     delete m_hPal;
    }
    m_hPal = phPal;
    m_bDeletePal = pbCanDelete;
}

PID::Image::~Image()
{
    CleanUp();
}

void PID::Image::CleanUp()
{
    /*if( m_iPixels != NULL ){
     delete [] m_iPixels;
     m_iPixels = NULL;
    }*/
    delete [] m_iData;

    if( m_hPal != NULL && m_bDeletePal ){
     delete m_hPal;
     m_hPal = NULL;
    }
}

void PID::Image::RenderTo(DWORD * dest, int iTexW, int piOffX, int piOffY)
{
    if( iTexW == 0 )
     iTexW = m_iW;

    for(int y=0;y<m_iH;y++){
     for(int x=0;x<m_iW;x++){
      if( iColorKeyType == Colorkey_None && m_iData[y*m_iW+x] == 0 ||
          iColorKeyType == Colorkey_ByPalette && m_iData[y*m_iW+x] == dwColorKey ||
          (iColorKeyType == Colorkey_ByColor &&
           (m_hPal == NULL && dwColorKey == ARGB(255, m_iData[y*m_iW+x], m_iData[y*m_iW+x], m_iData[y*m_iW+x]) ||
            m_hPal != NULL && dwColorKey == m_hPal->GetColor(m_iData[y*m_iW+x]))) )
       dest[(y+piOffY)*iTexW+x+piOffX] = 0x00FFFFFF;
      else{
       if( (m_iFlags & Flag_Lights) || m_bForceFlare ){
        dest[(y+piOffY)*iTexW+x+piOffX] = ARGB(m_iData[y*m_iW+x], 255, 255, 255);
       }else{
        if( m_hPal == NULL )
         dest[(y+piOffY)*iTexW+x+piOffX] = ARGB(255, m_iData[y*m_iW+x], m_iData[y*m_iW+x], m_iData[y*m_iW+x]);
        else
         dest[(y+piOffY)*iTexW+x+piOffX] = m_hPal->GetColor(m_iData[y*m_iW+x]);
       }
      }
     }
    }

    /*for(int i=0;i<m_iW*m_iH;i++){
     dest[i] = m_hPal->GetColor(m_iData[i]);
    }*/
}

DWORD PID::Image::GetColorAt(int x, int y)
{
    if( x < 0 || y < 0 || x >= m_iW || y >= m_iH || !m_bInited || !m_hPal ) return 0;
    return m_hPal->GetColor(m_iData[y*m_iW+x]);
}

void PID::Image::SetColorIdAt(int x, int y, byte val)
{
    if( x < 0 || y < 0 || x >= m_iW || y >= m_iH ) return;
    m_iData[y*m_iW+x] = val;
}

byte PID::Image::GetColorIdAt(int x, int y)
{
    if( x < 0 || y < 0 || x >= m_iW || y >= m_iH || !m_bInited ) return 0;
    return m_iData[y*m_iW+x];
}
