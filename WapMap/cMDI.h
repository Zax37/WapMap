#ifndef H_C_MDI
#define H_C_MDI

#include "../shared/cWWD.h"
#include "../shared/gcnWidgets/wContext.h"
#include <set>
#ifdef WAPMAP_ENABLE_IMGUI
#include <imgui.h>
#endif

#define MDI_CONTEXT_CLOSEEXCEPTACTIVE 1
#define MDI_CONTEXT_CLOSEALL          2
#define MDI_CONTEXT_PREVIOUSLYCLOSED  3
#define MDI_CONTEXT_RELOAD            4

namespace State {
    struct PlaneData;
    class Viewport;
};

struct DocumentData;

class cBankLogic;

class cBankTile;

class cBankImageSet;

class cBankSound;

class cBankAni;

class cBankMeta;

class cBankPalettes;

class cDataController;

class cMDI;

class cTabMDI {
public:
    cMDI* hMDI;
    int index;
    DocumentData *dd;
    bool bFocused, bSelected, bCloseFocused;
    float fTimer, fCloseTimer;

    static int MaxTabWidth;

    cTabMDI();

    ~cTabMDI();

    void Update();

    int GetWidth();

    int Render(int x, int y, bool bdisabled, bool bselected);

    void RenderSeparator(int x, int y, bool right = false);

    static void RenderBG(int x, int y, int w, int st, bool bfirst, bool bclosed);
};

#define GUIDE_HORIZONTAL 1
#define GUIDE_VERTICAL   0
struct stGuideLine {
    int iPos;
    bool bOrient;
};

struct stLocation {
    char Name[64] = {0};
    unsigned int X;
    unsigned int Y;
    WWD::Object *Object = NULL;
};

struct DocumentData {
    cBankImageSet *hSprBank;
    cBankSound *hSndBank;
    cBankAni *hAniBank;
    cBankTile *hTilesBank;
    cBankLogic *hCustomLogicBank;
    cBankMeta* hMetaBank;
    cBankPalettes* hPalettesBank;
    cDataController *hDataCtrl;

    cTabMDI *hTab;
    State::Viewport *hViewport = nullptr;
    int iViewportWidth = 0;
    int iViewportHeight = 0;
#ifdef WAPMAP_ENABLE_IMGUI
    ImGuiID iImGuiViewportId = 0;
#endif
    int iTilesOnScreen = 0;
    int iObjectsOnScreen = 0;
    bool bImGuiOpen = true;

    WWD::Parser *hParser;
    char *szFileName;
    std::vector<State::PlaneData *> hPlaneData;
    WWD::Object *hStartingPosObj;
    float fCamX, fCamY;
    float fPrevWarpX = 0.0f;
    float fPrevWarpY = 0.0f;
    int iSelectedPlane;
    bool bSaved;
    std::vector<WWD::Object *> vObjectsPicked;
    float fZoom;
    int iTileSelectX1, iTileSelectY1, iTileSelectX2, iTileSelectY2;

    //meta
    std::vector<stGuideLine> vGuides;
    std::vector<stLocation> vFavLocations;
    int iWapMapBuild, iMapBuild;
    std::string strWapMapVersion, strMapVersion, strMapDescription;
};

