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

enum Gfx32Icons {
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
    Icon_InfoCloud,
    Icon_Info,
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
    Icon_Apply,
    Icon_RSS,
    Icon_Pipette,
    Icon_Simulation,
    Icon_Select,
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
    Icon_Rope,
    Icon_TogglePeg,
    Icon_Elevator,
    Icon_ElevatorPath,
    Icon_CannonWall,
    Icon_BossWarp,
    Icon_Statue,
    Icon_BreakPlank,
    Icon_SpringBoard,
    Icon_Decoration,
    Icon_PowderKeg,
    Icon_Projectile,
    Icon_PowerUp,
    Icon_Enemy,
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
    Icon_Music,
    Icon_XTransparent,
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
    Icon_Animation,
    Icon_Code,
    Icon_Lock,
    Icon_WapMap,
    Icon_HardDrive,
    Icon_Package,
    Icon_Spray,
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
    Icon16_Rope,
    Icon16_TogglePeg,
    Icon16_Elevator,
    Icon16_ElevatorPath,
    Icon16_CannonWall,
    Icon16_BossWarp,
    Icon16_Map,
    Icon16_Spikes,
    Icon16_Enemy,
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
    Icon16_Decoration,
    Icon16_PowderKeg,
    Icon16_Projectile,
    Icon16_PowerUp,
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
    Icon16_Music,
    Icon16_XTransparent,
    Icon16_New,
    Icon16_Open,
    Icon16_Save,
    Icon16_SaveAs,
    Icon16_Settings,
    Icon16_InfoCloud,
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
    Icon16_Search,
    Icon16_Mapshot,
    Icon16_World,
    Icon16_Stats,
    Icon16_Flags,
    Icon16_Info,
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
    Icon16_UpRight,
    Icon16_DownRight,
    Icon16_DownLeft,
    Icon16_Right,
    Icon16_Left,
    Icon16_UpLeft,
    Icon16_Anchor,
    Icon16_Mirror,
    Icon16_Invert,
    Gfx16Icons_Count
};

enum APP_CURSOR {
    DEFAULT = 0,
    TEXT,
    DRAG,
    HAND,
    GRAB,
    ZOOM_IN,
    ZOOM_OUT,

    CURSORS_COUNT
};

enum WelcomeScreenOptions {
    NewDocument,
    OpenExisting,
    WhatsNew,
    WelcomeScreenOptions_Count
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

    void RenderLogoWithVersion(int x, int y, int alpha = 255);

    void SliderDrawBar(int dx, int dy, bool orient, int size, int type, DWORD col);

    HTEXTURE texMain;
    HTEXTURE texLevels;
    hgeSprite *sprLevels[18];
    hgeSprite *sprLogoBig, *sprLogoCaption;
    hgeSprite *sprIcons[GfxIcons_Count], *sprIcons16[Gfx16Icons_Count];
    hgeSprite *sprSmiley;
    hgeSprite *sprGamesBig[WWD::Games_Count + 1], *sprGamesSmall[WWD::Games_Count + 1], *sprBlank, *sprConsoleBG;

    hgeSprite *sprCaptionMinimze[2], *sprCaptionClose[2];

    hgeSprite *sprDottedLineCorner, *sprDottedLineHorizontal, *sprDottedLineVertical;
    hgeSprite *sprArrowVerticalU, *sprArrowVerticalM, *sprArrowVerticalD;

    hgeSprite *sprLevelsMicro16[14];

    SHR::cConsole *Console;
    SHR::Interface *IF;
    SHR::guiParts gcnParts;
    SHR::cStateMgr *StateMgr;
    SHR::cLanguage *Lang;

    hgeSprite *sprContextCascadeArrow;
    hgeSprite *sprTabCloseButton, *sprTabCloseButtonFocused;
    hgeSprite *sprTabAddButton, *sprTabAddButtonFocused;
    hgeSprite *sprHomepageBackButton;
    hgeSprite *sprSnowflake;

    hgeSprite *sprTile;
    hgeFont *fntMyriad10, *fntMyriad20, *fntSystem17, *fntMyriad16, *fntMyriad80, *fntMinimal;

    gcn::HGEInput *gcnInput;
    gcn::HGEGraphics *gcnGraphics;
    gcn::HGEImageLoader *gcnImageLoader;

    bool bKijan, bZax, bWinter;
    hgeSprite *sprKijan, *sprZax;

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

    HCURSOR cursors[CURSORS_COUNT];

    void SetCursor(APP_CURSOR cursor);

    WindowsVer iOS;
    char szNameOS[256];

    bool bRealSim;

    char *szSerial;

    hgeSprite *sprCheckboard;

    DWORD dwProcID;

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

    DWORD colBase, colBaseDark, colLineDark, colLineBright, colOutline, colFontWhite, colActive;
};

extern cGlobals *GV;
extern hgeQuad q;

#endif
