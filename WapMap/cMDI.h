#ifndef H_C_MDI
#define H_C_MDI

#include "../shared/cWWD.h"
#include "../shared/gcnWidgets/wContext.h"

#define MDI_CONTEXT_CLOSEEXCEPTACTIVE 1
#define MDI_CONTEXT_CLOSEALL          2
#define MDI_CONTEXT_PREVIOUSLYCLOSED  3
#define MDI_CONTEXT_RELOAD            4

namespace State
{
 struct PlaneData;
};

struct DocumentData;

class cBankLogic;
class cBankTile;
class cBankImageSet;
class cBankSound;
class cBankAni;
class cDataController;

class cTabMDI
{
 public:
  DocumentData * dd;
  bool bFocused, bSelected;
  float fTimer, fCloseTimer;
  cTabMDI();
  ~cTabMDI();

  void Update();
  int GetWidth();
  int Render(int x, int y, bool bdisabled, bool bselected, bool blast);

  static void RenderBG(int x, int y, int w, int st, bool bfirst, bool bclosed);
};

#define GUIDE_HORIZONTAL 1
#define GUIDE_VERTICAL   0
struct stGuideLine {
 int iPos;
 bool bOrient;
};

struct DocumentData {
 cBankImageSet * hSprBank;
 cBankSound * hSndBank;
 cBankAni * hAniBank;
 cBankTile * hTileset;
 cBankLogic * hCustomLogics;
 char * hTileClipboardImageSet;
 int iTileCBw, iTileCBh;
 WWD::Tile ** hTileClipboard;
 cDataController * hDataCtrl;

 cTabMDI * hTab;

 WWD::Parser * hParser;
 char * szFileName;
 std::vector<State::PlaneData*> hPlaneData;
 WWD::Object * hStartingPosObj;
 float fCamX, fCamY;
 int iSelectedPlane;
 bool bSaved;
 std::vector<WWD::Object*> vObjectsPicked;
 float fZoom, fDestZoom;

 //meta
 std::vector<stGuideLine> vGuides;
 int iWapMapBuild, iMapBuild;
 std::string strWapMapVersion, strMapVersion, strMapDescription;
};

class cMDI: public gcn::ActionListener {
 private:
  std::vector<DocumentData*> m_vhDoc;
  std::vector<std::string> vstrRecentlyClosed;
  int m_iActiveDoc;
  bool bBlock;
  bool bMouseHand;
  bool bUpdateCrashList;
  SHR::Context * hContext;
  SHR::Context * hContextClosed;
  int m_iPosY;
  bool bReloadingMap;
  cTabMDI * hDefTab;

  void RebuildContext(bool bForceRebuildBase);
 public:
  cMDI();
  ~cMDI();

  DocumentData * AddDocument(DocumentData * dd);

  void SetActiveDoc(DocumentData * doc);
  void SetActiveDocIt(int it);

  DocumentData * GetDocByIt(int i){ if( i >= 0 && i < m_vhDoc.size() ) return m_vhDoc[i]; return NULL; };
  DocumentData * GetActiveDoc(){ if( m_iActiveDoc == -1 ) return NULL; return m_vhDoc[m_iActiveDoc]; };
  int GetActiveDocIt(){ return m_iActiveDoc; };

  void PrepareDocToSave(int i);

  bool IsAnyDocUnsaved();
  bool CloseDocByIt(int i);
  void DeleteDocByIt(int i);
  void DeleteDocByPtr(DocumentData * hdd);

  int GetDocsCount(){ return m_vhDoc.size(); };

  void Think(bool bConsumed);
  void Render();

  bool IsDocumentSelectionBlocked(){ return bBlock; };
  void BlockDocumentSelection(bool b){ bBlock = b; };

  void UpdateCrashList();
  void action(const gcn::ActionEvent &actionEvent);

  void SetY(int y){ m_iPosY = y; };
  SHR::Context * GetClosedContext(){ return hContextClosed; };
  int GetCachedClosedDocsCount(){ return vstrRecentlyClosed.size(); };

  void CloseAllDocs(){ hContext->EmulateClickID(MDI_CONTEXT_CLOSEALL); };

  void UnlockMapReload(){ bReloadingMap = 0; };
};

#endif
