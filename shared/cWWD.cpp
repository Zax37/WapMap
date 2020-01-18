#include "cWWD.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <Windows.h> //for time functions
#include "cProgressInfo.h"
#include "../WapMap/cParallelLoop.h"

namespace WWD
{
 structProgressInfo * _ghProgressInfo = 0;
 cParallelLoop * _ghProgressCallback = 0;

 bool SortPlanes(WWD::Plane * a, WWD::Plane * b)
 {
     return a->GetZCoord() < b->GetZCoord();
 }
}

WWD::Parser::Parser(void * ptr, int iLen, CustomMetaSerializer * hSerializer)
{
    hMetaSerializer = hSerializer;
    //piInfo = NULL;
    m_szFile[0] = '\0';
    std::stringbuf * sb = new std::stringbuf(std::string((const char*)ptr, iLen), std::ios_base::in);
    std::istream * str = new std::istream(sb);
    if( str->fail() )
     throw WWD_EXCEPTION(Error_LoadingMemory);
    LoadFromStream(str);
    delete str;
    delete sb;
}

WWD::Parser::Parser(const char * pszFilename, CustomMetaSerializer * hSerializer)
{
    hMetaSerializer = hSerializer;
    //throw WWD_EXCEPTION(Error_Unknown);
    //piInfo = 0;
    strcpy(m_szFile, pszFilename);
    std::ifstream * str = new std::ifstream(pszFilename, std::ios_base::binary | std::ios_base::in);
    if( str->fail() )
     throw WWD_EXCEPTION(Error_OpenAccess);
    LoadFromStream(str);

    str->close();
    delete str;
}