class cMDI : public gcn::ActionListener {
private:
    std::set<WWD::Parser *> m_vhAutoFixedMaps;
    std::vector<DocumentData *> m_vhDoc;
    std::vector<std::string> vstrRecentlyClosed;
    int m_iActiveDoc, m_iLastActiveDoc, m_iContextMenuFocusedDoc;
    int m_iRenderingDoc = -1;
    DocumentData *m_pRequestedActiveDoc = nullptr;
    DocumentData *m_pRequestedCloseDoc = nullptr;
    DocumentData *m_pContextDoc = nullptr;
    DocumentData *m_pContextActionDoc = nullptr;
    SHR::Context *m_pContextAction = nullptr;
    int m_iContextActionId = -1;
    DocumentData *m_pNavigationDoc = nullptr;
    float m_fNavigationX = 0, m_fNavigationY = 0;
    DocumentData *m_pGuideDragDoc = nullptr;
    bool m_bGuideHorizontal = false;
    DocumentData *m_pGuideActionDoc = nullptr;
    stGuideLine m_guideAction{};
    DocumentData *m_pMapHoveredDoc = nullptr;
    DocumentData *m_pMapCapturedDoc = nullptr;
    float m_fMapInputOriginX = 0.0f;
    float m_fMapInputOriginY = 0.0f;
    float m_fMapCaptureOriginX = 0.0f;
    float m_fMapCaptureOriginY = 0.0f;
    ImGuiID m_iMapInputViewportId = 0;
    ImGuiID m_iMapCaptureViewportId = 0;
    unsigned int m_iPendingMapMouseDown = 0;
    int m_iPendingMapWheel = 0;
    int m_iMainViewportDockedDocCount = 0;
    bool bBlock;
    bool bUpdateCrashList;
    SHR::Context *hContext;
    SHR::Context *hContextClosed;
    int m_iPosY, m_iAddTabButtonOffset;
    bool bReloadingMap;
    //float fAddTabTimer;

    void RebuildContext(bool bForceRebuildBase);
#ifdef WAPMAP_ENABLE_IMGUI
    void DrawDocumentNavigation(DocumentData* doc, ImVec2 origin, ImVec2 imageMin, ImVec2 imageSize, bool interactive);
#endif

    friend class cTabMDI;
public:
    cMDI();

    ~cMDI();

    void MarkAsAutoFixed(WWD::Parser * hParser) { m_vhAutoFixedMaps.insert(hParser); }

    DocumentData *AddDocument(DocumentData *dd);

    void SetActiveDoc(DocumentData *doc);

    void SetActiveDocIt(int it);

    DocumentData *GetDocByIt(int i) {
        if (i >= 0 && i < m_vhDoc.size()) return m_vhDoc[i];
        return NULL;
    };

    DocumentData *GetActiveDoc() {
        const int index = m_iRenderingDoc == -1 ? m_iActiveDoc : m_iRenderingDoc;
        if (index == -1) return NULL;
        return m_vhDoc[index];
    };

    int GetActiveDocIt() { return m_iActiveDoc; };

    void PrepareDocToSave(int i);

    bool IsAnyDocUnsaved();

    void SaveCurrent();

    bool CanBeClosed(int i);

    bool CloseDocByIt(int i);

    void DeleteDocByIt(int i);

    int GetDocsCount() { return m_vhDoc.size(); };

    void Think(bool bConsumed);

    void Render();

#ifdef WAPMAP_ENABLE_IMGUI
    void DrawImGuiViews(unsigned int dockspaceId);
    void ProcessImGuiActions();
    void RouteImGuiInput();
    bool HasDockedMainViewportDoc() const { return m_iMainViewportDockedDocCount > 0; }
    void RequestImGuiActiveDoc(DocumentData *doc) { m_pRequestedActiveDoc = doc; }
    void RequestImGuiCloseDoc(DocumentData *doc) { m_pRequestedCloseDoc = doc; }
    void BeginRenderingDoc(int index) { m_iRenderingDoc = index; }
    void EndRenderingDoc() { m_iRenderingDoc = -1; }
#endif

    bool IsDocumentSelectionBlocked() { return bBlock; };

    void BlockDocumentSelection(bool b) { bBlock = b; };

    void UpdateCrashList();

    void action(const gcn::ActionEvent &actionEvent);

    void SetY(int y) { m_iPosY = y; };

    SHR::Context *GetClosedContext() { return hContextClosed; };

    std::string& GetMostRecentlyClosedDoc() { return vstrRecentlyClosed[0]; }

    int GetCachedClosedDocsCount() { return vstrRecentlyClosed.size(); };

    void CloseAllDocs() { hContext->EmulateClickID(MDI_CONTEXT_CLOSEALL); };

    void UnlockMapReload() { bReloadingMap = 0; };

    void UpdateMaxTabSize();

    void BackToLastActive() { SetActiveDocIt(m_iLastActiveDoc); }

    bool IsDocAlreadyOpen(const std::string& path);
};

#endif
