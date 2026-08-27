#ifndef H_C_OBJECTEDIT
#define H_C_OBJECTEDIT

#include "../../shared/cStateMgr.h"
#include "../../shared/gcnwidgets/wContainer.h"
#include "../../shared/gcnwidgets/wSlider.h"
#include "../../shared/gcnwidgets/wWin.h"
#include "../../shared/gcnwidgets/wButton.h"
#include "../../shared/gcnwidgets/wScrollArea.h"
#include "../../shared/gcnwidgets/wCheckbox.h"
#include "../../shared/gcnwidgets/wDropDown.h"
#include "../../shared/gcnwidgets/wLabel.h"
#include "../../shared/gcnwidgets/wTextField.h"
#include "../../shared/gcnwidgets/wTabbedArea.h"
#include "../../shared/gcnwidgets/wRadioBut.h"
#include "../../shared/cWWD.h"
#include "../wViewport.h"
#include "../cSoundPlayer.h"
#include "guichan.hpp"

namespace State {
    class EditingWW;
}

class cBankImageSet;

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