void WWD::Parser::LoadFromStream(std::istream * psSource)
{
    /*if( piInfo != 0 ){
     piInfo->Lock();
     piInfo->SetDetailedEnd(100);
     piInfo->SetDetailedProgress(0);
     piInfo->SetDescription("Mapa: wczytywanie naglowka");
     piInfo->Unlock();
    }*/
    if( _ghProgressInfo != 0 ){
     _ghProgressInfo->iDetailedProgress = 28;
     _ghProgressInfo->strDetailedCaption = "[NAGLOWEK]";
     _ghProgressInfo->iDetailedEnd = 198;
     _ghProgressCallback->Tick();
    }
    std::istream * str = psSource;
    unsigned char sign[2];
    str->RLEN(&sign, 2);
    if( sign[0] != 244 || sign[1] != 05 )
     throw WWD_EXCEPTION(Error_BadMagicNumber);

    str->seekg(6, std::ios_base::cur);
    byte b;
    str->RBYTE(b);
    m_iFlags = (WWD::WWD_FLAGS)b;

    str->seekg(7, std::ios_base::cur);

    str->RLEN(&m_szMapName, 64);
    //char test[64] = "Wreckage Level 6 aaa";
    m_iBaseLevel = 0;
    /*char * basenumch = strstr(m_szMapName, "Level");
    if( basenumch != NULL ){
     if( !sscanf(basenumch, "Level %d", &m_iBaseLevel) ){
      if( !sscanf(basenumch, "Level - %d", &m_iBaseLevel) ){
       printf("[WWD]Warning: unable to predict base level ID from level name.\n");
       //throw WWD_EXCEPTION("Invalid map name (bad \"Level\").", WWDERR_INVALIDNAMEBAD);
      }
     }
    }*/

    str->RLEN(&m_szAuthor, 64);
    str->RLEN(&m_szDate, 64);
    str->RLEN(&m_szRezPath, 256);
    str->RLEN(&m_szTilesPath, 128);
    str->RLEN(&m_szPalPath, 128);
    str->RINT(m_iStartX);
    str->RINT(m_iStartY);
    int unk;
    str->RINT(unk);
    printf("unknown: %d\n", unk);
    int m_iPlanesCount;
    str->RINT(m_iPlanesCount);

    int iPlanesHeader = 0, iPlanesHeaderEnd = 0;
    str->RINT(iPlanesHeader);
    str->RINT(iPlanesHeaderEnd);

    for(int i=0;i<3;i++){
     unsigned int t = 0;
     str->RINT(t);
     printf("#%d: %u\n", i, t);
    }
    //printf("checksum from formula: %u\n", CalculateChecksum(str, 1524));
    //exit(1);
    str->RLEN(&m_szExePath, 128);
    for(int i=0;i<4;i++)
     str->RLEN(&m_szImageSets[i], 128);
    for(int i=0;i<4;i++)
     str->RLEN(&m_szSetsPrefixes[i], 32);

    /*if( m_iBaseLevel == 0 ){
     for(int i=0;i<4;i++){
      if( m_szImageSets[i][0] != '\0' ){
       int test = 0;
       printf("testing %s\n", m_szImageSets[i]);
       if( sscanf(m_szImageSets[i], "LEVEL%d%*s", &test) ){
        m_iGame = Game_Claw;
        m_iBaseLevel = test;
       }else if( sscanf(m_szImageSets[i], "AREA%d%*s", &test) ){
        m_iGame = Game_Gruntz;
        m_iBaseLevel = test;
       }else if( sscanf(m_szImageSets[i], "DUNGEON%d%*s", &test) ){
        m_iGame = Game_GetMedieval;
        m_iBaseLevel = test;
       }
      }
     }
     if( m_iBaseLevel == 0 ){
      //printf("[WWD]Warning: unable to predict base level and game type from imagesets.");
      m_iGame = Game_Unknown;
     }
    }*/
    {
     int test = 0;
     char pref[3];
     for(int i=0;i<3;i++)
      pref[i] = '\0';
     if( m_szTilesPath[0] != 'L' && m_szTilesPath[0] != 'A' && m_szTilesPath[0] != 'D' )
      pref[0] = m_szTilesPath[0];
     char formants[3][64];
     sprintf(formants[0], "%s%s", pref, "LEVEL%d%*s");
     sprintf(formants[1], "%s%s", pref, "AREA%d%*s");
     sprintf(formants[2], "%s%s", pref, "DUNGEON%d%*s");
     if( sscanf(m_szTilesPath, formants[0], &test) ){
      m_iGame = Game_Claw;
      m_iBaseLevel = test;
     }else if( sscanf(m_szTilesPath, formants[1], &test) ){
      m_iGame = Game_Gruntz;
      m_iBaseLevel = test;
     }else if( sscanf(m_szTilesPath, formants[2], &test) ){
      m_iGame = Game_GetMedieval;
      m_iBaseLevel = test;
     }else{
      m_iGame = Game_Unknown;
      m_iBaseLevel = 0;
     }
    }

    if( m_iFlags & Flag_w_Compress ){
     /*if( piInfo != 0 ){
      piInfo->Lock();
      piInfo->SetDetailedProgress(25);
      piInfo->SetDescription("Mapa: dekompresja");
      piInfo->Unlock();
     }*/
     if( _ghProgressInfo != 0 ){
      _ghProgressInfo->iDetailedProgress = 2*28;
      _ghProgressInfo->strDetailedCaption = "[DEKOMPRESJA]";
      _ghProgressCallback->Tick();
     }
     str->seekg(1524, std::ios_base::beg);
     std::istringstream * uncompressed;
     uncompressed = Inflate(str);
     printf("dekompresja ok\n");
     int len = uncompressed->str().size();
     char * chrbuf = new (std::nothrow) char[len];
     if( !chrbuf )
      throw WWD_EXCEPTION(Error_DecompressNotEnoughMem);
     for(int i=0;i<len;i++)
      chrbuf[i] = uncompressed->str().c_str()[i];
     str = new std::stringstream();

     psSource->seekg(0, std::ios_base::beg);
     char header[1524];
     psSource->RLEN(&header, 1524);
     ((std::iostream*)str)->WLEN(&header, 1524);
     ((std::iostream*)str)->WLEN(chrbuf, len);
     delete [] chrbuf;
     str->seekg(1524, std::ios_base::beg);
     //throw WWD_EXCEPTION("Compressed WWD are not supported.", 3);
     printf("zrobione\n");
    }

    int objcnt[m_iPlanesCount];
    int tdataoffset = 0;

    /*if( piInfo != 0 ){
     piInfo->Lock();
     piInfo->SetDetailedProgress(50);
     piInfo->SetDescription("Mapa: wczytywanie warstw");
     piInfo->Unlock();
    }*/
     if( _ghProgressInfo != 0 ){
      _ghProgressInfo->iDetailedProgress = 3*28;
      _ghProgressInfo->strDetailedCaption = "[WARSTWY]";
      _ghProgressCallback->Tick();
     }

    for(int i=0;i<m_iPlanesCount;i++){
     Plane * pl = new Plane();
     pl->m_hObjDeletionCB = NULL;
     str->seekg(8, std::ios_base::cur);
     str->RBYTE(b);
     pl->m_iFlags = (WWD::PLANE_FLAGS)b;
     str->seekg(7, std::ios_base::cur);
     str->RLEN(pl->m_szName, 64);

     if( _ghProgressInfo != 0 ){
      _ghProgressInfo->iDetailedProgress = 3*28+(float(i)/float(m_iPlanesCount)*28.0f);
      _ghProgressInfo->strDetailedCaption = "[NAGLOWEK WARSTWY]";
      _ghProgressCallback->Tick();
     }

     for(int z=0;z<i;z++){
      if( !strcmp(pl->m_szName, m_hPlanes[z]->GetName()) ){
       char ntmp[64];
       char tmp[64];
       strncpy(tmp, pl->m_szName, 60);
       int d = 2;
       while(1){
        sprintf(ntmp, "%s (%d)", tmp, d);
        bool bFound = 0;
        for(int y=0;y<i;y++){
         if( !strcmp(ntmp, m_hPlanes[y]->GetName()) )
          bFound = 1;
        }
        d++;
        if( !bFound ) break;
       }
       strcpy(pl->m_szName, ntmp);
       //printf("[WWD]: Plane name conflict for '%s'. Renamed to: '%s'.\n", m_hPlanes[z]->GetName(), ntmp);
       break;
      }
     }

     /*if( piInfo != 0 ){
      piInfo->Lock();
      if( i == 0 )
       piInfo->SetDetailedProgress(50);
      else
       piInfo->SetDetailedProgress(50+(20*((100.0f/(float(m_iPlanesCount)/float(i))/100))));

      //10 = 20
      //5 = 10

      char tmp[129];
      sprintf(tmp, "Mapa: wczytywanie warstwy %s (%d/%d)", pl->m_szName, i, m_iPlanesCount);
      piInfo->SetDescription(tmp);
      piInfo->Unlock();
     }*/

     str->RINT(pl->m_iWpx);
     str->RINT(pl->m_iHpx);
     str->RINT(pl->m_iTileW);
     str->RINT(pl->m_iTileH);
     str->RINT(pl->m_iW);
     str->RINT(pl->m_iH);
     pl->m_hTiles = new Tile * [pl->m_iW];
     for(int x=0;x<pl->m_iW;x++){
      pl->m_hTiles[x] = new Tile[pl->m_iH];
      //printf("creating %d: %p\n", x, pl->m_hTiles[x]);
     }
     int i1, i2;
     str->RINT(i1);
     str->RINT(i2);

     str->RINT(pl->m_iMoveX);
     str->RINT(pl->m_iMoveY);
     str->RINT(pl->m_iFillColor);
     if( pl->m_iFillColor < 0 ) pl->m_iFillColor = 0;
     if( pl->m_iFillColor > 255 ) pl->m_iFillColor = 255;

     str->RINT(pl->m_iSetsCount);
     str->RINT(objcnt[i]);
     if( objcnt[i] == 0 )
      pl->m_vObjects.clear();
     else
      pl->m_vObjects.reserve(objcnt[i]);

     int iImageSetCharsAddr, iObjectsAddr, iTilesAddr;
     str->RINT(iTilesAddr);
     str->RINT(iImageSetCharsAddr);
     str->RINT(iObjectsAddr);
     printf("%s: %d %d\n", pl->m_szName, i1, i2);

     str->RINT(pl->m_iZCoord);
     str->seekg(12, std::ios_base::cur);
     int setto = str->tellg();
     str->seekg(160*(m_iPlanesCount-1-i)+tdataoffset, std::ios_base::cur);
     tdataoffset += pl->m_iW*pl->m_iH*4;
     //klocki

     if( _ghProgressInfo != 0 ){
      _ghProgressInfo->strDetailedCaption = "[KLOCKI]";
      _ghProgressCallback->Tick();
     }
     for(int y=0;y<pl->m_iH;y++){
      for(int x=0;x<pl->m_iW;x++){
       byte bytes[4];
       str->RLEN(&bytes, 4);
       if( bytes[0] == 255 && bytes[1] == 255 && bytes[2] == 255 && bytes[3] == 255 ){
        pl->m_hTiles[x][y].m_bInvisible = 1;
        pl->m_hTiles[x][y].m_iID = 0;
       }else
        pl->m_hTiles[x][y].m_bInvisible = 0;

       if( bytes[0] == 238 && bytes[1] == 238 && bytes[2] == 238 && bytes[3] == 238 ){
        pl->m_hTiles[x][y].m_bFilled = 1;
        pl->m_hTiles[x][y].m_iID = 0;
       }else
        pl->m_hTiles[x][y].m_bFilled = 0;

       pl->m_hTiles[x][y].m_iID = bytes[0]+bytes[1]*256;
      }
     }

     str->seekg(setto);
     m_hPlanes.push_back(pl);
    }
    Plane * mainPlane = NULL;
    int mainplaneid = 0;
    for(int i=0;i<m_iPlanesCount;i++){
     if( m_hPlanes[i]->m_iFlags & Flag_p_MainPlane ){
      mainPlane = m_hPlanes[i];
      mainplaneid = i;
      break;
     }
    }
    if( mainPlane == NULL )
     throw WWD_EXCEPTION(Error_NoMainPlane);

    str->seekg(tdataoffset, std::ios_base::cur);

    for(int i=0;i<m_iPlanesCount;i++){
     for(int x=0;x<m_hPlanes[i]->m_iSetsCount;x++){
      char set[256];
      for(int y=0;y<256;y++){
       str->RLEN(&set[y], 1);
       if( set[y] == '\0' )
        break;
      }
      char * push = new char[strlen(set)+1];
      strcpy(push, set);
      m_hPlanes[i]->m_vImageSets.push_back(push);
     }
    }
    if( _ghProgressInfo != 0 ){
     _ghProgressInfo->iDetailedProgress = 5*28;
     _ghProgressInfo->strDetailedCaption = "[OBIEKTY]";
     _ghProgressCallback->Tick();
    }
    for(int i=0;i<objcnt[mainplaneid];i++){
     /*if( piInfo != 0 ){
      piInfo->Lock();
      if( i == 0 )
       piInfo->SetDetailedProgress(70);
      else
       piInfo->SetDetailedProgress(70+(20*((100.0f/(float(objcnt[mainplaneid])/float(i))/100))));

      //10 = 20
      //5 = 10

      char tmp[129];
      sprintf(tmp, "Mapa: wczytywanie obiektow (%d/%d)", i, objcnt[mainplaneid]);
      piInfo->SetDescription(tmp);
      piInfo->Unlock();
     }*/
     mainPlane->m_vObjects.push_back(new Object());
     ReadObject(mainPlane->m_vObjects[i], str);
    }

    int atrcount = 0;
    int checksum;
    str->RINT(checksum);
    str->seekg(4, std::ios_base::cur);
    str->RINT(atrcount);
    str->seekg(20, std::ios_base::cur);

     if( _ghProgressInfo != 0 ){
      _ghProgressInfo->iDetailedProgress = 6*28;
      _ghProgressInfo->strDetailedCaption = "[ARETYBUTY KLOCKOW]";
      _ghProgressCallback->Tick();
     }

    for(int i=0;i<atrcount;i++){
     m_hTileAtribs.push_back(new TileAtrib());
     str->RINT(m_hTileAtribs[i]->m_iType);
     if( m_hTileAtribs[i]->m_iType != 1 && m_hTileAtribs[i]->m_iType != 2 ){
      throw WWD_EXCEPTION(Error_InvalidTileProperty);
     }
     str->seekg(4, std::ios_base::cur);
     str->RINT(m_hTileAtribs[i]->m_iW);
     str->RINT(m_hTileAtribs[i]->m_iH);
     str->RINT(m_hTileAtribs[i]->m_iAtribInside);
     if( m_hTileAtribs[i]->m_iType == AtribType_Double ){
      m_hTileAtribs[i]->m_iAtribOutside = m_hTileAtribs[i]->m_iAtribInside;
      str->RINT(m_hTileAtribs[i]->m_iAtribInside);
      ReadRect(&m_hTileAtribs[i]->m_rMask, str);
     }
    }
    int at = str->tellg();
    str->seekg(0, std::ios_base::end);
    if( ((int)str->tellg())-at != 0 ){
     throw WWD_EXCEPTION(Error_NotCompleteCRC);
    }
    if( m_iFlags & Flag_w_Compress ){
     delete str;
    }

     if( _ghProgressInfo != 0 ){
      _ghProgressInfo->iDetailedProgress = 7*28;
      _ghProgressInfo->strDetailedCaption = "[META TAGI]";
      _ghProgressCallback->Tick();
     }

    if( hMetaSerializer != 0 ){
     psSource->seekg(0, std::ios_base::end);
     hMetaSerializer->DeserializeFrom(psSource);
    }
}

