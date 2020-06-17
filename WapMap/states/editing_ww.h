#ifndef H_STATE_EDIT_WW
#define H_STATE_EDIT_WW

#define DISTANCE(x1, y1, x2, y2) sqrt(pow((x2)-(x1), 2)+pow((y2)-(y1), 2))

#include "../../shared/cStateMgr.h"
#include "../../shared/cWWD.h"
#include "../../shared/gcnWidgets/wContainer.h"
#include "../../shared/gcnWidgets/wSlider.h"
#include "../../shared/gcnWidgets/wWin.h"
#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wScrollArea.h"
#include "../../shared/gcnWidgets/wCheckbox.h"
#include "../../shared/gcnWidgets/wDropDown.h"
#include "../../shared/gcnWidgets/wLink.h"
#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wTextBox.h"
#include "../../shared/gcnWidgets/wTextField.h"
#include "../../shared/gcnWidgets/wTabbedArea.h"
#include "../../shared/gcnWidgets/wRadioBut.h"
#include "../../shared/gcnWidgets/wContext.h"
#include "../cMDI.h"
#include "../cSoundPlayer.h"
#include "../cColorPicker.h"
#include "../wViewport.h"
#include "../cTileGhost.h"
#include "../vpFocusWidg.h"
#include "guichan.hpp"
#include "../cAutoUpdater.h"
#include "../objQuadTree.h"
#include "../globals.h"
#include "../../shared/cPieChart.h"
#include "objedit.h"
#include "../cMRUList.h"
#include "../cInventoryController.h"
#include "../cModeMenuBar.h"
#include "hgesprite.h"
#include "../cControllerIPC.h"
#include "../states/mapshot.h"
#include "../LogicInfo.h"
#include "../cRulers.h"
#include "../cAppMenu.h"

#ifndef byte
typedef unsigned char byte;
typedef unsigned long DWORD;
#endif

#define SIM_ROPE_DEFAULT_SPEED 3.000f //in seconds
#define SIM_TOGGLEPEG_DEFAULT_ON 1.500f //in secs
#define SIM_TOGGLEPEG_DEFAULT_OFF 1.500f //in secs
#define SIM_TOGGLEPEG_ADDITIONAL_DELAY 0.750f //in secs
#define SIM_EYECANDY_DELAY 0.100f //in seconds
#define DEFAULT_ELEVATOR_SPEED 125 //in pixels per second

#define COL_ORANGE_ARROW 0xFFffae00

#define TILE_PICK_COLOR         0x7700FF00
#define TILE_HIGHLIGHT_COLOR    0x77FFFF00

#define TILE_PICK_COLOR_SOLID       0xFF00FF00
#define TILE_HIGHLIGHT_COLOR_SOLID  0xFFFFFF00

#define ZCOORD_DIF 10000.0f

#define EWW_MODE_NONE       -1
#define EWW_MODE_TILE        0
#define EWW_MODE_OBJECT      1

#define EWW_TOOL_NONE            0  //no tool
#define EWW_TOOL_PENCIL          1  //tile mode pencil
#define EWW_TOOL_BRUSH           2  //tile mode lua brushes
#define EWW_TOOL_FILL            3  //tile mode flood feature
#define EWW_TOOL_DUPLICATE       4  //object duplication
#define EWW_TOOL_MOVEOBJECT      5  //moving objects, self-explaining
#define EWW_TOOL_BRUSHOBJECT     6  //object brush, self-explaining
#define EWW_TOOL_MEASURE         7  //measure tool, self-explaining
#define EWW_TOOL_OBJSELAREA      8  //Tool to select X and Y min and max object area. Simple hack to avoid switching states.
#define EWW_TOOL_EDITOBJ         9  //entry point for all object's easyedit applets
#define EWW_TOOL_ALIGNOBJ        10 //used to implement selection of which object user wants to align objects to
#define EWW_TOOL_SPACEOBJ        11 //object interval selection
#define EWW_TOOL_WRITEID         12 //tile mode write id

#define TOOL_OBJSA_NONE     0
#define TOOL_OBJSA_PICKMINX 1
#define TOOL_OBJSA_PICKMINY 2
#define TOOL_OBJSA_PICKMAXX 3
#define TOOL_OBJSA_PICKMAXY 4
#define TOOL_OBJSA_PICKALL  5

#define EWW_TILE_NONE       -1
#define EWW_TILE_FILL       -2
#define EWW_TILE_ERASE      -3
#define EWW_TILE_PIPETTE    -4

#define EWW_DRAW_POINT      0
#define EWW_DRAW_LINE       1
#define EWW_DRAW_RECT       2
#define EWW_DRAW_ELLIPSE    3
#define EWW_DRAW_SPRAY      4

#define VP_VIEWPORT         0
//unused                    1
//unused                    2
#define VP_WORLD            3
#define VP_LOAD             4
#define VP_DB               5
#define VP_WAR              6
#define VP_TILEPROP         7
#define VP_NAVI             8
#define VP_OBJSEARCH        11
#define VP_AUTOUPDATE       12
#define VP_FILLCOLOR        13
//unused                    14
#define VP_NEWMAP           15
#define VP_FIRSTRUN         16
#define VP_PLANEPROPERTIES  17
#define VP_WELCOMESCREEN    18
#define VP_CRASHRETRIEVE    19
#define VP_TILEPICKER       20
#define VP_LOGICBROWSER     21
#define VP_TILEBROWSER      22

