#include "cWWD.h"
#include <fstream>
#include <sstream>
#include <algorithm>

#ifdef WAP_MAP
#include "cProgressInfo.h"
#include "../WapMap/cParallelLoop.h"
#include "../WapMap/cObjectUserData.h"
#endif // WAP_MAP

namespace WWD {
#ifdef WAP_MAP
    structProgressInfo *_ghProgressInfo = 0;
    cParallelLoop *_ghProgressCallback = 0;
#endif // WAP_MAP

    bool SortPlanes(WWD::Plane *a, WWD::Plane *b) {
        return a->GetZCoord() < b->GetZCoord();
    }
}

WWD::Parser::Parser(void *ptr, uint32_t iLen, CustomMetaSerializer *hSerializer) {
    hMetaSerializer = hSerializer;
    //piInfo = NULL;
    m_szFile[0] = '\0';
    std::stringbuf *sb = new std::stringbuf(std::string((const char *) ptr, iLen), std::ios_base::in);
    std::istream *str = new std::istream(sb);
    if (str->fail())
        throw WWD_EXCEPTION(Error_LoadingMemory);
    LoadFromStream(str);
    delete str;
    delete sb;
}

WWD::Parser::Parser(const char *pszFilename, CustomMetaSerializer *hSerializer) {
    hMetaSerializer = hSerializer;
    strcpy(m_szFile, pszFilename);
    std::ifstream str(pszFilename, std::ios_base::binary | std::ios_base::in);
    if (str.fail())
        throw WWD_EXCEPTION(Error_OpenAccess);
    LoadFromStream(&str);

    str.close();
}

void WWD::Parser::LoadFileHeader(std::istream *psSource) {
    psSource->read((char*)&m_Header, sizeof(WWDHeader));
    if (m_Header.size != sizeof(WWDHeader))
        throw WWD_EXCEPTION(Error_BadMagicNumber);

    char pref[3] = {0};
    if (m_Header.m_szTilesPath[0] != 'L' && m_Header.m_szTilesPath[0] != 'A' && m_Header.m_szTilesPath[0] != 'D')
        pref[0] = m_Header.m_szTilesPath[0];
    char formats[3][64];
    sprintf(formats[0], "%s%s", pref, "LEVEL%d%*s");
    sprintf(formats[1], "%s%s", pref, "AREA%d%*s");
    sprintf(formats[2], "%s%s", pref, "DUNGEON%d%*s");
    if (sscanf(m_Header.m_szTilesPath, formats[0], &m_iBaseLevel)) {
        m_iGame = Game_Claw;
    } else if (sscanf(m_Header.m_szTilesPath, formats[1], &m_iBaseLevel)) {
        m_iGame = Game_Gruntz;
    } else if (sscanf(m_Header.m_szTilesPath, formats[2], &m_iBaseLevel)) {
        m_iGame = Game_GetMedieval;
    } else {
        m_iGame = Game_Unknown;
        m_iBaseLevel = 0;
    }
}

