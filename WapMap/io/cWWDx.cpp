#include "cWWDx.h"
#include "../cMDI.h"

#include <fstream>
#include <sstream>

enum WWDx_BlockType
{
 BlockType_MetaInfo = 0,
 BlockType_MetaEndInfo,
 BlockType_Header,
 BlockType_Guides
};

cIO_WWDx::cIO_WWDx(DocumentData * dd)
{
    hDocumentData = dd;
}

cIO_WWDx::~cIO_WWDx()
{

}


std::string cIO_WWDx::ReadZeroTerminatedString(std::istream * hStream)
{
    std::string acc("");
     while(1){
      byte a;
      hStream->RBYTE(a);
      if( a == 0 ) break;
      acc.append(1, a);
     }
    return acc;
}

void cIO_WWDx::SerializeTo(std::iostream * hStream)
{
    int initpos = hStream->tellp();
    int metasize = 0;
    char tmpbuf[256];
    int tmpi, blocklen;

    sprintf(tmpbuf, "<wm_meta>");
    hStream->write((char*)tmpbuf, 9);
    metasize += 9;

    tmpbuf[0] = BlockType_MetaInfo;
    hStream->write((char*)tmpbuf, 1);
    tmpi = 4;
    hStream->WLEN(&tmpi, 4);
    tmpi = 0;
    hStream->WLEN(&tmpi, 4);
    metasize += 9;

    tmpbuf[0] = BlockType_Header;
    hStream->write((char*)tmpbuf, 1);
    blocklen = 4+strlen(hDocumentData->strWapMapVersion.c_str())+1+4;
    blocklen += hDocumentData->strMapVersion.length()+1;
    blocklen += hDocumentData->strMapDescription.length()+1;
    hStream->WLEN(&blocklen, 4);
    tmpi = hDocumentData->iWapMapBuild;
    hStream->WLEN(&tmpi, 4);
    strcpy(tmpbuf, hDocumentData->strWapMapVersion.c_str());
    hStream->write((char*)tmpbuf, strlen(hDocumentData->strWapMapVersion.c_str())+1);

    hStream->WLEN(&(hDocumentData->iMapBuild), 4);

    int strlen = hDocumentData->strMapVersion.length();
    char * varbuf = new char[strlen+1];
    strcpy(varbuf, hDocumentData->strMapVersion.c_str());
    hStream->WLEN(varbuf, strlen+1);
    delete [] varbuf;

    strlen = hDocumentData->strMapDescription.length();
    varbuf = new char[strlen+1];
    strcpy(varbuf, hDocumentData->strMapDescription.c_str());
    hStream->WLEN(varbuf, strlen+1);
    delete [] varbuf;
    metasize += 5+blocklen;

    if( hDocumentData->vGuides.size() > 0 ){
     tmpbuf[0] = BlockType_Guides;
     hStream->write((char*)tmpbuf, 1);
     blocklen = 4+5*hDocumentData->vGuides.size();
     hStream->WLEN(&blocklen, 4);

     tmpi = hDocumentData->vGuides.size();
     hStream->WLEN(&tmpi, 4);
     for(size_t i=0;i<hDocumentData->vGuides.size();i++){
      tmpbuf[0] = hDocumentData->vGuides[i].bOrient;
      tmpi = hDocumentData->vGuides[i].iPos;
      hStream->write((char*)tmpbuf, 1);
      hStream->WLEN(&tmpi, 4);
     }

     metasize += 5+blocklen;
    }

    tmpbuf[0] = BlockType_MetaEndInfo;
    hStream->write((char*)tmpbuf, 1);
    tmpi = 4;
    hStream->WLEN(&tmpi, 4);
    tmpi = 0;
    hStream->WLEN(&tmpi, 4);
    metasize += 9;

    sprintf(tmpbuf, "</wm_meta>");
    hStream->write((char*)tmpbuf, 10);
    metasize += 10;

    hStream->seekp(initpos+9+5, std::ios_base::beg);
    hStream->WLEN(&metasize, 4);
    hStream->seekp(initpos+metasize-10-4, std::ios_base::beg);
    hStream->WLEN(&metasize, 4);
}

void cIO_WWDx::DeserializeFrom(std::istream * hStream)
{
    char tmpbuf[256];
    for(int i=0;i<256;i++) tmpbuf[i] = 0;
    hStream->seekg(-10, std::ios_base::cur);
    hStream->RLEN(tmpbuf, 10);
    if( strcmp(tmpbuf, "</wm_meta>") != 0 ) return;
    hStream->seekg(-14, std::ios_base::cur);
    int metasize;
    hStream->RINT(metasize);
    hStream->seekg(10-metasize, std::ios_base::cur);
    if( metasize < 19+9+9 ) return;

    for(int i=0;i<256;i++) tmpbuf[i] = 0;
    hStream->RLEN(tmpbuf, 9);
    if( strcmp(tmpbuf, "<wm_meta>") != 0 ) return;

    while(1){
     byte b;
     hStream->RBYTE(b);
     WWDx_BlockType blocktype = (WWDx_BlockType)b;
     int blocklen;
     hStream->RINT(blocklen);
     int datastart = hStream->tellg();

     if( blocktype == BlockType_MetaEndInfo ){
      return;
     }else if( blocktype == BlockType_Header ){
      hStream->RINT(hDocumentData->iWapMapBuild);
      hDocumentData->strWapMapVersion = ReadZeroTerminatedString(hStream);
      hStream->RINT(hDocumentData->iMapBuild);
      hDocumentData->strMapVersion = ReadZeroTerminatedString(hStream);
      hDocumentData->strMapDescription = ReadZeroTerminatedString(hStream);
     }else if( blocktype == BlockType_Guides ){
      int guidecount;
      hStream->RINT(guidecount);
      for(int i=0;i<guidecount;i++){
       byte a;
       hStream->RBYTE(a);
       int pos;
       hStream->RINT(pos);
       stGuideLine ng;
       ng.iPos = pos;
       ng.bOrient = a;
       hDocumentData->vGuides.push_back(ng);
      }
     }

     int actpos = hStream->tellg();
     if( actpos != datastart+blocklen )
      hStream->seekg(datastart+blocklen-actpos, std::ios_base::cur);
    }
}