#define LMODEL_PLANES           0
#define LMODEL_SCALE            1
#define LMODEL_ASSETS           2
#define LMODEL_ASSETFRAMES      3
#define LMODEL_IMAGESETS        4
#define LMODEL_TILETYPE         5
#define LMODEL_TILEATRIB        6
#define LMODEL_LANGUAGES        7
#define LMODEL_SOUNDS           8
#define LMODEL_ANIMATIONS       9
#define LMODEL_ANIMFRAMES       10
#define LMODEL_ANIMSPEED        11
#define LMODEL_SOUNDS_CLEAR     12
#define LMODEL_SEARCHTERM       13
#define LMODEL_PLANESPROP       14

enum OBJMENU {
	PROPERTIES = 0,
	DUPLIC,
	MOVE,
	COPYTOCB,
	DELETE,
	CUT,
	COPY,
	PASTE,
	USEASBRUSH,
	SETSPAWNP,
	NEWOBJ,
	EDIT,
	EDITLOGIC,

	ZCOORD,
	ZC_INC,
	ZC_INC2,
	ZC_INC3,
	ZC_DEC,
	ZC_DEC2,
	ZC_DEC3,
	ZC_BACK,
	ZC_ACTION,
	ZC_FRONT,
	ALIGN,
	ALIGN_HOR,
	ALIGN_VERT,
	SPECIFICPROP,
	SPACE,
	SPACE_HOR,
	SPACE_VERT,
	TESTFROMHERE,

	FLAGS,
	FLAGS_DRAW,
	FLAGS_DRAW_NODRAW,
	FLAGS_DRAW_FLIPX,
	FLAGS_DRAW_FLIPY,
	FLAGS_DRAW_FLASH,

	FLAGS_DYNAMIC,
	FLAGS_DYNAMIC_NOHIT,
	FLAGS_DYNAMIC_ALWAYSACTIVE,
	FLAGS_DYNAMIC_SAFE,
	FLAGS_DYNAMIC_AUTOHITDAMAGE,

	FLAGS_ADDITIONAL,
	FLAGS_ADDITIONAL_DIFFICULT,
	FLAGS_ADDITIONAL_EYECANDY,
	FLAGS_ADDITIONAL_HIGHDETAIL,
	FLAGS_ADDITIONAL_MULTIPLAYER,
	FLAGS_ADDITIONAL_EXTRAMEMORY,
	FLAGS_ADDITIONAL_FASTCPU,

	ADV_WARP_GOTO,
	ADV_CONTAINER_RAND,
};

#define OBJMENU_PROPERTIES   OBJMENU::PROPERTIES
#define OBJMENU_DUPLICATE    OBJMENU::DUPLIC
#define OBJMENU_MOVE         OBJMENU::MOVE
#define OBJMENU_COPYTOCB     OBJMENU::COPYTOCB
#define OBJMENU_DELETE       OBJMENU::DELETE
#define OBJMENU_CUT          OBJMENU::CUT
#define OBJMENU_COPY         OBJMENU::COPY
#define OBJMENU_PASTE        OBJMENU::PASTE
#define OBJMENU_USEASBRUSH   OBJMENU::USEASBRUSH
#define OBJMENU_SETSPAWNP    OBJMENU::SETSPAWNP
#define OBJMENU_NEWOBJ       OBJMENU::NEWOBJ
#define OBJMENU_EDIT         OBJMENU::EDIT
#define OBJMENU_EDITLOGIC    OBJMENU::EDITLOGIC

#define OBJMENU_ZCOORD       OBJMENU::ZCOORD
#define OBJMENU_ZC_INC       OBJMENU::ZC_INC
#define OBJMENU_ZC_INC2      OBJMENU::ZC_INC2
#define OBJMENU_ZC_INC3      OBJMENU::ZC_INC3
#define OBJMENU_ZC_DEC       OBJMENU::ZC_DEC
#define OBJMENU_ZC_DEC2      OBJMENU::ZC_DEC2
#define OBJMENU_ZC_DEC3      OBJMENU::ZC_DEC3
#define OBJMENU_ZC_BACK      OBJMENU::ZC_BACK
#define OBJMENU_ZC_ACTION    OBJMENU::ZC_ACTION
#define OBJMENU_ZC_FRONT     OBJMENU::ZC_FRONT
#define OBJMENU_ALIGN        OBJMENU::ALIGN
#define OBJMENU_ALIGN_HOR    OBJMENU::ALIGN_HOR
#define OBJMENU_ALIGN_VERT   OBJMENU::ALIGN_VERT
#define OBJMENU_SPECIFICPROP OBJMENU::SPECIFICPROP
#define OBJMENU_SPACE        OBJMENU::SPACE
#define OBJMENU_SPACE_HOR    OBJMENU::SPACE_HOR
#define OBJMENU_SPACE_VERT   OBJMENU::SPACE_VERT
#define OBJMENU_TESTFROMHERE OBJMENU::TESTFROMHERE

#define OBJMENU_FLAGS        OBJMENU::FLAGS

