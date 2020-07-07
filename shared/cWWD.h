#ifndef WWD_HEADER
#define WWD_HEADER
#define WWD_V 6

#ifdef BUILD_DEBUG
#define WWD_EXCEPTION(errorc) WWD::Exception(errorc, __FILE__, __LINE__)
#else
#define WWD_EXCEPTION(errorc) WWD::Exception(errorc)
#endif

#define WBYTE(b) write((char*)(&b), 1)
#define WLEN(data, len) write((char*)(data), len)
#define RBYTE(b) read((char*)(&b), 1)
#define RLEN(data, len) read((char*)(data), len)
#define RINT(i) read((char*)(&i), 4)

#include <zlib.h>
#include <windows.h>
#include <string>
#include <cstring>
#include <cstdio>
#include <vector>
#include <ostream>
#include <map>
#include "cWMD.h"

#ifndef byte
typedef unsigned char byte;
#endif

#define WWD_TILE_FILL 0xEEEEEEEE
#define WWD_TILE_EMPTY 0xFFFFFFFF

class cParallelLoop;

struct structProgressInfo;
namespace WWD {
    extern structProgressInfo *_ghProgressInfo;
    extern cParallelLoop *_ghProgressCallback;

    enum ERROR_CODE {
        Error_Unknown = 0,
        Error_OpenAccess,
        Error_BadMagicNumber,
        Error_DecompressNotEnoughMem,
        Error_NoMainPlane,
        Error_InvalidTileProperty,
        Error_NotCompleteCRC,
        Error_SaveAccess,
        Error_Inflate,
        Error_Deflate,
        Error_LoadingMemory,
        Error_UnknownMapExtension
    };

#define OBJ_PARAMS_CNT 29
    enum OBJ_PARAMS {
        Param_ID = 0,
        Param_LocationX,
        Param_LocationY,
        Param_LocationZ,
        Param_LocationI,
        Param_Score,
        Param_Points,
        Param_Powerup,
        Param_Damage,
        Param_Smarts,
        Param_Health,
        Param_MinX,
        Param_MinY,
        Param_MaxX,
        Param_MaxY,
        Param_SpeedX,
        Param_SpeedY,
        Param_TweakX,
        Param_TweakY,
        Param_Counter,
        Param_Speed,
        Param_Width,
        Param_Height,
        Param_Direction,
        Param_FaceDir,
        Param_TimeDelay,
        Param_FrameDelay,
        Param_MoveResX,
        Param_MoveResY
    };

    enum OBJ_TYPE_FLAGS {
        Flag_t_Nothing = 0,
        Flag_t_Generic = 1,
        Flag_t_Player = 2,
        Flag_t_Enemy = 4,
        Flag_t_Powerup = 8,
        Flag_t_Shot = 16,
        Flag_t_PShot = 32,
        Flag_t_EShot = 64,
        Flag_t_Special = 128,
        Flag_t_User1 = 256,
        Flag_t_User2 = 512,
        Flag_t_User3 = 1024,
        Flag_t_User4 = 2048
    };

    enum OBJ_USER_FLAGS {
        Flag_u_Nothing = 0,
        Flag_u_1 = 1,
        Flag_u_2 = 2,
        Flag_u_3 = 4,
        Flag_u_4 = 8,
        Flag_u_5 = 16,
        Flag_u_6 = 32,
        Flag_u_7 = 64,
        Flag_u_8 = 128,
        Flag_u_9 = 256,
        Flag_u_10 = 512,
        Flag_u_11 = 1024,
        Flag_u_12 = 2048
    };

    enum OBJ_DRAW_FLAGS {
        Flag_dr_Nothing = 0,
        Flag_dr_NoDraw = 1,
        Flag_dr_Mirror = 2,
        Flag_dr_Invert = 4,
        Flag_dr_Flash = 8
    };

    enum OBJ_DYNAMIC_FLAGS {
        Flag_dy_Nothing = 0,
        Flag_dy_NoHit = 1,
        Flag_dy_AlwaysActive = 2,
        Flag_dy_Safe = 4,
        Flag_dy_AutoHitDamage = 8
    };

