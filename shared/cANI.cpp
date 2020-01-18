#include "cANI.h"

#include <cstring>

namespace ANI
{
 Animation::Animation(void * data, int len)
 {
     std::istringstream src(std::string((char*)data, len), std::ios_base::binary | std::ios_base::in);
     Load(&src);
 }

 Animation::~Animation()
 {
    for(int i=0;i<m_vFrames.size();i++){
     delete m_vFrames[i];
     m_vFrames[i] = NULL;
    }
    delete [] m_szImageset;
 }

 void Animation::Load(std::istream * src)
 {
     src->seekg(0);
     byte b;
     src->RBYTE(b);
     if( b != 32 ){
      bBadBit = 1;
      return;
     }else
      bBadBit = 0;
     src->seekg(11, std::ios_base::cur);
     int framesCount;
     src->RINT(framesCount);
     int imglen;
     src->RINT(imglen);
     src->seekg(12, std::ios_base::cur);
     if( imglen == 0 )
      m_szImageset = NULL;
     else{
      m_szImageset = new char[imglen+1];
      for(int i=0;i<imglen+1;i++)
       m_szImageset[i] = '\0';
      src->RLEN(m_szImageset, imglen);
     }

     m_vFrames.resize(framesCount);
     for(int i=0;i<framesCount;i++){
      Frame * fr = new Frame();
      src->RLEN(&(fr->m_iFlag), 2);
      src->seekg(6, std::ios_base::cur);
      src->RLEN(&(fr->m_iImageID), 2);
      src->RLEN(&(fr->m_iDuration), 2);
      src->seekg(8, std::ios_base::cur);
      if( fr->m_iFlag & Flag_KeyFrame ){
       char tmp[256];
       int off = 0;
       while(1){
        src->RLEN(&(tmp[off]), 1);
        if( tmp[off] == '\0' )
         break;
        off++;
       }
       fr->m_szKeyFrameID = new char[strlen(tmp)+1];
       strcpy(fr->m_szKeyFrameID, tmp);
      }else{
       fr->m_szKeyFrameID = NULL;
      }
      /*if( fr->m_iFlag & Flag_KeyFrame )
       GV->Console->Printf("frame id: %d, duration %d, key %s\n", fr->m_iImageID, fr->m_iDuration, fr->m_szKeyFrameID);
      else
       GV->Console->Printf("frame id: %d, duration %d\n", fr->m_iImageID, fr->m_iDuration);*/
      m_vFrames[i] = fr;
     }
 }

 Frame::~Frame()
 {
     delete [] m_szKeyFrameID;
 }
}
