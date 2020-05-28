#ifndef H_WIN_OPTIONS
#define H_WIN_OPTIONS

#include "../../shared/gcnWidgets/wIconListbox.h"
#include "../../shared/gcnWidgets/wScrollArea.h"
#include "../shared/cWWD.h"
#include "window.h"

namespace SHR {
    class Lab;

    class But;

    class DropDown;

    class CBox;

    class TextField;
}

class cDynamicListModel;

class cListModelDisplay : public gcn::ListModel {
private:
    std::vector<std::string> m_vOpt;
public:
    cListModelDisplay(bool bCrazyHook = 0);

    ~cListModelDisplay();

    std::string getElementAt(int i);

    int getNumberOfElements();
};

class winOptions : public cWindow, SHR::IconListModel {
private:
    std::vector<gcn::Widget*> widgetsToDelete;
    std::vector<SHR::Container*> optionsForCategory;
    std::vector<SHR::TextField*> pathTextFields;
    SHR::Lab *labChangesRes, *labChangesLang, *labRes, *labLang, *labGameRes, *labCrazyHookSettings, *labVersion;
    SHR::But *butPath[WWD::Games_Count], *butSave;
    SHR::DropDown *ddoptLang, *ddoptRes, *ddoptGameRes;
    SHR::CBox *cbOptionsAlfaHigherPlanes, *cboptCrazyHookGodMode, *cboptCrazyHookArmor, *cboptCrazyHookDebugInfo,
            *cboptAutoUpdate, *cboptSmoothZooming;
    WIDG::Viewport *vp;
    cDynamicListModel *lmLang;
    SHR::ScrollArea *scrollAreaCategories;
    SHR::ListBox *settingsCategoriesList;
public:
    winOptions();

    ~winOptions();

    virtual void Draw(int piCode);

    virtual void Think();

    virtual void Open(WWD::GAME game = WWD::Game_Unknown);

    virtual void Close();

    virtual void action(const ActionEvent &actionEvent);

    void SyncWithExe();

    void UpdatedClawExePath(bool forceDetect = false);

    void PickAndSetGameLocation(WWD::GAME);

    std::string getElementAt(int i) override;

    hgeSprite* getIcon(int i) override;

    int getNumberOfElements() override { return WWD::Games_Count + 1; }
};

#endif // H_WIN_TILEBROWSER
