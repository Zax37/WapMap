#ifndef H_C_OBJECTEDIT
#define H_C_OBJECTEDIT

#include "../../shared/cStateMgr.h"
#include "../../shared/gcnWidgets/wContainer.h"
#include "../../shared/gcnWidgets/wSlider.h"
#include "../../shared/gcnWidgets/wWin.h"
#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wScrollArea.h"
#include "../../shared/gcnWidgets/wCheckbox.h"
#include "../../shared/gcnWidgets/wDropDown.h"
#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wTextField.h"
#include "../../shared/gcnWidgets/wTabbedArea.h"
#include "../../shared/gcnWidgets/wRadioBut.h"
#include "../../shared/cWWD.h"
#include "../wViewport.h"
#include "../cSoundPlayer.h"
#include "guichan.hpp"
#ifdef WAPMAP_ENABLE_IMGUI
#include <array>
#include <map>
#endif

namespace State {
    class EditingWW;
}

class cBankImageSet;
namespace SHR { class InvTab; }

namespace ObjEdit {
    enum enType {
        enCurse = 0,
        enPathElevator,
        enCheckpoint,
        enWarp,
        enWallCannon,
        enCrumblingPeg,
        enStatue,
        enBreakPlank,
        enCrate,
        enTreasure,
        enRope,
        enHealth,
        enSpecialPowerup,
        enTogglePeg,
        enCandy,
        enSpringboard,
        enDialog,
        enProjectile,
        enElevator,
        enCrabNest,
        enShake,
        enStalactite,
        enLaser,
        enAmbient,
        enEnemy,
        enText,
        enFloorSpike,
        enLavaHand
    };

    class EditingWW;

    class cObjEdit;

    class cObjEditVP : public WIDG::VpCallback {
    protected:
		cObjEdit *m_hOwn;
    public:
		virtual void Draw(int iCode);

        cObjEditVP(cObjEdit *owner) { m_hOwn = owner; };
    };

    class cObjEditAL : public gcn::ActionListener {
    private:
        cObjEdit *m_hOwn;
    public:
        void action(const gcn::ActionEvent &actionEvent);

        cObjEditAL(cObjEdit *owner) { m_hOwn = owner; };
    };

    class cObjEdit {
    private:
        bool bObjectSaved, bChangesMade;
        bool _bAddNext;

        void SaveChanges();

    protected:
        int _iDragOffX, _iDragOffY;
#ifdef WAPMAP_ENABLE_IMGUI
        bool nativeWindow = false, nativePosition = true, nativeInputFocused = false, nativeBusy = false;
        bool nativeValid = true;
        float nativeHeightLimit = 0, nativeFooterY = -1;
        struct NativeInteger { std::array<char, 64> text{}; std::string legacy; };
        std::map<SHR::TextField*, NativeInteger> nativeIntegers;
        struct NativeText { std::array<char, 4096> text{}; std::string legacy; };
        std::map<gcn::Widget*, NativeText> nativeTexts;
        gcn::Widget* nativeAction = nullptr;
        SHR::InvTab* nativeInventoryTarget = nullptr;
        int nativeInventoryItem = -1, nativeInventorySelection = -1;
        bool BeginNativeWindow();
        void DrawNativeControls(SHR::Container* container = nullptr, int offsetX = 0, int offsetY = 0);
        void DrawNativeInventory(const int* items, SHR::RadBut* const* choices, int count, float effectY);
        void DrawNativeSeries(SHR::Win* panel, SHR::TextField* delay, SHR::TextField* increment);
        void DrawNativeImageChoices(const std::vector<std::string>& sets, SHR::RadBut* const* choices, int frame);
        void EndNativeWindow();
#endif
        DWORD dwHighlightColor;
        bool bAllowDragging;
        bool _bDragging;
		cObjEditVP *vpCB;

        WWD::Object *hOrigObj;
        WWD::Object *hTempObj;

        WIDG::Viewport *vpAdv;
        State::EditingWW *hState;
        bool bKill;
        cObjEditAL *hAL;

		SHR::Win *win;
        SHR::But *_butAddNext, *_butSave;

        friend class cObjEditVP;

        friend class cObjEditAL;

        bool ChangesMade();

        virtual void _Think(bool bMouseConsumed) {};
    public:
        enType iType;

        virtual ~cObjEdit();

        cObjEdit(WWD::Object *obj, State::EditingWW *st);

        virtual void Draw() {};
#ifdef WAPMAP_ENABLE_IMGUI
        virtual void DrawImGui();
#endif

        virtual void RenderObjectOverlay() {};

        virtual void Think(bool bMouseConsumed);

        virtual void Action(const gcn::ActionEvent &actionEvent) {};

        virtual void Save() {};

        void SetKill(bool b) { bKill = b; };

        bool Kill() { return bKill; };

        bool ObjectSaved() { return bObjectSaved; };

        WWD::Object *GetOrigObj() { return hOrigObj; };

        WWD::Object *GetTempObj() { return hTempObj; };

        bool IsAddingNext() { return _bAddNext; };

        DWORD GetHighlightColor() { return dwHighlightColor; };
        int _iMoveInitX, _iMoveInitY;

        bool IsMovingObject() { return _bDragging; };

        virtual void *GenerateNextObjectData() { return NULL; };

        virtual void ApplyDataFromPrevObject(void *ptr) {};

        virtual void ObjectMovedInsideEasyEdit(int fromX, int fromY) {};

		virtual bool IsAnyInputFocused();

        void GetWindowPosition(int& x, int& y);

        void SetWindowPosition(int x, int y);

        virtual void HandleEscape();
    };
};

#endif
