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
//#include "objprop.h"
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

#include "hgeSprite.h"

#include "../cControllerIPC.h"

#ifndef byte
typedef unsigned char byte;
typedef unsigned long DWORD;
#endif

#define SIM_ROPE_DEFAULT_SPEED 3.000f //in seconds
#define SIM_TOGGLEPEG_DEFAULT_ON 1.500f //in secs
#define SIM_TOGGLEPEG_DEFAULT_OFF 1.500f //in secs
#define SIM_TOGGLEPEG_ADDITIONAL_DELAY 0.750f //in secs
#define SIM_EYECANDY_DELAY 0.100f //in seconds
#define SIM_ELEVATOR_DEFAULT_SPEED 125 //in pixels per second

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
#define EWW_TOOL_NEWOBJECT       4
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
#define EWW_DRAW_FLOOD      4
#define EWW_DRAW_SPRAY      5

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

#define OBJMENU_PROPERTIES   0
#define OBJMENU_DUPLICATE    1
#define OBJMENU_MOVE         2
#define OBJMENU_COPYTOCB     3
#define OBJMENU_DELETE       4
#define OBJMENU_CUT          5
#define OBJMENU_COPY         6
#define OBJMENU_PASTE        7
#define OBJMENU_USEASBRUSH   8
#define OBJMENU_SETSPAWNP    9
#define OBJMENU_NEWOBJ       10
#define OBJMENU_EDIT         11
#define OBJMENU_EDITLOGIC    12

#define OBJMENU_ZCOORD       12
#define OBJMENU_ZC_INC       13
#define OBJMENU_ZC_INC2      14
#define OBJMENU_ZC_INC3      15
#define OBJMENU_ZC_DEC       16
#define OBJMENU_ZC_DEC2      17
#define OBJMENU_ZC_DEC3      18
#define OBJMENU_ZC_BACK      19
#define OBJMENU_ZC_ACTION    20
#define OBJMENU_ZC_FRONT     21
#define OBJMENU_ALIGN        22
#define OBJMENU_ALIGN_HOR    23
#define OBJMENU_ALIGN_VERT   24
#define OBJMENU_SPECIFICPROP 25
#define OBJMENU_SPACE        26
#define OBJMENU_SPACE_HOR    27
#define OBJMENU_SPACE_VERT   28
#define OBJMENU_TESTFROMHERE 29

#define OBJMENU_FLAGS        30

 #define OBJMENU_FLAGS_DRAW                    31
  #define OBJMENU_FLAGS_DRAW_NODRAW             32
  #define OBJMENU_FLAGS_DRAW_FLIPX              33
  #define OBJMENU_FLAGS_DRAW_FLIPY              34
  #define OBJMENU_FLAGS_DRAW_FLASH              35

 #define OBJMENU_FLAGS_DYNAMIC                 36
  #define OBJMENU_FLAGS_DYNAMIC_NOHIT           37
  #define OBJMENU_FLAGS_DYNAMIC_ALWAYSACTIVE    38
  #define OBJMENU_FLAGS_DYNAMIC_SAFE            39
  #define OBJMENU_FLAGS_DYNAMIC_AUTOHITDAMAGE   40

 #define OBJMENU_FLAGS_ADDITIONAL              41
  #define OBJMENU_FLAGS_ADDITIONAL_DIFFICULT    42
  #define OBJMENU_FLAGS_ADDITIONAL_EYECANDY     43
  #define OBJMENU_FLAGS_ADDITIONAL_HIGHDETAIL   44
  #define OBJMENU_FLAGS_ADDITIONAL_MULTIPLAYER  45
  #define OBJMENU_FLAGS_ADDITIONAL_EXTRAMEMORY  46
  #define OBJMENU_FLAGS_ADDITIONAL_FASTCPU      47

#define OBJMENU_ADV_WARP_GOTO      100
#define OBJMENU_ADV_CONTAINER_RAND 101

#define TILMENU_CUT     0
#define TILMENU_COPY    1
#define TILMENU_PASTE   2
#define TILMENU_DELETE  3

#define SPECOBJ_START -2

#define OBJ_UNKNOWN     0
#define OBJ_ROPE        1
#define OBJ_CURSE       2
#define OBJ_TREASURE    3
#define OBJ_SOUND       4
#define OBJ_ENEMY       5