    enum OBJ_ADD_FLAGS {
        Flag_a_Nothing = 0,
        Flag_a_Difficult = 1,
        Flag_a_EyeCandy = 2,
        Flag_a_HighDetail = 4,
        Flag_a_Multiplayer = 8,
        Flag_a_ExtraMemory = 16,
        Flag_a_FastCPU = 32
    };

    enum WWD_FLAGS {
        Flag_w_Nothing = 0,
        Flag_w_UseZCoords = 1,
        Flag_w_Compress = 2
    };

    enum PLANE_FLAGS {
        Flag_p_Nothing = 0,
        Flag_p_MainPlane = 1,
        Flag_p_NoDraw = 2,
        Flag_p_XWrapping = 4,
        Flag_p_YWrapping = 8,
        Flag_p_AutoTileSize = 16
    };

    typedef enum TILE_ATTRIB : unsigned char {
        Attrib_Clear = 0,
        Attrib_Solid = 1,
        Attrib_Ground = 2,
        Attrib_Climb = 3,
        Attrib_Death = 4,
        Attrib_Unknown = 5
    };

    enum TILE_ATTRIB_TYPE {
        AttribType_Single = 1,
        AttribType_Double = 2,
        AttribType_Mask = 3
    };

    class Rect;

    typedef std::pair<TILE_ATTRIB, Rect> CollisionRect;
#define WWD_CR_TYPE first
#define WWD_CR_RECT second

    enum GAME {
        Game_Unknown = 0,
        Game_Claw = 1,
        Game_GetMedieval = 2,
        Game_Gruntz = 3,
        Game_Claw2 = 4,

        Games_First = Game_Claw,
        Games_Last = Game_Claw2,
        Games_Count = Games_Last,
    };

    void operator++ (GAME& game);

    static std::map<WWD::GAME, const char*> GAME_NAMES({
        {Game_Claw, "Claw"},
        {Game_GetMedieval, "Get Medieval"},
        {Game_Gruntz, "Gruntz"},
        {Game_Claw2, "Claw 2"}
    });

    class CustomMetaSerializer {
    protected:

    public:
        virtual ~CustomMetaSerializer() {};

        virtual void SerializeTo(std::iostream *hStream) {};

        virtual void DeserializeFrom(std::istream *hStream) {};
    };

    class Exception {
    public:
        ERROR_CODE iErrorCode;
#ifdef BUILD_DEBUG
        char * szFile;
        int iLine;
        Exception(ERROR_CODE errorc, char * pszFile, int piLine){ iLine = piLine; iErrorCode = errorc; szFile = new char[strlen(pszFile)+1]; strcpy(szFile, pszFile); };
        ~Exception(){ if( szFile != NULL ) delete [] szFile; }
#else

        Exception(ERROR_CODE errorc) { iErrorCode = errorc; };

        ~Exception() {};
#endif
    };

    class Tile {
    private:
        short m_iID;
        bool m_bInvisible, m_bFilled;

        friend class Plane;

        friend class Parser;

    public:
        Tile() {
            m_iID = -1;
            m_bFilled = 0;
            m_bInvisible = 1;
        };

        bool IsInvisible() { return m_bInvisible; };

        bool IsFilled() { return m_bFilled; };

        int GetID() { return m_iID; };

        void SetID(short piID) {
            m_iID = piID;
            m_bInvisible = m_bFilled = 0;
        };

        void SetInvisible(bool bP) {
            m_bInvisible = bP;
            if (bP) {
                m_bFilled = 0;
                m_iID = -1;
            }
        };

        void SetFilled(bool bP) {
            m_bFilled = bP;
            if (bP) {
                m_bInvisible = 0;
                m_iID = -1;
            }
        };

        Tile &operator=(const Tile &src);

        bool operator==(const Tile &src);

        bool operator!=(const Tile &src);
    };