#define OBJMENU_FLAGS_DRAW                    OBJMENU::FLAGS_DRAW
#define OBJMENU_FLAGS_DRAW_NODRAW             OBJMENU::FLAGS_DRAW_NODRAW
#define OBJMENU_FLAGS_DRAW_FLIPX              OBJMENU::FLAGS_DRAW_FLIPX
#define OBJMENU_FLAGS_DRAW_FLIPY              OBJMENU::FLAGS_DRAW_FLIPY
#define OBJMENU_FLAGS_DRAW_FLASH              OBJMENU::FLAGS_DRAW_FLASH

#define OBJMENU_FLAGS_DYNAMIC                 OBJMENU::FLAGS_DYNAMIC
#define OBJMENU_FLAGS_DYNAMIC_NOHIT           OBJMENU::FLAGS_DYNAMIC_NOHIT
#define OBJMENU_FLAGS_DYNAMIC_ALWAYSACTIVE    OBJMENU::FLAGS_DYNAMIC_ALWAYSACTIVE
#define OBJMENU_FLAGS_DYNAMIC_SAFE            OBJMENU::FLAGS_DYNAMIC_SAFE
#define OBJMENU_FLAGS_DYNAMIC_AUTOHITDAMAGE   OBJMENU::FLAGS_DYNAMIC_AUTOHITDAMAGE

#define OBJMENU_FLAGS_ADDITIONAL              OBJMENU::FLAGS_ADDITIONAL
#define OBJMENU_FLAGS_ADDITIONAL_DIFFICULT    OBJMENU::FLAGS_ADDITIONAL_DIFFICULT
#define OBJMENU_FLAGS_ADDITIONAL_EYECANDY     OBJMENU::FLAGS_ADDITIONAL_EYECANDY
#define OBJMENU_FLAGS_ADDITIONAL_HIGHDETAIL   OBJMENU::FLAGS_ADDITIONAL_HIGHDETAIL
#define OBJMENU_FLAGS_ADDITIONAL_MULTIPLAYER  OBJMENU::FLAGS_ADDITIONAL_MULTIPLAYER
#define OBJMENU_FLAGS_ADDITIONAL_EXTRAMEMORY  OBJMENU::FLAGS_ADDITIONAL_EXTRAMEMORY
#define OBJMENU_FLAGS_ADDITIONAL_FASTCPU      OBJMENU::FLAGS_ADDITIONAL_FASTCPU

#define OBJMENU_ADV_WARP_GOTO      OBJMENU::ADV_WARP_GOTO
#define OBJMENU_ADV_CONTAINER_RAND OBJMENU::ADV_CONTAINER_RAND

#define TILMENU_CUT     0
#define TILMENU_COPY    1
#define TILMENU_PASTE   2
#define TILMENU_DELETE  3

#define COLOR_SOLID     0x770000FF
#define COLOR_GROUND    0x7700FF00
#define COLOR_CLIMB     0X77FFFF00
#define COLOR_DEATH     0x77FF0000
#define COLOR_UNKNOWN   0x77FFFFFF

#define HINT_TIME 5

//forward declarations
class cAppMenu;

class cNativeController;

class cWindow;

class winTileBrowser;

class winImageSetBrowser;

class winOptions;

class winAbout;

namespace SHR {
    class ComboBut;
};

namespace WIDG {
    class Viewport;
};

void EditingWW_ObjDeletionCB(WWD::Object *obj);

class cDynamicListModel : public gcn::ListModel {
private:
    std::vector<std::string> vOptions;
public:
    cDynamicListModel() {};

    ~cDynamicListModel() {};

    std::string getElementAt(int i) {
        if (i >= 0 && i < vOptions.size()) return vOptions[i];
        return "";
    };

    int getNumberOfElements() { return vOptions.size(); };

    void Add(std::string n) { vOptions.push_back(n); };
};

namespace State {

    bool ObjSortCoordX(WWD::Object *a, WWD::Object *b);

    bool ObjSortCoordY(WWD::Object *a, WWD::Object *b);

    bool ObjSortCoordZ(WWD::Object *a, WWD::Object *b);

    class Viewport;

    class cLayerRenderBuffer {
    private:
        HTARGET hTarget;
        hgeSprite *sprTarget;
        bool bRedraw;
        Viewport *vPort;
        WWD::Plane *hPlane;
        EditingWW *hOwner;
        int iEntityCnt;
    public:
        cLayerRenderBuffer(EditingWW *hown, Viewport *nvPort, WWD::Plane *npl);

        ~cLayerRenderBuffer();

        void Update();

        void Render(int x = 0, int y = 0, float fZoom = 1.0f);

        void Redraw() { bRedraw = 1; };

        void GfxLost();

        int GetEntityCount(int x1, int y1, int x2, int y2);
    };

    struct stObjectData {
        bool bEmpty;
        cObjectQuadTree *hQuadTree;
    };

    struct PlaneData {
    public:
        bool bDraw;
        bool bDrawGrid;
        bool bDrawBoundary;
        bool bDrawObjects;
        byte iAlpha;
        stObjectData ObjectData;
        bool bUpdateBuffer;
        cLayerRenderBuffer *hRB;
    };

    class EditingWW;

    class EditingWWActionListener;