void WWD::Parser::CleanStr(char * pszStr, int piSize)
{
    for(int i=strlen(pszStr);i<piSize;i++)
     pszStr[i] = '\0';
}

WWD::TileAtrib::TileAtrib(int pW, int pH, TILE_ATRIBTYPE pType, TILE_ATRIB pIns, TILE_ATRIB pOut, Rect pMask)
{
    m_iW = pW;
    m_iH = pH;
    m_iType = pType;
    m_iAtribInside = pIns;
    m_iAtribOutside = pOut;
    m_rMask = pMask;
}

void WWD::Parser::CompileToFile(const char * pszFilename, bool pbWithActualDate)
{
    std::fstream file(pszFilename, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc | std::ios_base::in);
    if( file.fail() )
     throw WWD_EXCEPTION(Error_SaveAccess);
    char datecopy[64];
    if( pbWithActualDate ){
     strncpy(datecopy, m_szDate, 64);
     UpdateDate();
    }
    CompileToStream(&file);
    file.close();
    if( pbWithActualDate )
     strncpy(m_szDate, datecopy, 64);
}

void WWD::Parser::CompileToStream(std::iostream * psDestination)
{
    //printf("atrib is %d\n", (int)GetTileAtribs(298)->GetAtribInside());
    //if( m_iFlags & Flag_w_Compress )
    // throw WWD_EXCEPTION("Compressed WWD are not supported.", 3);

    int iOverallTilesCount = 0;
    for(size_t x=0;x<m_hPlanes.size();x++){
     iOverallTilesCount += m_hPlanes[x]->GetPlaneWidth()*m_hPlanes[x]->GetPlaneHeight();
    }

    int iPlanesStart = 1524;
    int iTilesOffset = iPlanesStart + m_hPlanes.size()*160, iSetStringsOffset = iTilesOffset + iOverallTilesCount*4;
    int iObjectsStart = iSetStringsOffset;

    for(size_t z=0;z<m_hPlanes.size();z++){
     for(int y=0;y<m_hPlanes[z]->GetImageSetsCount();y++){
      iObjectsStart += strlen(m_hPlanes[z]->m_vImageSets[y])+1;
     }
    }
    //printf("sets start %d\n", iSetStringsOffset);
    int iPlanesEnd = iObjectsStart;
    for(size_t z=0;z<m_hPlanes.size();z++){
     for(int y=0;y<m_hPlanes[z]->GetObjectsCount();y++){
      Object * obj = m_hPlanes[z]->GetObjectByIterator(y);
      iPlanesEnd += 284 + strlen(obj->m_szName) + strlen(obj->m_szLogic) + strlen(obj->m_szImageSet) + strlen(obj->m_szAnim);
     }
    }
    //printf("planes end %d\n", iPlanesEnd);
    //printf("atrib is %d\n", (int)GetTileAtribs(298)->GetAtribInside());

    char sign[4];
    sign[0] = 0xF4;
    sign[1] = 0x05;
    sign[2] = sign[3] = 0x00;
    psDestination->write((char*)sign, 4);

    byte b = 0;
    for(int i=0;i<4;i++)
     psDestination->WBYTE(b);

    b = m_iFlags;
    psDestination->WBYTE(b);

    b = 0;
    for(int x=0;x<7;x++)
     psDestination->WBYTE(b);

    CleanStr(m_szMapName, 64);
    CleanStr(m_szAuthor, 64);
    CleanStr(m_szDate, 64);
    CleanStr(m_szRezPath, 256);
    CleanStr(m_szTilesPath, 128);
    CleanStr(m_szPalPath, 128);
    CleanStr(m_szExePath, 128);
    for(int x=0;x<4;x++)
     CleanStr(m_szImageSets[x], 128);
    for(int x=0;x<4;x++)
     CleanStr(m_szSetsPrefixes[x], 32);

    psDestination->WLEN(&m_szMapName, 64);
    psDestination->WLEN(&m_szAuthor, 64);
    psDestination->WLEN(&m_szDate, 64);
    psDestination->WLEN(&m_szRezPath, 256);
    psDestination->WLEN(&m_szTilesPath, 128);
    psDestination->WLEN(&m_szPalPath, 128);
    psDestination->WLEN(&m_iStartX, 4);
    psDestination->WLEN(&m_iStartY, 4);
    int i = 0;
    psDestination->WLEN(&i, 4);
    i = m_hPlanes.size();
    psDestination->WLEN(&i, 4);
    i = iPlanesStart;
    psDestination->WLEN(&i, 4);
    i = iPlanesEnd;
    psDestination->WLEN(&i, 4);
    i = 0;
    psDestination->WLEN(&i, 4);
    psDestination->WLEN(&i, 4);
    psDestination->WLEN(&i, 4);
    psDestination->WLEN(&m_szExePath, 128);
    for(int x=0;x<4;x++)
     psDestination->WLEN(&m_szImageSets[x], 128);
    for(int x=0;x<4;x++)
     psDestination->WLEN(&m_szSetsPrefixes[x], 32);

    std::iostream * psTrueDestination = psDestination;
    if( m_iFlags & Flag_w_Compress ){
     psDestination = new std::stringstream("", std::ios_base::out | std::ios_base::in | std::ios_base::binary);
     printf("original: 0x%p, uncompressed: 0x%p\n", psTrueDestination, psDestination);
    }

    int iTOff = 0, iSOff = 0;
    for(size_t x=0;x<m_hPlanes.size();x++){
     b = 160;
     psDestination->WBYTE(b);

     b = 0;
     for(int y=0;y<7;y++)
      psDestination->WBYTE(b);

     b = m_hPlanes[x]->GetFlags();
     psDestination->WBYTE(b);

     b = 0;
     for(int y=0;y<7;y++)
      psDestination->WBYTE(b);

     CleanStr(m_hPlanes[x]->m_szName, 64);
     psDestination->WLEN(&m_hPlanes[x]->m_szName, 64);
     psDestination->WLEN(&m_hPlanes[x]->m_iWpx, 4);
     psDestination->WLEN(&m_hPlanes[x]->m_iHpx, 4);
     psDestination->WLEN(&m_hPlanes[x]->m_iTileW, 4);
     psDestination->WLEN(&m_hPlanes[x]->m_iTileH, 4);
     psDestination->WLEN(&m_hPlanes[x]->m_iW, 4);
     psDestination->WLEN(&m_hPlanes[x]->m_iH, 4);
     i = 0;
     psDestination->WLEN(&i, 4);
     psDestination->WLEN(&i, 4);
     psDestination->WLEN(&m_hPlanes[x]->m_iMoveX, 4);
     psDestination->WLEN(&m_hPlanes[x]->m_iMoveY, 4);
     psDestination->WLEN(&m_hPlanes[x]->m_iFillColor, 4);
     i = m_hPlanes[x]->m_vImageSets.size();
     psDestination->WLEN(&i, 4);
     i = m_hPlanes[x]->GetObjectsCount();
     psDestination->WLEN(&i, 4);
     i = 1524 + 160 * m_hPlanes.size() + iTOff;
     psDestination->WLEN(&i, 4);
     i = 1524+160*m_hPlanes.size()+iOverallTilesCount*4 + iSOff;
     psDestination->WLEN(&i, 4);

     iTOff += m_hPlanes[x]->m_iW*m_hPlanes[x]->m_iH*4;
     for(int y=0;y<m_hPlanes[x]->GetImageSetsCount();y++){
      iSOff += strlen(m_hPlanes[x]->m_vImageSets[y])+1;
     }

     if( m_hPlanes[x]->GetObjectsCount() == 0 ){
      i = 0;
      psDestination->WLEN(&i, 4);
     }else{
      psDestination->WLEN(&iObjectsStart, 4);
     }
     psDestination->WLEN(&m_hPlanes[x]->m_iZCoord, 4);
     b = 0;
     for(int y=0;y<12;y++)
      psDestination->WBYTE(b);
    }
    //printf("atrib is %d\n", (int)GetTileAtribs(298)->GetAtribInside());

    for(size_t z=0;z<m_hPlanes.size();z++){
     for(int y=0;y<m_hPlanes[z]->GetPlaneHeight();y++)
      for(int x=0;x<m_hPlanes[z]->GetPlaneWidth();x++){
       Tile * ptr = m_hPlanes[z]->GetTile(x, y);
       unsigned char tile[4];
       if( ptr->IsInvisible() ){
        for(int w=0;w<4;w++)
         tile[w] = 0xFF;
       }else if( ptr->IsFilled() ){
        for(int w=0;w<4;w++)
         tile[w] = 0xEE;
       }else{
        int id = ptr->GetID();
        if( id < 256 ){
         tile[0] = id;
         tile[1] = tile[2] = tile[3] = 0;
        }else{
         tile[2] = tile[3] = 0;
         tile[0] = id%256;
         tile[1] = int(id/256);
        }
       }
       psDestination->WLEN(&tile, 4);
      }
    }

    for(size_t z=0;z<m_hPlanes.size();z++){
     for(int y=0;y<m_hPlanes[z]->GetImageSetsCount();y++){
      psDestination->WLEN(m_hPlanes[z]->m_vImageSets[y], strlen(m_hPlanes[z]->m_vImageSets[y])+1);
     }
    }

    for(size_t z=0;z<m_hPlanes.size();z++){
     for(int y=0;y<m_hPlanes[z]->GetObjectsCount();y++){
      WriteObject(m_hPlanes[z]->GetObjectByIterator(y), psDestination);
     }
    }

    //printf("atrib is %d\n", (int)GetTileAtribs(298)->GetAtribInside());
    i = 32;
    psDestination->WLEN(&i, 4);
    i = 0;
    psDestination->WLEN(&i, 4);
    i = m_hTileAtribs.size();
    psDestination->WLEN(&i, 4);
    i = 0;
    for(int z=0;z<5;z++)
     psDestination->WLEN(&i, 4);

    for(size_t z=0;z<m_hTileAtribs.size();z++){
     //printf("wr %d@%d|", z, (int)psDestination->tellp());
     //if( (int)psDestination->tellp() == 188545){
      //printf("flag is %d (%d)\n", (int)m_hTileAtribs[z].m_iAtribInside, z);
     //}
     psDestination->WLEN(&m_hTileAtribs[z]->m_iType, 4);
     psDestination->WLEN(&i, 4);
     psDestination->WLEN(&m_hTileAtribs[z]->m_iW, 4);
     psDestination->WLEN(&m_hTileAtribs[z]->m_iH, 4);
     if( m_hTileAtribs[z]->m_iType == AtribType_Double ){
      psDestination->WLEN(&m_hTileAtribs[z]->m_iAtribOutside, 4);
      psDestination->WLEN(&m_hTileAtribs[z]->m_iAtribInside, 4);
      WriteRect(&m_hTileAtribs[z]->m_rMask, psDestination);
     }else
      psDestination->WLEN(&m_hTileAtribs[z]->m_iAtribInside, 4);
    }
    if( m_iFlags & Flag_w_Compress ){
     //printf("original: 0x%p, uncompressed: 0x%p\n", psTrueDestination, psDestination);
     //printf("orig pos: %d uncompr pos: %d\n", int(psTrueDestination->tellp()), int(psDestination->tellp()));
     std::iostream * uncompressed = psDestination;
     psDestination = psTrueDestination;
     uncompressed->seekg(0, std::ios_base::end);
     int len = (int)uncompressed->tellp();
     uncompressed->seekg(0, std::ios_base::beg);
     psDestination->seekp(744, std::ios_base::beg);
     psDestination->WLEN(&len, 4);
     psDestination->seekp(1524, std::ios_base::beg);
     Deflate(uncompressed, psDestination, 9);
    }

    unsigned int sum = CalculateChecksum(psDestination, 1524);
    psDestination->seekp(748, std::ios_base::beg);
    psDestination->WLEN(&sum, 4);

    if( hMetaSerializer != 0 ){
     psDestination->seekp(0, std::ios_base::end);
     psDestination->seekg(0, std::ios_base::end);
     hMetaSerializer->SerializeTo(psDestination);
    }
}