    class Rect {
    public:
        int x1, y1, x2, y2;

        Rect() { x1 = y1 = x2 = y2 = 0; };

        Rect(int nx1, int ny1, int nx2, int ny2) {
            x1 = nx1;
            y1 = ny1;
            x2 = nx2;
            y2 = ny2;
        };

        void Set(Rect src) {
            x1 = src.x1;
            y1 = src.y1;
            x2 = src.x2;
            y2 = src.y2;
        };

        bool Collide(Rect b);

        bool operator==(const Rect &oth) const {
            return (x1 == oth.x1 && y1 == oth.y1 && x2 == oth.x2 && y2 == oth.y2);
        };

        bool operator!=(const Rect &oth) const { return !(*this == oth); };
    };

    class TileAttrib {
    protected:
        int m_iW, m_iH;
    public:
        TileAttrib();
        TileAttrib(int pW, int pH) : m_iW(pW), m_iH(pH) {}
        virtual ~TileAttrib() {}

        virtual bool operator!=(TileAttrib& other) = 0;

        int getWidth() { return m_iW; }

        int getHeight() { return m_iH; }

        void setWidth(int w) { m_iW = w; }

        void setHeight(int h) { m_iH = h; }

        virtual TILE_ATTRIB_TYPE getType() = 0;
        virtual std::string toString() = 0;
        virtual void readFromStream(std::istream* psSource) = 0;
        virtual void compileToStream(std::iostream* psDestination) = 0;
        virtual std::vector<TILE_ATTRIB> getAttribSummary() = 0;
    };

    class SingleTileAttrib : public TileAttrib {
    private:
        TILE_ATTRIB attrib;
    public:
        SingleTileAttrib() {}

        SingleTileAttrib(SingleTileAttrib* other) : TileAttrib(other->m_iW, other->m_iH), attrib(other->attrib) {}

        SingleTileAttrib(int pW, int pH, TILE_ATTRIB attr) : TileAttrib(pW, pH), attrib(attr) {}

        bool operator!=(TileAttrib& other) override;

        TILE_ATTRIB_TYPE getType() override { return AttribType_Single; }

        TILE_ATTRIB getAttrib() { return attrib; }

        void setAttrib(TILE_ATTRIB atr) { attrib = atr; }

        std::string toString() override;
        void readFromStream(std::istream* psSource) override;
        void compileToStream(std::iostream* psDestination) override;
        std::vector<TILE_ATTRIB> getAttribSummary() override;
    };

    class DoubleTileAttrib : public TileAttrib {
    private:
        TILE_ATTRIB m_iAttribOutside, m_iAttribInside;
        Rect m_rMask;

        friend class Parser;

    public:
        DoubleTileAttrib() {}

        DoubleTileAttrib(DoubleTileAttrib* other) : TileAttrib(other->m_iW, other->m_iH), m_iAttribInside(other->m_iAttribInside),
            m_iAttribOutside(other->m_iAttribOutside), m_rMask(other->m_rMask) {}

        DoubleTileAttrib(int pW, int pH, TILE_ATTRIB pIns, TILE_ATTRIB pOut = Attrib_Clear,
            Rect pMask = Rect(0, 0, 0, 0)) : TileAttrib(pW, pH), m_iAttribInside(pIns), m_iAttribOutside(pOut), m_rMask(pMask) {}

        bool operator!=(TileAttrib& other) override;

        TILE_ATTRIB_TYPE getType() override { return AttribType_Double; }

        std::string toString() override;
        void readFromStream(std::istream* psSource) override;
        void compileToStream(std::iostream* psDestination) override;

        std::vector<TILE_ATTRIB> getAttribSummary() override;

        TILE_ATTRIB getInsideAttrib() { return m_iAttribInside; }

        void setInsideAttrib(TILE_ATTRIB atr) { m_iAttribInside = atr; }

        TILE_ATTRIB getOutsideAttrib() { return m_iAttribOutside; };

        void setOutsideAttrib(TILE_ATTRIB atr) { m_iAttribOutside = atr; };

