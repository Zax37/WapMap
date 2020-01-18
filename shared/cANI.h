#ifndef SHR_H_ANI
#define SHR_H_ANI

#include <vector>
#include <sstream>

#define RINT(i) read((char*)(&i), 4)
#define RBYTE(b) read((char*)(&b), 1)
#define RLEN(data, len) read((char*)(data), len)

#ifndef byte
typedef unsigned char byte;
#endif

namespace ANI {

 enum FLAGS {
  Flag_None = 0,
  Flag_KeyFrame = 2
 };

 class Frame {
  private:
   FLAGS m_iFlag;
   short m_iIt;
   short m_iImageID;
   short m_iDuration;
   char * m_szKeyFrameID;
   ~Frame();
  friend class Animation;
  public:
   FLAGS GetFlags(){ return m_iFlag; };
   short GetIterator(){ return m_iIt; };
   short GetImageID(){ return m_iImageID; };
   short GetDuration(){ return m_iDuration; };
   const char * GetKeyFrameName(){ return m_szKeyFrameID; };
 };

 class Animation {
  private:
   std::vector<Frame*> m_vFrames;
   char * m_szImageset;
   void Load(std::istream  * src);
   bool bBadBit;
  public:
   Animation(void * data, int len);
   ~Animation();
   int GetFramesCount(){ return m_vFrames.size(); };
   Frame * GetFrame(int id){ return m_vFrames[id]; };
   bool Valid(){ return !bBadBit; };
   const char * GetImageset(){ return (const char*)m_szImageset; };
 };
}

#endif
/**
naglowek: 32 bajtow + char
0b: ID (dec 32 hex 20) [int]
4b: ? [int]
8b: ? [int]
12b: ilosc klatek [int]
16b: dlugosc adresu do klatek [int]
20b: ? [int]
24b: ? [int]
28b: ? [int]
32b: adres do klatek [char()]

-klatka [20b]:
0b: [short] //flaga!
 2b: [short]
 4b: [int]
 6b: obrazek (frame_xxx) [short]
 8b: czas trwania (ms) [short]
 10b: [int]
 14b: [int]
jesli flaga 2:
 18b: [char null terminated]
**/