unsigned int WWD::Parser::CalculateChecksum(std::istream * psStream, int piOffset)
{
    int pos = psStream->tellg();
    unsigned int length;
    psStream->seekg (0, std::ios::end);
    length = psStream->tellg();
    psStream->seekg(piOffset, std::ios::beg);
    unsigned int newCheckSum = 4294967295 - 159;
    for(unsigned int n=piOffset; n<length; n++){
     unsigned char buffer;
	 psStream->read((char*)&buffer, 1);
	 newCheckSum = newCheckSum - (n-(piOffset-1)) + (unsigned int)buffer;
    }
    psStream->seekg(pos);
    return newCheckSum;
}

void WWD::Parser::MoveBytes(std::ostream * psStream, int c)
{
    byte b = 0;
    for(int i=0;i<c;i++)
     psStream->WBYTE(b);
}

void WWD::Parser::WriteObject(Object * hObj, std::ostream * psDestination)
{
    psDestination->WLEN(&hObj->m_iParams[Param_ID], 4);
    int i = strlen(hObj->m_szName);
    psDestination->WLEN(&i, 4);
    i = strlen(hObj->m_szLogic);
    psDestination->WLEN(&i, 4);
    i = strlen(hObj->m_szImageSet);
    psDestination->WLEN(&i, 4);
    i = strlen(hObj->m_szAnim);
    psDestination->WLEN(&i, 4);
    psDestination->WLEN(&hObj->m_iParams[Param_LocationX], 16);
    byte b = hObj->m_iFlagsAdd;
    psDestination->WBYTE(b);
    MoveBytes(psDestination, 3);
    b = hObj->m_iFlagsDynamic;
    psDestination->WBYTE(b);
    MoveBytes(psDestination, 3);

    b = hObj->m_iFlagsDraw;
    psDestination->WBYTE(b);
    MoveBytes(psDestination, 3);

    WriteFlag(hObj->m_iFlagsUser, psDestination);

    psDestination->WLEN(&hObj->m_iParams[Param_Score], 24);

    WriteRect(&hObj->m_rMove, psDestination);
    WriteRect(&hObj->m_rHit, psDestination);
    WriteRect(&hObj->m_rAttack, psDestination);
    WriteRect(&hObj->m_rClip, psDestination);
    WriteRect(&hObj->m_rUser[0], psDestination);
    WriteRect(&hObj->m_rUser[1], psDestination);
    psDestination->WLEN(&hObj->m_iUserValues[0], 32);

    psDestination->WLEN(&hObj->m_iParams[Param_MinX], 64);
    WriteFlag(hObj->m_iFlagsType, psDestination);
    WriteFlag(hObj->m_iFlagsHitType, psDestination);
    psDestination->WLEN(&hObj->m_iParams[Param_MoveResX], 8);

    psDestination->WLEN(hObj->m_szName, strlen(hObj->m_szName));
    psDestination->WLEN(hObj->m_szLogic, strlen(hObj->m_szLogic));
    psDestination->WLEN(hObj->m_szImageSet, strlen(hObj->m_szImageSet));
    psDestination->WLEN(hObj->m_szAnim, strlen(hObj->m_szAnim));
    //psDestination->
}