void WWD::Parser::LoadFromStream(std::istream *psSource) {
#ifdef WAP_MAP
    if (_ghProgressInfo != 0) {
        _ghProgressInfo->iDetailedProgress = 28;
        _ghProgressInfo->strDetailedCaption = "[NAGLOWEK]";
        _ghProgressInfo->iDetailedEnd = 198;
        _ghProgressCallback->Tick();
    }
#endif // WAP_MAP

    LoadFileHeader(psSource);

	std::stringstream uncompressed_data;
    if (m_Header.m_iFlags & Flag_w_Compress) {
#ifdef WAP_MAP
        if (_ghProgressInfo != 0) {
            _ghProgressInfo->iDetailedProgress = 2 * 28;
            _ghProgressInfo->strDetailedCaption = "[DEKOMPRESJA]";
            _ghProgressCallback->Tick();
        }
#endif // WAP_MAP

		Inflate(psSource, uncompressed_data);
		psSource = &uncompressed_data;
    }

#ifdef WAP_MAP
    if (_ghProgressInfo != 0) {
        _ghProgressInfo->iDetailedProgress = 3 * 28;
        _ghProgressInfo->strDetailedCaption = "[WARSTWY]";
        _ghProgressCallback->Tick();
    }
#endif // WAP_MAP

    size_t pos = m_Header.size;
    if (pos != m_Header.m_hPlanesStart) {
        psSource->seekg(m_Header.m_hPlanesStart - pos, std::ios_base::cur);
    }

    for (int i = 0; i < m_Header.m_iPlanesCount; i++) {
        Plane *pl = new Plane(false);
        psSource->read((char *) &pl->m_Header, sizeof(PlaneHeader));
        pos += sizeof(PlaneHeader);
        if (pl->m_Header.size != sizeof(PlaneHeader))
            throw WWD_EXCEPTION(Error_BadMagicNumber);
        pl->m_hObjDeletionCB = NULL;

        for (int z = 0; z < i; z++) {
            if (!strcmp(pl->m_Header.m_szName, m_hPlanes[z]->GetName())) {
                char ntmp[64];
                char tmp[64];
                strncpy(tmp, pl->m_Header.m_szName, 60);
                int d = 2;
                while (1) {
                    sprintf(ntmp, "%s (%d)", tmp, d);
                    bool bFound = 0;
                    for (int y = 0; y < i; y++) {
                        if (!strcmp(ntmp, m_hPlanes[y]->GetName()))
                            bFound = 1;
                    }
                    d++;
                    if (!bFound) break;
                }
                strcpy(pl->m_Header.m_szName, ntmp);
                //printf("[WWD]: Plane name conflict for '%s'. Renamed to: '%s'.\n", m_hPlanes[z]->GetName(), ntmp);
                break;
            }
        }

        pl->m_hTiles = new Tile[pl->m_Header.m_iW * pl->m_Header.m_iH];
        pl->rowOffsets = new unsigned[pl->m_Header.m_iH];

        for (int i = 0; i < pl->m_Header.m_iH; ++i) {
            pl->rowOffsets[i] = pl->m_Header.m_iW * i;
        }

        if (pl->m_Header.m_iFillColor < 0) pl->m_Header.m_iFillColor = 0;
        if (pl->m_Header.m_iFillColor > 255) pl->m_Header.m_iFillColor = 255;

        if (pl->m_Header.m_iObjectsCount > 0)
            pl->m_vObjects.reserve(pl->m_Header.m_iObjectsCount);

        m_hPlanes.push_back(pl);

        if (pl->m_Header.m_iFlags & Flag_p_MainPlane) {
            mainPlane = pl;
        }
    }

#ifdef WAP_MAP
        if (_ghProgressInfo != 0) {
            _ghProgressInfo->strDetailedCaption = "[KLOCKI]";
            _ghProgressCallback->Tick();
        }
#endif // WAP_MAP
    for (int i = 0; i < m_Header.m_iPlanesCount; i++) {
        Plane *pl = m_hPlanes[i];
        if (pos != pl->m_Header.m_iTilesDataPtr) {
            psSource->seekg(pl->m_Header.m_iTilesDataPtr - pos, std::ios_base::cur);
        }

        size_t planeSize = pl->m_Header.m_iW * pl->m_Header.m_iH;
        for (size_t i = 0; i < planeSize; ++i) {
            unsigned int tile;
            psSource->RINT(tile);
            if (tile == WWD_TILE_EMPTY) {
                pl->m_hTiles[i].m_bInvisible = 1;
                pl->m_hTiles[i].m_bFilled = 0;
                pl->m_hTiles[i].m_iID = 0;
            } else {
                pl->m_hTiles[i].m_bInvisible = 0;

                if (tile == WWD_TILE_FILL) {
                    pl->m_hTiles[i].m_bFilled = 1;
                    pl->m_hTiles[i].m_iID = 0;
                } else {
                    pl->m_hTiles[i].m_bFilled = 0;
                    pl->m_hTiles[i].m_iID = tile;
                }
            }
        }
        pos += planeSize * 4;
    }

    if (mainPlane == NULL) {
        throw WWD_EXCEPTION(Error_NoMainPlane);
    }

    for (int i = 0; i < m_Header.m_iPlanesCount; ++i) {
        Plane *pl = m_hPlanes[i];

        if (pos != pl->m_Header.m_iSetsDataPtr) {
            psSource->seekg(pl->m_Header.m_iSetsDataPtr - pos, std::ios_base::cur);
        }

        int setsToLoad = m_hPlanes[i]->m_Header.m_iSetsCount;
        m_hPlanes[i]->m_vImageSets.reserve(setsToLoad);

        while (--setsToLoad >= 0) {
            std::string strBuf;
            std::getline( *psSource, strBuf, '\0' );
            pos += strBuf.length() + 1;
            m_hPlanes[i]->m_vImageSets.emplace_back(strBuf);
        }
    }

#ifdef WAP_MAP
    if (_ghProgressInfo != 0) {
        _ghProgressInfo->iDetailedProgress = 5 * 28;
        _ghProgressInfo->strDetailedCaption = "[OBIEKTY]";
        _ghProgressCallback->Tick();
    }
#endif // WAP_MAP
    if (mainPlane->m_Header.m_iObjectsDataPtr) {
        if (pos != mainPlane->m_Header.m_iObjectsDataPtr) {
            psSource->seekg(mainPlane->m_Header.m_iObjectsDataPtr - pos, std::ios_base::cur);
        }

        for (int i = 0; i < mainPlane->m_Header.m_iObjectsCount; i++) {
            mainPlane->m_vObjects.push_back(new Object());
            pos += ReadObject(mainPlane->m_vObjects[i], psSource);
        }
    }

#ifdef WAP_MAP
    if (_ghProgressInfo != 0) {
        _ghProgressInfo->iDetailedProgress = 6 * 28;
        _ghProgressInfo->strDetailedCaption = "[ARETYBUTY KLOCKOW]";
        _ghProgressCallback->Tick();
    }
#endif // WAP_MAP
    if (pos != m_Header.m_hTileAttributesStart) {
        psSource->seekg(m_Header.m_hTileAttributesStart - pos, std::ios_base::cur);
    }

    int attribsCount = 0, checksum;
    psSource->RINT(checksum);
    psSource->seekg(4, std::ios_base::cur);
    psSource->RINT(attribsCount);
    psSource->seekg(20, std::ios_base::cur);

    m_hTileAttribs.reserve(attribsCount);

    for (int i = 0; i < attribsCount; i++) {
        m_hTileAttribs.push_back(new TileAttrib());
        psSource->RINT(m_hTileAttribs[i]->m_iType);
        if (m_hTileAttribs[i]->m_iType != 1 && m_hTileAttribs[i]->m_iType != 2) {
            throw WWD_EXCEPTION(Error_InvalidTileProperty);
        }
        psSource->seekg(4, std::ios_base::cur);
        psSource->RINT(m_hTileAttribs[i]->m_iW);
        psSource->RINT(m_hTileAttribs[i]->m_iH);
        psSource->RINT(m_hTileAttribs[i]->m_iAtribInside);
        if (m_hTileAttribs[i]->m_iType == AttribType_Double) {
            m_hTileAttribs[i]->m_iAtribOutside = m_hTileAttribs[i]->m_iAtribInside;
            psSource->RINT(m_hTileAttribs[i]->m_iAtribInside);
            ReadRect(&m_hTileAttribs[i]->m_rMask, psSource);
        }
    }
    int at = psSource->tellg();
    psSource->seekg(0, std::ios_base::end);
    if (((int) psSource->tellg()) - at != 0) {
        throw WWD_EXCEPTION(Error_NotCompleteCRC);
    }
/*
#ifdef WAP_MAP
    if (_ghProgressInfo != 0) {
        _ghProgressInfo->iDetailedProgress = 7 * 28;
        _ghProgressInfo->strDetailedCaption = "[META TAGI]";
        _ghProgressCallback->Tick();
    }
#endif // WAP_MAP

    if (hMetaSerializer != 0) {
        psSource->seekg(0, std::ios_base::end);
        hMetaSerializer->DeserializeFrom(psSource);
    }*/
}

void WWD::Parser::CleanStr(char *pszStr, int piSize) {
    for (int i = strlen(pszStr); i < piSize; i++)
        pszStr[i] = '\0';
}

WWD::TileAttrib::TileAttrib(int pW, int pH, TILE_ATTRIB_TYPE pType, TILE_ATTRIB pIns, TILE_ATTRIB pOut, Rect pMask) {
    m_iW = pW;
    m_iH = pH;
    m_iType = pType;
    m_iAtribInside = pIns;
    m_iAtribOutside = pOut;
    m_rMask = pMask;
}

