#ifndef H_GLOBALS
#define H_GLOBALS

#include "../shared/cStateMgr.h"
#include "../shared/cConsole.h"
#include "../shared/cLang.h"
#include "../shared/cGUIparts.h"
#include "../shared/cInterface.h"
#include "../shared/cWWD.h"
#include "../shared/cPID.h"
#include "../shared/SimpleIni.h"

#include "hge.h"
#include "hgesprite.h"
#include "hgefont.h"

#include <csetjmp>

#include "guichan.hpp"
#include "guichan/hge.hpp"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <set>

#define MEMUSAGEUPDATEDELAY 3.5f

namespace State {
    class EditingWW;
}

struct cInterfaceSheet;

enum WindowsVer {
    OS_UNKNOWN = 0,
    OS_OLD,
    OS_2000,
    OS_XP,
    OS_SERVER2003,
    OS_VISTA,
    OS_7,
    OS_8,
    OS_10,
};

enum EnumGfxIcons {
    Icon_New = 0,
    Icon_Open,
    Icon_Save,
    Icon_SaveAs,
    Icon_Undo,
    Icon_Redo,
    Icon_GoUp,
    Icon_Home,
    Icon_Cut,
    Icon_Copy,
    Icon_Paste,
    Icon_Import,
    Icon_Export,
    Icon_NewFile,
    Icon_NewFolder,
    Icon_DeleteFile,
    Icon_Stats,
    Icon_Settings,
    Icon_Info,
    Icon_Help,
    Icon_Test,
    Icon_Mapshot,
    Icon_Pause,
    Icon_Stop,
    Icon_Play,
    Icon_World,
    Icon_Layers,
    Icon_Move,
    Icon_Pencil,
    Icon_Fill,
    Icon_Eye,
    Icon_Bricks,
    Icon_Erase,
    Icon_Brush,
    Icon_Row,
    Icon_Grid,
    Icon_Objects,
    Icon_Treasure,
    Icon_Health,
    Icon_Crate,
    Icon_Exclamation,
    Icon_ClawHead,
    Icon_Warp,
    Icon_Curse,
    Icon_Flag,
    Icon_GridLines,
    Icon_Boundary,
    Icon_Palette,
    Icon_Database,
    Icon_Sound,
    Icon_Lua,
    Icon_Camera,
    Icon_Star,
    Icon_X,
    Icon_Properties,
    Icon_Text,
    Icon_Reload,
    Icon_TileProperties,
    Icon_Apply,
    Icon_RSS,
    Icon_Pipette,
    Icon_Simulation,
    Icon_Select,
    Icon_Navigator,
    Icon_AddDown,
    Icon_AddUp,
    Icon_AddRight,
    Icon_AddLeft,
    Icon_AddUpRight,
    Icon_AddUpLeft,
    Icon_AddDownRight,
    Icon_AddDownLeft,
    Icon_UpRight,
    Icon_DownRight,
    Icon_DownLeft,
    Icon_Right,
    Icon_Left,
    Icon_Up,
    Icon_Down,
    Icon_CrumblinPeg,
    Icon_Elevator,
    Icon_Rope,
    Icon_TogglePeg,
    Icon_SlidingPeg,
    Icon_CannonWall,
    Icon_BossWarp,
    Icon_Statue,
    Icon_BreakPlank,
    Icon_SpringBoard,
    Icon_DoNothing,
    Icon_Animated,
    Icon_PowderKeg,
    Icon_ElevatorPath,
    Icon_Cannon,
    Icon_Projectile,
    Icon_Catnip,
    Icon_NPC_Rat,
    Icon_Measure,
    Icon_Zoom,
    Icon_Anchor,
    Icon_UpLeft,
    Icon_Map,
    Icon_Spikes,
    Icon_CrabNest,
    Icon_Shake,
    Icon_Stalactite,
    Icon_Laser,
    Icon_Tentacle,
    Icon_LaRauxe,
    Icon_Catherine,
    Icon_Wolvington,
    Icon_Gabriel,
    Icon_Marrow,
    Icon_Aquatis,
    Icon_RedTail,
    Icon_Omar,
    Icon_NPC_TownGuard1,
    Icon_NPC_TownGuard2,
    Icon_NPC_RedTailPirate,
    Icon_NPC_BearSailor,
    Icon_NPC_Soldier,
    Icon_NPC_Officer,
    Icon_NPC_CutThroat,
    Icon_NPC_RobberThief,
    Icon_NPC_Merkat,
    Icon_NPC_Siren,
    Icon_NPC_Fish,
    Icon_NPC_TigerGuard1,
    Icon_NPC_Crab,
    Icon_NPC_Seagull,
    Icon_NPC_Pegleg,
    Icon_NPC_CrazyHook,
    Icon_NPC_TigerGuard2,
    Icon_NPC_Chameleon,
    Icon_Music,
    Icon_XTransparent,
    Icon_Diamonds,
    Icon_TextAlign_Justify,
    Icon_TextAlign_Left,
    Icon_TextAlign_Right,
    Icon_Tool,
    Icon_Point,
    Icon_Line,
    Icon_Rect,
    Icon_Ellipse,
    Icon_Flags,
    Icon_Trash,
    Icon_Warning,
    Icon_WriteID,
    Icon_CrazyHook,
    Icon_WapMap,
    Icon_Animation,
    Icon_Code,
    Icon_HardDrive,
    Icon_Package,
    Icon_Spray,
    Icon_Lock,
    GfxIcons_Count
};