        Rect getMask() { return m_rMask; };

        void setMask(int x1, int y1, int x2, int y2) {
            m_rMask.x1 = x1;
            m_rMask.y1 = y1;
            m_rMask.x2 = x2;
            m_rMask.y2 = y2;
        };
    };

    class MaskTileAttrib : public TileAttrib {
    private:
        unsigned size;
        TILE_ATTRIB *data = nullptr;
    public:
        MaskTileAttrib() {}

        MaskTileAttrib(int pW, int pH) : TileAttrib(pW, pH) {
            size = pW * pH;
            data = new TILE_ATTRIB[size]{};
        }

        MaskTileAttrib(MaskTileAttrib* other) : TileAttrib(other->m_iW, other->m_iH), size(other->size) {
            data = new TILE_ATTRIB[size];
            for (int i = 0; i < size; ++i) data[i] = other->data[i];
        }

        ~MaskTileAttrib() override;

        bool operator!=(TileAttrib& other) override;

        TILE_ATTRIB_TYPE getType() override { return AttribType_Mask; }
        
        const TILE_ATTRIB* getData() const { return data; }

        void setArea(int x1, int y1, int x2, int y2, WWD::TILE_ATTRIB attrib);

        std::string toString() override;
        void readFromStream(std::istream* psSource) override;
        void compileToStream(std::iostream* psDestination) override;

        std::vector<TILE_ATTRIB> getAttribSummary() override;
    };

    class Object {
    private:
        OBJ_ADD_FLAGS m_iFlagsAdd;
        OBJ_DYNAMIC_FLAGS m_iFlagsDynamic;
        OBJ_DRAW_FLAGS m_iFlagsDraw;
        OBJ_USER_FLAGS m_iFlagsUser;
        OBJ_TYPE_FLAGS m_iFlagsType;
        OBJ_TYPE_FLAGS m_iFlagsHitType;
        int m_iParams[OBJ_PARAMS_CNT], m_iUserValues[8];
        Rect m_rMove, m_rHit, m_rAttack, m_rClip, m_rUser[2];
        char *m_szName, *m_szLogic, *m_szImageSet, *m_szAnim;
        void *m_hUserData;

        friend class Parser;

        friend class Plane;

    public:
        Object();

        Object(Object *src);

        ~Object();

        OBJ_ADD_FLAGS GetAddFlags() { return m_iFlagsAdd; };

        OBJ_DYNAMIC_FLAGS GetDynamicFlags() { return m_iFlagsDynamic; };

        OBJ_DRAW_FLAGS GetDrawFlags() { return m_iFlagsDraw; };

        OBJ_USER_FLAGS GetUserFlags() { return m_iFlagsUser; };

        OBJ_TYPE_FLAGS GetTypeFlags() { return m_iFlagsType; };

        OBJ_TYPE_FLAGS GetHitTypeFlags() { return m_iFlagsHitType; };

        const char *GetName() { return (const char *) m_szName; };

        const char *GetLogic() { return (const char *) m_szLogic; };

        const char *GetImageSet() { return (const char *) m_szImageSet; };

        const char *GetAnim() { return (const char *) m_szAnim; };

        int GetParam(OBJ_PARAMS Param) { return m_iParams[Param]; };

        int GetUserValue(int i) { return m_iUserValues[i]; };

        Rect GetMoveRect() { return m_rMove; };

        Rect GetHitRect() { return m_rHit; };

        Rect GetAttackRect() { return m_rAttack; };

        Rect GetClipRect() { return m_rClip; };

        Rect GetUserRect(int i) { return m_rUser[i]; };

        void SetUserData(void *hPtr) { m_hUserData = hPtr; };

        void SetName(const char *nname);

        void SetLogic(const char *nlogic);

        void SetImageSet(const char *niset);

        void SetAnim(const char *nanim);

        void SetParam(OBJ_PARAMS Param, int iVal) { m_iParams[Param] = iVal; };

        void SetUserValue(int i, int v) { m_iUserValues[i] = v; };