void WWD::Parser::WriteFlag(int piFlag, std::ostream * psDestination)
{
    if( piFlag < 256 ){
     byte b = piFlag;
     psDestination->WBYTE(b);
     b = 0;
     psDestination->WBYTE(b);
     psDestination->WBYTE(b);
     psDestination->WBYTE(b);
    }else{
     byte second = 0;
     if( piFlag & 256 ){
      second += 1;
      piFlag -= 256;
     }
     if( piFlag & 512 ){
      second += 2;
      piFlag -= 512;
     }
     if( piFlag & 1024 ){
      second += 4;
      piFlag -= 1024;
     }
     if( piFlag & 2048 ){
      second += 8;
      piFlag -= 2048;
     }
     if( piFlag & 4096 ){
      second += 16;
      piFlag -= 4096;
     }
     byte first = piFlag;
     psDestination->WBYTE(first);
     psDestination->WBYTE(second);
     first = 0;
     psDestination->WBYTE(first);
     psDestination->WBYTE(first);
    }
}

void WWD::Parser::WriteRect(Rect * phRect, std::ostream * psDestination)
{
    psDestination->WLEN(&phRect->x1, 4);
    psDestination->WLEN(&phRect->y1, 4);
    psDestination->WLEN(&phRect->x2, 4);
    psDestination->WLEN(&phRect->y2, 4);
}

void WWD::Parser::Deflate(std::istream * psSource, std::ostream * psDest, int iLevel)
{
    int iAbsolutePos = psSource->tellg();
    psSource->seekg(0, std::ios_base::end);
    int iSourceLen = psSource->tellg();
    psSource->seekg(0, std::ios_base::beg);
    #define ZLIB_CHUNK 262144
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[ZLIB_CHUNK];
    unsigned char out[ZLIB_CHUNK];
    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, iLevel);
    if (ret != Z_OK)
        throw WWD_EXCEPTION(WWD::Error_Deflate);
    /* compress until end of file */

    do {
        printf("pos %d, len %d\n", (int)psSource->tellg(), iSourceLen);
        strm.avail_in = (iSourceLen-((int)psSource->tellg()) > ZLIB_CHUNK) ? ZLIB_CHUNK : iSourceLen-((int)psSource->tellg());
        psSource->read((char*)&in, (int)strm.avail_in);
        if( psSource->fail() ){
            (void)deflateEnd(&strm);
            throw WWD_EXCEPTION(WWD::Error_Deflate);
        }
        flush = (psSource->eof() || iSourceLen == (int)psSource->tellg()) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;
        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            printf("chunk done writing\n");
            strm.avail_out = ZLIB_CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            if( ret == Z_STREAM_ERROR ) throw WWD_EXCEPTION(WWD::Error_Deflate);
            have = ZLIB_CHUNK - strm.avail_out;
            psDest->write((char*)&out, (int)have);
            /*if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }*/
        } while (strm.avail_out == 0);
        if( strm.avail_in != 0 ) throw WWD_EXCEPTION(WWD::Error_Deflate);
        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    if( ret != Z_STREAM_END ) throw WWD_EXCEPTION(WWD::Error_Deflate);
    /* clean up and return */
    (void)deflateEnd(&strm);
    //return Z_OK;
    #undef ZLIB_CHUNK
    psSource->seekg(iAbsolutePos, std::ios_base::beg);
}

std::istringstream * WWD::Parser::Inflate(std::istream * psSource)
{
    std::ostringstream output(std::ostringstream::out);
    int ret;
    unsigned have;
    z_stream strm;
    int initial = psSource->tellg();
    psSource->seekg(0, std::ios_base::end);
    int len = (int)psSource->tellg() - initial;
    psSource->seekg(initial, std::ios_base::beg);
    unsigned char in[len];
    unsigned char out[len];
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if( ret != Z_OK )
     throw WWD_EXCEPTION(Error_Inflate);
    do{
     strm.avail_in = len;
     //printf("readng %d bytes\n", len);
     psSource->RLEN(&in, len);
        if ( psSource->fail() ) {
            (void)inflateEnd(&strm);
            throw WWD_EXCEPTION(Error_Inflate);
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;
        do {
            if( _ghProgressInfo != 0 ){
             _ghProgressInfo->iDetailedProgress = 2*28+(float(strm.total_in)/float(len)*28);
             _ghProgressInfo->strDetailedCaption = "[DEKOMPRESJA]";
             _ghProgressCallback->Tick();
            }
            strm.avail_out = len;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            if( ret == Z_STREAM_ERROR ) throw WWD_EXCEPTION(WWD::Error_Deflate);
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                throw WWD_EXCEPTION(Error_Inflate);
            }
            have = len - strm.avail_out;
            output.WLEN(out, have);
            /*if (output.WLEN(out, have) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return NULL;
            }*/
        } while (strm.avail_out == 0);

    } while (ret != Z_STREAM_END);
    /* clean up and return */
    (void)inflateEnd(&strm);
    //return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
    std::istringstream * rets = new std::istringstream(output.str(), std::ios_base::in);
    return rets;
}

void WWD::Parser::ReadObject(Object * hObj, std::istream * psSource)
{
    psSource->RINT(hObj->m_iParams[Param_ID]);
    int iNameLen, iLogicLen, iImageSetLen, iAnimLen;
    psSource->RINT(iNameLen);
    psSource->RINT(iLogicLen);
    psSource->RINT(iImageSetLen);
    psSource->RINT(iAnimLen);
    psSource->RLEN(&hObj->m_iParams[Param_LocationX], 16);
    byte b;
    psSource->RBYTE(b);
    psSource->seekg(3, std::ios_base::cur);
    hObj->m_iFlagsAdd = (WWD::OBJ_ADD_FLAGS)b;

    psSource->RBYTE(b);
    psSource->seekg(3, std::ios_base::cur);
    hObj->m_iFlagsDynamic = (WWD::OBJ_DYNAMIC_FLAGS)b;

    psSource->RBYTE(b);
    psSource->seekg(3, std::ios_base::cur);
    hObj->m_iFlagsDraw = (WWD::OBJ_DRAW_FLAGS)b;

    psSource->RBYTE(b);
    byte b2;
    psSource->RBYTE(b2);

    hObj->m_iFlagsUser = (WWD::OBJ_USER_FLAGS)FormFlag(b, b2);

    psSource->seekg(2, std::ios_base::cur);

    psSource->RLEN(&hObj->m_iParams[Param_Score], 24);

    ReadRect(&hObj->m_rMove, psSource);
    ReadRect(&hObj->m_rHit, psSource);
    ReadRect(&hObj->m_rAttack, psSource);
    ReadRect(&hObj->m_rClip, psSource);
    ReadRect(&hObj->m_rUser[0], psSource);
    ReadRect(&hObj->m_rUser[1], psSource);
    psSource->RLEN(&hObj->m_iUserValues[0], 32);

    psSource->RLEN(&hObj->m_iParams[Param_MinX], 64);

    psSource->RBYTE(b);
    psSource->RBYTE(b2);
    hObj->m_iFlagsType = (WWD::OBJ_TYPE_FLAGS)FormFlag(b, b2);
    psSource->seekg(2, std::ios_base::cur);

    psSource->RBYTE(b);
    psSource->RBYTE(b2);
    hObj->m_iFlagsHitType = (WWD::OBJ_TYPE_FLAGS)FormFlag(b, b2);
    psSource->seekg(2, std::ios_base::cur);

    psSource->RLEN(&hObj->m_iParams[Param_MoveResX], 8);

    delete [] hObj->m_szName;
    hObj->m_szName = new char[iNameLen+1];
    psSource->RLEN(hObj->m_szName, iNameLen);
    hObj->m_szName[iNameLen] = '\0';

    delete [] hObj->m_szLogic;
    hObj->m_szLogic = new char[iLogicLen+1];
    psSource->RLEN(hObj->m_szLogic, iLogicLen);
    hObj->m_szLogic[iLogicLen] = '\0';

    delete [] hObj->m_szImageSet;
    hObj->m_szImageSet = new char[iImageSetLen+1];
    psSource->RLEN(hObj->m_szImageSet, iImageSetLen);
    hObj->m_szImageSet[iImageSetLen] = '\0';

    delete [] hObj->m_szAnim;
    hObj->m_szAnim = new char[iAnimLen+1];
    psSource->RLEN(hObj->m_szAnim, iAnimLen);
    hObj->m_szAnim[iAnimLen] = '\0';
}