#define COLOR_SOLID     0x770000FF
#define COLOR_GROUND    0x7700FF00
#define COLOR_CLIMB     0X77FFFF00
#define COLOR_DEATH     0x77FF0000

#define HINT_TIME 5

//#define SCR2WRD_X(x) int( m_hOwn->fCamX*(m_hOwn->GetActivePlane()->GetMoveModX()/100.0f)*+mx/m_hOwn->fZoom )

//forward declarations
class cAppMenu;
class cNativeController;
class cWindow;
class winTileBrowser;
class winImageSetBrowser;
class winOptions;
class winAbout;

namespace SHR
{
 class ComboBut;
};

namespace WIDG {
 class Viewport;
};

void EditingWW_ObjDeletionCB(WWD::Object * obj);

 class cDynamicListModel: public gcn::ListModel {
 private:
  std::vector<std::string> vOptions;
 public:
  cDynamicListModel(){};
  ~cDynamicListModel(){};
  std::string getElementAt(int i){ if( i >= 0 && i < vOptions.size() ) return vOptions[i]; return ""; };
  int getNumberOfElements(){ return vOptions.size(); };
  void Add(std::string n){ vOptions.push_back(n); };
};

namespace State {

 bool ObjSortCoordX(WWD::Object * a, WWD::Object * b);
 bool ObjSortCoordY(WWD::Object * a, WWD::Object * b);
 bool ObjSortCoordZ(WWD::Object * a, WWD::Object * b);

 class Viewport;

 class cLayerRenderBuffer {
  private:
   HTARGET hTarget;
   hgeSprite * sprTarget;
   bool bRedraw;
   Viewport * vPort;
   WWD::Plane * hPlane;
   EditingWW * hOwner;
   int iEntityCnt;
  public:
   cLayerRenderBuffer(EditingWW * hown, Viewport * nvPort, WWD::Plane * npl);
   ~cLayerRenderBuffer();
   void Update();
   void Render(int x = 0, int y = 0, float fZoom = 1.0f);
   void Redraw(){ bRedraw = 1; };
   void GfxLost();
   int GetEntityCount(int x1, int y1, int x2, int y2);
 };

 struct stObjectData {
  bool bEmpty;
  cObjectQuadTree * hQuadTree;
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
  cLayerRenderBuffer * hRB;
 };
 class EditingWW;

 class EditingWWActionListener;

 class EditingWWvpCallback: public WIDG::VpCallback {
  private:
   EditingWW * m_hOwn;
  public:
   virtual void Draw(int iCode);
   EditingWWvpCallback(EditingWW * owner);
 };

 class EditingWWlModel: public gcn::ListModel {
  private:
   EditingWW * m_hOwn;
   int m_iType;
  public:
   EditingWWlModel(EditingWW * owner, int type);

   std::string getElementAt(int i);
   int getNumberOfElements();
 };

 class EditingWWActionListener: public gcn::ActionListener {
  private:
   EditingWW * m_hOwn;
  public:
   void action(const gcn::ActionEvent &actionEvent);
   EditingWWActionListener(EditingWW * owner);
 };

 class Viewport {
  private:
   int iX, iY, iW, iH;
   HTARGET target;
   hgeSprite * sprViewport;
   bool bRedrawViewport;
   EditingWW * hOwn;
   vpFoc * hFocuser;
  public:
   void MarkToRedraw(bool mark);
   bool IsMarkedToRedraw(){ return bRedrawViewport; };
   void Update();
   void Render();

   Viewport(EditingWW * phOwn, int x, int y, int w, int h);
   ~Viewport();
   void Resize(int w, int h);
   void SetPos(int x, int y);

   int GetWidth(){ return iW; };
   int GetHeight(){ return iH; };
   int GetX(){ return iX; };
   int GetY(){ return iY; };
   vpFoc * GetWidget(){ return hFocuser; };
   void GfxLost();
   void ClipScreen();
 };

 struct win_NewMap
 {
  SHR::Win * winNewMap;
  SHR::Lab * labSelectBaseLevel;
  WIDG::Viewport * vp;
  SHR::Slider * sliBaseLvls;
  SHR::CBox * cbAddBonusPlanes;
  SHR::TextField * tfAuthor;
  SHR::TextField * tfName;
  SHR::TextField * tfPlaneWidth, * tfPlaneHeight;
  SHR::Lab * labName, * labAuthor, * labPlaneSize, * labX;
  int iSelectedBase;
  SHR::But * butOK;
  #ifdef WM_EXT_TILESETS
  float fBaseTimer[16];
  #else
  float fBaseTimer[14];
  #endif
  bool bKill;
 };