        void SetMoveRect(Rect n) { m_rMove = n; };

        void SetHitRect(Rect n) { m_rHit = n; };

        void SetAttackRect(Rect n) { m_rAttack = n; };

        void SetClipRect(Rect n) { m_rClip = n; };

        void SetUserRect(int i, Rect n) { m_rUser[i] = n; };

        void SetAddFlags(OBJ_ADD_FLAGS n) { m_iFlagsAdd = n; };

        void SetDynamicFlags(OBJ_DYNAMIC_FLAGS n) { m_iFlagsDynamic = n; };

        void SetDrawFlags(OBJ_DRAW_FLAGS n) { m_iFlagsDraw = n; };

        void SetUserFlags(OBJ_USER_FLAGS n) { m_iFlagsUser = n; };

        void SetTypeFlags(OBJ_TYPE_FLAGS n) { m_iFlagsType = n; };

        void SetHitTypeFlags(OBJ_TYPE_FLAGS n) { m_iFlagsHitType = n; };

        void *GetUserData() { return m_hUserData; };

        bool ShouldPromptForRectChange(WWD::Parser* hParser);

        int GetX();

        int GetY();

        int GetZ();

        int GetI();

        bool GetFlipX();

        bool GetFlipY();

        void SetFlip(bool x, bool y);
    };

    struct PlaneHeader {
        int size;
        int null;
        int m_iFlags;
        int null2;
        char m_szName[64];
        int m_iWpx, m_iHpx, m_iTileW, m_iTileH, m_iW, m_iH,
        m_iOffsetX, m_iOffsetY, m_iMoveX, m_iMoveY, m_iFillColor;
        unsigned m_iSetsCount, m_iObjectsCount, m_iTilesDataPtr, m_iSetsDataPtr, m_iObjectsDataPtr;
        int m_iZCoord, null3, null4, null5;
    };

    static_assert(sizeof(PlaneHeader) == 160, "Bad plane header size!");

    class Plane {
    private:
        PlaneHeader m_Header;
        std::vector<std::string> m_vImageSets;
        Tile *m_hTiles; unsigned *rowOffsets;
        std::vector<Object *> m_vObjects;

        ~Plane();

        void (*m_hObjDeletionCB)(Object *obj);

        friend class Parser;

    public:
        Plane(bool zeroFillHeader = true) {
            m_hTiles = NULL;
            if (zeroFillHeader) {
                SecureZeroMemory(&m_Header.size, sizeof(PlaneHeader));
                m_Header.size = sizeof(PlaneHeader);
            }
        };

        PLANE_FLAGS GetFlags() { return (PLANE_FLAGS)m_Header.m_iFlags; };

        void SetFlag(PLANE_FLAGS piFlag, bool pbValue);

        bool GetFlag(PLANE_FLAGS piFlag);

        void SetName(const char *n) { strncpy(m_Header.m_szName, n, 64); };

        const char *GetName() { return m_Header.m_szName; };

        void SetZCoord(int n) { m_Header.m_iZCoord = n; };

        int GetZCoord() { return m_Header.m_iZCoord; };

        int GetFillColor() { return m_Header.m_iFillColor; };

        void SetTileWidth(int n) {
            m_Header.m_iTileW = n;
            m_Header.m_iWpx = m_Header.m_iW * n;
        };

        int GetTileWidth() { return m_Header.m_iTileW; };

        void SetTileHeight(int n) {
            m_Header.m_iTileH = n;
            m_Header.m_iHpx = m_Header.m_iH * n;
        };

        int GetTileHeight() { return m_Header.m_iTileH; };

        int GetPlaneWidth() { return m_Header.m_iW; };

        int GetPlaneHeight() { return m_Header.m_iH; };

        int GetPlaneWidthPx() { return m_Header.m_iWpx; };

        int GetPlaneHeightPx() { return m_Header.m_iHpx; };

        void SetMoveModX(int n) { m_Header.m_iMoveX = n; };

        int GetMoveModX() { return m_Header.m_iMoveX; };