void WWD::Parser::CompileToFile(const char *pszFilename, bool pbWithActualDate) {
    std::fstream file(pszFilename,
                      std::ios_base::binary | std::ios_base::out | std::ios_base::trunc | std::ios_base::in);
    if (file.fail())
        throw WWD_EXCEPTION(Error_SaveAccess);
    char datecopy[64];
    if (pbWithActualDate) {
        strncpy(datecopy, m_Header.m_szDate, 64);
        UpdateDate();
    }
    CompileToStream(&file);
    file.close();
    if (pbWithActualDate)
        strncpy(m_Header.m_szDate, datecopy, 64);
}

void WWD::Parser::CompileToStream(std::iostream *psDestination) {
    m_Header.m_iPlanesCount = m_hPlanes.size();
    unsigned iOverallTilesCount = 0;
    unsigned iTilesStart = m_Header.size + m_hPlanes.size() * sizeof(PlaneHeader);
    unsigned iObjectsStart = iTilesStart;
    for (auto & m_hPlane : m_hPlanes) {
        iOverallTilesCount += m_hPlane->GetPlaneWidth() * m_hPlane->GetPlaneHeight();
        for (int y = 0; y < m_hPlane->GetImageSetsCount(); y++) {
            iObjectsStart += m_hPlane->m_vImageSets[y].length() + 1;
        }
    }
    iObjectsStart += iOverallTilesCount * 4;

    m_Header.m_hTileAttributesStart = iObjectsStart;
    for (auto & m_hPlane : m_hPlanes) {
        for (int y = 0; y < m_hPlane->GetObjectsCount(); y++) {
            Object *obj = m_hPlane->GetObjectByIterator(y);
            m_Header.m_hTileAttributesStart += 284 + strlen(obj->m_szName) + strlen(obj->m_szLogic)
                                                   + strlen(obj->m_szImageSet) + strlen(obj->m_szAnim);
        }
    }

    CleanStr(m_Header.m_szMapName, 64);
    CleanStr(m_Header.m_szAuthor, 64);
    CleanStr(m_Header.m_szDate, 64);
    CleanStr(m_Header.m_szRezPath, 256);
    CleanStr(m_Header.m_szTilesPath, 128);
    CleanStr(m_Header.m_szPalPath, 128);
    CleanStr(m_Header.m_szExePath, 128);
    for (int x = 0; x < 4; x++)
        CleanStr(m_Header.m_szImageSets[x], 128);
    for (int x = 0; x < 4; x++)
        CleanStr(m_Header.m_szSetsPrefixes[x], 32);

    psDestination->WLEN(&m_Header.size, m_Header.size);

    std::iostream *psTrueDestination = psDestination;
    if (m_Header.m_iFlags & Flag_w_Compress) {
        psDestination = new std::stringstream("", std::ios_base::out | std::ios_base::in | std::ios_base::binary);
        printf("original: 0x%p, uncompressed: 0x%p\n", psTrueDestination, psDestination);
    }

    int iTOff = 0, iSOff = 0; byte b;
    for (size_t x = 0; x < m_hPlanes.size(); x++) {
        CleanStr(m_hPlanes[x]->m_Header.m_szName, 64);
        m_hPlanes[x]->m_Header.m_iSetsCount = m_hPlanes[x]->m_vImageSets.size();
        m_hPlanes[x]->m_Header.m_iObjectsCount = m_hPlanes[x]->GetObjectsCount();
        m_hPlanes[x]->m_Header.m_iTilesDataPtr = iTilesStart + iTOff;
        m_hPlanes[x]->m_Header.m_iSetsDataPtr = iTilesStart + iOverallTilesCount * 4 + iSOff;
        m_hPlanes[x]->m_Header.m_iObjectsDataPtr = (m_hPlanes[x]->GetObjectsCount() == 0) ? 0 : iObjectsStart;
        psDestination->WLEN(&m_hPlanes[x]->m_Header.size, m_hPlanes[x]->m_Header.size);

        iTOff += m_hPlanes[x]->m_Header.m_iW * m_hPlanes[x]->m_Header.m_iH * 4;
        for (int y = 0; y < m_hPlanes[x]->GetImageSetsCount(); y++) {
            iSOff += m_hPlanes[x]->m_vImageSets[y].length() + 1;
        }
    }

    for (size_t z = 0; z < m_hPlanes.size(); z++) {
        for (int y = 0; y < m_hPlanes[z]->GetPlaneHeight(); y++)
            for (int x = 0; x < m_hPlanes[z]->GetPlaneWidth(); x++) {
                Tile *ptr = m_hPlanes[z]->GetTile(x, y);
                unsigned char tile[4];
                if (ptr->IsInvisible()) {
                    for (int w = 0; w < 4; w++)
                        tile[w] = 0xFF;
                } else if (ptr->IsFilled()) {
                    for (int w = 0; w < 4; w++)
                        tile[w] = 0xEE;
                } else {
                    int id = ptr->GetID();
                    if (id < 256) {
                        tile[0] = id;
                        tile[1] = tile[2] = tile[3] = 0;
                    } else {
                        tile[2] = tile[3] = 0;
                        tile[0] = id % 256;
                        tile[1] = int(id / 256);
                    }
                }
                psDestination->WLEN(&tile, 4);
            }
    }

    for (size_t z = 0; z < m_hPlanes.size(); z++) {
        for (int y = 0; y < m_hPlanes[z]->GetImageSetsCount(); y++) {
            psDestination->WLEN(m_hPlanes[z]->m_vImageSets[y].c_str(), m_hPlanes[z]->m_vImageSets[y].length() + 1);
        }
    }

    for (size_t z = 0; z < m_hPlanes.size(); z++) {
        for (int y = 0; y < m_hPlanes[z]->GetObjectsCount(); y++) {
            WriteObject(m_hPlanes[z]->GetObjectByIterator(y), psDestination);
        }
    }

    int i = 32;
    psDestination->WLEN(&i, 4);
    i = 0;
    psDestination->WLEN(&i, 4);
    i = m_hTileAttribs.size();
    psDestination->WLEN(&i, 4);
    i = 0;
    for (int z = 0; z < 5; z++)
        psDestination->WLEN(&i, 4);

    for (size_t z = 0; z < m_hTileAttribs.size(); z++) {
        //printf("wr %d@%d|", z, (int)psDestination->tellp());
        //if( (int)psDestination->tellp() == 188545){
        //printf("flag is %d (%d)\n", (int)m_hTileAttribs[z].m_iAtribInside, z);
        //}
        psDestination->WLEN(&m_hTileAttribs[z]->m_iType, 4);
        psDestination->WLEN(&i, 4);
        psDestination->WLEN(&m_hTileAttribs[z]->m_iW, 4);
        psDestination->WLEN(&m_hTileAttribs[z]->m_iH, 4);
        if (m_hTileAttribs[z]->m_iType == AttribType_Double) {
            psDestination->WLEN(&m_hTileAttribs[z]->m_iAtribOutside, 4);
            psDestination->WLEN(&m_hTileAttribs[z]->m_iAtribInside, 4);
            WriteRect(&m_hTileAttribs[z]->m_rMask, psDestination);
        } else
            psDestination->WLEN(&m_hTileAttribs[z]->m_iAtribInside, 4);
    }
    if (m_Header.m_iFlags & Flag_w_Compress) {
        //printf("original: 0x%p, uncompressed: 0x%p\n", psTrueDestination, psDestination);
        //printf("orig pos: %d uncompr pos: %d\n", int(psTrueDestination->tellp()), int(psDestination->tellp()));
        std::iostream *uncompressed = psDestination;
        psDestination = psTrueDestination;
        uncompressed->seekg(0, std::ios_base::end);
        int len = (int) uncompressed->tellp();
        uncompressed->seekg(0, std::ios_base::beg);
        psDestination->seekp(744, std::ios_base::beg);
        psDestination->WLEN(&len, 4);
        psDestination->seekp(1524, std::ios_base::beg);
        Deflate(uncompressed, psDestination, 9);
    }

    unsigned int sum = CalculateChecksum(psDestination, 1524);
    psDestination->seekp(748, std::ios_base::beg);
    psDestination->WLEN(&sum, 4);

    /*if (hMetaSerializer != 0) {
        psDestination->seekp(0, std::ios_base::end);
        psDestination->seekg(0, std::ios_base::end);
        hMetaSerializer->SerializeTo(psDestination);
    }*/
}