enum Gfx16Icons {
    Icon16_Move = 0,
    Icon16_Brush,
    Icon16_Export,
    Icon16_Cut,
    Icon16_Copy,
    Icon16_Paste,
    Icon16_Star,
    Icon16_X,
    Icon16_Properties,
    Icon16_Edit,
    Icon16_Treasure,
    Icon16_Health,
    Icon16_Crate,
    Icon16_Exclamation,
    Icon16_ClawHead,
    Icon16_Curse,
    Icon16_Flag,
    Icon16_CrumblingPeg,
    Icon16_Elevator,
    Icon16_Rope,
    Icon16_TogglePeg,
    Icon16_SlidingPeg,
    Icon16_CannonWall,
    Icon16_BossWarp,
    Icon16_Map,
    Icon16_Spikes,
    Icon16_NPC_Rat,
    Icon16_Flip,
    Icon16_FlipY,
    Icon16_FlipX,
    Icon16_ZCoord,
    Icon16_PropertyTree,
    Icon16_Warp,
    Icon16_Add,
    Icon16_AddDouble,
    Icon16_AddTriple,
    Icon16_Sub,
    Icon16_Statue,
    Icon16_BreakPlank,
    Icon16_SpringBoard,
    Icon16_DoNothing,
    Icon16_Animated,
    Icon16_PowderKeg,
    Icon16_ElevatorPath,
    Icon16_Cannon,
    Icon16_Projectile,
    Icon16_Catnip,
    Icon16_SubDouble,
    Icon16_SubTriple,
    Icon16_PlaneFront,
    Icon16_PlaneAction,
    Icon16_PlaneBack,
    Icon16_Align,
    Icon16_Applied,
    Icon16_Up,
    Icon16_Down,
    Icon16_Text,
    Icon16_CrabNest,
    Icon16_Shake,
    Icon16_Stalactite,
    Icon16_Laser,
    Icon16_Tentacle,
    Icon16_LaRauxe,
    Icon16_Catherine,
    Icon16_Wolvington,
    Icon16_Gabriel,
    Icon16_Marrow,
    Icon16_Aquatis,
    Icon16_RedTail,
    Icon16_Omar,
    Icon16_NPC_CutThroat,
    Icon16_NPC_RobberThief,
    Icon16_NPC_Merkat,
    Icon16_NPC_Siren,
    Icon16_NPC_Fish,
    Icon16_NPC_TigerGuard1,
    Icon16_NPC_Crab,
    Icon16_NPC_Seagull,
    Icon16_NPC_Pegleg,
    Icon16_NPC_CrazyHook,
    Icon16_NPC_TigerGuard2,
    Icon16_NPC_TownGuard1,
    Icon16_NPC_TownGuard2,
    Icon16_NPC_RedTailPirate,
    Icon16_NPC_BearSailor,
    Icon16_NPC_Soldier,
    Icon16_NPC_Officer,
    Icon16_NPC_Chameleon,
    Icon16_Music,
    Icon16_XTransparent,
    Icon16_Diamonds,
    Icon16_New,
    Icon16_Open,
    Icon16_Save,
    Icon16_SaveAs,
    Icon16_Settings,
    Icon16_Info,
    Icon16_AutoUpdate,
    Icon16_Readme,
    Icon16_ModeTile,
    Icon16_ModeObject,
    Icon16_ModeSim,
    Icon16_Pencil,
    Icon16_Fill,
    Icon16_Planes,
    Icon16_View,
    Icon16_Tools,
    Icon16_RSS,
    Icon16_Database,
    Icon16_Measure,
    Icon16_Grid,
    Icon16_Boundary,
    Icon16_ViewNo,
    Icon16_Camera,
    Icon16_Play,
    Icon16_TileProp,
    Icon16_Search,
    Icon16_Home,
    Icon16_Mapshot,
    Icon16_World,
    Icon16_Stats,
    Icon16_Flags,
    Icon16_i,
    Icon16_Trash,
    Icon16_Warning,
    Icon16_WriteID,
    Icon16_CrazyHook,
    Icon16_Sound,
    Icon16_Animation,
    Icon16_Code,
    Icon16_HardDrive,
    Icon16_Package,
    Icon16_AppliedPartially,
    Icon16_Lock,
    Gfx16Icons_Count
};

#define Gfx128IconsCnt 4
enum Gfx128Icons {
    Icon128_NewDocument,
    Icon128_OpenDocument,
    Icon128_RecentlyUsed,
    Icon128_WhatsNew
};

