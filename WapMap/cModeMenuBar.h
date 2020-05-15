#ifndef H_C_MODEMENUBAR
#define H_C_MODEMENUBAR

#include "../shared/gcnWidgets/wButton.h"
#include "../shared/gcnWidgets/wContext.h"
#include "../shared/gcnWidgets/wDropDown.h"

#define NOBJCON_PREV        0
#define NOBJCON_NEXT        1
#define NOBJCON_ADV         2
#define NOBJCON_CRUMBLE     3
#define NOBJCON_BREAKPL     4
#define NOBJCON_TOGGLE      5
#define NOBJCON_ELEVATOR    6
#define NOBJCON_PATHELEV    7
#define NOBJCON_SPRINGBRD   8
#define NOBJCON_TREASURE    9
#define NOBJCON_CURSE       10
#define NOBJCON_CRATE       11
#define NOBJCON_STATUE      12
#define NOBJCON_NPC         13
#define NOBJCON_POWDER      14
#define NOBJCON_CANNON      15
#define NOBJCON_SPIKES      16
#define NOBJCON_EYECANDY    17
#define NOBJCON_CPOINT      18
#define NOBJCON_TEXT        19
#define NOBJCON_WARP        20
#define NOBJCON_DIALOG      21
#define NOBJCON_SOUND       22
#define NOBJCON_ROPE        23
#define NOBJCON_MAP         24
#define NOBJCON_HEALTH      25
#define NOBJCON_CATNIP      26
#define NOBJCON_PROJECTILE  27
#define NOBJCON_WALCAN      28
#define NOBJCON_CRABNEST    29
#define NOBJCON_SHAKE       30
#define NOBJCON_LASER       31
#define NOBJCON_STALACTITE  32

class cModeMenuBar {
protected:
    bool bVisible, bEnabled;

    virtual void _setVisible(bool bVis) {};

    virtual void _setEnabled(bool bEn) {};
public:
    cModeMenuBar(int startX);

    virtual ~cModeMenuBar();

    void SetVisible(bool bVis);

    void SetEnabled(bool bEn);

    bool IsVisible() { return bVisible; };

    bool IsEnabled() { return bEnabled; };

    virtual void DocumentSwitched() {};
    std::vector<int> vSeparators;
};

class cmmbTile : public cModeMenuBar {
private:
protected:
    virtual void _setVisible(bool bVis);

    virtual void _setEnabled(bool bEn);

public:
    SHR::DropDown* ddActivePlane;
    SHR::But *butIconSelect, *butIconPencil, *butIconBrush, *butIconFill, *butIconWriteID;

    cmmbTile(int startX);

    ~cmmbTile();
};

class cNewObjContextEl {
public:
    std::string strCaption;
    hgeSprite *sprIcon;
    int iID;
    SHR::But *hButton;

    cNewObjContextEl(int id, std::string cap, hgeSprite *spr, SHR::But *but) {
        iID = id;
        strCaption = cap;
        sprIcon = spr;
        hButton = but;
    };
};

class cmmbObject : public cModeMenuBar, public gcn::ActionListener {
private:
    int iContextOffset;
    SHR::Context *conNewObject;
    std::vector<cNewObjContextEl> vContextElements;

    void RebuildContext();

protected:
    virtual void _setVisible(bool bVis);

    virtual void _setEnabled(bool bEn);

public:
    SHR::But *butIconSearchObject, *butIconNewObjEmpty, *butIconCrumblinPeg, *butIconBreakPlank, *butIconTogglePeg, *butIconElevator,
            *butIconPathElevator, *butIconSpringBoard, *butIconTreasure, *butIconCurse, *butIconCrate, *butIconStatue,
            *butIconPowderKeg, *butIconCannon, *butIconSpikes, *butIconEyeCandy,
            *butIconCheckpoint, *butIconText, *butIconWarp, *butIconDialog, *butIconSound, *butIconRope, *butIconMapPiece,
            *butIconHealth, *butIconCatnip, *butIconProjectile, *butIconCrabNest, *butIconShake, *butIconStalactite, *butIconLaser,
            *butIconEnemy;
    std::vector<SHR::But *> vButtons;

    cmmbObject(int startX);

    ~cmmbObject();

    SHR::Context *GetContext() { return conNewObject; };

    virtual void DocumentSwitched();

    void action(const gcn::ActionEvent &actionEvent); //inherited from ActionListener
};

#endif