        void SetMoveModY(int n) { m_Header.m_iMoveY = n; };

        int GetMoveModY() { return m_Header.m_iMoveY; };

        void SetOffsetX(int n) { m_Header.m_iOffsetX = n; };

        int GetOffsetX() { return m_Header.m_iOffsetX; };

        void SetOffsetY(int n) { m_Header.m_iOffsetY = n; };

        int GetOffsetY() { return m_Header.m_iOffsetY; };

        int GetObjectsCount() { return m_vObjects.size(); };

        int ClampX(int x) { return x % (m_Header.m_iW); };

        int ClampY(int y) { return y % (m_Header.m_iH); };

        const char *GetImageSet(int piID) { return piID < m_vImageSets.size() ? m_vImageSets[piID].c_str() : NULL; };

        int GetImageSetsCount() { return m_vImageSets.size(); };

        void AddImageSet(const char *n);

        void ClearImageSets();

        void AddObjectAndCalcID(Object *n);

        Object *GetObjectByObjectID(int piID) {
            for (int i = 0; i < GetObjectsCount(); i++)
                if (m_vObjects[i]->GetParam(Param_ID) == piID)return m_vObjects[i];
            return NULL;
        };

        Object *GetObjectByIterator(int piID) { return m_vObjects[piID]; };

        void DeleteObject(Object *ptr);

        void DeleteObjectByIterator(int piID);

        void DeleteObjectByID(int piID);

        void SetObjectDeletionCallback(void (*nCB)(Object *obj)) { m_hObjDeletionCB = nCB; };

        void DeleteObjectFromList(Object *ptr);

        void DeleteObjectFromListByIterator(int piID);

        void DeleteObjectFromListByID(int piID);

        void SetFillColor(int iColor) { if (iColor >= 0 && iColor < 256) m_Header.m_iFillColor = iColor; };

        Tile* GetTile(int piX, int piY);
        Tile* GetTile(int it);

        void Resize(int nw, int nh, int ox = 0, int oy = 0, bool creating = false);

        void ResizeAddTiles(int ax, int ay);

        void ResizeAnchor(int nw, int nh, int anchor);
    };

    struct WWDHeader
    {
        unsigned size;
        int null;
        int m_iFlags;
        int null2;
        char m_szMapName[64];
        char m_szAuthor[64];
        char m_szDate[64];
        char m_szRezPath[256];
        char m_szTilesPath[128];
        char m_szPalPath[128];
        int m_iStartX;
        int m_iStartY;
        int null3;
        unsigned m_iPlanesCount;
        unsigned m_hPlanesStart;
        unsigned m_hTileAttributesStart;
        unsigned unpackedSize;
        int checksum;
        int null4;
        char m_szExePath[128];
        char m_szImageSets[4][128];
        char m_szSetsPrefixes[4][32];
    };

    static_assert(sizeof(WWDHeader) == 1524, "Bad WWD header size!");

    class Parser {
    private:
        CustomMetaSerializer *hMetaSerializer;
        std::vector<TileAttrib *> m_hTileAttribs;
        std::vector<Plane *> m_hPlanes;
        char m_szFile[MAX_PATH];
        WWDHeader m_Header;
        Plane *mainPlane = NULL;
        int m_iBaseLevel;
        GAME m_iGame;

        void LoadFileHeader(std::istream *psSource);

        void LoadFromStream(std::istream *psSource);

        void WriteFlag(int piFlag, std::ostream *psDestination);

        void WriteRect(Rect *phRect, std::ostream *psDestination);

        void WriteObject(Object *hObj, std::ostream *psDestination);

        void MoveBytes(std::ostream *psStream, int c);

        size_t ReadObject(Object *hObj, std::istream *psSource);

        void ReadRect(Rect *phRect, std::istream *psSource);

        int FormFlag(byte b1, byte b2);

        void CleanStr(char *pszStr, int piSize);

        unsigned int CalculateChecksum(std::istream *psStream, int piOffset);