    class EditingWWvpCallback : public WIDG::VpCallback {
    private:
        EditingWW *m_hOwn;
    public:
        virtual void Draw(int iCode);

        EditingWWvpCallback(EditingWW *owner);
    };

    class EditingWWlModel : public gcn::ListModel {
    private:
        EditingWW *m_hOwn;
        int m_iType;
    public:
        EditingWWlModel(EditingWW *owner, int type);

        std::string getElementAt(int i);

        int getNumberOfElements();
    };

    class EditingWWActionListener : public gcn::ActionListener {
    private:
        EditingWW *m_hOwn;
    public:
        EditingWWActionListener(EditingWW *owner);

        void action(const gcn::ActionEvent &actionEvent) override;
    };

    class EditingWWFocusListener : public gcn::FocusListener {
    private:
        EditingWW *m_hOwn;
    public:
        EditingWWFocusListener(EditingWW *owner);

        void focusLost(const FocusEvent &event) override;
    };

    class EditingWWKeyListener : public gcn::KeyListener {
    private:
        EditingWW *m_hOwn;
        bool lastPressedWasAlt;
    public:
        EditingWWKeyListener(EditingWW *owner);

        void stopAltMenu() { lastPressedWasAlt = false; }

        void keyPressed(KeyEvent &keyEvent) override;

        void keyReleased(KeyEvent &keyEvent) override;
    };

    class EditingWWMouseListener : public gcn::MouseListener {
        EditingWW *m_hOwn;

    public:
        EditingWWMouseListener(EditingWW *owner);

        void mousePressed(MouseEvent& mouseEvent) override;
    };

    class Viewport {
    private:
        int iX, iY, iW, iH;
        HTARGET target;
        hgeSprite *sprViewport;
        bool bRedrawViewport;
        EditingWW *hOwn;
        vpFoc *hFocuser;
    public:
        void MarkToRedraw(bool mark);

        bool IsMarkedToRedraw() { return bRedrawViewport; };

        void Update();

        void Render();

        Viewport(EditingWW *phOwn, int x, int y, int w, int h);

        ~Viewport();

        void Resize(int w, int h);

        void SetPos(int x, int y);

        int GetWidth() { return iW; };

        int GetHeight() { return iH; };

        int GetX() { return iX; };

        int GetY() { return iY; };

        vpFoc *GetWidget() { return hFocuser; };

        void GfxLost();

        void ClipScreen();

        friend class State::MapShot;
    };

    struct win_NewMap {
        SHR::Win *winNewMap;
        SHR::Lab *labSelectBaseLevel;
        WIDG::Viewport *vp;
        SHR::Slider *sliBaseLvls;
        SHR::CBox *cbAddBonusPlanes;
        SHR::TextField *tfAuthor;
        SHR::TextField *tfName;
        SHR::TextField *tfPlaneWidth, *tfPlaneHeight;
        SHR::Lab *labName, *labAuthor, *labPlaneSize, *labX;
        int iSelectedBase;
        SHR::But *butOK;
#ifdef WM_EXT_TILESETS
        float fBaseTimer[16];
#else
        float fBaseTimer[14];
#endif
        bool bKill;
    };

    struct win_FirstRun {
        SHR::Win *win;
        WIDG::Viewport *vp;
        SHR::But *setClawDir;
        SHR::But *website;
        bool bKill;
    };

    class EditingWW : public SHR::cState {
    public:
        SHR::Context *objContext, *tilContext, *objZCoordContext, *objmAlignContext, *objmSpaceContext; //basic contexts
        SHR::Context *objFlagContext, *objFlagDrawContext, *objFlagAddContext, *objFlagDynamicContext; //flags contexts
        SHR::Context *advcon_Warp, *advcon_Container; //advanced contexts (object specific)

        SHR::ContextModel *conmodObject, *conmodObjectMultiple, *conmodTilesSelected, *conmodTilesPaste, *conmodEditableObject;
        SHR::ContextModel *conmodPaste, *conmodSpawnPoint, *conmodAtEmpty;//, * conmodUseAsBrush;
        std::vector<TileGhost> vTileGhosting;

        SHR::Container *conResizeUp, *conResizeLeft, *conResizeRight, *conResizeDown;
        SHR::But *butExtLayerLeft, *butExtLayerRight, *butExtLayerUp, *butExtLayerDown, *butExtLayerUR, *butExtLayerUL, *butExtLayerDL,
                *butExtLayerDR;

        gcn::Gui *gui;
        SHR::Slider *sliVer, *sliHor;

        cmmbTile *hmbTile;
        cmmbObject *hmbObject;

        cModeMenuBar *hmbActive;

        SHR::Container *conWriteID;
        SHR::TextField *tfWriteID;

        cAutoUpdater *hAU;

#ifdef WM_ADD_LUA_EXECUTER
        SHR::But * butIconLua;
#endif

        SHR::TextField *tfwpName, *tfwpAuthor, *tfwpDate, *tfwpREZ, *tfwpTiles, *tfwpPalette, *tfwpExe;
        /*        *tfwpMapVersion, *tfwpMapBuild, *tfwpWapVersion;
        SHR::TextBox *tbwpMapDescription;
        SHR::ScrollArea *sawpMapDescription;
        SHR::Lab *labwpMapVersion, *labwpMapBuild, *labwpMapDescription, *labwpWapVersion;*/