unsigned int WWD::Parser::CalculateChecksum(std::istream *psStream, int piOffset) {
    int pos = psStream->tellg();
    unsigned int length;
    psStream->seekg(0, std::ios::end);
    length = psStream->tellg();
    psStream->seekg(piOffset, std::ios::beg);
    unsigned int newCheckSum = 4294967295 - 159;
    for (unsigned int n = piOffset; n < length; n++) {
        unsigned char buffer;
        psStream->read((char *) &buffer, 1);
        newCheckSum = newCheckSum - (n - (piOffset - 1)) + (unsigned int) buffer;
    }
    psStream->seekg(pos);
    return newCheckSum;
}

void WWD::Parser::MoveBytes(std::ostream *psStream, int c) {
    byte b = 0;
    for (int i = 0; i < c; i++)
        psStream->WBYTE(b);
}

void WWD::Parser::WriteObject(Object *hObj, std::ostream *psDestination) {
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

void WWD::Parser::WriteFlag(int piFlag, std::ostream *psDestination) {
    if (piFlag < 256) {
        byte b = piFlag;
        psDestination->WBYTE(b);
        b = 0;
        psDestination->WBYTE(b);
        psDestination->WBYTE(b);
        psDestination->WBYTE(b);
    } else {
        byte second = 0;
        if (piFlag & 256) {
            second += 1;
            piFlag -= 256;
        }
        if (piFlag & 512) {
            second += 2;
            piFlag -= 512;
        }
        if (piFlag & 1024) {
            second += 4;
            piFlag -= 1024;
        }
        if (piFlag & 2048) {
            second += 8;
            piFlag -= 2048;
        }
        if (piFlag & 4096) {
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

void WWD::Parser::WriteRect(Rect *phRect, std::ostream *psDestination) {
    psDestination->WLEN(&phRect->x1, 4);
    psDestination->WLEN(&phRect->y1, 4);
    psDestination->WLEN(&phRect->x2, 4);
    psDestination->WLEN(&phRect->y2, 4);
}

void WWD::Parser::Deflate(std::istream *psSource, std::ostream *psDest, int iLevel) {
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
        printf("pos %d, len %d\n", (int) psSource->tellg(), iSourceLen);
        strm.avail_in = (iSourceLen - ((int) psSource->tellg()) > ZLIB_CHUNK) ? ZLIB_CHUNK : iSourceLen -
                                                                                             ((int) psSource->tellg());
        psSource->read((char *) &in, (int) strm.avail_in);
        if (psSource->fail()) {
            (void) deflateEnd(&strm);
            throw WWD_EXCEPTION(WWD::Error_Deflate);
        }
        flush = (psSource->eof() || iSourceLen == (int) psSource->tellg()) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;
        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            printf("chunk done writing\n");
            strm.avail_out = ZLIB_CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            if (ret == Z_STREAM_ERROR) throw WWD_EXCEPTION(WWD::Error_Deflate);
            have = ZLIB_CHUNK - strm.avail_out;
            psDest->write((char *) &out, (int) have);
            /*if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }*/
        } while (strm.avail_out == 0);
        if (strm.avail_in != 0) throw WWD_EXCEPTION(WWD::Error_Deflate);
        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    if (ret != Z_STREAM_END) throw WWD_EXCEPTION(WWD::Error_Deflate);
    /* clean up and return */
    (void) deflateEnd(&strm);
    //return Z_OK;
#undef ZLIB_CHUNK
    psSource->seekg(iAbsolutePos, std::ios_base::beg);
}

void WWD::Parser::Inflate(std::istream *psSource, std::stringstream& output) {
    int ret;
    unsigned have;
    z_stream strm;
    int initial = psSource->tellg();
    psSource->seekg(0, std::ios_base::end);
    int len = (int) psSource->tellg() - initial;
    psSource->seekg(initial, std::ios_base::beg);
    unsigned char *in = new unsigned char[len];
    unsigned char *out = new unsigned char[len];
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK) {
        delete[] in;
        delete[] out;
        throw WWD_EXCEPTION(Error_Inflate);
    }
    do {
        strm.avail_in = len;
        psSource->RLEN(in, len);
        if (psSource->fail()) {
            (void) inflateEnd(&strm);
            delete[] in;
            delete[] out;
            throw WWD_EXCEPTION(Error_Inflate);
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;
        do {
#ifdef WAP_MAP
            if (_ghProgressInfo != 0) {
                _ghProgressInfo->iDetailedProgress = 2 * 28 + (float(strm.total_in) / float(len) * 28);
                _ghProgressInfo->strDetailedCaption = "[DEKOMPRESJA]";
                _ghProgressCallback->Tick();
            }
#endif // WAP_MAP
            strm.avail_out = len;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR) {
                delete[] in;
                delete[] out;
                throw WWD_EXCEPTION(WWD::Error_Deflate);
            }
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void) inflateEnd(&strm);
                    delete[] in;
                    delete[] out;
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
    delete[] in;
    delete[] out;
    /* clean up and return */
    (void) inflateEnd(&strm);
    //return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

size_t WWD::Parser::ReadObject(Object *hObj, std::istream *psSource) {
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
    hObj->m_iFlagsAdd = (WWD::OBJ_ADD_FLAGS) b;

    psSource->RBYTE(b);
    psSource->seekg(3, std::ios_base::cur);
    hObj->m_iFlagsDynamic = (WWD::OBJ_DYNAMIC_FLAGS) b;

    psSource->RBYTE(b);
    psSource->seekg(3, std::ios_base::cur);
    hObj->m_iFlagsDraw = (WWD::OBJ_DRAW_FLAGS) b;

    psSource->RBYTE(b);
    byte b2;
    psSource->RBYTE(b2);

    hObj->m_iFlagsUser = (WWD::OBJ_USER_FLAGS) FormFlag(b, b2);

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
    hObj->m_iFlagsType = (WWD::OBJ_TYPE_FLAGS) FormFlag(b, b2);
    psSource->seekg(2, std::ios_base::cur);

    psSource->RBYTE(b);
    psSource->RBYTE(b2);
    hObj->m_iFlagsHitType = (WWD::OBJ_TYPE_FLAGS) FormFlag(b, b2);
    psSource->seekg(2, std::ios_base::cur);

    psSource->RLEN(&hObj->m_iParams[Param_MoveResX], 8);

    delete[] hObj->m_szName;
    hObj->m_szName = new char[iNameLen + 1];
    psSource->RLEN(hObj->m_szName, iNameLen);
    hObj->m_szName[iNameLen] = '\0';

    delete[] hObj->m_szLogic;
    hObj->m_szLogic = new char[iLogicLen + 1];
    psSource->RLEN(hObj->m_szLogic, iLogicLen);
    hObj->m_szLogic[iLogicLen] = '\0';

    delete[] hObj->m_szImageSet;
    hObj->m_szImageSet = new char[iImageSetLen + 1];
    psSource->RLEN(hObj->m_szImageSet, iImageSetLen);
    hObj->m_szImageSet[iImageSetLen] = '\0';

    delete[] hObj->m_szAnim;
    hObj->m_szAnim = new char[iAnimLen + 1];
    psSource->RLEN(hObj->m_szAnim, iAnimLen);
    hObj->m_szAnim[iAnimLen] = '\0';

    return 284 + iNameLen + iLogicLen + iImageSetLen + iAnimLen;
}

int WWD::Parser::FormFlag(byte b1, byte b2) {
    int ret = b1;
    if (b2 & 1)
        ret += 256;
    if (b2 & 2)
        ret += 512;
    if (b2 & 4)
        ret += 1024;
    if (b2 & 8)
        ret += 2048;
    if (b2 & 16)
        ret += 4096;
    return ret;
}

void WWD::Parser::ReadRect(Rect *phRect, std::istream *psSource) {
    psSource->RINT(phRect->x1);
    psSource->RINT(phRect->y1);
    psSource->RINT(phRect->x2);
    psSource->RINT(phRect->y2);
}

WWD::Object::~Object() {
    delete[] m_szName;
    delete[] m_szLogic;
    delete[] m_szImageSet;
    delete[] m_szAnim;
}

WWD::Plane::~Plane() {
    for (size_t i = 0; i < m_vObjects.size(); i++)
        delete m_vObjects[i];

    delete[] rowOffsets;
    delete[] m_hTiles;
}

WWD::TileAttrib::~TileAttrib() {

}

WWD::Parser::~Parser() {
    for (size_t i = 0; i < m_hPlanes.size(); i++)
        delete m_hPlanes[i];
    for (size_t i = 0; i < m_hTileAttribs.size(); i++)
        delete m_hTileAttribs[i];
}

WWD::Tile *WWD::Plane::GetTile(int piX, int piY) {
    if (piX < 0 || piY < 0) return 0;
    if (piX >= m_Header.m_iW) {
        if ((m_Header.m_iFlags & Flag_p_XWrapping) != 0) {
            piX = ClampX(piX);
        } else {
            return 0;
        }
    }
    if (piY >= m_Header.m_iH) {
        if ((m_Header.m_iFlags & Flag_p_YWrapping) != 0) {
            piY = ClampY(piY);
        } else {
            return 0;
        }
    }
    return &m_hTiles[rowOffsets[piY] + piX];
}

WWD::Tile *WWD::Plane::GetTile(int it) {
    return &m_hTiles[it];
}

void WWD::Parser::SetFilePath(const char *nPath) {
    strcpy(m_szFile, nPath);
}

void WWD::Parser::SetFlag(WWD_FLAGS piFlag, bool pbValue) {
    if ((pbValue && (m_Header.m_iFlags & piFlag)) ||
        (!pbValue && !(m_Header.m_iFlags & piFlag)))
        return;
    if (pbValue) {
        int a = m_Header.m_iFlags;
        a += piFlag;
        m_Header.m_iFlags = a;
    } else {
        int a = m_Header.m_iFlags;
        //a &= piFlag;
        a -= piFlag;
        m_Header.m_iFlags = a;
    }
}

bool WWD::Parser::GetFlag(WWD_FLAGS piFlag) {
    return m_Header.m_iFlags & piFlag;
}

void WWD::Plane::DeleteObject(Object *ptr) {
    for (size_t i = 0; i < m_vObjects.size(); i++)
        if (m_vObjects[i] == ptr) {
            if (m_hObjDeletionCB != NULL)
                (*m_hObjDeletionCB)(ptr);
            delete ptr;
            m_vObjects.erase(m_vObjects.begin() + i);
            return;
        }
}

void WWD::Plane::DeleteObjectFromListByIterator(int piID) {
    m_vObjects[piID] = NULL;
    m_vObjects.erase(m_vObjects.begin() + piID);
}

void WWD::Plane::DeleteObjectFromListByID(int piID) {
    for (size_t i = 0; i < m_vObjects.size(); i++) {
        if (m_vObjects[i]->GetParam(Param_ID) == piID) {
            m_vObjects[piID] = NULL;
            m_vObjects.erase(m_vObjects.begin() + i);
            i--;
        }
    }
}

void WWD::Plane::DeleteObjectByIterator(int piID) {
    if (m_hObjDeletionCB != NULL)
        (*m_hObjDeletionCB)(m_vObjects[piID]);
    delete m_vObjects[piID];
    m_vObjects[piID] = NULL;
    m_vObjects.erase(m_vObjects.begin() + piID);
}

void WWD::Plane::DeleteObjectByID(int piID) {
    for (size_t i = 0; i < m_vObjects.size(); i++) {
        if (m_vObjects[i]->GetParam(Param_ID) == piID) {
            if (m_hObjDeletionCB != NULL)
                (*m_hObjDeletionCB)(m_vObjects[piID]);
            delete m_vObjects[i];
            m_vObjects[piID] = NULL;
            m_vObjects.erase(m_vObjects.begin() + i);
            i--;
        }
    }
}

WWD::Object::Object() {
    m_szName = new char[1];
    m_szName[0] = '\0';
    m_szAnim = new char[1];
    m_szAnim[0] = '\0';
    m_szImageSet = new char[1];
    m_szImageSet[0] = '\0';
    m_szLogic = new char[1];
    m_szLogic[0] = '\0';
    for (int i = 0; i < 8; i++)
        m_iUserValues[i] = 0;
    m_iFlagsAdd = Flag_a_Nothing;
    m_iFlagsDynamic = Flag_dy_Nothing;
    m_iFlagsDraw = Flag_dr_Nothing;
    m_iFlagsUser = Flag_u_Nothing;
    m_iFlagsType = Flag_t_Nothing;
    m_iFlagsHitType = Flag_t_Nothing;
    for (int i = 0; i < OBJ_PARAMS_CNT; i++)
        m_iParams[i] = 0;
    m_hUserData = NULL;
}

WWD::Object::Object(Object *src) {
    m_iFlagsAdd = src->m_iFlagsAdd;
    m_iFlagsDynamic = src->m_iFlagsDynamic;
    m_iFlagsDraw = src->m_iFlagsDraw;
    m_iFlagsUser = src->m_iFlagsUser;
    m_iFlagsType = src->m_iFlagsType;
    m_iFlagsHitType = src->m_iFlagsHitType;
    for (int i = 0; i < OBJ_PARAMS_CNT; i++)
        m_iParams[i] = src->m_iParams[i];
    for (int i = 0; i < 8; i++)
        m_iUserValues[i] = src->m_iUserValues[i];

    m_szName = new char[strlen(src->m_szName) + 1];
    strcpy(m_szName, src->m_szName);
    m_szLogic = new char[strlen(src->m_szLogic) + 1];
    strcpy(m_szLogic, src->m_szLogic);
    m_szImageSet = new char[strlen(src->m_szImageSet) + 1];
    strcpy(m_szImageSet, src->m_szImageSet);
    m_szAnim = new char[strlen(src->m_szAnim) + 1];
    strcpy(m_szAnim, src->m_szAnim);

    m_rMove.Set(src->m_rMove);
    m_rHit.Set(src->m_rHit);
    m_rAttack.Set(src->m_rAttack);
    m_rClip.Set(src->m_rClip);
    m_rUser[0].Set(src->m_rUser[0]);
    m_rUser[1].Set(src->m_rUser[1]);

    m_hUserData = NULL;
}

void WWD::Plane::AddObjectAndCalcID(Object *n) {
    int freeID = n->GetParam(Param_ID);
    bool initial = 1;
    bool bChange = 1;
    while (bChange) {
        bChange = 0;
        for (size_t i = 0; i < m_vObjects.size(); i++) {
            if (m_vObjects[i]->GetParam(Param_ID) == freeID) {
                if (initial) {
                    freeID = 0;
                    initial = 0;
                } else
                    freeID++;
                bChange = 1;
                break;
            }
        }
    }
    n->SetParam(Param_ID, freeID);
    m_vObjects.push_back(n);
}

WWD::GAME WWD::GetGameTypeFromFile(const char *pszFilepath, int *piBaseLevel) throw(Exception) {
    const char* dot = strrchr(pszFilepath, '.');
    if (!dot || (dot[1] != 'W' && dot[1] != 'w')) {
        throw WWD_EXCEPTION(Error_UnknownMapExtension);
    }

    if ((dot[2] == 'M' || dot[2] == 'm')
        && (dot[3] == 'D' || dot[3] == 'd')
        && dot[4] == 0) {
        return Game_Claw2;
    } else if ((dot[2] != 'W' && dot[2] != 'w')
        || (dot[3] != 'D' && dot[3] != 'd') || dot[4] != 0) {
        throw WWD_EXCEPTION(Error_UnknownMapExtension);
    }

    std::ifstream str(pszFilepath, std::ios_base::binary | std::ios_base::in);
    if (str.fail()) {
        throw WWD_EXCEPTION(Error_OpenAccess);
    }

    str.seekg(884, std::ios_base::beg);
    for (int i = 0; i < 4; i++) {
        char tmp[128];
        str.RLEN(&tmp, 128);
        if (tmp[0] != '\0') {
            int test = 0;
            if (sscanf(tmp, "LEVEL%d%*s", &test)) {
                if (piBaseLevel != NULL)
                    *piBaseLevel = test;
                return Game_Claw;
            } else if (sscanf(tmp, "AREA%d%*s", &test)) {
                if (piBaseLevel != NULL)
                    *piBaseLevel = test;
                return Game_Gruntz;
            } else if (sscanf(tmp, "DUNGEON%d%*s", &test)) {
                if (piBaseLevel != NULL)
                    *piBaseLevel = test;
                return Game_GetMedieval;
            }
        }
    }
    //str.close();
    return Game_Unknown;
}

void WWD::operator++(WWD::GAME& game) {
    game = (WWD::GAME)(game + 1);
}

void WWD::Object::SetName(const char *nname) {
    if (m_szName != NULL)
        delete[] m_szName;
    m_szName = new char[strlen(nname) + 1];
    strcpy(m_szName, nname);
}

void WWD::Object::SetLogic(const char *nlogic) {
    if (m_szLogic != NULL)
        delete[] m_szLogic;
    m_szLogic = new char[strlen(nlogic) + 1];
    strcpy(m_szLogic, nlogic);
}

void WWD::Object::SetImageSet(const char *niset) {
    if (m_szImageSet != NULL)
        delete[] m_szImageSet;
    m_szImageSet = new char[strlen(niset) + 1];
    strcpy(m_szImageSet, niset);
}

void WWD::Object::SetAnim(const char *nanim) {
    if (m_szAnim != NULL)
        delete[] m_szAnim;
    m_szAnim = new char[strlen(nanim) + 1];
    strcpy(m_szAnim, nanim);
}

int WWD::Object::GetX() {
    return m_hUserData ? GetUserDataFromObj(this)->GetX() : GetParam(Param_LocationX);
}

int WWD::Object::GetY() {
    return m_hUserData ? GetUserDataFromObj(this)->GetY() : GetParam(Param_LocationY);
}

int WWD::Object::GetZ() {
    return m_hUserData ? GetUserDataFromObj(this)->GetZ() : GetParam(Param_LocationZ);
}

int WWD::Object::GetI() {
    return m_hUserData ? GetUserDataFromObj(this)->GetI() : GetParam(Param_LocationI);
}

bool WWD::Object::GetFlipX() {
    return m_hUserData ? GetUserDataFromObj(this)->GetFlipX() : GetDrawFlags() & Flag_dr_Mirror;
}

bool WWD::Object::GetFlipY() {
    return m_hUserData ? GetUserDataFromObj(this)->GetFlipY() : GetDrawFlags() & Flag_dr_Invert;
}

void WWD::Parser::SetImageSetPrefix(int id, const char *npref) {
    strncpy(m_Header.m_szSetsPrefixes[id], npref, 32);
}

void WWD::Parser::SetImageSet(int id, const char *npath) {
    strncpy(m_Header.m_szImageSets[id], npath, 128);
}

WWD::TileAttrib::TileAttrib(TileAttrib *src) {
    SetTo(src);
}

void WWD::TileAttrib::SetTo(TileAttrib *src) {
    m_iW = src->GetW();
    m_iH = src->GetH();
    m_iType = src->GetType();
    m_iAtribInside = src->GetAtribInside();
    m_iAtribOutside = src->GetAtribOutside();
    m_rMask = src->GetMask();
}

WWD::TileAttrib::TileAttrib() {
    m_iW = m_iH = 0;
}

bool WWD::Rect::Collide(WWD::Rect b) {
    int w1 = x2 - x1,
            h1 = y2 - y1,
            w2 = b.x2 - b.x1,
            h2 = b.y2 - b.y1;
    bool collide = 0, overlap = 0;
    if ((x1 > b.x1 && x1 < b.x1 + w2 && y1 > b.y1 && y1 < b.y1 + h2) ||
        (x1 + w1 > b.x1 && x1 + w1 < b.x1 + w2 && y1 > b.y1 && y1 < b.y1 + h2) ||
        (x1 + w1 > b.x1 && x1 + w1 < b.x1 + w2 && y1 + h1 > b.y1 && y1 + h1 < b.y1 + h2) ||
        (x1 > b.x1 && x1 < b.x1 + w2 && y1 + h1 > b.y1 && y1 + h1 < b.y1 + h2))
        collide = 1;

    int minx, miny, maxx, maxy;
    if (x1 < b.x1) {
        minx = x1;
        maxx = b.x1 + w2;
    } else {
        minx = b.x1;
        maxx = x1 + w1;
    }
    if (y1 < b.y1) {
        miny = y1;
        maxy = b.y1 + h2;
    } else {
        miny = b.y1;
        maxy = y1 + h1;
    }

    if (!((maxx - minx) > (w1 + w2) || (maxy - miny) > (h1 + h2)))
        overlap = 1;

    return collide || overlap;
}

std::vector<WWD::CollisionRect> WWD::TileAttrib::GetColRects() {
    std::vector<CollisionRect> r;
    if (GetType() == WWD::AttribType_Single ||
        GetType() == WWD::AttribType_Double && GetAtribInside() == GetAtribOutside()) {
        if (GetAtribInside() == WWD::Attrib_Clear) return r;
        CollisionRect n;
        n.WWD_CR_TYPE = GetAtribInside();
        n.WWD_CR_RECT = Rect(0, 0, 63, 63);
        r.push_back(n);
        return r;
    }
    if (GetAtribInside() != WWD::Attrib_Clear) {
        CollisionRect n;
        n.WWD_CR_TYPE = GetAtribInside();
        n.WWD_CR_RECT = GetMask();
        r.push_back(n);
    }
    if (GetAtribOutside() != WWD::Attrib_Clear) {
        Rect mask = GetMask();
        if (mask.y1 != 0) {
            CollisionRect n;
            n.WWD_CR_TYPE = GetAtribOutside();
            n.WWD_CR_RECT = Rect(0, 0, 63, mask.y1);
            r.push_back(n);
        }
        if (mask.x1 != 0) {
            CollisionRect n;
            n.WWD_CR_TYPE = GetAtribOutside();
            n.WWD_CR_RECT = Rect(0, 0, mask.x1, 63);
            r.push_back(n);
        }
        if (mask.y2 != 63) {
            CollisionRect n;
            n.WWD_CR_TYPE = GetAtribOutside();
            n.WWD_CR_RECT = Rect(0, mask.y2, 63, 63);
            r.push_back(n);
        }
        if (mask.x2 != 63) {
            CollisionRect n;
            n.WWD_CR_TYPE = GetAtribOutside();
            n.WWD_CR_RECT = Rect(mask.x2, 0, 63, 63);
            r.push_back(n);
        }
    }
    return r;
}

WWD::Tile &WWD::Tile::operator=(const WWD::Tile &src) {
    if (this == &src) return *this;
    m_iID = src.m_iID;
    m_bInvisible = src.m_bInvisible;
    m_bFilled = src.m_bFilled;
    return *this;
}

bool WWD::Tile::operator==(const WWD::Tile &src) {
    if (this == &src) return 1;
    if (src.m_bInvisible && m_bInvisible) return 1;
    if (src.m_bFilled && m_bFilled) return 1;
    if (src.m_iID == m_iID && m_iID != -1) return 1;
    return 0;
}

bool WWD::Tile::operator!=(const WWD::Tile &src) {
    return !(*this == src);
}

void WWD::Plane::Resize(int nw, int nh, int ox, int oy) {
    int s = nw * nh;
    Tile * newt = new Tile[s];

    if (m_hTiles) {
        if (ox || oy) {
            int objectsOffsetX = -ox * m_Header.m_iTileW,
                objectsOffsetY = -oy * m_Header.m_iTileH;

            for (auto& m_vObject : m_vObjects) {
                m_vObject->SetParam(WWD::Param_LocationX, m_vObject->GetParam(WWD::Param_LocationX) + objectsOffsetX);
                m_vObject->SetParam(WWD::Param_LocationY, m_vObject->GetParam(WWD::Param_LocationY) + objectsOffsetY);
            }
        }

        int i = 0, y = 0;
        while (oy > 0) {
            for (int x = 0; x < nw; ++x, ++i) {
                newt[i].SetInvisible(1);
            }
            --oy;
            ++y;
        }

        int tx = std::min(nw, m_Header.m_iW + ox),
            ty = std::min(nh, m_Header.m_iH + oy);

        for (; y < ty; ++y) {
            int x = 0;
            int rowOffset = rowOffsets[y];

            for (; x < ox; ++x, ++i) {
                newt[i].SetInvisible(1);
            }

            for (; x < tx; ++x, ++i) {
                newt[i] = m_hTiles[rowOffset + x];
            }

            for (; x < nw; ++x, ++i) {
                newt[i].SetInvisible(1);
            }
        }

        for (; y < nh; ++y) {
            for (int x = 0; x < nw; ++x, ++i) {
                newt[i].SetInvisible(1);
            }
        }
    
        delete[] m_hTiles;
        delete[] rowOffsets;
    }
    else {
        for (int i = 0; i < s; ++i) {
            newt[i].SetInvisible(1);
        }
    }

    m_hTiles = newt;
    rowOffsets = new unsigned[nh];

    for (int i = 0; i < nh; ++i) {
        rowOffsets[i] = i * nw;
    }

    m_Header.m_iW = nw;
    m_Header.m_iH = nh;
    m_Header.m_iWpx = m_Header.m_iW * m_Header.m_iTileW;
    m_Header.m_iHpx = m_Header.m_iH * m_Header.m_iTileH;
}

void WWD::Plane::ResizeAnchor(int nw, int nh, int anchor) {
    int xDiff = nw - m_Header.m_iW, yDiff = nh - m_Header.m_iH;

    int ox = 0, oy = 0;

    switch (anchor % 3) {
    case 0: // right anchor
        ox = xDiff;
        break;
    case 2: // center anchor
        ox = xDiff / 2;
        break;
    }

    switch (anchor / 3) {
    case 0: // bottom anchor
        oy = yDiff;
        break;
    case 2: // center anchor
        oy = yDiff / 2;
        break;
    }

    Resize(nw, nh, ox, oy);
}

void WWD::Plane::ResizeAddTiles(int ax, int ay) {
    int ox = 0, oy = 0;

    if (ax < 0) {
        ax = -ax;
        ox = ax;
    }

    if (ay < 0) {
        ay = -ay;
        oy = ay;
    }

    int nw = m_Header.m_iW + ax, nh = m_Header.m_iH + ay;
    Resize(nw, nh, ox, oy);
}

void WWD::Parser::UpdateDate() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    sprintf(m_Header.m_szDate, "%02d:%02d %02d.%02d.%d", st.wHour, st.wMinute, st.wDay, st.wMonth, st.wYear);
}

void WWD::Parser::DeletePlane(int piID) {
    if (size_t(piID) < 0 || size_t(piID) >= m_hPlanes.size()) return;
    delete m_hPlanes[piID];
    m_hPlanes.erase(m_hPlanes.begin() + piID);
}

void WWD::Parser::AddPlane(WWD::Plane *np) {
    /*for(int i=0;i<m_hPlanes.size();i++){
     if( np->GetZCoord() < m_hPlanes[i]->GetZCoord() ){
      m_hPlanes.insert(m_hPlanes.begin()+i, np);
      return;
     }
    }*/
    m_hPlanes.push_back(np);
}

void WWD::Parser::SortPlanes() {
    std::sort(m_hPlanes.begin(), m_hPlanes.end(), WWD::SortPlanes);
}

void WWD::Plane::SetFlag(PLANE_FLAGS piFlag, bool pbValue) {
    if (pbValue && (m_Header.m_iFlags & piFlag) ||
        !pbValue && !(m_Header.m_iFlags & piFlag))
        return;
    if (pbValue) {
        int a = m_Header.m_iFlags;
        a += piFlag;
        m_Header.m_iFlags = (PLANE_FLAGS) a;
    } else {
        int a = m_Header.m_iFlags;
        //a &= piFlag;
        a -= piFlag;
        m_Header.m_iFlags = (PLANE_FLAGS) a;
    }
}

bool WWD::Plane::GetFlag(PLANE_FLAGS piFlag) {
    return m_Header.m_iFlags & piFlag;
}

void WWD::Plane::AddImageSet(const char *n) {
    m_vImageSets.emplace_back(n);
}

void WWD::Plane::ClearImageSets() {
    m_vImageSets.clear();
}

void WWD::Parser::SetTileAttribsCount(int i) {
    if (i == m_hTileAttribs.size()) return;
    if (i > m_hTileAttribs.size()) {
        int num = i - m_hTileAttribs.size();
        for (int x = 0; x < num; x++)
            m_hTileAttribs.push_back(new TileAttrib(64, 64, AttribType_Single, Attrib_Clear));
        return;
    }
    int num = m_hTileAttribs.size() - i;
    int start = m_hTileAttribs.size() - num;
    for (int x = start; x < m_hTileAttribs.size(); x++)
        delete m_hTileAttribs[x];
    m_hTileAttribs.erase(m_hTileAttribs.begin() + start, m_hTileAttribs.begin() + start + num);
}