#define STR_IMPL_(x) #x      //stringify argument
#define STR(x) STR_IMPL_(x)  //indirection to expand argument macros
#define MIN_SUPPORTED_SCREEN_WIDTH 1024
#define MIN_SUPPORTED_SCREEN_HEIGHT 600
#define DEF_SCREEN_WIDTH 1024
#define DEF_SCREEN_HEIGHT 768
#define DEF_SCREEN_WIDTH_STR #DEF_SCREEN_WIDTH

int Console_Panic(lua_State *L);

class cGlobals {
public:
    cGlobals();

    ~cGlobals();

    void Init();
    void GetDesktopResolution(int& horizontal, int& vertical);

    HTEXTURE texMain;
    HTEXTURE texLevels;
    hgeSprite *sprLevels[18];
    hgeSprite *sprConsoleBG, *sprLogoBig;
    hgeSprite *sprShadeBar;
    hgeSprite *sprIconGoodSmall, *sprIconBadSmall, *sprIcons[GfxIcons_Count], *sprIcons16[Gfx16Icons_Count], *sprIcons128[Gfx128IconsCnt];
    hgeSprite *sprMicroVert, *sprMicroHor, *sprMicroZoom, *sprSmiley;
    hgeSprite *sprGamesBig[WWD::Games_Count + 1], *sprGamesSmall[WWD::Games_Count + 1], *sprBlank;
    hgeSprite *sprMicroIcons[2];

    hgeSprite *sprCaptionMinimalize[2], *sprCaptionClose[2];
    hgeSprite *sprLogoMini;
    hgeSprite *sprIconCaption;

    hgeSprite *sprDottedLineHorizontal, *sprDottedLineVertical;
    hgeSprite *sprArrowHorizontalL, *sprArrowHorizontalC, *sprArrowHorizontalR;
    hgeSprite *sprArrowVerticalU, *sprArrowVerticalM, *sprArrowVerticalD;

    hgeSprite *sprLevelsMicro16[14];

    SHR::cConsole *Console;
    SHR::Interface *IF;
    SHR::guiParts gcnParts;
    SHR::cStateMgr *StateMgr;
    SHR::cLanguage *Lang;

    hgeSprite *sprCloudTip[4];
    hgeSprite *sprContextCascadeArrow;

    hgeSprite *sprTile;
    hgeFont *fntMyriad10, *fntMyriad20, *fntSystem17, *fntMyriad13, *fntMyriad80, *fntMinimal;

    gcn::HGEInput *gcnInput;
    gcn::HGEGraphics *gcnGraphics;
    gcn::HGEImageLoader *gcnImageLoader;

    bool bKijan, bNapo;
    hgeSprite *sprKijan, *sprNapo;

    char *szLastOpenPath, *szLastSavePath;
    char *szUrlTCR;

    std::map<WWD::GAME, std::string> gamePaths;

    bool bFirstRun;

    bool bAutoUpdate, bAlphaHigherPlanes, bSmoothZoom;

    jmp_buf jmp_env;
    int jmp_val;

    lua_State *conL;

    char *conLuaBuf;
    std::string szCmdLine;

    CSimpleIni *ini;

    void SetLastOpenPath(const char *npath);

    void SetLastSavePath(const char *npath);

    void ExecuteLua(const char *script, bool popuponerror);

    void ReloadLua();

    void ResetWD();

    State::EditingWW *editState;
    int iScreenW, iScreenH;

    WindowsVer iOS;
    char szNameOS[256];

    bool bRealSim;

    char *szSerial;

    hgeSprite *sprCheckboard;

    DWORD dwProcID;

    hgeSprite *sprDropShadowCorner, *sprDropShadowLeft, *sprDropShadowUp, *sprDropShadowVert, *sprDropShadowHor;

    hgeSprite *sprUsedLibs;

    hgeSprite *sprThumbtack;

    hgeSprite *sprFeedIcon[2], *sprTabs[3], *sprFrameGrad, *sprFrame;

    cInterfaceSheet *hGfxInterface;

    std::vector<std::string> vstrClawLogics;
    std::set<std::string> vstrStandardImagesets;
    std::map<std::string, std::string> vstrStandardImagesetAnimations;
    std::set<std::string> vstrNANIImagesets;

    bool anyMapLoaded = false;

#ifdef SHOWMEMUSAGE
    char szMemUsage[64];
    CRITICAL_SECTION csMemUsage;
    float fMemUsageTimer;
    void UpdateMemUsage();
#endif

    DWORD colBase, colBaseDark, colLineDark, colLineBright, colOutline,
            colBaseGCN, colBaseDarkGCN, colLineDarkGCN, colLineBrightGCN, colOutlineGCN;

    void RenderDropShadow(int x, int y, int w, int h, unsigned char alpha = 255);

    void RenderFrame(int x, int y, int w, int h);
};

extern cGlobals *GV;

#endif