int WWD::Parser::FormFlag(byte b1, byte b2)
{
    int ret = b1;
    if( b2 & 1 )
     ret += 256;
    if( b2 & 2 )
     ret += 512;
    if( b2 & 4 )
     ret += 1024;
    if( b2 & 8 )
     ret += 2048;
    if( b2 & 16 )
     ret += 4096;
    return ret;
}

void WWD::Parser::ReadRect(Rect * phRect, std::istream * psSource)
{
    psSource->RINT(phRect->x1);
    psSource->RINT(phRect->y1);
    psSource->RINT(phRect->x2);
    psSource->RINT(phRect->y2);
}

WWD::Object::~Object()
{
    delete [] m_szName;
    delete [] m_szLogic;
    delete [] m_szImageSet;
    delete [] m_szAnim;
}

WWD::Plane::~Plane()
{
    for(size_t i=0;i<m_vImageSets.size();i++)
     delete [] m_vImageSets[i];
    m_vImageSets.clear();
    for(size_t i=0;i<m_vObjects.size();i++)
     delete m_vObjects[i];
    m_vObjects.clear();

    for(int i=0;i<m_iW;i++){
     delete [] m_hTiles[i];
     m_hTiles[i] = 0;
    }
    delete [] m_hTiles;
}

WWD::TileAtrib::~TileAtrib()
{

}

WWD::Parser::~Parser()
{
    for(size_t i=0;i<m_hPlanes.size();i++)
     delete m_hPlanes[i];
    for(size_t i=0;i<m_hTileAtribs.size();i++)
     delete m_hTileAtribs[i];
}

WWD::Tile * WWD::Plane::GetTile(int piX, int piY)
{
    if( piX < 0 || piY < 0 ) return 0;
    if( piX >= m_iW ){
     if( (m_iFlags & Flag_p_XWrapping) != 0 ){
      piX = ClampX(piX);
     }else{
      return 0;
     }
    }
    if( piY >= m_iH ){
     if( (m_iFlags & Flag_p_YWrapping) != 0 ){
      piY = ClampY(piY);
     }else{
      return 0;
     }
    }
    return &m_hTiles[piX][piY];
}

void WWD::Parser::SetFilePath(const char * nPath)
{
    strcpy(m_szFile, nPath);
}

void WWD::Parser::SetFlag(WWD_FLAGS piFlag, bool pbValue)
{
    if( (pbValue && (m_iFlags & piFlag)) ||
        (!pbValue && !(m_iFlags & piFlag)) )
     return;
    if( pbValue ){
     int a = m_iFlags;
     a += piFlag;
     m_iFlags = (WWD_FLAGS)a;
    }else{
     int a = m_iFlags;
     //a &= piFlag;
     a -= piFlag;
     m_iFlags = (WWD_FLAGS)a;
    }
}

bool WWD::Parser::GetFlag(WWD_FLAGS piFlag)
{
    if( m_iFlags & piFlag )
     return 1;
    return 0;
}

void WWD::Plane::DeleteObject(Object * ptr)
{
    for(size_t i=0;i<m_vObjects.size();i++)
     if( m_vObjects[i] == ptr ){
      if( m_hObjDeletionCB != NULL )
       (*m_hObjDeletionCB)(ptr);
      delete ptr;
      m_vObjects[i] = NULL;
      m_vObjects.erase(m_vObjects.begin()+i);
      return;
     }
}

void WWD::Plane::DeleteObjectFromListByIterator(int piID)
{
    m_vObjects[piID] = NULL;
    m_vObjects.erase(m_vObjects.begin()+piID);
}

void WWD::Plane::DeleteObjectFromListByID(int piID)
{
    for(size_t i=0;i<m_vObjects.size();i++){
     if( m_vObjects[i]->GetParam(Param_ID) == piID ){
      m_vObjects[piID] = NULL;
      m_vObjects.erase(m_vObjects.begin()+i);
      i--;
     }
    }
}

void WWD::Plane::DeleteObjectByIterator(int piID)
{
    if( m_hObjDeletionCB != NULL )
     (*m_hObjDeletionCB)(m_vObjects[piID]);
    delete m_vObjects[piID];
    m_vObjects[piID] = NULL;
    m_vObjects.erase(m_vObjects.begin()+piID);
}

void WWD::Plane::DeleteObjectByID(int piID)
{
    for(size_t i=0;i<m_vObjects.size();i++){
     if( m_vObjects[i]->GetParam(Param_ID) == piID ){
      if( m_hObjDeletionCB != NULL )
       (*m_hObjDeletionCB)(m_vObjects[piID]);
      delete m_vObjects[i];
      m_vObjects[piID] = NULL;
      m_vObjects.erase(m_vObjects.begin()+i);
      i--;
     }
    }
}

WWD::Object::Object()
{
    m_szName = new char[1];
    m_szName[0] = '\0';
    m_szAnim = new char[1];
    m_szAnim[0] = '\0';
    m_szImageSet = new char[1];
    m_szImageSet[0] = '\0';
    m_szLogic = new char[1];
    m_szLogic[0] = '\0';
    for(int i=0;i<8;i++)
     m_iUserValues[i] = 0;
    m_iFlagsAdd = Flag_a_Nothing;
    m_iFlagsDynamic = Flag_dy_Nothing;
    m_iFlagsDraw = Flag_dr_Nothing;
    m_iFlagsUser = Flag_u_Nothing;
    m_iFlagsType = Flag_t_Nothing;
    m_iFlagsHitType = Flag_t_Nothing;
    for(int i=0;i<OBJ_PARAMS_CNT;i++)
     m_iParams[i] = 0;
    m_hUserData = NULL;
}

WWD::Object::Object(Object * src)
{
    m_iFlagsAdd = src->m_iFlagsAdd;
    m_iFlagsDynamic = src->m_iFlagsDynamic;
    m_iFlagsDraw = src->m_iFlagsDraw;
    m_iFlagsUser = src->m_iFlagsUser;
    m_iFlagsType = src->m_iFlagsType;
    m_iFlagsHitType = src->m_iFlagsHitType;
    for(int i=0;i<OBJ_PARAMS_CNT;i++)
     m_iParams[i] = src->m_iParams[i];
    for(int i=0;i<8;i++)
     m_iUserValues[i] = src->m_iUserValues[i];

    m_szName = new char[strlen(src->m_szName)+1];
    strcpy(m_szName, src->m_szName);
    m_szLogic = new char[strlen(src->m_szLogic)+1];
    strcpy(m_szLogic, src->m_szLogic);
    m_szImageSet = new char[strlen(src->m_szImageSet)+1];
    strcpy(m_szImageSet, src->m_szImageSet);
    m_szAnim = new char[strlen(src->m_szAnim)+1];
    strcpy(m_szAnim, src->m_szAnim);

    m_rMove.Set(src->m_rMove);
    m_rHit.Set(src->m_rHit);
    m_rAttack.Set(src->m_rAttack);
    m_rClip.Set(src->m_rClip);
    m_rUser[0].Set(src->m_rUser[0]);
    m_rUser[1].Set(src->m_rUser[1]);

    m_hUserData = NULL;
}

