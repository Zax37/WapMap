#include "cWWDx.h"
#include "../cMDI.h"

enum WWDx_BlockType {
    BlockType_MetaInfo = 0,
    BlockType_MetaEndInfo,
    BlockType_WapMapHeader,
    BlockType_Guides,
    BlockType_Favourites
};

std::string cIO_WWDx::ReadCString(std::istream *hStream) {
    std::string acc("");
    char h;
    while (1) {
        hStream->RBYTE(h);
        if (h == 0) break;
        acc.append(1, h);
    }
    return acc;
}

void cIO_WWDx::SerializeTo(std::iostream *hStream) {
    char hBlockType;
    int iBlockSize = 0;
    const int iInitPos = hStream->tellp();

    StartTag: {
        hStream->WLEN(m_startTag, WWDX_START_TAG_SIZE);
    }

    MetaInfo: {
        hBlockType = BlockType_MetaInfo;
        hStream->WBYTE(hBlockType);
        iBlockSize = 4;
        hStream->WLEN(&iBlockSize, 4);

        int iTemp = 0; // temporarily set to 0, write in MetaSize
        hStream->WLEN(&iTemp, 4);
    }

    WapMapHeader: {
        hBlockType = BlockType_WapMapHeader;
        hStream->WBYTE(hBlockType);
        const int iPosBlockSize = hStream->tellp();
        iBlockSize = 0; // temporarily set to 0 and save the p position, write here later
        hStream->WLEN(&iBlockSize, 4);

        hStream->WLEN(&(hDocumentData->iWapMapBuild), 4);
        iBlockSize = 4;

        int iLen = hDocumentData->strWapMapVersion.length() + 1;
        hStream->WLEN(hDocumentData->strWapMapVersion.c_str(), iLen);
        iBlockSize += iLen;

        hStream->WLEN(&(hDocumentData->iMapBuild), 4);
        iBlockSize += 4;

        iLen = hDocumentData->strMapVersion.length() + 1;
        hStream->WLEN(hDocumentData->strMapVersion.c_str(), iLen);
        iBlockSize += iLen;

        iLen = hDocumentData->strMapDescription.length() + 1;
        hStream->WLEN(hDocumentData->strMapDescription.c_str(), iLen);
        iBlockSize += iLen;

        const int iPos = hStream->tellp();
        hStream->seekp(iPosBlockSize, std::ios_base::beg);
        hStream->WLEN(&iBlockSize, 4);
        hStream->seekp(iPos, std::ios_base::beg);
    }

    Guides: {
        const int iGuidesCount = hDocumentData->vGuides.size();
        if (iGuidesCount == 0)
            goto Favourites;

        hBlockType = BlockType_Guides;
        hStream->WBYTE(hBlockType);
        iBlockSize = 4 + 5 * iGuidesCount;
        hStream->WLEN(&iBlockSize, 4);

        hStream->WLEN(&iGuidesCount, 4);
        char cOrient;
        for (int i = 0; i < iGuidesCount; i++) {
            cOrient = hDocumentData->vGuides[i].bOrient;
            hStream->WBYTE(cOrient);
            hStream->WLEN(&(hDocumentData->vGuides[i].iPos), 4);
        }
    }

    Favourites: {
        const int iFavsCount = hDocumentData->vFavLocations.size();
        if (iFavsCount == 0)
            goto MetaEndInfo;

        hBlockType = BlockType_Favourites;
        hStream->WBYTE(hBlockType);
        iBlockSize = 0;
        const int iBlockSizePos = hStream->tellp();
        hStream->WLEN(&iBlockSize, 4); // temporarily set to 0 and save the p position, write here later
        
        hStream->WLEN(&iFavsCount, 4);
        iBlockSize += 4;

        int iLen;
        for (int i = 0; i < iFavsCount; i++) {
            const auto fav = hDocumentData->vFavLocations[i];
            iLen = strlen(fav.Name) + 1;
            hStream->WLEN(fav.Name, iLen);
            hStream->WLEN(&(fav.X), 4);
            hStream->WLEN(&(fav.Y), 4);
            iBlockSize += iLen + 4 + 4;
        }

        const int iPos = hStream->tellp();
        hStream->seekp(iBlockSizePos, std::ios_base::beg);
        hStream->WLEN(&iBlockSize, 4);
        hStream->seekp(iPos, std::ios_base::beg);
    }

    MetaEndInfo: {
        hBlockType = BlockType_MetaEndInfo;
        hStream->WBYTE(hBlockType);
        iBlockSize = 4;
        hStream->WLEN(&iBlockSize, 4);

        int iTemp = 0; // temporarily set to zero, write in MetaSize
        hStream->WLEN(&iTemp, 4);
    }

    EndTag: {
        hStream->WLEN(m_endTag, WWDX_END_TAG_SIZE);
    }

    MetaSize: {
        const int iMetaSize = (int)(hStream->tellp()) - iInitPos;
        hStream->seekp(iInitPos + WWDX_START_TAG_SIZE + WWDX_TAG_HEADER_SIZE, std::ios_base::beg);
        hStream->WLEN(&iMetaSize, 4);
        hStream->seekp(iInitPos + iMetaSize - WWDX_END_TAG_SIZE - 4, std::ios_base::beg);
        hStream->WLEN(&iMetaSize, 4);
    }
}

