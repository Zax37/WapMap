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

#ifndef byte
typedef unsigned char byte;
#endif

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
        Error_LoadingMemory
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

    enum TILE_ATRIB {
        Atrib_Clear = 0,
        Atrib_Solid = 1,
        Atrib_Ground = 2,
        Atrib_Climb = 3,
        Atrib_Death = 4
    };

    enum TILE_ATRIBTYPE {
        AtribType_Single = 1,
        AtribType_Double = 2
    };

    class Rect;

    typedef std::pair<TILE_ATRIB, Rect> CollisionRect;
#define WWD_CR_TYPE first
#define WWD_CR_RECT second

    enum GAME {
        Game_Unknown = 0,
        Game_Claw = 1,
        Game_GetMedieval = 2,
        Game_Gruntz = 3
    };

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

    class TileAtrib {
    private:
        TILE_ATRIBTYPE m_iType;
        int m_iW, m_iH;
        TILE_ATRIB m_iAtribOutside, m_iAtribInside;
        Rect m_rMask;

        friend class Parser;

    public:
        ~TileAtrib();

        TileAtrib(TileAtrib *src);

        TileAtrib();

        TileAtrib(int pW, int pH, TILE_ATRIBTYPE pType, TILE_ATRIB pIns, TILE_ATRIB pOut = Atrib_Clear,
                  Rect pMask = Rect(0, 0, 0, 0));

        void SetTo(TileAtrib *src);

        TILE_ATRIBTYPE GetType() { return m_iType; };

        void SetType(TILE_ATRIBTYPE atr) { m_iType = atr; };

        TILE_ATRIB GetAtribInside() { return m_iAtribInside; };

        void SetAtribInside(TILE_ATRIB atr) { m_iAtribInside = atr; };

        int GetW() { return m_iW; };

        int GetH() { return m_iH; };

        void SetW(int w) { m_iW = w; };

        void SetH(int h) { m_iH = h; };

        //double only
        TILE_ATRIB GetAtribOutside() { return m_iAtribOutside; };

        void SetAtribOutside(TILE_ATRIB atr) { m_iAtribOutside = atr; };

        Rect GetMask() { return m_rMask; };

        void SetMask(int x1, int y1, int x2, int y2) {
            m_rMask.x1 = x1;
            m_rMask.y1 = y1;
            m_rMask.x2 = x2;
            m_rMask.y2 = y2;
        };

        std::vector<CollisionRect> GetColRects();
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
    };

    class Plane {
    private:
        PLANE_FLAGS m_iFlags;
        char m_szName[64];
        std::vector<char *> m_vImageSets;
        Tile **m_hTiles;
        std::vector<Object *> m_vObjects;
        int m_iZCoord, m_iFillColor, m_iMoveX, m_iMoveY, m_iW, m_iH, m_iTileW, m_iTileH, m_iSetsCount, m_iWpx, m_iHpx;

        ~Plane();

        void (*m_hObjDeletionCB)(Object *obj);

        friend class Parser;

    public:
        Plane() { m_hTiles = NULL; };

        PLANE_FLAGS GetFlags() { return m_iFlags; };

        void SetFlag(PLANE_FLAGS piFlag, bool pbValue);

        bool GetFlag(PLANE_FLAGS piFlag);

        void SetName(const char *n) { strncpy(m_szName, n, 64); };

        const char *GetName() { return m_szName; };

        void SetZCoord(int n) { m_iZCoord = n; };

        int GetZCoord() { return m_iZCoord; };

        int GetFillColor() { return m_iFillColor; };

        void SetTileWidth(int n) {
            m_iTileW = n;
            m_iWpx = m_iW * n;
        };

        int GetTileWidth() { return m_iTileW; };

        void SetTileHeight(int n) {
            m_iTileH = n;
            m_iHpx = m_iH * n;
        };

        int GetTileHeight() { return m_iTileH; };

        int GetPlaneWidth() { return m_iW; };

        int GetPlaneHeight() { return m_iH; };

        int GetPlaneWidthPx() { return m_iWpx; };

        int GetPlaneHeightPx() { return m_iHpx; };

        void SetMoveModX(int n) { m_iMoveX = n; };

        int GetMoveModX() { return m_iMoveX; };

        void SetMoveModY(int n) { m_iMoveY = n; };

        int GetMoveModY() { return m_iMoveY; };

        int GetObjectsCount() { return m_vObjects.size(); };

        int ClampX(int x) { return x % (m_iW); };

        int ClampY(int y) { return y % (m_iH); };

        const char *GetImageSet(int piID) { return m_vImageSets[piID]; };

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

        void SetFillColor(int iColor) { if (iColor >= 0 && iColor < 256) m_iFillColor = iColor; };

        Tile *GetTile(int piX, int piY);

        void Resize(int nw, int nh);

        void ResizeRelative(int rx, int ry, bool ax, bool ay);

        void ResizeAnchor(int rx, int ry, int anchor);
    };

    class Parser {
    private:
        CustomMetaSerializer *hMetaSerializer;
        std::vector<TileAtrib *> m_hTileAtribs;
        std::vector<Plane *> m_hPlanes;
        char m_szFile[MAX_PATH];
        char m_szMapName[64], m_szAuthor[64], m_szDate[64], m_szRezPath[256], m_szTilesPath[128], m_szPalPath[128], m_szExePath[128];
        char m_szImageSets[4][128], m_szSetsPrefixes[4][32];
        int m_iStartX, m_iStartY;
        WWD_FLAGS m_iFlags;
        int m_iBaseLevel;
        GAME m_iGame;
        int m_iPlanesCount;

        void LoadFileHeader(std::istream *psSource);

        void LoadFromStream(std::istream *psSource);

        void WriteFlag(int piFlag, std::ostream *psDestination);

        void WriteRect(Rect *phRect, std::ostream *psDestination);

        void WriteObject(Object *hObj, std::ostream *psDestination);

        void MoveBytes(std::ostream *psStream, int c);

        void ReadObject(Object *hObj, std::istream *psSource);

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

        const char *GetName() { return (const char *) m_szMapName; };

        void SetName(const char *nname) { strncpy(m_szMapName, nname, 64); };

        const char *GetAuthor() { return (const char *) m_szAuthor; };

        void SetAuthor(const char *nauthor) { strncpy(m_szAuthor, nauthor, 64); };

        const char *GetDate() { return (const char *) m_szDate; };

        void UpdateDate();

        const char *GetRezPath() { return (const char *) m_szRezPath; };

        void SetRezPath(const char *n) { strncpy(m_szRezPath, n, 256); };

        const char *GetTilesPath() { return (const char *) m_szTilesPath; };

        void SetTilesPath(const char *n) { strncpy(m_szTilesPath, n, 128); };

        const char *GetPalettePath() { return (const char *) m_szPalPath; };

        void SetPalettePath(const char *n) { strncpy(m_szPalPath, n, 128); };

        const char *GetExePath() { return (const char *) m_szExePath; };

        void SetExePath(const char *n) { strncpy(m_szExePath, n, 128); };

        const char *GetImageSet(int piID) { return (const char *) m_szImageSets[piID]; };

        void SetImageSet(int id, const char *npath);

        const char *GetImageSetPrefix(int piID) { return (const char *) m_szSetsPrefixes[piID]; };

        void SetImageSetPrefix(int id, const char *npref);

        const char *GetFilePath() { return (const char *) m_szFile; };

        void SetFilePath(const char *nPath);

        int GetStartX() { return m_iStartX; };

        int GetStartY() { return m_iStartY; };

        void SetStartX(int x) { m_iStartX = x; };

        void SetStartY(int y) { m_iStartY = y; };

        int GetBaseLevel() { return m_iBaseLevel; };

        WWD_FLAGS GetFlags() { return m_iFlags; };

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

        TileAtrib *GetTileAtribs(int piTile) {
            if (piTile < 0 || piTile >= int(m_hTileAtribs.size())) return NULL;
            return m_hTileAtribs[piTile];
        };

        void SetTileAtribs(int piTile, TileAtrib *htaAtribs) { m_hTileAtribs[piTile]->SetTo(htaAtribs); };

        int GetTileAtribsCount() { return m_hTileAtribs.size(); };

        void SetTileAtribsCount(int i);

        void SetFlag(WWD_FLAGS piFlag, bool pbValue);

        bool GetFlag(WWD_FLAGS piFlag);

        GAME GetGame() { return m_iGame; };

        CustomMetaSerializer *GetCustomMetaSerializer() { return hMetaSerializer; };
    };

    GAME GetGameTypeFromFile(const char *pszFilepath, int *piBaseLevel = NULL) throw(Exception);
}