void WWD::Plane::AddObjectAndCalcID(Object * n)
{
    int freeID = n->GetParam(Param_ID);
    bool initial = 1;
    bool bChange = 1;
    while( bChange ){
     bChange = 0;
     for(size_t i=0;i<m_vObjects.size();i++){
      if( m_vObjects[i]->GetParam(Param_ID) == freeID ){
       if( initial ){
        freeID = 0;
        initial = 0;
       }else
        freeID++;
       bChange = 1;
       break;
      }
     }
    }
    n->SetParam(Param_ID, freeID);
    m_vObjects.push_back(n);
}

WWD::GAME WWD::GetGameTypeFromFile(const char * pszFilepath, int * piBaseLevel) throw(Exception)
{
    std::ifstream str(pszFilepath, std::ios_base::binary | std::ios_base::in);
    if( str.fail() )
     throw WWD_EXCEPTION(Error_OpenAccess);

    str.seekg(884, std::ios_base::beg);
    for(int i=0;i<4;i++){
     char tmp[128];
     str.RLEN(&tmp, 128);
     if( tmp[0] != '\0' ){
      int test = 0;
      if( sscanf(tmp, "LEVEL%d%*s", &test) ){
       if( piBaseLevel != NULL )
        *piBaseLevel = test;
       return Game_Claw;
      }else if( sscanf(tmp, "AREA%d%*s", &test) ){
       if( piBaseLevel != NULL )
        *piBaseLevel = test;
       return Game_Gruntz;
      }else if( sscanf(tmp, "DUNGEON%d%*s", &test) ){
       if( piBaseLevel != NULL )
        *piBaseLevel = test;
       return Game_GetMedieval;
      }
     }
    }
    //str.close();
    return Game_Unknown;
}

void WWD::Object::SetName(const char * nname)
{
    if( m_szName != NULL )
     delete [] m_szName;
    m_szName = new char[strlen(nname)+1];
    strcpy(m_szName, nname);
}

void WWD::Object::SetLogic(const char * nlogic)
{
    if( m_szLogic != NULL )
     delete [] m_szLogic;
    m_szLogic = new char[strlen(nlogic)+1];
    strcpy(m_szLogic, nlogic);
}

void WWD::Object::SetImageSet(const char * niset)
{
    if( m_szImageSet != NULL )
     delete [] m_szImageSet;
    m_szImageSet = new char[strlen(niset)+1];
    strcpy(m_szImageSet, niset);
}

void WWD::Object::SetAnim(const char * nanim)
{
    if( m_szAnim != NULL )
     delete [] m_szAnim;
    m_szAnim = new char[strlen(nanim)+1];
    strcpy(m_szAnim, nanim);
}

void WWD::Parser::SetImageSetPrefix(int id, const char * npref)
{
    strncpy(m_szSetsPrefixes[id], npref, 32);
}

void WWD::Parser::SetImageSet(int id, const char * npath)
{
    strncpy(m_szImageSets[id], npath, 128);
}

WWD::TileAtrib::TileAtrib(TileAtrib * src)
{
    SetTo(src);
}

void WWD::TileAtrib::SetTo(TileAtrib * src)
{
    m_iW = src->GetW();
    m_iH = src->GetH();
    m_iType = src->GetType();
    m_iAtribInside = src->GetAtribInside();
    m_iAtribOutside = src->GetAtribOutside();
    m_rMask = src->GetMask();
}

WWD::TileAtrib::TileAtrib()
{
    m_iW = m_iH = 0;
}

bool WWD::Rect::Collide(WWD::Rect b)
{
    int w1 = x2-x1,
        h1 = y2-y1,
        w2 = b.x2-b.x1,
        h2 = b.y2-b.y1;
    bool collide = 0, overlap = 0;
    if( (x1    > b.x1 && x1    < b.x1+w2 && y1    > b.y1 && y1    < b.y1+h2) ||
        (x1+w1 > b.x1 && x1+w1 < b.x1+w2 && y1    > b.y1 && y1    < b.y1+h2) ||
        (x1+w1 > b.x1 && x1+w1 < b.x1+w2 && y1+h1 > b.y1 && y1+h1 < b.y1+h2) ||
        (x1    > b.x1 && x1    < b.x1+w2 && y1+h1 > b.y1 && y1+h1 < b.y1+h2) )
     collide = 1;

    int minx, miny, maxx, maxy;
    if( x1 < b.x1 ){
     minx = x1;
     maxx = b.x1+w2;
    }else{
     minx = b.x1;
     maxx = x1+w1;
    }
    if( y1 < b.y1 ){
     miny = y1;
     maxy = b.y1+h2;
    }else{
     miny = b.y1;
     maxy = y1+h1;
    }

    if( !((maxx-minx) > (w1+w2) || (maxy-miny) > (h1+h2)) )
     overlap = 1;

    return collide||overlap;
}

std::vector<WWD::CollisionRect> WWD::TileAtrib::GetColRects()
{
    std::vector<CollisionRect> r;
    if( GetType() == WWD::AtribType_Single ||
        GetType() == WWD::AtribType_Double && GetAtribInside() == GetAtribOutside() ){
     if( GetAtribInside() == WWD::Atrib_Clear ) return r;
     CollisionRect n;
     n.WWD_CR_TYPE = GetAtribInside();
     n.WWD_CR_RECT = Rect(0, 0, 63, 63);
     r.push_back(n);
     return r;
    }
     if( GetAtribInside() != WWD::Atrib_Clear ){
      CollisionRect n;
      n.WWD_CR_TYPE = GetAtribInside();
      n.WWD_CR_RECT = GetMask();
      r.push_back(n);
     }
     if( GetAtribOutside() != WWD::Atrib_Clear ){
      Rect mask = GetMask();
      if( mask.y1 != 0 ){
       CollisionRect n;
       n.WWD_CR_TYPE = GetAtribOutside();
       n.WWD_CR_RECT = Rect(0,0,63,mask.y1);
       r.push_back(n);
      }
      if( mask.x1 != 0 ){
       CollisionRect n;
       n.WWD_CR_TYPE = GetAtribOutside();
       n.WWD_CR_RECT = Rect(0,0,mask.x1,63);
       r.push_back(n);
      }
      if( mask.y2 != 63 ){
       CollisionRect n;
       n.WWD_CR_TYPE = GetAtribOutside();
       n.WWD_CR_RECT = Rect(0,mask.y2,63,63);
       r.push_back(n);
      }
      if( mask.x2 != 63 ){
       CollisionRect n;
       n.WWD_CR_TYPE = GetAtribOutside();
       n.WWD_CR_RECT = Rect(mask.x2,0,63,63);
       r.push_back(n);
      }
     }
    return r;
}

WWD::Tile & WWD::Tile::operator=(const WWD::Tile &src)
{
    if( this == &src ) return *this;
    m_iID = src.m_iID;
    m_bInvisible = src.m_bInvisible;
    m_bFilled = src.m_bFilled;
    return *this;
}

bool WWD::Tile::operator==(const WWD::Tile &src)
{
    if( this == &src ) return 1;
    if( src.m_bInvisible && m_bInvisible ) return 1;
    if( src.m_bFilled && m_bFilled ) return 1;
    if( src.m_iID == m_iID && m_iID != -1 ) return 1;
    return 0;
}

bool WWD::Tile::operator!=(const WWD::Tile &src)
{
    return !(*this == src);
}

void WWD::Plane::Resize(int nw, int nh)
{
    Tile ** newt;
    newt = new Tile * [nw];
    for(int x=0;x<nw;x++)
     newt[x] = new Tile[nh];

    for(int y=0;y<nh;y++)
     for(int x=0;x<nw;x++){
      if( (x >= m_iW || y >= m_iH) || m_hTiles == NULL ){
       newt[x][y].SetInvisible(1);
       continue;
      }
      newt[x][y] = m_hTiles[x][y];
     }

    if( m_hTiles != NULL ){
     for(int x=0;x<m_iW;x++){
      delete [] m_hTiles[x];
      m_hTiles[x] = 0;
     }
     delete [] m_hTiles;
    }
    m_hTiles = newt;
    m_iW = nw;
    m_iH = nh;
    m_iWpx = m_iW*m_iTileW;
    m_iHpx = m_iH*m_iTileH;
}