        void Inflate(std::istream *psSource, std::stringstream& output);

        void Deflate(std::istream *psSource, std::ostream *psDest, int iLevel = 0);
        //void PerformStep();
    public:
        Parser(const char *pszFilename, CustomMetaSerializer *hSerializer = 0);

        Parser(void *ptr, uint32_t iLen, CustomMetaSerializer *hSerializer = 0);

        //void Step(int piIterations = 1);
        ~Parser();

        Plane* GetMainPlane() { return mainPlane; }

        const char *GetName() { return (const char *) m_Header.m_szMapName; };

        void SetName(const char *nname) { strncpy(m_Header.m_szMapName, nname, 64); };

        const char *GetAuthor() { return (const char *) m_Header.m_szAuthor; };

        void SetAuthor(const char *nauthor) { strncpy(m_Header.m_szAuthor, nauthor, 64); };

        const char *GetDate() { return (const char *) m_Header.m_szDate; };

        void UpdateDate();

        const char *GetRezPath() { return (const char *) m_Header.m_szRezPath; };

        void SetRezPath(const char *n) { strncpy(m_Header.m_szRezPath, n, 256); };

        const char *GetTilesPath() { return (const char *) m_Header.m_szTilesPath; };

        void SetTilesPath(const char *n) { strncpy(m_Header.m_szTilesPath, n, 128); };

        const char *GetPalettePath() { return (const char *) m_Header.m_szPalPath; };

        void SetPalettePath(const char *n) { strncpy(m_Header.m_szPalPath, n, 128); };

        const char *GetExePath() { return (const char *) m_Header.m_szExePath; };

        void SetExePath(const char *n) { strncpy(m_Header.m_szExePath, n, 128); };

        const char *GetImageSet(int piID) { return (const char *) m_Header.m_szImageSets[piID]; };

        void SetImageSet(int id, const char *npath);

        const char *GetImageSetPrefix(int piID) { return (const char *) m_Header.m_szSetsPrefixes[piID]; };

        void SetImageSetPrefix(int id, const char *npref);

        const char *GetFilePath() { return (const char *) m_szFile; };

        void SetFilePath(const char *nPath);

        int GetStartX() { return m_Header.m_iStartX; };

        int GetStartY() { return m_Header.m_iStartY; };

        void SetStartX(int x) { m_Header.m_iStartX = x; };

        void SetStartY(int y) { m_Header.m_iStartY = y; };

        int GetBaseLevel() { return m_iBaseLevel; };

        WWD_FLAGS GetFlags() { return (WWD_FLAGS)m_Header.m_iFlags; };

        int GetPlanesCount() { return m_hPlanes.size(); };

        Plane *GetPlane(int piID) {
            if (piID < 0 || piID >= int(m_hPlanes.size())) return NULL;
            return m_hPlanes[piID];
        };

        void AddPlane(WWD::Plane *np);

        void DeletePlane(int piID);

        void SortPlanes();

        void CompileToStream(std::iostream *psDestination);

        void CompileToFile(const char *pszFilename, bool pbWithActualDate = 1);

        TileAttrib *GetTileAttribs(int piTile) {
            if (piTile < 0 || piTile >= int(m_hTileAttribs.size())) return NULL;
            return m_hTileAttribs[piTile];
        };

        void SetTileAttribs(int piTile, TileAttrib *htaAttribs);

        int GetTileAttribsCount() { return m_hTileAttribs.size(); };

        void SetTileAttribsCount(int i);

        void SetFlag(WWD_FLAGS piFlag, bool pbValue);

        bool GetFlag(WWD_FLAGS piFlag);

        GAME GetGame() { return m_iGame; };

        CustomMetaSerializer *GetCustomMetaSerializer() { return hMetaSerializer; };

        friend void WMD::ExportTileProperties(WWD::Parser* hParser, std::ofstream& ofstream);
    };

    GAME GetGameTypeFromFile(const char *pszFilepath, int *piBaseLevel = NULL) throw(Exception);
}

#endif