 struct win_FirstRun
 {
  SHR::Win * win;
  WIDG::Viewport * vp;
  SHR::But * runManual;
  SHR::But * website;
  bool bKill;
 };

 class EditingWW : public SHR::cState {
  public:
   SHR::Context * objContext, * tilContext, * objZCoordContext, * objmAlignContext, * objmSpaceContext; //basic contexts
   SHR::Context * objFlagContext, * objFlagDrawContext, * objFlagAddContext, * objFlagDynamicContext; //flags contexts
   SHR::Context * advcon_Warp, * advcon_Container; //advanced contexts (object specific)
   friend class cAppMenu;
   SHR::ContextModel * conmodObject, * conmodObjectMultiple, * conmodTilesSelected, * conmodTilesPaste, * conmodEditableObject;
   SHR::ContextModel * conmodPaste, * conmodSpawnPoint, * conmodAtEmpty;//, * conmodUseAsBrush;
   std::vector<TileGhost> vTileGhosting;

   SHR::Contener * conResizeUp, * conResizeLeft, * conResizeRight, * conResizeDown;
   SHR::But * butExtLayerLeft, * butExtLayerRight, * butExtLayerUp, * butExtLayerDown, * butExtLayerUR, * butExtLayerUL, * butExtLayerDL,
            * butExtLayerDR;

   gcn::Gui * gui;
   SHR::Slider * sliVer, * sliHor;

   cmmbTile   * hmbTile;
   cmmbObject * hmbObject;

   cModeMenuBar * hmbActive;

   SHR::But * butIconMove;

   SHR::Contener * conWriteID;
   SHR::TextField * tfWriteID;

#ifdef WM_ADD_LUA_EXECUTER
   SHR::But * butIconLua;
#endif

   SHR::TextField * tfwpName, * tfwpAuthor, * tfwpDate, * tfwpREZ, * tfwpTiles, * tfwpPalette, * tfwpExe,
                  * tfwpMapVersion, * tfwpMapBuild, * tfwpWapVersion;
   SHR::TextBox * tbwpMapDescription;
   SHR::ScrollArea * sawpMapDescription;
   SHR::Lab * labwpMapVersion, * labwpMapBuild, * labwpMapDescription, * labwpWapVersion;

   SHR::But * butwpSave, * butwpCancel;

   cNativeController * hNativeController;

   SHR::DropDown * ddActivePlane, * ddmsPlane;
   SHR::Slider * slimsScale, * sliTilePicker, * slimsCompression;
   SHR::Lab * labmsSaveAs, * labmsScale, * labmsPlane, * labmsDimensions;
   SHR::TextField * tfmsSaveAs, * winobjseaName;
   SHR::But * butmsSaveAs, * butmsSave;
   SHR::Win * winWorld, * winMapShot, * winDB, * winSearchObj;
   SHR::CBox * cbmsOpenFileDirectory;
   SHR::CBox * cbwpCompress, * cbwpZCoord, * cbdbFlip, * cbdbAniAnimate, * cbdbBorderAni, *cbdbAniLoop;
   SHR::ScrollArea * sadbAssetsImg, * sadbFramesImg, * sadbAssetsAni, * sadbFramesAni, * sadbAssetsSnd;
   SHR::Lab * labwpName, * labwpAuthor, * labwpDate, * labwpREZ, * labwpTiles, * labwpPalette, * labwpExe, * labmsCompression,
            * labmsCompDisc;
   SHR::ListBox * dbAssetsImg, * dbFramesImg, * dbAssetsAni, * dbFramesAni, * dbAssetsSnd;

   SHR::Win * winSoundBrowser;
   SHR::ListBox * lbdbsSounds;
   SHR::Lab * labdbsDisplay;
   SHR::CBox * cbdbsOriginal, * cbdbsCustom;
   WIDG::Viewport * vpdbsDraw;
   SHR::But * butdbsAddSounds, * butdbsReload, * butdbsDelete;

   cSoundPlayer * spdbSounds;