void cIO_WWDx::DeserializeFrom(std::istream *hStream) {
    char buff[32] = {0};
    char hBlockType;
    int iBlockSize, iMetaSize;

    hStream->clear();
    hStream->seekg(-WWDX_END_TAG_SIZE, std::ios_base::cur);
    hStream->read(buff, WWDX_END_TAG_SIZE);
    if (strncmp(buff, m_endTag, WWDX_END_TAG_SIZE))
        return;

    hStream->seekg(-WWDX_END_TAG_SIZE - 4, std::ios_base::cur);

    hStream->RINT(iMetaSize);
    m_metaSize = iMetaSize;
    if (iMetaSize < WWDX_META_MIN_SIZE)
        return;
    
    memset(buff, 0, sizeof(buff));

    hStream->seekg(WWDX_END_TAG_SIZE - iMetaSize, std::ios_base::cur);

    hStream->RLEN(buff, WWDX_START_TAG_SIZE);
    if (strncmp(buff, m_startTag, WWDX_START_TAG_SIZE))
        return;

    while (1) {
        hStream->RBYTE(hBlockType);
        WWDx_BlockType id = (WWDx_BlockType) hBlockType;
        hStream->RINT(iBlockSize);
        const int iBlockPos = hStream->tellg();

        switch (id) {
            case BlockType_MetaEndInfo:
                return;
            case BlockType_WapMapHeader: {
                hStream->RINT(hDocumentData->iWapMapBuild);
                hDocumentData->strWapMapVersion = ReadCString(hStream);
                hStream->RINT(hDocumentData->iMapBuild);
                hDocumentData->strMapVersion = ReadCString(hStream);
                hDocumentData->strMapDescription = ReadCString(hStream);
                break;
            }
            case BlockType_Guides: {
                size_t guideCount;
                hStream->RINT(guideCount);
                if (guideCount == 0)
                    break;

                char hOrient; 
                int iPos;
                for (int i = 0; i < guideCount; i++) {
                    stGuideLine guide;
                    hStream->RBYTE(guide.bOrient);
                    hStream->RINT(guide.iPos);
                    hDocumentData->vGuides.push_back(guide);
                }
                break;
            }
            case BlockType_Favourites: {
                size_t favCount;
                hStream->RINT(favCount);
                if (favCount == 0)
                    break;

                for (int i = 0; i < favCount; i++) {
                    stLocation fav;
                    std::string name = ReadCString(hStream);
                    snprintf(fav.Name, 63, "%s", name.c_str());
                    fav.Name[63] = 0;
                    hStream->RINT(fav.X);
                    hStream->RINT(fav.Y);
                    hDocumentData->vFavLocations.push_back(fav);
                }
                break;
            }
            default:
                break;
        }

        int iPos = hStream->tellg();
        if (iPos != iBlockPos + iBlockSize)
            hStream->seekg(iBlockPos + iBlockSize - iPos, std::ios_base::cur);
    }
}
