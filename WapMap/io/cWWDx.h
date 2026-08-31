#ifndef IO_WWDX
#define IO_WWDX

#include "../../shared/cWWD.h"

#define WWDX_START_TAG_SIZE 9
#define WWDX_END_TAG_SIZE 10
#define WWDX_TAG_HEADER_SIZE 5
#define WWDX_META_MIN_SIZE 37

struct DocumentData;

class cIO_WWDx : public WWD::CustomMetaSerializer {
private:
    DocumentData *hDocumentData;

    std::string ReadCString(std::istream *hStream);

    const char m_startTag[WWDX_START_TAG_SIZE] = {'<', 'w', 'm', '_', 'm', 'e', 't', 'a', '>'};

    const char m_endTag[WWDX_END_TAG_SIZE] = {'<', '/', 'w', 'm', '_', 'm', 'e', 't', 'a', '>'};

    int m_metaSize = 0;

public:
    cIO_WWDx(DocumentData *dd) {hDocumentData = dd;};

    ~cIO_WWDx() {};

    virtual void SerializeTo(std::iostream *hStream) override;

    virtual void DeserializeFrom(std::istream *hStream) override;

    unsigned int getSize() override { return m_metaSize; }
};

/*
WWD extended meta
Data types:
    zstring - null-terminated string

General:
    Starting tag:
        "<wm_meta>" - WITHOUT null at the end.

    Blocks:
        Every block starts with the block header:
            typedef struct {
                byte ID;
                size_t dataSize;
            } metaHeader;

        Which is followed by the block's data.

        If a block's type (ID) is not supported, just skip forward by the length of its data size.

        First block must be MetaInfo (ID = 0), the last MetaEndInfo (ID = 1),
        whereas in between them can be any amount of blocks in any order.

        Blocks of the same type should not repeat - that is only one block of a given type should exist.

    End tag:
        "</wm_meta>" - WITHOUT null at the end.

    Block types:

        MetaInfo (ID = 0, Size = 4):
            size_t metaSize; // includes the starting and ending tags

        MetaEndInfo (ID = 1, Size = 4):
            size_t metaSize; // includes the starting and ending tags

        Header (ID = 2, Size = varying):
            int wapMapBuild;
            zstring wapMapVersion;
            int wwdBuild;
            zstring wwdVersion;
            zstring wwdDescription;
        
        Guides (ID = 3, Size = varying):
            size_t guidesNumber;
            struct {
                char orientation; // 1 - horizontal, 0 - vertical
                unsigned int position;
            } guide[guidesNumber];

        Favourite locations (ID = 4, Size = varying):
            size_t favsNumber;
            struct {
                zstring name; // max. 64 bytes
                unsigned int positionX;
                unsigned int positionY;
            } favourite[favsNumber];

*/

#endif // IO_WWDX