   SHR::Win * winLogicBrowser;
   SHR::ScrollArea * sabrlLogicList;
   SHR::ListBox * lbbrlLogicList;
   SHR::But * butbrlNew, * butbrlEdit, * butbrlEditExternal, * butbrlDelete, * butbrlBrowseDir, * butbrlRenameOK, * butbrlRename;
   SHR::Lab * labbrlLogicName, * labbrlFilePath, * labbrlFileSize, * labbrlFileChecksum, * labbrlFileModDate,
            * labbrlLogicNameV, * labbrlFilePathV, * labbrlFileSizeV, * labbrlFileChecksumV, * labbrlFileModDateV;
   SHR::TextField * tfbrlRename;
   WIDG::Viewport * vpLogicBrowser;
   void DrawLogicBrowser();
   void SyncLogicBrowser();

   float fdbAniTimer;

   SHR::TabbedArea * tadbTabs;
   SHR::Contener * condbTiles, * condbImages, * condbAnims, * condbSounds;
   SHR::Lab * labdbAnimSpeed, * labdbAniImageSet;
   SHR::DropDown * dddbAnimSpeed;
   SHR::TextField * tfdbAniImageSet;

   SHR::RadBut * rbdbisShow[3];
   SHR::Lab * labdbisSetChecksum, * labdbisSetFileSize, * labdbisSetFileCount,
            * labdbisFileChecksum, * labdbisFileSize, * labdbisFileDim, * labdbisFileOffset, * labdbisFileUserData,
            * labdbisFileID, * labdbisFileIndex, * labdbisFrameID;
   SHR::CBox * cbdbisFlipX, * cbdbisFlipY, * cbdbisOffsetBorder, * cbdbisBorder;
   int idbisMoveX, idbisMoveY;
   float fdbisZoom;

   SHR::But * butObjSearchSelectAll;
   SHR::DropDown * ddObjSearchTerm;
   SHR::CBox * cbObjSearchCaseSensitive;
   SHR::Slider * sliSearchObj;

   SHR::Win * winCamera;
   SHR::Lab * labcamCoordX, * labcamCoordY, * labObjSearchResults;
   SHR::TextField * tfcamSetToX, * tfcamSetToY;
   SHR::But * butcamSetTo, * butcamSetToSpawn;

   SHR::Win * winObjectBrush;
   SHR::Lab * labobrSource;
   SHR::Lab * labobrDistance, * labobrDispX, * labobrDispY;
   SHR::TextField * tfobrDispX, * tfobrDispY;
   SHR::Slider * sliobrDistance;
   SHR::CBox * cbobrApplyScatterSeparately;

   SHR::CBox * cbmsDrawObjects;
   SHR::Lab * labloadLastOpened, * labClawPath, * labLang;
   SHR::Link * lnkLastOpened[10];

   SHR::Win * winDuplicate;
   SHR::Lab * labdTimes, * labdOffsetX, * labdOffsetY, * labdCopiedObjectID, * labobjseaInfo, * labdChangeZ;
   SHR::TextField * tfdTimes, * tfdOffsetX, * tfdOffsetY, * tfdChangeZ;
   SHR::But * butdOK;
   WWD::Object * objdSource;

   int iobjbrLastDrawnX, iobjbrLastDrawnY;

   SHR::Win * winTileProp;
   SHR::But * buttpPrev, * buttpNext, * buttpZoom, * buttpApply, * buttpShow, * buttpPipette;
   SHR::TextField * tftpTileID, * tftpW, * tftpH, * tftpX1, * tftpX2, * tftpY1, * tftpY2;
   SHR::RadBut * rbtpSingle, * rbtpDouble, * rbtpIn[5], * rbtpOut[5];
   WWD::TileAtrib * htpWorkingAtrib;
   bool btpDragDropMask;
   int itpDDx1, itpDDy1;
   int itpSelectedTile;
   bool btpZoomTile;

   WIDG::Viewport * vpobjseaRender, * vptpfcPalette;

   SHR::Win * winMeasureOpt;
   SHR::But * butmeasClear;
   SHR::CBox * cbmeasAbsoluteDistance;

   SHR::But * butMicroTileCB, * butMicroObjectCB;

   bool bObjBrushDrawing;

   //std::vector<cObjectProp*> vObjectWins;
   WIDG::Viewport * vpMain, * vpAbout, * vpWorld, * vpLoad, * vpDB, * vpTileProp;
   char cScrollOrientation;
   bool bDragDropScroll;
   float fDragLastMx, fDragLastMy;