#endif
/**
16b - naglowek
F4 05 (244, 05) - sygnatura wwd
na 8b flagi (za pomoca obrotu bitowego):
 0x01 - use z coords
 0x02 - compress
na 16b - nazwa poziomu [64]
na 80b - autor [64]
na 144b - data utworzenia [64]
na 204b - sciezka do reza [256]
na 464b - sciezka do folderu z klockami w rezie [128b]
na 592b - sciezka do palety w rezie [128b]
na 720b - start x [4b]
na 724b - start y [4b]
na 728b - ? [b]
na 732b - ilosc plane'ow [4b]
na 736b - wskaznik na bajt na ktorym zaczynaja sie plane'y [4b]
na 740b - wskaznik na bajt na ktorym koncza sie plane'y [4b]
na 744b - checksuma tylko przy kompresji: ilosc bajtow po wypakowaniu [4b]
na 748b - checksuma (szczeg. impl. funkcja CalculateChecksum) [4b]
na 752b - ?

na 756b - sciezka do exe clawa [128] //relatywna do pozycji .wwd na dysku
na 884b - image set 1 [128]
na 1012b - image set 2 [128]
na 1140b - image set 3 [128]
na 1268b - image set 4 [128]
na 1396b - prefix 1 [32]
na 1428b - prefix 2 [32]
na 1460b - prefix 3 [32]
na 1492b - prefix 4 [32]

//jesli jest kompresja:
na 1524b: 0x78 (120)
na 1525b: 0x9C (156)

//plany: [160b naglowek]
na 0b: 0x20 (32)
na 8b: flagi [1]:
 0x01 - main plane
 0x02 - no draw
 0x04 - x wrapping
 0x08 - y wrapping
 0x16 - auto tile size
na 16b: nazwa [64]
na 80b: szerokosc w px [4]
na 84b: wysokosc w px [4]
na 88b: tile width int [4]
na 92b: tile height int [4]
na 96b: tiles wide int [4]
na 100b: tiles high int [4]
na 104b: nieznany int [4]------------!
na 108b: nieznany int [4]------------!
na 112b: move x percent int [4]
na 116b: move y percent int [4]
na 120b: fill color int [4]
na 124b: ilosc image setow [4]
na 128b: ilosc obiektow [4]
na 132b: adres do klockow [4]
na 136b: adres do stringow image sets [4]
na 140b: adres do obiektow jesli main plane, inaczej 0 [4]
na 144b: z coord [4]
[12b - 3 inty?]

//kafle planow [w*h*4] kolejnosc tak jak kolejnosc planow
kazdy kafel 4b:
id kafla = a+b*256, gdzie a to bajt pierwszy, a b to bajt drugi
jesli niewidoczny, wszystkie cztery bajty 0xFF (255)
jesli wypelniony, wszystkie cztery bajty 0xEE (238)

na 160*ilosc planow + suma szerokosci * suma wysokosci * 4: image sets [stringi null terminated] w kolejnosci tak jak plany (ilosc predef)

!note: recty sa zawsze podawane w kolejnosci lewo-gora-prawo-dol (clockwise od lewa)!
//obiekty
na 0b: ID obiektu int [4b]
na 4b: dlugosc nazwy obiektu int [4b]
na 8b: dlugosc nazwy logiki int [4b]
na 12b: dlugosc nazwy imagesetu int [4b]
na 16b: location - x int [4b]
na 20b: location - y int [4b]
na 24b: location - z int [4b]
na 28b: location - i int [4b]
na 32b: add flags [1b]
 1 - difficult
 2 - eye candy
 4 - high detail
 8 - multiplayer
 16 - extra memory
 32 - fast cpu
[3b przerwy?]
na 36b: dynamic flags [1b]:
 1 - no hit
 2 - always active
 4 - safe
 8 - auto hit damage
[3b przerwy?]
na 40b: draw flags [1b]:
 1 - no draw
 2 - mirror
 4 - invert
 8 - flash
[3b przerwy?]
na 44b: user flags p.1 [1b]:
 1 - flag 1
 2 - flag 2
 4 - flag 3
 8 - flag 4
 16 - flag 5
 32 - flag 6
 64 - flag 7
 128 - flag 8
na 45b: user flags p.2 [1b]:
 1 - flag 9
 2 - flag 10
 4 - flag 11
 8 - flag 12
[2b przerwy?]
na 48b: atrybut score int [4b]
na 52b: atrybut points int [4b]
na 56b: atrybut powerup int [4b]
na 60b: atrybut damage int [4b]
na 64b: atrybut smarts int [4b]
na 68b: atrybut health int [4b]

na 72b: rect move 4xint [16b]
na 86b: rect hit 4xint [16b]
na 102b: rect attack 4xint [16b]
na 118b: rect clip 4xint [16b]
na 134b: rect user1 4xint [16b]
na 150b: rect user2 4xint [16b]
na 166b: user 1-8 8xint [32b]
na 198b: atrybut x min int [4b]
na 202b: atrybut y min int [4b]
na 206b: atrybut x max int [4b]
na 210b: atrybut y max int [4b]
na 214b: atrybut speed x int [4b]
na 218b: atrybut speed y int [4b]
na 222b: x tweak int [4b]
na 226b: y tweak int [4b]
na 230b: counter int [4b]
na 234b: speed int [4b]
na 238b: width int [4b]
na 242b: height int [4b]
na 246b: direction int [4b]
na 250b: face dir int [4b]
na 254b: time delay int [4b]
na 258b: frame delay int [4b]
na 262b: flagi type [2b]
[2b przerwy]
na 266b: flagi hit type [2b]
[2b przerwy]
na 270b: x move res int [4b]
na 274b: y move res int [4b]

nastepnie stringi (not null terminated!!):
- nazwa obiektu
- logika
- image set
- animacja

[naglowek? 8x4int=32b]
0b: int ? - 32 [4b]
4b: ? [4b]
8b; ilosc atrybutow do klockow [4b]
12b: ? [4b]
16b: ? [4b]
20b: ? [4b]
24b: ? [4b]
28b: ? [4b]

dla kazdego klocka [20b lub 40b dla double]
0b: typ [4b] (? 0 single 1 double 2 mask 3)
4b: ? [4b]
8b: szer [4b]
12b: wys [4b]
16b: atrib [4b] (outside dla double)
(jesli double)
20b: atrib [4b] (inside)
24b: x1 [4b]
28b: y1 [4b]
32b: x2 [4b]
36b: y2 [4b]

atrib {
 clear - 0
 solid - 1
 ground - 2
 climb - 3
 death - 4

 ? - 54
}
**/