        SHR::But *butwpSave, *butwpCancel;

        cNativeController *hNativeController;

        SHR::DropDown *ddmsPlane;
        SHR::Slider *slimsScale, *sliTilePicker, *slimsCompression;
        SHR::Lab *labmsSaveAs, *labmsScale, *labmsPlane, *labmsDimensions;
        SHR::TextField *tfmsSaveAs, *winobjseaName;
        SHR::But *butmsSaveAs, *butmsSave;
        SHR::Win *winWorld, *winMapShot, *winDB, *winSearchObj;
        SHR::CBox *cbmsOpenFileDirectory;
        SHR::CBox *cbwpCompress, *cbwpZCoord, *cbdbFlip, *cbdbAniAnimate, *cbdbBorderAni, *cbdbAniLoop;
        SHR::ScrollArea *sadbAssetsImg, *sadbFramesImg, *sadbAssetsAni, *sadbFramesAni, *sadbAssetsSnd;
        SHR::Lab *labwpName, *labwpAuthor, *labwpDate, *labwpREZ, *labwpTiles, *labwpPalette, *labwpExe, *labmsCompression,
                *labmsCompDisc;
        SHR::ListBox *dbAssetsImg, *dbFramesImg, *dbAssetsAni, *dbFramesAni, *dbAssetsSnd;

        SHR::Win *winSoundBrowser;
        SHR::ListBox *lbdbsSounds;
        SHR::Lab *labdbsDisplay;
        SHR::CBox *cbdbsOriginal, *cbdbsCustom;
        WIDG::Viewport *vpdbsDraw;
        SHR::But *butdbsAddSounds, *butdbsReload, *butdbsDelete;

        cSoundPlayer *spdbSounds;

        SHR::Win *winLogicBrowser;
        bool bLogicBrowserExpanded = false;

        SHR::ScrollArea *sabrlLogicList;
        SHR::ListBox *lbbrlLogicList;
        SHR::But *butbrlNew, *butbrlEdit, *butbrlEditExternal, *butbrlDelete, *butbrlBrowseDir, *butbrlRenameOK, *butbrlRename;
        SHR::Lab *labbrlLogicName, *labbrlFilePath, *labbrlFileSize, *labbrlFileChecksum, *labbrlFileModDate,
                *labbrlLogicNameV, *labbrlFilePathV, *labbrlFileSizeV, *labbrlFileChecksumV, *labbrlFileModDateV;
        SHR::TextField *tfbrlRename;
        WIDG::Viewport *vpLogicBrowser;

        void DrawLogicBrowser();

        void SyncLogicBrowser();

        void ExpandLogicBrowser();

        LogicInfo GetLogicInfo(const char* logic) { return {logic}; }

        float fdbAniTimer;

        SHR::TabbedArea *tadbTabs;
        SHR::Container *condbTiles, *condbImages, *condbAnims, *condbSounds;
        SHR::Lab *labdbAnimSpeed, *labdbAniImageSet;
        SHR::DropDown *dddbAnimSpeed;
        SHR::TextField *tfdbAniImageSet;

        SHR::RadBut *rbdbisShow[3];
        SHR::Lab *labdbisSetChecksum, *labdbisSetFileSize, *labdbisSetFileCount,
                *labdbisFileChecksum, *labdbisFileSize, *labdbisFileDim, *labdbisFileOffset, *labdbisFileUserData,
                *labdbisFileID, *labdbisFileIndex, *labdbisFrameID;
        SHR::CBox *cbdbisFlipX, *cbdbisFlipY, *cbdbisOffsetBorder, *cbdbisBorder;
        int idbisMoveX, idbisMoveY;
        float fdbisZoom;

        SHR::But *butObjSearchSelect, *butObjSearchSelectAll;
        SHR::DropDown *ddObjSearchTerm;
        SHR::CBox *cbObjSearchCaseSensitive;
        SHR::Slider *sliSearchObj;

        SHR::Win *winCamera;
        SHR::Lab *labcamCoordX, *labcamCoordY, *labObjSearchResults;
        SHR::TextField *tfcamSetToX, *tfcamSetToY;
        SHR::But *butcamSetTo, *butcamSetToSpawn;

        SHR::Win *winObjectBrush;
        SHR::Lab *labobrDistance, *labobrDispX, *labobrDispY;
        SHR::TextField *tfobrDispX, *tfobrDispY;
        SHR::Slider *sliobrDistance;
        SHR::CBox *cbobrApplyScatterSeparately, *cbobrUpdateRects;

        SHR::CBox *cbmsDrawObjects;
        SHR::Lab *labLoadLastOpened;
        SHR::Link *lnkLastOpened[10];

        SHR::Win *winDuplicate;
        SHR::Lab *labdTimes, *labdOffsetX, *labdOffsetY, *labobjseaInfo, *labdChangeZ;
        SHR::TextField *tfdTimes, *tfdOffsetX, *tfdOffsetY, *tfdChangeZ;
        SHR::But *butdOK;

        int iobjbrLastDrawnX, iobjbrLastDrawnY;