   float fObjPickLastMx, fObjPickLastMy;

   int iMode;
   int iActiveTool;

   bool ** bFloodFillBuf;

   int iTilePicked;

   bool bClipboardCopy;

   char szHint[256];
   //int iHintID;
   float fHintTime;

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
   EditingWW(WWD::Parser * phParser);
   virtual bool Opaque();
   virtual void Init();
   virtual void Destroy();
   virtual bool Think();
   virtual void PreRender();
   virtual bool Render();
   virtual void GainFocus(int iReturnCode, bool bFlipped);
   virtual bool PromptExit();
   virtual void GfxRestore();
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
   WWD::Plane * GetActivePlane();
   void SetHint(const char * pszFormat, ...);

   void SetTool(int iNTool);
   void OpenTool(int iTool);
   void CloseTool(int iTool);

   void FreeResources();
   void MarkUnsaved();
   void SwitchPlane();

   void SetMode(int piMode);
   void EnterMode(int piMode);
   void ExitMode(int piMode);

   bool TileThink(bool pbConsumed);
   bool ObjectThink(bool pbConsumed);

   void FloodFill(int base, int x, int y, int tile);
   void DrawTileAtrib(int tileid, float posx, float posy, float width, float height);

   char * ExportObject(WWD::Object * obj);

   void OpenObjectWindow(WWD::Object * obj, bool bMove = 0);

   void SynchronizeWithParser();
   void InitEmpty();

   WWD::Object * GetObjUnderWorldPos(int mx, int my);

   SHR::But * MakeButton(int x, int y, EnumGfxIcons icon, SHR::Contener * dest, bool enable = 1, bool visible = 1);

   void RenderToViewportBuffer();

   bool bConstRedraw;

   float fCamLastX, fCamLastY, fLastZoom;

   int iTilePreviewX, iTilePreviewY;

   cBankTile * hTileset;
   cBankImageSet * SprBank;
   cDataController * hDataCtrl;

   void ShowAndUpdateDuplicateMenu();

   int GetObjectType(WWD::Object * obj);

   float fObjContextX, fObjContextY;
   bool bDrawTileProperties;
   bool bLockScroll;

   //camera coords calculation
   int Scr2WrdX(WWD::Plane * pl, int px); //screen to world X
   int Scr2WrdY(WWD::Plane * pl, int py); //screen to world Y
   int Wrd2ScrX(WWD::Plane * pl, int px); //world to screen X
   int Wrd2ScrY(WWD::Plane * pl, int py); //world to screen Y
   int Wrd2ScrXrb(WWD::Plane * pl, int px); //world to screen X within renderbuffer
   int Wrd2ScrYrb(WWD::Plane * pl, int py); //world to screen Y within renderbuffer

   void UpdateScrollBars();
   WWD::Parser * hParser;

   void SyncAtribMenuWithTile();

   std::vector< std::pair<int,int> > m_vMeasurePoints;

   void DrawTileAtrib(WWD::TileAtrib * atrib, float posx, float posy, float width, float height);

   std::vector<WWD::Object*> GetObjectsInArea(WWD::Plane * plane, int x, int y, int w, int h);

   bool bInitedAutoUpdate;

   void DrawObjSearch();

   char * szObjSearchBuffer;

   std::vector< std::pair<int,int> > vObjSearchResults;

   cAutoUpdater * hAU;

   int RenderLayer(WWD::Plane * hPl, bool bDefaultZoom);

   bool bObjDragSelection;
   int iObjDragOrigX, iObjDragOrigY;
   void ObjectOverlay();
   int iMoveRelX, iMoveRelY;
   void NotifyObjectDeletion(WWD::Object * obj);

   Viewport * vPort;
   cBankSound * hSndBank;
   cBankAni * hAniBank;
   cBankLogic * hCustomLogics;
   WWD::Object * hStartingPosObj;
   SHR::Contener * conMain;

   float fCamX, fCamY;
   float fZoom;
   float fDestZoom;

   std::vector<PlaneData*> hPlaneData;
   DWORD dwBG;
   cColorPicker * cpmsPicker;

   float fDoubleClickTimer;
   int iDoubleClickX, iDoubleClickY;
   SHR::Win * wintpFillColor;
   SHR::But * buttpfcShow;
   void DrawSelectFillColor();