void WWD::Plane::ResizeAnchor(int rx, int ry, int anchor)
{
    Tile ** tnew = new Tile * [rx];
    for(int x=0;x<rx;x++){
     tnew[x] = new Tile[ry];
     for(int y=0;y<ry;y++)
      tnew[x][y].SetInvisible(1);
    }
    int xdiff = rx-m_iW, ydiff = ry-m_iH;
    int xplace, yplace;
    if( anchor == 1 || anchor == 2 || anchor == 3 ) yplace = 0;
    if( anchor == 4 || anchor == 5 || anchor == 6 ) yplace = 1;
    if( anchor == 7 || anchor == 8 || anchor == 9 ) yplace = 2;

    if( anchor == 1 || anchor == 4 || anchor == 7 ) xplace = 0;
    if( anchor == 2 || anchor == 5 || anchor == 8 ) xplace = 1;
    if( anchor == 3 || anchor == 6 || anchor == 9 ) xplace = 2;

    int srcx = 0, srcy = 0, destx = 0, desty = 0;
    int objx = 0, objy = 0;

    if( xdiff < 0 ){ //plane cut x
     if( xplace == 1 ) //cut from both sides
      srcx = xdiff/2;
     else if( xplace == 2 ) //cut from left
      srcx = xdiff;
    }
    if( ydiff < 0 ){ //plane cut y
     if( yplace == 1 ) //cut from both sides
      srcy = ydiff/2;
     else if( yplace == 2 ) //cut from right
      srcy = ydiff;
    }

    if( xdiff >= 0 ){ //plane extend x
     if( xplace == 1 ) //add from both sides
      destx = xdiff/2;
     else if( xplace == 2 ) //add from left
      destx = xdiff;
    }
    if( ydiff >= 0 ){ //plane extend y
     if( yplace == 1 ) //add from both sides
      desty = ydiff/2;
     else if( yplace == 2 ) //add from up
      desty = ydiff;
    }

    objx = (-srcx*m_iTileW)+(destx*m_iTileW);
    objy = (-srcy*m_iTileH)+(desty*m_iTileH);

    for(int y=desty;y<ry;y++)
     for(int x=destx;x<rx;x++){
      if( x-destx+srcx < 0 || x-destx+srcx >=  m_iW ||
          y-desty+srcy < 0 || y-desty+srcy >= m_iH ) continue;
      tnew[x][y] = m_hTiles[x-destx+srcx][y-desty+srcy];
     }

    for(int x=0;x<m_iW;x++){
     delete [] m_hTiles[x];
     m_hTiles[x] = 0;
    }
    delete [] m_hTiles;
    m_hTiles = tnew;
    m_iW = rx;
    m_iH = ry;
    m_iWpx = m_iW*m_iTileW;
    m_iHpx = m_iH*m_iTileH;

    if( objx != 0 || objy != 0 ){
     for(size_t i=0;i<m_vObjects.size();i++){
      m_vObjects[i]->SetParam(WWD::Param_LocationX, m_vObjects[i]->GetParam(WWD::Param_LocationX)+objx);
      m_vObjects[i]->SetParam(WWD::Param_LocationY, m_vObjects[i]->GetParam(WWD::Param_LocationY)+objy);
     }
    }
}

void WWD::Plane::ResizeRelative(int rx, int ry, bool ax, bool ay)
{
    int nw = m_iW+rx, nh = m_iH+ry;
    Tile ** newt;
    newt = new Tile * [nw];
    for(int x=0;x<nw;x++){
     newt[x] = new Tile[nh];
    }

    int srcstartx = 0, srcstarty = 0;
    int destx = 0, desty = 0;
    int objoffx = 0, objoffy = 0;
    if( !ax ){
     if( rx > 0 ) destx = rx;
     else if( rx < 0 ) srcstartx = -rx;
     objoffx = abs(rx)*m_iTileW;
    }
    if( !ay ){
     if( ry > 0 ) desty = ry;
     else if( ry < 0 ) srcstarty = -ry;
     objoffy = abs(ry)*m_iTileH;
    }
    for(int y=0;y<m_iH;y++)
     for(int x=0;x<m_iW;x++){
      if( x+srcstartx >= m_iW || y+srcstarty >= m_iH ) continue;
      if( x+destx >= nw || y+desty >= nh ) continue;
      newt[x+destx][y+desty] = m_hTiles[x+srcstartx][y+srcstarty];
     }

    for(int x=0;x<m_iW;x++){
     delete [] m_hTiles[x];
     m_hTiles[x] = 0;
    }
    delete [] m_hTiles;
    m_hTiles = newt;
    m_iW = nw;
    m_iH = nh;
    m_iWpx = m_iW*m_iTileW;
    m_iHpx = m_iH*m_iTileH;

    if( objoffx != 0 || objoffy != 0 ){
     for(size_t i=0;i<m_vObjects.size();i++){
      m_vObjects[i]->SetParam(WWD::Param_LocationX, m_vObjects[i]->GetParam(WWD::Param_LocationX)+objoffx);
      m_vObjects[i]->SetParam(WWD::Param_LocationY, m_vObjects[i]->GetParam(WWD::Param_LocationY)+objoffy);
     }
    }
}

void WWD::Parser::UpdateDate()
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    sprintf(m_szDate, "%02d:%02d %02d.%02d.%d", st.wHour, st.wMinute, st.wDay, st.wMonth, st.wYear);
}

void WWD::Parser::DeletePlane(int piID)
{
    if( size_t(piID) < 0 || size_t(piID) >= m_hPlanes.size() ) return;
    delete m_hPlanes[piID];
    m_hPlanes.erase(m_hPlanes.begin()+piID);
}

void WWD::Parser::AddPlane(WWD::Plane * np)
{
    /*for(int i=0;i<m_hPlanes.size();i++){
     if( np->GetZCoord() < m_hPlanes[i]->GetZCoord() ){
      m_hPlanes.insert(m_hPlanes.begin()+i, np);
      return;
     }
    }*/
    m_hPlanes.push_back(np);
}

void WWD::Parser::SortPlanes()
{
    std::sort(m_hPlanes.begin(), m_hPlanes.end(), WWD::SortPlanes);
}

void WWD::Plane::SetFlag(PLANE_FLAGS piFlag, bool pbValue)
{
    if( pbValue && (m_iFlags & piFlag) ||
        !pbValue && !(m_iFlags & piFlag) )
     return;
    if( pbValue ){
     int a = m_iFlags;
     a += piFlag;
     m_iFlags = (PLANE_FLAGS)a;
    }else{
     int a = m_iFlags;
     //a &= piFlag;
     a -= piFlag;
     m_iFlags = (PLANE_FLAGS)a;
    }
}

bool WWD::Plane::GetFlag(PLANE_FLAGS piFlag)
{
    if( m_iFlags & piFlag )
     return 1;
    return 0;
}

void WWD::Plane::AddImageSet(const char * n)
{
    char * n2 = new char[strlen(n)+1];
    strcpy(n2, n);
    m_vImageSets.push_back(n2);
}

void WWD::Plane::ClearImageSets()
{
    for(int i=0;i<m_vImageSets.size();i++)
     delete [] m_vImageSets[i];
    m_vImageSets.clear();
}

void WWD::Parser::SetTileAtribsCount(int i)
{
    if( i == m_hTileAtribs.size() ) return;
    if( i > m_hTileAtribs.size() ){
     int num = i-m_hTileAtribs.size();
     for(int x=0;x<num;x++)
      m_hTileAtribs.push_back(new TileAtrib(64, 64, AtribType_Single, Atrib_Clear));
     return;
    }
    int num = m_hTileAtribs.size()-i;
    int start = m_hTileAtribs.size()-num;
    for(int x=start;x<m_hTileAtribs.size();x++)
     delete m_hTileAtribs[x];
    m_hTileAtribs.erase(m_hTileAtribs.begin()+start, m_hTileAtribs.begin()+start+num);
}