        SHR::Win *winTileProp;
        SHR::But *buttpPrev, *buttpNext, *buttpZoom, *buttpApply, *buttpShow;//, *buttpPipette;
        SHR::TextField *tftpTileID, *tftpW, *tftpH, *tftpX1, *tftpX2, *tftpY1, *tftpY2;
        SHR::RadBut *rbtpSingle, *rbtpDouble, *rbtpMask, *rbtpIn[5], *rbtpOut[5];
        WWD::TileAttrib *hTempAttrib = nullptr;
        bool btpDragDropMask;
        int itpDDx1, itpDDy1, itpDDx2, itpDDy2;
        int itpSelectedTile;
        bool btpZoomTile;

        WIDG::Viewport *vpobjseaRender, *vptpfcPalette;

        SHR::Win *winMeasureOpt;
        SHR::But *butmeasClear;
        SHR::CBox *cbmeasAbsoluteDistance;

        SHR::But *butMicroTileCB, *butMicroObjectCB;

        bool bObjBrushDrawing;

        //std::vector<cObjectProp*> vObjectWins;
        WIDG::Viewport *vpMain, *vpAbout, *vpWorld, *vpLoad, *vpDB, *vpTileProp;
        char cScrollOrientation;
        bool bDragDropScroll;
        float fDragLastMx, fDragLastMy;

        float fObjPickLastMx, fObjPickLastMy;

        int iMode;
        int iActiveTool;

        int iTilePicked;
        bool lockDrawing;

        bool bOpenObjContext;

        bool bExit;

        float fade_fAlpha;
        float fade_fTimer;
        byte fade_iAction;

        int lastbrushx, lastbrushy, iLastBrushPlacedX, iLastBrushPlacedY;

        friend class EditingWWActionListener;

        friend class EditingWWvpCallback;

        friend class EditingWWlModel;

        friend class Viewport;

    public:

        EditingWW();

        EditingWW(WWD::Parser *phParser);

        virtual bool Opaque();

        virtual void Init();

        virtual void Destroy();

        virtual bool Think();

        virtual void PreRender();

        virtual bool Render();

        virtual void GainFocus(int iReturnCode, bool bFlipped);

        void ToggleFullscreen();

		void FixInterfacePositions();

        virtual bool PromptExit();

        virtual void GfxRestore();

        virtual void FileDraggedIn();

        virtual void FileDraggedOut();

        virtual void FileDropped();

        virtual void AppFocus(bool bGain);

        bool bWindowFocused;

        void DrawViewport();

        void DrawAbout();

        void DrawWorldP();

        void DrawLoad();

        void DrawDB();

        void DrawTileProperties();

        int GetActivePlaneID();

        WWD::Plane *GetActivePlane();

        void SetTool(int iNewTool);

        void OpenTool(int iNewTool);

        void CloseTool(int iNewTool);

        void FreeResources();

        void MarkUnsaved();

        void SwitchPlane();

        void SetMode(int piMode);

        void EnterMode(int piMode);

        void ExitMode(int piMode);

        bool TileThink(bool pbConsumed);

        bool ObjectThink(bool pbConsumed);

        void FloodFill(int x, int y, int tile);

        void DrawTileAttributes(int tileId, float posX, float posY, float widthMod, float heightMod);

        char *ExportObject(WWD::Object *obj);

        void OpenObjectWindow(WWD::Object *obj, bool bMove = 0);

        void SynchronizeWithParser();

        void InitEmpty();

        WWD::Object *GetObjUnderWorldPos(int mx, int my);

        SHR::But *MakeButton(int x, int y, EnumGfxIcons icon, SHR::Container *dest, bool enable = 1, bool visible = 1,
                             gcn::ActionListener *actionListener = NULL);

        void RenderToViewportBuffer();

        bool bConstRedraw;

        float fCamLastX, fCamLastY, fLastZoom;

        int iTilePreviewX, iTilePreviewY;

        cBankTile *hTileset;
        cBankImageSet *SprBank;
        cDataController *hDataCtrl;

        void ShowAndUpdateDuplicateMenu();

        float fObjContextX, fObjContextY;
        bool bDrawTileProperties;
        bool bLockScroll;

        //camera coords calculation
        int Scr2WrdX(WWD::Plane *pl, int px); //screen to world X
        int Scr2WrdY(WWD::Plane *pl, int py); //screen to world Y
        int Wrd2ScrX(WWD::Plane *pl, int px); //world to screen X
        int Wrd2ScrY(WWD::Plane *pl, int py); //world to screen Y
        int Wrd2ScrXrb(WWD::Plane *pl, int px); //world to screen X within renderbuffer
        int Wrd2ScrYrb(WWD::Plane *pl, int py); //world to screen Y within renderbuffer

        void UpdateScrollBars();

        WWD::Parser *hParser;

        void SyncAttribMenuWithTile();

        std::vector<std::pair<int, int> > m_vMeasurePoints;

        void DrawTileAttributes(WWD::TileAttrib *attrib, float posX, float posY, float widthMod, float heightMod);

        std::vector<WWD::Object *> GetObjectsInArea(WWD::Plane *plane, int x, int y, int w, int h);

        void DrawObjSearch();

        char *szObjSearchBuffer;

        std::vector<std::pair<int, int> > vObjSearchResults;

        int RenderLayer(WWD::Plane *hPl, bool bDefaultZoom);