   int iTileSelectX1, iTileSelectY1, iTileSelectX2, iTileSelectY2;
   int iTileWriteIDx, iTileWriteIDy;
   void DrawPlaneOverlay(WWD::Plane * hPl);

   void ViewportOverlay();

   bool bSimFold;
   float fSimFoldY;

   float fCameraMoveTimer;

   #ifdef BUILD_DEBUG
    bool bShowConsole;
   #endif

   cMDI * MDI;

   bool bDragWindow;
   int iWindowDragX, iWindowDragY;

   EditingWWvpCallback * vp;

   void SetZoom(float fZ);
   void RenderCloudTip(int x, int y, int w, int h, int ax, int ay);

   bool bForceTileClipbPreview, bForceObjectClipbPreview;

   void RenderTileClipboardPreview();
   void RenderObjectClipboardPreview();

   void UpdateSearchResults();

   //EWW_TOOL_OBJSELAREA tool vars
   SHR::Win * wintoolSelArea; //tool window
   SHR::But * buttoolSelAreaOK, * buttoolSelAreaPickMinX, * buttoolSelAreaPickMinY,//accept, select min x, select min y buttons
        * buttoolSelAreaPickMaxX, * buttoolSelAreaPickMaxY, * buttoolSelAreaAll; //select max x, select max y, select by area buttons
   SHR::Lab * labtoolSelAreaValues; //label with values
   int toolsaMinX, toolsaMinY, toolsaMaxX, toolsaMaxY; //cached values
   int toolsaAction; //selected action

   //PLANE MANAGER vars
   SHR::Win * winpmMain;
   SHR::ListBox * lbpmPlanes;
   SHR::ScrollArea * sapmPlanes;
   SHR::Lab * labpmPlanes, * labpmAnchor, * labpmWidth, * labpmHeight;
   WIDG::Viewport * vppm;
   SHR::Lab * labpmName, * labpmTileSize, * labpmTileSize_x, * labpmMovX, * labpmMovY, * labpmPlaneSize, * labpmMovement, * labpmZCoord, * labpmFlags, * labpmTileSet;
   SHR::TextField * tfpmName, * tfpmTileSizeX, * tfpmTileSizeY, * tfpmPlaneSizeX, * tfpmPlaneSizeY, * tfpmMovX, * tfpmMovY, * tfpmZCoord;
   SHR::DropDown * ddpmTileSet;
   SHR::TextBox * tbpmImagesets;
   SHR::But * butpmResUL, * butpmResU, * butpmResUR, * butpmResL, * butpmResC, * butpmResR, * butpmResDL, * butpmResD, * butpmResDR;
   SHR::CBox * cbpmFlagMainPlane, * cbpmFlagNoDraw, * cbpmFlagWrapX, * cbpmFlagWrapY, * cbpmAutoTileSize;
   SHR::But * butpmSave, * butpmDelete;
   int ipmAnchor, ipmSizeX, ipmSizeY;
   void SyncPlaneProperties();
   void DrawPlaneProperties();
   void SavePlaneProperties();
   void DeletePlaneProperties();
   void SetAnchorPlaneProperties(int anchor);

   void HandleHotkeys();

   //events
   void ApplicationStartup(); //executed when intro is done
   void PrepareForDocumentSwitch();
   void DocumentSwitched();

   //new map
   win_NewMap * NewMap_data;
   void NewMap_Open();
   void NewMap_Close();
   void NewMap_Think();
   void NewMap_OK(); //called when OK button is pressed
   void NewMap_Validate();

   //first run
   win_FirstRun * FirstRun_data;
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
   ObjEdit::cObjEdit * hEditObj;
   bool IsEditableObject(WWD::Object * obj, ObjEdit::cObjEdit ** hEdit = 0); //whether 'easy edit' is available
   void OpenObjectEdit(WWD::Object * obj);

   std::vector<WWD::Object*> vObjectsPicked, vObjectsHL, vObjectsForbidHL, vObjectClipboard, vObjectsBrushCB;

   void SwitchActiveModeMenuBar(cModeMenuBar * n);
   void CreateObjectWithEasyEdit(gcn::Widget * widg);

   cInventoryController * hInvCtrl;

   bool bShowHand;
   DWORD dwCursorColor;

   bool IsObjectEnemy(WWD::Object * obj);