        bool bObjDragSelection;
        int iObjDragOrigX, iObjDragOrigY;

        void ObjectOverlay();

        int iMoveRelX, iMoveRelY;

        void NotifyObjectDeletion(WWD::Object *obj);

        Viewport *vPort;
        cBankSound *hSndBank;
        cBankAni *hAniBank;
        cBankLogic *hCustomLogics;
        WWD::Object *hStartingPosObj;
        SHR::Container *conMain;

        float fCamX, fCamY;
        float fZoom;
        float fDestZoom;

        std::vector<PlaneData *> hPlaneData;
        DWORD dwBG;
        cColorPicker *cpmsPicker;

        float fDoubleClickTimer;
        int iDoubleClickX, iDoubleClickY;
        SHR::Win *wintpFillColor;
        SHR::But *buttpfcShow;

        void DrawSelectFillColor();

        int iTileSelectX1, iTileSelectY1, iTileSelectX2, iTileSelectY2;
        int iTileWriteIDx, iTileWriteIDy;

        void DrawPlaneOverlay(WWD::Plane *hPl);

        void ViewportOverlay();

        bool bSimFold;
        float fSimFoldY;

        float fCameraMoveTimer;

        bool draggedFilesIn = false;

        cMDI *MDI;

        bool bMaximized, bDragWindow;
        int iWindowDragX, iWindowDragY;
        RECT windowDragStartRect;

        EditingWWvpCallback *vp;

        void SetZoom(float fZ);

        void RenderCloudTip(int x, int y, int w, int h, int ax, int ay);

        bool bForceTileClipbPreview, bForceObjectClipbPreview;

        void RenderTileClipboardPreview();

        void RenderObjectClipboardPreview();

        void UpdateSearchResults();

        //EWW_TOOL_OBJSELAREA tool vars
        SHR::Win *wintoolSelArea; //tool window
        SHR::But *buttoolSelAreaOK, *buttoolSelAreaPickMinX, *buttoolSelAreaPickMinY,//accept, select min x, select min y buttons
                *buttoolSelAreaPickMaxX, *buttoolSelAreaPickMaxY, *buttoolSelAreaAll; //select max x, select max y, select by area buttons
        SHR::Lab *labtoolSelAreaValues; //label with values
        int toolsaMinX, toolsaMinY, toolsaMaxX, toolsaMaxY; //cached values
        int toolsaAction; //selected action

        //PLANE MANAGER vars
        SHR::Win *winpmMain;
        SHR::ListBox *lbpmPlanes;
        SHR::ScrollArea *sapmPlanes;
        SHR::Lab *labpmPlanes, *labpmAnchor, *labpmWidth, *labpmHeight;
        WIDG::Viewport *vppm;
        SHR::Lab *labpmName, *labpmTileSize, *labpmTileSize_x, *labpmMovement, *labpmMovX, *labpmMovY,
                 *labpmOffX, *labpmOffY, *labpmOffset, *labpmPlaneSize, *labpmZCoord, *labpmFlags, *labpmTileSet;
        SHR::TextField *tfpmName, *tfpmTileSizeX, *tfpmTileSizeY, *tfpmPlaneSizeX, *tfpmPlaneSizeY, *tfpmMovX, *tfpmMovY, *tfpmOffX, *tfpmOffY, *tfpmZCoord;
        SHR::DropDown *ddpmTileSet;
        SHR::TextBox *tbpmImagesets;
        SHR::But *butpmResUL, *butpmResU, *butpmResUR, *butpmResL, *butpmResC, *butpmResR, *butpmResDL, *butpmResD, *butpmResDR;
        SHR::CBox *cbpmFlagMainPlane, *cbpmFlagNoDraw, *cbpmFlagWrapX, *cbpmFlagWrapY, *cbpmAutoTileSize;
        SHR::But *butpmSave, *butpmDelete;
        int ipmAnchor, ipmSizeX, ipmSizeY;

        void SyncPlaneProperties();

        void DrawPlaneProperties();

        void SavePlaneProperties();

        void DeletePlaneProperties();

        void SetAnchorPlaneProperties(int anchor);

        void HandleMirrorAndInvertHotkeys();
        void HandleHotkeys();

        //events
        void ApplicationStartup(); //executed when intro is done
        void PrepareForDocumentSwitch();

        void DocumentSwitched();

        //new map
        win_NewMap *NewMap_data;

        void NewMap_Open();

        void NewMap_Close();

        void NewMap_Think();

        void NewMap_OK(); //called when OK button is pressed
        void NewMap_Validate();

        //first run
        win_FirstRun *FirstRun_data;

        void FirstRun_Open();

        void FirstRun_Close();

        void FirstRun_Think();

        void FirstRun_Action(bool but);

        //world options
        void SyncWorldOptionsWithParser();

        void SaveWorldOptions();

        void LockToolSpecificFunctions(bool bLock); //to specific tools
        void SetIconBarVisible(bool b);

        float fObjPropMouseX, fObjPropMouseY; //to lock ppm object menu

        //object edit
        bool bEditObjDelete;
        ObjEdit::cObjEdit *hEditObj;

        bool IsEditableObject(WWD::Object *obj, ObjEdit::cObjEdit **hEdit = 0); //whether 'easy edit' is available
        void OpenObjectEdit(WWD::Object *obj);

        std::vector<WWD::Object *> vObjectsPicked, vObjectsHL, vObjectsForbidHL, vObjectClipboard, vObjectsBrushCB;

        void SwitchActiveModeMenuBar(cModeMenuBar *n);

        void CreateObjectWithEasyEdit(gcn::Widget *widg);

        const char* GetDefaultElevatorImageSet();

        cInventoryController *hInvCtrl;

        std::vector<gcn::Widget *> vWidgetsToMinimalise;
        std::vector<gcn::Widget *> vMinimalisedWidgets;

        void MinimizeWindows();

        void MaximizeWindows();

        bool bObjectAlignAxis;

        bool AreObjectSpecificOptionsAvailable(WWD::Object *obj, SHR::Context **conMod = 0);

        int GetActiveMode() { return iMode; };

        //app bar
        float fAppBarTimers[LAY_APP_BUTTONS_COUNT];

        //window spacing
        SHR::TextField *tfspacingDistance;
        SHR::Win *winSpacing;
        SHR::Lab *labspacingDistance;
        SHR::But *butspacingOK;

        //welcome screen
        SHR::Container *winWelcome;
        SHR::Link* welcomeScreenOptions[WelcomeScreenOptions_Count];
        WIDG::Viewport *vpws;

        void DrawWelcomeScreen();

        void UpdateSettings();

        //wincrashretrieve
        SHR::Container *conCrashRetrieve, *conRecentFiles;
        SHR::But *butCrashRetrieve;
        WIDG::Viewport *vpCrashRetrieve;
        char *szCrashRetrieve[10];
        byte iCrashRetrieveIcon[10];
        char *szCrashRetrieveMore;

        void DrawCrashRetrieve();

        std::vector<std::string> vstrMapsToLoad;
		byte iDraggedFileIcon[10];

        //MRU
        cMruList *hMruList;

        void MruListUpdated(); //called from cMruList when list has been updates

        void RenderAreaRect(WWD::Rect r, WWD::Rect dr, bool bClip, DWORD hwCol, bool bFill,
                            DWORD hwFillCol); //renders area in viewport, given absolute screen-coords, if any coord = 0 then no draw
        void RenderArrow(int x, int y, int x2, int y2, bool finished, bool setColors = true, bool twoSided = false);

        bool ValidateLevelName(const char *name, bool bAllowNoNum);

        char *FixLevelName(int iBaseLvl, const char *name);

        std::vector<cInventoryItem> GetContainerItems(WWD::Object *obj);

        cServerIPC *hServerIPC;

        EditingWWActionListener *al;
        EditingWWFocusListener *fl;
        EditingWWKeyListener *kl;
        EditingWWMouseListener *ml;

        WWD::Plane *plMain;

        cAppMenu *hAppMenu;
        byte gamesLastOpened[10];

		cRulers *hRulers;

        bool OpenDocuments();
        void SaveAs();
        void Export();

        int iTileDrawStartX, iTileDrawStartY;

        SHR::Slider *sliZoom;

        //tilepicker
        SHR::Win *winTilePicker;
        SHR::But *buttpiModePencil, *buttpiModeBrush, *buttpiSpray, *buttpiReloadBrush,
                *buttpiPoint, *buttpiLine, *buttpiRect, *buttpiEllipse, *buttpiFlood;
        SHR::Lab *labtpiReloadBrush, *labtpiModePencil, *labtpiModeBrush, *labtpiToolOptions, *labtpiPointSize, *labtpiSpraySize,
                *labtpiSprayDensity, *labtpiLineThickness;
        SHR::CBox *cbtpiRectFilled, *cbtpiEllipseFilled;
        SHR::Slider *slitpiPointSize, *slitpiSpraySize, *slitpiSprayDensity, *slitpiLineThickness;
        SHR::CBox *cbtpiShowTileID, *cbtpiShowProperties;
        SHR::Slider *slitpiPicker;
        int iTilePickerOffUp, iTilePickerOffDown;
        int iTileDrawMode;
        int iPipetteTileHL;
        bool btpiFixedPos;
        WIDG::Viewport *vptpi;

        //float fHomeBackButTimer;

        void RebuildTilePicker(bool forceSliderRefresh = false);

        void RefreshTilePickerSlider(bool forceSliderRefresh = false);

        void DrawTilePicker();

        void HandleBrushSwitch(int itOld, int itNew);

        void SyncDB_ImageSets();

        int iManipulatedGuide;
        bool bShowGuideLines;

        int RenderPlane(WWD::Plane *plane, int pl);

        int RenderObject(WWD::Object *hObj, int x, int y, DWORD col);

        int iTilesOnScreen, iObjectsOnScreen;

        SHR::ComboBut *cbutActiveMode;

        std::vector<cWindow *> hWindows;
        winTileBrowser *hwinTileBrowser;
        winImageSetBrowser *hwinImageSetBrowser;
        winOptions *hwinOptions;
        winAbout *hwinAbout;

        void TextEditMoveToNextTile(bool saving = false);

        void UpdateMovedObjectWithRects(std::vector<WWD::Object *>& vector, bool prompt = true);

        void ObjectBrush(int x, int y);

        void OnResize();
    };
};

#endif