   std::vector<gcn::Widget*> vWidgetsToMinimalise;
   std::vector<gcn::Widget*> vMinimalisedWidgets;
   void MinimaliseWindows();
   void MaximaliseWindows();

   bool bObjectAlignAxis;

   bool AreObjectSpecificOptionsAvailable(WWD::Object * obj, SHR::Context ** conMod = 0);

   int GetActiveMode(){ return iMode; };

   //app bar
   float fAppBarTimers[2];

   //window spacing
   SHR::TextField * tfspacingDistance;
   SHR::Win * winSpacing;
   SHR::Lab * labspacingDistance;
   SHR::But * butspacingOK;

   //welcome screen
   SHR::Contener * winWelcome;
   SHR::But * butwsNew, * butwsOpen, * butwsRecently, * butwsWhatsnew;
   WIDG::Viewport * vpws;
   SHR::Lab * labwsRecently;
   void DrawWelcomeScreen();

   void UpdateSettings();

   //wincrashretrieve
   SHR::Contener * conCrashRetrieve, * conRecentFiles;
   SHR::But * butCrashRetrieve;
   WIDG::Viewport * vpCrashRetrieve;
   char * szCrashRetrieve[10];
   byte iCrashRetrieveIcon[10];
   char * szCrashRetrieveMore;
   void DrawCrashRetrieve();
   std::vector<std::string> vstrMapsToLoad;

   //MRU
   cMruList * hMruList;
   void MruListUpdated(); //called from cMruList when list has been updates

   void RenderAreaRect(WWD::Rect r, WWD::Rect dr, bool bClip, DWORD hwCol, bool bFill, DWORD hwFillCol); //renders area in viewport, given absolute screen-coords, if any coord = 0 then no draw
   void RenderArrow(int x, int y, int x2, int y2, bool finished, bool setcolors = 1);

   bool ValidateLevelName(const char * name, bool bAllowNoNum);
   char * FixLevelName(int iBaseLvl, const char * name);

   std::vector<cInventoryItem> GetContenerItems(WWD::Object * obj);

   cServerIPC * hServerIPC;

   EditingWWActionListener * al;

   WWD::Plane * plMain;

   cAppMenu * hAppMenu;
   byte gamesLastOpened[10];
   int PromptForDocument(char * dest);

   int iTileDrawStartX, iTileDrawStartY;

   SHR::Slider * sliZoom;

   //tilepicker
   SHR::Win * winTilePicker;
   SHR::But * buttpiModePencil, * buttpiModeBrush, * buttpiSpray, * buttpiReloadBrush,
            * buttpiPoint, * buttpiLine, * buttpiRect, * buttpiEllipse, * buttpiFlood;
   SHR::Lab * labtpiReloadBrush, * labtpiModePencil, * labtpiModeBrush, * labtpiToolOptions, * labtpiPointSize, * labtpiSpraySize,
            * labtpiSprayDensity, * labtpiLineThickness;
   SHR::CBox * cbtpiRectFilled, * cbtpiEllipseFilled;
   SHR::Slider * slitpiPointSize, * slitpiSpraySize, * slitpiSprayDensity, * slitpiLineThickness;
   SHR::CBox * cbtpiShowTileID, * cbtpiShowProperties, * cbtpiDrawFilled;
   SHR::TextField * tftpiPointSize;
   SHR::Slider * slitpiPicker;
   int iTilePickerOffUp, iTilePickerOffDown;
   int iTileDrawMode;
   int iPipetteTileHL;
   bool btpiFixedPos;
   WIDG::Viewport * vptpi;

   void RebuildTilePicker();
   void RefreshTilePickerSlider();
   void DrawTilePicker();
   void HandleBrushSwitch(int itOld, int itNew);

   void SyncDB_ImageSets();

   int iManipulatedGuide;
   bool bShowGuideLines;

   int RenderPlane(WWD::Plane * plane, int pl);
   int RenderObject(WWD::Object * hObj, int x, int y, DWORD col);
   int iTilesOnScreen, iObjectsOnScreen;

   SHR::ComboBut * cbutActiveMode;

   std::vector<cWindow*> hWindows;
   winTileBrowser * hwinTileBrowser;
   winImageSetBrowser * hwinImageSetBrowser;
   winOptions * hwinOptions;
   winAbout * hwinAbout;
 };
};

#endif
